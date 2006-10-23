/* tinymail - Tiny Mail
 * Copyright (C) 2006-2007 Philip Van Hoof <pvanhoof@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with self library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <sys/mman.h>

#include <config.h>
#include <glib/gi18n-lib.h>

#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>

#include <tny-platform-factory.h>
#include <tny-gnome-platform-factory.h>
#include <tny-account-store.h>
#include <tny-gnome-account-store.h>
#include <tny-gnome-password-dialog.h>
#include <tny-account.h>
#include <tny-store-account.h>
#include <tny-transport-account.h>
#include <tny-device.h>

#include <tny-camel-account.h>
#include <tny-camel-store-account.h>
#include <tny-camel-imap-store-account.h>
#include <tny-camel-nntp-store-account.h>
#include <tny-camel-pop-store-account.h>
#include <tny-camel-transport-account.h>
#include <tny-gnome-device.h>
#include <tny-session-camel.h>

#ifdef GNOME
#include <libgnomeui/gnome-password-dialog.h>
#include <gnome-keyring.h>
#endif

/* "GConf vs. libtinymail-camel" account-store implementation */

static GObjectClass *parent_class = NULL;

typedef struct _TnyGnomeAccountStorePriv TnyGnomeAccountStorePriv;

struct _TnyGnomeAccountStorePriv
{
	GConfClient *client;
	gchar *cache_dir;
	TnySessionCamel *session;
	TnyDevice *device;
	guint notify;
};

#define TNY_GNOME_ACCOUNT_STORE_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_GNOME_ACCOUNT_STORE, TnyGnomeAccountStorePriv))


#ifdef GNOME
static gchar* 
per_account_get_pass_func (TnyAccount *account, const gchar *prompt, gboolean *cancel)
{
	gchar *retval = NULL;
	GList *list;
	GnomeKeyringResult keyringret;
	gchar *keyring;

	gnome_keyring_get_default_keyring_sync (&keyring);

	keyringret = gnome_keyring_find_network_password_sync (
		tny_account_get_user (account),
		"Mail", tny_account_get_hostname (account),
		"password", tny_account_get_proto (account), 
		"PLAIN", 0, &list);

	if (keyringret != GNOME_KEYRING_RESULT_OK)
	{
		gboolean canc = FALSE;

		GnomePasswordDialog *dialog = GNOME_PASSWORD_DIALOG 
				(gnome_password_dialog_new
					(_("Enter password"), prompt,
					tny_account_get_user (account), 
					NULL, TRUE));

		gnome_password_dialog_set_domain (dialog, "Mail");
		gnome_password_dialog_set_remember (dialog, 
			GNOME_PASSWORD_DIALOG_REMEMBER_FOREVER);
		gnome_password_dialog_set_readonly_username (dialog, TRUE);
		gnome_password_dialog_set_username (dialog, 
			tny_account_get_user (account));

		gnome_password_dialog_set_show_username (dialog, FALSE);
		gnome_password_dialog_set_show_remember (dialog, 
			gnome_keyring_is_available ());
		gnome_password_dialog_set_show_domain (dialog, FALSE);
		gnome_password_dialog_set_show_userpass_buttons (dialog, FALSE);

		canc = gnome_password_dialog_run_and_block (dialog);

		if (canc)
		{
			guint32 item_id;
			GnomePasswordDialogRemember r;

			retval = gnome_password_dialog_get_password (dialog);

			r = gnome_password_dialog_get_remember (dialog);

			if (r == GNOME_PASSWORD_DIALOG_REMEMBER_FOREVER)
			{
				gnome_keyring_set_network_password_sync (keyring,
					tny_account_get_user (account),
					"Mail", tny_account_get_hostname (account),
					"password", tny_account_get_proto (account), 
					"PLAIN", 0, retval, &item_id);
			}
		}

		*cancel = (!canc);

		/* this causes warnings, but should be done afaik */
		gtk_object_destroy (GTK_OBJECT (dialog));

		while (gtk_events_pending ())
			gtk_main_iteration ();
	} else {

		GnomeKeyringNetworkPasswordData *pwd_data;
		pwd_data = list->data;
		retval = g_strdup (pwd_data->password);

		*cancel = FALSE;

		gnome_keyring_network_password_list_free (list);
	}

	return retval;
}

static void
per_account_forget_pass_func (TnyAccount *account)
{
	GList *list=NULL;
	GnomeKeyringResult keyringret;
	gchar *keyring;
	GnomeKeyringNetworkPasswordData *pwd_data;

	gnome_keyring_get_default_keyring_sync (&keyring);

	keyringret = gnome_keyring_find_network_password_sync (
		tny_account_get_user (account),
		"Mail", tny_account_get_hostname (account),
		"password", tny_account_get_proto (account), 
		"PLAIN", 0, &list);

	if (keyringret == GNOME_KEYRING_RESULT_OK)
	{
		pwd_data = list->data;
		gnome_keyring_item_delete_sync (keyring, pwd_data->item_id);
		gnome_keyring_network_password_list_free (list);
	}
	return;
}

#else 

static GHashTable *passwords;

static gchar* 
per_account_get_pass_func (TnyAccount *account, const gchar *prompt, gboolean *cancel)
{
	gchar *retval = NULL;
	const gchar *accountid = tny_account_get_id (account);

	if (G_UNLIKELY (!passwords))
		passwords = g_hash_table_new (g_str_hash, g_str_equal);

	retval = g_hash_table_lookup (passwords, accountid);

	if (G_UNLIKELY (!retval))
	{
		GtkDialog *dialog = GTK_DIALOG (tny_gnome_password_dialog_new ());
	
		tny_gnome_password_dialog_set_prompt (TNY_GNOME_PASSWORD_DIALOG (dialog), prompt);

		if (G_LIKELY (gtk_dialog_run (dialog) == GTK_RESPONSE_OK))
		{
			const gchar *pwd = tny_gnome_password_dialog_get_password 
				(TNY_GNOME_PASSWORD_DIALOG (dialog));
	
			retval = g_strdup (pwd);

			mlock (retval, strlen (retval));

			g_hash_table_insert (passwords, g_strdup (accountid), 
				retval);

			*cancel = FALSE;

		} else {

			*cancel = TRUE;

		}

		gtk_widget_destroy (GTK_WIDGET (dialog));

		while (gtk_events_pending ())
			gtk_main_iteration ();
	} else {
		*cancel = FALSE;
	}

	return retval;
}

static void
per_account_forget_pass_func (TnyAccount *account)
{
	if (G_LIKELY (passwords))
	{
		const gchar *accountid = tny_account_get_id (account);

		gchar *pwd = g_hash_table_lookup (passwords, accountid);

		if (G_LIKELY (pwd))
		{
			memset (pwd, 0, strlen (pwd));
			/* g_free (pwd); uhm, crashed once */
			g_hash_table_remove (passwords, accountid);
		}

	}

	return;
}

#endif


static gboolean
tny_gnome_account_store_alert (TnyAccountStore *self, TnyAlertType type, const gchar *prompt)
{
	GtkMessageType gtktype;
	gboolean retval = FALSE;
	GtkWidget *dialog;

	switch (type)
	{
		case TNY_ALERT_TYPE_INFO:
		gtktype = GTK_MESSAGE_INFO;
		break;
		case TNY_ALERT_TYPE_WARNING:
		gtktype = GTK_MESSAGE_WARNING;
		break;
		case TNY_ALERT_TYPE_ERROR:
		default:
		gtktype = GTK_MESSAGE_ERROR;
		break;
	}

	dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
                                  gtktype, GTK_BUTTONS_YES_NO, prompt);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_YES)
		retval = TRUE;

	gtk_widget_destroy (dialog);

	return retval;
}

static void
gconf_listener_account_changed (GConfClient *client, guint cnxn_id,
			GConfEntry *entry, gpointer user_data)
{
	TnyAccountStore *self = user_data;

	gchar *key = g_strdup (entry->key);
	gchar *ptr = strrchr (key, '/'); ptr++;

	if (!strcmp (ptr, "count"))
	{
		g_signal_emit (self, 
			tny_account_store_signals [TNY_ACCOUNT_STORE_ACCOUNTS_RELOADED], 0);

	}

	g_free (key);

	return;
}


static const gchar*
tny_gnome_account_store_get_cache_dir (TnyAccountStore *self)
{
	TnyGnomeAccountStorePriv *priv = TNY_GNOME_ACCOUNT_STORE_GET_PRIVATE (self);

	if (G_UNLIKELY (!priv->cache_dir))
	{
		/* Note that there's no listener for this key. If it changes,
		   the camelsession should be destroyed and rebuild from scratch.
		   Which basically means reloading the accounts aswell. 
		  
		   So say you're a nut who wants this key to be updatable at 
		   runtime, you'll have to unload all the accounts here, and of
		   course reload them. All the functionality for that is already
		   available. Perhaps I should just do it ... hmm, maybe another
		   day. Soon. Perhaps. I don't know. Probably . . . . bleh. 

		   Oh and, not to forget! You should probably also move the old
		   cache location to the new one. Or cleanup the old one. */

		gchar *cache_dir = gconf_client_get_string (priv->client, 
			"/apps/tinymail/cache_dir", NULL);
		priv->cache_dir = g_build_filename (g_get_home_dir (), 
			cache_dir, NULL);
		g_free (cache_dir);
	}

	return priv->cache_dir;
}


static void
tny_gnome_account_store_get_accounts (TnyAccountStore *self, TnyList *list, TnyGetAccountsRequestType types)
{
	TnyGnomeAccountStorePriv *priv = TNY_GNOME_ACCOUNT_STORE_GET_PRIVATE (self);
	gint i=0, count;

	count = gconf_client_get_int (priv->client, 
			"/apps/tinymail/accounts/count", NULL);

	for (i=0; i < count; i++)
	{
		gchar *proto, *type, *key, *name;
		TnyAccount *account = NULL;
		GSList *options;

		key = g_strdup_printf ("/apps/tinymail/accounts/%d", i);
		
		if (!gconf_client_dir_exists (priv->client, (const gchar*)key, NULL))
		{
			g_free (key);
			continue;
		}
		g_free (key);

	    
		key = g_strdup_printf ("/apps/tinymail/accounts/%d/type", i);
		type = gconf_client_get_string (priv->client, 
			(const gchar*) key, NULL);
		g_free (key);
	    
		key = g_strdup_printf ("/apps/tinymail/accounts/%d/proto", i);
		proto = gconf_client_get_string (priv->client, 
			(const gchar*) key, NULL);
		g_free (key);
	    
		if (type && G_LIKELY (!g_ascii_strncasecmp (type, "transport", 9)))
		{
			if (types == TNY_ACCOUNT_STORE_BOTH || types == TNY_ACCOUNT_STORE_TRANSPORT_ACCOUNTS)
				account = TNY_ACCOUNT (tny_camel_transport_account_new ());
		} else if (type && (types == TNY_ACCOUNT_STORE_BOTH || types == TNY_ACCOUNT_STORE_STORE_ACCOUNTS))
		{		
			if (!g_ascii_strncasecmp (proto, "imap", 4))
				account = TNY_ACCOUNT (tny_camel_imap_store_account_new ());
			else if (!g_ascii_strncasecmp (proto, "nntp", 4))
				account = TNY_ACCOUNT (tny_camel_nntp_store_account_new ());
			else if (!g_ascii_strncasecmp (proto, "pop", 4))
				account = TNY_ACCOUNT (tny_camel_pop_store_account_new ());
			else	/* Unknown, create a generic one? */
			        account = TNY_ACCOUNT (tny_camel_store_account_new ());
		}

		if (type)
			g_free (type);

		if (account)
		{
			tny_camel_account_set_session (TNY_CAMEL_ACCOUNT (account), priv->session);
			tny_account_set_proto (TNY_ACCOUNT (account), proto);

			key = g_strdup_printf ("/apps/tinymail/accounts/%d/name", i);
			name = gconf_client_get_string (priv->client, 
				(const gchar*) key, NULL);
			g_free (key);
			tny_account_set_name (TNY_ACCOUNT (account), name);
			g_free (name);


			key = g_strdup_printf ("/apps/tinymail/accounts/%d/options", i);
			options = gconf_client_get_list (priv->client, 
				(const gchar*) key, GCONF_VALUE_STRING, NULL);
			g_free (key);

			if (options)
			{
				while (options)
				{
					tny_camel_account_add_option (TNY_CAMEL_ACCOUNT (account), options->data);
					g_free (options->data);
					options = g_slist_next (options);
				}
				g_slist_free (options);
			}

			/* Because we only check for the n first bytes, the pops, imaps and smtps also work */
			if (!g_ascii_strncasecmp (proto, "pop", 3) ||
				!g_ascii_strncasecmp (proto, "imap", 4)||
				!g_ascii_strncasecmp (proto, "smtp", 4))
			{
				gchar *user, *hostname;

				/* TODO: Add other supported and tested providers here */
				key = g_strdup_printf ("/apps/tinymail/accounts/%d/user", i);
				user = gconf_client_get_string (priv->client, 
					(const gchar*) key, NULL);

				g_free (key);
				tny_account_set_user (TNY_ACCOUNT (account), user);

				key = g_strdup_printf ("/apps/tinymail/accounts/%d/hostname", i);
				hostname = gconf_client_get_string (priv->client, 
					(const gchar*) key, NULL);
				g_free (key); 
				tny_account_set_hostname (TNY_ACCOUNT (account), 
					hostname);
				
				g_free (hostname); g_free (user);
			} else {
				gchar *url_string;

				/* Un officially supported provider */
				/* Assuming there's a url_string in this case */

				key = g_strdup_printf ("/apps/tinymail/accounts/%d/url_string", i);
				url_string = gconf_client_get_string (priv->client, 
					(const gchar*) key, NULL);

				g_free (key);
				tny_account_set_url_string (TNY_ACCOUNT (account), url_string);
				g_free (url_string);
			}

			key = g_strdup_printf ("/apps/tinymail/accounts/%d", i);
			tny_account_set_id (TNY_ACCOUNT (account), key);
			g_free (key);

			/* 
			 * Setting the password function must happen after
			 * setting the host, user and protocol.
			 */

			tny_account_set_forget_pass_func (TNY_ACCOUNT (account),
				per_account_forget_pass_func);

			tny_account_set_pass_func (TNY_ACCOUNT (account),
				per_account_get_pass_func);
		    
			tny_list_prepend (list, (GObject*)account);
			g_object_unref (G_OBJECT (account));

		}

		if (proto)
			g_free (proto);
	}

	return;	
}


static void
tny_gnome_account_store_notify_add (TnyAccountStore *self)
{
	TnyGnomeAccountStorePriv *priv = TNY_GNOME_ACCOUNT_STORE_GET_PRIVATE (self);
	priv->notify = gconf_client_notify_add (priv->client, 
		"/apps/tinymail/accounts", gconf_listener_account_changed,
		self, NULL, NULL);
	return;
}

static void
tny_gnome_account_store_notify_remove (TnyAccountStore *self)
{
	TnyGnomeAccountStorePriv *priv = TNY_GNOME_ACCOUNT_STORE_GET_PRIVATE (self);
	gconf_client_notify_remove (priv->client, priv->notify);
	return;
}

/*
	gconftool-2 -s /apps/tinymail/cache_dir -t string .tinymail

	gconftool-2 -s /apps/tinymail/accounts/count -t int COUNT
	gconftool-2 -s /apps/tinymail/accounts/0/proto -t string [smtp|imap|pop]
	gconftool-2 -s /apps/tinymail/accounts/0/type -t string [transport|store]

	gconftool-2 -s /apps/tinymail/accounts/0/user -t string username
	gconftool-2 -s /apps/tinymail/accounts/0/hostname -t string mailserver
or
	gconftool-2 -s /apps/tinymail/accounts/0/url_string -t string url_string

*/

static void
tny_gnome_account_store_add_account (TnyAccountStore *self, TnyAccount *account, const gchar *type)
{
	TnyGnomeAccountStorePriv *priv = TNY_GNOME_ACCOUNT_STORE_GET_PRIVATE (self);
	gchar *key = NULL;
	gint count = gconf_client_get_int (priv->client, "/apps/tinymail/accounts/count", NULL);

	key = g_strdup_printf ("/apps/tinymail/accounts/%d/hostname", count);
	gconf_client_set_string (priv->client, (const gchar*) key, 
		tny_account_get_hostname (account), NULL);
	g_free (key); 

	key = g_strdup_printf ("/apps/tinymail/accounts/%d/proto", count);
	gconf_client_set_string (priv->client, (const gchar*) key, 
		tny_account_get_proto (account), NULL);
	g_free (key); 

	key = g_strdup_printf ("/apps/tinymail/accounts/%d/type", count);
	gconf_client_set_string (priv->client, (const gchar*) key, type, NULL);
	g_free (key); 

	key = g_strdup_printf ("/apps/tinymail/accounts/%d/user", count);
	gconf_client_set_string (priv->client, (const gchar*) key, 
		tny_account_get_user (account), NULL);
	g_free (key); 

    	count++;

	gconf_client_set_int (priv->client, "/apps/tinymail/accounts/count", 
		count, NULL);

	return;
}



static void
tny_gnome_account_store_add_store_account (TnyAccountStore *self, TnyStoreAccount *account)
{
	tny_gnome_account_store_notify_remove (self);
	tny_gnome_account_store_add_account (self, TNY_ACCOUNT (account), "store");
	tny_gnome_account_store_notify_add (self);

	g_signal_emit (self, tny_account_store_signals [TNY_ACCOUNT_STORE_ACCOUNT_INSERTED], 0, account);

	return;
}

static void
tny_gnome_account_store_add_transport_account (TnyAccountStore *self, TnyTransportAccount *account)
{
	tny_gnome_account_store_notify_remove (self);
	tny_gnome_account_store_add_account (self, TNY_ACCOUNT (account), "transport");
	tny_gnome_account_store_notify_add (self);

	g_signal_emit (self, tny_account_store_signals [TNY_ACCOUNT_STORE_ACCOUNT_INSERTED], 0, account);

	return;
}

static TnyDevice*
tny_gnome_account_store_get_device (TnyAccountStore *self)
{
	TnyGnomeAccountStorePriv *priv = TNY_GNOME_ACCOUNT_STORE_GET_PRIVATE (self);

	return g_object_ref (G_OBJECT (priv->device));
}

/**
 * tny_gnome_account_store_new:
 *
 *
 * Return value: A new #TnyAccountStore instance
 **/
TnyAccountStore*
tny_gnome_account_store_new (void)
{
	TnyGnomeAccountStore *self = g_object_new (TNY_TYPE_GNOME_ACCOUNT_STORE, NULL);
	TnyGnomeAccountStorePriv *priv = TNY_GNOME_ACCOUNT_STORE_GET_PRIVATE (self);
	priv->session = tny_session_camel_new (TNY_ACCOUNT_STORE (self));

	return TNY_ACCOUNT_STORE (self);
}


static void
tny_gnome_account_store_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyGnomeAccountStore *self = (TnyGnomeAccountStore *)instance;
	TnyGnomeAccountStorePriv *priv = TNY_GNOME_ACCOUNT_STORE_GET_PRIVATE (self);
	TnyPlatformFactory *platfact;

	priv->client = gconf_client_get_default ();

	gconf_client_add_dir (priv->client, "/apps/tinymail", 
		GCONF_CLIENT_PRELOAD_RECURSIVE, NULL);

	tny_gnome_account_store_notify_add (TNY_ACCOUNT_STORE (self));

	platfact = TNY_PLATFORM_FACTORY (tny_gnome_platform_factory_get_instance ());
    	priv->device = tny_platform_factory_new_device (platfact);
	/* tny_device_force_online (priv->device); */
	

	return;
}


static void
tny_gnome_account_store_finalize (GObject *object)
{
	TnyGnomeAccountStore *self = (TnyGnomeAccountStore *)object;	
	TnyGnomeAccountStorePriv *priv = TNY_GNOME_ACCOUNT_STORE_GET_PRIVATE (self);

	tny_gnome_account_store_notify_remove (TNY_ACCOUNT_STORE (self));
	g_object_unref (G_OBJECT (priv->client));

	if (G_LIKELY (priv->cache_dir))
		g_free (priv->cache_dir);

	(*parent_class->finalize) (object);

	return;
}


/**
 * tny_gnome_account_store_get_session:
 * @self: The #TnyGnomeAccountStore instance
 *
 * Return value: A #TnySessionCamel instance
 **/
TnySessionCamel*
tny_gnome_account_store_get_session (TnyGnomeAccountStore *self)
{
	TnyGnomeAccountStorePriv *priv = TNY_GNOME_ACCOUNT_STORE_GET_PRIVATE (self);
    
	return priv->session;
}

static void 
tny_gnome_account_store_class_init (TnyGnomeAccountStoreClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_gnome_account_store_finalize;

	g_type_class_add_private (object_class, sizeof (TnyGnomeAccountStorePriv));

	return;
}

static void
tny_account_store_init (gpointer g, gpointer iface_data)
{
	TnyAccountStoreIface *klass = (TnyAccountStoreIface *)g;

	klass->get_accounts_func = tny_gnome_account_store_get_accounts;
	klass->add_store_account_func = tny_gnome_account_store_add_store_account;
	klass->add_transport_account_func = tny_gnome_account_store_add_transport_account;
	klass->get_cache_dir_func = tny_gnome_account_store_get_cache_dir;
	klass->get_device_func = tny_gnome_account_store_get_device;
	klass->alert_func = tny_gnome_account_store_alert;

	return;
}


GType 
tny_gnome_account_store_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyGnomeAccountStoreClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_gnome_account_store_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyGnomeAccountStore),
		  0,      /* n_preallocs */
		  tny_gnome_account_store_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_account_store_info = 
		{
		  (GInterfaceInitFunc) tny_account_store_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyGnomeAccountStore",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_ACCOUNT_STORE, 
			&tny_account_store_info);
	}

	return type;
}

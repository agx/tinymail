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

#include <tny-platform-factory.h>
#include <tny-olpc-platform-factory.h>

#include <tny-account-store.h>
#include <tny-olpc-account-store.h>
#include <tny-olpc-password-dialog.h>
#include <tny-account.h>
#include <tny-store-account.h>
#include <tny-transport-account.h>
#include <tny-device.h>

#include <tny-camel-account.h>
#include <tny-camel-store-account.h>
#include <tny-camel-transport-account.h>
#include <tny-session-camel.h>
#include <tny-olpc-device.h>

/* GKeyFile vs. Camel implementation */

static GObjectClass *parent_class = NULL;

typedef struct _TnyOlpcAccountStorePriv TnyOlpcAccountStorePriv;

struct _TnyOlpcAccountStorePriv
{
	gchar *cache_dir;
	TnySessionCamel *session;
	TnyDevice *device;
	guint notify;
};

#define TNY_OLPC_ACCOUNT_STORE_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_OLPC_ACCOUNT_STORE, TnyOlpcAccountStorePriv))


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
		GtkDialog *dialog = GTK_DIALOG (tny_olpc_password_dialog_new ());
	
		tny_olpc_password_dialog_set_prompt (TNY_OLPC_PASSWORD_DIALOG (dialog), prompt);

		if (G_LIKELY (gtk_dialog_run (dialog) == GTK_RESPONSE_OK))
		{
			const gchar *pwd = tny_olpc_password_dialog_get_password 
				(TNY_OLPC_PASSWORD_DIALOG (dialog));
	
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
	TnyGetPassFunc func;
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

static gboolean
tny_olpc_account_store_alert (TnyAccountStore *self, TnyAlertType type, const gchar *prompt)
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


static const gchar*
tny_olpc_account_store_get_cache_dir (TnyAccountStore *self)
{
	TnyOlpcAccountStorePriv *priv = TNY_OLPC_ACCOUNT_STORE_GET_PRIVATE (self);

	if (!priv->cache_dir)
		priv->cache_dir = g_build_path (G_DIR_SEPARATOR_S, g_get_home_dir(), ".tinymail", NULL);

	return priv->cache_dir;
}


static void
tny_olpc_account_store_get_accounts (TnyAccountStore *self, TnyList *list, TnyGetAccountsRequestType types)
{
	TnyOlpcAccountStorePriv *priv = TNY_OLPC_ACCOUNT_STORE_GET_PRIVATE (self);
	const gchar *filen;
	gchar *configd;
	GDir *dir ;

	configd = g_build_path (G_DIR_SEPARATOR_S, g_get_home_dir(), 
		".tinymail", "accounts", NULL);
	dir = g_dir_open (configd, 0, NULL);
	g_free (configd);

	if (!dir)
		return;

	for (filen = g_dir_read_name (dir); filen; filen = g_dir_read_name (dir))
	{
		GKeyFile *keyfile;
		gchar *proto, *type, *key, *name;
		TnyAccount *account = NULL;
		gchar *fullfilen = g_build_filename (g_get_home_dir(), 
			".tinymail", "accounts", filen);
		keyfile = g_key_file_new ();

		if (!g_key_file_load_from_file (keyfile, fullfilen, G_KEY_FILE_NONE, NULL))
		{
			g_free (fullfilen);
			continue;
		}

		type = g_key_file_get_value (keyfile, "tinymail", "type", NULL);
		proto = g_key_file_get_value (keyfile, "tinymail", "proto", NULL);
	    
		if (type && G_LIKELY (!g_ascii_strncasecmp (type, "transport", 9)))
		{
			if (types == TNY_ACCOUNT_STORE_BOTH || types == TNY_ACCOUNT_STORE_TRANSPORT_ACCOUNTS)
				account = TNY_ACCOUNT (tny_camel_transport_account_new ());
		} else if (type && types == TNY_ACCOUNT_STORE_BOTH || types == TNY_ACCOUNT_STORE_STORE_ACCOUNTS)
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
			gsize options_len; gint i;
			gchar **options;

			tny_camel_account_set_session (TNY_CAMEL_ACCOUNT (account), priv->session);
			tny_account_set_proto (TNY_ACCOUNT (account), proto);

			name = g_key_file_get_value (keyfile, "tinymail", "name", NULL);
			tny_account_set_name (TNY_ACCOUNT (account), name);
			g_free (name);

			options = g_key_file_get_string_list (keyfile, "tinymail", "options", &options_len, NULL);
			if (options)
			{
				for (i=0; i<options_len; i++)
					tny_camel_account_add_option (TNY_CAMEL_ACCOUNT (account), options[i]);
				g_strfreev (options);
			}

			/* Because we only check for the n first bytes, the pops, imaps and smtps also work */
			if (!g_ascii_strncasecmp (proto, "pop", 3) ||
				!g_ascii_strncasecmp (proto, "imap", 4)||
				!g_ascii_strncasecmp (proto, "smtp", 4))
			{
				gchar *user, *hostname;

				/* TODO: Add other supported and tested providers here */
				user = g_key_file_get_value (keyfile, "tinymail", "user", NULL);
				tny_account_set_user (TNY_ACCOUNT (account), user);

				hostname = g_key_file_get_value (keyfile, "tinymail", "hostname", NULL);
				tny_account_set_hostname (TNY_ACCOUNT (account), hostname);
				
				g_free (hostname); g_free (user);
			} else {
				gchar *url_string;

				/* Un officially supported provider */
				/* Assuming there's a url_string in this case */
				url_string = g_key_file_get_value (keyfile, "tinymail", "url_string", NULL);
				tny_account_set_url_string (TNY_ACCOUNT (account), url_string);
				g_free (url_string);
			}

			tny_account_set_id (TNY_ACCOUNT (account), fullfilen);

			g_free (fullfilen);

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

		g_key_file_free (keyfile);
	}	
	g_dir_close (dir);


	tny_session_camel_set_current_accounts (priv->session, list);

	return;	
}



static void
tny_olpc_account_store_add_account (TnyAccountStore *self, TnyAccount *account, const gchar *type)
{
	g_warning ("Not implemented\n");

	return;
}



static void
tny_olpc_account_store_add_store_account (TnyAccountStore *self, TnyStoreAccount *account)
{
	TnyOlpcAccountStorePriv *priv = TNY_OLPC_ACCOUNT_STORE_GET_PRIVATE (self);

	tny_olpc_account_store_add_account (self, TNY_ACCOUNT (account), "store");

	g_signal_emit (self, tny_account_store_signals [TNY_ACCOUNT_STORE_ACCOUNT_INSERTED], 0, account);

	return;
}

static void
tny_olpc_account_store_add_transport_account (TnyAccountStore *self, TnyTransportAccount *account)
{
	TnyOlpcAccountStorePriv *priv = TNY_OLPC_ACCOUNT_STORE_GET_PRIVATE (self);

	tny_olpc_account_store_add_account (self, TNY_ACCOUNT (account), "transport");

	g_signal_emit (self, tny_account_store_signals [TNY_ACCOUNT_STORE_ACCOUNT_INSERTED], 0, account);

	return;
}

static TnyDevice*
tny_olpc_account_store_get_device (TnyAccountStore *self)
{
	TnyOlpcAccountStorePriv *priv = TNY_OLPC_ACCOUNT_STORE_GET_PRIVATE (self);

	return priv->device;
}

/**
 * tny_olpc_account_store_new:
 *
 *
 * Return value: A new #TnyAccountStore implemented for OLPC
 **/
TnyAccountStore*
tny_olpc_account_store_new (void)
{
	TnyOlpcAccountStore *self = g_object_new (TNY_TYPE_OLPC_ACCOUNT_STORE, NULL);
	TnyOlpcAccountStorePriv *priv = TNY_OLPC_ACCOUNT_STORE_GET_PRIVATE (self);
	priv->session = tny_session_camel_new (TNY_ACCOUNT_STORE (self));

	return TNY_ACCOUNT_STORE (self);
}


static void
tny_olpc_account_store_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyOlpcAccountStorePriv *priv = TNY_OLPC_ACCOUNT_STORE_GET_PRIVATE (instance);
	TnyPlatformFactory *platfact = TNY_PLATFORM_FACTORY (
		tny_olpc_platform_factory_get_instance ());

	priv->device = tny_platform_factory_new_device (platfact);
	/* tny_device_force_online (priv->device); */
	
	return;
}


static void
tny_olpc_account_store_finalize (GObject *object)
{	
	TnyOlpcAccountStorePriv *priv = TNY_OLPC_ACCOUNT_STORE_GET_PRIVATE (object);

	if (priv->cache_dir)
		g_free (priv->cache_dir);

	(*parent_class->finalize) (object);

	return;
}


/**
 * tny_olpc_account_store_get_session:
 * @self: The #TnyOlpcAccountStore instance
 *
 * Return value: A #TnySessionCamel instance
 **/
TnySessionCamel*
tny_olpc_account_store_get_session (TnyOlpcAccountStore *self)
{
	TnyOlpcAccountStorePriv *priv = TNY_OLPC_ACCOUNT_STORE_GET_PRIVATE (self);

	return priv->session;
}

static void 
tny_olpc_account_store_class_init (TnyOlpcAccountStoreClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_olpc_account_store_finalize;

	g_type_class_add_private (object_class, sizeof (TnyOlpcAccountStorePriv));

	return;
}

static void
tny_account_store_init (gpointer g, gpointer iface_data)
{
	TnyAccountStoreClass *klass = (TnyAccountStoreClass *)g;

	klass->get_accounts_func = tny_olpc_account_store_get_accounts;
	klass->add_store_account_func = tny_olpc_account_store_add_store_account;
	klass->add_transport_account_func = tny_olpc_account_store_add_transport_account;
	klass->get_cache_dir_func = tny_olpc_account_store_get_cache_dir;
	klass->get_device_func = tny_olpc_account_store_get_device;
	klass->alert_func = tny_olpc_account_store_alert;

	return;
}


GType 
tny_olpc_account_store_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyOlpcAccountStoreClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_olpc_account_store_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyOlpcAccountStore),
		  0,      /* n_preallocs */
		  tny_olpc_account_store_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_account_store_info = 
		{
		  (GInterfaceInitFunc) tny_account_store_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyOlpcAccountStore",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_ACCOUNT_STORE, 
			&tny_account_store_info);
	}

	return type;
}

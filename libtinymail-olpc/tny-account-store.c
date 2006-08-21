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

#include <tny-platform-factory-iface.h>
#include <tny-platform-factory.h>

#include <tny-account-store-iface.h>
#include <tny-account-store.h>
#include <tny-password-dialog.h>

#include <tny-account-iface.h>
#include <tny-store-account-iface.h>
#include <tny-transport-account-iface.h>

#include <tny-store-account.h>
#include <tny-transport-account.h>

#include <tny-device-iface.h>
#include <tny-device.h>

/* GKeyFile vs. Camel implementation */

static GObjectClass *parent_class = NULL;

typedef struct _TnyAccountStorePriv TnyAccountStorePriv;

struct _TnyAccountStorePriv
{
	gchar *cache_dir;
	TnySessionCamel *session;
	TnyDeviceIface *device;
	guint notify;
};

#define TNY_ACCOUNT_STORE_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_ACCOUNT_STORE, TnyAccountStorePriv))


static GHashTable *passwords;

static gchar* 
per_account_get_pass_func (TnyAccountIface *account, const gchar *prompt, gboolean *cancel)
{
	gchar *retval = NULL;
	const gchar *accountid = tny_account_iface_get_id (account);

	if (G_UNLIKELY (!passwords))
		passwords = g_hash_table_new (g_str_hash, g_str_equal);

	retval = g_hash_table_lookup (passwords, accountid);

	if (G_UNLIKELY (!retval))
	{
		GtkDialog *dialog = GTK_DIALOG (tny_password_dialog_new ());
	
		tny_password_dialog_set_prompt (TNY_PASSWORD_DIALOG (dialog), prompt);

		if (G_LIKELY (gtk_dialog_run (dialog) == GTK_RESPONSE_OK))
		{
			const gchar *pwd = tny_password_dialog_get_password 
				(TNY_PASSWORD_DIALOG (dialog));
	
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
per_account_forget_pass_func (TnyAccountIface *account)
{
	const TnyAccountStoreIface *self = tny_account_iface_get_account_store (account);
	TnyAccountStorePriv *priv = TNY_ACCOUNT_STORE_GET_PRIVATE (self);
	TnyGetPassFunc func;
	if (G_LIKELY (passwords))
	{
		const gchar *accountid = tny_account_iface_get_id (account);

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
tny_account_store_alert (TnyAccountStoreIface *self, TnyAlertType type, const gchar *prompt)
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
tny_account_store_get_cache_dir (TnyAccountStoreIface *self)
{
	TnyAccountStorePriv *priv = TNY_ACCOUNT_STORE_GET_PRIVATE (self);

	if (!priv->cache_dir)
		priv->cache_dir = g_build_path (G_DIR_SEPARATOR_S, g_get_home_dir(), ".tinymail", NULL);

	return priv->cache_dir;
}


static void
tny_account_store_get_accounts (TnyAccountStoreIface *self, TnyListIface *list, TnyGetAccountsRequestType types)
{
	TnyAccountStorePriv *priv = TNY_ACCOUNT_STORE_GET_PRIVATE (self);
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
		TnyAccountIface *account = NULL;
		gchar *fullfilen = g_build_filename (g_get_home_dir(), 
			".tinymail", "accounts", filen);
		keyfile = g_key_file_new ();

		if (!g_key_file_load_from_file (keyfile, fullfilen, G_KEY_FILE_NONE, NULL))
		{
			g_free (fullfilen);
			continue;
		}

		type = g_key_file_get_value (keyfile, "tinymail", "type", NULL);

		if (type && G_LIKELY (!g_ascii_strncasecmp (type, "transport", 9)))
		{
			if (types == TNY_ACCOUNT_STORE_IFACE_BOTH || 
			    types == TNY_ACCOUNT_STORE_IFACE_TRANSPORT_ACCOUNTS)
			{
				account = TNY_ACCOUNT_IFACE (tny_transport_account_new ());
			}
	
		} else 
		{

			if (types == TNY_ACCOUNT_STORE_IFACE_BOTH || 
			    types == TNY_ACCOUNT_STORE_IFACE_STORE_ACCOUNTS)
			{
				account = TNY_ACCOUNT_IFACE (tny_store_account_new ());
			}
		}

		if (type)
			g_free (type);

		if (account)
		{
			gsize options_len; gint i;
			gchar **options;

			tny_account_iface_set_account_store (account, self);

			proto = g_key_file_get_value (keyfile, "tinymail", "proto", NULL);
			tny_account_iface_set_proto (TNY_ACCOUNT_IFACE (account), proto);

			name = g_key_file_get_value (keyfile, "tinymail", "name", NULL);
			tny_account_iface_set_name (TNY_ACCOUNT_IFACE (account), name);
			g_free (name);

			options = g_key_file_get_string_list (keyfile, "tinymail", "options", &options_len, NULL);
			if (options)
			{
				for (i=0; i<options_len; i++)
					tny_account_add_option (TNY_ACCOUNT (account), options[i]);
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
				tny_account_iface_set_user (TNY_ACCOUNT_IFACE (account), user);

				hostname = g_key_file_get_value (keyfile, "tinymail", "hostname", NULL);
				tny_account_iface_set_hostname (TNY_ACCOUNT_IFACE (account), hostname);
				
				g_free (hostname); g_free (user);
			} else {
				gchar *url_string;

				/* Un officially supported provider */
				/* Assuming there's a url_string in this case */
				url_string = g_key_file_get_value (keyfile, "tinymail", "url_string", NULL);
				tny_account_iface_set_url_string (TNY_ACCOUNT_IFACE (account), url_string);
				g_free (url_string);
			}

			tny_account_iface_set_id (TNY_ACCOUNT_IFACE (account), fullfilen);

			g_free (fullfilen);

			/* 
			 * Setting the password function must happen after
			 * setting the host, user and protocol.
			 */

			tny_account_iface_set_forget_pass_func (TNY_ACCOUNT_IFACE (account),
				per_account_forget_pass_func);
	
			tny_account_iface_set_pass_func (TNY_ACCOUNT_IFACE (account),
				per_account_get_pass_func);

			tny_list_iface_prepend (list, (GObject*)account);
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
tny_account_store_add_account (TnyAccountStoreIface *self, TnyAccountIface *account, const gchar *type)
{
	g_warning ("Not implemented\n");

	return;
}



static void
tny_account_store_add_store_account (TnyAccountStoreIface *self, TnyStoreAccountIface *account)
{
	TnyAccountStorePriv *priv = TNY_ACCOUNT_STORE_GET_PRIVATE (self);

	tny_account_store_add_account (self, TNY_ACCOUNT_IFACE (account), "store");

	g_signal_emit (self, tny_account_store_iface_signals [TNY_ACCOUNT_STORE_IFACE_ACCOUNT_INSERTED], 0, account);

	return;
}

static void
tny_account_store_add_transport_account (TnyAccountStoreIface *self, TnyTransportAccountIface *account)
{
	TnyAccountStorePriv *priv = TNY_ACCOUNT_STORE_GET_PRIVATE (self);

	tny_account_store_add_account (self, TNY_ACCOUNT_IFACE (account), "transport");

	g_signal_emit (self, tny_account_store_iface_signals [TNY_ACCOUNT_STORE_IFACE_ACCOUNT_INSERTED], 0, account);

	return;
}

static TnyDeviceIface*
tny_account_store_get_device (TnyAccountStoreIface *self)
{
	TnyAccountStorePriv *priv = TNY_ACCOUNT_STORE_GET_PRIVATE (self);

	return priv->device;
}

/**
 * tny_account_store_new:
 *
 *
 * Return value: A new #TnyAccountStoreIface instance
 **/
TnyAccountStore*
tny_account_store_new (void)
{
	TnyAccountStore *self = g_object_new (TNY_TYPE_ACCOUNT_STORE, NULL);
	
	return self;
}


static void
tny_account_store_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyAccountStorePriv *priv = TNY_ACCOUNT_STORE_GET_PRIVATE (instance);
	TnyPlatformFactoryIface *platfact = TNY_PLATFORM_FACTORY_IFACE (
		tny_platform_factory_get_instance ());

	priv->device = tny_platform_factory_iface_new_device (platfact);
	/* tny_device_iface_force_online (priv->device); */
	priv->session = tny_session_camel_new (TNY_ACCOUNT_STORE_IFACE (self));
    
	return;
}


static void
tny_account_store_finalize (GObject *object)
{	
	TnyAccountStorePriv *priv = TNY_ACCOUNT_STORE_GET_PRIVATE (object);

	if (priv->cache_dir)
		g_free (priv->cache_dir);

	(*parent_class->finalize) (object);

	return;
}


/**
 * tny_account_store_get_session:
 * @self: The #TnyAccountStore instance
 *
 * Return value: A #TnySessionCamel instance
 **/
TnySessionCamel*
tny_account_store_get_session (TnyAccountStore *self)
{
	TnyAccountStorePriv *priv = TNY_ACCOUNT_STORE_GET_PRIVATE (self);

	return priv->session;
}

static void 
tny_account_store_class_init (TnyAccountStoreClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_account_store_finalize;

	g_type_class_add_private (object_class, sizeof (TnyAccountStorePriv));

	return;
}

static void
tny_account_store_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyAccountStoreIfaceClass *klass = (TnyAccountStoreIfaceClass *)g_iface;

	klass->get_accounts_func = tny_account_store_get_accounts;
	klass->add_store_account_func = tny_account_store_add_store_account;
	klass->add_transport_account_func = tny_account_store_add_transport_account;
	klass->get_cache_dir_func = tny_account_store_get_cache_dir;
	klass->get_device_func = tny_account_store_get_device;
	klass->alert_func = tny_account_store_alert;

	return;
}


GType 
tny_account_store_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyAccountStoreClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_account_store_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyAccountStore),
		  0,      /* n_preallocs */
		  tny_account_store_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_account_store_iface_info = 
		{
		  (GInterfaceInitFunc) tny_account_store_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyAccountStore",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_ACCOUNT_STORE_IFACE, 
			&tny_account_store_iface_info);
	}

	return type;
}

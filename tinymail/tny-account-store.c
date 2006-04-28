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

/* TODO: Refactor this type to a libtinymail-emerge */

#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>

#include <tny-account-store-iface.h>
#include <tny-account-store.h>
#include <tny-password-dialog.h>

#include <tny-account-iface.h>
#include <tny-store-account-iface.h>
#include <tny-transport-account-iface.h>

#include <tny-store-account.h>
#include <tny-transport-account.h>

/* "GConf vs. Camel" account implementation */

static GObjectClass *parent_class = NULL;

typedef struct _TnyAccountStorePriv TnyAccountStorePriv;

struct _TnyAccountStorePriv
{
	GConfClient *client;

	gchar *cache_dir;

	GMutex *store_accounts_lock;
	GList *store_accounts;

	GMutex *transport_accounts_lock;
	GList *transport_accounts;

	guint notify;
};

#define TNY_ACCOUNT_STORE_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_ACCOUNT_STORE, TnyAccountStorePriv))

static GHashTable *passwords;


static const GList* tny_account_store_get_accounts (TnyAccountStoreIface *self);

static void
destroy_account (gpointer data, gpointer user_data)
{
	g_object_unref (G_OBJECT (data));

	return;
}

static void
destroy_these_accounts (GList *accounts)
{
	g_list_foreach (accounts, destroy_account, NULL);
	g_list_free (accounts);
	accounts = NULL;

	return;
}

static void
destroy_current_accounts (TnyAccountStorePriv *priv)
{
	g_mutex_lock (priv->store_accounts_lock);
	g_mutex_lock (priv->transport_accounts_lock);

	if (priv->store_accounts)
		destroy_these_accounts (priv->store_accounts);

	if (priv->transport_accounts)
		destroy_these_accounts (priv->transport_accounts);

	priv->transport_accounts = NULL;
	priv->store_accounts = NULL;

	g_mutex_unlock (priv->transport_accounts_lock);
	g_mutex_unlock (priv->store_accounts_lock);

	return;
}

static gchar* 
per_account_get_pass_func (TnyAccountIface *account, const gchar *prompt)
{
	gchar *retval = NULL;
	const gchar *accountid = tny_account_iface_get_id (account);

	if (!passwords)
		passwords = g_hash_table_new (g_str_hash, g_str_equal);

	retval = g_hash_table_lookup (passwords, accountid);

	if (!retval)
	{
		GtkDialog *dialog = GTK_DIALOG (tny_password_dialog_new ());
	
		tny_password_dialog_set_prompt (TNY_PASSWORD_DIALOG (dialog),
			prompt);

		if (gtk_dialog_run (dialog) == GTK_RESPONSE_OK)
		{
			const gchar *pwd = tny_password_dialog_get_password 
				(TNY_PASSWORD_DIALOG (dialog));
	
			retval = g_strdup (pwd);

			g_hash_table_insert (passwords, g_strdup (accountid), 
				retval);
		}

		gtk_widget_destroy (GTK_WIDGET (dialog));
	}

	return retval;
}

static void
per_account_forget_pass_func (TnyAccountIface *account)
{
	if (passwords)
	{
		const gchar *accountid = tny_account_iface_get_id (account);

		gchar *pwd = g_hash_table_lookup (passwords, accountid);

		if (pwd) 
		{
			memset (pwd, 0, strlen (pwd));
			g_free (pwd);
			g_hash_table_remove (passwords, accountid);
		}
	}

	return;
}

static TnyAccountIface *
find_account_by_gconf_key (GList *accounts, const gchar *key)
{
	TnyAccountIface *found = NULL;

	while (accounts)
	{
		TnyAccountIface *account = accounts->data;
		const gchar *aid = tny_account_iface_get_id (account);
		
		if (strcmp (key, aid)==0)
		{
			found = account;
			break;
		}
		accounts = g_list_next (accounts);
	}

	return found;
}

static void
gconf_listener_account_changed (GConfClient *client, guint cnxn_id,
			GConfEntry *entry, gpointer user_data)
{
	TnyAccountStoreIface *self = user_data;
	TnyAccountStorePriv *priv = TNY_ACCOUNT_STORE_GET_PRIVATE (self);

	gchar *key = g_strdup (entry->key);
	gchar *ptr = strrchr (key, '/'); ptr++;

	if (!strcmp (ptr, "count"))
	{
		destroy_current_accounts (priv);

		g_signal_emit (self, 
			tny_account_store_iface_signals [ACCOUNTS_RELOADED], 0);

	} else {
		GList *accounts;
		TnyAccountIface *found = NULL;
		const gchar *val;

		/* Whooo, crazy pointer hocus! */
		gchar orig = *ptr--; *ptr = '\0';

		g_mutex_lock (priv->transport_accounts_lock);
		accounts = priv->transport_accounts;
		found = find_account_by_gconf_key (accounts, key);
		g_mutex_unlock (priv->transport_accounts_lock);

		g_mutex_lock (priv->store_accounts_lock);
		accounts = priv->store_accounts;
		if (!found) 
			found = find_account_by_gconf_key (accounts, key);
		g_mutex_unlock (priv->store_accounts_lock);

		/* pocus! */
		*ptr = orig; *ptr++;

		val = gconf_value_get_string (entry->value);

		/* phoef! */
		if (found && strcmp (ptr, "user")==0)
			tny_account_iface_set_user (found, val);
		else if (found && strcmp (ptr, "proto")==0)
			tny_account_iface_set_proto (found, val);
		else if (found && strcmp (ptr, "hostname")==0)
			tny_account_iface_set_hostname (found, val);
	}

	g_free (key);
	return;
}


static const gchar*
tny_account_store_get_cache_dir (TnyAccountStoreIface *self)
{
	TnyAccountStorePriv *priv = TNY_ACCOUNT_STORE_GET_PRIVATE (self);

	if (!priv->cache_dir)
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
tny_account_store_get_all_accounts (TnyAccountStoreIface *self)
{
	TnyAccountStorePriv *priv = TNY_ACCOUNT_STORE_GET_PRIVATE (self);

	gint i=0, count;

	destroy_current_accounts (priv);

	count = gconf_client_get_int (priv->client, 
			"/apps/tinymail/accounts/count", NULL);

	for (i=0; i < count; i++)
	{
		gchar *proto, *user, *hostname, *type, *key;
		TnyAccountIface *account;

		key = g_strdup_printf ("/apps/tinymail/accounts/%d/type", i);
		type = gconf_client_get_string (priv->client, 
			(const gchar*) key, NULL);
		g_free (key);

		if (type && !strcmp (type, "transport"))
		{
			account = TNY_ACCOUNT_IFACE (tny_transport_account_new ());
			priv->transport_accounts = 
				g_list_append (priv->transport_accounts, account);
		} else {
			account = TNY_ACCOUNT_IFACE (tny_store_account_new ());
			priv->store_accounts = 
				g_list_append (priv->store_accounts, account);
		}

		tny_account_iface_set_account_store (account, self);

		if (type)
			g_free (type);

		key = g_strdup_printf ("/apps/tinymail/accounts/%d/proto", i);
		proto = gconf_client_get_string (priv->client, 
			(const gchar*) key, NULL);
		g_free (key);
		tny_account_iface_set_proto (TNY_ACCOUNT_IFACE (account), proto);

		key = g_strdup_printf ("/apps/tinymail/accounts/%d/user", i);
		user = gconf_client_get_string (priv->client, 
			(const gchar*) key, NULL);

		g_free (key);
		tny_account_iface_set_user (TNY_ACCOUNT_IFACE (account), user);

		key = g_strdup_printf ("/apps/tinymail/accounts/%d/hostname", i);
		hostname = gconf_client_get_string (priv->client, 
			(const gchar*) key, NULL);
		g_free (key); 
		tny_account_iface_set_hostname (TNY_ACCOUNT_IFACE (account), 
			hostname);
		
		g_free (hostname); g_free (proto); g_free (user);

		key = g_strdup_printf ("/apps/tinymail/accounts/%d", i);
		tny_account_iface_set_id (TNY_ACCOUNT_IFACE (account), key);
		g_free (key);

		/* 
		 * Setting the password function must happen after
		 * setting the host, user and protocol.
		 */

		tny_account_iface_set_pass_func (TNY_ACCOUNT_IFACE (account),
			per_account_get_pass_func);
	}
}

static const GList*
tny_account_store_get_store_accounts (TnyAccountStoreIface *self)
{
	TnyAccountStorePriv *priv = TNY_ACCOUNT_STORE_GET_PRIVATE (self);
	const GList *retval;

	if (!priv->store_accounts)
		tny_account_store_get_all_accounts (self);

	g_mutex_lock (priv->store_accounts_lock);
	retval = (const GList*) priv->store_accounts;
	g_mutex_unlock (priv->store_accounts_lock);

	return retval;
}


static const GList*
tny_account_store_get_transport_accounts (TnyAccountStoreIface *self)
{
	TnyAccountStorePriv *priv = TNY_ACCOUNT_STORE_GET_PRIVATE (self);
	const GList *retval;

	if (!priv->transport_accounts)
		tny_account_store_get_all_accounts (self);

	g_mutex_lock (priv->transport_accounts_lock);
	retval = (const GList*) priv->transport_accounts;
	g_mutex_unlock (priv->transport_accounts_lock);

	return retval;
}

/*
	gconftool-2 -s /apps/tinymail/cache_dir -t string .tinymail

	gconftool-2 -s /apps/tinymail/accounts/count -t int COUNT
	gconftool-2 -s /apps/tinymail/accounts/0/proto -t string [smtp|imap|pop]
	gconftool-2 -s /apps/tinymail/accounts/0/user -t string username
	gconftool-2 -s /apps/tinymail/accounts/0/hostname -t string mailserver
	gconftool-2 -s /apps/tinymail/accounts/0/type -t string [transport|store]
*/

static void
tny_account_store_add_account (TnyAccountStoreIface *self, TnyAccountIface *account, const gchar *type)
{
	TnyAccountStorePriv *priv = TNY_ACCOUNT_STORE_GET_PRIVATE (self);
	gchar *key = NULL;
	gint count = gconf_client_get_int (priv->client, "/apps/tinymail/accounts/count", NULL);

	count++;

	key = g_strdup_printf ("/apps/tinymail/accounts/%d/hostname", count);
	gconf_client_set_string (priv->client, (const gchar*) key, 
		tny_account_iface_get_hostname (account), NULL);
	g_free (key); 

	key = g_strdup_printf ("/apps/tinymail/accounts/%d/proto", count);
	gconf_client_set_string (priv->client, (const gchar*) key, 
		tny_account_iface_get_proto (account), NULL);
	g_free (key); 

	key = g_strdup_printf ("/apps/tinymail/accounts/%d/type", count);
	gconf_client_set_string (priv->client, (const gchar*) key, type, NULL);
	g_free (key); 

	key = g_strdup_printf ("/apps/tinymail/accounts/%d/user", count);
	gconf_client_set_string (priv->client, (const gchar*) key, 
		tny_account_iface_get_user (account), NULL);
	g_free (key); 

	gconf_client_set_int (priv->client, "/apps/tinymail/accounts/count", 
		count, NULL);

	return;
}

static void
tny_account_store_add_store_account (TnyAccountStoreIface *self, TnyStoreAccountIface *account)
{
	TnyAccountStorePriv *priv = TNY_ACCOUNT_STORE_GET_PRIVATE (self);

	tny_account_store_add_account (self, TNY_ACCOUNT_IFACE (account), "store");
	g_object_ref (G_OBJECT (account));

	g_mutex_lock (priv->store_accounts_lock);
	priv->store_accounts = g_list_append (priv->store_accounts, account);
	g_mutex_unlock (priv->store_accounts_lock);

	g_signal_emit (self, tny_account_store_iface_signals [ACCOUNT_INSERTED], 0, account);

	return;
}

static void
tny_account_store_add_transport_account (TnyAccountStoreIface *self, TnyTransportAccountIface *account)
{
	TnyAccountStorePriv *priv = TNY_ACCOUNT_STORE_GET_PRIVATE (self);

	g_object_ref (G_OBJECT (account));
	tny_account_store_add_account (self, TNY_ACCOUNT_IFACE (account), "transport");

	g_mutex_lock (priv->transport_accounts_lock);
	priv->transport_accounts = g_list_append (priv->transport_accounts, account);
	g_mutex_unlock (priv->transport_accounts_lock);

	g_signal_emit (self, tny_account_store_iface_signals [ACCOUNT_INSERTED], 0, account);

	return;
}


/**
 * tny_account_store_get_instance:
 *
 *
 * Return value: The #TnyAccountStoreIface singleton instance
 **/
TnyAccountStore*
tny_account_store_get_instance (void)
{
	TnyAccountStore *self = g_object_new (TNY_TYPE_ACCOUNT_STORE, NULL);

	return self;
}

static void
tny_account_store_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyAccountStore *self = (TnyAccountStore *)instance;
	TnyAccountStorePriv *priv = TNY_ACCOUNT_STORE_GET_PRIVATE (self);

	priv->store_accounts_lock = g_mutex_new ();
	priv->transport_accounts_lock = g_mutex_new ();

	priv->client = gconf_client_get_default ();

	gconf_client_add_dir (priv->client, "/apps/tinymail", 
		GCONF_CLIENT_PRELOAD_RECURSIVE, NULL);

	priv->notify = gconf_client_notify_add (priv->client, 
		"/apps/tinymail/accounts", gconf_listener_account_changed,
		self, NULL, NULL);

	return;
}


static void
tny_account_store_finalize (GObject *object)
{
	TnyAccountStore *self = (TnyAccountStore *)object;	
	TnyAccountStorePriv *priv = TNY_ACCOUNT_STORE_GET_PRIVATE (self);

	gconf_client_notify_remove (priv->client, priv->notify);
	g_object_unref (G_OBJECT (priv->client));

	destroy_current_accounts (priv);

	g_mutex_free (priv->store_accounts_lock);
	g_mutex_free (priv->transport_accounts_lock);

	priv->store_accounts_lock = NULL;
	priv->transport_accounts_lock = NULL;

	if (priv->cache_dir)
		g_free (priv->cache_dir);

	(*parent_class->finalize) (object);

	return;
}

static TnyAccountStore *the_singleton = NULL;

static GObject*
tny_account_store_constructor (GType type, guint n_construct_params,
			GObjectConstructParam *construct_params)
{
	GObject *object;

	/* TODO: potential problem: singleton without lock */

	if (!the_singleton)
	{
		object = G_OBJECT_CLASS (parent_class)->constructor (type,
				n_construct_params, construct_params);

		the_singleton = TNY_ACCOUNT_STORE (object);
	}
	else
	{
		object = g_object_ref (G_OBJECT (the_singleton));
		g_object_freeze_notify (G_OBJECT(the_singleton));
	}

	return object;
}


static void 
tny_account_store_class_init (TnyAccountStoreClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_account_store_finalize;
	object_class->constructor = tny_account_store_constructor;

	g_type_class_add_private (object_class, sizeof (TnyAccountStorePriv));

	return;
}

static void
tny_account_store_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyAccountStoreIfaceClass *klass = (TnyAccountStoreIfaceClass *)g_iface;

	klass->add_store_account_func = tny_account_store_add_store_account;
	klass->get_store_accounts_func = tny_account_store_get_store_accounts;
	klass->add_transport_account_func = tny_account_store_add_transport_account;
	klass->get_transport_accounts_func = tny_account_store_get_transport_accounts;
	klass->get_cache_dir_func = tny_account_store_get_cache_dir;

	return;
}


GType 
tny_account_store_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
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


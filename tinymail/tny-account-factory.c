/* libtinymail - The Tiny Mail base library
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

#include <glib.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>

#include <tny-account-factory.h>
#include <tny-password-dialog.h>
#include <tny-account-iface.h>
#include <tny-account.h>

static GObjectClass *parent_class = NULL;


typedef struct _TnyAccountFactoryPriv TnyAccountFactoryPriv;

struct _TnyAccountFactoryPriv
{
	GConfClient *client;
	GList *accounts;
};

#define TNY_ACCOUNT_FACTORY_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_ACCOUNT_FACTORY_TYPE, TnyAccountFactoryPriv))


static GHashTable *passwords;

static gchar* 
per_account_get_pass_func (TnyAccountIface *account)
{
	gchar *retval = NULL;
	const gchar *accountid = tny_account_iface_get_id (account);

	if (!passwords)
		passwords = g_hash_table_new (g_str_hash, g_str_equal);

	retval = g_hash_table_lookup (passwords, accountid);

	if (!retval)
	{
		GtkDialog *dialog = GTK_DIALOG (tny_password_dialog_new ());
	
		if (gtk_dialog_run (dialog) == GTK_RESPONSE_OK)
		{
			const gchar *pwd = tny_password_dialog_get_password 
				(TNY_PASSWORD_DIALOG (dialog));
	
			retval = g_strdup (pwd);

			g_hash_table_insert (passwords, g_strdup (accountid), retval);
		}

		gtk_widget_destroy (GTK_WIDGET (dialog));
	}

	return retval;
}

static void
per_account_forget_pass_func (TnyAccountIface *self)
{
}

/*
	gconftool-2 -s /apps/tinymail/accounts/count -t int 1
	gconftool-2 -s /apps/tinymail/accounts/0/proto -t string imap
	gconftool-2 -s /apps/tinymail/accounts/0/user -t string username
	gconftool-2 -s /apps/tinymail/accounts/0/hostname -t string mailserver
 */
GList*
tny_account_factory_get_accounts (TnyAccountFactory *self)
{
	TnyAccountFactoryPriv *priv = TNY_ACCOUNT_FACTORY_GET_PRIVATE (self);

	if (!priv->accounts)
	{
		gint i=0, count = gconf_client_get_int (priv->client, "/apps/tinymail/accounts/count", NULL);

		for (i=0; i < count; i++)
		{
			gchar *proto, *user, *hostname;
			gchar *key;
			TnyAccountIface *account = TNY_ACCOUNT_IFACE (tny_account_new ());

			key = g_strdup_printf ("/apps/tinymail/accounts/%d/proto", i);
			proto = gconf_client_get_string (priv->client, (const gchar*) key, NULL);
			g_free (key);
			tny_account_iface_set_proto (account, proto);

			key = g_strdup_printf ("/apps/tinymail/accounts/%d/user", i);
			user = gconf_client_get_string (priv->client, (const gchar*) key, NULL);
			g_free (key);
			tny_account_iface_set_user (account, user);

			key = g_strdup_printf ("/apps/tinymail/accounts/%d/hostname", i);
			hostname = gconf_client_get_string (priv->client, (const gchar*) key, NULL);
			g_free (key); 
			tny_account_iface_set_hostname (account, hostname);
			
			g_free (hostname); g_free (proto); g_free (user);

			key = g_strdup_printf ("/apps/tinymail/accounts/%d", i);
			tny_account_iface_set_id (account, key); g_free (key);

			tny_account_iface_set_pass_func (account, per_account_get_pass_func);

			priv->accounts = g_list_append (priv->accounts, account);
		}
	}

	return priv->accounts;
}

void
tny_account_factory_add_account (TnyAccountFactory *self, TnyAccountIface *account)
{
	TnyAccountFactoryPriv *priv = TNY_ACCOUNT_FACTORY_GET_PRIVATE (self);
}


TnyAccountFactory*
tny_account_factory_new (void)
{
	TnyAccountFactory *self = g_object_new (TNY_ACCOUNT_FACTORY_TYPE, NULL);

	return self;
}

static void
tny_account_factory_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyAccountFactory *self = (TnyAccountFactory *)instance;
	TnyAccountFactoryPriv *priv = TNY_ACCOUNT_FACTORY_GET_PRIVATE (self);

	priv->client = gconf_client_get_default ();

	return;
}

static void
tny_account_factory_finalize (GObject *object)
{
	TnyAccountFactory *self = (TnyAccountFactory *)object;	
	TnyAccountFactoryPriv *priv = TNY_ACCOUNT_FACTORY_GET_PRIVATE (self);

	(*parent_class->finalize) (object);

	return;
}

static TnyAccountFactory *the_singleton = NULL;

static GObject*
tny_account_factory_constructor (GType type, guint n_construct_params,
			GObjectConstructParam *construct_params)
{
	GObject *object;

	if (!the_singleton)
	{
		object = G_OBJECT_CLASS (parent_class)->constructor (type,
				n_construct_params, construct_params);

		the_singleton = TNY_ACCOUNT_FACTORY (object);
	}
	else
		object = g_object_ref (G_OBJECT (the_singleton));

	return object;
}


static void 
tny_account_factory_class_init (TnyAccountFactoryClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_account_factory_finalize;
	object_class->constructor = tny_account_factory_constructor;

	g_type_class_add_private (object_class, sizeof (TnyAccountFactoryPriv));

	return;
}

GType 
tny_account_factory_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyAccountFactoryClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_account_factory_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyAccountFactory),
		  0,      /* n_preallocs */
		  tny_account_factory_instance_init    /* instance_init */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyAccountFactory",
			&info, 0);
	}

	return type;
}


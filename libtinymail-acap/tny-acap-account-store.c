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

#include <config.h>
#include <glib/gi18n-lib.h>
#include <string.h>
#include <glib.h>

#include <tny-account-store.h>
#include <tny-acap-account-store.h>

#include <tny-account.h>
#include <tny-store-account.h>
#include <tny-transport-account.h>


static GObjectClass *parent_class = NULL;

typedef struct _TnyAcapAccountStorePriv TnyAcapAccountStorePriv;

struct _TnyAcapAccountStorePriv
{
	TnyAccountStore *real;
};

#define TNY_ACAP_ACCOUNT_STORE_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_ACAP_ACCOUNT_STORE, TnyAcapAccountStorePriv))


static gboolean
tny_acap_account_store_alert (TnyAccountStore *self, TnyAlertType type, const gchar *prompt)
{
	TnyAcapAccountStorePriv *priv = TNY_ACAP_ACCOUNT_STORE_GET_PRIVATE (self);
	return tny_account_store_alert (priv->real, type, prompt);
}


static TnyAccount* 
tny_acap_account_store_find_account (TnyAccountStore *self, const gchar *url_string)
{
	TnyAcapAccountStorePriv *priv = TNY_ACAP_ACCOUNT_STORE_GET_PRIVATE (self);

	return tny_account_store_find_account (priv->real, url_string);
}

static const gchar*
tny_acap_account_store_get_cache_dir (TnyAccountStore *self)
{
	TnyAcapAccountStorePriv *priv = TNY_ACAP_ACCOUNT_STORE_GET_PRIVATE (self);

	return tny_account_store_get_cache_dir (priv->real);
}


static void
tny_acap_account_store_get_accounts (TnyAccountStore *self, TnyList *list, TnyGetAccountsRequestType types)
{
	TnyAcapAccountStorePriv *priv = TNY_ACAP_ACCOUNT_STORE_GET_PRIVATE (self);

	/* TODO: if online, sync local with remote */

	tny_account_store_get_accounts (self, list, types);

	return;
}



static void
add_account_remote (TnyAccountStore *self, TnyAccount *account, const gchar *type)
{
	g_warning ("Not implemented\n");

	/* TODO: implement for ACAP */

	return;
}



static void
tny_acap_account_store_add_store_account (TnyAccountStore *self, TnyStoreAccount *account)
{
	TnyAcapAccountStorePriv *priv = TNY_ACAP_ACCOUNT_STORE_GET_PRIVATE (self);

	add_account_remote (self, TNY_ACCOUNT (account), "store");

	tny_account_store_add_store_account (priv->real, account);

	return;
}

static void
tny_acap_account_store_add_transport_account (TnyAccountStore *self, TnyTransportAccount *account)
{
	TnyAcapAccountStorePriv *priv = TNY_ACAP_ACCOUNT_STORE_GET_PRIVATE (self);

	add_account_remote (self, TNY_ACCOUNT (account), "transport");

	tny_account_store_add_transport_account (priv->real, account);

	return;
}

static TnyDevice*
tny_acap_account_store_get_device (TnyAccountStore *self)
{
	TnyAcapAccountStorePriv *priv = TNY_ACAP_ACCOUNT_STORE_GET_PRIVATE (self);

	return tny_account_store_get_device (priv->real);
}

/**
 * tny_acap_account_store_new:
 *
 *
 * Return value: A new #TnyAccountStore implemented for ACAP
 **/
TnyAccountStore*
tny_acap_account_store_new (TnyAccountStore *real)
{
	TnyAcapAccountStore *self = g_object_new (TNY_TYPE_ACAP_ACCOUNT_STORE, NULL);
	TnyAcapAccountStorePriv *priv = TNY_ACAP_ACCOUNT_STORE_GET_PRIVATE (self);

	priv->real = TNY_ACCOUNT_STORE (g_object_ref (G_OBJECT (real))); 

	return TNY_ACCOUNT_STORE (self);
}


static void
tny_acap_account_store_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyAcapAccountStorePriv *priv = TNY_ACAP_ACCOUNT_STORE_GET_PRIVATE (instance);

	priv->real = NULL;

	return;
}



static void
tny_acap_account_store_finalize (GObject *object)
{	
	TnyAcapAccountStorePriv *priv = TNY_ACAP_ACCOUNT_STORE_GET_PRIVATE (object);


	if (priv->real)
		g_object_unref (G_OBJECT (priv->real));

	(*parent_class->finalize) (object);

	return;
}


/**
 * tny_acap_account_store_get_real:
 * @self: The #TnyAcapAccountStore instance
 *
 * Get the real from proxy @self. You must unreference the return value after 
 * use.
 *
 * Return value: A #TnyAccountStore instance
 **/
TnyAccountStore*
tny_acap_account_store_get_real (TnyAcapAccountStore *self)
{
	TnyAcapAccountStorePriv *priv = TNY_ACAP_ACCOUNT_STORE_GET_PRIVATE (self);

	return TNY_ACCOUNT_STORE (g_object_ref (G_OBJECT (priv->real)));
}

static void 
tny_acap_account_store_class_init (TnyAcapAccountStoreClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_acap_account_store_finalize;

	g_type_class_add_private (object_class, sizeof (TnyAcapAccountStorePriv));

	return;
}

static void
tny_account_store_init (gpointer g, gpointer iface_data)
{
	TnyAccountStoreIface *klass = (TnyAccountStoreIface *)g;

	klass->get_accounts_func = tny_acap_account_store_get_accounts;
	klass->add_store_account_func = tny_acap_account_store_add_store_account;
	klass->add_transport_account_func = tny_acap_account_store_add_transport_account;
	klass->get_cache_dir_func = tny_acap_account_store_get_cache_dir;
	klass->get_device_func = tny_acap_account_store_get_device;
	klass->alert_func = tny_acap_account_store_alert;
	klass->find_account_func = tny_acap_account_store_find_account;

	return;
}


GType 
tny_acap_account_store_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyAcapAccountStoreClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_acap_account_store_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyAcapAccountStore),
		  0,      /* n_preallocs */
		  tny_acap_account_store_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_account_store_info = 
		{
		  (GInterfaceInitFunc) tny_account_store_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyAcapAccountStore",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_ACCOUNT_STORE, 
			&tny_account_store_info);
	}

	return type;
}

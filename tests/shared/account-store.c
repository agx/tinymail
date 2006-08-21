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

#include <tny-platform-factory-iface.h>
#include <tny-platform-factory.h>

#include <tny-account-store-iface.h>
#include <tny-account-iface.h>
#include <tny-store-account-iface.h>
#include <tny-transport-account-iface.h>
#include <tny-store-account.h>
#include <tny-transport-account.h>
#include <tny-device-iface.h>
#include <tny-device.h>

#include <tny-camel-shared.h>

#include "account-store.h"


static GObjectClass *parent_class = NULL;


static gchar* 
per_account_get_pass_func (TnyAccountIface *account, const gchar *prompt, gboolean *cancel)
{
	return g_strdup ("unittest");
}

static void
per_account_forget_pass_func (TnyAccountIface *account)
{
	g_print ("Invalid test account\n");
	return;
}


static gboolean
tny_memtest_account_store_alert (TnyAccountStoreIface *self, TnyAlertType type, const gchar *prompt)
{
	return TRUE;
}


static const gchar*
tny_memtest_account_store_get_cache_dir (TnyAccountStoreIface *self)
{
	/* FIXME: Small memleak here */
	return g_build_filename (g_get_home_dir (), ".tinymail", NULL);
}


static void
tny_memtest_account_store_get_accounts (TnyAccountStoreIface *self, TnyListIface *list, TnyGetAccountsRequestType types)
{
	TnyAccountIface *account = TNY_ACCOUNT_IFACE (tny_store_account_new ());
    
	tny_account_iface_set_account_store (account, (TnyAccountStoreIface*)self);
	tny_account_iface_set_proto (TNY_ACCOUNT_IFACE (account), "imap");
	tny_account_iface_set_name (TNY_ACCOUNT_IFACE (account), "unit test account");
	tny_account_iface_set_user (TNY_ACCOUNT_IFACE (account), "tinymailunittest");
	tny_account_iface_set_hostname (TNY_ACCOUNT_IFACE (account), "mail.tinymail.org");
	tny_account_iface_set_id (TNY_ACCOUNT_IFACE (account), "unique");
	tny_account_iface_set_forget_pass_func (TNY_ACCOUNT_IFACE (account),
			per_account_forget_pass_func);
	tny_account_iface_set_pass_func (TNY_ACCOUNT_IFACE (account),
			per_account_get_pass_func);

	tny_list_iface_prepend (list, (GObject*)account);
	g_object_unref (G_OBJECT (account));

	return;	
}



static void
tny_memtest_account_store_add_account (TnyAccountStoreIface *self, TnyAccountIface *account, const gchar *type)
{
	return;
}



TnyMemTestAccountStore*
tny_memtest_account_store_new (void)
{
	TnyMemTestAccountStore *self = g_object_new (TNY_TYPE_MEMTEST_ACCOUNT_STORE, NULL);

	return self;
}


static void
tny_memtest_account_store_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyDeviceIface *device = tny_account_store_iface_get_device (TNY_ACCOUNT_STORE_IFACE (instance));

	tny_device_iface_force_online (device);
    
	return;
}


static void
tny_memtest_account_store_finalize (GObject *object)
{
	(*parent_class->finalize) (object);

	return;
}


static void 
tny_memtest_account_store_class_init (TnyMemTestAccountStoreClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_memtest_account_store_finalize;

	return;
}

static void
tny_memtest_account_store_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyAccountStoreIfaceClass *klass = (TnyAccountStoreIfaceClass *)g_iface;

	klass->get_accounts_func = tny_memtest_account_store_get_accounts;
	klass->get_cache_dir_func = tny_memtest_account_store_get_cache_dir;
	klass->alert_func = tny_memtest_account_store_alert;
    
	return;
}


GType 
tny_memtest_account_store_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMemTestAccountStoreClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_memtest_account_store_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMemTestAccountStore),
		  0,      /* n_preallocs */
		  tny_memtest_account_store_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_memtest_account_store_iface_info = 
		{
		  (GInterfaceInitFunc) tny_memtest_account_store_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (TNY_TYPE_ACCOUNT_STORE,
			"TnyMemTestAccountStore",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_ACCOUNT_STORE_IFACE, 
			&tny_memtest_account_store_iface_info);
	}

	return type;
}

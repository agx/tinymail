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
#include <camel/camel-session.h>

#include <config.h>
#include <glib/gi18n-lib.h>

#include <string.h>
#include <glib.h>

#include <tny-platform-factory.h>
#include "platfact.h"

#include <tny-account-store.h>
#include <tny-account.h>
#include <tny-camel-account.h>

#include <tny-store-account.h>
#include <tny-transport-account.h>
#include <tny-camel-store-account.h>
#include <tny-camel-transport-account.h>
#include <tny-device.h>
#include <tny-session-camel.h>

#include "device.h"
#include "account-store.h"


static GObjectClass *parent_class = NULL;


static gchar* 
per_account_get_pass_func (TnyAccount *account, const gchar *prompt, gboolean *cancel)
{
	return g_strdup ("unittest");
}

static void
per_account_forget_pass_func (TnyAccount *account)
{
	g_print ("Invalid test account\n");
	return;
}


static gboolean
tny_test_account_store_alert (TnyAccountStore *self, TnyAlertType type, const gchar *prompt)
{
	return TRUE;
}


static const gchar*
tny_test_account_store_get_cache_dir (TnyAccountStore *self)
{
	TnyTestAccountStore *me = (TnyTestAccountStore*) self;

	if (me->cache_dir == NULL)
	{
		gint att=0;
		GDir *dir = NULL;
		do {
			gchar *attempt = g_strdup_printf ("tinymail.%d", att);
			gchar *str = g_build_filename (g_get_tmp_dir (), attempt, NULL);
			g_free (attempt);
			dir = g_dir_open (str, 0, NULL);
			if (dir)
			{
				g_dir_close (dir);
				g_free (str);
			} else 
				me->cache_dir = str;
			att++;
		} while (dir != NULL);          
	}
    
	return me->cache_dir;
}


static void
tny_test_account_store_get_accounts (TnyAccountStore *self, TnyList *list, TnyGetAccountsRequestType types)
{
	TnyTestAccountStore *me = (TnyTestAccountStore *) self;
	TnyAccount *account = TNY_ACCOUNT (tny_camel_store_account_new ());

	/* Dear visitor of the SVN-web. This is indeed a fully functional and
	   working IMAP account. This does not mean that you need to fuck it up */

	tny_camel_account_set_session (TNY_CAMEL_ACCOUNT (account), me->session);
	camel_session_set_online ((CamelSession*)me->session, me->force_online); 
	tny_camel_account_set_online_status (TNY_CAMEL_ACCOUNT (account), !me->force_online);

	tny_account_set_proto (account, "imap");
	tny_account_set_name (account, "unit test account");
	tny_account_set_user (account, "tinymailunittest");
	tny_account_set_hostname (account, "mail.tinymail.org");
	tny_account_set_id (account, "unique");
	tny_account_set_forget_pass_func (account, per_account_forget_pass_func);
	tny_account_set_pass_func (account, per_account_get_pass_func);

	tny_list_prepend (list, (GObject*)account);
	g_object_unref (G_OBJECT (account));

	return;
}



TnyAccountStore*
tny_test_account_store_new (gboolean force_online, const gchar *cachedir)
{
	TnyTestAccountStore *self = g_object_new (TNY_TYPE_TEST_ACCOUNT_STORE, NULL);

	if (cachedir)
	{
		if (self->cache_dir)
			g_free (self->cache_dir);
		self->cache_dir = g_strdup (cachedir);
	}

	self->session = tny_session_camel_new (TNY_ACCOUNT_STORE (self));
	self->force_online = force_online;

	if (self->force_online)
		tny_device_force_online (self->device);
	else
		tny_device_force_offline (self->device);

	return TNY_ACCOUNT_STORE (self);
}


static void
tny_test_account_store_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyTestAccountStore *self = (TnyTestAccountStore *)instance;
	TnyPlatformFactory *platfact = tny_test_platform_factory_get_instance ();

	self->device = tny_platform_factory_new_device (platfact);

	return;
}


static void
tny_test_account_store_finalize (GObject *object)
{
	TnyTestAccountStore *me = (TnyTestAccountStore*) object;
    
	if (me->cache_dir)
		g_free (me->cache_dir);
    
	(*parent_class->finalize) (object);

	return;
}


static void 
tny_test_account_store_class_init (TnyTestAccountStoreClass *class)
{
	GObjectClass *object_class;
    
	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_test_account_store_finalize;

	return;
}


static void
tny_test_account_store_add_store_account (TnyAccountStore *self, TnyStoreAccount *account)
{
	return;
}

static void
tny_test_account_store_add_transport_account (TnyAccountStore *self, TnyTransportAccount *account)
{
	return;
}

static TnyDevice*
tny_test_account_store_get_device (TnyAccountStore *self)
{
	TnyTestAccountStore *me =  (TnyTestAccountStore*) self;

	return g_object_ref (G_OBJECT (me->device));
}


static void
tny_account_store_init (gpointer g, gpointer iface_data)
{
	TnyAccountStoreIface *klass = (TnyAccountStoreIface *)g;

	klass->get_accounts_func = tny_test_account_store_get_accounts;
	klass->get_cache_dir_func = tny_test_account_store_get_cache_dir;
	klass->alert_func = tny_test_account_store_alert;
	klass->add_store_account_func = tny_test_account_store_add_store_account;
	klass->add_transport_account_func = tny_test_account_store_add_transport_account;
	klass->get_device_func = tny_test_account_store_get_device;
    
	return;
}


GType 
tny_test_account_store_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyTestAccountStoreClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_test_account_store_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyTestAccountStore),
		  0,      /* n_preallocs */
		  tny_test_account_store_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_account_store_info = 
		{
		  (GInterfaceInitFunc) tny_account_store_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyTestAccountStore",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_ACCOUNT_STORE, 
			&tny_account_store_info);
	}

	return type;
}

/* libtinymail-camel - The Tiny Mail base library for Camel
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

#include "platfact.h"

#include <tny-save-strategy-iface.h>
#include <tny-save-strategy.h>

#include <tny-account-store-iface.h>
#include "account-store.h"

#include <tny-device-iface.h>
#include "device.h"

#include <tny-msg-view-iface.h>
#include <tny-msg-view.h>


static GObjectClass *parent_class = NULL;

static void
tny_platform_factory_instance_init (GTypeInstance *instance, gpointer g_class)
{
	return;
}


static TnyAccountStoreIface*
tny_platform_factory_new_account_store (TnyPlatformFactoryIface *self)
{
	return TNY_ACCOUNT_STORE_IFACE (tny_account_store_new (FALSE, NULL));
}

static TnyDeviceIface*
tny_platform_factory_new_device (TnyPlatformFactoryIface *self)
{
	return TNY_DEVICE_IFACE (tny_device_new ());
}

static TnyMsgViewIface*
tny_platform_factory_new_msg_view (TnyPlatformFactoryIface *self)
{
	TnySaveStrategyIface *save_strategy = 
		TNY_SAVE_STRATEGY_IFACE (tny_save_strategy_new ());

	return TNY_MSG_VIEW_IFACE (tny_msg_view_new (save_strategy));
}

/**
 * tny_platform_factory_get_instance:
 *
 *
 * Return value: The #TnyPlatformFactoryIface singleton instance
 **/
TnyPlatformFactory*
tny_platform_factory_get_instance (void)
{
	TnyPlatformFactory *self = g_object_new (TNY_TYPE_PLATFORM_FACTORY, NULL);

	return self;
}


static void
tny_platform_factory_finalize (GObject *object)
{
	(*parent_class->finalize) (object);

	return;
}


static void
tny_platform_factory_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyPlatformFactoryIfaceClass *klass = (TnyPlatformFactoryIfaceClass *)g_iface;

	klass->new_account_store_func = tny_platform_factory_new_account_store;
	klass->new_device_func = tny_platform_factory_new_device;
	klass->new_msg_view_func = tny_platform_factory_new_msg_view;

	return;
}


static TnyPlatformFactory *the_singleton = NULL;


static GObject*
tny_platform_factory_constructor (GType type, guint n_construct_params,
			GObjectConstructParam *construct_params)
{
	GObject *object;

	/* TODO: potential problem: singleton without lock */

	if (G_UNLIKELY (!the_singleton))
	{
		object = G_OBJECT_CLASS (parent_class)->constructor (type,
				n_construct_params, construct_params);

		the_singleton = TNY_PLATFORM_FACTORY (object);
	}
	else
	{
		/* refdbg killed bug! 
		object = g_object_ref (G_OBJECT (the_singleton)); */

		object = G_OBJECT (the_singleton);
		g_object_freeze_notify (G_OBJECT(the_singleton));
	}

	return object;
}

static void 
tny_platform_factory_class_init (TnyPlatformFactoryClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_platform_factory_finalize;
	object_class->constructor = tny_platform_factory_constructor;

	return;
}

GType 
tny_platform_factory_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyPlatformFactoryClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_platform_factory_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyPlatformFactory),
		  0,      /* n_preallocs */
		  tny_platform_factory_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_platform_factory_iface_info = 
		{
		  (GInterfaceInitFunc) tny_platform_factory_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyPlatformFactory",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_PLATFORM_FACTORY_IFACE, 
			&tny_platform_factory_iface_info);

	}

	return type;
}

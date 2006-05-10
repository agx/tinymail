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

#include <tny-device.h>
#include <libnm_glib.h>

static GObjectClass *parent_class = NULL;

#include "tny-device-priv.h"

static void
tny_device_on_online (TnyDeviceIface *self)
{
	g_signal_emit (self, tny_device_iface_signals [CONNECTION_CHANGED], 0, TRUE);

	return;
}

static void
tny_device_on_offline (TnyDeviceIface *self)
{
	g_signal_emit (self, tny_device_iface_signals [CONNECTION_CHANGED], 0, FALSE);

	return;
}

static void 
nm_callback (libnm_glib_ctx *nm_ctx, gpointer user_data)
{
	TnyDevice *self = (TnyDevice *)user_data;
	TnyDevicePriv *priv = TNY_DEVICE_GET_PRIVATE (self);
	libnm_glib_state state = libnm_glib_get_network_state (nm_ctx);

	switch (state)
	{
		case LIBNM_NO_NETWORK_CONNECTION:
		tny_device_on_offline (TNY_DEVICE_IFACE (self));
		break;

		case LIBNM_NO_DBUS:
		case LIBNM_NO_NETWORKMANAGER:
		case LIBNM_INVALID_CONTEXT:
		g_print ("Invalid network manager installation. Going to assume Online status\n");
		case LIBNM_ACTIVE_NETWORK_CONNECTION:
		default:
		tny_device_on_online (TNY_DEVICE_IFACE (self));
		break;
	}

	return;
}

static void
tny_device_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyDevice *self = (TnyDevice *)instance;
	TnyDevicePriv *priv = TNY_DEVICE_GET_PRIVATE (self);

	priv->nm_ctx = libnm_glib_init ();
	priv->callback_id = libnm_glib_register_callback 
		(priv->nm_ctx, nm_callback, self, NULL);

	return;
}



/**
 * tny_device_get_instance:
 *
 *
 * Return value: The #TnyDeviceIface singleton instance
 **/
TnyDevice*
tny_device_new (void)
{
	TnyDevice *self = g_object_new (TNY_TYPE_DEVICE, NULL);

	return self;
}


static void
tny_device_finalize (GObject *object)
{
	TnyDevice *self = (TnyDevice *)object;	
	TnyDevicePriv *priv = TNY_DEVICE_GET_PRIVATE (self);


	libnm_glib_unregister_callback (priv->nm_ctx, priv->callback_id);
	libnm_glib_shutdown (priv->nm_ctx);

	(*parent_class->finalize) (object);

	return;
}


static void
tny_device_iface_init (gpointer g_iface, gpointer iface_data)
{
	/* TnyDeviceIfaceClass *klass = (TnyDeviceIfaceClass *)g_iface; */

	return;
}



static void 
tny_device_class_init (TnyDeviceClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_device_finalize;

	g_type_class_add_private (object_class, sizeof (TnyDevicePriv));

	return;
}

GType 
tny_device_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyDeviceClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_device_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyDevice),
		  0,      /* n_preallocs */
		  tny_device_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_device_iface_info = 
		{
		  (GInterfaceInitFunc) tny_device_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyDevice",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_DEVICE_IFACE, 
			&tny_device_iface_info);

	}

	return type;
}

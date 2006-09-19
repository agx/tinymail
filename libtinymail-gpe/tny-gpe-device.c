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

#include <glib/gi18n-lib.h>

#include <tny-gpe-device.h>

static GObjectClass *parent_class = NULL;

#include "tny-gpe-device-priv.h"

static void tny_gpe_device_on_online (TnyDevice *self);
static void tny_gpe_device_on_offline (TnyDevice *self);
static gboolean tny_gpe_device_is_online (TnyDevice *self);


static void 
tny_gpe_device_reset (TnyDevice *self)
{
	TnyGpeDevicePriv *priv = TNY_GPE_DEVICE_GET_PRIVATE (self);

	priv->fset = FALSE;
	priv->forced = FALSE;

}

static void 
tny_gpe_device_force_online (TnyDevice *self)
{
	TnyGpeDevicePriv *priv = TNY_GPE_DEVICE_GET_PRIVATE (self);

	priv->fset = TRUE;
	priv->forced = TRUE;

	tny_gpe_device_on_online (self);

	return;
}


static void
tny_gpe_device_force_offline (TnyDevice *self)
{
	TnyGpeDevicePriv *priv = TNY_GPE_DEVICE_GET_PRIVATE (self);

	priv->fset = TRUE;
	priv->forced = FALSE;


	tny_gpe_device_on_offline (self);
	
	return;
}

static void
tny_gpe_device_on_online (TnyDevice *self)
{
	g_signal_emit (self, tny_device_signals [TNY_DEVICE_CONNECTION_CHANGED], 0, TRUE);

	return;
}

static void
tny_gpe_device_on_offline (TnyDevice *self)
{
	g_signal_emit (self, tny_device_signals [TNY_DEVICE_CONNECTION_CHANGED], 0, FALSE);

	return;
}

static gboolean
tny_gpe_device_is_online (TnyDevice *self)
{
	TnyGpeDevicePriv *priv = TNY_GPE_DEVICE_GET_PRIVATE (self);
	gboolean retval = TRUE;
	return retval;
}

static void
tny_gpe_device_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyGpeDevice *self = (TnyGpeDevice *)instance;
	TnyGpeDevicePriv *priv = TNY_GPE_DEVICE_GET_PRIVATE (self);

	return;
}



/**
 * tny_gpe_device_new:
 *
 * Return value: A new #TnyDevice instance
 **/
TnyDevice*
tny_gpe_device_new (void)
{
	TnyGpeDevice *self = g_object_new (TNY_TYPE_GPE_DEVICE, NULL);

	return TNY_DEVICE (self);
}


static void
tny_gpe_device_finalize (GObject *object)
{
	(*parent_class->finalize) (object);
    
	return;
}


static void
tny_device_init (gpointer g, gpointer iface_data)
{
	TnyDeviceIface *klass = (TnyDeviceIface *)g;

	klass->is_online_func = tny_gpe_device_is_online;
	klass->reset_func = tny_gpe_device_reset;
	klass->force_offline_func = tny_gpe_device_force_offline;
	klass->force_online_func = tny_gpe_device_force_online;

	return;
}



static void 
tny_gpe_device_class_init (TnyGpeDeviceClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_gpe_device_finalize;

	g_type_class_add_private (object_class, sizeof (TnyGpeDevicePriv));

	return;
}

GType 
tny_gpe_device_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyGpeDeviceClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_gpe_device_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyGpeDevice),
		  0,      /* n_preallocs */
		  tny_gpe_device_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_device_info = 
		{
		  (GInterfaceInitFunc) tny_device_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyGpeDevice",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_DEVICE, 
			&tny_device_info);

	}

	return type;
}

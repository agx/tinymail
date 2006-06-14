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

#include <config.h>

#ifndef TNY_DEVICE_IFACE_C
#define TNY_DEVICE_IFACE_C
#endif

#include <tny-device-iface.h>

#ifdef TNY_DEVICE_IFACE_C
#undef TNY_DEVICE_IFACE_C
#endif

guint *tny_device_iface_signals = NULL;


/**
 * tny_device_iface_reset:
 * @self: a #TnyDeviceIface object
 * 
 * Reset status
 **/
void 
tny_device_iface_reset (TnyDeviceIface *self)
{
#ifdef DEBUG
	if (!TNY_DEVICE_IFACE_GET_CLASS (self)->reset_func)
		g_critical ("You must implement tny_device_iface_reset\n");
#endif

	TNY_DEVICE_IFACE_GET_CLASS (self)->reset_func (self);
	return;
}

/**
 * tny_device_iface_force_offline:
 * @self: a #TnyDeviceIface object
 * 
 * Force offline status
 **/
void 
tny_device_iface_force_online (TnyDeviceIface *self)
{
#ifdef DEBUG
	if (!TNY_DEVICE_IFACE_GET_CLASS (self)->force_online_func)
		g_critical ("You must implement tny_device_iface_force_online\n");
#endif

	TNY_DEVICE_IFACE_GET_CLASS (self)->force_online_func (self);
	return;
}

/**
 * tny_device_iface_force_online:
 * @self: a #TnyDeviceIface object
 * 
 * Force online status
 **/
void
tny_device_iface_force_offline (TnyDeviceIface *self)
{
#ifdef DEBUG
	if (!TNY_DEVICE_IFACE_GET_CLASS (self)->force_offline_func)
		g_critical ("You must implement tny_device_iface_force_offline\n");
#endif

	TNY_DEVICE_IFACE_GET_CLASS (self)->force_offline_func (self);
	return;
}

/**
 * tny_device_iface_is_online:
 * @self: a #TnyDeviceIface object
 * 
 * Return value: Whether the device is online
 **/
gboolean 
tny_device_iface_is_online (TnyDeviceIface *self)
{
#ifdef DEBUG
	if (!TNY_DEVICE_IFACE_GET_CLASS (self)->is_online_func)
		g_critical ("You must implement tny_device_iface_is_online\n");
#endif

	return TNY_DEVICE_IFACE_GET_CLASS (self)->is_online_func (self);
}


static void
tny_device_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) 
	{
		tny_device_iface_signals = g_new0 (guint, TNY_DEVICE_IFACE_LAST_SIGNAL);

/**
 * TnyDeviceIface::connection_changed:
 * @self: the object on which the signal is emitted
 * @arg1: Whether or not the device is now online
 * @user_data: user data set when the signal handler was connected.
 *
 * Emitted when the connection status of a device changes.
 */
		tny_device_iface_signals[TNY_DEVICE_IFACE_CONNECTION_CHANGED] =
		   g_signal_new ("connection_changed",
			TNY_TYPE_DEVICE_IFACE,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnyDeviceIfaceClass, connection_changed),
			NULL, NULL,
			g_cclosure_marshal_VOID__BOOLEAN, G_TYPE_NONE, 1,
				G_TYPE_BOOLEAN);

		initialized = TRUE;
	}
}

GType
tny_device_iface_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyDeviceIfaceClass),
		  tny_device_iface_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyDeviceIface", &info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}

	return type;
}

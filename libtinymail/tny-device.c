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

#include <tny-device.h>

guint tny_device_signals [TNY_DEVICE_LAST_SIGNAL];

/**
 * tny_device_reset:
 * @self: a #TnyDevice object
 * 
 * Reset the status (unforce the status)
 **/
void 
tny_device_reset (TnyDevice *self)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_DEVICE (self));
	g_assert (TNY_DEVICE_GET_IFACE (self)->reset_func != NULL);
#endif

	TNY_DEVICE_GET_IFACE (self)->reset_func (self);

#ifdef DBC /* ensure */
#endif

	return;
}

/**
 * tny_device_force_offline:
 * @self: a #TnyDevice object
 * 
 * Force offline status
 * 
 * Example:
 * <informalexample><programlisting>
 * TnyDevice *device = ...
 * tny_device_force_offline (device);
 * if (tny_device_is_online (device))
 *      g_print ("Something is wrong\n");
 * tny_device_reset (device);
 * </programlisting></informalexample>
 **/
void 
tny_device_force_online (TnyDevice *self)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_DEVICE (self));
	g_assert (TNY_DEVICE_GET_IFACE (self)->force_online_func != NULL);
#endif

	TNY_DEVICE_GET_IFACE (self)->force_online_func (self);

#ifdef DBC /* ensure */
	g_assert (tny_device_is_online (self) == TRUE);
#endif

	return;
}

/**
 * tny_device_force_online:
 * @self: a #TnyDevice object
 * 
 * Force online status
 **/
void
tny_device_force_offline (TnyDevice *self)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_DEVICE (self));
	g_assert (TNY_DEVICE_GET_IFACE (self)->force_offline_func != NULL);
#endif

	TNY_DEVICE_GET_IFACE (self)->force_offline_func (self);

#ifdef DBC /* ensure */
	g_assert (tny_device_is_online (self) == FALSE);
#endif

	return;
}

/**
 * tny_device_is_online:
 * @self: a #TnyDevice object
 * 
 * Example:
 * <informalexample><programlisting>
 * static void
 * connection_changed (TnyDevice *device, gboolean online, gpointer user_data)
 * {
 *      if (!online && tny_device_is_online (device))
 *           g_print ("Something is wrong\n");
 * }
 * TnyDevice *device = ...
 * g_signal_connect (G_OBJECT (device), "connection_changed",
 *       G_CALLBACK (connection_changed), self);
 * </programlisting></informalexample>
 * 
 * Return value: Whether the device is online
 **/
gboolean 
tny_device_is_online (TnyDevice *self)
{
	gboolean retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_DEVICE (self));
	g_assert (TNY_DEVICE_GET_IFACE (self)->is_online_func != NULL);
#endif

	retval = TNY_DEVICE_GET_IFACE (self)->is_online_func (self);

#ifdef DBC /* ensure */
#endif

	return retval;
}

static void
tny_device_base_init (gpointer g_class)
{
	static gboolean tny_device_initialized = FALSE;

	if (!tny_device_initialized) 
	{
/**
 * TnyDevice::connection-changed
 * @self: the object on which the signal is emitted
 * @arg1: Whether or not the device is now online
 * @user_data: user data set when the signal handler was connected.
 *
 * Emitted when the connection status of a device changes.
 */
		tny_device_signals[TNY_DEVICE_CONNECTION_CHANGED] =
		   g_signal_new ("connection_changed",
			TNY_TYPE_DEVICE,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnyDeviceIface, connection_changed),
			NULL, NULL,
			g_cclosure_marshal_VOID__BOOLEAN, G_TYPE_NONE, 1,
				G_TYPE_BOOLEAN);

		tny_device_initialized = TRUE;
	}
}

GType
tny_device_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyDeviceIface),
		  tny_device_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL,   /* instance_init */
		  NULL
		};

		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyDevice", &info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}

	return type;
}

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
 * @online: Whether or not the device is now online
 *
 * The "connection_changed" signal is emitted when the connection
 * status of a device changes.
 **/
		tny_device_iface_signals[CONNECTION_CHANGED] =
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

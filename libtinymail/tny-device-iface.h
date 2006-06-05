#ifndef TNY_DEVICE_IFACE_H
#define TNY_DEVICE_IFACE_H

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
#include <glib-object.h>
#include <tny-shared.h>

G_BEGIN_DECLS

#define TNY_TYPE_DEVICE_IFACE             (tny_device_iface_get_type ())
#define TNY_DEVICE_IFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_DEVICE_IFACE, TnyDeviceIface))
#define TNY_DEVICE_IFACE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_DEVICE_IFACE, TnyDeviceIfaceClass))
#define TNY_IS_DEVICE_IFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_DEVICE_IFACE))
#define TNY_IS_DEVICE_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_DEVICE_IFACE))
#define TNY_DEVICE_IFACE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_DEVICE_IFACE, TnyDeviceIfaceClass))

#ifndef TNY_DEVICE_IFACE_C
extern guint *tny_device_iface_signals;
#endif

enum
{
	TNY_DEVICE_IFACE_CONNECTION_CHANGED,
	TNY_DEVICE_IFACE_LAST_SIGNAL
};

struct _TnyDeviceIfaceClass
{
	GTypeInterface parent;

	gboolean (*is_online_func)  (TnyDeviceIface *self);

	/* Signals */
	void (*connection_changed)  (TnyDeviceIface *self, gboolean online);

};

GType tny_device_iface_get_type (void);

gboolean tny_device_iface_is_online (TnyDeviceIface *self);

G_END_DECLS

#endif

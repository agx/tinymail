#ifndef TNY_DEVICE_H
#define TNY_DEVICE_H

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

#define TNY_TYPE_DEVICE             (tny_device_get_type ())
#define TNY_DEVICE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_DEVICE, TnyDevice))
#define TNY_IS_DEVICE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_DEVICE))
#define TNY_DEVICE_GET_IFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_DEVICE, TnyDeviceIface))

#ifndef TNY_DEVICE_C
extern guint *tny_device_signals;
#endif

enum
{
	TNY_DEVICE_CONNECTION_CHANGED,
	TNY_DEVICE_LAST_SIGNAL
};

struct _TnyDeviceIface
{
	GTypeInterface parent;

	gboolean (*is_online_func) (TnyDevice *self);

	void (*force_online_func) (TnyDevice *self);
	void (*force_offline_func) (TnyDevice *self);
	void (*reset_func) (TnyDevice *self);

	/* Signals */
	void (*connection_changed) (TnyDevice *self, gboolean online);

};

GType tny_device_get_type (void);

gboolean tny_device_is_online (TnyDevice *self);
void tny_device_force_online (TnyDevice *self);
void tny_device_force_offline (TnyDevice *self);

G_END_DECLS

#endif

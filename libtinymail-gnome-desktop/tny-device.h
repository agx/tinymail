#ifndef TNY_DEVICE_H
#define TNY_DEVICE_H

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

#include <glib.h>
#include <glib-object.h>

#include <tny-device-iface.h>

G_BEGIN_DECLS

#define TNY_TYPE_DEVICE             (tny_device_get_type ())
#define TNY_DEVICE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_DEVICE, TnyDevice))
#define TNY_DEVICE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_DEVICE, TnyDeviceClass))
#define TNY_IS_DEVICE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_DEVICE))
#define TNY_IS_DEVICE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_DEVICE))
#define TNY_DEVICE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_DEVICE, TnyDeviceClass))

/* This is an abstract type */

typedef struct _TnyDevice TnyDevice;
typedef struct _TnyDeviceClass TnyDeviceClass;

struct _TnyDevice
{
	GObject parent;
};

struct _TnyDeviceClass 
{
	GObjectClass parent;
};

GType tny_device_get_type (void);

TnyDevice* tny_device_get_new (void);

G_END_DECLS

#endif


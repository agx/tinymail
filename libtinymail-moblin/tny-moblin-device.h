#ifndef TNY_MOBLIN_DEVICE_H
#define TNY_MOBLIN_DEVICE_H

/* libtinymail-moblin - The Tinymail base library for Moblin
 * Copyright (C) 2010 Sergio Villar Senin <svillar@igalia.com>
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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <glib.h>
#include <glib-object.h>
#include <tny-device.h>

G_BEGIN_DECLS

#define TNY_TYPE_MOBLIN_DEVICE             (tny_moblin_device_get_type ())
#define TNY_MOBLIN_DEVICE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_MOBLIN_DEVICE, TnyMoblinDevice))
#define TNY_MOBLIN_DEVICE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_MOBLIN_DEVICE, TnyMoblinDeviceClass))
#define TNY_IS_MOBLIN_DEVICE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_MOBLIN_DEVICE))
#define TNY_IS_MOBLIN_DEVICE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_MOBLIN_DEVICE))
#define TNY_MOBLIN_DEVICE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_MOBLIN_DEVICE, TnyMoblinDeviceClass))

typedef struct _TnyMoblinDevice TnyMoblinDevice;
typedef struct _TnyMoblinDeviceClass TnyMoblinDeviceClass;

struct _TnyMoblinDevice
{
	GObject parent;
};

struct _TnyMoblinDeviceClass
{
	GObjectClass parent;
};

GType tny_moblin_device_get_type (void);

TnyDevice* tny_moblin_device_new (void);

G_END_DECLS

#endif

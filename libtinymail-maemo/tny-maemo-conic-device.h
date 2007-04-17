#ifndef TNY_MAEMO_CONIC_DEVICE_H
#define TNY_MAEMO_CONIC_DEVICE_H

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

#include <tny-device.h>
#include <conicevent.h>
#include <coniciap.h>
#include <conicconnection.h>
#include <conicconnectionevent.h>

G_BEGIN_DECLS

#define TNY_TYPE_MAEMO_CONIC_DEVICE             (tny_maemo_device_get_type ())
#define TNY_MAEMO_CONIC_DEVICE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_MAEMO_CONIC_DEVICE, TnyMaemoConicDevice))
#define TNY_MAEMO_CONIC_DEVICE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_MAEMO_CONIC_DEVICE, TnyMaemoConicDeviceClass))
#define TNY_IS_MAEMO_CONIC_DEVICE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_MAEMO_CONIC_DEVICE))
#define TNY_IS_MAEMO_CONIC_DEVICE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_MAEMO_CONIC_DEVICE))
#define TNY_MAEMO_CONIC_DEVICE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_MAEMO_CONIC_DEVICE, TnyMaemoConicDeviceClass))

/* This is an abstract type */

typedef struct _TnyMaemoConicDevice      TnyMaemoConicDevice;
typedef struct _TnyMaemoConicDeviceClass TnyMaemoConicDeviceClass;

struct _TnyMaemoConicDevice
{
	GObject parent;
};

struct _TnyMaemoConicDeviceClass 
{
	GObjectClass parent;
};

GType tny_maemo_conic_device_get_type (void);

TnyDevice* tny_maemo_conic_device_new (void);

gboolean tny_maemo_conic_device_connect (TnyDevice *self, const gchar* iap_id);
gboolean tny_maemo_conic_device_disconnect (TnyDevice *self, const gchar* iap_id);
const gchar* tny_maemo_conic_device_get_current_iap_id (TnyDevice *self);
ConIcIap* tny_maemo_conic_device_get_iap (TnyDevice *self, const gchar *iap_id);
GSList* tny_maemo_conic_device_get_iap_list (TnyDevice *self);
void tny_maemo_conic_device_free_iap_list (TnyDevice *self, GSList* cnx_list);

G_END_DECLS

#endif

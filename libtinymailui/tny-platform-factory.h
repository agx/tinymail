#ifndef TNY_PLATFORM_FACTORY_H
#define TNY_PLATFORM_FACTORY_H

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
#include <tny-msg-view.h>
#include <tny-device.h>
#include <tny-account-store.h>

G_BEGIN_DECLS

#define TNY_TYPE_PLATFORM_FACTORY             (tny_platform_factory_get_type ())
#define TNY_PLATFORM_FACTORY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_PLATFORM_FACTORY, TnyPlatformFactory))
#define TNY_IS_PLATFORM_FACTORY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_PLATFORM_FACTORY))
#define TNY_PLATFORM_FACTORY_GET_IFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_PLATFORM_FACTORY, TnyPlatformFactoryIface))

typedef struct _TnyPlatformFactory TnyPlatformFactory;
typedef struct _TnyPlatformFactoryIface TnyPlatformFactoryIface;

struct _TnyPlatformFactoryIface
{
	GTypeInterface parent;

	TnyAccountStore* (*new_account_store_func) (TnyPlatformFactory *self);
	TnyDevice* (*new_device_func) (TnyPlatformFactory *self);
	TnyMsgView* (*new_msg_view_func) (TnyPlatformFactory *self);
	TnyMsg* (*new_msg_func) (TnyPlatformFactory *self);
	TnyMimePart* (*new_mime_part_func) (TnyPlatformFactory *self);
};

GType tny_platform_factory_get_type (void);

TnyAccountStore* tny_platform_factory_new_account_store (TnyPlatformFactory *self);
TnyDevice* tny_platform_factory_new_device (TnyPlatformFactory *self);
TnyMsgView* tny_platform_factory_new_msg_view (TnyPlatformFactory *self);
TnyMsg* tny_platform_factory_new_msg (TnyPlatformFactory *self);
TnyMimePart* tny_platform_factory_new_mime_part (TnyPlatformFactory *self);

G_END_DECLS

#endif

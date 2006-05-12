#ifndef TNY_PLATFORM_FACTORY_IFACE_H
#define TNY_PLATFORM_FACTORY_IFACE_H

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
#include <tny-msg-view-iface.h>

G_BEGIN_DECLS

#define TNY_TYPE_PLATFORM_FACTORY_IFACE             (tny_platform_factory_iface_get_type ())
#define TNY_PLATFORM_FACTORY_IFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_PLATFORM_FACTORY_IFACE, TnyPlatformFactoryIface))
#define TNY_PLATFORM_FACTORY_IFACE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_PLATFORM_FACTORY_IFACE, TnyPlatformFactoryIfaceClass))
#define TNY_IS_PLATFORM_FACTORY_IFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_PLATFORM_FACTORY_IFACE))
#define TNY_IS_PLATFORM_FACTORY_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_PLATFORM_FACTORY_IFACE))
#define TNY_PLATFORM_FACTORY_IFACE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_PLATFORM_FACTORY_IFACE, TnyPlatformFactoryIfaceClass))

typedef struct _TnyPlatformFactoryIface TnyPlatformFactoryIface;
typedef struct _TnyPlatformFactoryIfaceClass TnyPlatformFactoryIfaceClass;

struct _TnyPlatformFactoryIfaceClass
{
	GTypeInterface parent;

	TnyAccountStoreIface*  (*new_account_store_func)  (TnyPlatformFactoryIface *self);
	TnyDeviceIface*        (*new_device_func)         (TnyPlatformFactoryIface *self);
	TnyMsgViewIface*       (*new_msg_view_func)       (TnyPlatformFactoryIface *self);
};

GType tny_platform_factory_iface_get_type (void);

TnyAccountStoreIface*  tny_platform_factory_iface_new_account_store  (TnyPlatformFactoryIface *self);
TnyDeviceIface*        tny_platform_factory_iface_new_device         (TnyPlatformFactoryIface *self);
TnyMsgViewIface*       tny_platform_factory_iface_new_msg_view       (TnyPlatformFactoryIface *self);

G_END_DECLS

#endif

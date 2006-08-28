#ifndef TNY_TRANSPORT_ACCOUNT_IFACE_H
#define TNY_TRANSPORT_ACCOUNT_IFACE_H

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

#include <tny-account-iface.h>

G_BEGIN_DECLS

#define TNY_TYPE_TRANSPORT_ACCOUNT_IFACE             (tny_transport_account_iface_get_type ())
#define TNY_TRANSPORT_ACCOUNT_IFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_TRANSPORT_ACCOUNT_IFACE, TnyTransportAccountIface))
#define TNY_TRANSPORT_ACCOUNT_IFACE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_TRANSPORT_ACCOUNT_IFACE, TnyTransportAccountIfaceClass))
#define TNY_IS_TRANSPORT_ACCOUNT_IFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_TRANSPORT_ACCOUNT_IFACE))
#define TNY_IS_TRANSPORT_ACCOUNT_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_TRANSPORT_ACCOUNT_IFACE))
#define TNY_TRANSPORT_ACCOUNT_IFACE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_TRANSPORT_ACCOUNT_IFACE, TnyTransportAccountIfaceClass))

struct _TnyTransportAccountIfaceClass
{
	GTypeInterface parent;

	void (*send_func) (TnyTransportAccountIface *self, TnyMsgIface *msg);
};

GType tny_transport_account_iface_get_type (void);
void tny_transport_account_iface_send (TnyTransportAccountIface *self, TnyMsgIface *msg);

G_END_DECLS

#endif

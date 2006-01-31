#ifndef TNY_MSG_BODY_IFACE_H
#define TNY_MSG_BODY_IFACE_H

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

G_BEGIN_DECLS


#define TNY_MSG_BODY_IFACE_TYPE             (tny_msg_body_iface_get_type ())
#define TNY_MSG_BODY_IFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_MSG_BODY_IFACE_TYPE, TnyMsgBodyIface))
#define TNY_MSG_BODY_IFACE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_MSG_BODY_IFACE_TYPE, TnyMsgBodyIfaceClass))
#define TNY_IS_MSG_BODY_IFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_MSG_BODY_IFACE_TYPE))
#define TNY_IS_MSG_BODY_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_MSG_BODY_IFACE_TYPE))
#define TNY_MSG_BODY_IFACE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_MSG_BODY_IFACE_TYPE, TnyMsgBodyIfaceClass))


typedef struct _TnyMsgBodyIface TnyMsgBodyIface;
typedef struct _TnyMsgBodyIfaceClass TnyMsgBodyIfaceClass;

struct _TnyMsgBodyIfaceClass
{
	GTypeInterface parent;

	const gchar* (*get_data_func) (TnyMsgBodyIface *self);
	void         (*set_data_func) (TnyMsgBodyIface *self, gchar* data);
};

GType        tny_msg_body_iface_get_type (void);

const gchar* tny_msg_body_iface_get_data (TnyMsgBodyIface *self);
void         tny_msg_body_iface_set_data (TnyMsgBodyIface *self, gchar *data);

G_END_DECLS

#endif

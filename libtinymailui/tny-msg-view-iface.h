#ifndef TNY_MSG_VIEW_IFACE_H
#define TNY_MSG_VIEW_IFACE_H

/* libtinymailui - The Tiny Mail UI library
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

#define TNY_TYPE_MSG_VIEW_IFACE             (tny_msg_view_iface_get_type ())
#define TNY_MSG_VIEW_IFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_MSG_VIEW_IFACE, TnyMsgViewIface))
#define TNY_MSG_VIEW_IFACE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_MSG_VIEW_IFACE, TnyMsgViewIfaceClass))
#define TNY_IS_MSG_VIEW_IFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_MSG_VIEW_IFACE))
#define TNY_IS_MSG_VIEW_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_MSG_VIEW_IFACE))
#define TNY_MSG_VIEW_IFACE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_MSG_VIEW_IFACE, TnyMsgViewIfaceClass))

typedef struct _TnyMsgViewIface TnyMsgViewIface;
typedef struct _TnyMsgViewIfaceClass TnyMsgViewIfaceClass;


struct _TnyMsgViewIfaceClass
{
	GTypeInterface parent;

	void (*set_msg_func) (TnyMsgViewIface *self, TnyMsgIface *msg);
};

GType         tny_msg_view_iface_get_type        (void);

void          tny_msg_view_iface_set_msg         (TnyMsgViewIface *self, TnyMsgIface *msg);


G_END_DECLS

#endif

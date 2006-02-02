#ifndef TNY_MSG_IFACE_H
#define TNY_MSG_IFACE_H

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
#include <tny-shared.h>
#include <tny-msg-header-iface.h>
#include <tny-msg-body-iface.h>
#include <tny-msg-attachment-iface.h>

G_BEGIN_DECLS

#define TNY_MSG_IFACE_TYPE             (tny_msg_iface_get_type ())
#define TNY_MSG_IFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_MSG_IFACE_TYPE, TnyMsgIface))
#define TNY_MSG_IFACE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_MSG_IFACE_TYPE, TnyMsgIfaceClass))
#define TNY_IS_MSG_IFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_MSG_IFACE_TYPE))
#define TNY_IS_MSG_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_MSG_IFACE_TYPE))
#define TNY_MSG_IFACE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_MSG_IFACE_TYPE, TnyMsgIfaceClass))

struct _TnyMsgIfaceClass
{
	GTypeInterface parent;

	const GList*                  (*get_attachments_func) (TnyMsgIface *self);
	const TnyMsgBodyIface*        (*get_body_func)        (TnyMsgIface *self);
	const TnyMsgHeaderIface*      (*get_header_func)      (TnyMsgIface *self);

	gint (*add_attachment_func)   (TnyMsgIface *self, TnyMsgAttachmentIface *attachment);
	void (*del_attachment_func)   (TnyMsgIface *self, gint id);

	void (*set_body_func)         (TnyMsgIface *self, TnyMsgBodyIface *body);
	void (*set_header_func)       (TnyMsgIface *self, TnyMsgHeaderIface *header);
};

GType tny_msg_iface_get_type        (void);

const GList*                        tny_msg_iface_get_attachments  (TnyMsgIface *self);
const TnyMsgBodyIface*              tny_msg_iface_get_body         (TnyMsgIface *self);
const TnyMsgHeaderIface*            tny_msg_iface_get_header       (TnyMsgIface *self);

/* Must also set attachment's id (therefore not a const) */
gint tny_msg_iface_add_attachment   (TnyMsgIface *self, TnyMsgAttachmentIface *attachment);
void tny_msg_iface_del_attachment   (TnyMsgIface *self, gint id);

void tny_msg_iface_set_body         (TnyMsgIface *self, TnyMsgBodyIface *body);
void tny_msg_iface_set_header       (TnyMsgIface *self, TnyMsgHeaderIface *header);

G_END_DECLS

#endif

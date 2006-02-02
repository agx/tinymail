#ifndef TNY_MSG_ATTACHMENT_IFACE_H
#define TNY_MSG_ATTACHMENT_IFACE_H

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

#define TNY_MSG_ATTACHMENT_IFACE_TYPE             (tny_msg_attachment_iface_get_type ())
#define TNY_MSG_ATTACHMENT_IFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_MSG_ATTACHMENT_IFACE_TYPE, TnyMsgAttachmentIface))
#define TNY_MSG_ATTACHMENT_IFACE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_MSG_ATTACHMENT_IFACE_TYPE, TnyMsgAttachmentIfaceClass))
#define TNY_IS_MSG_ATTACHMENT_IFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_MSG_ATTACHMENT_IFACE_TYPE))
#define TNY_IS_MSG_ATTACHMENT_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_MSG_ATTACHMENT_IFACE_TYPE))
#define TNY_MSG_ATTACHMENT_IFACE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_MSG_ATTACHMENT_IFACE_TYPE, TnyMsgAttachmentIfaceClass))

struct _TnyMsgAttachmentIfaceClass
{
	GTypeInterface parent;

	const gchar* (*get_mime_type_func)  (TnyMsgAttachmentIface *self);
	const gchar* (*get_data_func)       (TnyMsgAttachmentIface *self);
	const gint   (*get_id_func)         (TnyMsgAttachmentIface *self);

	void         (*set_mime_type_func)  (TnyMsgAttachmentIface *self, gchar *mime_type);
	void         (*set_data_func)       (TnyMsgAttachmentIface *self, gchar* data);
	void         (*set_id_func)         (TnyMsgAttachmentIface *self, gint id);
};

GType        tny_msg_attachment_iface_get_type      (void);

const gint   tny_msg_attachment_iface_get_id        (TnyMsgAttachmentIface *self);
const gchar* tny_msg_attachment_iface_get_mime_type (TnyMsgAttachmentIface *self);
const gchar* tny_msg_attachment_iface_get_data      (TnyMsgAttachmentIface *self);

void         tny_msg_attachment_iface_set_id        (TnyMsgAttachmentIface *self, gint id);
void         tny_msg_attachment_iface_set_mime_type (TnyMsgAttachmentIface *self, gchar *mime_type);
void         tny_msg_attachment_iface_set_data      (TnyMsgAttachmentIface *self, gchar *data);

G_END_DECLS

#endif

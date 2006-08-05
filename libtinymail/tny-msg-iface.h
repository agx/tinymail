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
#include <tny-header-iface.h>
#include <tny-stream-iface.h>
#include <tny-mime-part-iface.h>

G_BEGIN_DECLS

#define TNY_TYPE_MSG_IFACE             (tny_msg_iface_get_type ())
#define TNY_MSG_IFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_MSG_IFACE, TnyMsgIface))
#define TNY_MSG_IFACE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_MSG_IFACE, TnyMsgIfaceClass))
#define TNY_IS_MSG_IFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_MSG_IFACE))
#define TNY_IS_MSG_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_MSG_IFACE))
#define TNY_MSG_IFACE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_MSG_IFACE, TnyMsgIfaceClass))

struct _TnyMsgIfaceClass
{
	GTypeInterface parent;

	void               (*get_parts_func)        (TnyMsgIface *self, TnyListIface *list);
	TnyHeaderIface* (*get_header_func)       (TnyMsgIface *self);

        TnyFolderIface*
                                 (*get_folder_func)       (TnyMsgIface *self);
	void                     (*set_header_func)       (TnyMsgIface *self, TnyHeaderIface *header);

	void                     (*del_part_func)         (TnyMsgIface *self, gint id);
	gint                     (*add_part_func)         (TnyMsgIface *self, TnyMimePartIface *part);

};

GType                    tny_msg_iface_get_type         (void);

void               tny_msg_iface_get_parts        (TnyMsgIface *self, TnyListIface *list);
TnyHeaderIface* tny_msg_iface_get_header       (TnyMsgIface *self);

gint                     tny_msg_iface_add_part         (TnyMsgIface *self, TnyMimePartIface *part);
void                     tny_msg_iface_del_part         (TnyMsgIface *self, gint id);

TnyFolderIface*
                         tny_msg_iface_get_folder       (TnyMsgIface *self);
void                     tny_msg_iface_set_header       (TnyMsgIface *self, TnyHeaderIface *header);

G_END_DECLS

#endif

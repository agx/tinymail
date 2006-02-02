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

#include <tny-msg-iface.h>

const TnyMsgFolderIface* 
tny_msg_iface_get_folder (TnyMsgIface *self)
{
	return TNY_MSG_IFACE_GET_CLASS (self)->get_folder_func (self);
}


void
tny_msg_iface_set_folder (TnyMsgIface *self, const TnyMsgFolderIface *folder)
{
	TNY_MSG_IFACE_GET_CLASS (self)->set_folder_func (self, folder);
	return;
}


const GList*
tny_msg_iface_get_attachments (TnyMsgIface *self)
{

	return TNY_MSG_IFACE_GET_CLASS (self)->get_attachments_func (self);
}

const TnyMsgBodyIface*
tny_msg_iface_get_body (TnyMsgIface *self)
{
	return TNY_MSG_IFACE_GET_CLASS (self)->get_body_func (self);
}

const TnyMsgHeaderIface*
tny_msg_iface_get_header (TnyMsgIface *self)
{
	return TNY_MSG_IFACE_GET_CLASS (self)->get_header_func (self);
}

gint
tny_msg_iface_add_attachment (TnyMsgIface *self, TnyMsgAttachmentIface *attachment)
{
	return TNY_MSG_IFACE_GET_CLASS (self)->add_attachment_func (self, attachment);
}

void
tny_msg_iface_del_attachment (TnyMsgIface *self, gint id)
{
	TNY_MSG_IFACE_GET_CLASS (self)->del_attachment_func (self, id);
	return;
}

void
tny_msg_iface_set_body (TnyMsgIface *self, TnyMsgBodyIface *body)
{
	TNY_MSG_IFACE_GET_CLASS (self)->set_body_func (self, body);
	return;
}

void
tny_msg_iface_set_header (TnyMsgIface *self, TnyMsgHeaderIface *header)
{
	TNY_MSG_IFACE_GET_CLASS (self)->set_header_func (self, header);
	return;
}

static void
tny_msg_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_msg_iface_get_type (void)
{
	static GType type = 0;
	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgIfaceClass),
		  tny_msg_iface_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyMsgIface", &info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}

	return type;
}



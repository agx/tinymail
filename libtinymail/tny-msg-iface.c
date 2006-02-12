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

/**
 * tny_msg_iface_get_folder:
 * @self: a TnyMsgIface object
 * 
 * Get the parent folder of this message
 *
 * Return value: The parent folder of this message
 **/
const TnyMsgFolderIface* 
tny_msg_iface_get_folder (TnyMsgIface *self)
{
	return TNY_MSG_IFACE_GET_CLASS (self)->get_folder_func (self);
}

/**
 * tny_msg_iface_set_folder:
 * @self: a TnyMsgIface object
 * @folder: The parent folder
 * 
 * Set the parent folder of this message
 * * TODO: Make this private *
 *
 * Return value: Set parent folder of this message
 **/
void
tny_msg_iface_set_folder (TnyMsgIface *self, const TnyMsgFolderIface *folder)
{
	TNY_MSG_IFACE_GET_CLASS (self)->set_folder_func (self, folder);
	return;
}


/**
 * tny_msg_iface_get_attachments:
 * @self: a TnyMsgIface object
 * 
 * Get a read-only list of attachments of this message
 *
 * Return value: A read-only GList with TnyMsgAttachmentIface instances
 **/
const GList*
tny_msg_iface_get_attachments (TnyMsgIface *self)
{

	return TNY_MSG_IFACE_GET_CLASS (self)->get_attachments_func (self);
}

/**
 * tny_msg_iface_get_body:
 * @self: a TnyMsgIface object
 * 
 * Get the body of the message
 *
 * Return value: The body of the message
 **/
const TnyMsgBodyIface*
tny_msg_iface_get_body (TnyMsgIface *self)
{
	return TNY_MSG_IFACE_GET_CLASS (self)->get_body_func (self);
}

/**
 * tny_msg_iface_get_header:
 * @self: a TnyMsgIface object
 * 
 * Get the header of the message
 *
 * Return value: The header of the message
 **/
const TnyMsgHeaderIface*
tny_msg_iface_get_header (TnyMsgIface *self)
{
	return TNY_MSG_IFACE_GET_CLASS (self)->get_header_func (self);
}


/**
 * tny_msg_iface_add_attachment:
 * @self: a TnyMsgIface object
 * @attachment: the attachment to add
 * 
 * Add an attachment to a message
 *
 * Return value: The id of the added attachment
 **/
gint
tny_msg_iface_add_attachment (TnyMsgIface *self, TnyMsgAttachmentIface *attachment)
{
	return TNY_MSG_IFACE_GET_CLASS (self)->add_attachment_func (self, attachment);
}

/**
 * tny_msg_iface_del_attachment:
 * @self: a TnyMsgIface object
 * @id: the attachment id to delete
 * 
 * Delete an attachment from a message
 *
 * Return value: 
 **/
void
tny_msg_iface_del_attachment (TnyMsgIface *self, gint id)
{
	TNY_MSG_IFACE_GET_CLASS (self)->del_attachment_func (self, id);
	return;
}

/**
 * tny_msg_iface_set_body:
 * @self: a TnyMsgIface object
 * @body: the body to set
 * 
 * Set the body of a message
 *
 * Return value: 
 **/
void
tny_msg_iface_set_body (TnyMsgIface *self, TnyMsgBodyIface *body)
{
	TNY_MSG_IFACE_GET_CLASS (self)->set_body_func (self, body);
	return;
}

/**
 * tny_msg_iface_set_body:
 * @self: a TnyMsgIface object
 * @header: the header to set
 * 
 * Set the header of a message
 *
 * Return value: 
 **/
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



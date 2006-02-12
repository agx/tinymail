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

#include <tny-msg-attachment-iface.h>

/**
 * tny_msg_attachment_iface_get_id:
 * @self: a TnyMsgAttachmentIface object
 * 
 * Get the unique id for the attachment (unique per TnyMsgIface instance)
 * * Uncertain API * 
 * 
 * Return value: A unique id
 **/
const gint
tny_msg_attachment_iface_get_id (TnyMsgAttachmentIface *self)
{
	return TNY_MSG_ATTACHMENT_IFACE_GET_CLASS (self)->get_id_func (self);
}


/**
 * tny_msg_attachment_iface_get_mime_type:
 * @self: a TnyMsgAttachmentIface object
 * 
 * Get the MIME-type of the attachment
 * * Uncertain API * 
 *
 * Return value: A read-only string with the MIME-type
 **/
const gchar*
tny_msg_attachment_iface_get_mime_type (TnyMsgAttachmentIface *self)
{
	return TNY_MSG_ATTACHMENT_IFACE_GET_CLASS (self)->get_mime_type_func (self);
}


/**
 * tny_msg_attachment_iface_get_data:
 * @self: a TnyMsgAttachmentIface object
 * 
 * Get the data of the attachment
 * * Uncertain API *
 *
 * Return value: A read-only string with the data
 **/
const gchar*
tny_msg_attachment_iface_get_data (TnyMsgAttachmentIface *self)
{
	return TNY_MSG_ATTACHMENT_IFACE_GET_CLASS (self)->get_data_func (self);
}

void
tny_msg_attachment_iface_set_id (TnyMsgAttachmentIface *self, gint id)
{
	TNY_MSG_ATTACHMENT_IFACE_GET_CLASS (self)->set_id_func (self, id);
	return;
}

void
tny_msg_attachment_iface_set_mime_type (TnyMsgAttachmentIface *self, gchar *mime_type)
{
	TNY_MSG_ATTACHMENT_IFACE_GET_CLASS (self)->set_mime_type_func (self, mime_type);
	return;
}

void
tny_msg_attachment_iface_set_data (TnyMsgAttachmentIface *self, gchar *data)
{
	TNY_MSG_ATTACHMENT_IFACE_GET_CLASS (self)->set_data_func (self, data);
	return;
}


static void
tny_msg_attachment_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_msg_attachment_iface_get_type (void)
{
	static GType type = 0;
	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgAttachmentIfaceClass),
		  tny_msg_attachment_iface_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};

		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyMsgAttachmentIface", &info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}

	return type;
}

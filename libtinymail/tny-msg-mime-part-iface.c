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

#include <config.h>

#include <tny-msg-mime-part-iface.h>

/**
 * tny_msg_mime_part_iface_is_attachment:
 * @self: a #TnyMsgMimePartIface object
 * 
 * Return value: whether or not the mime part is an attachment
 *
 **/
gboolean 
tny_msg_mime_part_iface_is_attachment (TnyMsgMimePartIface *self)
{
	return TNY_MSG_MIME_PART_IFACE_GET_CLASS (self)->is_attachment_func (self);
}


/**
 * tny_msg_mime_part_iface_set_content_location:
 * @self: a #TnyMsgMimePartIface object
 * @content_location: the location 
 * 
 * Set the content location of a message part.
 *
 **/
void
tny_msg_mime_part_iface_set_content_location (TnyMsgMimePartIface *self, const gchar *content_location)
{
	TNY_MSG_MIME_PART_IFACE_GET_CLASS (self)->set_content_location_func (self, content_location);
	return;
}

/**
 * tny_msg_mime_part_iface_set_description:
 * @self: a #TnyMsgMimePartIface object
 * @description: the description 
 * 
 * Set the description of a message part.
 *
 **/
void
tny_msg_mime_part_iface_set_description (TnyMsgMimePartIface *self, const gchar *description)
{
	TNY_MSG_MIME_PART_IFACE_GET_CLASS (self)->set_description_func (self, description);
	return;
}

/**
 * tny_msg_mime_part_iface_set_content_id:
 * @self: a #TnyMsgMimePartIface object
 * @content_id: the content id 
 * 
 * Set the content id of a message part.
 *
 **/
void
tny_msg_mime_part_iface_set_content_id (TnyMsgMimePartIface *self, const gchar *content_id)
{
	TNY_MSG_MIME_PART_IFACE_GET_CLASS (self)->set_content_id_func (self, content_id);
	return;
}

/**
 * tny_msg_mime_part_iface_set_filename:
 * @self: a #TnyMsgMimePartIface object
 * @filename: the filename 
 * 
 * Set the filename of a message part.
 *
 **/
void
tny_msg_mime_part_iface_set_filename (TnyMsgMimePartIface *self, const gchar *filename)
{
	TNY_MSG_MIME_PART_IFACE_GET_CLASS (self)->set_filename_func (self, filename);
	return;
}

/**
 * tny_msg_mime_part_iface_set_content_type:
 * @self: a #TnyMsgMimePartIface object
 * @content_type: the content_type 
 * 
 * Set the content type of a message part. Formatted as type/subtype
 *
 **/
void
tny_msg_mime_part_iface_set_content_type (TnyMsgMimePartIface *self, const gchar *content_type)
{
	TNY_MSG_MIME_PART_IFACE_GET_CLASS (self)->set_content_type_func (self, content_type);
	return;
}


/**
 * tny_msg_mime_part_iface_get_filename:
 * @self: a #TnyMsgMimePartIface object
 * 
 *
 * Return value: the filename of a message part
 **/
const gchar*
tny_msg_mime_part_iface_get_filename (TnyMsgMimePartIface *self)
{
	return TNY_MSG_MIME_PART_IFACE_GET_CLASS (self)->get_filename_func (self);
}

/**
 * tny_msg_mime_part_iface_get_content_id:
 * @self: a #TnyMsgMimePartIface object
 * 
 *
 * Return value: the content-id of a message part
 **/
const gchar*
tny_msg_mime_part_iface_get_content_id (TnyMsgMimePartIface *self)
{
	return TNY_MSG_MIME_PART_IFACE_GET_CLASS (self)->get_content_id_func (self);
}

/**
 * tny_msg_mime_part_iface_get_description:
 * @self: a #TnyMsgMimePartIface object
 * 
 *
 * Return value: the description of a message part
 **/
const gchar*
tny_msg_mime_part_iface_get_description (TnyMsgMimePartIface *self)
{
	return TNY_MSG_MIME_PART_IFACE_GET_CLASS (self)->get_description_func (self);
}

/**
 * tny_msg_mime_part_iface_get_content_location:
 * @self: a #TnyMsgMimePartIface object
 * 
 *
 * Return value: the content-location of a message part
 **/
const gchar*
tny_msg_mime_part_iface_get_content_location (TnyMsgMimePartIface *self)
{
	return TNY_MSG_MIME_PART_IFACE_GET_CLASS (self)->get_content_location_func (self);
}

/**
 * tny_msg_mime_part_iface_write_to_stream:
 * @self: a #TnyMsgMimePartIface object
 * @stream: a #TnyMsgStreamIface stream
 * 
 * Efficiently write the message part to a stream. This will not keep the data
 * of the part in memory, but in stead will read from the part and write to the
 * stream efficiently.
 *
 **/
void
tny_msg_mime_part_iface_write_to_stream (TnyMsgMimePartIface *self, TnyStreamIface *stream)
{
	TNY_MSG_MIME_PART_IFACE_GET_CLASS (self)->write_to_stream_func (self, stream);
	return;
}


/**
 * tny_msg_mime_part_iface_decode_to_stream:
 * @self: a #TnyMsgMimePartIface object
 * @stream: a #TnyMsgStreamIface stream
 * 
 * Efficiently decode the message part to a stream. This will not keep the data
 * of the part in memory, but in stead will read from the part and write to the
 * stream efficiently.
 *
 **/
void
tny_msg_mime_part_iface_decode_to_stream (TnyMsgMimePartIface *self, TnyStreamIface *stream)
{
	TNY_MSG_MIME_PART_IFACE_GET_CLASS (self)->decode_to_stream_func (self, stream);
	return;
}
/**
 * tny_msg_mime_part_iface_construct_from_stream:
 * @self: a #TnyMsgMimePartIface object
 * @stream: a #TnyMsgStreamIface stream
 * 
 * Set the stream from which the message part will read its content
 *
 * Return value: 0 on success or -1 on fail
 **/
gint
tny_msg_mime_part_iface_construct_from_stream (TnyMsgMimePartIface *self, TnyStreamIface *stream, const gchar *type)
{
	return TNY_MSG_MIME_PART_IFACE_GET_CLASS (self)->construct_from_stream_func (self, stream, type);
}


/**
 * tny_msg_mime_part_iface_get_stream:
 * @self: a #TnyMsgMimePartIface object
 * 
 * Inefficiently get a stream from a message part. The entire data of the
 * the part will be kept in memory until the stream is unreferenced.
 *
 * Return value: An in-memory stream
 **/
TnyStreamIface* 
tny_msg_mime_part_iface_get_stream (TnyMsgMimePartIface *self)
{
	return TNY_MSG_MIME_PART_IFACE_GET_CLASS (self)->get_stream_func (self);
}

/**
 * tny_msg_mime_part_iface_get_content_type:
 * @self: a #TnyMsgMimePartIface object
 * 
 * A read-only string in the format "type/subtype". You shouldn't free this
 * value (it's internally handled). Hence it's a const.
 *
 * Return value: the read-only content-type of a message part
 **/
const gchar*
tny_msg_mime_part_iface_get_content_type (TnyMsgMimePartIface *self)
{
	return TNY_MSG_MIME_PART_IFACE_GET_CLASS (self)->get_content_type_func (self);
}

/**
 * tny_msg_mime_part_iface_content_type_is:
 * @self: a #TnyMsgMimePartIface object
 * @content_type: The content type in the string format "type/subtype"
 * 
 * Efficiently checks whether a part is of type content_type
 *
 * Return value: Whether or not the part is the content type
 **/
gboolean
tny_msg_mime_part_iface_content_type_is (TnyMsgMimePartIface *self, const gchar *content_type)
{
	return TNY_MSG_MIME_PART_IFACE_GET_CLASS (self)->content_type_is_func (self, content_type);
}



static void
tny_msg_mime_part_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_msg_mime_part_iface_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0)) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgMimePartIfaceClass),
		  tny_msg_mime_part_iface_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};

		type = g_type_register_static (G_TYPE_INTERFACE,
			"TnyMsgMimePartIface", &info, 0);

		/* g_type_interface_add_prerequisite (type, G_TYPE_OBJECT); */
	}

	return type;
}

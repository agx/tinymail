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

#include <tny-mime-part.h>


/**
 * tny_mime_part_get_parts:
 * @self: a #TnyMimePart object
 * @list: a #TnyList object
 * 
 * Get a read-only list of mime-parts in @self.
 *
 * Example:
 * <informalexample><programlisting>
 * TnyMsg *message = ...
 * TnyList *parts = tny_simple_list_new ();
 * tny_mime_part_get_parts (TNY_MIME_PART (message), parts);
 * iter = tny_list_create_iterator (parts);
 * while (!tny_iterator_is_done (iter))
 * {
 *      TnyMimePart *part = TNY_MIME_PART (tny_iterator_get_current (iter));
 *      g_object_unref (G_OBJECT (part));
 *      tny_iterator_next (iter);
 * }
 * g_object_unref (G_OBJECT (iter));
 * g_object_unref (G_OBJECT (parts));
 * </programlisting></informalexample>
 *
 **/
void
tny_mime_part_get_parts (TnyMimePart *self, TnyList *list)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_GET_IFACE (self)->get_parts_func)
		g_critical ("You must implement tny_mime_part_get_parts\n");
#endif

	TNY_MIME_PART_GET_IFACE (self)->get_parts_func (self, list);
	return;
}



/**
 * tny_mime_part_add_part:
 * @self: a #TnyMimePart object
 * @part: the mime-part to add
 * 
 * Add a mime-part to @self.
 *
 * Return value: The id of the added mime-part
 *
 **/
gint
tny_mime_part_add_part (TnyMimePart *self, TnyMimePart *part)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_GET_IFACE (self)->add_part_func)
		g_critical ("You must implement tny_mime_part_add_part\n");
#endif

	return TNY_MIME_PART_GET_IFACE (self)->add_part_func (self, part);
}

/**
 * tny_mime_part_del_part:
 * @self: a #TnyMimePart object
 * @part: the mime-part to delete
 * 
 * Delete a mime-part from @self
 *
 **/
void
tny_mime_part_del_part (TnyMimePart *self, TnyMimePart *part)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_GET_IFACE (self)->del_part_func)
		g_critical ("You must implement tny_mime_part_del_part\n");
#endif

	TNY_MIME_PART_GET_IFACE (self)->del_part_func (self, part);
	return;
}

/**
 * tny_mime_part_is_attachment:
 * @self: a #TnyMimePart object
 * 
 * Figures out whether or not a mime part is an attachment. An attachment
 * is typically something with a original filename. Examples are attached
 * files. Examples that will return FALSE are PGP signatures.
 *
 * Example:
 * <informalexample><programlisting>
 * TnyMsg *message = ...
 * TnyList *parts = tny_simple_list_new ();
 * tny_mime_part_get_parts (TNY_MIME_PART (message), parts);
 * iter = tny_list_create_iterator (parts);
 * while (!tny_iterator_is_done (iter))
 * {
 *      TnyMimePart *part = TNY_MIME_PART (tny_iterator_get_current (iter));
 *      if (tny_mime_part_is_attachment (part))
 *      {
 *          g_print ("Found an attachment (%s)\n",
 *               tny_mime_part_get_filename (part));
 *      }
 *      g_object_unref (G_OBJECT (part));
 *      tny_iterator_next (iter);
 * }
 * g_object_unref (G_OBJECT (iter));
 * g_object_unref (G_OBJECT (parts));
 * </programlisting></informalexample>
 *
 * Return value: whether or not the mime part is an attachment
 *
 **/
gboolean 
tny_mime_part_is_attachment (TnyMimePart *self)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_GET_IFACE (self)->is_attachment_func)
		g_critical ("You must implement tny_mime_part_is_attachment\n");
#endif
	return TNY_MIME_PART_GET_IFACE (self)->is_attachment_func (self);
}


/**
 * tny_mime_part_set_content_location:
 * @self: a #TnyMimePart object
 * @content_location: the location 
 * 
 * Set the content location of @self.
 *
 **/
void
tny_mime_part_set_content_location (TnyMimePart *self, const gchar *content_location)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_GET_IFACE (self)->set_content_location_func)
		g_critical ("You must implement tny_mime_part_set_content_location\n");
#endif

	TNY_MIME_PART_GET_IFACE (self)->set_content_location_func (self, content_location);
	return;
}

/**
 * tny_mime_part_set_description:
 * @self: a #TnyMimePart object
 * @description: the description 
 * 
 * Set the description of @self.
 *
 **/
void
tny_mime_part_set_description (TnyMimePart *self, const gchar *description)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_GET_IFACE (self)->set_description_func)
		g_critical ("You must implement tny_mime_part_set_description\n");
#endif

	TNY_MIME_PART_GET_IFACE (self)->set_description_func (self, description);
	return;
}

/**
 * tny_mime_part_set_content_id:
 * @self: a #TnyMimePart object
 * @content_id: the content id 
 * 
 * Set the content id of @self.
 *
 **/
void
tny_mime_part_set_content_id (TnyMimePart *self, const gchar *content_id)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_GET_IFACE (self)->set_content_id_func)
		g_critical ("You must implement tny_mime_part_set_content_id\n");
#endif

	TNY_MIME_PART_GET_IFACE (self)->set_content_id_func (self, content_id);
	return;
}

/**
 * tny_mime_part_set_filename:
 * @self: a #TnyMimePart object
 * @filename: the filename 
 * 
 * Set the filename of @self.
 *
 **/
void
tny_mime_part_set_filename (TnyMimePart *self, const gchar *filename)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_GET_IFACE (self)->set_filename_func)
		g_critical ("You must implement tny_mime_part_set_filename\n");
#endif

	TNY_MIME_PART_GET_IFACE (self)->set_filename_func (self, filename);
	return;
}

/**
 * tny_mime_part_set_content_type:
 * @self: a #TnyMimePart object
 * @contenttype: the content_type 
 * 
 * Set the content type of @self. Formatted as "type/subtype"
 *
 **/
void
tny_mime_part_set_content_type (TnyMimePart *self, const gchar *contenttype)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_GET_IFACE (self)->set_content_type_func)
		g_critical ("You must implement tny_mime_part_set_content_type\n");
#endif

	TNY_MIME_PART_GET_IFACE (self)->set_content_type_func (self, contenttype);
	return;
}


/**
 * tny_mime_part_get_filename:
 * @self: a #TnyMimePart object
 * 
 * Get the filename of @self. The returned value should not be freed.
 *
 * Return value: the filename of a mime part as a read-only string
 *
 **/
const gchar*
tny_mime_part_get_filename (TnyMimePart *self)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_GET_IFACE (self)->get_filename_func)
		g_critical ("You must implement tny_mime_part_get_filename\n");
#endif

	return TNY_MIME_PART_GET_IFACE (self)->get_filename_func (self);
}

/**
 * tny_mime_part_get_content_id:
 * @self: a #TnyMimePart object
 * 
 * Get the content-id of @self. The returned value should not be freed.
 *
 * Return value: the content-id of a mime part as a read-only string
 *
 **/
const gchar*
tny_mime_part_get_content_id (TnyMimePart *self)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_GET_IFACE (self)->get_content_id_func)
		g_critical ("You must implement tny_mime_part_get_content_id\n");
#endif

	return TNY_MIME_PART_GET_IFACE (self)->get_content_id_func (self);
}

/**
 * tny_mime_part_get_description:
 * @self: a #TnyMimePart object
 * 
 * Get the description of @self. The returned value should not be freed.
 *
 * Return value: the description of a mime part as a read-only string
 **/
const gchar*
tny_mime_part_get_description (TnyMimePart *self)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_GET_IFACE (self)->get_description_func)
		g_critical ("You must implement tny_mime_part_get_description\n");
#endif

	return TNY_MIME_PART_GET_IFACE (self)->get_description_func (self);
}

/**
 * tny_mime_part_get_content_location:
 * @self: a #TnyMimePart object
 * 
 * Get the content location of @self. The returned value should not be freed.
 *
 * Return value: the content-location of a mime part as a read-only string
 *
 **/
const gchar*
tny_mime_part_get_content_location (TnyMimePart *self)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_GET_IFACE (self)->get_content_location_func)
		g_critical ("You must implement tny_mime_part_get_content_location\n");
#endif

	return TNY_MIME_PART_GET_IFACE (self)->get_content_location_func (self);
}

/**
 * tny_mime_part_write_to_stream:
 * @self: a #TnyMimePart object
 * @stream: a #TnyMsgStream stream
 * 
 * Efficiently write the content of @self to a stream. This will not read the 
 * data of the part in a memory buffer. In stead it will read the part data while
 * already writing it to the stream efficiently.
 *
 * You probably want to utilise the tny_mime_part_decode_to_stream
 * method in stead of this one. This method will not attempt to decode the
 * mime part. Mime parts are usually encoded in E-mails.
 * 
 * Example:
 * <informalexample><programlisting>
 * int fd = open ("/tmp/attachment.png.base64enc", O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
 * TnyMimePart *part = ...
 * if (fd != -1)
 * {
 *      TnyFsStream *stream = tny_fs_stream_new (fd);
 *      tny_mime_part_write_to_stream (part, TNY_STREAM (stream));
 *      g_object_unref (G_OBJECT (stream));
 * }
 * </programlisting></informalexample>
 **/
void
tny_mime_part_write_to_stream (TnyMimePart *self, TnyStream *stream)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_GET_IFACE (self)->write_to_stream_func)
		g_critical ("You must implement tny_mime_part_write_to_stream\n");
#endif

	TNY_MIME_PART_GET_IFACE (self)->write_to_stream_func (self, stream);
	return;
}


/**
 * tny_mime_part_decode_to_stream:
 * @self: a #TnyMimePart object
 * @stream: a #TnyMsgStream stream
 * 
 * Efficiently decode @self to a stream. This will not read the data of the
 * part in a memory buffer. In stead it will read the part data while already
 * writing it to the stream efficiently.
 *
 * Example:
 * <informalexample><programlisting>
 * int fd = open ("/tmp/attachment.png", O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
 * TnyMimePart *part = ...
 * if (fd != -1)
 * {
 *      TnyFsStream *stream = tny_fs_stream_new (fd);
 *      tny_mime_part_decode_to_stream (part, TNY_STREAM (stream));
 *      g_object_unref (G_OBJECT (stream));
 * }
 * </programlisting></informalexample>
 *
 **/
void
tny_mime_part_decode_to_stream (TnyMimePart *self, TnyStream *stream)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_GET_IFACE (self)->decode_to_stream_func)
		g_critical ("You must implement tny_mime_part_decode_to_stream\n");
#endif

	TNY_MIME_PART_GET_IFACE (self)->decode_to_stream_func (self, stream);
	return;
}
/**
 * tny_mime_part_construct_from_stream:
 * @self: a #TnyMimePart object
 * @stream: a #TnyMsgStream stream
 * @type: the type like text/plain
 * 
 * Set the stream from which the mime part will read its content
 *
 * Example:
 * <informalexample><programlisting>
 * int fd = open ("/tmp/attachment.png", ...);
 * TnyMimePart *part = ...
 * if (fd != -1)
 * {
 *      TnyFsStream *stream = tny_fs_stream_new (fd);
 *      tny_mime_part_construct_from_stream (part, TNY_STREAM (stream));
 * }
 * </programlisting></informalexample>
 *
 * Return value: 0 on success or -1 on failure
 **/
gint
tny_mime_part_construct_from_stream (TnyMimePart *self, TnyStream *stream, const gchar *type)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_GET_IFACE (self)->construct_from_stream_func)
		g_critical ("You must implement tny_mime_part_construct_from_stream\n");
#endif

	return TNY_MIME_PART_GET_IFACE (self)->construct_from_stream_func (self, stream, type);
}


/**
 * tny_mime_part_get_stream:
 * @self: a #TnyMimePart object
 * 
 * Inefficiently get a stream for @self. The entire data of the part will be
 * kept in memory until the stream is unreferenced.
 *
 * Return value: An in-memory stream
 **/
TnyStream* 
tny_mime_part_get_stream (TnyMimePart *self)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_GET_IFACE (self)->get_stream_func)
		g_critical ("You must implement tny_mime_part_get_stream\n");
#endif

	return TNY_MIME_PART_GET_IFACE (self)->get_stream_func (self);
}

/**
 * tny_mime_part_get_content_type:
 * @self: a #TnyMimePart object
 * 
 * Get the mime part type in the format "type/subtype".  You shouldn't free the 
 * returned value.
 *
 * Return value: content-type of a message part as a read-only string
 **/
const gchar*
tny_mime_part_get_content_type (TnyMimePart *self)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_GET_IFACE (self)->get_content_type_func)
		g_critical ("You must implement tny_mime_part_get_content_type\n");
#endif

	return TNY_MIME_PART_GET_IFACE (self)->get_content_type_func (self);
}

/**
 * tny_mime_part_content_type_is:
 * @self: a #TnyMimePart object
 * @type: The content type in the string format type/subtype
 * 
 * Efficiently checks whether @self is of type @type. You can use things
 * like "type/*" for matching. Only '*' works and stands for 'any'. It's not 
 * a regular expression nor is it like a regular expression.
 *
 * Example:
 * <informalexample><programlisting>
 * TnyMsg *message = ...
 * TnyList *parts = tny_simple_list_new ();
 * tny_mime_part_get_parts (TNY_MIME_PART (message), parts);
 * iter = tny_list_create_iterator (parts);
 * while (!tny_iterator_is_done (iter))
 * {
 *      TnyMimePart *part = TNY_MIME_PART (tny_iterator_get_current (iter));
 *      if (tny_mime_part_content_type_is (part, "text/*"))
 *              g_print ("Found an E-mail body\n");
 *      if (tny_mime_part_content_type_is (part, "text/html"))
 *              g_print ("Found an E-mail HTML body\n"); 
 *      g_object_unref (G_OBJECT (part));
 *      tny_iterator_next (iter);
 * }
 * g_object_unref (G_OBJECT (iter));
 * g_object_unref (G_OBJECT (parts));
 * </programlisting></informalexample>
 *
 * Return value: Whether or not the part is the content type
 *
 **/
gboolean
tny_mime_part_content_type_is (TnyMimePart *self, const gchar *type)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_GET_IFACE (self)->content_type_is_func)
		g_critical ("You must implement tny_mime_part_content_type_is\n");
#endif

	return TNY_MIME_PART_GET_IFACE (self)->content_type_is_func (self, type);
}



static void
tny_mime_part_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_mime_part_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0)) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMimePartIface),
		  tny_mime_part_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL,   /* instance_init */
		  NULL
		};
		type = g_type_register_static (G_TYPE_INTERFACE,
			"TnyMimePart", &info, 0);
	}

	return type;
}

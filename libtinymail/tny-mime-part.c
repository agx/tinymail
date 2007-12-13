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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <config.h>

#ifdef DBC
#include <string.h>
#endif

#include <tny-mime-part.h>
#include <tny-list.h>

/* TNY TODO: Check MIME RFC to evaluate whether NULL string values should be
 * allowed and strengthen contracts if not */


/**
 * tny_mime_part_set_header_pair:
 * @self: a #TnyMimePart object
 * @name: the name of the header
 * @value: the value of the header or NULL to unset
 * 
 * Set a header pair (name: value) or delete a header (use NULL as value).
 *
 * Note that not all TnyMimePart instances are writable. Only when creating
 * a new message will the instance be guaranteed to be writable. This is a
 * writing operation.
 *
 * Example:
 * <informalexample><programlisting>
 * TnyMsg *message = ...
 * tny_mime_part_set_header_pair (TNY_MIME_PART (message),
 *        "X-MS-Has-Attach", "yes");
 * </programlisting></informalexample>
 **/
void 
tny_mime_part_set_header_pair (TnyMimePart *self, const gchar *name, const gchar *value)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (name);
	g_assert (strlen (name) > 0);
	g_assert (TNY_MIME_PART_GET_IFACE (self)->set_header_pair_func != NULL);
#endif

	TNY_MIME_PART_GET_IFACE (self)->set_header_pair_func (self, name, value);
	return;
}

/**
 * tny_mime_part_get_header_pairs:
 * @self: a #TnyMimePart object
 * @list: a #TnyList object
 * 
 * Get a read-only list of header pairs (#TnyPair) in @self.
 *
 * Example:
 * <informalexample><programlisting>
 * TnyMsg *message = ...
 * TnyList *pairs = tny_simple_list_new ();
 * tny_mime_part_get_header_pairs (TNY_MIME_PART (message), pairs);
 * iter = tny_list_create_iterator (pairs);
 * while (!tny_iterator_is_done (iter))
 * {
 *      TnyPair *pair = TNY_PAIR (tny_iterator_get_current (iter));
 *      g_print (%s: %s", tny_pair_get_name (pair), 
 *           tny_pair_get_value (pair));
 *      g_object_unref (G_OBJECT (pair));
 *      tny_iterator_next (iter);
 * }
 * g_object_unref (G_OBJECT (iter));
 * g_object_unref (G_OBJECT (pairs));
 * </programlisting></informalexample>
 *
 **/
void 
tny_mime_part_get_header_pairs (TnyMimePart *self, TnyList *list)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (list);
	g_assert (TNY_IS_LIST (list));
	g_assert (TNY_MIME_PART_GET_IFACE (self)->get_header_pairs_func != NULL);
#endif

	TNY_MIME_PART_GET_IFACE (self)->get_header_pairs_func (self, list);
	return;
}


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
#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (list);
	g_assert (TNY_IS_LIST (list));
	g_assert (TNY_MIME_PART_GET_IFACE (self)->get_parts_func != NULL);
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
 * Note that not all TnyMimePart instances are writable. Only when creating
 * a new message will the instance be guaranteed to be writable. This is a
 * writing operation.
 *
 * Return value: The id of the added mime-part
 **/
gint
tny_mime_part_add_part (TnyMimePart *self, TnyMimePart *part)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (part);
	g_assert (TNY_IS_MIME_PART (part));
	g_assert (TNY_MIME_PART_GET_IFACE (self)->add_part_func != NULL);
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
 * Note that not all TnyMimePart instances are writable. Only when creating
 * a new message will the instance be guaranteed to be writable. This is a
 * writing operation.
 **/
void
tny_mime_part_del_part (TnyMimePart *self, TnyMimePart *part)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (part);
	g_assert (TNY_IS_MIME_PART (part));
	g_assert (TNY_MIME_PART_GET_IFACE (self)->del_part_func != NULL);
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
#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (TNY_MIME_PART_GET_IFACE (self)->is_attachment_func != NULL);
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
 * Note that not all TnyMimePart instances are writable. Only when creating
 * a new message will the instance be guaranteed to be writable. This is a
 * writing operation.
 **/
void
tny_mime_part_set_content_location (TnyMimePart *self, const gchar *content_location)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (!content_location || strlen (content_location) > 0);
	g_assert (TNY_MIME_PART_GET_IFACE (self)->set_content_location_func != NULL);
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
 * Note that not all TnyMimePart instances are writable. Only when creating
 * a new message will the instance be guaranteed to be writable. This is a
 * writing operation.
 **/
void
tny_mime_part_set_description (TnyMimePart *self, const gchar *description)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (!description || strlen (description) > 0);
	g_assert (TNY_MIME_PART_GET_IFACE (self)->set_description_func != NULL);
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
 * Note that not all TnyMimePart instances are writable. Only when creating
 * a new message will the instance be guaranteed to be writable. This is a
 * writing operation.
 **/
void
tny_mime_part_set_content_id (TnyMimePart *self, const gchar *content_id)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (!content_id || strlen (content_id) > 0);
	g_assert (TNY_MIME_PART_GET_IFACE (self)->set_content_id_func != NULL);
#endif

	TNY_MIME_PART_GET_IFACE (self)->set_content_id_func (self, content_id);
	return;
}

/**
 * tny_mime_part_set_purged:
 * @self: a #TnyMimePart object
 * 
 * Set the message as purged in cache
 *
 * This is not a writing operation. Although it might change the content of a
 * message for a user who's not connected with the server where @self originates
 * from.
 *
 * Using the tny_msg_rewrite_cache API on a message instance will rewrite its 
 * purged mime parts with an empty body (saving storage space). The storage 
 * space is camel_recovered after using tny_msg_rewrite_cache. Only setting a mime 
 * part to purged might not remove it.
 *
 * There is no guarantee about what happens with a purged mime part internally 
 * (it might get destroyed or become unuseful more early than the call to
 * tny_msg_rewrite_cache).
 **/
void
tny_mime_part_set_purged (TnyMimePart *self)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (TNY_MIME_PART_GET_IFACE (self)->set_purged_func != NULL);
#endif

	TNY_MIME_PART_GET_IFACE (self)->set_purged_func (self);
	return;
}

/**
 * tny_mime_part_set_filename:
 * @self: a #TnyMimePart object
 * @filename: the filename 
 * 
 * Set the filename of @self.
 *
 * Note that not all TnyMimePart instances are writable. Only when creating
 * a new message will the instance be guaranteed to be writable. This is a
 * writing operation.
 **/
void
tny_mime_part_set_filename (TnyMimePart *self, const gchar *filename)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (!filename || strlen (filename) > 0);
	g_assert (TNY_MIME_PART_GET_IFACE (self)->set_filename_func != NULL);
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
 * Note that not all TnyMimePart instances are writable. Only when creating
 * a new message will the instance be guaranteed to be writable. This is a
 * writing operation.
 **/
void
tny_mime_part_set_content_type (TnyMimePart *self, const gchar *contenttype)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (!contenttype || strlen (contenttype) > 0);
	g_assert (TNY_MIME_PART_GET_IFACE (self)->set_content_type_func != NULL);
#endif

	TNY_MIME_PART_GET_IFACE (self)->set_content_type_func (self, contenttype);
	return;
}


/**
 * tny_mime_part_get_filename:
 * @self: a #TnyMimePart object
 * 
 * Get the filename of @self if it's an attachment or NULL otherwise. The
 * returned value should not be freed.
 *
 * Return value: the filename of a mime part as a read-only string
 **/
const gchar*
tny_mime_part_get_filename (TnyMimePart *self)
{
	const gchar *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (TNY_MIME_PART_GET_IFACE (self)->get_filename_func != NULL);
#endif

	retval = TNY_MIME_PART_GET_IFACE (self)->get_filename_func (self);

#ifdef DBC /* ensure */
	g_assert (retval == NULL || strlen (retval) > 0);
#endif

	return retval;
}

/**
 * tny_mime_part_get_content_id:
 * @self: a #TnyMimePart object
 * 
 * Get the content-id of @self. The returned value should not be freed.
 *
 * Return value: the content-id of a mime part as a read-only string
 **/
const gchar*
tny_mime_part_get_content_id (TnyMimePart *self)
{
	const gchar *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (TNY_MIME_PART_GET_IFACE (self)->get_content_id_func != NULL);
#endif

	retval = TNY_MIME_PART_GET_IFACE (self)->get_content_id_func (self);

#ifdef DBC /* ensure */
	g_assert (retval == NULL || strlen (retval) > 0);
#endif

	return retval;
}

/**
 * tny_mime_part_is_purged:
 * @self: a #TnyMimePart object
 * 
 * Get if this attachment has been purged from cache.
 *
 * Return value: a #gboolean
 **/
gboolean
tny_mime_part_is_purged (TnyMimePart *self)
{
	gboolean retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (TNY_MIME_PART_GET_IFACE (self)->is_purged_func != NULL);
#endif

	retval = TNY_MIME_PART_GET_IFACE (self)->is_purged_func (self);

	return retval;
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
	const gchar *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (TNY_MIME_PART_GET_IFACE (self)->get_description_func != NULL);
#endif

	retval = TNY_MIME_PART_GET_IFACE (self)->get_description_func (self);

#ifdef DBC /* ensure */
	g_assert (retval == NULL || strlen (retval) > 0);
#endif

	return retval;
}

/**
 * tny_mime_part_get_content_location:
 * @self: a #TnyMimePart object
 * 
 * Get the content location of @self. The returned value should not be freed.
 *
 * Return value: the content-location of a mime part as a read-only string
 **/
const gchar*
tny_mime_part_get_content_location (TnyMimePart *self)
{
	const gchar *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (TNY_MIME_PART_GET_IFACE (self)->get_content_location_func != NULL);
#endif

	retval = TNY_MIME_PART_GET_IFACE (self)->get_content_location_func (self);

#ifdef DBC /* ensure */
	g_assert (retval == NULL || strlen (retval) > 0);
#endif

	return retval;
}

/**
 * tny_mime_part_write_to_stream:
 * @self: a #TnyMimePart object
 * @stream: a #TnyMsgStream stream
 * 
 * Efficiently write the content of @self to a stream. This will not read the 
 * data of the part in a memory buffer. In stead it will read the part data while
 * already writing it to the stream efficiently. Although there is no guarantee
 * about the memory usage either (just that it's the most efficient way).
 *
 * You probably want to utilise the tny_mime_part_decode_to_stream
 * method in stead of this one. This method will not attempt to decode the
 * mime part. Mime parts are usually encoded in E-mails.
 *
 * When the mime part was received in BINARY mode from an IMAP server, then this
 * API has mostly the same effect as the tny_mime_part_decode_to_stream: You 
 * will get a non-encoded version of the data. A small difference will be that
 * the tny_mime_part_decode_to_stream will decode certain special characters in
 * TEXT/* mime parts (character set encoding) to UTF-8.
 *
 * However. A larger difference happens with binary mime parts that where not
 * retrieved using BINARY. For those this API will give you the encoded data
 * as is. This means that you will get a stream spitting out for example BASE64
 * data.
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
#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (stream);
	g_assert (TNY_IS_STREAM (stream));
	g_assert (TNY_MIME_PART_GET_IFACE (self)->write_to_stream_func != NULL);
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
 * writing it to the stream efficiently. Although there is no guarantee
 * about the memory usage either (just that it's the most efficient way).
 *
 * You will always get the decoded version of the data of @self. When your part
 * get received in BINARY from an IMAP server, then nothing will really happen
 * with your data (it will be streamed to you the way it got received). If we
 * received using BODY and the data is encoded in a known encoding (BASE65,
 * QUOTED-PRINTED, UUENCODED), the data will be decoded before delivered. TEXT/*
 * mime parts will also enjoy character set decoding.
 *
 * In short will this API prepare the cookie and deliver it to your stream. It's
 * most likely the one that you want to use. If you are planning to nevertheless
 * use the tny_mime_part_write_to_stream API, then please know and understand
 * what you are doing.
 *
 * It's possible that this API receives information from the service. If you 
 * don't want to block on this, use tny_mime_part_decode_to_stream_async.
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
#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (stream);
	g_assert (TNY_IS_STREAM (stream));
	g_assert (TNY_MIME_PART_GET_IFACE (self)->decode_to_stream_func != NULL);
#endif

	TNY_MIME_PART_GET_IFACE (self)->decode_to_stream_func (self, stream);
	return;
}

/**
 * tny_mime_part_decode_to_stream_async:
 * @self: a #TnyMimePart object
 * @stream: a #TnyMsgStream stream
 * @callback: a #TnyMimePartCallback callback
 * @status_callback: a #TnyStatusCallback callback
 * @user_data: user data for @callback
 *
 * This method does the same as tny_mime_part_decode_to_stream. It just does 
 * everything asynchronous and calls you back when finished.
 */
void
tny_mime_part_decode_to_stream_async (TnyMimePart *self, TnyStream *stream, TnyMimePartCallback callback, TnyStatusCallback status_callback, gpointer user_data)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (stream);
	g_assert (TNY_IS_STREAM (stream));
	g_assert (TNY_MIME_PART_GET_IFACE (self)->decode_to_stream_async_func != NULL);
#endif

	TNY_MIME_PART_GET_IFACE (self)->decode_to_stream_async_func (self, stream, callback, status_callback, user_data);
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
	gint retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (TNY_IS_STREAM (stream));
	g_assert (TNY_MIME_PART_GET_IFACE (self)->construct_from_stream_func != NULL);
#endif

	retval = TNY_MIME_PART_GET_IFACE (self)->construct_from_stream_func (self, stream, type);

#ifdef DBC /* ensure */
	g_assert (retval == 0 || retval == -1);
#endif

	return retval;
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
	TnyStream *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (TNY_MIME_PART_GET_IFACE (self)->get_stream_func != NULL);
#endif

	retval = TNY_MIME_PART_GET_IFACE (self)->get_stream_func (self);

#ifdef DBC /* ensure */
	g_assert (TNY_IS_STREAM (retval));
#endif

	return retval;
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
	const gchar *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (TNY_MIME_PART_GET_IFACE (self)->get_content_type_func != NULL);
#endif

	retval = TNY_MIME_PART_GET_IFACE (self)->get_content_type_func (self);

#ifdef DBC /* ensure */
	g_assert (retval == NULL || strlen (retval) > 0);
#endif

	return retval;
}

/**
 * tny_mime_part_content_type_is:
 * @self: a #TnyMimePart object
 * @type: The content type in the string format type/subtype
 * 
 * Efficiently checks whether @self is of type @type. You can use things
 * like "type/ *" for matching. Only '*' works and stands for 'any'. It's not 
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
 *      if (tny_mime_part_content_type_is (part, "text/ *"))
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
#ifdef DBC /* require */
	g_assert (TNY_IS_MIME_PART (self));
	g_assert (type == NULL || strlen (type) > 0);
	g_assert (TNY_MIME_PART_GET_IFACE (self)->content_type_is_func != NULL);
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

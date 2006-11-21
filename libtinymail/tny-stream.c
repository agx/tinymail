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

#include <tny-stream.h>


/**
 * tny_stream_write_to_stream:
 * @self: a #TnyStream object
 * @output: a #TnyStream object
 * 
 * Write @self to @output in an efficient way
 *
 * Return value: the number of bytes written to the output stream, or -1 on
 * error along with setting errno.
 *
 **/
gssize
tny_stream_write_to_stream (TnyStream *self, TnyStream *output)
{
#ifdef DEBUG
	if (!TNY_STREAM_GET_IFACE (self)->write_to_stream_func)
		g_critical ("You must implement tny_stream_write_to_stream\n");
#endif

	return TNY_STREAM_GET_IFACE (self)->write_to_stream_func (self, output);
}

/**
 * tny_stream_read:
 * @self: a #TnyStream object
 * @buffer: a buffer that is at least n in size
 * @n: the max amount of bytes to read from self and to write into buffer
 * 
 * Read n bytes @self and write it into @buffer. It's your responsibility
 * to pass a buffer that is large enough to hold n bytes.
 *
 * Return value: the number of bytes actually read, or -1 on error and 
 * set errno.
 *
 **/
gssize
tny_stream_read  (TnyStream *self, char *buffer, gsize n)
{
#ifdef DEBUG
	if (!TNY_STREAM_GET_IFACE (self)->read_func)
		g_critical ("You must implement tny_stream_read\n");
#endif

	return TNY_STREAM_GET_IFACE (self)->read_func (self, buffer, n);
}

/**
 * tny_stream_write:
 * @self: a #TnyStream object
 * @buffer: a buffer that has at least n bytes
 * @n: the amount of bytes to read from buffer and to write to self
 * 
 * Write n bytes of @buffer into @self. It's your responsibility to pass
 * a buffer that has at least n bytes.
 *
 * Return value: the number of bytes written to the stream, or -1 on error 
 * along with setting errno.
 *
 **/
gssize
tny_stream_write (TnyStream *self, const char *buffer, gsize n)
{
#ifdef DEBUG
	if (!TNY_STREAM_GET_IFACE (self)->write_func)
		g_critical ("You must implement tny_stream_write\n");
#endif

	return TNY_STREAM_GET_IFACE (self)->write_func (self, buffer, n);
}


/**
 * tny_stream_flush:
 * @self: a #TnyStream object
 * 
 * Flushes any buffered data to the stream's backing store. 
 *
 * Return value: 0 on success or -1 on fail along with setting errno.
 *
 **/
gint
tny_stream_flush (TnyStream *self)
{
#ifdef DEBUG
	if (!TNY_STREAM_GET_IFACE (self)->flush_func)
		g_critical ("You must implement tny_stream_flush\n");
#endif

	return TNY_STREAM_GET_IFACE (self)->flush_func (self);
}

/**
 * tny_stream_close:
 * @self: a #TnyStream object
 * 
 * Closes the stream
 *
 * Return value: 0 on success or -1 on fail.
 **/
gint
tny_stream_close (TnyStream *self)
{
#ifdef DEBUG
	if (!TNY_STREAM_GET_IFACE (self)->close_func)
		g_critical ("You must implement tny_stream_close\n");
#endif

	return TNY_STREAM_GET_IFACE (self)->close_func (self);
}


/**
 * tny_stream_is_eos:
 * @self: a #TnyStream object
 * 
 * Tests if there are bytes left to read on @self.
 *
 * Return value: TRUE on EOS or FALSE otherwise.
 *
 **/
gboolean
tny_stream_is_eos   (TnyStream *self)
{
#ifdef DEBUG
	if (!TNY_STREAM_GET_IFACE (self)->is_eos_func)
		g_critical ("You must implement tny_stream_is_eos\n");
#endif

	return TNY_STREAM_GET_IFACE (self)->is_eos_func (self);
}

/**
 * tny_stream_reset:
 * @self: a #TnyStream object
 * 
 * Resets @self. That is, put it in a state where it can be read from 
 * the beginning again.
 *
 * Return value: 0 on success or -1 on error along with setting errno.
 *
 **/
gint
tny_stream_reset (TnyStream *self)
{
#ifdef DEBUG
	if (!TNY_STREAM_GET_IFACE (self)->reset_func)
		g_critical ("You must implement tny_stream_reset\n");
#endif

	return TNY_STREAM_GET_IFACE (self)->reset_func (self);
}

static void
tny_stream_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_stream_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyStreamIface),
		  tny_stream_base_init,   /* base_init */
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
			"TnyStream", &info, 0);
	}

	return type;
}



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

#include <tny-stream-iface.h>


/**
 * tny_stream_iface_write_to_stream:
 * @self: a #TnyStreamIface object
 * @output: a #TnyStreamIface object
 * 
 * Write self to output (copy it) in an efficient way
 *
 * Return value: the number of bytes written to the output stream, or -1 on
 * error along with setting errno.
 **/
ssize_t
tny_stream_iface_write_to_stream (TnyStreamIface *self, TnyStreamIface *output)
{
#ifdef DEBUG
	if (!TNY_STREAM_IFACE_GET_CLASS (self)->write_to_stream_func)
		g_critical ("You must implement tny_stream_iface_write_to_stream\n");
#endif

	return TNY_STREAM_IFACE_GET_CLASS (self)->write_to_stream_func (self, output);
}

/**
 * tny_stream_iface_read:
 * @self: a #TnyStreamIface object
 * @buffer: a buffer that is at least n in size
 * @n: the max amount of bytes to read into buffer
 * 
 * Read n bytes of the stream into buffer
 *
 * Return value: the number of bytes actually read, or -1 on error and 
 * set errno.
 **/
ssize_t
tny_stream_iface_read  (TnyStreamIface *self, char *buffer, size_t n)
{
#ifdef DEBUG
	if (!TNY_STREAM_IFACE_GET_CLASS (self)->read_func)
		g_critical ("You must implement tny_stream_iface_read\n");
#endif

	return TNY_STREAM_IFACE_GET_CLASS (self)->read_func (self, buffer, n);
}

/**
 * tny_stream_iface_write:
 * @self: a #TnyStreamIface object
 * @buffer: a buffer that has at least n bytes
 * @n: the amount of bytes to read from buffer and to write
 * 
 * Write n bytes of the buffer into the stream
 *
 * Return value: the number of bytes written to the stream, or -1 on error 
 * along with setting errno.
 **/
ssize_t
tny_stream_iface_write (TnyStreamIface *self, const char *buffer, size_t n)
{
#ifdef DEBUG
	if (!TNY_STREAM_IFACE_GET_CLASS (self)->write_func)
		g_critical ("You must implement tny_stream_iface_write\n");
#endif

	return TNY_STREAM_IFACE_GET_CLASS (self)->write_func (self, buffer, n);
}


/**
 * tny_stream_iface_flush:
 * @self: a #TnyStreamIface object
 * 
 * Flushes any buffered data to the stream's backing store. 
 * Only meaningful for writable streams.
 *
 * Return value: 0 on success or -1 on fail along with setting errno.
 **/
gint
tny_stream_iface_flush (TnyStreamIface *self)
{
#ifdef DEBUG
	if (!TNY_STREAM_IFACE_GET_CLASS (self)->flush_func)
		g_critical ("You must implement tny_stream_iface_flush\n");
#endif

	return TNY_STREAM_IFACE_GET_CLASS (self)->flush_func (self);
}

/**
 * tny_stream_iface_close:
 * @self: a #TnyStreamIface object
 * 
 * Closes the stream
 *
 * Return value: 0 on success or -1 on fail.
 **/
gint
tny_stream_iface_close (TnyStreamIface *self)
{
#ifdef DEBUG
	if (!TNY_STREAM_IFACE_GET_CLASS (self)->close_func)
		g_critical ("You must implement tny_stream_iface_close\n");
#endif

	return TNY_STREAM_IFACE_GET_CLASS (self)->close_func (self);
}


/**
 * tny_stream_iface_eos:
 * @self: a #TnyStreamIface object
 * 
 * Tests if there are bytes left to read on the stream object.
 *
 * Return value: TRUE on EOS or FALSE otherwise.
 **/
gboolean
tny_stream_iface_eos   (TnyStreamIface *self)
{
#ifdef DEBUG
	if (!TNY_STREAM_IFACE_GET_CLASS (self)->eos_func)
		g_critical ("You must implement tny_stream_iface_eos\n");
#endif

	return TNY_STREAM_IFACE_GET_CLASS (self)->eos_func (self);
}

/**
 * tny_stream_iface_reset:
 * @self: a #TnyStreamIface object
 * 
 * Resets the stream. That is, put it in a state where it can be read from 
 * the beginning again. Not all streams are seekable, but they must 
 * all be resettable.
 *
 * Return value: 0 on success or -1 on error along with setting errno.
 **/
gint
tny_stream_iface_reset (TnyStreamIface *self)
{
#ifdef DEBUG
	if (!TNY_STREAM_IFACE_GET_CLASS (self)->reset_func)
		g_critical ("You must implement tny_stream_iface_reset\n");
#endif

	return TNY_STREAM_IFACE_GET_CLASS (self)->reset_func (self);
}

static void
tny_stream_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_stream_iface_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyStreamIfaceClass),
		  tny_stream_iface_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyStreamIface", &info, 0);

		/* g_type_interface_add_prerequisite (type, G_TYPE_OBJECT); */
	}

	return type;
}



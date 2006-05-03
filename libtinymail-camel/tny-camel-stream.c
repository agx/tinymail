/* libtinymail-camel - The Tiny Mail base library for Camel
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

#include <tny-camel-stream.h>

#include <tny-camel-shared.h>

static CamelStreamClass *parent_class = NULL;

/**
 * tny_camel_stream_write_to_stream:
 * @self: a #TnyCamelStream object
 * @output: a #TnyStreamIface object to write to
 * 
 * Write self to output (copy it) in an efficient way
 *
 * Return value: the number of bytes written to the output stream, or -1 on 
 * error along with setting errno.
 **/
ssize_t 
tny_camel_stream_write_to_stream (TnyCamelStream *self, TnyStreamIface *output)
{
	CamelStream *stream = CAMEL_STREAM (self);
	char tmp_buf[4096];
	ssize_t total = 0;
	ssize_t nb_read;
	ssize_t nb_written;
	g_return_val_if_fail (CAMEL_IS_STREAM (stream), -1);
	g_return_val_if_fail (TNY_IS_STREAM_IFACE (output), -1);

	while (!camel_stream_eos (stream)) {
		nb_read = camel_stream_read (stream, tmp_buf, sizeof (tmp_buf));
		if (nb_read < 0)
			return -1;
		else if (nb_read > 0) {
			nb_written = 0;
	
			while (nb_written < nb_read) {
				ssize_t len = tny_stream_iface_write (output, tmp_buf + nb_written,
								  nb_read - nb_written);
				if (len < 0)
					return -1;
				nb_written += len;
			}
			total += nb_written;
		}
	}
	return total;
}


static ssize_t
tny_camel_stream_read (CamelStream *stream, char *buffer, size_t n)
{
	TnyCamelStream *self = (TnyCamelStream *)stream;
	return tny_stream_iface_read (self->stream, buffer, n);
}

static ssize_t
tny_camel_stream_write (CamelStream *stream, const char *buffer, size_t n)
{
	TnyCamelStream *self = (TnyCamelStream *)stream;
	return tny_stream_iface_write (self->stream, buffer, n);
}

static int
tny_camel_stream_close (CamelStream *stream)
{
	TnyCamelStream *self = (TnyCamelStream *)stream;
	return tny_stream_iface_close (self->stream);
}

static gboolean
tny_camel_stream_eos (CamelStream *stream)
{
	TnyCamelStream *self = (TnyCamelStream *)stream;
	return tny_stream_iface_eos (self->stream);
}

static int
tny_camel_stream_flush (CamelStream *stream)
{
	TnyCamelStream *self = (TnyCamelStream *)stream;
	return tny_stream_iface_flush (self->stream);
}


static int
tny_camel_stream_reset (CamelStream *stream)
{
	TnyCamelStream *self = (TnyCamelStream *)stream;
	return tny_stream_iface_reset (self->stream);
}

static void
tny_camel_stream_init (CamelObject *object)
{
	TnyCamelStream *self = (TnyCamelStream *)object;
	self->stream = NULL;
	return;
}


static void
tny_camel_stream_finalize (CamelObject *object)
{
	TnyCamelStream *self = (TnyCamelStream *)object;

	if (self->stream)
		g_object_unref (G_OBJECT (self->stream));

	/* CamelObject types don't need parent finalization (build-in camel)
	(*((CamelObjectClass*)parent_class)->finalise) (object); */

	return;
}

static void
tny_camel_stream_class_init (TnyCamelStreamClass *klass)
{
	((CamelStreamClass *)klass)->read = tny_camel_stream_read;
	((CamelStreamClass *)klass)->write = tny_camel_stream_write;
	((CamelStreamClass *)klass)->close = tny_camel_stream_close;
	((CamelStreamClass *)klass)->eos = tny_camel_stream_eos;
	((CamelStreamClass *)klass)->reset = tny_camel_stream_reset;
	((CamelStreamClass *)klass)->flush = tny_camel_stream_flush;

	return;
}

CamelType
tny_camel_stream_get_type (void)
{
	static CamelType type = CAMEL_INVALID_TYPE;
	
	if (G_UNLIKELY (!camel_type_init_done))
	{
		camel_type_init ();
		camel_type_init_done = TRUE;
	}

	if (G_UNLIKELY (type == CAMEL_INVALID_TYPE)) 
	{
		parent_class = (CamelStreamClass *)camel_stream_get_type();
		type = camel_type_register ((CamelType)parent_class,
					    "TnyCamelStream",
					    sizeof (TnyCamelStream),
					    sizeof (TnyCamelStreamClass),
					    (CamelObjectClassInitFunc) tny_camel_stream_class_init,
					    NULL,
					    (CamelObjectInitFunc) tny_camel_stream_init,
					    (CamelObjectFinalizeFunc) tny_camel_stream_finalize);
	}
	
	return type;
}

/**
 * tny_camel_stream_set_stream:
 * @self: A #TnyCamelStream object
 * @stream: A #TnyStreamIface object
 *
 * Set the stream to play proxy for
 *
 **/
void
tny_camel_stream_set_stream (TnyCamelStream *self, TnyStreamIface *stream)
{

	if (self->stream)
		g_object_unref (G_OBJECT (self->stream));

	g_object_ref (G_OBJECT (stream));

	self->stream = stream;

	return;
}


/**
 * tny_camel_stream_new:
 * @stream: A #TnyStreamIface stream to play proxy for
 *
 * Return value: A new #CamelStream instance implemented as a proxy
 * for a #TnyStreamIface
 **/
TnyCamelStream *
tny_camel_stream_new (TnyStreamIface *stream)
{
	TnyCamelStream *self;

	self = (TnyCamelStream *)camel_object_new(tny_camel_stream_get_type());

	tny_camel_stream_set_stream (self, stream);

	return self;
}

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

#include <tny-camel-stream.h>

static CamelStreamClass *parent_class = NULL;


static ssize_t
tny_camel_stream_read (CamelStream *stream, char *buffer, size_t n)
{
}

static ssize_t
tny_camel_stream_write (CamelStream *stream, const char *buffer, size_t n)
{
}

static int
tny_camel_stream_close (CamelStream *stream)
{
}


static void
tny_camel_stream_init (CamelObject *object)
{
}

static void
tny_camel_stream_finalize (CamelObject *object)
{
	TnyCamelStream *self = (TnyCamelStream *)object;

	g_object_unref (G_OBJECT (self->stream));

	return;
}

static void
tny_camel_stream_class_init (TnyCamelStreamClass *klass)
{
	((CamelStreamClass *)klass)->read = tny_camel_stream_read;
	((CamelStreamClass *)klass)->write = tny_camel_stream_write;
	((CamelStreamClass *)klass)->close = tny_camel_stream_close;

	return;
}

CamelType
tny_camel_stream_get_type (void)
{
	static CamelType type = CAMEL_INVALID_TYPE;
	
	if (type == CAMEL_INVALID_TYPE) {
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

TnyCamelStream *
tny_camel_stream_stream_new (TnyStreamIface *stream)
{
	TnyCamelStream *self;

	self = (TnyCamelStream *)camel_object_new(tny_camel_stream_get_type());

	g_object_ref (G_OBJECT (stream));

	self->stream = stream;

	return self;
}

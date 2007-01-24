/*
 * Author:
 *  Philip Van Hoof <pvanhoof@gnome.org>
 *
 * Copyright 1999, 2007 Philip Van Hoof
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "camel-stream-gzip.h"

static CamelObjectClass *parent_class = NULL;

#define CSZ_CLASS(so) CAMEL_STREAM_GZIP_CLASS(CAMEL_OBJECT_GET_CLASS(so))


static ssize_t z_stream_read (CamelStream *stream, char *buffer, size_t n) 
{
	char *mem = NULL; ssize_t haveread = 0, retval = 0;
	CamelStreamGZip *self = (CamelStreamGZip *) stream;
	z_stream c_stream = * (self->stream);

	c_stream.next_out = (Bytef *) buffer;
	c_stream.avail_out = n;

	if (self->mode == CAMEL_STREAM_GZIP_ZIP)
	{
		mem = g_malloc0 (n);
		haveread = camel_stream_read (self->real, mem, n);
		c_stream.next_in = (Bytef *) mem;
		c_stream.avail_in = haveread;
		deflate (&c_stream, Z_FINISH);
		retval = n - c_stream.avail_out;
	} else
	{

		/* TODO */

		int block_size = n / 100;

		haveread = block_size;
		mem = g_malloc0 (block_size);

		c_stream.next_out = (Bytef *) buffer;
		c_stream.avail_out = n;

		while (haveread == block_size && c_stream.avail_out > 0)
		{
			haveread = camel_stream_read (self->real, mem, block_size);
			c_stream.next_in  = (Bytef *) mem;
			c_stream.avail_in = haveread;
			inflate (&c_stream, Z_FINISH);
			retval += haveread - c_stream.avail_out;
		}

	}

	g_free (mem);

	return retval; 
}


static ssize_t z_stream_write (CamelStream *stream, const char *buffer, size_t n) 
{ 
	CamelStreamGZip *self = (CamelStreamGZip *) stream;
	z_stream c_stream = * (self->stream);
	char *mem = NULL;
	ssize_t retval = 0;

	if (self->mode == CAMEL_STREAM_GZIP_ZIP)
	{

		/* TODO */

		mem = g_malloc0 (n);

		c_stream.next_in  = (Bytef *) buffer;
		c_stream.avail_in = n;
		c_stream.next_out = mem;
		c_stream.avail_out = n;

		deflate (&c_stream, Z_FINISH);

		camel_stream_write (self->real, mem, n - c_stream.avail_out);

		retval = n;

	} else 
	{
		mem = g_malloc0 (n);

		c_stream.next_in = (Bytef *) buffer;
		c_stream.avail_in = n;
		c_stream.next_out = (Bytef *) mem;
		c_stream.avail_out = n;

		while (c_stream.avail_in > 0)
		{
			inflate (&c_stream, Z_FINISH);

			camel_stream_write (self->real, mem, n - c_stream.avail_out);
			retval += n - c_stream.avail_out;

			c_stream.next_out = (Bytef *) mem;
			c_stream.avail_out = n;
		}
	}

	g_free (mem);

	return retval; 
}

static int z_stream_flush (CamelStream *stream) 
{ 
	CamelStreamGZip *self = (CamelStreamGZip *) stream;
	char mem[1024];
	z_stream c_stream = * (self->stream);


	if (self->mode != CAMEL_STREAM_GZIP_ZIP)
	{
		c_stream.next_out = (Bytef *) mem;
		c_stream.avail_out = 1024;

		while (c_stream.avail_in > 0)
		{
			inflate (&c_stream, Z_NO_FLUSH);
			camel_stream_write (self->real, mem, 1024 - c_stream.avail_out);
			c_stream.next_out = (Bytef *) mem;
			c_stream.avail_out = 1024;
		}
		inflateReset (self->stream);
	} else
		deflateReset (self->stream);

	return camel_stream_flush (self->real); 
}

static int z_stream_close (CamelStream *stream) 
{ 
	CamelStreamGZip *self = (CamelStreamGZip *) stream;

	z_stream_flush (stream);

	return camel_stream_close (self->real); 
}

static gboolean z_stream_eos (CamelStream *stream) 
{
	CamelStreamGZip *self = (CamelStreamGZip *) stream;
	return camel_stream_eos (self->real); 
}

static int z_stream_reset (CamelStream *stream) 
{ 
	CamelStreamGZip *self = (CamelStreamGZip *) stream;

	if (self->mode == CAMEL_STREAM_GZIP_ZIP)
		deflateReset (self->stream);
	else
		inflateReset (self->stream);

	return 0; 
}

static void
camel_stream_gzip_class_init (CamelStreamClass *camel_stream_gzip_class)
{
	CamelStreamClass *camel_stream_class = (CamelStreamClass *)camel_stream_gzip_class;

	parent_class = camel_type_get_global_classfuncs( CAMEL_OBJECT_TYPE );

	/* virtual method definition */
	camel_stream_class->read = z_stream_read;
	camel_stream_class->write = z_stream_write;
	camel_stream_class->close = z_stream_close;
	camel_stream_class->flush = z_stream_flush;
	camel_stream_class->eos = z_stream_eos;
	camel_stream_class->reset = z_stream_reset;
}

static void
camel_stream_gzip_finalize (CamelObject *object)
{
	CamelStreamGZip *self = (CamelStreamGZip *) object;

	camel_object_unref (CAMEL_OBJECT (self->real));

	if (self->mode == CAMEL_STREAM_GZIP_ZIP)
		deflateEnd (self->stream);
	else
		inflateEnd (self->stream);

	g_free (self->stream);

	return;
}

CamelType
camel_stream_gzip_get_type (void)
{
	static CamelType camel_stream_gzip_type = CAMEL_INVALID_TYPE;

	if (camel_stream_gzip_type == CAMEL_INVALID_TYPE) 
	{
		camel_stream_gzip_type = camel_type_register( 
			camel_stream_get_type(),
			"CamelStreamGZip",
			sizeof( CamelStreamGZip ),
			sizeof( CamelStreamGZipClass ),
			(CamelObjectClassInitFunc) camel_stream_gzip_class_init,
			NULL,
			NULL,
			(CamelObjectFinalizeFunc) camel_stream_gzip_finalize );
	}

	return camel_stream_gzip_type;
}

CamelStream *
camel_stream_gzip_new (CamelStream *real, int level, int mode)
{
	CamelStreamGZip *self = (CamelStreamGZip *) camel_object_new (camel_stream_gzip_get_type ());
	int retval;

	self->stream = g_new0 (z_stream, 1);
	self->level = level;
	self->mode = mode;

	if (self->mode == CAMEL_STREAM_GZIP_ZIP)
	{
		retval = deflateInit2 (self->stream, self->level, Z_DEFLATED, -MAX_WBITS, 
			MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);

			if (retval != Z_OK)
		{
			camel_object_unref (self);
			return NULL;
		}
	} else {
		retval = inflateInit2 (self->stream, -MAX_WBITS);
		if (retval != Z_OK)
		{
			camel_object_unref (self);
			return NULL;
		}
	}

	camel_object_ref (CAMEL_OBJECT (real));
	self->real = real;

	return CAMEL_STREAM (self);
}

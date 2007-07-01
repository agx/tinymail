#ifndef TNY_STREAM_CAMEL_H
#define TNY_STREAM_CAMEL_H

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

#include <glib.h>
#include <glib-object.h>
#include <tny-stream.h>

#include <camel/camel-seekable-stream.h>

G_BEGIN_DECLS

/* Strange Python binding generator wants this */
#define TNY_TYPE_STREAM_CAMEL_STREAM     (tny_stream_camel_get_type ())

#define TNY_TYPE_STREAM_CAMEL     (tny_stream_camel_get_type ())
#define TNY_STREAM_CAMEL(obj)     (CAMEL_CHECK_CAST((obj), TNY_TYPE_STREAM_CAMEL_STREAM, TnyStreamCamel))
#define TNY_STREAM_CAMEL_CLASS(k) (CAMEL_CHECK_CLASS_CAST ((k), TNY_TYPE_STREAM_CAMEL_STREAM, TnyStreamCamelClass))

typedef struct _TnyStreamCamel TnyStreamCamel;
typedef struct _TnyStreamCamelClass TnyStreamCamelClass;

struct _TnyStreamCamel
{
	CamelStream parent;

	TnyStream *stream;
};

struct _TnyStreamCamelClass 
{
	CamelStreamClass parent;
};

CamelType tny_stream_camel_get_type (void);
CamelStream* tny_stream_camel_new (TnyStream *stream);
void tny_stream_camel_set_stream (TnyStreamCamel *self, TnyStream *stream);

gssize tny_stream_camel_write_to_stream (TnyStreamCamel *self, TnyStream *output);

G_END_DECLS

#endif

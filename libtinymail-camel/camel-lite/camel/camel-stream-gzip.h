/*
 *  Copyright (C) 2007 Philip Van Hoof
 *
 *  Authors: Philip Van Hoof <pvanhoof@gnome.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#ifndef _CAMEL_STREAM_GZIP_H
#define _CAMEL_STREAM_GZIP_H

#ifdef __cplusplus
extern "C" {
#pragma }
#endif /* __cplusplus */

#include <zlib.h>

#include <camel/camel-stream.h>

#define CAMEL_STREAM_GZIP(obj)         CAMEL_CHECK_CAST (obj, camel_stream_gzip_get_type (), CamelStreamGZip)
#define CAMEL_STREAM_GZIP_CLASS(klass) CAMEL_CHECK_CLASS_CAST (klass, camel_stream_gzip_get_type (), CamelStreamGZipClass)
#define CAMEL_IS_STREAM_GZIP(obj)      CAMEL_CHECK_TYPE (obj, camel_stream_gzip_get_type ())

typedef struct _CamelStreamGZipClass CamelStreamGZipClass;

enum 
{
	CAMEL_STREAM_GZIP_ZIP,
	CAMEL_STREAM_GZIP_UNZIP
};

struct _CamelStreamGZip 
{
	CamelStream parent;

	CamelStream *real;
	z_stream *stream;
	int mode, level;
};

struct _CamelStreamGZipClass {
	CamelStreamClass parent_class;
};

CamelType camel_stream_gzip_get_type (void);

CamelStream *camel_stream_gzip_new (CamelStream *real, int level, int mode);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ! _CAMEL_STREAM_GZIP_H */

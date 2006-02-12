#ifndef TNY_CAMEL_STREAM_H
#define TNY_CAMEL_STREAM_H

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

#include <glib.h>
#include <glib-object.h>
#include <tny-stream-iface.h>

#include <camel/camel-seekable-stream.h>

G_BEGIN_DECLS

#define TNY_CAMEL_STREAM_TYPE     (tny_camel_stream_get_type ())
#define TNY_CAMEL_STREAM(obj)     (CAMEL_CHECK_CAST((obj), TNY_CAMEL_STREAM_TYPE, TnyCamelStream))
#define TNY_CAMEL_STREAM_CLASS(k) (CAMEL_CHECK_CLASS_CAST ((k), TNY_CAMEL_STREAM_TYPE, TnyCamelStreamClass))
#define TNY_CAMEL_VFS_STREAM(o)   (CAMEL_CHECK_TYPE((o), TNY_CAMEL_STREAM_TYPE))

typedef struct _TnyCamelStream TnyCamelStream;
typedef struct _TnyCamelStreamClass TnyCamelStreamClass;

struct _TnyCamelStream
{
	CamelStream parent;

	TnyStreamIface *stream;
};

struct _TnyCamelStreamClass 
{
	CamelStreamClass parent;
};

CamelType        tny_account_get_type           (void);
TnyCamelStream*  tny_camel_stream_stream_new    (TnyStreamIface *stream);

G_END_DECLS

#endif


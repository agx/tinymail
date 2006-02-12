#ifndef TNY_STREAM_CAMEL_H
#define TNY_STREAM_CAMEL_H

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
#include <camel/camel-stream.h>

G_BEGIN_DECLS

#define TNY_STREAM_CAMEL_TYPE             (tny_stream_camel_get_type ())
#define TNY_STREAM_CAMEL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_STREAM_CAMEL_TYPE, TnyStreamCamel))
#define TNY_STREAM_CAMEL_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_STREAM_CAMEL_TYPE, TnyStreamCamelClass))
#define TNY_IS_STREAM_CAMEL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_STREAM_CAMEL_TYPE))
#define TNY_IS_STREAM_CAMEL_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_STREAM_CAMEL_TYPE))
#define TNY_STREAM_CAMEL_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_STREAM_CAMEL_TYPE, TnyStreamCamelClass))

typedef struct _TnyStreamCamel TnyStreamCamel;
typedef struct _TnyStreamCamelClass TnyStreamCamelClass;

struct _TnyStreamCamel
{
	GObject parent;
};

struct _TnyStreamCamelClass 
{
	GObjectClass parent;
};

GType                   tny_stream_camel_get_type       (void);
TnyStreamCamel*         tny_stream_camel_new            (CamelStream *stream);

void                    tny_stream_camel_set_stream     (TnyStreamCamel *self, CamelStream *stream);

G_END_DECLS

#endif


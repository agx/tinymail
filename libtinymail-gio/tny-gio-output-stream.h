#ifndef TNY_GIO_OUTPUT_STREAM_H
#define TNY_GIO_OUTPUT_STREAM_H

/* libtinymail-gio - The Tiny Mail base library for Gio
 * Copyright (C) 2009 Javier Jardón <javierjc1982@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
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

#include <glib.h>
#include <gio/gio.h>
#include <glib-object.h>

#include <tny-stream.h>
#include <tny-seekable.h>

G_BEGIN_DECLS

#define TNY_TYPE_GIO_OUTPUT_STREAM             (tny_gio_output_stream_get_type ())
#define TNY_GIO_OUTPUT_STREAM(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_GIO_OUTPUT_STREAM, TnyGioOutputStream))
#define TNY_GIO_OUTPUT_STREAM_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_GIO_OUTPUT_STREAM, TnyGioOutputStreamClass))
#define TNY_IS_GIO_OUTPUT_STREAM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_GIO_OUTPUT_STREAM))
#define TNY_IS_GIO_OUTPUT_STREAM_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_GIO_OUTPUT_STREAM))
#define TNY_GIO_OUTPUT_STREAM_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_GIO_OUTPUT_STREAM, TnyGioOutputStreamClass))

typedef struct _TnyGioOutputStream TnyGioOutputStream;
typedef struct _TnyGioOutputStreamClass TnyGioOutputStreamClass;

struct _TnyGioOutputStream
{
	GObject parent;
};

struct _TnyGioOutputStreamClass
{
	GObjectClass parent;
};

GType tny_gio_output_stream_get_type (void);
TnyStream* tny_gio_output_stream_new (GOutputStream *output_stream);
void tny_gio_output_stream_set_stream (TnyGioOutputStream *self, GOutputStream *output_stream);

G_END_DECLS

#endif

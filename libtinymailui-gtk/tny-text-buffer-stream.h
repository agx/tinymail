#ifndef TNY_TEXT_BUFFER_STREAM_H
#define TNY_TEXT_BUFFER_STREAM_H

/* libtinymailui-gtk - The Tiny Mail UI library for Gtk+
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
#include <gtk/gtk.h>
#include <glib-object.h>

#include <tny-stream-iface.h>

G_BEGIN_DECLS

#define TNY_TEXT_BUFFER_STREAM_TYPE             (tny_text_buffer_stream_get_type ())
#define TNY_TEXT_BUFFER_STREAM(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TEXT_BUFFER_STREAM_TYPE, TnyTextBufferStream))
#define TNY_TEXT_BUFFER_STREAM_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TEXT_BUFFER_STREAM_TYPE, TnyTextBufferStreamClass))
#define TNY_IS_TEXT_BUFFER_STREAM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TEXT_BUFFER_STREAM_TYPE))
#define TNY_IS_TEXT_BUFFER_STREAM_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TEXT_BUFFER_STREAM_TYPE))
#define TNY_TEXT_BUFFER_STREAM_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TEXT_BUFFER_STREAM_TYPE, TnyTextBufferStreamClass))

typedef struct _TnyTextBufferStream TnyTextBufferStream;
typedef struct _TnyTextBufferStreamClass TnyTextBufferStreamClass;

struct _TnyTextBufferStream
{
	GObject parent;
};

struct _TnyTextBufferStreamClass 
{
	GObjectClass parent;
};

GType                   tny_text_buffer_stream_get_type        (void);
TnyTextBufferStream*    tny_text_buffer_stream_new             (GtkTextBuffer *buffer);

void                    tny_text_buffer_stream_set_text_buffer (TnyTextBufferStream *self, GtkTextBuffer *buffer);

G_END_DECLS

#endif


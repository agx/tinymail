#ifndef TNY_STREAM_IFACE_H
#define TNY_STREAM_IFACE_H

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

#include <stdarg.h>
#include <unistd.h>
#include <glib.h>
#include <glib-object.h>
#include <tny-shared.h>

G_BEGIN_DECLS

#define TNY_STREAM_IFACE_TYPE             (tny_stream_iface_get_type ())
#define TNY_STREAM_IFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_STREAM_IFACE_TYPE, TnyStreamIface))
#define TNY_STREAM_IFACE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_STREAM_IFACE_TYPE, TnyStreamIfaceClass))
#define TNY_IS_STREAM_IFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_STREAM_IFACE_TYPE))
#define TNY_IS_STREAM_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_STREAM_IFACE_TYPE))
#define TNY_STREAM_IFACE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_STREAM_IFACE_TYPE, TnyStreamIfaceClass))

struct _TnyStreamIfaceClass
{
	GTypeInterface parent;

	ssize_t   (*read_func)  (TnyStreamIface *self, char *buffer, size_t n);
	ssize_t   (*write_func) (TnyStreamIface *self, const char *buffer, size_t n);
	gint      (*flush_func) (TnyStreamIface *self);
	gint      (*close_func) (TnyStreamIface *self);
	gboolean  (*eos_func)   (TnyStreamIface *self);
	gint      (*reset_func) (TnyStreamIface *self);
};

GType        tny_stream_iface_get_type        (void);



ssize_t   tny_stream_iface_read  (TnyStreamIface *self, char *buffer, size_t n);
ssize_t   tny_stream_iface_write (TnyStreamIface *self, const char *buffer, size_t n);
gint      tny_stream_iface_flush (TnyStreamIface *self);
gint      tny_stream_iface_close (TnyStreamIface *self);
gboolean  tny_stream_iface_eos   (TnyStreamIface *self);
gint      tny_stream_iface_reset (TnyStreamIface *self);

G_END_DECLS

#endif

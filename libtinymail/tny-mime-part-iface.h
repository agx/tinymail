#ifndef TNY_MIME_PART_IFACE_H
#define TNY_MIME_PART_IFACE_H

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
#include <tny-shared.h>

#include <tny-stream-iface.h>

G_BEGIN_DECLS

#define TNY_TYPE_MIME_PART_IFACE             (tny_mime_part_iface_get_type ())
#define TNY_MIME_PART_IFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_MIME_PART_IFACE, TnyMimePartIface))
#define TNY_MIME_PART_IFACE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_MIME_PART_IFACE, TnyMimePartIfaceClass))
#define TNY_IS_MIME_PART_IFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_MIME_PART_IFACE))
#define TNY_IS_MIME_PART_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_MIME_PART_IFACE))
#define TNY_MIME_PART_IFACE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_MIME_PART_IFACE, TnyMimePartIfaceClass))

struct _TnyMimePartIfaceClass
{
	GTypeInterface parent;

	const gchar*    (*get_content_type_func)     (TnyMimePartIface *self);
	gboolean        (*content_type_is_func)      (TnyMimePartIface *self, const gchar *content_type);
	TnyStreamIface* (*get_stream_func)           (TnyMimePartIface *self);

	void            (*decode_to_stream_func)     (TnyMimePartIface *self, TnyStreamIface *stream);
	void            (*write_to_stream_func)      (TnyMimePartIface *self, TnyStreamIface *stream);
	gint            (*construct_from_stream_func)(TnyMimePartIface *self, TnyStreamIface *stream, const gchar *type);

	const gchar*    (*get_filename_func)         (TnyMimePartIface *self);
	const gchar*    (*get_content_id_func)       (TnyMimePartIface *self);
	const gchar*    (*get_description_func)      (TnyMimePartIface *self);
	const gchar*    (*get_content_location_func) (TnyMimePartIface *self);

	void            (*set_content_location_func) (TnyMimePartIface *self, const gchar *content_location); 
	void            (*set_description_func)      (TnyMimePartIface *self, const gchar *description); 
	void            (*set_content_id_func)       (TnyMimePartIface *self, const gchar *content_id); 
	void            (*set_filename_func)         (TnyMimePartIface *self, const gchar *filename);
	void            (*set_content_type_func)     (TnyMimePartIface *self, const gchar *content_type);

	gboolean 	(*is_attachment_func)        (TnyMimePartIface *self);
};

GType           tny_mime_part_iface_get_type              (void);

const gchar*    tny_mime_part_iface_get_content_type      (TnyMimePartIface *self);
gboolean        tny_mime_part_iface_content_type_is       (TnyMimePartIface *self, const gchar *type);

TnyStreamIface* tny_mime_part_iface_get_stream            (TnyMimePartIface *self);
void            tny_mime_part_iface_write_to_stream       (TnyMimePartIface *self, TnyStreamIface *stream);
gint            tny_mime_part_iface_construct_from_stream (TnyMimePartIface *self, TnyStreamIface *stream, const gchar *type);

const gchar*    tny_mime_part_iface_get_filename          (TnyMimePartIface *self);
const gchar*    tny_mime_part_iface_get_content_id        (TnyMimePartIface *self);
const gchar*    tny_mime_part_iface_get_description       (TnyMimePartIface *self);
const gchar*    tny_mime_part_iface_get_content_location  (TnyMimePartIface *self);


void            tny_mime_part_iface_set_content_location  (TnyMimePartIface *self, const gchar *content_location); 
void            tny_mime_part_iface_set_description       (TnyMimePartIface *self, const gchar *description); 
void            tny_mime_part_iface_set_content_id        (TnyMimePartIface *self, const gchar *content_id); 
void            tny_mime_part_iface_set_filename          (TnyMimePartIface *self, const gchar *filename);
void            tny_mime_part_iface_set_content_type      (TnyMimePartIface *self, const gchar *content_type);

gboolean	tny_mime_part_iface_is_attachment         (TnyMimePartIface *self);
void            tny_mime_part_iface_decode_to_stream      (TnyMimePartIface *self, TnyStreamIface *stream);

G_END_DECLS

#endif

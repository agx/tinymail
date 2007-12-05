#ifndef TNY_CAMEL_BS_MIME_PART_H
#define TNY_CAMEL_BS_MIME_PART_H

/* libtinymail-camel_bs - The Tiny Mail base library for CamelBs
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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <glib.h>
#include <tny-mime-part.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define TNY_TYPE_CAMEL_BS_MIME_PART             (tny_camel_bs_mime_part_get_type ())
#define TNY_CAMEL_BS_MIME_PART(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_CAMEL_BS_MIME_PART, TnyCamelBsMimePart))
#define TNY_CAMEL_BS_MIME_PART_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_CAMEL_BS_MIME_PART, TnyCamelBsMimePartClass))
#define TNY_IS_CAMEL_BS_MIME_PART(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_CAMEL_BS_MIME_PART))
#define TNY_IS_CAMEL_BS_MIME_PART_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_CAMEL_BS_MIME_PART))
#define TNY_CAMEL_BS_MIME_PART_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_CAMEL_BS_MIME_PART, TnyCamelBsMimePartClass))

typedef struct _TnyCamelBsMimePart TnyCamelBsMimePart;
typedef struct _TnyCamelBsMimePartClass TnyCamelBsMimePartClass;

struct _TnyCamelBsMimePart 
{
	GObject parent;
};

struct _TnyCamelBsMimePartClass 
{
	GObjectClass parent;

	/* virtual methods */
	const gchar* (*get_content_type_func) (TnyMimePart *self);
	gboolean (*content_type_is_func) (TnyMimePart *self, const gchar *content_type);
	TnyStream* (*get_stream_func) (TnyMimePart *self);
	void (*decode_to_stream_func) (TnyMimePart *self, TnyStream *stream);
	void (*write_to_stream_func) (TnyMimePart *self, TnyStream *stream);
	gint (*construct_from_stream_func) (TnyMimePart *self, TnyStream *stream, const gchar *type);
	const gchar* (*get_filename_func) (TnyMimePart *self);
	const gchar* (*get_content_id_func) (TnyMimePart *self);
	const gchar* (*get_description_func) (TnyMimePart *self);
	const gchar* (*get_content_location_func) (TnyMimePart *self);
	gboolean (*is_purged_func) (TnyMimePart *self);
	void (*set_content_location_func) (TnyMimePart *self, const gchar *content_location); 
	void (*set_description_func) (TnyMimePart *self, const gchar *description); 
	void (*set_content_id_func) (TnyMimePart *self, const gchar *content_id); 
	void (*set_filename_func) (TnyMimePart *self, const gchar *filename);
	void (*set_content_type_func) (TnyMimePart *self, const gchar *contenttype);
	void (*set_purged_func) (TnyMimePart *self);
	gboolean (*is_attachment_func) (TnyMimePart *self);
	void (*get_parts_func) (TnyMimePart *self, TnyList *list);
	void (*del_part_func) (TnyMimePart *self, TnyMimePart *part);
	gint (*add_part_func) (TnyMimePart *self, TnyMimePart *part);
	void (*get_header_pairs_func) (TnyMimePart *self, TnyList *list);
	void (*set_header_pair_func) (TnyMimePart *self, const gchar *name, const gchar *value);
	void (*decode_to_stream_async_func) (TnyMimePart *self, TnyStream *stream, TnyMimePartCallback callback, TnyStatusCallback status_callback, gpointer user_data);
};


GType tny_camel_bs_mime_part_get_type (void);


G_END_DECLS

#endif
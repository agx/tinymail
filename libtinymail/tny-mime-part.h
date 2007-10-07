#ifndef TNY_MIME_PART_H
#define TNY_MIME_PART_H

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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <glib.h>
#include <glib-object.h>
#include <tny-shared.h>

#include <tny-stream.h>
#include <tny-pair.h>

G_BEGIN_DECLS

#define TNY_TYPE_MIME_PART             (tny_mime_part_get_type ())
#define TNY_MIME_PART(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_MIME_PART, TnyMimePart))
#define TNY_IS_MIME_PART(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_MIME_PART))
#define TNY_MIME_PART_GET_IFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_MIME_PART, TnyMimePartIface))

#ifndef TNY_SHARED_H
typedef struct _TnyMimePart TnyMimePart;
typedef struct _TnyMimePartIface TnyMimePartIface;
#endif

struct _TnyMimePartIface
{
	GTypeInterface parent;

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
};

GType tny_mime_part_get_type (void);

const gchar* tny_mime_part_get_content_type (TnyMimePart *self);
gboolean tny_mime_part_content_type_is (TnyMimePart *self, const gchar *type);
TnyStream* tny_mime_part_get_stream (TnyMimePart *self);
void tny_mime_part_write_to_stream (TnyMimePart *self, TnyStream *stream);
gint tny_mime_part_construct_from_stream (TnyMimePart *self, TnyStream *stream, const gchar *type);
const gchar* tny_mime_part_get_filename (TnyMimePart *self);
const gchar* tny_mime_part_get_content_id (TnyMimePart *self);
const gchar* tny_mime_part_get_description (TnyMimePart *self);
const gchar* tny_mime_part_get_content_location (TnyMimePart *self);
gboolean tny_mime_part_is_purged (TnyMimePart *self);
void tny_mime_part_set_content_location (TnyMimePart *self, const gchar *content_location);
void tny_mime_part_set_description (TnyMimePart *self, const gchar *description); 
void tny_mime_part_set_content_id (TnyMimePart *self, const gchar *content_id); 
void tny_mime_part_set_filename (TnyMimePart *self, const gchar *filename);
void tny_mime_part_set_content_type (TnyMimePart *self, const gchar *contenttype);
void tny_mime_part_set_purged (TnyMimePart *self);
gboolean tny_mime_part_is_attachment (TnyMimePart *self);
void tny_mime_part_decode_to_stream (TnyMimePart *self, TnyStream *stream);
void tny_mime_part_get_parts (TnyMimePart *self, TnyList *list);
gint tny_mime_part_add_part (TnyMimePart *self, TnyMimePart *part);
void tny_mime_part_del_part (TnyMimePart *self, TnyMimePart *part);
void tny_mime_part_get_header_pairs (TnyMimePart *self, TnyList *list);
void tny_mime_part_set_header_pair (TnyMimePart *self, const gchar *name, const gchar *value);

G_END_DECLS

#endif

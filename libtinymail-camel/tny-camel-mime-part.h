#ifndef TNY_CAMEL_MIME_PART_H
#define TNY_CAMEL_MIME_PART_H

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
#include <tny-mime-part.h>
#include <camel/camel-mime-part.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define TNY_TYPE_CAMEL_MIME_PART             (tny_camel_mime_part_get_type ())
#define TNY_CAMEL_MIME_PART(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_CAMEL_MIME_PART, TnyCamelMimePart))
#define TNY_CAMEL_MIME_PART_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_CAMEL_MIME_PART, TnyCamelMimePartClass))
#define TNY_IS_CAMEL_MIME_PART(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_CAMEL_MIME_PART))
#define TNY_IS_CAMEL_MIME_PART_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_CAMEL_MIME_PART))
#define TNY_CAMEL_MIME_PART_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_CAMEL_MIME_PART, TnyCamelMimePartClass))

typedef struct _TnyCamelMimePart TnyCamelMimePart;
typedef struct _TnyCamelMimePartClass TnyCamelMimePartClass;

struct _TnyCamelMimePart 
{
	GObject parent;
};

struct _TnyCamelMimePartClass 
{
	GObjectClass parent;
};


GType tny_camel_mime_part_get_type (void);

TnyMimePart* tny_camel_mime_part_new (CamelMimePart *part);
void tny_camel_mime_part_set_part (TnyCamelMimePart *self, CamelMimePart *part);
CamelMimePart* tny_camel_mime_part_get_part (TnyCamelMimePart *self);

G_END_DECLS

#endif

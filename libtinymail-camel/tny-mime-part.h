#ifndef TNY_MIME_PART_H
#define TNY_MIME_PART_H

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
#include <tny-mime-part-iface.h>
#include <camel/camel-mime-part.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define TNY_TYPE_MIME_PART             (tny_msg_mime_part_get_type ())
#define TNY_MIME_PART(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_MIME_PART, TnyMimePart))
#define TNY_MIME_PART_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_MIME_PART, TnyMimePartClass))
#define TNY_IS_MIME_PART(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_MIME_PART))
#define TNY_IS_MIME_PART_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_MIME_PART))
#define TNY_MIME_PART_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_MIME_PART, TnyMimePartClass))

typedef struct _TnyMimePart TnyMimePart;
typedef struct _TnyMimePartClass TnyMimePartClass;

struct _TnyMimePart 
{
	GObject parent;
};

struct _TnyMimePartClass 
{
	GObjectClass parent;
};


GType                 tny_msg_mime_part_get_type  (void);

TnyMimePart*       tny_msg_mime_part_new       (CamelMimePart *part);
void                  tny_msg_mime_part_set_part  (TnyMimePart *self, CamelMimePart *part);
CamelMimePart*	      tny_msg_mime_part_get_part  (TnyMimePart *self);

G_END_DECLS

#endif

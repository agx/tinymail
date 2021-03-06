#ifndef TNY_CAMEL_HEADER_H
#define TNY_CAMEL_HEADER_H

/* This is a fake header to make binding generators believe nothing about this
 * type is special. */

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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <glib.h>
#include <glib-object.h>
#include <tny-header.h>


G_BEGIN_DECLS


#define TNY_TYPE_CAMEL_HEADER             (tny_camel_header_get_type ())
#define TNY_CAMEL_HEADER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_CAMEL_HEADER, TnyCamelHeader))
#define TNY_CAMEL_HEADER_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_CAMEL_HEADER, TnyCamelHeaderClass))
#define TNY_IS_CAMEL_HEADER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_CAMEL_HEADER))
#define TNY_IS_CAMEL_HEADER_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_CAMEL_HEADER))
#define TNY_CAMEL_HEADER_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_CAMEL_HEADER, TnyCamelHeaderClass))

typedef struct _TnyCamelHeader TnyCamelHeader;
typedef struct _TnyCamelHeaderClass TnyCamelHeaderClass;

struct _TnyCamelHeader
{
	GObject parent;
};

struct _TnyCamelHeaderClass 
{
	GObjectClass parent_class;
};

GType tny_camel_header_get_type (void);

G_END_DECLS

#endif


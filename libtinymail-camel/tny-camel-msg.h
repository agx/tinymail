#ifndef TNY_CAMEL_MSG_H
#define TNY_CAMEL_MSG_H

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
#include <glib-object.h>

#include <tny-msg.h>
#include <tny-mime-part.h>
#include <tny-stream.h>
#include <tny-header.h>
#include <tny-camel-mime-part.h>

G_BEGIN_DECLS

#define TNY_TYPE_CAMEL_MSG             (tny_camel_msg_get_type ())
#define TNY_CAMEL_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_CAMEL_MSG, TnyCamelMsg))
#define TNY_CAMEL_MSG_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_CAMEL_MSG, TnyCamelMsgClass))
#define TNY_IS_CAMEL_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_CAMEL_MSG))
#define TNY_IS_CAMEL_MSG_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_CAMEL_MSG))
#define TNY_CAMEL_MSG_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_CAMEL_MSG, TnyCamelMsgClass))

typedef struct _TnyCamelMsg TnyCamelMsg;
typedef struct _TnyCamelMsgClass TnyCamelMsgClass;

struct _TnyCamelMsg
{
	TnyCamelMimePart parent;
};

struct _TnyCamelMsgClass 
{
	TnyCamelMimePartClass parent;

	/* virtual methods */
	TnyHeader* (*get_header_func) (TnyMsg *self);
	TnyFolder* (*get_folder_func) (TnyMsg *self);
	gchar* (*get_url_string_func) (TnyMsg *self);
};

GType tny_camel_msg_get_type (void);

TnyMsg* tny_camel_msg_new (void);
TnyMsg* tny_camel_msg_new_with_part (CamelMimePart *part);


G_END_DECLS

#endif


#ifndef TNY_MSG_H
#define TNY_MSG_H

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

#include <tny-msg-iface.h>
#include <tny-msg-mime-part-iface.h>
#include <tny-stream-iface.h>
#include <tny-msg-header-iface.h>

G_BEGIN_DECLS

#define TNY_TYPE_MSG             (tny_msg_get_type ())
#define TNY_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_MSG, TnyMsg))
#define TNY_MSG_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_MSG, TnyMsgClass))
#define TNY_IS_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_MSG))
#define TNY_IS_MSG_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_MSG))
#define TNY_MSG_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_MSG, TnyMsgClass))

typedef struct _TnyMsg TnyMsg;
typedef struct _TnyMsgClass TnyMsgClass;

struct _TnyMsg
{
	GObject parent;	
};

struct _TnyMsgClass 
{
	GObjectClass parent;
};

GType     tny_msg_get_type (void);

TnyMsg*   tny_msg_new                       (void);
TnyMsg*   tny_msg_new_with_header           (TnyMsgHeaderIface *header);
TnyMsg*   tny_msg_new_with_header_and_parts (TnyMsgHeaderIface *header, const GList *parts);

G_END_DECLS

#endif


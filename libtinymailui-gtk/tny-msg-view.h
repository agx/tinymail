#ifndef TNY_MSG_VIEW_H
#define TNY_MSG_VIEW_H

/*
 * Copyright (C) 2006-2007 Philip Van Hoof <pvanhoof@gnome.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with self program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <gtk/gtk.h>
#include <glib-object.h>
#include <tny-shared.h>

#include <tny-msg-view-iface.h>
#include <tny-msg-header-iface.h>
#include <tny-msg-iface.h>
#include <tny-stream-iface.h>
#include <tny-msg-mime-part-iface.h>

G_BEGIN_DECLS

#define TNY_MSG_VIEW_TYPE             (tny_msg_view_get_type ())
#define TNY_MSG_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_MSG_VIEW_TYPE, TnyMsgView))
#define TNY_MSG_VIEW_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_MSG_VIEW_TYPE, TnyMsgViewClass))
#define TNY_IS_MSG_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_MSG_VIEW_TYPE))
#define TNY_IS_MSG_VIEW_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_MSG_VIEW_TYPE))
#define TNY_MSG_VIEW_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_MSG_VIEW_TYPE, TnyMsgViewClass))

typedef struct _TnyMsgView TnyMsgView;
typedef struct _TnyMsgViewClass TnyMsgViewClass;

struct _TnyMsgView
{
	GtkScrolledWindow parent;

};

struct _TnyMsgViewClass
{
	GtkScrolledWindowClass parent_class;
};

GType               tny_msg_view_get_type       (void);
TnyMsgView*         tny_msg_view_new            (void);

G_END_DECLS

#endif

#ifndef TNY_MOZ_EMBED_MSG_VIEW_H
#define TNY_MOZ_EMBED_MSG_VIEW_H

/* libtinymailui-gtk - The Tiny Mail UI library for Gtk+
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

#include <tny-msg-view.h>
#include <tny-header.h>
#include <tny-msg.h>
#include <tny-stream.h>
#include <tny-mime-part.h>
#include <tny-save-strategy.h>

G_BEGIN_DECLS

#define TNY_TYPE_MOZ_EMBED_MSG_VIEW             (tny_moz_embed_msg_view_get_type ())
#define TNY_MOZ_EMBED_MSG_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_MOZ_EMBED_MSG_VIEW, TnyMozEmbedMsgView))
#define TNY_MOZ_EMBED_MSG_VIEW_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_MOZ_EMBED_MSG_VIEW, TnyMozEmbedMsgViewClass))
#define TNY_IS_MOZ_EMBED_MSG_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_MOZ_EMBED_MSG_VIEW))
#define TNY_IS_MOZ_EMBED_MSG_VIEW_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_MOZ_EMBED_MSG_VIEW))
#define TNY_MOZ_EMBED_MSG_VIEW_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_MOZ_EMBED_MSG_VIEW, TnyMozEmbedMsgViewClass))

typedef struct _TnyMozEmbedMsgView TnyMozEmbedMsgView;
typedef struct _TnyMozEmbedMsgViewClass TnyMozEmbedMsgViewClass;

struct _TnyMozEmbedMsgView
{
	GtkScrolledWindow parent;

};

struct _TnyMozEmbedMsgViewClass
{
	GtkScrolledWindowClass parent_class;
};

GType tny_moz_embed_msg_view_get_type (void);
TnyMsgView* tny_moz_embed_msg_view_new (TnySaveStrategy *save_strategy);

G_END_DECLS

#endif

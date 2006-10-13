#ifndef TNY_GTK_ATTACHMENT_MIME_PART_VIEW_H
#define TNY_GTK_ATTACHMENT_MIME_PART_VIEW_H

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

#include <tny-mime-part-view.h>
#include <tny-stream.h>
#include <tny-mime-part.h>
#include <tny-save-strategy.h>
#include <tny-gtk-attach-list-model.h>

G_BEGIN_DECLS

#define TNY_TYPE_GTK_ATTACHMENT_MIME_PART_VIEW             (tny_gtk_attachment_mime_part_view_get_type ())
#define TNY_GTK_ATTACHMENT_MIME_PART_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_GTK_ATTACHMENT_MIME_PART_VIEW, TnyGtkAttachmentMimePartView))
#define TNY_GTK_ATTACHMENT_MIME_PART_VIEW_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_GTK_ATTACHMENT_MIME_PART_VIEW, TnyGtkAttachmentMimePartViewClass))
#define TNY_IS_GTK_ATTACHMENT_MIME_PART_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_GTK_ATTACHMENT_MIME_PART_VIEW))
#define TNY_IS_GTK_ATTACHMENT_MIME_PART_VIEW_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_GTK_ATTACHMENT_MIME_PART_VIEW))
#define TNY_GTK_ATTACHMENT_MIME_PART_VIEW_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_GTK_ATTACHMENT_MIME_PART_VIEW, TnyGtkAttachmentMimePartViewClass))

typedef struct _TnyGtkAttachmentMimePartView TnyGtkAttachmentMimePartView;
typedef struct _TnyGtkAttachmentMimePartViewClass TnyGtkAttachmentMimePartViewClass;

struct _TnyGtkAttachmentMimePartView
{
	GObject parent;

};

struct _TnyGtkAttachmentMimePartViewClass
{
	GObjectClass parent_class;
};

GType tny_gtk_attachment_mime_part_view_get_type (void);
TnyMimePartView* tny_gtk_attachment_mime_part_view_new (TnySaveStrategy *save_strategy, TnyGtkAttachListModel *iview);

G_END_DECLS

#endif

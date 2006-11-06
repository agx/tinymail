#ifndef TNY_GTK_HEADER_VIEW_H
#define TNY_GTK_HEADER_VIEW_H

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

#include <tny-header-view.h>

G_BEGIN_DECLS

#define TNY_TYPE_GTK_HEADER_VIEW             (tny_gtk_header_view_get_type ())
#define TNY_GTK_HEADER_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_GTK_HEADER_VIEW, TnyGtkHeaderView))
#define TNY_GTK_HEADER_VIEW_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_GTK_HEADER_VIEW, TnyGtkHeaderViewClass))
#define TNY_IS_GTK_HEADER_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_GTK_HEADER_VIEW))
#define TNY_IS_GTK_HEADER_VIEW_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_GTK_HEADER_VIEW))
#define TNY_GTK_HEADER_VIEW_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_GTK_HEADER_VIEW, TnyGtkHeaderViewClass))

typedef struct _TnyGtkHeaderView TnyGtkHeaderView;
typedef struct _TnyGtkHeaderViewClass TnyGtkHeaderViewClass;

struct _TnyGtkHeaderView
{
	GtkTable parent;

};

struct _TnyGtkHeaderViewClass
{
	GtkTableClass parent_class;

	/* virtual methods */
	void (*set_header_func) (TnyHeaderView *self, TnyHeader *header);
	void (*clear_func) (TnyHeaderView *self);   
};

GType tny_gtk_header_view_get_type (void);
TnyHeaderView* tny_gtk_header_view_new (void);

G_END_DECLS

#endif

#ifndef TNY_SUMMARY_WINDOW_H
#define TNY_SUMMARY_WINDOW_H

/* tinymail - Tiny Mail
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

G_BEGIN_DECLS

#define TNY_TYPE_SUMMARY_WINDOW             (tny_summary_window_get_type ())
#define TNY_SUMMARY_WINDOW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_SUMMARY_WINDOW, TnySummaryWindow))
#define TNY_SUMMARY_WINDOW_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_SUMMARY_WINDOW, TnySummaryWindowClass))
#define TNY_IS_SUMMARY_WINDOW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_SUMMARY_WINDOW))
#define TNY_IS_SUMMARY_WINDOW_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_SUMMARY_WINDOW))
#define TNY_SUMMARY_WINDOW_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_SUMMARY_WINDOW, TnySummaryWindowClass))

typedef struct _TnySummaryWindow TnySummaryWindow;
typedef struct _TnySummaryWindowClass TnySummaryWindowClass;

struct _TnySummaryWindow
{
	GtkWindow parent;

};

struct _TnySummaryWindowClass
{
	GtkWindowClass parent_class;
};

GType               tny_summary_window_get_type          (void);
TnySummaryWindow*   tny_summary_window_new               (void);


G_END_DECLS

#endif

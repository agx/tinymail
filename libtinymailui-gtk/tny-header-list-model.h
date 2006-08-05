#ifndef TNY_HEADER_LIST_MODEL_H
#define TNY_HEADER_LIST_MODEL_H

/* libtinymailui-gtk - The Tiny Mail UI library for Gtk+
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

#include <gtk/gtktreemodel.h>
#include <tny-header-iface.h>
#include <tny-folder-iface.h>
#include <tny-list-iface.h>

G_BEGIN_DECLS

#define TNY_TYPE_HEADER_LIST_MODEL             (tny_header_list_model_get_type ())
#define TNY_HEADER_LIST_MODEL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_HEADER_LIST_MODEL, TnyHeaderListModel))
#define TNY_HEADER_LIST_MODEL_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_HEADER_LIST_MODEL, TnyHeaderListModelClass))
#define TNY_IS_HEADER_LIST_MODEL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_HEADER_LIST_MODEL))
#define TNY_IS_HEADER_LIST_MODEL_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_HEADER_LIST_MODEL))
#define TNY_HEADER_LIST_MODEL_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_HEADER_LIST_MODEL, TnyHeaderListModelClass))

/* Implements GtkTreeModelIface and TnyListIface */

typedef struct _TnyHeaderListModel TnyHeaderListModel;
typedef struct _TnyHeaderListModelClass TnyHeaderListModelClass;
typedef enum _TnyHeaderListModelColumn TnyHeaderListModelColumn;

#define TNY_TYPE_HEADER_LIST_MODEL_COLUMN (tny_header_list_model_column_get_type())

enum _TnyHeaderListModelColumn
{
	TNY_HEADER_LIST_MODEL_FROM_COLUMN,
	TNY_HEADER_LIST_MODEL_TO_COLUMN,
	TNY_HEADER_LIST_MODEL_SUBJECT_COLUMN,
	TNY_HEADER_LIST_MODEL_CC_COLUMN,
	TNY_HEADER_LIST_MODEL_DATE_SENT_COLUMN,
	TNY_HEADER_LIST_MODEL_DATE_RECEIVED_TIME_T_COLUMN,
	TNY_HEADER_LIST_MODEL_DATE_SENT_TIME_T_COLUMN,
	TNY_HEADER_LIST_MODEL_DATE_RECEIVED_COLUMN,
	TNY_HEADER_LIST_MODEL_INSTANCE_COLUMN,
	TNY_HEADER_LIST_MODEL_FLAGS_COLUMN,
	TNY_HEADER_LIST_MODEL_N_COLUMNS
};


struct _TnyHeaderListModel 
{
	GObject parent;

	GMutex *folder_lock, *iterator_lock;
	TnyFolderIface *folder;
	gint length;
	gint stamp;

	gboolean usable_index;
	GList *first, *index;
	TnyIteratorIface *iterator;
	guint last_nth;
};

struct _TnyHeaderListModelClass 
{
	GObjectClass parent;
};

GType tny_header_list_model_get_type (void);
GType tny_header_list_model_column_get_type (void);
TnyHeaderListModel* tny_header_list_model_new (void);
void tny_header_list_model_set_folder (TnyHeaderListModel *self, TnyFolderIface *folder, gboolean refresh);
gint tny_header_list_model_received_date_sort_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data);
gint tny_header_list_model_sent_date_sort_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data);

G_END_DECLS

#endif

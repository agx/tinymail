/* libtinymailui - The Tiny Mail UI library
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

#include <tny-msg-header-list-model.h>
#include <tny-msg-header-iface.h>

#define G_LIST(o) ((GList *) o)

static GObjectClass *parent_class;

struct _TnyMsgHeaderListModel 
{
	GObject parent_instance;

	TnyMsgFolderIface *folder;
	const GList *headers;
	gint length;

	gint stamp;
};

struct _TnyMsgHeaderListModelClass 
{
	GObjectClass parent_class;
};


static guint
tny_msg_header_list_model_get_flags (GtkTreeModel *self)
{
	return 0;
}

static gint
tny_msg_header_list_model_get_n_columns (GtkTreeModel *self)
{
	return TNY_MSG_HEADER_LIST_MODEL_NUM_COLUMNS;
}

static GType
tny_msg_header_list_model_get_column_type (GtkTreeModel *self, gint column)
{		
	switch (column) 
	{
		case TNY_MSG_HEADER_LIST_MODEL_CC_COLUMN:
		case TNY_MSG_HEADER_LIST_MODEL_DATE_SENT_COLUMN:
		case TNY_MSG_HEADER_LIST_MODEL_DATE_RECEIVED_COLUMN:
		case TNY_MSG_HEADER_LIST_MODEL_TO_COLUMN:
		case TNY_MSG_HEADER_LIST_MODEL_FROM_COLUMN:
		case TNY_MSG_HEADER_LIST_MODEL_SUBJECT_COLUMN:
			return G_TYPE_STRING;
		case TNY_MSG_HEADER_LIST_MODEL_INSTANCE_COLUMN:
			return G_TYPE_POINTER;
		default:
		return G_TYPE_INVALID;
	}
}

static gboolean
tny_msg_header_list_model_get_iter (GtkTreeModel *self, GtkTreeIter *iter, GtkTreePath *path)
{
	TnyMsgHeaderListModel *list_model = TNY_MSG_HEADER_LIST_MODEL (self);
	GList *list ;
	gint i;

	g_return_val_if_fail (gtk_tree_path_get_depth (path) > 0, FALSE);

	i = gtk_tree_path_get_indices (path)[0];
	list = g_list_nth (G_LIST (list_model->headers), i);

	if (i >= list_model->length)
		return FALSE;
		
	iter->stamp = list_model->stamp;
	iter->user_data = list;

	return TRUE;
}

static GtkTreePath *
tny_msg_header_list_model_get_path (GtkTreeModel *self, GtkTreeIter *iter)
{
	GList *list;
	GtkTreePath *tree_path;
	gint i = 0;

	g_return_val_if_fail (iter->stamp == TNY_MSG_HEADER_LIST_MODEL 
			(self)->stamp, NULL);

	for (list = G_LIST (TNY_MSG_HEADER_LIST_MODEL (self)->headers); 
		list; list = list->next) 
	{
		if (list == G_LIST (iter->user_data))
			break;
		i++;
	}

	if (list == NULL)
		return NULL;

		
	tree_path = gtk_tree_path_new ();
	gtk_tree_path_append_index (tree_path, i);

	return tree_path;
}

static gchar *
get_readable_date (const time_t file_time_raw)
{
	struct tm *file_time;
	static gchar readable_date[64];
	gsize readable_date_size;

	file_time = localtime (&file_time_raw);

	readable_date_size = strftime (readable_date, 63, "%Y-%m-%d, %-I:%M %p", file_time);		
	
	return readable_date;
}


static void
tny_msg_header_list_model_get_value (GtkTreeModel *self, GtkTreeIter *iter, gint column, GValue *value)
{
	TnyMsgHeaderIface *header = NULL;
	gchar *readable;

	g_return_if_fail (iter->stamp == 
		TNY_MSG_HEADER_LIST_MODEL (self)->stamp);

	g_return_if_fail (iter->user_data != NULL);
	
	header = G_LIST (iter->user_data)->data;
	
	switch (column) 
	{
		case TNY_MSG_HEADER_LIST_MODEL_CC_COLUMN:
			g_value_init (value, G_TYPE_STRING);
			g_value_set_string (value, tny_msg_header_iface_get_cc (header));
			break;
		case TNY_MSG_HEADER_LIST_MODEL_DATE_SENT_COLUMN:
			g_value_init (value, G_TYPE_STRING);
			g_value_set_string (value, 
				get_readable_date (tny_msg_header_iface_get_date_sent (header)));
			break;
		case TNY_MSG_HEADER_LIST_MODEL_DATE_RECEIVED_COLUMN:
			g_value_init (value, G_TYPE_STRING);
			g_value_set_string (value, 
			get_readable_date (tny_msg_header_iface_get_date_received (header)));
			break;
		case TNY_MSG_HEADER_LIST_MODEL_INSTANCE_COLUMN:
			g_value_init (value, G_TYPE_POINTER);
			g_value_set_pointer (value, header);
			break;
		case TNY_MSG_HEADER_LIST_MODEL_TO_COLUMN:
			g_value_init (value, G_TYPE_STRING);
			g_value_set_string (value, tny_msg_header_iface_get_to (header));
			break;
		case TNY_MSG_HEADER_LIST_MODEL_SUBJECT_COLUMN:
			g_value_init (value, G_TYPE_STRING);
			g_value_set_string (value, tny_msg_header_iface_get_subject (header));
			break;
		case TNY_MSG_HEADER_LIST_MODEL_FROM_COLUMN:
			g_value_init (value, G_TYPE_STRING);
			g_value_set_string (value, tny_msg_header_iface_get_from (header));			
			break;
		default:
			break;
	}

	return;
}

static gboolean
tny_msg_header_list_model_iter_next (GtkTreeModel *self, GtkTreeIter *iter)
{
	g_return_val_if_fail (iter->stamp == TNY_MSG_HEADER_LIST_MODEL 
		(self)->stamp, FALSE);

	iter->user_data = G_LIST (iter->user_data)->next;

	return (iter->user_data != NULL);
}

static gboolean
tny_msg_header_list_model_iter_has_child (GtkTreeModel *self, GtkTreeIter *iter)
{
	return FALSE;
}

static gint
tny_msg_header_list_model_iter_n_children (GtkTreeModel *self, GtkTreeIter *iter)
{
	if (!iter)
		return TNY_MSG_HEADER_LIST_MODEL (self)->length;

	return -1;
}

static gboolean
tny_msg_header_list_model_iter_nth_child (GtkTreeModel *self, GtkTreeIter *iter, GtkTreeIter *parent, gint n)
{
	GList *child;

	if (parent)
		return FALSE;

	child = g_list_nth (G_LIST (TNY_MSG_HEADER_LIST_MODEL (self)->headers), n);

	if (child) 
	{
		iter->stamp = TNY_MSG_HEADER_LIST_MODEL (self)->stamp;
		iter->user_data = child;
		return TRUE;
	}

	return FALSE;
}

static void
tny_msg_header_list_model_unref_node (GtkTreeModel *self, GtkTreeIter  *iter)
{
	TnyMsgHeaderIface *header = NULL;
	
	g_return_if_fail (iter->stamp == TNY_MSG_HEADER_LIST_MODEL (self)->stamp);
	g_return_if_fail (iter->user_data != NULL);

	header = G_LIST (iter->user_data)->data;
	
	if (header)
		tny_msg_header_iface_uncache (header);

	return;
}

static void
tny_msg_header_list_model_ref_node (GtkTreeModel *self, GtkTreeIter  *iter)
{
	TnyMsgHeaderIface *header = NULL;
	
	g_return_if_fail (iter->stamp == TNY_MSG_HEADER_LIST_MODEL (self)->stamp);
	g_return_if_fail (iter->user_data != NULL);

	header = G_LIST (iter->user_data)->data;

	return;
}

static void
tny_msg_header_list_model_tree_model_init (GtkTreeModelIface *iface)
{
	iface->get_flags = tny_msg_header_list_model_get_flags;
	iface->get_n_columns = tny_msg_header_list_model_get_n_columns;
	iface->get_column_type = tny_msg_header_list_model_get_column_type;
	iface->get_iter = tny_msg_header_list_model_get_iter;
	iface->get_path = tny_msg_header_list_model_get_path;
	iface->get_value = tny_msg_header_list_model_get_value;
	iface->iter_next = tny_msg_header_list_model_iter_next;
	iface->iter_has_child = tny_msg_header_list_model_iter_has_child;
	iface->iter_n_children = tny_msg_header_list_model_iter_n_children;
	iface->iter_nth_child = tny_msg_header_list_model_iter_nth_child;
	iface->ref_node = tny_msg_header_list_model_ref_node;
	iface->unref_node = tny_msg_header_list_model_unref_node;

	return;
}

static void
destroy_internal_list (TnyMsgHeaderListModel *self)
{

	self->headers = NULL;
	self->length = 0;

	return;
}

static void
tny_msg_header_list_model_finalize (GObject *object)
{
	TnyMsgHeaderListModel *self = (TnyMsgHeaderListModel *)object;

	if (self->folder)
		g_object_unref (G_OBJECT (self->folder));

	parent_class->finalize (object);

	return;
}

static void
tny_msg_header_list_model_class_init (TnyMsgHeaderListModelClass *klass)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *)klass;
	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = tny_msg_header_list_model_finalize;

	return;
}

static void
tny_msg_header_list_model_init (TnyMsgHeaderListModel *self)
{
	self->headers = NULL;

	return;
}


void
tny_msg_header_list_model_set_folder (TnyMsgHeaderListModel *self, TnyMsgFolderIface *folder)
{
	const GList* headers = tny_msg_folder_iface_get_headers (folder);

	self->headers = NULL;
	self->length = 0;

	if (self->folder)
		g_object_unref (G_OBJECT (self->folder));

	self->folder = folder;
	g_object_ref (G_OBJECT (folder));

	self->headers = headers;
	self->length = tny_msg_folder_iface_get_all_count (folder);

	return;
}


GtkTreeModel *
tny_msg_header_list_model_new (void)
{
	TnyMsgHeaderListModel *model;

	model = g_object_new (TNY_MSG_HEADER_TYPE_LIST_MODEL, NULL);
	
	return GTK_TREE_MODEL (model);
}

GType
tny_msg_header_list_model_get_type (void)
{
	static GType object_type = 0;

	if (!object_type) {
		static const GTypeInfo object_info = {
			sizeof (TnyMsgHeaderListModelClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) tny_msg_header_list_model_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (TnyMsgHeaderListModel),
			0,              /* n_preallocs */
			(GInstanceInitFunc) tny_msg_header_list_model_init
		};

		static const GInterfaceInfo tree_model_info = {
			(GInterfaceInitFunc) tny_msg_header_list_model_tree_model_init,
			NULL,
			NULL
		};
		
		object_type = g_type_register_static (G_TYPE_OBJECT, 
						"TnyMsgHeaderListModel", &object_info, 0);

		g_type_add_interface_static (object_type,
					     GTK_TYPE_TREE_MODEL,
					     &tree_model_info);
	}

	return object_type;
}

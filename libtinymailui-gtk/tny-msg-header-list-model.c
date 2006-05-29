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

#include <config.h>

#include <glib.h>
#include <glib/gi18n-lib.h>

#include <tny-msg-header-list-model.h>
#include <tny-msg-header-iface.h>
#include <tny-msg-folder-iface.h>

#include <tny-list-iface.h>
#include <tny-iterator-iface.h>

#include <tny-msg-header-list-iterator.h>

#define G_LIST(o) ((GList *) o)

static GObjectClass *parent_class;

#include "tny-msg-header-list-model-priv.h"

/* Private stuff */
TnyMsgHeaderListIterator* _tny_msg_header_list_iterator_new (TnyMsgHeaderListModel *model);
void _tny_msg_header_list_iterator_set_model (TnyMsgHeaderListIterator *self, TnyMsgHeaderListModel *model);
void _tny_msg_header_list_iterator_travel_to_nth (TnyMsgHeaderListIterator *self, guint cur, guint nth);


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
	GType retval;

	switch (column) 
	{
		case TNY_MSG_HEADER_LIST_MODEL_CC_COLUMN:
		case TNY_MSG_HEADER_LIST_MODEL_DATE_SENT_COLUMN:
		case TNY_MSG_HEADER_LIST_MODEL_DATE_RECEIVED_COLUMN:
		case TNY_MSG_HEADER_LIST_MODEL_TO_COLUMN:
		case TNY_MSG_HEADER_LIST_MODEL_FROM_COLUMN:
		case TNY_MSG_HEADER_LIST_MODEL_SUBJECT_COLUMN:
			retval = G_TYPE_STRING;
			break;
		case TNY_MSG_HEADER_LIST_MODEL_INSTANCE_COLUMN:
			retval = G_TYPE_POINTER;
			break;
		case TNY_MSG_HEADER_LIST_MODEL_FLAGS_COLUMN:
			retval = G_TYPE_INT;
			break;
		default:
			retval = G_TYPE_INVALID;
			break;
	}


	return retval;
}


static gboolean
tny_msg_header_list_model_get_iter (GtkTreeModel *self, GtkTreeIter *iter, GtkTreePath *path)
{
	TnyMsgHeaderListModel *list_model = TNY_MSG_HEADER_LIST_MODEL (self);
	gint i; gpointer ptr;

	g_return_val_if_fail (gtk_tree_path_get_depth (path) > 0, FALSE);

	g_mutex_lock (list_model->folder_lock);

	
	g_mutex_lock (list_model->iterator_lock);

	i = gtk_tree_path_get_indices (path)[0];

	if (G_UNLIKELY (i >= list_model->length))
	{
		g_mutex_unlock (list_model->iterator_lock);
		g_mutex_unlock (list_model->folder_lock);
		return FALSE;
	}

	g_mutex_unlock (list_model->iterator_lock);

	_tny_msg_header_list_iterator_travel_to_nth 
		((TnyMsgHeaderListIterator*)list_model->iterator, 
		list_model->last_nth, i);
	ptr = tny_iterator_iface_current (list_model->iterator);

	g_mutex_lock (list_model->iterator_lock);
	list_model->last_nth = i;
	iter->stamp = list_model->stamp;
	iter->user_data = ptr;

	g_mutex_unlock (list_model->iterator_lock);

	
	g_mutex_unlock (list_model->folder_lock);

	return TRUE;
}

static GtkTreePath *
tny_msg_header_list_model_get_path (GtkTreeModel *self, GtkTreeIter *iter)
{
	GList *list, *headers;
	GtkTreePath *tree_path;
	gint i = 0;
	TnyMsgHeaderListModel *list_model = TNY_MSG_HEADER_LIST_MODEL (self);

	g_return_val_if_fail (iter->stamp == TNY_MSG_HEADER_LIST_MODEL 
			(self)->stamp, NULL);

	g_mutex_lock (list_model->folder_lock);


	while (tny_iterator_iface_has_next (list_model->iterator))
	{
		if (tny_iterator_iface_next (list_model->iterator) == iter->user_data)
			break;
		i++;
	}
	tny_iterator_iface_first (list_model->iterator);
	
	g_mutex_lock (list_model->iterator_lock);
	tree_path = gtk_tree_path_new ();
	gtk_tree_path_append_index (tree_path, i);
	g_mutex_unlock (list_model->iterator_lock);

	g_mutex_unlock (list_model->folder_lock);

	return tree_path;
}

static gchar *
_get_readable_date (const time_t file_time_raw)
{
	struct tm *file_time;
	static gchar readable_date[64];
	gsize readable_date_size;

	file_time = localtime (&file_time_raw);

	readable_date_size = strftime (readable_date, 63, _("%Y-%m-%d, %-I:%M %p"), file_time);		
	
	return readable_date;
}


static void
tny_msg_header_list_model_get_value (GtkTreeModel *self, GtkTreeIter *iter, gint column, GValue *value)
{
	TnyMsgHeaderIface *header = NULL;
	gchar *readable;
	TnyMsgHeaderListModel *list_model = TNY_MSG_HEADER_LIST_MODEL (self);

	g_return_if_fail (iter->stamp == TNY_MSG_HEADER_LIST_MODEL (self)->stamp);
	g_return_if_fail (iter->user_data != NULL);

	g_mutex_lock (list_model->folder_lock);
	g_mutex_lock (list_model->iterator_lock);

	header = iter->user_data;
	
	switch (column) 
	{
		case TNY_MSG_HEADER_LIST_MODEL_CC_COLUMN:
			g_value_init (value, G_TYPE_STRING);
			g_value_set_string (value, tny_msg_header_iface_get_cc (header));
			break;
		case TNY_MSG_HEADER_LIST_MODEL_DATE_SENT_COLUMN:
			g_value_init (value, G_TYPE_STRING);
			g_value_set_string (value, 
				_get_readable_date (tny_msg_header_iface_get_date_sent (header)));
			break;
		case TNY_MSG_HEADER_LIST_MODEL_DATE_RECEIVED_COLUMN:
			g_value_init (value, G_TYPE_STRING);
			g_value_set_string (value, 
				_get_readable_date (tny_msg_header_iface_get_date_sent (header)));
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
		case TNY_MSG_HEADER_LIST_MODEL_FLAGS_COLUMN:
			g_value_init (value, G_TYPE_INT);
			g_value_set_int (value, tny_msg_header_iface_get_flags (header));
			break;
		default:
			break;
	}

	g_mutex_unlock (list_model->iterator_lock);
	g_mutex_unlock (list_model->folder_lock);

	return;
}

static gboolean
tny_msg_header_list_model_iter_next (GtkTreeModel *self, GtkTreeIter *iter)
{
	gboolean retval;
	TnyMsgHeaderListModel *list_model = TNY_MSG_HEADER_LIST_MODEL (self);
	GList *headers;

	g_return_val_if_fail (iter->stamp == TNY_MSG_HEADER_LIST_MODEL 
		(self)->stamp, FALSE);

	g_mutex_lock (list_model->folder_lock);

	g_mutex_lock (list_model->iterator_lock);
	iter->user_data = tny_iterator_iface_next (list_model->iterator);
	retval = (iter->user_data != NULL);
	g_mutex_unlock (list_model->iterator_lock);

	g_mutex_unlock (list_model->folder_lock);

	return retval;
}

static gboolean
tny_msg_header_list_model_iter_has_child (GtkTreeModel *self, GtkTreeIter *iter)
{
	return FALSE;
}

static gint
tny_msg_header_list_model_iter_n_children (GtkTreeModel *self, GtkTreeIter *iter)
{
	gint retval = -1;
	TnyMsgHeaderListModel *list_model = TNY_MSG_HEADER_LIST_MODEL (self);

	g_mutex_lock (list_model->folder_lock);

	g_mutex_lock (list_model->iterator_lock);
	if (G_LIKELY (!iter))
		retval = TNY_MSG_HEADER_LIST_MODEL (self)->length;
	g_mutex_unlock (list_model->iterator_lock);

	g_mutex_unlock (list_model->folder_lock);

	return retval;
}

static gboolean
tny_msg_header_list_model_iter_nth_child (GtkTreeModel *self, GtkTreeIter *iter, GtkTreeIter *parent, gint n)
{
	GList *child, *headers;
	TnyMsgHeaderListModel *list_model = TNY_MSG_HEADER_LIST_MODEL (self);

	if (G_UNLIKELY (parent))
		return FALSE;

	g_mutex_lock (list_model->folder_lock);

	child = tny_iterator_iface_nth (list_model->iterator, n);

	g_mutex_lock (list_model->iterator_lock);

	if (G_LIKELY (child))
	{
		iter->stamp = TNY_MSG_HEADER_LIST_MODEL (self)->stamp;
		iter->user_data = child;

		g_mutex_unlock (list_model->iterator_lock);
		g_mutex_unlock (list_model->folder_lock);

		return TRUE;
	}

	g_mutex_unlock (list_model->iterator_lock);
	g_mutex_unlock (list_model->folder_lock);

	return FALSE;
}

static void
tny_msg_header_list_model_unref_node (GtkTreeModel *self, GtkTreeIter  *iter)
{
	TnyMsgHeaderIface *header = NULL;
	TnyMsgHeaderListModel *list_model = TNY_MSG_HEADER_LIST_MODEL (self);

	g_return_if_fail (self);
	g_return_if_fail (iter->stamp == TNY_MSG_HEADER_LIST_MODEL (self)->stamp);
	g_return_if_fail (iter->user_data != NULL);

	g_mutex_lock (list_model->folder_lock);

	g_mutex_lock (list_model->iterator_lock);

	header = iter->user_data;
	
	if (G_LIKELY (header))
		tny_msg_header_iface_uncache (header);

	g_mutex_unlock (list_model->iterator_lock);

	g_mutex_unlock (list_model->folder_lock);

	return;
}

static void
tny_msg_header_list_model_ref_node (GtkTreeModel *self, GtkTreeIter  *iter)
{
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
unref_header (gpointer data, gpointer user_data)
{
	g_object_unref (G_OBJECT (data));
	return;
}

static void
ref_header (gpointer data, gpointer user_data)
{
	g_object_ref (G_OBJECT (data));
	return;
}


static void
tny_msg_header_list_model_prepend (TnyListIface *self, gpointer item)
{
	TnyMsgHeaderListModel *me = (TnyMsgHeaderListModel*)self;

	g_mutex_lock (me->iterator_lock);
	me->first = g_list_prepend (me->first, item);
	g_mutex_unlock (me->iterator_lock);

}

static void
tny_msg_header_list_model_append (TnyListIface *self, gpointer item)
{
	TnyMsgHeaderListModel *me = (TnyMsgHeaderListModel*)self;

	g_mutex_lock (me->iterator_lock);
	me->first = g_list_append (me->first, item);
	g_mutex_unlock (me->iterator_lock);

}

static void
tny_msg_header_list_model_remove (TnyListIface *self, gpointer item)
{
	TnyMsgHeaderListModel *me = (TnyMsgHeaderListModel*)self;

	g_mutex_lock (me->iterator_lock);
	me->first = g_list_remove (me->first, (gconstpointer)item);
	g_mutex_unlock (me->iterator_lock);

}

static TnyIteratorIface*
tny_msg_header_list_model_create_iterator (TnyListIface *self)
{
	TnyMsgHeaderListModel *me = (TnyMsgHeaderListModel*)self;

	return TNY_ITERATOR_IFACE (_tny_msg_header_list_iterator_new (me));
}

static TnyListIface*
tny_msg_header_list_model_copy_the_list (TnyListIface *self)
{
	TnyMsgHeaderListModel *me = (TnyMsgHeaderListModel*)self;
	TnyMsgHeaderListModel *copy = g_object_new (TNY_TYPE_MSG_HEADER_LIST_MODEL, NULL);

	/* This only copies the TnyListIface pieces */

	g_mutex_lock (me->iterator_lock);
	GList *list_copy = g_list_copy (me->first);
	copy->first = list_copy;
	g_mutex_unlock (me->iterator_lock);

	return TNY_LIST_IFACE (copy);
}

static void 
tny_msg_header_list_model_foreach_in_the_list (TnyListIface *self, GFunc func, gpointer user_data)
{
	TnyMsgHeaderListModel *me = (TnyMsgHeaderListModel*)self;

	g_mutex_lock (me->iterator_lock);
	g_list_foreach (me->first, func, user_data);
	g_mutex_unlock (me->iterator_lock);

	return;
}

static void
tny_list_iface_init (TnyListIfaceClass *klass)
{
	klass->prepend_func = tny_msg_header_list_model_prepend;
	klass->append_func = tny_msg_header_list_model_append;
	klass->remove_func = tny_msg_header_list_model_remove;
	klass->create_iterator_func = tny_msg_header_list_model_create_iterator;
	klass->copy_func = tny_msg_header_list_model_copy_the_list;
	klass->foreach_func = tny_msg_header_list_model_foreach_in_the_list;

	return;
}



static void
tny_msg_header_list_model_finalize (GObject *object)
{
	TnyMsgHeaderListModel *self = (TnyMsgHeaderListModel *)object;

	g_mutex_lock (self->folder_lock);

	/* We have to unreference all */
	if (self->folder) 
	{
		tny_msg_header_list_model_foreach_in_the_list 
			(TNY_LIST_IFACE (self), unref_header, NULL);
		g_object_unref (G_OBJECT (self->folder));
	}

	g_mutex_lock (self->iterator_lock);
	if (self->iterator)
		g_object_unref (G_OBJECT (self->iterator));
	g_mutex_unlock (self->iterator_lock);

	g_mutex_unlock (self->folder_lock);

	g_mutex_free (self->folder_lock);
	self->folder_lock = NULL;

	g_mutex_free (self->iterator_lock);
	self->iterator_lock = NULL;

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
	self->folder = NULL;
	self->folder_lock = g_mutex_new ();
	self->iterator_lock = g_mutex_new ();
	self->first = NULL;

	return;
}


/**
 * tny_msg_header_list_model_set_folder:
 * @self: A #TnyMsgHeaderListModel instance
 * @folder: a #TnyMsgFolderIface instance
 *
 * Set the folder where the #TnyMsgHeaderIface instances are located
 * 
 **/

void
tny_msg_header_list_model_set_folder (TnyMsgHeaderListModel *self, TnyMsgFolderIface *folder)
{
	const GList* headers;

	g_mutex_lock (self->folder_lock);
	
	self->iterator = TNY_ITERATOR_IFACE (_tny_msg_header_list_iterator_new (self));

	g_mutex_lock (self->iterator_lock);

	if (G_LIKELY (self->folder))
		g_object_unref (G_OBJECT (self->folder));

	self->folder = folder;
	g_object_ref (G_OBJECT (folder));

	/* To avoid a g_list_length (note that the implementation must
	 * absolutely be correct!) */
	self->length = tny_msg_folder_iface_get_all_count (folder);

	/* We add a reference to each header instance because this type
	   references it (needs it) using the tree-iter token. */
	g_mutex_unlock (self->iterator_lock);

	tny_msg_header_list_model_foreach_in_the_list 
		(TNY_LIST_IFACE (self), ref_header, NULL);

	
	g_mutex_unlock (self->folder_lock);

	return;
}

/**
 * tny_msg_header_list_model_new:
 *
 *
 * Return value: a new #GtkTreeModel instance suitable for showing lots of 
 * #TnyMsgHeaderIface instances
 **/
GtkTreeModel *
tny_msg_header_list_model_new (void)
{
	TnyMsgHeaderListModel *model;

	model = g_object_new (TNY_TYPE_MSG_HEADER_LIST_MODEL, NULL);

	return GTK_TREE_MODEL (model);
}

GType
tny_msg_header_list_model_get_type (void)
{
	static GType object_type = 0;

	if (G_UNLIKELY(object_type == 0))
	{
		static const GTypeInfo object_info = 
		{
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
		

		static const GInterfaceInfo tny_list_iface_info = {
			(GInterfaceInitFunc) tny_list_iface_init,
			NULL,
			NULL
		};

		object_type = g_type_register_static (G_TYPE_OBJECT, 
						"TnyMsgHeaderListModel", &object_info, 0);

		g_type_add_interface_static (object_type, GTK_TYPE_TREE_MODEL,
					     &tree_model_info);

		g_type_add_interface_static (object_type, TNY_TYPE_LIST_IFACE,
					     &tny_list_iface_info);

	}

	return object_type;
}

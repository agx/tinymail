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

#define INDEX_OFFSET 64
#define INDEX_THRESHOLD 5000

static GObjectClass *parent_class;

#include "tny-msg-header-list-model-priv.h"
#include "tny-msg-header-list-iterator-priv.h"

/* The function is locked, so this is secure (and might cause less
stack allocations)? It's probably a naïve manual optimization.*/


G_INLINE_FUNC void /* When sorting, this method is called a gazillion times */
_tny_msg_header_list_iterator_travel_to_nth_nl (TnyMsgHeaderListIterator *self, register guint cur, register guint nth)
{

  /* If we have a usable index table, we'll use it. This happens when as only
     add operation the prepend has happened more than INDEX_THRESHOLD times.
     This will trigger on folders larger than INDEX_THRESHOLD headers that have
     never had appends nor removes of headers.

     Therefore, TODO: rapidly update the index table on append and remove. ;-) 

     Currently, the only adder that keeps the index correct is the prepend.
     The other ones will set usable_index FALSE. Hev phun .. */
//>>6 would be the / and &0x3F
  if G_UNLIKELY (self->model->usable_index)
  {
	/* register guint idx = nth / INDEX_OFFSET, remain = nth % INDEX_OFFSET, cidx = 0; */
	register guint idx = nth >> 6, remain = nth & 0x3F, cidx = 0;

	register GList *start = self->model->index, *ret;

	/* Math seems faster than walking a next pointer 1000ths of times */

	if G_LIKELY (idx)
	{       
		/* If destination is not in the beginning of the list */

		while G_LIKELY (cidx++ < idx-1)
			start = start->next;
		ret = start->data;

	} else  ret = self->model->first;

	/* So we are now at 0 or at (50 * index), walk the remainder */

	while G_LIKELY (remain--)
		ret = ret->next;

	self->current = ret;

  } else {
	
	/* First we check whether we are walking somewhere in the beginning 
	   of the list: We know the first location, so ... (faster to start 
	   from the first than from the current) */

	if G_UNLIKELY (nth - cur < nth)
	{
		self->current = self->model->first;
		cur = 0;
	}

	/* If the current location is less than the destination location */
	if G_LIKELY (cur < nth)
		while G_LIKELY (cur++ < nth)
			self->current = self->current->next;

	/* And if not ... */
	else if G_LIKELY (cur > nth)
		while G_LIKELY (cur-- > nth)
			self->current = self->current->prev;
  }

  return;
}

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

	/* Fill an GtkTreeIter (this is not a TnyIteratorIface!) using a path */

	g_mutex_lock (list_model->iterator_lock);

	i = gtk_tree_path_get_indices (path)[0];

	if (G_UNLIKELY (i >= list_model->length))
	{
		g_mutex_unlock (list_model->iterator_lock);
		g_mutex_unlock (list_model->folder_lock);
		return FALSE;
	}

	/* We walk the iter to the nth position and we store the current postition */
	_tny_msg_header_list_iterator_travel_to_nth_nl
		((TnyMsgHeaderListIterator*)list_model->iterator, 
		list_model->last_nth, i);

	/* We will store this as user_data of the GtkTreeIter */
	ptr = _tny_msg_header_list_iterator_current_nl ((TnyMsgHeaderListIterator*)list_model->iterator);
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

	/* Return the path of an existing GtkTreeIter */

	if  (!(iter->stamp == TNY_MSG_HEADER_LIST_MODEL (self)->stamp))
		return NULL;

	g_mutex_lock (list_model->folder_lock);
	g_mutex_lock (list_model->iterator_lock);

	while (_tny_msg_header_list_iterator_has_next_nl ((TnyMsgHeaderListIterator*)list_model->iterator))
	{
		if (_tny_msg_header_list_iterator_next_nl ((TnyMsgHeaderListIterator*)list_model->iterator) == iter->user_data)
			break;
		i++;
	}

	/* Reset the internal iterator */
	((TnyMsgHeaderListIterator*)list_model->iterator)->current = list_model->first;
	list_model->last_nth = 0;

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

	/* Remember the ptr we've set above? Get the header instance out of the
	   token and return the asked-for column as a GValue instance. */

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
	GList *headers; gpointer ptr;

	/* Move the GtkTreeIter to the next item */

	g_return_val_if_fail (iter->stamp == TNY_MSG_HEADER_LIST_MODEL 
		(self)->stamp, FALSE);

	g_mutex_lock (list_model->folder_lock);
	g_mutex_lock (list_model->iterator_lock);

	/* We simply move the iterator and get the value */
	ptr = _tny_msg_header_list_iterator_next_nl ((TnyMsgHeaderListIterator*)list_model->iterator);
	list_model->last_nth++;
	iter->user_data = ptr;
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

	/* Return the amount of children for this GtkTreeIter. Because this
	   is a flat list and has_child is always FALSE, we'll just always
	   return the full length. */

	g_mutex_lock (list_model->folder_lock);
	g_mutex_lock (list_model->iterator_lock);

	if (G_LIKELY (!iter))
		retval = list_model->length;

	g_mutex_unlock (list_model->iterator_lock);
	g_mutex_unlock (list_model->folder_lock);

	return retval;
}

static gboolean
tny_msg_header_list_model_iter_nth_child (GtkTreeModel *self, GtkTreeIter *iter, GtkTreeIter *parent, gint n)
{
	GList *child, *headers;
	TnyMsgHeaderListModel *list_model = TNY_MSG_HEADER_LIST_MODEL (self);
	GList *restore;

	if (G_UNLIKELY (parent))
		return FALSE;

	g_mutex_lock (list_model->folder_lock);
	g_mutex_lock (list_model->iterator_lock);

	restore = ((TnyMsgHeaderListIterator*)list_model->iterator)->current;
	/* Move the GtkTreeIter to the nth child */
	child = _tny_msg_header_list_iterator_nth_nl ((TnyMsgHeaderListIterator*)list_model->iterator, n);

	if (G_LIKELY (child))
	{
		list_model->last_nth = n;
		iter->stamp = TNY_MSG_HEADER_LIST_MODEL (self)->stamp;
		iter->user_data = child;

		g_mutex_unlock (list_model->iterator_lock);
		g_mutex_unlock (list_model->folder_lock);

		return TRUE;
	}

	((TnyMsgHeaderListIterator*)list_model->iterator)->current = restore;

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

	/* Unref node happens when the GtkTreeView no longer needs the 
	   reference to the GtkTreeIter (nor its user_data) */

	if (!iter->user_data);
		return;

	g_mutex_lock (list_model->folder_lock);
	g_mutex_lock (list_model->iterator_lock);

	header = iter->user_data;

	/* We can use the knowledge that it no longer needs the reference,
	   to uncache the instance. Uncached instances are instances that
	   typically no longer have their real subject inmem. Next time they'll
	   get a property-request, they'll create a new real subject (which
	   takes a certain amount of time) and will cache that before replying
	   the request using the real subject. */

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
	GtkTreePath *path;
	GtkTreeIter iter;

	path = gtk_tree_path_new ();
	gtk_tree_path_append_index (path, 0);
	iter.stamp = me->stamp;
	iter.user_data = item;

	/* Prepend something to the list */

	g_mutex_lock (me->iterator_lock);
	me->first = g_list_prepend (me->first, item);
	me->length++;

	if G_UNLIKELY (me->length % INDEX_OFFSET == 0)
		me->index = g_list_append (me->index, me->first);

	if G_UNLIKELY (!me->usable_index && me->length >= INDEX_THRESHOLD)
		me->usable_index = TRUE;

	g_object_ref (G_OBJECT (item));

	/* Reset the internal iterator */
	((TnyMsgHeaderListIterator*)me->iterator)->current = me->first;
	me->last_nth = 0;
	g_mutex_unlock (me->iterator_lock);

	/* Letting the observers know about this (the GtkTreeView) */
	gtk_tree_model_row_inserted (GTK_TREE_MODEL (me), path, &iter);
	gtk_tree_path_free (path);
}

static void
tny_msg_header_list_model_append (TnyListIface *self, gpointer item)
{
	TnyMsgHeaderListModel *me = (TnyMsgHeaderListModel*)self;
	GtkTreePath *path;
	GtkTreeIter iter;

	path = gtk_tree_path_new ();
	iter.stamp = me->stamp;
	iter.user_data = item;

	/* Append something to the list */

	g_mutex_lock (me->iterator_lock);
	me->first = g_list_append (me->first, item);
	me->length++;

	me->usable_index = FALSE;
	if (me->index)
		g_list_free (me->index);
	me->index = NULL;

	g_object_ref (G_OBJECT (item));

	/* Reset the internal iterator */
	((TnyMsgHeaderListIterator*)me->iterator)->current = me->first;
	me->last_nth = 0;
	g_mutex_unlock (me->iterator_lock);

	/* Letting the observers know about this (the GtkTreeView) */
	gtk_tree_path_append_index (path, me->length-1);
	gtk_tree_model_row_inserted (GTK_TREE_MODEL (me), path, &iter);
	gtk_tree_path_free (path);

}

static guint
tny_msg_header_list_model_length (TnyListIface *self)
{
	TnyMsgHeaderListModel *me = (TnyMsgHeaderListModel*)self;
	guint retval = 0;

	g_mutex_lock (me->iterator_lock);
	retval = me->first?g_list_length (me->first):0;
	g_mutex_unlock (me->iterator_lock);

	return retval;
}

static void
tny_msg_header_list_model_remove (TnyListIface *self, gpointer item)
{
	TnyMsgHeaderListModel *me = (TnyMsgHeaderListModel*)self;
	GtkTreePath *path;
	GtkTreeIter iter;

	iter.stamp = me->stamp;
	iter.user_data = item;
	path = tny_msg_header_list_model_get_path (GTK_TREE_MODEL (me), &iter);

	/* Remove something from the list */

	g_mutex_lock (me->iterator_lock);
	me->first = g_list_remove (me->first, (gconstpointer)item);
	me->length--;
	me->usable_index = FALSE;
	if (me->index)
		g_list_free (me->index);
	me->index = NULL;
	g_object_unref (G_OBJECT (item));

	/* Reset the internal iterator */
	((TnyMsgHeaderListIterator*)me->iterator)->current = me->first;
	me->last_nth = 0;
	g_mutex_unlock (me->iterator_lock);

	/* Letting the observers know about this (the GtkTreeView) */
	gtk_tree_model_row_deleted (GTK_TREE_MODEL (me), path);
	gtk_tree_path_free (path);

}

static TnyIteratorIface*
tny_msg_header_list_model_create_iterator (TnyListIface *self)
{
	TnyMsgHeaderListModel *me = (TnyMsgHeaderListModel*)self;

	/* Return a new iterator */

	return TNY_ITERATOR_IFACE (_tny_msg_header_list_iterator_new (me, TRUE));
}

static TnyListIface*
tny_msg_header_list_model_copy_the_list (TnyListIface *self)
{
	TnyMsgHeaderListModel *me = (TnyMsgHeaderListModel*)self;
	TnyMsgHeaderListModel *copy = g_object_new (TNY_TYPE_MSG_HEADER_LIST_MODEL, NULL);

	/* This only copies the TnyListIface pieces. The result is not a
	   correct or good TnyMsgHeaderListModel. But it will be a correct
	   TnyListIface instance. It is the only thing the user of this
	   method expects.

	   The new list will point to the same instances, of course. It's
	   only a copy of the list-nodes of course. */

	g_mutex_lock (me->iterator_lock);
	GList *list_copy = g_list_copy (me->first);
	copy->first = list_copy;
	copy->usable_index = FALSE;
	g_mutex_unlock (me->iterator_lock);

	return TNY_LIST_IFACE (copy);
}

static void 
tny_msg_header_list_model_foreach_in_the_list (TnyListIface *self, GFunc func, gpointer user_data)
{
	TnyMsgHeaderListModel *me = (TnyMsgHeaderListModel*)self;

	/* Foreach item in the list (without using a slower iterator) */

	g_mutex_lock (me->iterator_lock);
	g_list_foreach (me->first, func, user_data);
	g_mutex_unlock (me->iterator_lock);

	return;
}

static void
tny_list_iface_init (TnyListIfaceClass *klass)
{
	klass->length_func = tny_msg_header_list_model_length;
	klass->prepend_func = tny_msg_header_list_model_prepend;
	klass->append_func = tny_msg_header_list_model_append;
	klass->remove_func = tny_msg_header_list_model_remove;
	klass->create_iterator_func = tny_msg_header_list_model_create_iterator;
	klass->copy_func = tny_msg_header_list_model_copy_the_list;
	klass->foreach_func = tny_msg_header_list_model_foreach_in_the_list;

	return;
}


/* The "relaxed performers" uses g_idle to in a relax way perform a certain
   function on each item in the list of headers  */

typedef struct
{
	GList *list;
	GFunc relaxed_func;
} RelaxedData;


static void
tny_msg_header_list_model_relaxed_data_destroyer (gpointer data)
{
	RelaxedData *d = data;

	/* The destroyer */

	g_list_free (d->list);
	d->list = NULL;
	g_free (d);

	return;
}

static gboolean
tny_msg_header_list_model_relaxed_performer (gpointer data)
{
	RelaxedData *d = data;
	GList *list = d->list;
	gint count = 0;

	/* The performer itself */

	while ((count < 5) && list)
	{
		GList *element = list;
		if (element && element->data)
			d->relaxed_func (element->data, NULL);
		list = g_list_remove_link (list, element);
		g_list_free (element);
		count++;
	}

	d->list = list;

	if (count <= 1)
		return FALSE;

	return TRUE;
}

static void 
proxy_uncache_func (gpointer data, gpointer user_data)
{
	if (data)
		tny_msg_header_iface_uncache (TNY_MSG_HEADER_IFACE (data));
	return;
}


static void 
tny_msg_header_list_model_hdr_cache_uncacher_copy (TnyMsgHeaderListModel *self)
{
	RelaxedData *d = g_new (RelaxedData, 1);

	/* This one will perform an uncache on each item in the list. It uses 
	   a copy of the list. */

	d->relaxed_func = (GFunc)proxy_uncache_func;
	d->list = g_list_copy (self->first);

	g_idle_add_full (G_PRIORITY_LOW, tny_msg_header_list_model_relaxed_performer, 
		d, tny_msg_header_list_model_relaxed_data_destroyer);

	return;
}

static void
proxy_destroy_func (gpointer data, gpointer user_data)
{
	if (data)
		g_object_unref (G_OBJECT (data));
	data = NULL;

	return;
}

static void 
tny_msg_header_list_model_hdr_cache_remover (TnyMsgHeaderListModel *self)
{
	RelaxedData *d = g_new (RelaxedData, 1);

	/* This one will perform a destruction of each item in the list. It uses 
	   the original list. */

	d->relaxed_func = (GFunc)proxy_destroy_func;
	d->list = self->first;

	g_idle_add_full (G_PRIORITY_LOW, tny_msg_header_list_model_relaxed_performer, 
		d, tny_msg_header_list_model_relaxed_data_destroyer);

	return;
} 


static void 
tny_msg_header_list_model_hdr_cache_remover_copy (TnyMsgHeaderListModel *self)
{
	RelaxedData *d = g_new (RelaxedData, 1);

	/* This one will perform a destruction of each item in the list. It uses 
	   a copy of the list. */

	d->relaxed_func = (GFunc)proxy_destroy_func;
	d->list = g_list_copy (self->first);

	g_idle_add_full (G_PRIORITY_LOW, tny_msg_header_list_model_relaxed_performer, 
		d, tny_msg_header_list_model_relaxed_data_destroyer);

	return;
} 

static void
tny_msg_header_list_model_finalize (GObject *object)
{
	TnyMsgHeaderListModel *self = (TnyMsgHeaderListModel *)object;

	g_mutex_lock (self->folder_lock);
	g_mutex_lock (self->iterator_lock);

	/* Reset the internal iterator */
	self->length = 0;
	((TnyMsgHeaderListIterator*)self->iterator)->current = self->first;
	self->last_nth = 0;

	/* Unreference the headers */
	if (self->first)
	{
		tny_msg_header_list_model_hdr_cache_remover_copy (self);
		g_list_free (self->first);
		self->first = NULL;
	}

	self->usable_index = FALSE;
	if (self->index)
	{
		g_list_free (self->index);
		self->index = NULL;
	}

	/* Unreference the folder instance */
	if (self->folder) 
	{
		g_object_unref (G_OBJECT (self->folder));
		if (self->iterator)
			g_object_unref (G_OBJECT (self->iterator));
	}

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
	self->length = 0;
	self->usable_index = FALSE;

	/* This is an internal iterator used by the GtkTreeModel implementation */
	self->iterator = TNY_ITERATOR_IFACE (_tny_msg_header_list_iterator_new (self, FALSE));

	return;
}


/**
 * tny_msg_header_list_model_set_folder:
 * @self: A #TnyMsgHeaderListModel instance
 * @folder: a #TnyMsgFolderIface instance
 * @refresh: refresh first
 *
 * Set the folder where the #TnyMsgHeaderIface instances are located
 * 
 **/
void
tny_msg_header_list_model_set_folder (TnyMsgHeaderListModel *self, TnyMsgFolderIface *folder, gboolean refresh)
{
	GtkTreeIter iter;
	GtkTreePath *path;

	iter.stamp = self->stamp;
	iter.user_data = self->first;
	path = tny_msg_header_list_model_get_path (GTK_TREE_MODEL (self), &iter);

	g_mutex_lock (self->folder_lock);
	g_mutex_lock (self->iterator_lock);


	/* Unreference the previous headers */
	if (self->first)
	{
		tny_msg_header_list_model_hdr_cache_remover_copy (self);
		g_list_free (self->first);
	}
	self->first = NULL;

	/* Unreference the previous folder instance */
	if (G_LIKELY (self->folder))
		g_object_unref (G_OBJECT (self->folder));

	/* Reset the internal iterator */
	self->length = 0;
	((TnyMsgHeaderListIterator*)self->iterator)->current = self->first;
	self->last_nth = 0;

	g_mutex_unlock (self->iterator_lock);
	g_mutex_unlock (self->folder_lock);

	/* Get a new list of headers */
	tny_msg_folder_iface_get_headers (folder, TNY_LIST_IFACE (self), refresh);

	g_mutex_lock (self->folder_lock);
	g_mutex_lock (self->iterator_lock);

	/* Reset the internal iterator */
	((TnyMsgHeaderListIterator*)self->iterator)->current = self->first;
	self->last_nth = 0;

	/* Reference the new folder instance */
	g_object_ref (G_OBJECT (folder));
	self->folder = folder;

	g_mutex_unlock (self->iterator_lock);
	g_mutex_unlock (self->folder_lock);	

	/* Notify the observers (the GtkTreeView) */
	gtk_tree_path_append_index (path, 0);
	gtk_tree_model_row_inserted (GTK_TREE_MODEL (self), path, &iter);
	gtk_tree_path_free (path);

	return;
}

/**
 * tny_msg_header_list_model_new:
 *
 *
 * Return value: a new #TnyMsgHeaderListModel instance suitable for showing lots of 
 * #TnyMsgHeaderIface instances
 **/
TnyMsgHeaderListModel*
tny_msg_header_list_model_new (void)
{
	TnyMsgHeaderListModel *model;

	model = g_object_new (TNY_TYPE_MSG_HEADER_LIST_MODEL, NULL);
	
	return model;
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

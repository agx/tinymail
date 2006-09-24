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

/*#ifndef DEBUG
#define G_IMPLEMENTS_INLINES
#endif*/

#include <config.h>

#include <glib.h>
#include <glib/gi18n-lib.h>

#include <tny-gtk-header-list-model.h>
#include <tny-gtk-header-list-iterator-priv.h>

#include <tny-header.h>
#include <tny-folder.h>

#include <tny-list.h>
#include <tny-iterator.h>

#define INDEX_OFFSET 64
#define INDEX_THRESHOLD 5000

static GObjectClass *parent_class;

#include "tny-gtk-header-list-iterator-priv.h"

/* The function is locked, so this is secure (and might cause less
stack allocations)? It's probably a naïve manual optimization.*/


void /* When sorting, this method is called a gazillion times */
_tny_gtk_header_list_iterator_travel_to_nth_nl (TnyGtkHeaderListIterator *self, guint cur, guint nth)
{

  /* If we have a usable index table, we'll use it. This happens when as only
     add operation the prepend has happened more than INDEX_THRESHOLD times.
     This will trigger on folders larger than INDEX_THRESHOLD headers that have
     never had appends nor removes of headers.

     Therefore, TODO: rapidly update the index table on append and remove. ;-) 

     Currently, the only adder that keeps the index correct is the prepend.
     The other ones will set usable_index FALSE. Hev phun .. */

  if G_UNLIKELY (self->model->usable_index)
  {
	/* register guint idx = nth / INDEX_OFFSET, remain = nth % INDEX_OFFSET, cidx = 0; */
	guint idx = nth >> 6, remain = nth & 0x3F, cidx = 0;
	GList *start = self->model->index, *ret;

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


	if (G_UNLIKELY (nth - cur < nth) || G_UNLIKELY (nth == 0))
	{
		self->current = self->model->first;
		cur = 0;
	}

	/* If nth is 0, then we are fine. Leave it then. */
	if (G_LIKELY (nth != 0))
	{

		/* If the current location is less than the destination location */
		if G_LIKELY (cur < nth)
			while G_LIKELY (cur++ < nth)
				self->current = self->current->next;

		/* And if not ... */
		else if G_LIKELY (cur > nth)
			while G_LIKELY (cur-- > nth)
				if (G_LIKELY (self->current))
					self->current = self->current->prev;
				else 
				{	
					/* This is a strange case, but it means that we must 
					   be at the first item */

					self->current = self->model->first; 
					break; 
				}
	}
  }

  return;
}

static guint
tny_gtk_header_list_model_get_flags (GtkTreeModel *self)
{
	return 0;
}

static gint
tny_gtk_header_list_model_get_n_columns (GtkTreeModel *self)
{
	return TNY_GTK_HEADER_LIST_MODEL_N_COLUMNS;
}

static GType
tny_gtk_header_list_model_get_column_type (GtkTreeModel *self, gint column)
{
	GType retval;

	switch (column) 
	{
		case TNY_GTK_HEADER_LIST_MODEL_CC_COLUMN:
		case TNY_GTK_HEADER_LIST_MODEL_DATE_SENT_COLUMN:
		case TNY_GTK_HEADER_LIST_MODEL_DATE_RECEIVED_COLUMN:
		case TNY_GTK_HEADER_LIST_MODEL_TO_COLUMN:
		case TNY_GTK_HEADER_LIST_MODEL_FROM_COLUMN:
		case TNY_GTK_HEADER_LIST_MODEL_SUBJECT_COLUMN:
			retval = G_TYPE_STRING;
			break;
		case TNY_GTK_HEADER_LIST_MODEL_INSTANCE_COLUMN:
			retval = G_TYPE_OBJECT;
			break;
		case TNY_GTK_HEADER_LIST_MODEL_DATE_SENT_TIME_T_COLUMN:
		case TNY_GTK_HEADER_LIST_MODEL_DATE_RECEIVED_TIME_T_COLUMN:
		case TNY_GTK_HEADER_LIST_MODEL_FLAGS_COLUMN:
			retval = G_TYPE_INT;
			break;
		default:
			retval = G_TYPE_INVALID;
			break;
	}


	return retval;
}


static gboolean
tny_gtk_header_list_model_get_iter (GtkTreeModel *self, GtkTreeIter *iter, GtkTreePath *path)
{
	TnyGtkHeaderListModel *list_model = TNY_GTK_HEADER_LIST_MODEL (self);
	gint i; gpointer ptr;
	gboolean retval=FALSE;

	g_return_val_if_fail (gtk_tree_path_get_depth (path) > 0, FALSE);

	g_mutex_lock (list_model->folder_lock);

	/* Fill an GtkTreeIter (this is not a TnyIterator!) using a path */

	g_mutex_lock (list_model->iterator_lock);

	i = gtk_tree_path_get_indices (path)[0];

	if (G_UNLIKELY (i >= list_model->length))
	{
		g_mutex_unlock (list_model->iterator_lock);
		g_mutex_unlock (list_model->folder_lock);
		return FALSE;
	}

	/* We walk the iter to the nth position and we store the current postition */
	_tny_gtk_header_list_iterator_travel_to_nth_nl
		((TnyGtkHeaderListIterator*)list_model->iterator, 
		list_model->last_nth, i);

	/* We will store this as user_data of the GtkTreeIter */
	/* don't unref */	
	ptr = _tny_gtk_header_list_iterator_get_current_nl ((TnyGtkHeaderListIterator*)list_model->iterator);
	list_model->last_nth = i;
	iter->stamp = list_model->stamp;
	iter->user_data = ptr;
	retval = (iter->user_data != NULL);
	
	g_mutex_unlock (list_model->iterator_lock);

	
	g_mutex_unlock (list_model->folder_lock);

	return retval;
}

static GtkTreePath *
tny_gtk_header_list_model_get_path (GtkTreeModel *self, GtkTreeIter *iter)
{
	GtkTreePath *tree_path;
	gint i = 0;
	TnyGtkHeaderListModel *list_model = TNY_GTK_HEADER_LIST_MODEL (self);

	/* Return the path of an existing GtkTreeIter */

	if  (!(iter->stamp == TNY_GTK_HEADER_LIST_MODEL (self)->stamp))
		return NULL;

	g_mutex_lock (list_model->folder_lock);
	g_mutex_lock (list_model->iterator_lock);

	while (!_tny_gtk_header_list_iterator_is_done_nl ((TnyGtkHeaderListIterator*)list_model->iterator))
	{
		/* header list iterator does not need to be unref'd */
		if (_tny_gtk_header_list_iterator_get_current_nl ((TnyGtkHeaderListIterator*)list_model->iterator) == iter->user_data)
			break;
		
		_tny_gtk_header_list_iterator_next_nl ((TnyGtkHeaderListIterator*)list_model->iterator);

		i++;
	}

	/* Reset the internal iterator */
	((TnyGtkHeaderListIterator*)list_model->iterator)->current = list_model->first;
	list_model->last_nth = 0;

	tree_path = gtk_tree_path_new ();
	gtk_tree_path_append_index (tree_path, i);

	g_mutex_unlock (list_model->iterator_lock);

	g_mutex_unlock (list_model->folder_lock);

	return tree_path;
}


gchar *
_get_readable_date (time_t file_time_raw)
{
	struct tm *file_time;
	static gchar readable_date[64];
	gsize readable_date_size;

	file_time = localtime (&file_time_raw);

	readable_date_size = strftime (readable_date, 63, _("%Y-%m-%d, %-I:%M %p"), file_time);		
	
	return readable_date;
}

/**
 * tny_gtk_header_list_model_received_date_sort_func:
 * @model: The GtkTreeModel the comparison is within
 * @a : A GtkTreeIter in model
 * @b : Another GtkTreeIter in model
 * @user_data: Data passed
 *
 * A GtkTreeIterCompareFunc that sorts using the received date
 *
 * Return value: a negative integer, zero, or a positive integer 
 **/
gint 
tny_gtk_header_list_model_received_date_sort_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data)
{
	TnyHeader *hdr_a, *hdr_b;
	time_t recv_a, recv_b;

	hdr_a = a->user_data;
	hdr_b = b->user_data;

	recv_a = tny_header_get_date_received (hdr_a);
	recv_b = tny_header_get_date_received (hdr_b);

	return (recv_a - recv_b);
}

/**
 * tny_gtk_header_list_model_sent_date_sort_func:
 * @model: The GtkTreeModel the comparison is within
 * @a : A GtkTreeIter in model
 * @b : Another GtkTreeIter in model
 * @user_data: Data passed
 *
 * A GtkTreeIterCompareFunc that sorts using the sent date
 *
 * Return value: a negative integer, zero, or a positive integer 
 **/
gint  
tny_gtk_header_list_model_sent_date_sort_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data)
{
	TnyHeader *hdr_a, *hdr_b;
	time_t recv_a, recv_b;

	hdr_a = a->user_data;
	hdr_b = b->user_data;

	recv_a = tny_header_get_date_sent (hdr_a);
	recv_b = tny_header_get_date_sent (hdr_b);

	return (recv_a - recv_b);
}

static void
tny_gtk_header_list_model_get_value (GtkTreeModel *self, GtkTreeIter *iter, gint column, GValue *value)
{
	TnyHeader *header = NULL;
	TnyGtkHeaderListModel *list_model = TNY_GTK_HEADER_LIST_MODEL (self);

	g_return_if_fail (iter->stamp == TNY_GTK_HEADER_LIST_MODEL (self)->stamp);

	if (iter->user_data == NULL || !TNY_IS_HEADER (iter->user_data))
		return;
        
	g_mutex_lock (list_model->folder_lock);
	g_mutex_lock (list_model->iterator_lock);

	/* Remember the ptr we've set above? Get the header instance out of the
	   token and return the asked-for column as a GValue instance. */

	header = iter->user_data;
	
	switch (column) 
	{
		case TNY_GTK_HEADER_LIST_MODEL_CC_COLUMN:
			g_value_init (value, G_TYPE_STRING);
			g_value_set_string (value, tny_header_get_cc (header));
			break;
		case TNY_GTK_HEADER_LIST_MODEL_DATE_SENT_COLUMN:
			g_value_init (value, G_TYPE_STRING);
			g_value_set_string (value, 
				_get_readable_date (tny_header_get_date_sent (header)));
			break;
		case TNY_GTK_HEADER_LIST_MODEL_DATE_RECEIVED_COLUMN:
			g_value_init (value, G_TYPE_STRING);
			g_value_set_string (value, 
				_get_readable_date (tny_header_get_date_received (header)));
			break;
		case TNY_GTK_HEADER_LIST_MODEL_DATE_SENT_TIME_T_COLUMN:
			g_value_init (value, G_TYPE_INT);
			g_value_set_int (value, 
					    tny_header_get_date_sent (header));
			break;
		case TNY_GTK_HEADER_LIST_MODEL_DATE_RECEIVED_TIME_T_COLUMN:
			g_value_init (value, G_TYPE_INT);
			g_value_set_int (value, 
					 tny_header_get_date_received (header));
			break;
		case TNY_GTK_HEADER_LIST_MODEL_INSTANCE_COLUMN:
			g_value_init (value, G_TYPE_OBJECT);
			g_value_set_object (value, header);
			break;
		case TNY_GTK_HEADER_LIST_MODEL_TO_COLUMN:
			g_value_init (value, G_TYPE_STRING);
			g_value_set_string (value, tny_header_get_to (header));
			break;
		case TNY_GTK_HEADER_LIST_MODEL_SUBJECT_COLUMN:
			g_value_init (value, G_TYPE_STRING);
			g_value_set_string (value, tny_header_get_subject (header));
			break;
		case TNY_GTK_HEADER_LIST_MODEL_FROM_COLUMN:
			g_value_init (value, G_TYPE_STRING);
			g_value_set_string (value, tny_header_get_from (header));
			break;
		case TNY_GTK_HEADER_LIST_MODEL_FLAGS_COLUMN:
			g_value_init (value, G_TYPE_INT);
			g_value_set_int (value, tny_header_get_flags (header));
			break;
		default:
			break;
	}

	g_mutex_unlock (list_model->iterator_lock);
	g_mutex_unlock (list_model->folder_lock);

	return;
}

static gboolean
tny_gtk_header_list_model_iter_next (GtkTreeModel *self, GtkTreeIter *iter)
{
	gboolean retval;
	TnyGtkHeaderListModel *list_model = TNY_GTK_HEADER_LIST_MODEL (self);
	gpointer ptr;

	/* Move the GtkTreeIter to the next item */

	g_return_val_if_fail (iter->stamp == TNY_GTK_HEADER_LIST_MODEL 
		(self)->stamp, FALSE);

	g_mutex_lock (list_model->folder_lock);
	g_mutex_lock (list_model->iterator_lock);

	/* We simply move the iterator and get the value */
	_tny_gtk_header_list_iterator_next_nl ((TnyGtkHeaderListIterator*)list_model->iterator);
	/* the ptr needs not to be unref'd... */
	ptr = _tny_gtk_header_list_iterator_get_current_nl ((TnyGtkHeaderListIterator*)list_model->iterator);
	list_model->last_nth++;
	iter->user_data = ptr;
	retval = (iter->user_data != NULL);

	g_mutex_unlock (list_model->iterator_lock);
	g_mutex_unlock (list_model->folder_lock);

	return retval;
}

static gboolean
tny_gtk_header_list_model_iter_has_child (GtkTreeModel *self, GtkTreeIter *iter)
{
	return FALSE;
}

static gint
tny_gtk_header_list_model_iter_n_children (GtkTreeModel *self, GtkTreeIter *iter)
{
	gint retval = -1;
	TnyGtkHeaderListModel *list_model = TNY_GTK_HEADER_LIST_MODEL (self);

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
tny_gtk_header_list_model_iter_nth_child (GtkTreeModel *self, GtkTreeIter *iter, GtkTreeIter *parent, gint n)
{
	GList *child;
	TnyGtkHeaderListModel *list_model = TNY_GTK_HEADER_LIST_MODEL (self);
	GList *restore;

	if (G_UNLIKELY (parent))
		return FALSE;

	g_mutex_lock (list_model->folder_lock);
	g_mutex_lock (list_model->iterator_lock);

	restore = ((TnyGtkHeaderListIterator*)list_model->iterator)->current;
	/* Move the GtkTreeIter to the nth child */
	_tny_gtk_header_list_iterator_nth_nl ((TnyGtkHeaderListIterator*)list_model->iterator, n);
	/* child needs not to be unref'd */
	child = _tny_gtk_header_list_iterator_get_current_nl ((TnyGtkHeaderListIterator*)list_model->iterator);
	 
	if (G_LIKELY (child))
	{
		list_model->last_nth = n;
		iter->stamp = TNY_GTK_HEADER_LIST_MODEL (self)->stamp;
		iter->user_data = child;

		g_mutex_unlock (list_model->iterator_lock);
		g_mutex_unlock (list_model->folder_lock);

		return TRUE;
	}

	((TnyGtkHeaderListIterator*)list_model->iterator)->current = restore;

	g_mutex_unlock (list_model->iterator_lock);
	g_mutex_unlock (list_model->folder_lock);

	return FALSE;
}


static void
tny_gtk_header_list_model_unref_node (GtkTreeModel *self, GtkTreeIter  *iter)
{
	return;
}
/*
	TnyHeader *header = NULL;
	TnyGtkHeaderListModel *list_model = TNY_GTK_HEADER_LIST_MODEL (self);

	g_return_if_fail (self);
	g_return_if_fail (iter->stamp == TNY_GTK_HEADER_LIST_MODEL (self)->stamp);

	* Unref node happens when the GtkTreeView no longer needs the 
	   reference to the GtkTreeIter (nor its user_data) *

	if (!iter->user_data);
		return;

	g_mutex_lock (list_model->folder_lock);
	g_mutex_lock (list_model->iterator_lock);

	header = iter->user_data;

	* We can use the knowledge that it no longer needs the reference,
	   to uncache the instance. Uncached instances are instances that
	   typically no longer have their real subject inmem. Next time they'll
	   get a property-request, they'll create a new real subject (which
	   takes a certain amount of time) and will cache that before replying
	   the request using the real subject. *

	if (G_LIKELY (header))
		tny_header_uncache (header);

	g_mutex_unlock (list_model->iterator_lock);
	g_mutex_unlock (list_model->folder_lock);

	return;
}
*/

static void
tny_gtk_header_list_model_ref_node (GtkTreeModel *self, GtkTreeIter  *iter)
{
	return;
}

static void
tny_gtk_header_list_model_tree_model_init (GtkTreeModelIface *iface)
{
	iface->get_flags = tny_gtk_header_list_model_get_flags;
	iface->get_n_columns = tny_gtk_header_list_model_get_n_columns;
	iface->get_column_type = tny_gtk_header_list_model_get_column_type;
	iface->get_iter = tny_gtk_header_list_model_get_iter;
	iface->get_path = tny_gtk_header_list_model_get_path;
	iface->get_value = tny_gtk_header_list_model_get_value;
	iface->iter_next = tny_gtk_header_list_model_iter_next;
	iface->iter_has_child = tny_gtk_header_list_model_iter_has_child;
	iface->iter_n_children = tny_gtk_header_list_model_iter_n_children;
	iface->iter_nth_child = tny_gtk_header_list_model_iter_nth_child;
	iface->ref_node = tny_gtk_header_list_model_ref_node;
	iface->unref_node = tny_gtk_header_list_model_unref_node;

	return;
}


static void
tny_gtk_header_list_model_prepend (TnyList *self, GObject* item)
{
	TnyGtkHeaderListModel *me = (TnyGtkHeaderListModel*)self;
	GtkTreePath *path;
	GtkTreeIter iter;

	path = gtk_tree_path_new ();
	gtk_tree_path_append_index (path, 0);
	iter.stamp = me->stamp;
	iter.user_data = item;

	/* Prepend something to the list */

	g_mutex_lock (me->iterator_lock);
	g_object_ref (G_OBJECT (item));
	me->first = g_list_prepend (me->first, item);
	me->length++;

	if G_UNLIKELY (me->length % INDEX_OFFSET == 0)
		me->index = g_list_append (me->index, me->first);

	if G_UNLIKELY (!me->usable_index && me->length >= INDEX_THRESHOLD)
		me->usable_index = TRUE;

	/* Reset the internal iterator */
	((TnyGtkHeaderListIterator*)me->iterator)->current = me->first;
	me->last_nth = 0;
	g_mutex_unlock (me->iterator_lock);

	/* Letting the observers know about this (the GtkTreeView) */
	gtk_tree_model_row_inserted (GTK_TREE_MODEL (me), path, &iter);
	gtk_tree_path_free (path);
}

static void
tny_gtk_header_list_model_append (TnyList *self, GObject* item)
{
	TnyGtkHeaderListModel *me = (TnyGtkHeaderListModel*)self;
	GtkTreePath *path;
	GtkTreeIter iter;

	path = gtk_tree_path_new ();
	iter.stamp = me->stamp;
	iter.user_data = item;

	/* Append something to the list */

	g_mutex_lock (me->iterator_lock);
	g_object_ref (G_OBJECT (item));
	me->first = g_list_append (me->first, item);
	me->length++;

	me->usable_index = FALSE;
	if (me->index)
		g_list_free (me->index);
	me->index = NULL;

	/* Reset the internal iterator */
	((TnyGtkHeaderListIterator*)me->iterator)->current = me->first;
	me->last_nth = 0;
	g_mutex_unlock (me->iterator_lock);

	/* Letting the observers know about this (the GtkTreeView) */
	gtk_tree_path_append_index (path, me->length-1);
	gtk_tree_model_row_inserted (GTK_TREE_MODEL (me), path, &iter);
	gtk_tree_path_free (path);

}

static guint
tny_gtk_header_list_model_get_length (TnyList *self)
{
	TnyGtkHeaderListModel *me = (TnyGtkHeaderListModel*)self;
	guint retval = 0;

	g_mutex_lock (me->iterator_lock);
	retval = me->first?g_list_length (me->first):0;
	g_mutex_unlock (me->iterator_lock);

	return retval;
}

static void
tny_gtk_header_list_model_remove (TnyList *self, GObject* item)
{
	TnyGtkHeaderListModel *me = (TnyGtkHeaderListModel*)self;
	GtkTreePath *path;
	GtkTreeIter iter;

	g_return_if_fail (G_IS_OBJECT (item));
	g_return_if_fail (G_IS_OBJECT (me));

	iter.stamp = me->stamp;
	iter.user_data = item;
	path = tny_gtk_header_list_model_get_path (GTK_TREE_MODEL (me), &iter);

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
	((TnyGtkHeaderListIterator*)me->iterator)->current = me->first;
	me->last_nth = 0;
	g_mutex_unlock (me->iterator_lock);

	/* Letting the observers know about this (the GtkTreeView) */
	gtk_tree_model_row_deleted (GTK_TREE_MODEL (me), path);
	gtk_tree_path_free (path);

}

static TnyIterator*
tny_gtk_header_list_model_create_iterator (TnyList *self)
{
	TnyGtkHeaderListModel *me = (TnyGtkHeaderListModel*)self;

	/* Return a new iterator */

	return _tny_gtk_header_list_iterator_new (me, TRUE);
}

static TnyList*
tny_gtk_header_list_model_copy_the_list (TnyList *self)
{
	TnyGtkHeaderListModel *me = (TnyGtkHeaderListModel*)self;
	TnyGtkHeaderListModel *copy = g_object_new (TNY_TYPE_GTK_HEADER_LIST_MODEL, NULL);

	/* This only copies the TnyList pieces. The result is not a
	   correct or good TnyGtkHeaderListModel. But it will be a correct
	   TnyList instance. It is the only thing the user of this
	   method expects.

	   The new list will point to the same instances, of course. It's
	   only a copy of the list-nodes of course. */

	g_mutex_lock (me->iterator_lock);
	GList *list_copy = g_list_copy (me->first);
	g_list_foreach (list_copy, (GFunc)g_object_ref, NULL);
	copy->first = list_copy;
	copy->usable_index = FALSE;
	g_mutex_unlock (me->iterator_lock);

	return TNY_LIST (copy);
}

static void 
tny_gtk_header_list_model_foreach_in_the_list (TnyList *self, GFunc func, gpointer user_data)
{
	TnyGtkHeaderListModel *me = (TnyGtkHeaderListModel*)self;

	/* Foreach item in the list (without using a slower iterator) */

	g_mutex_lock (me->iterator_lock);
	g_list_foreach (me->first, func, user_data);
	g_mutex_unlock (me->iterator_lock);

	return;
}

static void
tny_list_init (TnyListIface *klass)
{
	klass->get_length_func = tny_gtk_header_list_model_get_length;
	klass->prepend_func = tny_gtk_header_list_model_prepend;
	klass->append_func = tny_gtk_header_list_model_append;
	klass->remove_func = tny_gtk_header_list_model_remove;
	klass->create_iterator_func = tny_gtk_header_list_model_create_iterator;
	klass->copy_func = tny_gtk_header_list_model_copy_the_list;
	klass->foreach_func = tny_gtk_header_list_model_foreach_in_the_list;

	return;
}


/* The "relaxed performers" uses g_idle to in a relax way perform a certain
   function on each item in the list of headers  */

typedef struct
{
	GList *list;
	GFunc relaxed_func, final_func;
    	gpointer ffdata, ffudata;
} RelaxedData;


static void
tny_gtk_header_list_model_relaxed_data_destroyer (gpointer data)
{
	RelaxedData *d = data;

	/* The destroyer */

	g_list_free (d->list);
	d->list = NULL;
	d->final_func (d->ffdata, d->ffudata);
	g_free (d);

	return;
}

static gboolean
tny_gtk_header_list_model_relaxed_performer (gpointer data)
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
proxy_destroy_func (gpointer data, gpointer user_data)
{
	if (data)
		g_object_unref (G_OBJECT (data));
	data = NULL;

	return;
}



static void 
tny_gtk_header_list_model_hdr_cache_remover_copy (TnyGtkHeaderListModel *self, GFunc final_func, gpointer ffdata, gpointer ffudata)
{
	RelaxedData *d = g_new (RelaxedData, 1);

	/* This one will perform a destruction of each item in the list. It uses 
	   a copy of the list. */

	d->relaxed_func = (GFunc)proxy_destroy_func;
	d->list = g_list_copy (self->first);
	d->final_func = final_func;
    	d->ffdata = ffdata;
    	d->ffudata = ffudata;
    
	g_idle_add_full (G_PRIORITY_LOW, tny_gtk_header_list_model_relaxed_performer, 
		d, tny_gtk_header_list_model_relaxed_data_destroyer);

	return;
} 

static void
folder_overwrite_destruction (gpointer data, gpointer udata)
{
    	TnyGtkHeaderListModel *self = data;
    
    	/* Unreference the folder instance */
	if (self->folder) 
		g_object_unref (G_OBJECT (self->folder));
}


static void
final_destruction (gpointer data, gpointer udata)
{
    	TnyGtkHeaderListModel *self = data;
    
    	/* Unreference the folder instance */
	if (self->folder) 
	{
		g_object_unref (G_OBJECT (self->folder));
		if (self->iterator)
			g_object_unref (G_OBJECT (self->iterator));
	}
}

static void
tny_gtk_header_list_model_finalize (GObject *object)
{
	TnyGtkHeaderListModel *self = (TnyGtkHeaderListModel *)object;
    
	g_mutex_lock (self->folder_lock);
	g_mutex_lock (self->iterator_lock);

	self->usable_index = FALSE;

	/* Reset the internal iterator */
	self->length = 0;
	((TnyGtkHeaderListIterator*)self->iterator)->current = self->first;
	self->last_nth = 0;

	/* Unreference the headers */
	if (self->first)
	{

		if (G_LIKELY (g_main_depth () > 0))
			tny_gtk_header_list_model_hdr_cache_remover_copy (
				self, final_destruction, self, NULL);
		else {
			g_list_foreach (self->first, (GFunc)g_object_unref, NULL);
			final_destruction (self, NULL);
		}
		g_list_free (self->first);
		self->first = NULL;
	} else
		final_destruction (self, NULL);

	if (self->index)
	{
		g_list_free (self->index);
		self->index = NULL;
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
tny_gtk_header_list_model_class_init (TnyGtkHeaderListModelClass *klass)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *)klass;
	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = tny_gtk_header_list_model_finalize;

	return;
}

static void
tny_gtk_header_list_model_init (TnyGtkHeaderListModel *self)
{
	self->folder = NULL;
	self->folder_lock = g_mutex_new ();
	self->iterator_lock = g_mutex_new ();
	self->first = NULL;
	self->length = 0;
	self->usable_index = FALSE;

	/* This is an internal iterator used by this GtkTreeModel implementation */
	self->iterator = _tny_gtk_header_list_iterator_new (self, FALSE);

	return;
}


/**
 * tny_gtk_header_list_model_set_folder:
 * @self: A #TnyGtkHeaderListModel instance
 * @folder: a #TnyFolder instance
 * @refresh: refresh first
 *
 * Set the folder where the #TnyHeader instances are located
 * 
 **/
void
tny_gtk_header_list_model_set_folder (TnyGtkHeaderListModel *self, TnyFolder *folder, gboolean refresh)
{
	GtkTreeIter iter;
	GtkTreePath *path;

	iter.stamp = self->stamp;
	iter.user_data = self->first;
	path = tny_gtk_header_list_model_get_path (GTK_TREE_MODEL (self), &iter);

	g_mutex_lock (self->folder_lock);
	g_mutex_lock (self->iterator_lock);


	/* Unreference the previous headers */
	if (self->first)
	{
		if (G_LIKELY (g_main_depth () > 0))
			tny_gtk_header_list_model_hdr_cache_remover_copy (
				self, folder_overwrite_destruction, self, NULL);
		else {
			g_list_foreach (self->first, (GFunc)g_object_unref, NULL);
			folder_overwrite_destruction (self, NULL);
		}
	    
		g_list_free (self->first);
		self->first = NULL;
	}
	

	/* Reset the internal iterator */
	self->length = 0;
	((TnyGtkHeaderListIterator*)self->iterator)->current = self->first;
	self->last_nth = 0;

	g_mutex_unlock (self->iterator_lock);
	g_mutex_unlock (self->folder_lock);

	/* Get a new list of headers */
	tny_folder_get_headers (folder, TNY_LIST (self), refresh);

	g_mutex_lock (self->folder_lock);
	g_mutex_lock (self->iterator_lock);

	/* Reset the internal iterator */
   
    	/* Code review question (by Philip to myself, so don't ask 
	Philip), shouldn't this reset self->usable_index = FALSE ?*/

	((TnyGtkHeaderListIterator*)self->iterator)->current = self->first;
	self->last_nth = 0;

    	/* Note to myself: After more code review I decided that the answer 
	was yes ;-) */
    
    	self->usable_index = FALSE;
	if (self->index)
		g_list_free (self->index);
	self->index = NULL;
    
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
 * tny_gtk_header_list_model_new:
 *
 *
 * Return value: a new #GtkTreeModel instance suitable for showing lots of 
 * #TnyHeader instances
 **/
GtkTreeModel*
tny_gtk_header_list_model_new (void)
{
	TnyGtkHeaderListModel *model;

	model = g_object_new (TNY_TYPE_GTK_HEADER_LIST_MODEL, NULL);
	
	return GTK_TREE_MODEL (model);
}

GType
tny_gtk_header_list_model_get_type (void)
{
	static GType object_type = 0;

	if (G_UNLIKELY(object_type == 0))
	{
		static const GTypeInfo object_info = 
		{
			sizeof (TnyGtkHeaderListModelClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) tny_gtk_header_list_model_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (TnyGtkHeaderListModel),
			0,              /* n_preallocs */
			(GInstanceInitFunc) tny_gtk_header_list_model_init,
			NULL
		};

		static const GInterfaceInfo tree_model_info = {
			(GInterfaceInitFunc) tny_gtk_header_list_model_tree_model_init,
			NULL,
			NULL
		};
		

		static const GInterfaceInfo tny_list_info = {
			(GInterfaceInitFunc) tny_list_init,
			NULL,
			NULL
		};

		object_type = g_type_register_static (G_TYPE_OBJECT, 
						"TnyGtkHeaderListModel", &object_info, 0);

		g_type_add_interface_static (object_type, GTK_TYPE_TREE_MODEL,
					     &tree_model_info);

		g_type_add_interface_static (object_type, TNY_TYPE_LIST,
					     &tny_list_info);

	}

	return object_type;
}

GType
tny_gtk_header_list_model_column_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {

      { TNY_GTK_HEADER_LIST_MODEL_FROM_COLUMN, "TNY_GTK_HEADER_LIST_MODEL_FROM_COLUMN", "from" },
      { TNY_GTK_HEADER_LIST_MODEL_TO_COLUMN, "TNY_GTK_HEADER_LIST_MODEL_TO_COLUMN", "to" },
      { TNY_GTK_HEADER_LIST_MODEL_SUBJECT_COLUMN, "TNY_GTK_HEADER_LIST_MODEL_SUBJECT_COLUMN", "subject" },
      { TNY_GTK_HEADER_LIST_MODEL_CC_COLUMN, "TNY_GTK_HEADER_LIST_MODEL_CC_COLUMN", "cc" },
      { TNY_GTK_HEADER_LIST_MODEL_DATE_SENT_COLUMN, "TNY_GTK_HEADER_LIST_MODEL_DATE_SENT_COLUMN", "date_sent" },
      { TNY_GTK_HEADER_LIST_MODEL_DATE_RECEIVED_TIME_T_COLUMN, "TNY_GTK_HEADER_LIST_MODEL_DATE_RECEIVED_TIME_T_COLUMN", "date_received_t" },
      { TNY_GTK_HEADER_LIST_MODEL_DATE_SENT_TIME_T_COLUMN, "TNY_GTK_HEADER_LIST_MODEL_DATE_SENT_TIME_T_COLUMN", "date_sent_t" },
      { TNY_GTK_HEADER_LIST_MODEL_DATE_RECEIVED_COLUMN, "TNY_GTK_HEADER_LIST_MODEL_DATE_RECEIVED_COLUMN", "date_received" },
      { TNY_GTK_HEADER_LIST_MODEL_INSTANCE_COLUMN, "TNY_GTK_HEADER_LIST_MODEL_INSTANCE_COLUMN", "instance" },
      { TNY_GTK_HEADER_LIST_MODEL_FLAGS_COLUMN, "TNY_GTK_HEADER_LIST_MODEL_FLAGS_COLUMN", "flags" },
      { TNY_GTK_HEADER_LIST_MODEL_N_COLUMNS, "TNY_GTK_HEADER_LIST_MODEL_N_COLUMNS", "n" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyGtkHeaderListModelColumn", values);
  }
  return etype;
}


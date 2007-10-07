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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <config.h>

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <gdk/gdk.h>

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


static gint 
add_del_timeout (TnyGtkHeaderListModel *me, guint num)
{
	TnyGtkHeaderListModelPriv *priv = TNY_GTK_HEADER_LIST_MODEL_GET_PRIVATE (me);

	gint retval = 0;
	g_mutex_lock (priv->to_lock);
	if (!priv->del_timeouts)
		priv->del_timeouts = g_array_new (FALSE, FALSE, sizeof (guint));
	g_array_append_val (priv->del_timeouts, num);
	retval = priv->del_timeouts->len-1;
	g_mutex_unlock (priv->to_lock);

	return retval;
}

static void
remove_del_timeouts (TnyGtkHeaderListModel *me)
{
	TnyGtkHeaderListModelPriv *priv = TNY_GTK_HEADER_LIST_MODEL_GET_PRIVATE (me);

	gint i;
	guint src;

	g_mutex_lock (priv->to_lock);

	if (!priv->del_timeouts)
	{
		g_mutex_unlock (priv->to_lock);
		return;
	}

	for (i = 0; i < priv->del_timeouts->len; i++)
	{
		src = g_array_index (priv->del_timeouts, guint, i);
		if (src > 0)
			g_source_remove (src);
		g_array_index (priv->del_timeouts, guint, i) = 0;
	}
	g_array_free (priv->del_timeouts, TRUE);
	priv->del_timeouts = NULL;
	g_mutex_unlock (priv->to_lock);
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
		case TNY_GTK_HEADER_LIST_MODEL_MESSAGE_SIZE_COLUMN:
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
	TnyGtkHeaderListModelPriv *priv = TNY_GTK_HEADER_LIST_MODEL_GET_PRIVATE (self);

	gint i, tlen; 
	gboolean retval=FALSE;

	g_return_val_if_fail (gtk_tree_path_get_depth (path) > 0, FALSE);

	g_static_rec_mutex_lock (priv->iterator_lock); 

	i = gtk_tree_path_get_indices (path)[0];

	g_mutex_lock (priv->ra_lock);
	tlen = priv->cur_len;
	g_mutex_unlock (priv->ra_lock);

	if (i >= tlen)
		retval = FALSE;
	else {
		if (i >= 0 && i < tlen) {
			iter->stamp = priv->stamp;
			iter->user_data = (gpointer) i;
			retval = TRUE;
		}
	}

	g_static_rec_mutex_unlock (priv->iterator_lock);

	return retval;
}

static GtkTreePath *
tny_gtk_header_list_model_get_path (GtkTreeModel *self, GtkTreeIter *iter)
{
	TnyGtkHeaderListModelPriv *priv = TNY_GTK_HEADER_LIST_MODEL_GET_PRIVATE (self);
	GtkTreePath *tree_path;
	gint i = 0, tlen;

	/* Return the path of an existing GtkTreeIter */
	if  (!(iter->stamp == priv->stamp))
		return NULL;

	g_static_rec_mutex_lock (priv->iterator_lock);

	i = (gint) iter->user_data;

	g_mutex_lock (priv->ra_lock);
	tlen = priv->cur_len;
	g_mutex_unlock (priv->ra_lock);

	if (i < 0 || i >= tlen) {
		g_static_rec_mutex_unlock (priv->iterator_lock);
		return NULL;
	}

	tree_path = gtk_tree_path_new ();
	gtk_tree_path_append_index (tree_path, i);

	g_static_rec_mutex_unlock (priv->iterator_lock);

	return tree_path;
}


static gchar *
_get_readable_date (time_t file_time_raw)
{
	struct tm file_time;
	static gchar readable_date[64];
	gsize readable_date_size;

	gmtime_r (&file_time_raw, &file_time);

	readable_date_size = strftime (readable_date, 63, _("%Y-%m-%d, %-I:%M %p"), &file_time);

	if (readable_date_size > 0)
		return readable_date;

	return NULL;
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
	TnyGtkHeaderListModelPriv *priv = TNY_GTK_HEADER_LIST_MODEL_GET_PRIVATE (model);

	TnyHeader *hdr_a, *hdr_b;
	time_t recv_a, recv_b;

	/* Get the index out of the iter, and use the item by looking it up in
	 * the GPtrArray. This must be quite efficient, as this is called lots
	 * of times while you are sorting things. */

	hdr_a = priv->items->pdata[(gint)a->user_data];
	hdr_b = priv->items->pdata[(gint)b->user_data];

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
	TnyGtkHeaderListModelPriv *priv = TNY_GTK_HEADER_LIST_MODEL_GET_PRIVATE (model);

	TnyHeader *hdr_a, *hdr_b;
	time_t recv_a, recv_b;

	/* Get the index out of the iter, and use the item by looking it up in
	 * the GPtrArray. This must be quite efficient, as this is called lots
	 * of times while you are sorting things. */

	hdr_a = priv->items->pdata[(gint)a->user_data];
	hdr_b = priv->items->pdata[(gint)b->user_data];

	recv_a = tny_header_get_date_sent (hdr_a);
	recv_b = tny_header_get_date_sent (hdr_b);

	return (recv_a - recv_b);
}

static void
tny_gtk_header_list_model_get_value (GtkTreeModel *self, GtkTreeIter *iter, gint column, GValue *value)
{
	TnyGtkHeaderListModelPriv *priv = TNY_GTK_HEADER_LIST_MODEL_GET_PRIVATE (self);

	const gchar *str;
	gchar *rdate = NULL;
	gint i;

	if (iter->stamp != priv->stamp)
		return;

	g_static_rec_mutex_lock (priv->iterator_lock);

	/* Get the index out of the iter, and use the item by looking it up in
	 * the GPtrArray. This must be quite efficient, as this is called lots
	 * of times while you are sorting things. */

	i = (gint) iter->user_data;

	/* Exception on list_model->cur_len (this one truly allows the full 
	 * length of the available data in the array, not only the registered
	 * length, although in 99.999999% of the cases it's the same) */
	if (i < 0 || i >= priv->items->len)
	{
		g_warning ("GtkTreeModel in invalid state\n");
		g_static_rec_mutex_unlock (priv->iterator_lock);
		return;
	}

	if (priv->items->pdata[i] == NULL)
		return;

	switch (column) 
	{
		case TNY_GTK_HEADER_LIST_MODEL_CC_COLUMN:
			g_value_init (value, G_TYPE_STRING);
			str = tny_header_get_cc ((TnyHeader*) priv->items->pdata[i]);
			if (str)
				g_value_set_string (value, str);
			break;
		case TNY_GTK_HEADER_LIST_MODEL_DATE_SENT_COLUMN:
			g_value_init (value, G_TYPE_STRING);
			rdate = _get_readable_date (tny_header_get_date_sent ((TnyHeader*) priv->items->pdata[i]));
			if (rdate)
				g_value_set_string (value, rdate);
			else
				g_value_set_string (value, "");
			break;
		case TNY_GTK_HEADER_LIST_MODEL_DATE_RECEIVED_COLUMN:
			g_value_init (value, G_TYPE_STRING);
			rdate = _get_readable_date (tny_header_get_date_received ((TnyHeader*) priv->items->pdata[i]));
			if (rdate)
				g_value_set_string (value, rdate);
			else
				g_value_set_string (value, "");
			break;
		case TNY_GTK_HEADER_LIST_MODEL_DATE_SENT_TIME_T_COLUMN:
			g_value_init (value, G_TYPE_INT);
			g_value_set_int (value, 
					    tny_header_get_date_sent ((TnyHeader*) priv->items->pdata[i]));
			break;
		case TNY_GTK_HEADER_LIST_MODEL_DATE_RECEIVED_TIME_T_COLUMN:
			g_value_init (value, G_TYPE_INT);
			g_value_set_int (value, 
					 tny_header_get_date_received ((TnyHeader*) priv->items->pdata[i]));
			break;

		case TNY_GTK_HEADER_LIST_MODEL_MESSAGE_SIZE_COLUMN:
			g_value_init (value, G_TYPE_INT);
			g_value_set_int (value, tny_header_get_message_size((TnyHeader*) priv->items->pdata[i]));
			break;			
		case TNY_GTK_HEADER_LIST_MODEL_INSTANCE_COLUMN:
			g_value_init (value, G_TYPE_OBJECT);
			g_value_set_object (value, (TnyHeader*) priv->items->pdata[i]);
			break;
		case TNY_GTK_HEADER_LIST_MODEL_TO_COLUMN:
			g_value_init (value, G_TYPE_STRING);
			str = tny_header_get_to ((TnyHeader*) priv->items->pdata[i]);
			if (str)
				g_value_set_string (value, str);
			break;
		case TNY_GTK_HEADER_LIST_MODEL_SUBJECT_COLUMN:
			g_value_init (value, G_TYPE_STRING);
			str = tny_header_get_subject ((TnyHeader*) priv->items->pdata[i]);
			if (str)
				g_value_set_string (value, str);
			break;
		case TNY_GTK_HEADER_LIST_MODEL_FROM_COLUMN:
			g_value_init (value, G_TYPE_STRING);
			str = tny_header_get_from ((TnyHeader*) priv->items->pdata[i]);
			if (str)
				g_value_set_string (value, str);
			break;
		case TNY_GTK_HEADER_LIST_MODEL_FLAGS_COLUMN:
			g_value_init (value, G_TYPE_INT);
			g_value_set_int (value, tny_header_get_flags ((TnyHeader*) priv->items->pdata[i]));
			break;
		default:
			break;
	}

	g_static_rec_mutex_unlock (priv->iterator_lock);

	return;
}

static gboolean
tny_gtk_header_list_model_iter_next (GtkTreeModel *self, GtkTreeIter *iter)
{
	TnyGtkHeaderListModelPriv *priv = TNY_GTK_HEADER_LIST_MODEL_GET_PRIVATE (self);
	gboolean retval;
	gint newv;

	if (iter->stamp != priv->stamp)
		return FALSE;

	g_static_rec_mutex_lock (priv->iterator_lock);

	/* The next will simply be the current plus one: That's because we are
	 * storing the GPtrArray's index in the iters. Simple, efficient. Just
	 * always make sure that such an iter doesn't contain an index that
	 * can't be part of the GPtrArray instance (would be devastating). */

	newv = ((gint) iter->user_data);
	newv++; 
	iter->user_data = (gpointer) newv;

	g_mutex_lock (priv->ra_lock);
	retval = (newv >= 0 && newv < priv->cur_len);
	g_mutex_unlock (priv->ra_lock);

	if (!retval) {
		iter->stamp = -1;
		iter->user_data = (gpointer) 0;
	}

	g_static_rec_mutex_unlock (priv->iterator_lock);

	return retval;
}

static gboolean
tny_gtk_header_list_model_iter_has_child (GtkTreeModel *self, GtkTreeIter *iter)
{
	/* This is a flat list (we don't yet support threaded views anyway), so
	 * the answer is flat-out no. Always. */

	return FALSE;
}

static gint
tny_gtk_header_list_model_iter_n_children (GtkTreeModel *self, GtkTreeIter *iter)
{
	TnyGtkHeaderListModelPriv *priv = TNY_GTK_HEADER_LIST_MODEL_GET_PRIVATE (self);
	gint retval = -1;

	/* Return the amount of children for this GtkTreeIter. Because this
	   is a flat list and has_child is always FALSE, we'll just always
	   return the full length. */

	g_static_rec_mutex_lock (priv->iterator_lock);

	if (G_LIKELY (!iter))
		retval = priv->cur_len;

	g_static_rec_mutex_unlock (priv->iterator_lock);

	return retval;
}

static gboolean
tny_gtk_header_list_model_iter_nth_child (GtkTreeModel *self, GtkTreeIter *iter, GtkTreeIter *parent, gint n)
{
	TnyGtkHeaderListModelPriv *priv = TNY_GTK_HEADER_LIST_MODEL_GET_PRIVATE (self);
	gboolean retval = FALSE;
	gint tlen;

	if (G_UNLIKELY (parent))
		return FALSE;

	g_static_rec_mutex_lock (priv->iterator_lock);

	g_mutex_lock (priv->ra_lock);
	tlen = priv->cur_len;
	g_mutex_unlock (priv->ra_lock);

	if (n >= 0 && n < tlen) 
	{
		iter->stamp = priv->stamp;
		iter->user_data = (gpointer) n;
		retval = TRUE;
	} else {
		iter->stamp = -1;
		iter->user_data = (gpointer) 0;
	}

	g_static_rec_mutex_unlock (priv->iterator_lock);

	return retval;
}

static gboolean                                                                                                                                                             
tny_gtk_header_list_model_iter_children (GtkTreeModel *tree_model,
					 GtkTreeIter  *iter,
					 GtkTreeIter  *parent) 
{                                                                                                                                                                           
	gboolean retval = FALSE;

	retval = tny_gtk_header_list_model_iter_nth_child (tree_model, iter, parent, 0);
	
	return retval;
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
	iface->iter_children = tny_gtk_header_list_model_iter_children;
	iface->iter_nth_child = tny_gtk_header_list_model_iter_nth_child;

	return;
}


static void
notify_views_add_destroy (gpointer data)
{
	TnyGtkHeaderListModelPriv *priv = TNY_GTK_HEADER_LIST_MODEL_GET_PRIVATE (data);

	g_mutex_lock (priv->ra_lock);
	priv->updating_views = -1;
	g_mutex_unlock (priv->ra_lock);

	if (priv->timeout_span < 5000)
		priv->timeout_span += 300;
	priv->add_timeout = 0;
	g_object_unref (data);

	return;
}

static gboolean
notify_views_add (gpointer data)
{
	TnyGtkHeaderListModelPriv *priv = TNY_GTK_HEADER_LIST_MODEL_GET_PRIVATE (data);
	gint already_registered, going_tb_registered, i;
 
	gboolean needmore = FALSE;
	GtkTreePath *path;
	GtkTreeIter iter;

	g_mutex_lock (priv->ra_lock);
	priv->updating_views++;
	if (priv->registered >= priv->items->len) {
		g_mutex_unlock (priv->ra_lock);
		return FALSE;
	}

	already_registered = priv->registered;

	if (priv->items->len - already_registered > 1000) 
	{
		going_tb_registered = already_registered + 1000;
		needmore = TRUE;
	} else
		going_tb_registered = priv->items->len;

	priv->registered = going_tb_registered;

	if (priv->updating_views < 2)
		needmore = TRUE;

	g_mutex_unlock (priv->ra_lock);

	gdk_threads_enter();

	for (i = already_registered; i < going_tb_registered; i++)
	{
		iter.stamp = priv->stamp;
		iter.user_data = (gpointer) i;
		path = gtk_tree_path_new ();
		gtk_tree_path_append_index (path, i);
		if (G_LIKELY (path))
		{
			priv->cur_len = i+1;
			gtk_tree_model_row_inserted ((GtkTreeModel *) data, path, &iter);
			gtk_tree_path_free (path);
		}
	}
	gdk_threads_leave();

	return needmore;
}



/* This will be called often while you are in tny_folder_refresh(_async). It can
 * and will be called from a thread, so we must cope with that in case we want 
 * to update the GtkTreeViews that have been attached to this model (self). */
static void
tny_gtk_header_list_model_prepend (TnyList *self, GObject* item)
{
	TnyGtkHeaderListModelPriv *priv = TNY_GTK_HEADER_LIST_MODEL_GET_PRIVATE (self);

	g_static_rec_mutex_lock (priv->iterator_lock);

	/* Prepend something to the list itself. The get_length will auto update
	 * because that one uses GPtrArray's len property. We are reusing the 
	 * iterator_lock for locking this out, by the way. */

	g_object_ref (item);

	g_mutex_lock (priv->ra_lock);

	g_ptr_array_add (priv->items, item);

	/* This prepend will happen very often, the notificating of the view is, 
	 * however, quite slow and gdk wants us to do this from the mainloop */

	if (priv->updating_views == -1)
	{
		priv->updating_views = 0;
		g_object_ref (self);

		if (priv->add_timeout > 0) {
			g_source_remove (priv->add_timeout);
			priv->add_timeout = 0;
		}

		priv->add_timeout = g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE, 
			priv->timeout_span, notify_views_add, self, 
			notify_views_add_destroy);
	}

	g_mutex_unlock (priv->ra_lock);

	g_static_rec_mutex_unlock (priv->iterator_lock);

	return;
}

static void
tny_gtk_header_list_model_append (TnyList *self, GObject* item)
{
	/* Not really correct, but what the heck */
	tny_gtk_header_list_model_prepend (self, item);
}


typedef struct
{
	TnyGtkHeaderListModel *self;
	GObject *item;
	/* GMainLoop *loop; */
	gint src;
} notify_views_data_t;


static void 
notify_views_delete_destroy (gpointer data)
{
	notify_views_data_t *stuff = data;
	TnyGtkHeaderListModelPriv *priv = TNY_GTK_HEADER_LIST_MODEL_GET_PRIVATE (stuff->self);

	g_mutex_lock (priv->to_lock);
	if (stuff->src != -1 && stuff->src < priv->del_timeouts->len)
		g_array_index (priv->del_timeouts, guint, (guint) stuff->src) = 0;
	g_mutex_unlock (priv->to_lock);

	g_object_unref (stuff->item);
	g_object_unref (stuff->self);
	/* g_main_loop_unref (stuff->loop); */

	g_slice_free (notify_views_data_t, data);
	return;
}


static gboolean
notify_views_delete (gpointer data)
{
	notify_views_data_t *stuff = data;
	TnyGtkHeaderListModelPriv *priv = TNY_GTK_HEADER_LIST_MODEL_GET_PRIVATE (stuff->self);

	GtkTreePath *path;
	GtkTreeIter iter;
	gint i; gboolean found = FALSE;
	GObject *mitem, *item = stuff->item;

	g_static_rec_mutex_lock (priv->iterator_lock);

	for (i=0; i < priv->items->len; i++)
		if (priv->items->pdata[i] == item)
		{
			found = TRUE;
			break;
		}

	if (found)
	{
		iter.stamp = priv->stamp;
		iter.user_data = (gpointer) i;
		path = gtk_tree_path_new ();
		gtk_tree_path_append_index (path, i);
		gtk_tree_model_row_deleted ((GtkTreeModel *) stuff->self, path);
		priv->stamp++;
		g_mutex_lock (priv->ra_lock);
		priv->cur_len--;
		priv->registered--;
		g_mutex_unlock (priv->ra_lock);
		gtk_tree_path_free (path);

		mitem = g_ptr_array_remove_index (priv->items, i);
		if (mitem)
			g_object_unref (mitem);
	}

	g_static_rec_mutex_unlock (priv->iterator_lock);

	/* if (g_main_loop_is_running (stuff->loop))
		g_main_loop_quit (stuff->loop); */

	return FALSE;
}


static void
tny_gtk_header_list_model_remove (TnyList *self, GObject* item)
{
	notify_views_data_t *stuff;
	guint src;

	stuff = g_slice_new (notify_views_data_t);
	stuff->src = -1;
	stuff->self = g_object_ref (self);
	stuff->item = g_object_ref (item);

	/* stuff->loop = g_main_loop_new (NULL, FALSE); */

	src = g_timeout_add_full (G_PRIORITY_HIGH_IDLE, 0,
		notify_views_delete, stuff, notify_views_delete_destroy);
	stuff->src = (gint) add_del_timeout ((TnyGtkHeaderListModel *) self, src);

	/* This truly sucks :-( 
	g_main_loop_run (stuff->loop); */

	return;
}

static guint
tny_gtk_header_list_model_get_length (TnyList *self)
{
	TnyGtkHeaderListModelPriv *priv = TNY_GTK_HEADER_LIST_MODEL_GET_PRIVATE (self);
	guint retval = 0;

	g_static_rec_mutex_lock (priv->iterator_lock);
	retval = priv->items->len;
	g_static_rec_mutex_unlock (priv->iterator_lock);

	return retval;
}

static TnyIterator*
tny_gtk_header_list_model_create_iterator (TnyList *self)
{
	return _tny_gtk_header_list_iterator_new ((TnyGtkHeaderListModel *) self);
}

static void
copy_it (gpointer data, gpointer user_data)
{
	GPtrArray *items = user_data;
	g_ptr_array_add (items, g_object_ref (data));
	return;
}

static TnyList*
tny_gtk_header_list_model_copy_the_list (TnyList *self)
{
	TnyGtkHeaderListModelPriv *priv = TNY_GTK_HEADER_LIST_MODEL_GET_PRIVATE (self);
	TnyGtkHeaderListModel *copy = g_object_new (TNY_TYPE_GTK_HEADER_LIST_MODEL, NULL);
	TnyGtkHeaderListModelPriv *cpriv = TNY_GTK_HEADER_LIST_MODEL_GET_PRIVATE (copy);
	GPtrArray *items_copy = NULL;

	/* This only copies the TnyList pieces. The result is not a
	   correct or good TnyGtkHeaderListModel. But it will be a correct
	   TnyList instance. It is the only thing the user of this
	   method expects.

	   The new list will point to the same instances, of course. It's
	   only a copy of the list-nodes of course. */

	g_static_rec_mutex_lock (priv->iterator_lock);
	items_copy = g_ptr_array_sized_new (priv->items->len);
	g_ptr_array_foreach (priv->items, copy_it, items_copy);
	cpriv->items = items_copy;
	g_static_rec_mutex_unlock (priv->iterator_lock);

	return TNY_LIST (copy);
}

static void 
tny_gtk_header_list_model_foreach_in_the_list (TnyList *self, GFunc func, gpointer user_data)
{
	TnyGtkHeaderListModelPriv *priv = TNY_GTK_HEADER_LIST_MODEL_GET_PRIVATE (self);

	g_static_rec_mutex_lock (priv->iterator_lock);
	g_ptr_array_foreach (priv->items, func, user_data);
	g_static_rec_mutex_unlock (priv->iterator_lock);

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

#ifdef DEBUG_EXTRA
static void forea (gpointer u, gpointer o)
{
	g_print ("TnyGtkHeaderListModel::finalize unrefs hdr to: %d\n", ((GObject *)u)->ref_count);
	if (((GObject *)u)->ref_count != 1)
		printf ("ps. Usually, this should be 1 here\n");
	g_object_unref (u);
}
#endif


static void
tny_gtk_header_list_model_finalize (GObject *object)
{
	TnyGtkHeaderListModel *self = (TnyGtkHeaderListModel *) object;
	TnyGtkHeaderListModelPriv *priv = TNY_GTK_HEADER_LIST_MODEL_GET_PRIVATE (self);

	g_static_rec_mutex_lock (priv->iterator_lock);

#ifdef DEBUG
	g_print ("tny_gtk_header_list_model_finalize\n");
#endif

	if (priv->add_timeout > 0) {
		g_source_remove (priv->add_timeout);
		priv->add_timeout = 0;
	}

	remove_del_timeouts (self);

#ifdef DEBUG_EXTRA
	g_ptr_array_foreach (priv->items, (GFunc) forea, NULL);
#else
	g_ptr_array_foreach (priv->items, (GFunc) g_object_unref, NULL);
#endif

	if (priv->folder)
		g_object_unref (priv->folder);
	g_ptr_array_free (priv->items, TRUE);
	priv->items = NULL;

	g_static_rec_mutex_unlock (priv->iterator_lock);

	g_static_rec_mutex_free (priv->iterator_lock);
	priv->iterator_lock = NULL;

	g_mutex_free (priv->ra_lock);
	priv->ra_lock = NULL;

	g_mutex_free (priv->to_lock);
	priv->to_lock = NULL;

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

	g_type_class_add_private (object_class, sizeof (TnyGtkHeaderListModelPriv));


	return;
}

static void
tny_gtk_header_list_model_init (TnyGtkHeaderListModel *self)
{
	TnyGtkHeaderListModelPriv *priv = (G_TYPE_INSTANCE_GET_PRIVATE ((self),
					   TNY_TYPE_GTK_HEADER_LIST_MODEL,
					   TnyGtkHeaderListModelPriv));
	self->priv = priv;

	priv->folder = NULL;
	priv->iterator_lock = g_new0 (GStaticRecMutex, 1);
	g_static_rec_mutex_init (priv->iterator_lock);
	priv->cur_len = 0;

	priv->timeout_span = 100;
	priv->del_timeouts = NULL;
	priv->add_timeout = 0;
	priv->items = g_ptr_array_sized_new (1000);
	priv->updating_views = -1;
	priv->ra_lock = g_mutex_new ();
	priv->to_lock = g_mutex_new ();
	priv->registered = 0;

	return;
}


/**
 * tny_gtk_header_list_model_set_folder:
 * @self: A #TnyGtkHeaderListModel instance
 * @folder: a #TnyFolder instance
 * @refresh: refresh first
 * @callback: a #TnyGetHeadersCallback
 * @status_callback: a #TnyStatusCallback
 * @user_data: user data for the callbacks
 *
 * Set the folder where the #TnyHeader instances are located
 * 
 **/
void
tny_gtk_header_list_model_set_folder (TnyGtkHeaderListModel *self, TnyFolder *folder, gboolean refresh, TnyGetHeadersCallback callback, TnyStatusCallback status_callback, gpointer user_data)
{
	TnyGtkHeaderListModelPriv *priv = TNY_GTK_HEADER_LIST_MODEL_GET_PRIVATE (self);
	GtkTreeIter iter;
	GtkTreePath *path;

	g_static_rec_mutex_lock (priv->iterator_lock);

	priv->timeout_span = 100;
	if (priv->add_timeout > 0) {
		g_source_remove (priv->add_timeout);
		priv->add_timeout = 0;
	}

	remove_del_timeouts (self);

	/* Set it to 1 as initial value, else you cause the length > 0 
	 * assertion in gtk_tree_model_sort_build_level (I have no idea why the
	 * assertion is placed there, in stead of a normal return, though) */
 
	g_ptr_array_foreach (priv->items, (GFunc) g_object_unref, NULL);
	if (priv->folder)
		g_object_unref (priv->folder);
	priv->folder = TNY_FOLDER (g_object_ref (folder));
	priv->registered = 0;
	g_ptr_array_free (priv->items, TRUE);
	priv->items = g_ptr_array_sized_new (tny_folder_get_all_count (folder));

	g_static_rec_mutex_unlock (priv->iterator_lock);

	/* Get a new list of headers */
	/* TODO add error handling and reporting here */
	tny_folder_get_headers_async (folder, TNY_LIST (self), refresh, callback, status_callback, user_data);

	iter.stamp = priv->stamp;
	iter.user_data = (gpointer) 0;
	path = tny_gtk_header_list_model_get_path ((GtkTreeModel *) self, &iter);

	g_static_rec_mutex_lock (priv->iterator_lock);

	/* Reference the new folder instance */

	/* Notify the observers (the GtkTreeView) */
	if (path) {
		gtk_tree_path_append_index (path, 0);
		gtk_tree_model_row_inserted ((GtkTreeModel *) self, path, &iter);
		gtk_tree_path_free (path);
	}

	g_static_rec_mutex_unlock (priv->iterator_lock);

	return;
}

/**
 * tny_gtk_header_list_model_new:
 *
 * Create a new #GtkTreeModel instance suitable for showing lots of 
 * #TnyHeader instances
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

/**
 * tny_gtk_header_list_model_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
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

/**
 * tny_gtk_header_list_model_column_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
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
      { TNY_GTK_HEADER_LIST_MODEL_MESSAGE_SIZE_COLUMN, "TNY_GTK_HEADER_LIST_MODEL_MESSAGE_SIZE_COLUMN", "message_size"},
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


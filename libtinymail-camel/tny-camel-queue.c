/* libtinymail-camel - The Tiny Mail base library for Camel
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

#include <glib/gi18n-lib.h>
#include <glib.h>
#include <string.h>
#include <time.h>
#include <tny-list.h>
#include <tny-iterator.h>
#include <tny-camel-queue-priv.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <tny-camel-shared.h>
#include <tny-status.h>

static GObjectClass *parent_class = NULL;

#define TNY_CAMEL_QUEUE_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_CAMEL_QUEUE, TnyCamelQueuePriv))


static void
tny_camel_queue_finalize (GObject *object)
{
	TnyCamelQueue *self = (TnyCamelQueue*) object;

	self->stopped = TRUE;

	g_static_rec_mutex_lock (self->lock);
	g_list_free (self->list);
	self->list = NULL;
	g_static_rec_mutex_unlock (self->lock);

	if (self->thread)
		g_thread_join (self->thread);

	g_static_rec_mutex_free (self->lock);
	self->lock = NULL;

	(*parent_class->finalize) (object);

	return;
}

TnyCamelQueue*
_tny_camel_queue_new (TnyCamelStoreAccount *account)
{
	TnyCamelQueue *self = g_object_new (TNY_TYPE_CAMEL_QUEUE, NULL);

	self->account = account;

	return TNY_CAMEL_QUEUE (self);
}

typedef struct
{
	GThreadFunc func;
	gpointer data;
	TnyCamelQueueItemFlags flags;
	const gchar *name;
} QueueItem;


static gpointer 
thread_main_func (gpointer user_data)
{
	TnyCamelQueue *queue = user_data;

	while (!queue->stopped)
	{
		GList *first = NULL;
		QueueItem *item = NULL;

		g_static_rec_mutex_lock (queue->lock);
		first = g_list_first (queue->list);
		if (first) {
			item = first->data;
			queue->current = item;
		} else
			queue->stopped = TRUE;
		g_static_rec_mutex_unlock (queue->lock);

		if (item) 
			item->func (item->data);

		g_static_rec_mutex_lock (queue->lock);
		if (first)
			queue->list = g_list_delete_link (queue->list, first);
		queue->current = NULL;

		if (g_list_length (queue->list) == 0) {
			queue->thread = NULL;
			queue->stopped = TRUE;
		}
		g_static_rec_mutex_unlock (queue->lock);

		/*if (first)
			g_list_free (first);*/
		if (item)
			g_slice_free (QueueItem, item);
	}

	queue->thread = NULL;
	queue->stopped = TRUE;
	g_object_unref (queue);

	g_thread_exit (NULL);
	return NULL;
}


void 
_tny_camel_queue_remove_items (TnyCamelQueue *queue, TnyCamelQueueItemFlags flags)
{
	GList *copy = NULL, *rem = NULL;

	g_static_rec_mutex_lock (queue->lock);
	copy = queue->list;
	while (copy) {
		QueueItem *item = copy->data;
		if (queue->current != item)
		{
			if (item && (item->flags & flags)) {
				rem = g_list_prepend (rem, copy);
				g_slice_free (QueueItem, item);
			}
		}
		copy = g_list_next (copy);
	}
	while (rem) {
		queue->list = g_list_delete_link (queue->list, rem->data);
		rem = g_list_next (rem);
	}
	g_static_rec_mutex_unlock (queue->lock);
}

void 
_tny_camel_queue_cancel_remove_items (TnyCamelQueue *queue, TnyCamelQueueItemFlags flags)
{
	QueueItem *item = NULL;

	g_static_rec_mutex_lock (queue->lock);
	item = queue->current;

	/* Remove all the cancellables */
	_tny_camel_queue_remove_items (queue, flags);

	/* Cancel the current */
	if (item) {
		if (item->flags & TNY_CAMEL_QUEUE_CANCELLABLE_ITEM)
			_tny_camel_account_actual_cancel (TNY_CAMEL_ACCOUNT (queue->account));
	}

	g_static_rec_mutex_unlock (queue->lock);

}

void 
_tny_camel_queue_launch_wflags (TnyCamelQueue *queue, GThreadFunc func, gpointer data, TnyCamelQueueItemFlags flags, const gchar *name)
{
	QueueItem *item = g_slice_new (QueueItem);

	if (!g_thread_supported ())
		g_thread_init (NULL);

	item->func = func;
	item->data = data;
	item->flags = flags;
	item->name = name;

	g_static_rec_mutex_lock (queue->lock);

	if (flags & TNY_CAMEL_QUEUE_PRIORITY_ITEM) 
	{
		/* Preserve the order for prioritized items */
		gboolean stop = FALSE; gint cnt = 0;
		GList *first = g_list_first (queue->list);
		while (first && !stop) {
			QueueItem *item = first->data;
			if (item)
				if (!(item->flags & TNY_CAMEL_QUEUE_PRIORITY_ITEM)) {
					stop = TRUE;
					cnt--;
				}
			cnt++;
			first = g_list_next (first);
		}
		queue->list = g_list_insert (queue->list, item, cnt);
	} else /* Normal items simply get appended */
		queue->list = g_list_append (queue->list, item);

	if (queue->stopped) 
	{
		GError *err = NULL;
		queue->stopped = FALSE;
		g_object_ref (queue);
		queue->thread = g_thread_create (thread_main_func, 
			queue, TRUE, &err);
		if (err) {
			queue->stopped = TRUE;
		}
	}

	g_static_rec_mutex_unlock (queue->lock);

}

void 
_tny_camel_queue_launch (TnyCamelQueue *queue, GThreadFunc func, gpointer data, const gchar *name)
{
	_tny_camel_queue_launch_wflags (queue, func, data, TNY_CAMEL_QUEUE_NORMAL_ITEM, name);
}

static void 
tny_camel_queue_class_init (TnyCamelQueueClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_camel_queue_finalize;

	return;
}


static void
tny_camel_queue_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyCamelQueue *self = (TnyCamelQueue*)instance;

	self->account = NULL;
	self->stopped = TRUE;
	self->list = NULL;
	self->thread = NULL;
	self->lock = g_new0 (GStaticRecMutex, 1);
	g_static_rec_mutex_init (self->lock);
	self->current = NULL;

	return;
}

/**
 * tny_camel_queue_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
GType 
tny_camel_queue_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
	    static const GTypeInfo info = 
		  {
		  sizeof (TnyCamelQueueClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_camel_queue_class_init, /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyCamelQueue),
		  0,      /* n_preallocs */
		  tny_camel_queue_instance_init,    /* instance_init */
		  NULL
		};

		type = g_type_register_static (G_TYPE_OBJECT,
					       "TnyCamelQueue",
					       &info, 0);
	}

	return type;
}


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
#include <tny-camel-account-priv.h>
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

/**
 * _tny_camel_queue_new
 * @account: the queue
 *
 * Internal, non-public API documentation of Tinymail
 *
 * Make a new queue for @account.
 **/
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
	GSourceFunc callback;
	GDestroyNotify destroyer;
	gpointer data;
	TnyCamelQueueItemFlags flags;
	const gchar *name;
	gboolean *cancel_field;
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

		if (queue->next_uncancel)
		{
			_tny_camel_account_actual_uncancel (TNY_CAMEL_ACCOUNT (queue->account));
			queue->next_uncancel = FALSE;
		}

		first = g_list_first (queue->list);
		if (first) {
			item = first->data;
			queue->current = item;
		} else
			queue->stopped = TRUE;
		g_static_rec_mutex_unlock (queue->lock);

		if (item) {
			tny_debug ("TnyCamelQueue: %s is on the stage, now performing\n", item->name);
			item->func (item->data);
			tny_debug ("TnyCamelQueue: %s is off the stage, done performing\n", item->name);
		}

		g_static_rec_mutex_lock (queue->lock);
		if (first)
			queue->list = g_list_delete_link (queue->list, first);
		queue->current = NULL;

		if (g_list_length (queue->list) == 0) {
			queue->thread = NULL;
			queue->stopped = TRUE;
		}
		g_static_rec_mutex_unlock (queue->lock);

		if (item)
			g_slice_free (QueueItem, item);
	}

	queue->thread = NULL;
	queue->stopped = TRUE;
	g_object_unref (queue);

	g_thread_exit (NULL);
	return NULL;
}

/**
 * _tny_camel_queue_remove_items
 * @queue: the queue
 * @flags: flags
 *
 * Internal, non-public API documentation of Tinymail
 *
 * Remove all items from the queue where their flags match @flags. As items get
 * removed will their callback and destroyer happen and will their cancel_field
 * be set to TRUE.
 **/
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
			if (item && (item->flags & flags)) 
			{
				tny_debug ("TnyCamelQueue: %s 's performance is cancelled\n", item->name);

				if (item->cancel_field)
					*item->cancel_field = TRUE;

				if (item->callback)
					g_idle_add_full (G_PRIORITY_HIGH, item->callback, 
						item->data, item->destroyer);

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

/**
 * _tny_camel_queue_cancel_remove_items
 * @queue: the queue
 * @flags: flags
 *
 * Internal, non-public API documentation of Tinymail
 *
 * Remove all items from the queue where their flags match @flags. As items get
 * removed will their callback and destroyer happen and will their cancel_field
 * be set to TRUE.
 *
 * Also cancel the current item (make the up-next read() of the operation fail).
 * Note that the current item's @callback and @destroyer will not be called as
 * as soon as the item's work func has been launched, it's considered to be 
 * never cancelled. The current-item cancel just means that the operation will
 * be set to fail at its next read() or write operation. The developer of the
 * work func must deal with this himself.
 **/
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
		if (item->flags & TNY_CAMEL_QUEUE_CANCELLABLE_ITEM) {
			if (!(item->flags & TNY_CAMEL_QUEUE_SYNC_ITEM)) {
				_tny_camel_account_actual_cancel (TNY_CAMEL_ACCOUNT (queue->account));
				queue->next_uncancel = TRUE;
			}
		}
	}

	g_static_rec_mutex_unlock (queue->lock);

	return;
}

/**
 * _tny_camel_queue_launch_wflags
 * @queue: the queue
 * @func: the work function
 * @callback: for in case of a cancellation, can be NULL
 * @destroyer: for in case of a cancellation, can be NULL
 * @cancel_field: a byref location of a gboolean that will be set to TRUE in case of a cancellation
 * @data: data that will be passed to @func, @callback and @destroyer
 * @flags: flags of this item
 * @name: a name for this item for debugging (__FUNCTION__ will do)
 *
 * Internal, non-public API documentation of Tinymail
 *
 * Queue a new item. The contract is that @queue will invoke @func in future if
 * it doesn't get cancelled. If it does get cancelled and @callback is not NULL,
 * @callback will be called in the GMainLoop with @destroyer as GDestroyNotify.
 * A cancelled item's @cancel_field will also be set to TRUE.
 **/
void 
_tny_camel_queue_launch_wflags (TnyCamelQueue *queue, GThreadFunc func, GSourceFunc callback, GDestroyNotify destroyer, gboolean *cancel_field, gpointer data, TnyCamelQueueItemFlags flags, const gchar *name)
{
	QueueItem *item = g_slice_new (QueueItem);

	if (!g_thread_supported ())
		g_thread_init (NULL);

	item->func = func;
	item->callback = callback;
	item->destroyer = destroyer;
	item->data = data;
	item->flags = flags;
	item->name = name;
	item->cancel_field = cancel_field;

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

/**
 * _tny_camel_queue_launch
 * @queue: the queue
 * @func: the work function
 * @callback: for in case of a cancellation, can be NULL
 * @destroyer: for in case of a cancellation, can be NULL
 * @cancel_field: a byref location of a gboolean that will be set to TRUE in case of a cancellation
 * @data: data that will be passed to @func, @callback and @destroyer
 * @name: a name for this item for debugging (__FUNCTION__ will do)
 *
 * Internal, non-public API documentation of Tinymail
 *
 * Queue a new item. The contract is that @queue will invoke @func in future if
 * it doesn't get cancelled. If it does get cancelled and @callback is not NULL,
 * @callback will be called in the GMainLoop with @destroyer as GDestroyNotify.
 * A cancelled item's @cancel_field will also be set to TRUE.
 *
 * The flags of the queue item will be TNY_CAMEL_QUEUE_NORMAL_ITEM.
 **/
void 
_tny_camel_queue_launch (TnyCamelQueue *queue, GThreadFunc func, GSourceFunc callback, GDestroyNotify destroyer, gboolean *cancel_field, gpointer data, const gchar *name)
{
	_tny_camel_queue_launch_wflags (queue, func, callback, destroyer, 
		cancel_field, data, TNY_CAMEL_QUEUE_NORMAL_ITEM, name);
	return;
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
	self->next_uncancel = FALSE;

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


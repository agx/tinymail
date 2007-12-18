/* libtinymail - The Tiny Mail queues library
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

#include <tny-status.h>
#include <tny-simple-list.h>
#include <tny-msg.h>
#include <tny-folder.h>
#include <tny-folder-observer.h>

#include <tny-get-msg-queue.h>

static GObjectClass *parent_class = NULL;
static GObjectClass *int_parent_class = NULL;

#include "tny-get-msg-queue-priv.h"


typedef struct {
	TnyGetMsgQueue *self;
	TnyHeader *header;
	TnyGetMsgCallback callback;
	TnyStatusCallback status_callback;
	gpointer user_data;
	TnyFolder *folder;
	guint i;
	GError *err;
} GetMsgInfo;


static gpointer
get_msg_task (TnyQueueTask *task, gpointer arguments)
{
	TnyMsg *retval = NULL;
	GetMsgInfo *info = (GetMsgInfo *) arguments;
	TnyGetMsgQueuePriv *priv = TNY_GET_MSG_QUEUE_GET_PRIVATE (info->self);

	info->err = NULL;

	g_print ("Getting message (%d of %d): %s\n", info->i, priv->total, 
		tny_header_get_subject (info->header));
	info->folder = tny_header_get_folder (info->header);

	if (info->status_callback) {

		TnyStatus *status = tny_status_new (TNY_GET_MSG_QUEUE_STATUS, 
			TNY_GET_MSG_QUEUE_STATUS_GET_MSG,
			info->i, priv->total, "Receiving message");

		/* This could also be the status of the message retrieval itself,
		 * rather than the status of the queue receiving messages. */

		info->status_callback (G_OBJECT (info->self), status, info->user_data);

		tny_status_free (status);
	}

	if (info->folder)
		retval = tny_folder_get_msg (info->folder, info->header, &info->err);

	return (gpointer) retval;
}

static void 
get_msg_callback (TnyQueueTask *task, gpointer func_result)
{
	GetMsgInfo *info = tny_queue_task_get_arguments (task);
	TnyMsg *msg = (TnyMsg *) func_result;

	info->err = NULL;

	if (info->callback && info->folder)
		info->callback (info->folder, FALSE, msg, info->err, info->user_data);

	if (msg)
		g_object_unref (msg);


	g_object_unref (info->header);

	if (info->folder)
		g_object_unref (info->folder);

	g_object_unref (info->self);

	g_slice_free (GetMsgInfo, info);
}


/**
 * tny_get_msg_queue_get_msg:
 * @self: a #TnyGetMsgQueue object
 * @header: a #TnyHeader object
 * @callback: The callback handler
 * @status_callback: The status_callback handler
 * @user_data: user data for the callback
 *
 * Queue getting a message identified by @header
 *
 **/
void
tny_get_msg_queue_get_msg (TnyGetMsgQueue *self, TnyHeader *header, TnyGetMsgCallback callback, TnyStatusCallback status_callback, gpointer user_data)
{
	TNY_GET_MSG_QUEUE_GET_CLASS (self)->get_msg_func (self, header, callback, status_callback, user_data);
	return;
}

static void
tny_get_msg_queue_get_msg_default (TnyGetMsgQueue *self, TnyHeader *header, TnyGetMsgCallback callback, TnyStatusCallback status_callback, gpointer user_data)
{
	TnyGetMsgQueuePriv *priv = TNY_GET_MSG_QUEUE_GET_PRIVATE (self);
	TnyQueueTask *task = tny_queue_create_task (priv->queue);
	GetMsgInfo *info = g_slice_new (GetMsgInfo);

	priv->total++;
	info->i = priv->total;
	info->self = TNY_GET_MSG_QUEUE (g_object_ref (self));
	info->header = TNY_HEADER (g_object_ref (header));
	info->callback = callback;
	info->status_callback = status_callback;
	info->user_data = user_data;

	tny_queue_task_set_arguments (task, info);
	tny_queue_task_set_func (task, get_msg_task);
	tny_queue_task_set_callback (task, get_msg_callback);

	g_mutex_lock (priv->lock);
	tny_queue_add_task (priv->queue, task);
	g_mutex_unlock (priv->lock);

}

typedef struct { 
	GObject parent; 
	TnyGetMsgQueue *queue;
	TnyGetMsgCallback callback;
	TnyStatusCallback status_callback;
	gpointer user_data;
} IntFolderMonitor;

typedef struct { 
	GObjectClass parent; 
} IntFolderMonitorClass;


static void 
get_messages (TnyGetMsgQueue *self, TnyList *list, TnyGetMsgCallback callback, TnyStatusCallback status_callback, gpointer user_data)
{
	TnyIterator *iter;
	iter = tny_list_create_iterator (list);
	while (!tny_iterator_is_done (iter))
	{
		TnyHeader *header = TNY_HEADER (tny_iterator_get_current (iter));

		tny_get_msg_queue_get_msg (self, header, callback, status_callback, user_data);

		g_object_unref (G_OBJECT (header));
		tny_iterator_next (iter);
	}
	g_object_unref (G_OBJECT (iter));
}

static void 
int_folder_monitor_update (TnyFolderObserver *self, TnyFolderChange *change)
{
	TnyFolderChangeChanged changed;
	IntFolderMonitor *mon = (IntFolderMonitor *) self;

	g_object_ref (self);

	changed = tny_folder_change_get_changed (change);

	if (changed & TNY_FOLDER_CHANGE_CHANGED_ADDED_HEADERS)
	{
		TnyList *list = tny_simple_list_new ();
		tny_folder_change_get_added_headers (change, list);
		get_messages (mon->queue, list, mon->callback, mon->status_callback, mon->user_data);
		g_object_unref (G_OBJECT (list));
	}

	g_object_unref (self);

	return;
}

static void
int_folder_monitor_instance_init (GTypeInstance *instance, gpointer g_class) { }
static void
int_folder_monitor_finalize (GObject *object) {
	IntFolderMonitor *self = (IntFolderMonitor *) object;
	g_object_unref (self->queue);
	int_parent_class->finalize (object);
}
static void
int_tny_folder_observer_init (TnyFolderObserverIface *klass) {
	klass->update_func = int_folder_monitor_update; 
}
static void
int_folder_monitor_class_init (TnyFolderMonitorClass *klass) {
	GObjectClass *object_class;
	int_parent_class = g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;
	object_class->finalize = int_folder_monitor_finalize;
}
static GType
int_folder_monitor_get_type (void)
{
	static GType type = 0;
	if (G_UNLIKELY(type == 0)) {
		static const GTypeInfo info = { sizeof (IntFolderMonitorClass),
			NULL, NULL, (GClassInitFunc) int_folder_monitor_class_init, 
			NULL,  NULL, sizeof (IntFolderMonitor), 0,
			int_folder_monitor_instance_init, NULL };
		static const GInterfaceInfo tny_folder_observer_info = {
			(GInterfaceInitFunc) int_tny_folder_observer_init, 
			NULL, NULL };
		type = g_type_register_static (G_TYPE_OBJECT, 
			"IntFolderMonitor", &info, 0);
		g_type_add_interface_static (type, TNY_TYPE_FOLDER_OBSERVER,
			&tny_folder_observer_info); }
	return type;
}

static void
refresh_done (TnyFolder *folder, gboolean cancelled, GError *err, gpointer user_data)
{
	IntFolderMonitor *mon = user_data;
	tny_folder_remove_observer (folder, TNY_FOLDER_OBSERVER (mon));
	g_object_unref (mon);
}
static void
status_update (GObject *sender, TnyStatus *status, gpointer user_data) { }

static void
tny_get_msg_queue_full_msg_retrieval_default (TnyGetMsgQueue *self, TnyFolder *folder, TnyList *headers, TnyGetMsgCallback callback, TnyStatusCallback status_callback, gpointer user_data)
{
	TnyList *list = NULL;

	if (headers)
		list = g_object_ref (headers);
	else {

		IntFolderMonitor *mon = g_object_new (int_folder_monitor_get_type (), NULL);

		mon->queue = TNY_GET_MSG_QUEUE (g_object_ref (self));
		mon->callback = callback;
		mon->status_callback = status_callback;
		mon->user_data = user_data;

		tny_folder_add_observer (folder, TNY_FOLDER_OBSERVER (mon));
		tny_folder_refresh_async (folder, refresh_done, status_update, 
			g_object_ref (mon));

		list = tny_simple_list_new ();
		tny_folder_get_headers (folder, list, FALSE, NULL);

		g_object_unref (mon);
	}

	get_messages (self, list, callback, status_callback, user_data);

	g_object_unref (G_OBJECT (list));

}

/**
 * tny_get_msg_queue_full_msg_retrieval:
 * @self: a #TnyGetMsgQueue object
 * @folder: a #TnyFolder object
 * @headers: a #TnyList with #TnyHeader instances or NULL
 * @callback: The callback handler
 * @status_callback: The status_callback handler
 * @user_data: user data for the callback
 *
 * Queue getting full messages of @folder. If @headers is NULL, all messages
 * will be retrieved while getting the summary.
 *
 **/
void
tny_get_msg_queue_full_msg_retrieval (TnyGetMsgQueue *self, TnyFolder *folder, TnyList *headers, TnyGetMsgCallback callback, TnyStatusCallback status_callback, gpointer user_data)
{
	TNY_GET_MSG_QUEUE_GET_CLASS (self)->full_msg_retrieval_func (self, folder, headers, callback, status_callback, user_data);
	return;
}

/**
 * tny_get_msg_queue_new:
 * @decorated: The #TnyQueue to decorate with this queue
 *
 * Creates a queue that can get messages for you
 *
 * Return value: a new #TnyQueue instance
 **/
TnyQueue*
tny_get_msg_queue_new (TnyQueue *decorated)
{
	TnyGetMsgQueue *self = g_object_new (TNY_TYPE_GET_MSG_QUEUE, NULL);
	TnyGetMsgQueuePriv *priv = TNY_GET_MSG_QUEUE_GET_PRIVATE (self);

	priv->queue = TNY_QUEUE (g_object_ref (decorated));

	return TNY_QUEUE (self);
}

static gint
tny_get_msg_queue_add_task (TnyQueue *self, TnyQueueTask *task)
{
	TnyGetMsgQueuePriv *priv = TNY_GET_MSG_QUEUE_GET_PRIVATE (self);
	return tny_queue_add_task (priv->queue, task);
}


static void
tny_get_msg_queue_join (TnyQueue *self)
{
	TnyGetMsgQueuePriv *priv = TNY_GET_MSG_QUEUE_GET_PRIVATE (self);
	tny_queue_join (priv->queue);
	return;
}

static TnyQueueTask*
tny_get_msg_queue_create_task (TnyQueue *self)
{
	TnyGetMsgQueuePriv *priv = TNY_GET_MSG_QUEUE_GET_PRIVATE (self);
	return tny_queue_create_task (priv->queue);
}


static void
tny_queue_init (TnyQueueIface *klass)
{
	klass->add_task_func = tny_get_msg_queue_add_task;
	klass->join_func = tny_get_msg_queue_join;
	klass->create_task_func = tny_get_msg_queue_create_task;
}


static void
tny_get_msg_queue_finalize (GObject *object)
{
	TnyGetMsgQueuePriv *priv = TNY_GET_MSG_QUEUE_GET_PRIVATE (object);

	g_mutex_lock (priv->lock);
	g_object_unref (priv->queue);
	g_mutex_unlock (priv->lock);
	g_mutex_free (priv->lock);

	parent_class->finalize (object);
}

static void
tny_get_msg_queue_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyGetMsgQueue *self = (TnyGetMsgQueue *)instance;
	TnyGetMsgQueuePriv *priv = TNY_GET_MSG_QUEUE_GET_PRIVATE (self);

	priv->lock = g_mutex_new ();
	priv->total = 0;
	g_mutex_lock (priv->lock);
	priv->queue = NULL;
	g_mutex_unlock (priv->lock);

	return;
}

static void
tny_get_msg_queue_class_init (TnyGetMsgQueueClass *klass)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;

	klass->get_msg_func = tny_get_msg_queue_get_msg_default;
	klass->full_msg_retrieval_func = tny_get_msg_queue_full_msg_retrieval_default;

	object_class->finalize = tny_get_msg_queue_finalize;
	g_type_class_add_private (object_class, sizeof (TnyGetMsgQueuePriv));
}

GType
tny_get_msg_queue_get_type (void)
{
	static GType type = 0;
	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
			sizeof (TnyGetMsgQueueClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			(GClassInitFunc) tny_get_msg_queue_class_init,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			sizeof (TnyGetMsgQueue),
			0,      /* n_preallocs */
			tny_get_msg_queue_instance_init,    /* instance_init */
			NULL
		};

		static const GInterfaceInfo tny_queue_info = 
		{
			(GInterfaceInitFunc) tny_queue_init, /* interface_init */
			NULL,         /* interface_finalize */
			NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyGetMsgQueue",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_QUEUE,
			&tny_queue_info);
	}
	return type;
}

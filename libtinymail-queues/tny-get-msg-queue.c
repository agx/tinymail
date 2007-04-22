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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include <glib.h>
#include <glib/gi18n-lib.h>

#include <tny-get-msg-queue.h>
#include <oasyncworker/oasyncworker.h>

static GObjectClass *parent_class = NULL;

#include "tny-get-msg-queue-priv.h"

typedef struct {
	TnyGetMsgQueue *self;
	TnyHeader *header;
	TnyGetMsgCallback callback;
	gpointer user_data;
	GError *err;
} GetMsgInfo;


static gpointer
get_msg_task (OAsyncWorkerTask *task, gpointer arguments)
{
	TnyMsg *retval = NULL;
	GetMsgInfo *info = (GetMsgInfo *) arguments;
	TnyFolder *folder;

	info->err = NULL;

	folder = tny_header_get_folder (info->header);
	retval = tny_folder_get_msg (folder, info->header, &info->err);

	g_object_unref (folder);

	return (gpointer) retval;
}

static void 
get_msg_callback (OAsyncWorkerTask *task, gpointer func_result)
{
	GetMsgInfo *info = o_async_worker_task_get_arguments (task);
	TnyMsg *msg = (TnyMsg *) func_result;
	TnyFolder *folder;

	info->err = NULL;

	folder = tny_header_get_folder (info->header);

	if (info->callback)
		info->callback (folder, msg, &info->err, info->user_data);

	if (msg)
		g_object_unref (msg);
 
	g_object_unref (folder);
	g_object_unref (info->header);
	g_object_unref (info->self);

	g_slice_free (GetMsgInfo, info);
}


/**
 * tny_get_msg_queue_get_msg:
 * @self: a #TnyGetMsgQueue object
 * @header: a #TnyHeader object
 * @callback: The callback handler
 * @user_data: user data for the callback
 *
 * Queue getting a message identified by @header
 *
 **/
void
tny_get_msg_queue_get_msg (TnyGetMsgQueue *self, TnyHeader *header, TnyGetMsgCallback callback, gpointer user_data)
{
	TnyGetMsgQueuePriv *priv = TNY_GET_MSG_QUEUE_GET_PRIVATE (self);
	OAsyncWorkerTask *task = o_async_worker_task_new ();
	GetMsgInfo *info = g_slice_new (GetMsgInfo);

	info->self = TNY_GET_MSG_QUEUE (g_object_ref (self));
	info->header = TNY_HEADER (g_object_ref (header));
	info->callback = callback;
	info->user_data = user_data;

	o_async_worker_task_set_arguments (task, info);
	o_async_worker_task_set_func (task, get_msg_task);
	o_async_worker_task_set_callback (task, get_msg_callback);

	g_mutex_lock (priv->lock);
	o_async_worker_add (priv->queue, task);
	g_mutex_unlock (priv->lock);

}

/**
 * tny_get_msg_queue_new:
 * @get: a #TnyGet instance
 *
 * Creates a queue that can get messages for you
 *
 * Return value: a new #TnyGetMsgQueue instance
 **/
TnyGetMsgQueue*
tny_get_msg_queue_new (void)
{
	TnyGetMsgQueue *self = g_object_new (TNY_TYPE_GET_MSG_QUEUE, NULL);

	return self;
}


static void
tny_get_msg_queue_finalize (GObject *object)
{
	TnyGetMsgQueuePriv *priv = TNY_GET_MSG_QUEUE_GET_PRIVATE (object);

	g_mutex_lock (priv->lock);
	g_object_unref (G_OBJECT (priv->queue));
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

	g_mutex_lock (priv->lock);
	priv->queue = o_async_worker_new ();
	g_mutex_unlock (priv->lock);

	return;
}

static void
tny_get_msg_queue_class_init (TnyGetMsgQueueClass *klass)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;
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

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyGetMsgQueue",
			&info, 0);
	}
	return type;
}

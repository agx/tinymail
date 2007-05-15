/* libtinymail-asyncworker - The Tiny Mail queues library
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

#include <tny-async-worker-queue.h>
#include <tny-async-worker-queue-task.h>

#include "tny-async-worker-queue-priv.h"
#include "tny-async-worker-queue-task-priv.h"

static GObjectClass *parent_class = NULL;

static TnyQueueTask*
tny_async_worker_queue_create_task (TnyQueue *self)
{
	OAsyncWorkerTask *r_task = o_async_worker_task_new ();
	return _tny_async_worker_queue_task_new (r_task);
}

static gint
tny_async_worker_queue_add_task (TnyQueue *self, TnyQueueTask *task)
{
	TnyAsyncWorkerQueuePriv *priv = TNY_ASYNC_WORKER_QUEUE_GET_PRIVATE (self);
	TnyAsyncWorkerQueueTaskPriv *tpriv = TNY_ASYNC_WORKER_QUEUE_TASK_GET_PRIVATE (task);

	g_assert (TNY_IS_ASYNC_WORKER_QUEUE_TASK (task));

	return o_async_worker_add (priv->real, tpriv->real);
}

static void
tny_async_worker_queue_join (TnyQueue *self)
{
	TnyAsyncWorkerQueuePriv *priv = TNY_ASYNC_WORKER_QUEUE_GET_PRIVATE (self);

	o_async_worker_join (priv->real);

	return;
}

static void
tny_async_worker_queue_finalize (GObject *object)
{
	TnyAsyncWorkerQueue *self = (TnyAsyncWorkerQueue*) object;
	TnyAsyncWorkerQueuePriv *priv = TNY_ASYNC_WORKER_QUEUE_GET_PRIVATE (self);

	g_object_unref (priv->real);

	parent_class->finalize (object);
}

static void
tny_queue_init (TnyQueueIface *klass)
{
	klass->add_task_func = tny_async_worker_queue_add_task;
	klass->join_func = tny_async_worker_queue_join;
	klass->create_task_func = tny_async_worker_queue_create_task;
}


static void
tny_async_worker_queue_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyAsyncWorkerQueue *self = (TnyAsyncWorkerQueue *)instance;
	TnyAsyncWorkerQueuePriv *priv = TNY_ASYNC_WORKER_QUEUE_GET_PRIVATE (self);

	priv->real = NULL;

	return;
}


/**
 * tny_async_worker_queue_new:
 * @r_queue: The real #OAsyncWorker queue
 *
 * Creates a new queue that uses #OAsyncWorker 
 *
 * Return value: a new #TnyQueue instance
 **/
TnyQueue*
tny_async_worker_queue_new (OAsyncWorker *r_queue)
{
	TnyAsyncWorkerQueue *self = g_object_new (TNY_TYPE_ASYNC_WORKER_QUEUE, NULL);
	TnyAsyncWorkerQueuePriv *priv = TNY_ASYNC_WORKER_QUEUE_GET_PRIVATE (self);

	priv->real = O_ASYNC_WORKER (g_object_ref (r_queue));

	return TNY_QUEUE (self);
}

/**
 * tny_async_worker_queue_new_from_scratch:
 *
 * Creates a new queue that uses #OAsyncWorker 
 *
 * Return value: a new #TnyQueue instance
 **/
TnyQueue*
tny_async_worker_queue_new_from_scratch (void)
{
	TnyAsyncWorkerQueue *self = g_object_new (TNY_TYPE_ASYNC_WORKER_QUEUE, NULL);
	TnyAsyncWorkerQueuePriv *priv = TNY_ASYNC_WORKER_QUEUE_GET_PRIVATE (self);

	priv->real = o_async_worker_new ();
	return TNY_QUEUE (self);
}

static void
tny_async_worker_queue_class_init (TnyAsyncWorkerQueueClass *klass)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;
	object_class->finalize = tny_async_worker_queue_finalize;

	g_type_class_add_private (object_class, sizeof (TnyAsyncWorkerQueuePriv));

}

GType
tny_async_worker_queue_get_type (void)
{
	static GType type = 0;
	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
			sizeof (TnyAsyncWorkerQueueClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			(GClassInitFunc) tny_async_worker_queue_class_init,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			sizeof (TnyAsyncWorkerQueue),
			0,      /* n_preallocs */
			tny_async_worker_queue_instance_init,    /* instance_init */
			NULL
		};


		static const GInterfaceInfo tny_queue_info = 
		{
			(GInterfaceInitFunc) tny_queue_init, /* interface_init */
			NULL,         /* interface_finalize */
			NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyAsyncWorkerQueue",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_QUEUE,
			&tny_queue_info);
	}

	return type;
}

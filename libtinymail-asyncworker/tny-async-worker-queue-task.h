#ifndef TNY_ASYNC_WORKER_QUEUE_TASK_H
#define TNY_ASYNC_WORKER_QUEUE_TASK_H

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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <tny-shared.h>
#include <tny-queue-task.h>

G_BEGIN_DECLS

#define TNY_TYPE_ASYNC_WORKER_QUEUE_TASK             (tny_async_worker_queue_task_get_type ())
#define TNY_ASYNC_WORKER_QUEUE_TASK(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_ASYNC_WORKER_QUEUE_TASK, TnyAsyncWorkerQueueTask))
#define TNY_ASYNC_WORKER_QUEUE_TASK_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_ASYNC_WORKER_QUEUE_TASK, TnyAsyncWorkerQueueTaskClass))
#define TNY_IS_ASYNC_WORKER_QUEUE_TASK(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_ASYNC_WORKER_QUEUE_TASK))
#define TNY_IS_ASYNC_WORKER_QUEUE_TASK_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_ASYNC_WORKER_QUEUE_TASK))
#define TNY_ASYNC_WORKER_QUEUE_TASK_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_ASYNC_WORKER_QUEUE_TASK, TnyAsyncWorkerQueueTaskClass))

typedef struct _TnyAsyncWorkerQueueTask TnyAsyncWorkerQueueTask;
typedef struct _TnyAsyncWorkerQueueTaskClass TnyAsyncWorkerQueueTaskClass;

struct _TnyAsyncWorkerQueueTask
{
	GObject parent;
};

struct _TnyAsyncWorkerQueueTaskClass
{
	GObjectClass parent;
};

GType tny_async_worker_queue_task_get_type (void);



G_END_DECLS

#endif



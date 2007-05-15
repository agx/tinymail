#ifndef TNY_ASYNC_WORKER_QUEUE_H
#define TNY_ASYNC_WORKER_QUEUE_H

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

#include <tny-shared.h>
#include <tny-queue.h>

#include <oasyncworker/oasyncworker.h>

G_BEGIN_DECLS

#define TNY_TYPE_ASYNC_WORKER_QUEUE             (tny_async_worker_queue_get_type ())
#define TNY_ASYNC_WORKER_QUEUE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_ASYNC_WORKER_QUEUE, TnyAsyncWorkerQueue))
#define TNY_ASYNC_WORKER_QUEUE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_ASYNC_WORKER_QUEUE, TnyAsyncWorkerQueueClass))
#define TNY_IS_ASYNC_WORKER_QUEUE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_ASYNC_WORKER_QUEUE))
#define TNY_IS_ASYNC_WORKER_QUEUE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_ASYNC_WORKER_QUEUE))
#define TNY_ASYNC_WORKER_QUEUE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_ASYNC_WORKER_QUEUE, TnyAsyncWorkerQueueClass))

typedef struct _TnyAsyncWorkerQueue TnyAsyncWorkerQueue;
typedef struct _TnyAsyncWorkerQueueClass TnyAsyncWorkerQueueClass;

struct _TnyAsyncWorkerQueue
{
	GObject parent;
};

struct _TnyAsyncWorkerQueueClass 
{
	GObjectClass parent;
};

GType tny_async_worker_queue_get_type (void);

TnyQueue* tny_async_worker_queue_new (OAsyncWorker *r_queue);
TnyQueue* tny_async_worker_queue_new_from_scratch (void);


G_END_DECLS

#endif

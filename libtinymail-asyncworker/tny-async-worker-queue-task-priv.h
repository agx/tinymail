#ifndef TNY_ASYNC_WORKER_QUEUE_TASK_PRIV_H
#define TNY_ASYNC_WORKER_QUEUE_TASK_PRIV_H

typedef struct _TnyAsyncWorkerQueueTaskPriv TnyAsyncWorkerQueueTaskPriv;

struct _TnyAsyncWorkerQueueTaskPriv
{
	OAsyncWorkerTask *real;
};

#define TNY_ASYNC_WORKER_QUEUE_TASK_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_ASYNC_WORKER_QUEUE_TASK, TnyAsyncWorkerQueueTaskPriv))

TnyQueueTask* _tny_async_worker_queue_task_new (OAsyncWorkerTask *r_task);


#endif

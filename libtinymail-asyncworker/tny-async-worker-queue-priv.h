#ifndef TNY_ASYNC_WORKER_QUEUE_PRIV_H
#define TNY_ASYNC_WORKER_QUEUE_PRIV_H

typedef struct _TnyAsyncWorkerQueuePriv TnyAsyncWorkerQueuePriv;

struct _TnyAsyncWorkerQueuePriv
{
	OAsyncWorker *real;
};

#define TNY_ASYNC_WORKER_QUEUE_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_ASYNC_WORKER_QUEUE, TnyAsyncWorkerQueuePriv))


#endif

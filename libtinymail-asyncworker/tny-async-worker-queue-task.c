/* Your copyright here */

#include <config.h>
#include <glib.h>
#include <glib/gi18n-lib.h>

#include <oasyncworker/oasyncworker.h>

#include <tny-async-worker-queue-task.h>

#include "tny-async-worker-queue-priv.h"
#include "tny-async-worker-queue-task-priv.h"

static GObjectClass *parent_class = NULL;

typedef struct {
	TnyQueueTask *self;
	TnyQueueTaskFunc func;
	TnyQueueTaskCallback cb;
	gpointer args;
} TheArgs;


static gpointer
launcher (OAsyncWorkerTask *task, gpointer args)
{
	TheArgs *margs = (TheArgs *) o_async_worker_task_get_arguments (task);

	return margs->func (margs->self, margs->args);
}

static void 
the_cb (OAsyncWorkerTask *task, gpointer func_result)
{
	TheArgs *margs = (TheArgs *) o_async_worker_task_get_arguments (task);

	margs->cb (margs->self, func_result);

	g_object_unref (margs->self);
	g_slice_free (TheArgs, margs);

	return;
}

static void
tny_async_worker_queue_task_set_arguments (TnyQueueTask *self, gpointer arguments)
{
	TnyAsyncWorkerQueueTaskPriv *priv = TNY_ASYNC_WORKER_QUEUE_TASK_GET_PRIVATE (self);
	TheArgs *args = o_async_worker_task_get_arguments (priv->real);
	args->args = arguments;
}


static void
tny_async_worker_queue_task_set_func (TnyQueueTask *self, TnyQueueTaskFunc func)
{
	TnyAsyncWorkerQueueTaskPriv *priv = TNY_ASYNC_WORKER_QUEUE_TASK_GET_PRIVATE (self);
	TheArgs *args = o_async_worker_task_get_arguments (priv->real);
	args->func = func;
}

static void
tny_async_worker_queue_task_set_callback (TnyQueueTask *self, TnyQueueTaskCallback callback)
{
	TnyAsyncWorkerQueueTaskPriv *priv = TNY_ASYNC_WORKER_QUEUE_TASK_GET_PRIVATE (self);
	TheArgs *args = o_async_worker_task_get_arguments (priv->real);
	args->cb = callback;
}

static void
tny_async_worker_queue_task_set_priority (TnyQueueTask *self, gint priority)
{
	TnyAsyncWorkerQueueTaskPriv *priv = TNY_ASYNC_WORKER_QUEUE_TASK_GET_PRIVATE (self);
	o_async_worker_task_set_priority (priv->real, priority);
}

static gpointer
tny_async_worker_queue_task_get_arguments (TnyQueueTask *self)
{
	TnyAsyncWorkerQueueTaskPriv *priv = TNY_ASYNC_WORKER_QUEUE_TASK_GET_PRIVATE (self);
	TheArgs *args = o_async_worker_task_get_arguments (priv->real);
	return args->args;
}

static TnyQueueTaskFunc
tny_async_worker_queue_task_get_func (TnyQueueTask *self)
{
	TnyAsyncWorkerQueueTaskPriv *priv = TNY_ASYNC_WORKER_QUEUE_TASK_GET_PRIVATE (self);
	TheArgs *args = tny_queue_task_get_arguments (self);

	return args->func;
}

static TnyQueueTaskCallback
tny_async_worker_queue_task_get_callback (TnyQueueTask *self)
{
	TnyAsyncWorkerQueueTaskPriv *priv = TNY_ASYNC_WORKER_QUEUE_TASK_GET_PRIVATE (self);
	TheArgs *args = tny_queue_task_get_arguments (self);

	return args->cb;
}

static void
tny_async_worker_queue_task_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyAsyncWorkerQueueTask *self = (TnyAsyncWorkerQueueTask *)instance;
	TnyAsyncWorkerQueueTaskPriv *priv = TNY_ASYNC_WORKER_QUEUE_TASK_GET_PRIVATE (self);

	priv->real = NULL;

	return;
}


TnyQueueTask*
_tny_async_worker_queue_task_new (OAsyncWorkerTask *r_task)
{
	TnyAsyncWorkerQueueTask *self = g_object_new (TNY_TYPE_ASYNC_WORKER_QUEUE_TASK, NULL);
	TnyAsyncWorkerQueueTaskPriv *priv = TNY_ASYNC_WORKER_QUEUE_TASK_GET_PRIVATE (self);
	TheArgs *args = g_slice_new0 (TheArgs);

	priv->real = O_ASYNC_WORKER_TASK (g_object_ref (r_task));

	args->self = (TnyQueueTask *) self;
	o_async_worker_task_set_func (priv->real, launcher);
	o_async_worker_task_set_callback (priv->real, the_cb);
	o_async_worker_task_set_arguments (priv->real, args);

	return TNY_QUEUE_TASK (self);
}


static void
tny_async_worker_queue_task_finalize (GObject *object)
{
	TnyAsyncWorkerQueueTask *self = (TnyAsyncWorkerQueueTask*) object;
	TnyAsyncWorkerQueueTaskPriv *priv = TNY_ASYNC_WORKER_QUEUE_TASK_GET_PRIVATE (self);

	/* This is done right after the callback in OAsyncWorker
	 * g_object_unref (priv->real); 
	 * Let's nevertheless measure this soon */

	parent_class->finalize (object);
}

static void
tny_queue_task_init (TnyQueueTaskIface *klass)
{
	klass->set_arguments_func = tny_async_worker_queue_task_set_arguments;
	klass->set_func_func = tny_async_worker_queue_task_set_func;
	klass->set_callback_func = tny_async_worker_queue_task_set_callback;
	klass->set_priority_func = tny_async_worker_queue_task_set_priority;
	klass->get_arguments_func = tny_async_worker_queue_task_get_arguments;
	klass->get_func_func = tny_async_worker_queue_task_get_func;
	klass->get_callback_func = tny_async_worker_queue_task_get_callback;
}

static void
tny_async_worker_queue_task_class_init (TnyAsyncWorkerQueueTaskClass *klass)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;
	object_class->finalize = tny_async_worker_queue_task_finalize;

	g_type_class_add_private (object_class, sizeof (TnyAsyncWorkerQueueTaskPriv));

}

GType
tny_async_worker_queue_task_get_type (void)
{
	static GType type = 0;
	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
			sizeof (TnyAsyncWorkerQueueTaskClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			(GClassInitFunc) tny_async_worker_queue_task_class_init,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			sizeof (TnyAsyncWorkerQueueTask),
			0,      /* n_preallocs */
			tny_async_worker_queue_task_instance_init,    /* instance_init */
			NULL
		};


		static const GInterfaceInfo tny_queue_task_info = 
		{
			(GInterfaceInitFunc) tny_queue_task_init, /* interface_init */
			NULL,         /* interface_finalize */
			NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyAsyncWorkerQueueTask",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_QUEUE_TASK,
			&tny_queue_task_info);
	}

	return type;
}

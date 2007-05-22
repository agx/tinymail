/* libtinymail - The Tiny Mail queue library
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

#include <tny-queue-task.h>


/**
 * tny_queue_task_set_arguments:
 * @task: A #TnyQueueTask instance
 * @arguments: arguments
 *
 * Set the arguments of @task
 **/
void 
tny_queue_task_set_arguments (TnyQueueTask *task, gpointer arguments)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_QUEUE_TASK (task));
	g_assert (TNY_QUEUE_TASK_GET_IFACE (task)->set_arguments_func != NULL);
#endif

	TNY_QUEUE_TASK_GET_IFACE (task)->set_arguments_func (task, arguments);


#ifdef DBC /* ensure */
	g_assert (arguments == tny_queue_task_get_arguments (task));
#endif
}



/**
 * tny_queue_task_set_func:
 * @task: A #TnyQueueTask instance
 * @func: the task's func
 *
 * Set the function to launch as task
 **/
void 
tny_queue_task_set_func (TnyQueueTask *task, TnyQueueTaskFunc func)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_QUEUE_TASK (task));
	g_assert (TNY_QUEUE_TASK_GET_IFACE (task)->set_func_func != NULL);
#endif

	TNY_QUEUE_TASK_GET_IFACE (task)->set_func_func (task, func);

#ifdef DBC /* ensure */
	g_assert (func == tny_queue_task_get_func (task));
#endif
	return;
}


/**
 * tny_queue_task_set_callback:
 * @task: A #TnyQueueTask instance
 * @callback: the task's callback
 *
 * Set the function to launch in the mainloop when the task is finished
 **/
void
tny_queue_task_set_callback (TnyQueueTask *task, TnyQueueTaskCallback callback)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_QUEUE_TASK (task));
	g_assert (TNY_QUEUE_TASK_GET_IFACE (task)->set_callback_func != NULL);
#endif

	TNY_QUEUE_TASK_GET_IFACE (task)->set_callback_func (task, callback);

#ifdef DBC /* ensure */
	g_assert (callback == tny_queue_task_get_callback (task));
#endif

	return;
}

/**
 * tny_queue_task_set_priority:
 * @task: A #TnyQueueTask instance
 * @priority: the task's priority
 *
 * Set the priority of this task, note that the queue itself is not guaranteed
 * to be preemptive (running tasks with a lower priority will not be cancelled
 * nor interupted).
 **/
void
tny_queue_task_set_priority (TnyQueueTask *task, gint priority)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_QUEUE_TASK (task));
	g_assert (TNY_QUEUE_TASK_GET_IFACE (task)->set_priority_func != NULL);
#endif

	TNY_QUEUE_TASK_GET_IFACE (task)->set_priority_func (task, priority);

#ifdef DBC /* ensure */
	/* g_assert (priority == tny_queue_task_get_priority (task)); */
#endif

	return;
}


/**
 * tny_queue_task_get_arguments:
 * @task: A #TnyQueueTask instance
 *
 * Get the arguments of @task
 *
 * Return value: the arguments of @task
 **/
gpointer
tny_queue_task_get_arguments (TnyQueueTask *task)
{
	gpointer retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_QUEUE_TASK (task));
	g_assert (TNY_QUEUE_TASK_GET_IFACE (task)->get_arguments_func != NULL);
#endif

	retval = TNY_QUEUE_TASK_GET_IFACE (task)->get_arguments_func (task);

	return retval;
}

/**
 * tny_queue_task_get_func:
 * @task: A #TnyQueueTask instance
 *
 * Get the function to launch of @task
 *
 * Return value: the function to launch of @task
 **/
TnyQueueTaskFunc 
tny_queue_task_get_func (TnyQueueTask *task)
{
	TnyQueueTaskFunc retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_QUEUE_TASK (task));
	g_assert (TNY_QUEUE_TASK_GET_IFACE (task)->get_func_func != NULL);
#endif

	retval = TNY_QUEUE_TASK_GET_IFACE (task)->get_func_func (task);

	return retval;
}

/**
 * tny_queue_task_get_callback:
 * @task: A #TnyQueueTask instance
 *
 * Get the callback to launch after @task finished of @task
 *
 * Return value: the callback to launch after the task finished of @task
 **/
TnyQueueTaskCallback
tny_queue_task_get_callback (TnyQueueTask *task)
{
	TnyQueueTaskCallback retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_QUEUE_TASK (task));
	g_assert (TNY_QUEUE_TASK_GET_IFACE (task)->get_callback_func != NULL);
#endif

	retval = TNY_QUEUE_TASK_GET_IFACE (task)->get_callback_func (task);

	return retval;
}


static void
tny_queue_task_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_queue_task_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyQueueTaskIface),
		  tny_queue_task_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL,    /* instance_init */
		  NULL
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyQueueTask", &info, 0);
	}

	return type;
}


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

#include <tny-queue.h>


/**
 * tny_queue_create_task:
 * @self: a #TnyQueue object
 * 
 * Create a task for @self
 * 
 * Return value: a new #TnyQueueTask for @self
 **/
TnyQueueTask* 
tny_queue_create_task (TnyQueue *self)
{
	TnyQueueTask *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_QUEUE (self));
	g_assert (TNY_QUEUE_GET_IFACE (self)->create_task_func != NULL);
#endif

	retval = TNY_QUEUE_GET_IFACE (self)->create_task_func (self);

#ifdef DBC /* ensure */
	g_assert (TNY_IS_QUEUE_TASK (retval));
#endif

	return retval;
}

/**
 * tny_queue_join:
 * @self: a #TnyQueue object
 * 
 * Join with @self. This means that your context will wait for @self to complete
 * all its tasks.
 **/
void
tny_queue_join (TnyQueue *self)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_QUEUE (self));
	g_assert (TNY_QUEUE_GET_IFACE (self)->join_func != NULL);
#endif

	TNY_QUEUE_GET_IFACE (self)->join_func (self);

	return;
}

/**
 * tny_queue_add_task:
 * @self: a #TnyQueue object
 * @task: a #TnyQueueTask object
 * 
 * Add @task to @self. The queue will act by trying to launch the task as soon
 * as possible or according to the priority of @task when compared to other 
 * tasks in @self.
 *
 * You must not unreference @task yourself. The queue @self will do this for you
 * when it's finished processing @task.
 *
 * Return value: a unique id on @self
 **/
gint
tny_queue_add_task (TnyQueue *self, TnyQueueTask *task)
{
	gint retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_QUEUE (self));
	g_assert (TNY_IS_QUEUE_TASK (task));
	g_assert (TNY_QUEUE_GET_IFACE (self)->add_task_func != NULL);
#endif

	retval = TNY_QUEUE_GET_IFACE (self)->add_task_func (self, task);

	return retval;
}


static void
tny_queue_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_queue_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0)) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyQueueIface),
		  tny_queue_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL,   /* instance_init */
		  NULL
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyQueue", &info, 0);
	}

	return type;
}



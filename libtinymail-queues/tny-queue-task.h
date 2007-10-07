#ifndef TNY_QUEUE_TASK_H
#define TNY_QUEUE_TASK_H

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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <glib.h>
#include <glib-object.h>
#include <tny-shared.h>

G_BEGIN_DECLS

#define TNY_TYPE_QUEUE_TASK             (tny_queue_task_get_type ())
#define TNY_QUEUE_TASK(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_QUEUE_TASK, TnyQueueTask))
#define TNY_IS_QUEUE_TASK(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_QUEUE_TASK))
#define TNY_QUEUE_TASK_GET_IFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_QUEUE_TASK, TnyQueueTaskIface))

typedef struct _TnyQueueTask TnyQueueTask;
typedef struct _TnyQueueTaskIface TnyQueueTaskIface;
typedef gpointer (*TnyQueueTaskFunc) (TnyQueueTask *task, gpointer arguments);
typedef void (*TnyQueueTaskCallback) (TnyQueueTask *task, gpointer func_result);

struct _TnyQueueTaskIface
{
	GTypeInterface parent;

	void (*set_arguments_func) (TnyQueueTask *task, gpointer arguments);
	void (*set_func_func) (TnyQueueTask *task, TnyQueueTaskFunc func);
	void (*set_callback_func) (TnyQueueTask *task, TnyQueueTaskCallback callback);
	void (*set_priority_func) (TnyQueueTask *task, gint priority);
	gpointer (*get_arguments_func) (TnyQueueTask *task);
	TnyQueueTaskFunc (*get_func_func) (TnyQueueTask *task);
	TnyQueueTaskCallback (*get_callback_func) (TnyQueueTask *task);
};

GType tny_queue_task_get_type (void);


void tny_queue_task_set_arguments (TnyQueueTask *task, gpointer arguments);
void tny_queue_task_set_func (TnyQueueTask *task, TnyQueueTaskFunc func);
void tny_queue_task_set_callback (TnyQueueTask *task, TnyQueueTaskCallback callback);
void tny_queue_task_set_priority (TnyQueueTask *task, gint priority);
gpointer tny_queue_task_get_arguments (TnyQueueTask *task);
TnyQueueTaskFunc tny_queue_task_get_func (TnyQueueTask *task);
TnyQueueTaskCallback tny_queue_task_get_callback (TnyQueueTask *task);

G_END_DECLS

#endif

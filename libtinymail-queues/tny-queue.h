#ifndef TNY_QUEUE_H
#define TNY_QUEUE_H

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

#include <glib.h>
#include <tny-shared.h>
#include <tny-queue-task.h>

G_BEGIN_DECLS

#define TNY_TYPE_QUEUE             (tny_queue_get_type ())
#define TNY_QUEUE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_QUEUE, TnyQueue))
#define TNY_IS_QUEUE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_QUEUE))
#define TNY_QUEUE_GET_IFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_QUEUE, TnyQueueIface))

typedef struct _TnyQueue TnyQueue;
typedef struct _TnyQueueIface TnyQueueIface;

struct _TnyQueueIface
{
	GTypeInterface parent;

	gint (*add_task_func) (TnyQueue *self, TnyQueueTask *task);
	void (*join_func) (TnyQueue *self);
	TnyQueueTask* (*create_task_func) (TnyQueue *self);
};

GType tny_queue_get_type (void);

gint tny_queue_add_task (TnyQueue *self, TnyQueueTask *task);
void tny_queue_join (TnyQueue *self);
TnyQueueTask* tny_queue_create_task (TnyQueue *self);

G_END_DECLS

#endif

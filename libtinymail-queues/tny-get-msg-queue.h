#ifndef TNY_GET_MSG_QUEUE_H
#define TNY_GET_MSG_QUEUE_H

/* libtinymail-queues - The Tiny Mail queues library
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

#include <tny-folder.h>
#include <tny-msg.h>

G_BEGIN_DECLS

#define TNY_TYPE_GET_MSG_QUEUE             (tny_get_msg_queue_get_type ())
#define TNY_GET_MSG_QUEUE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_GET_MSG_QUEUE, TnyGetMsgQueue))
#define TNY_GET_MSG_QUEUE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_GET_MSG_QUEUE, TnyGetMsgQueueClass))
#define TNY_IS_GET_MSG_QUEUE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_GET_MSG_QUEUE))
#define TNY_IS_GET_MSG_QUEUE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_GET_MSG_QUEUE))
#define TNY_GET_MSG_QUEUE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_GET_MSG_QUEUE, TnyGetMsgQueueClass))

typedef struct _TnyGetMsgQueue TnyGetMsgQueue;
typedef struct _TnyGetMsgQueueClass TnyGetMsgQueueClass;

struct _TnyGetMsgQueue 
{
	GObject parent;
};

struct _TnyGetMsgQueueClass 
{
	GObjectClass parent;
};

GType tny_get_msg_queue_get_type (void);
TnyGetMsgQueue* tny_get_msg_queue_new (void);

void tny_get_msg_queue_get_msg (TnyGetMsgQueue *self, TnyHeader *header, TnyGetMsgCallback callback, gpointer user_data);


G_END_DECLS

#endif

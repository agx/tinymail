#ifndef TNY_CAMEL_SEND_QUEUE_H
#define TNY_CAMEL_SEND_QUEUE_H

/* libtinymail-camel - The Tiny Mail base library for Camel
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
#include <glib-object.h>

#include <tny-send-queue.h>
#include <tny-msg.h>

G_BEGIN_DECLS

#define TNY_TYPE_CAMEL_SEND_QUEUE             (tny_camel_send_queue_get_type ())
#define TNY_CAMEL_SEND_QUEUE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_CAMEL_SEND_QUEUE, TnyCamelSendQueue))
#define TNY_CAMEL_SEND_QUEUE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_CAMEL_SEND_QUEUE, TnyCamelSendQueueClass))
#define TNY_IS_CAMEL_SEND_QUEUE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_CAMEL_SEND_QUEUE))
#define TNY_IS_CAMEL_SEND_QUEUE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_CAMEL_SEND_QUEUE))
#define TNY_CAMEL_SEND_QUEUE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_CAMEL_SEND_QUEUE, TnyCamelSendQueueClass))

typedef struct _TnyCamelSendQueue TnyCamelSendQueue;
typedef struct _TnyCamelSendQueueClass TnyCamelSendQueueClass;

struct _TnyCamelSendQueue
{
	GObject parent;
};

struct _TnyCamelSendQueueClass 
{
	GObjectClass parent;

	/* virtual methods */
	void (*add_func) (TnySendQueue *self, TnyMsg *msg);
	TnyFolder* (*get_sentbox_func) (TnySendQueue *self);
	TnyFolder* (*get_outbox_func) (TnySendQueue *self);
};

GType tny_camel_send_queue_get_type (void);

TnySendQueue* tny_camel_send_queue_new (TnyTransportAccount *trans_account);

G_END_DECLS

#endif


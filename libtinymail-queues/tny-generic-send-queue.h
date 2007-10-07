#ifndef TNY_GENERIC_SEND_QUEUE_H
#define TNY_GENERIC_SEND_QUEUE_H

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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include <tny-shared.h>
#include <tny-queue.h>
#include <tny-queue-task.h>
#include <tny-send-queue.h>
#include <tny-msg.h>
#include <tny-folder.h>
#include <tny-transport-account.h>

G_BEGIN_DECLS

#define TNY_TYPE_GENERIC_SEND_QUEUE             (tny_generic_send_queue_get_type ())
#define TNY_GENERIC_SEND_QUEUE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_GENERIC_SEND_QUEUE, TnyGenericSendQueue))
#define TNY_GENERIC_SEND_QUEUE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_GENERIC_SEND_QUEUE, TnyGenericSendQueueClass))
#define TNY_IS_GENERIC_SEND_QUEUE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_GENERIC_SEND_QUEUE))
#define TNY_IS_GENERIC_SEND_QUEUE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_GENERIC_SEND_QUEUE))
#define TNY_GENERIC_SEND_QUEUE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_GENERIC_SEND_QUEUE, TnyGenericSendQueueClass))

typedef struct _TnyGenericSendQueue TnyGenericSendQueue;
typedef struct _TnyGenericSendQueueClass TnyGenericSendQueueClass;

struct _TnyGenericSendQueue 
{
	GObject parent;
};

struct _TnyGenericSendQueueClass 
{
	GObjectClass parent;

	/* virtual methods */
	void (*add_func) (TnySendQueue *self, TnyMsg *msg, GError **err);
	TnyFolder* (*get_sentbox_func) (TnySendQueue *self);
	TnyFolder* (*get_outbox_func) (TnySendQueue *self);
	void (*cancel_func) (TnySendQueue *self, gboolean remove, GError **err);
	void (*update_func) (TnyFolderObserver *self, TnyFolderChange *change);
};

GType tny_generic_send_queue_get_type (void);

TnySendQueue* tny_generic_send_queue_new (TnyQueue *decorated, TnyTransportAccount *account, TnyFolder *outbox, TnyFolder *sentbox);

G_END_DECLS

#endif

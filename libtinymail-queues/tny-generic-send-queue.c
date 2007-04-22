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

#include <config.h>
#include <glib.h>
#include <glib/gi18n-lib.h>

#include <oasyncworker/oasyncworker.h>

#include <tny-generic-send-queue.h>
#include <tny-simple-list.h>

static GObjectClass *parent_class = NULL;

#include "tny-generic-send-queue-priv.h"

typedef struct {
	TnyGenericSendQueue *self;
	TnyMsg *msg;
	GError **err;
} GenericSendInfo;


static gpointer
generic_send_task (OAsyncWorkerTask *task, gpointer arguments)
{
	GenericSendInfo *info = (GenericSendInfo *) arguments;
	TnyGenericSendQueuePriv *priv = TNY_GENERIC_SEND_QUEUE_GET_PRIVATE (info->self);
	TnyList *list = tny_simple_list_new ();

	g_mutex_lock (priv->lock);

	tny_transport_account_send (priv->account, info->msg, info->err);
	/* TODO handle err */

	if (info->err == NULL)
	{
		tny_list_prepend (list, G_OBJECT (info->msg));
		tny_folder_transfer_msgs (priv->outbox, list, priv->sentbox, TRUE, info->err);
		g_object_unref (list);
		/* TODO handle err */
	}

	g_object_unref (info->msg);

	g_mutex_unlock (priv->lock);

	return NULL;
}

static void 
generic_send_callback (OAsyncWorkerTask *task, gpointer func_result)
{
	GenericSendInfo *info = o_async_worker_task_get_arguments (task);

	g_object_unref (info->self);
	g_slice_free (GenericSendInfo, info);
}

static void
tny_generic_send_queue_add (TnySendQueue *self, TnyMsg *msg, GError **err)
{
	TnyGenericSendQueuePriv *priv = TNY_GENERIC_SEND_QUEUE_GET_PRIVATE (self);
	TnyIterator *iter;
	TnyList *list = tny_simple_list_new ();

	g_mutex_lock (priv->lock);

	tny_folder_add_msg (priv->outbox, msg, err);
	/* TODO: handle err */

	tny_folder_get_headers (priv->outbox, list, FALSE, err);
	/* TODO: handle err */

	iter = tny_list_create_iterator (list);

	while (!tny_iterator_is_done (iter))
	{
		OAsyncWorkerTask *task = o_async_worker_task_new ();
		GenericSendInfo *info = g_slice_new (GenericSendInfo);
		TnyHeader *header = TNY_HEADER (tny_iterator_get_current (iter));

		info->self = TNY_GENERIC_SEND_QUEUE (g_object_ref (self));
		info->msg = tny_folder_get_msg (priv->outbox, header, err);

		/* TODO: Handle err */

		info->err = err;
		o_async_worker_task_set_arguments (task, info);
		o_async_worker_task_set_func (task, generic_send_task);
		o_async_worker_task_set_callback (task, generic_send_callback);

		o_async_worker_add (priv->queue, task);

		g_object_unref (header);
		tny_iterator_next (iter);
	}

	g_object_unref (iter);
	g_object_unref (list);

	g_mutex_unlock (priv->lock);
}



static void
tny_generic_send_queue_cancel (TnySendQueue *self, gboolean remove, GError **err)
{
	/* TODO */
}


static TnyFolder*
tny_generic_send_queue_get_sentbox (TnySendQueue *self)
{
	TnyGenericSendQueuePriv *priv = TNY_GENERIC_SEND_QUEUE_GET_PRIVATE (self);
	return TNY_FOLDER (g_object_ref (priv->sentbox));
}


static TnyFolder*
tny_generic_send_queue_get_outbox (TnySendQueue *self)
{
	TnyGenericSendQueuePriv *priv = TNY_GENERIC_SEND_QUEUE_GET_PRIVATE (self);
	return TNY_FOLDER (g_object_ref (priv->outbox));
}

/**
 * tny_generic_send_queue_new:
 * @account: a #TnyTransportAccount object
 * @outbox: a #TnyFolder object
 * @sentbox: a #TnyFolder object
 *
 * Creates a queue that can send messages using @account, storing unsent 
 * messages in @outbox and sent messages in @sentbox.
 *
 * Return value: a new #TnyGenericSendQueue instance
 **/
TnyGenericSendQueue*
tny_generic_send_queue_new (TnyTransportAccount *account, TnyFolder *outbox, TnyFolder *sentbox)
{
	TnyGenericSendQueue *self = g_object_new (TNY_TYPE_GENERIC_SEND_QUEUE, NULL);
	TnyGenericSendQueuePriv *priv = TNY_GENERIC_SEND_QUEUE_GET_PRIVATE (self);

	priv->account = TNY_TRANSPORT_ACCOUNT (g_object_ref (account));
	priv->outbox = TNY_FOLDER (g_object_ref (outbox));
	priv->sentbox = TNY_FOLDER (g_object_ref (sentbox));

	return self;
}


static void
tny_generic_send_queue_finalize (GObject *object)
{
	TnyGenericSendQueuePriv *priv = TNY_GENERIC_SEND_QUEUE_GET_PRIVATE (object);

	g_mutex_lock (priv->lock);
	o_async_worker_join (priv->queue);
	g_object_unref (G_OBJECT (priv->queue));
	g_object_unref (G_OBJECT (priv->sentbox));
	g_object_unref (G_OBJECT (priv->outbox));
	g_object_unref (G_OBJECT (priv->account));
	g_mutex_unlock (priv->lock);
	g_mutex_free (priv->lock);

	parent_class->finalize (object);
}

static void
tny_generic_send_queue_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyGenericSendQueue *self = (TnyGenericSendQueue *)instance;
	TnyGenericSendQueuePriv *priv = TNY_GENERIC_SEND_QUEUE_GET_PRIVATE (self);

	priv->lock = g_mutex_new ();

	g_mutex_lock (priv->lock);
	priv->queue = o_async_worker_new ();
	priv->account = NULL;
	priv->sentbox = NULL;
	priv->outbox = NULL;
	g_mutex_unlock (priv->lock);

	return;
}

static void
tny_generic_send_queue_class_init (TnyGenericSendQueueClass *klass)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;
	object_class->finalize = tny_generic_send_queue_finalize;
	g_type_class_add_private (object_class, sizeof (TnyGenericSendQueuePriv));

	return;
}

static void
tny_send_queue_init (gpointer g, gpointer iface_data)
{
	TnySendQueueIface *klass = (TnySendQueueIface *)g;

	klass->add_func = tny_generic_send_queue_add;
	klass->get_sentbox_func = tny_generic_send_queue_get_sentbox;
	klass->get_outbox_func = tny_generic_send_queue_get_outbox;
	klass->cancel_func = tny_generic_send_queue_cancel;

	return;
}

GType
tny_generic_send_queue_generic_type (void)
{
	static GType type = 0;
	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
			sizeof (TnyGenericSendQueueClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			(GClassInitFunc) tny_generic_send_queue_class_init,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			sizeof (TnyGenericSendQueue),
			0,      /* n_preallocs */
			tny_generic_send_queue_instance_init,    /* instance_init */
			NULL
		};

		static const GInterfaceInfo tny_send_queue_info = 
		{
		  (GInterfaceInitFunc) tny_send_queue_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyGenericSendQueue",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_SEND_QUEUE,
			&tny_send_queue_info);

	}
	return type;
}

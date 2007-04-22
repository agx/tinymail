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
	TnySendQueue *self;
	TnyMsg *msg;
	GError *error;
	gint i, total;
} ErrorInfo;

static gboolean 
emit_error_on_mainloop (gpointer data)
{
	ErrorInfo *info = data;
	g_signal_emit (info->self, tny_send_queue_signals [TNY_SEND_QUEUE_ERROR_HAPPENED], 
		0, info->msg, info->error, info->i, info->total);
	return FALSE;
}


static void
destroy_error_info (gpointer data)
{
	ErrorInfo *info = data;
	if (info->msg)
		g_object_unref (G_OBJECT (info->msg));
	if (info->self)
		g_object_unref (G_OBJECT (info->self));
	if (info->error)
		g_error_free (info->error);
	g_slice_free (ErrorInfo, info);
}

static void
emit_error (TnySendQueue *self, TnyMsg *msg, GError *error, int i, int total)
{
	ErrorInfo *info = g_slice_new0 (ErrorInfo);
	if (error != NULL)
		info->error = g_error_copy ((const GError *) error);
	if (self)
		info->self = TNY_SEND_QUEUE (g_object_ref (G_OBJECT (self)));
	if (msg)
		info->msg = TNY_MSG (g_object_ref (G_OBJECT (msg)));
	info->i = i;
	info->total = total;
	g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
		emit_error_on_mainloop, info, destroy_error_info);
	return;
}

typedef struct {
	TnyGenericSendQueue *self;
	TnyMsg *msg;
	gint i, total;
} GenericSendInfo;


static gpointer
generic_send_task (OAsyncWorkerTask *task, gpointer arguments)
{
	GenericSendInfo *info = (GenericSendInfo *) arguments;
	TnySendQueue *self = (TnySendQueue *) info->self;
	TnyGenericSendQueuePriv *priv = TNY_GENERIC_SEND_QUEUE_GET_PRIVATE (info->self);
	TnyList *list;
	GError *err = NULL;

	if (priv->cancelled)
		return;

	list = tny_simple_list_new ();
	tny_transport_account_send (priv->account, info->msg, &err);
	if (err != NULL) {
		emit_error (self, info->msg, err, info->i, info->total);
		g_error_free (err);
	} else {
		TnyFolder *outbox, *sentbox;

		outbox = tny_send_queue_get_outbox (TNY_SEND_QUEUE (info->self));
		sentbox = tny_send_queue_get_sentbox (TNY_SEND_QUEUE (info->self));

		tny_list_prepend (list, G_OBJECT (info->msg));

		g_mutex_lock (priv->lock);
		tny_folder_transfer_msgs (outbox, list, sentbox, TRUE, &err);
		g_mutex_unlock (priv->lock);

		g_object_unref (list);

		if (err != NULL) {
			emit_error (self, info->msg, err, info->i, info->total);
			g_error_free (err);
		}

		g_object_unref (outbox);
		g_object_unref (sentbox);
	}
	g_object_unref (info->msg);

	return NULL;
}

static void 
generic_send_callback (OAsyncWorkerTask *task, gpointer func_result)
{
	GenericSendInfo *info = o_async_worker_task_get_arguments (task);
	TnyGenericSendQueuePriv *priv = TNY_GENERIC_SEND_QUEUE_GET_PRIVATE (info->self);

	g_object_unref (info->self);
	g_slice_free (GenericSendInfo, info);
}

static void
tny_generic_send_queue_add (TnySendQueue *self, TnyMsg *msg, GError **err)
{
	TNY_GENERIC_SEND_QUEUE_GET_CLASS (self)->add_func (self, msg, err);
	return;
}

static void
tny_generic_send_queue_add_default (TnySendQueue *self, TnyMsg *msg, GError **err)
{
	TnyGenericSendQueuePriv *priv = TNY_GENERIC_SEND_QUEUE_GET_PRIVATE (self);
	TnyIterator *iter;
	TnyList *list;
	gint i=1, total;

	TnyFolder *outbox;

	g_mutex_lock (priv->lock);

	outbox = tny_send_queue_get_outbox (self);

	tny_folder_add_msg (outbox, msg, err);

	if (err!= NULL && *err != NULL) {
		g_object_unref (G_OBJECT (outbox));
		g_mutex_unlock (priv->lock);
		return;
	}

	list = tny_simple_list_new ();
	tny_folder_get_headers (outbox, list, FALSE, err);

	if (err != NULL && *err != NULL)
	{
		g_object_unref (G_OBJECT (outbox));
		g_object_unref (G_OBJECT (list));
		g_mutex_unlock (priv->lock);
		return;
	}

	total = tny_list_get_length (list);
	iter = tny_list_create_iterator (list);

	while (!tny_iterator_is_done (iter))
	{
		OAsyncWorkerTask *task = o_async_worker_task_new ();
		GenericSendInfo *info = g_slice_new (GenericSendInfo);
		TnyHeader *header = TNY_HEADER (tny_iterator_get_current (iter));
		guint item = 0;

		info->msg = NULL;
		info->i = i;
		info->total = total;
		info->self = TNY_GENERIC_SEND_QUEUE (g_object_ref (self));
		info->msg = tny_folder_get_msg (priv->outbox, header, err);

		if (err != NULL && *err != NULL)
		{
			g_object_unref (G_OBJECT (task));
			g_object_unref (G_OBJECT (info->self));
			if (info->msg)
				g_object_unref (G_OBJECT (info->msg));
			g_object_unref (G_OBJECT (header));
			g_object_unref (G_OBJECT (iter));
			g_object_unref (G_OBJECT (list));
			g_slice_free (GenericSendInfo, info);
			g_object_unref (G_OBJECT (outbox));
			g_mutex_unlock (priv->lock);
			return;
		}

		o_async_worker_task_set_arguments (task, info);
		o_async_worker_task_set_func (task, generic_send_task);
		o_async_worker_task_set_callback (task, generic_send_callback);

		item = o_async_worker_add (priv->queue, task);

		g_object_unref (header);
		tny_iterator_next (iter); 
		i++;
	}

	g_object_unref (iter);
	g_object_unref (list);
	g_object_unref (G_OBJECT (outbox));

	g_mutex_unlock (priv->lock);
}



static void
tny_generic_send_queue_cancel (TnySendQueue *self, gboolean remove, GError **err)
{
	TNY_GENERIC_SEND_QUEUE_GET_CLASS (self)->cancel_func (self, remove, err);
	return;
}

static void
tny_generic_send_queue_cancel_default (TnySendQueue *self, gboolean remove, GError **err)
{
	TnyGenericSendQueuePriv *priv = TNY_GENERIC_SEND_QUEUE_GET_PRIVATE (self);

	priv->cancelled = TRUE;
	o_async_worker_join (priv->queue);

	g_mutex_lock (priv->lock);
	if (remove)
	{
		TnyFolder *outbox;
		TnyList *headers = tny_simple_list_new ();
		TnyIterator *iter;

		outbox = tny_send_queue_get_outbox (self);

		tny_folder_get_headers (outbox, headers, TRUE, err);

		if (err != NULL && *err != NULL)
		{
			g_object_unref (G_OBJECT (headers));
			g_object_unref (G_OBJECT (outbox));
			priv->cancelled = FALSE;
			g_mutex_unlock (priv->lock);
			return;
		}

		iter = tny_list_create_iterator (headers);

		while (!tny_iterator_is_done (iter))
		{
			TnyHeader *header = TNY_HEADER (tny_iterator_get_current (iter));
			tny_folder_remove_msg (outbox, header, err);

			if (err != NULL && *err != NULL)
			{
				g_object_unref (G_OBJECT (header));
				g_object_unref (G_OBJECT (iter));
				g_object_unref (G_OBJECT (headers));
				g_object_unref (G_OBJECT (outbox));
				priv->cancelled = FALSE;
				g_mutex_unlock (priv->lock);
				return;
			}

			g_object_unref (G_OBJECT (header));
			tny_iterator_next (iter);
		}
		g_object_unref (G_OBJECT (iter));
		g_object_unref (G_OBJECT (headers));

		tny_folder_sync (outbox, TRUE, err);

		g_object_unref (G_OBJECT (outbox));
	}

	priv->cancelled = FALSE;
	g_mutex_unlock (priv->lock);

	return;
}


static TnyFolder*
tny_generic_send_queue_get_sentbox (TnySendQueue *self)
{
	return TNY_GENERIC_SEND_QUEUE_GET_CLASS (self)->get_sentbox_func (self);
}

static TnyFolder*
tny_generic_send_queue_get_sentbox_default (TnySendQueue *self)
{
	TnyGenericSendQueuePriv *priv = TNY_GENERIC_SEND_QUEUE_GET_PRIVATE (self);
	return TNY_FOLDER (g_object_ref (priv->sentbox));
}

static TnyFolder*
tny_generic_send_queue_get_outbox (TnySendQueue *self)
{
	return TNY_GENERIC_SEND_QUEUE_GET_CLASS (self)->get_outbox_func (self);
}

static TnyFolder*
tny_generic_send_queue_get_outbox_default (TnySendQueue *self)
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
 * Return value: a new #TnySendQueue instance
 **/
TnySendQueue*
tny_generic_send_queue_new (TnyTransportAccount *account, TnyFolder *outbox, TnyFolder *sentbox)
{
	TnyGenericSendQueue *self = g_object_new (TNY_TYPE_GENERIC_SEND_QUEUE, NULL);
	TnyGenericSendQueuePriv *priv = TNY_GENERIC_SEND_QUEUE_GET_PRIVATE (self);

	priv->account = TNY_TRANSPORT_ACCOUNT (g_object_ref (account));
	priv->outbox = TNY_FOLDER (g_object_ref (outbox));
	priv->sentbox = TNY_FOLDER (g_object_ref (sentbox));

	return TNY_SEND_QUEUE (self);
}


static void
tny_generic_send_queue_finalize (GObject *object)
{
	TnyGenericSendQueuePriv *priv = TNY_GENERIC_SEND_QUEUE_GET_PRIVATE (object);

	priv->cancelled = TRUE;
	o_async_worker_join (priv->queue);

	g_mutex_lock (priv->lock);
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
	priv->cancelled = FALSE;
	g_mutex_unlock (priv->lock);

	return;
}

static void
tny_generic_send_queue_class_init (TnyGenericSendQueueClass *klass)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;

	klass->add_func = tny_generic_send_queue_add_default;
	klass->get_sentbox_func = tny_generic_send_queue_get_sentbox_default;
	klass->get_outbox_func = tny_generic_send_queue_get_outbox_default;
	klass->cancel_func = tny_generic_send_queue_cancel_default;

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
tny_generic_send_queue_get_type (void)
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

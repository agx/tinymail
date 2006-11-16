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

#include <config.h>

#include <glib/gi18n-lib.h>

#include <tny-camel-send-queue.h>
#include <tny-camel-shared.h>
#include <tny-camel-msg.h>

static GObjectClass *parent_class = NULL;

#include "tny-camel-send-queue-priv.h"

#define TNY_CAMEL_SEND_QUEUE_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_CAMEL_SEND_QUEUE, TnyCamelSendQueuePriv))

static void
tny_camel_send_queue_add (TnySendQueue *self, TnyMsg *msg)
{
	TNY_CAMEL_SEND_QUEUE_GET_CLASS (self)->add_func (self, msg);
}

static gpointer
thread_main (gpointer data)
{
	TnySendQueue *self = (TnySendQueue *) data;
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);
	TnyMsg *current;
	guint i = 0;

	while (priv->todo)
	{
		g_mutex_lock (priv->todo_lock);
		current = priv->todo->data;
		g_mutex_unlock (priv->todo_lock);

		tny_transport_account_send (TNY_TRANSPORT_ACCOUNT (priv->trans_account), current);			
		g_signal_emit (self, tny_send_queue_signals [TNY_SEND_QUEUE_MSG_SENT], 3, current, i, priv->total);
		i++;
		g_object_unref (G_OBJECT (current));

		g_mutex_lock (priv->todo_lock);
		priv->todo = g_list_delete_link (priv->todo, priv->todo);
		g_mutex_unlock (priv->todo_lock);
	}

	g_thread_exit (NULL);
	return NULL;
}

static void
tny_camel_send_queue_add_default (TnySendQueue *self, TnyMsg *msg)
{
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);
	gboolean launch_thread = FALSE;

	g_assert (TNY_IS_CAMEL_MSG (msg));
	
	g_mutex_lock (priv->todo_lock);

	priv->total = g_list_length (priv->todo);
	launch_thread = (priv->total == 0);
	priv->total++;
	priv->todo = g_list_prepend (priv->todo, g_object_ref (G_OBJECT (msg)));

	if (launch_thread)
		priv->thread = g_thread_create (thread_main, self, TRUE, NULL);

	g_mutex_unlock (priv->todo_lock);
								

	return;
}


static void
tny_camel_send_queue_finalize (GObject *object)
{
	TnyCamelSendQueue *self = (TnyCamelSendQueue*) object;
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);

	g_mutex_lock (priv->todo_lock);
	
	g_list_foreach (priv->todo, (GFunc) g_object_unref, NULL);
	g_list_free (priv->todo);
	priv->todo = NULL;
	
	g_mutex_unlock (priv->todo_lock);
	g_mutex_free (priv->todo_lock);

	g_object_unref (G_OBJECT (priv->trans_account));
	
	(*parent_class->finalize) (object);

	return;
}

/**
 * tny_camel_send_queue_new:
 * @trans_account: A #TnyCamelTransportAccount instance
 *
 *
 * Return value: A new #TnySendQueue instance implemented for Camel
 **/
TnySendQueue*
tny_camel_send_queue_new (TnyTransportAccount *trans_account)
{
	TnyCamelSendQueue *self = g_object_new (TNY_TYPE_CAMEL_SEND_QUEUE, NULL);
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);

	g_assert (TNY_IS_CAMEL_TRANSPORT_ACCOUNT (trans_account));
	
	priv->trans_account = TNY_CAMEL_TRANSPORT_ACCOUNT 
			(g_object_ref (G_OBJECT (trans_account)));

	return TNY_SEND_QUEUE (self);
}


static void
tny_send_queue_init (gpointer g, gpointer iface_data)
{
	TnySendQueueIface *klass = (TnySendQueueIface *)g;

	klass->add_func = tny_camel_send_queue_add;

	return;
}

static void 
tny_camel_send_queue_class_init (TnyCamelSendQueueClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	class->add_func = tny_camel_send_queue_add_default;

	object_class->finalize = tny_camel_send_queue_finalize;

	g_type_class_add_private (object_class, sizeof (TnyCamelSendQueuePriv));

	return;
}


static void
tny_camel_send_queue_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyCamelSendQueue *self = (TnyCamelSendQueue*)instance;
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);

	priv->todo = NULL;
	priv->todo_lock = g_mutex_new ();

	return;
}

GType 
tny_camel_send_queue_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY (!camel_type_init_done))
	{
		if (!g_thread_supported ()) 
			g_thread_init (NULL);

		camel_type_init ();
		camel_type_init_done = TRUE;
	}

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyCamelSendQueueClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_camel_send_queue_class_init, /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyCamelSendQueue),
		  0,      /* n_preallocs */
		  tny_camel_send_queue_instance_init,    /* instance_init */
		  NULL
		};

		static const GInterfaceInfo tny_send_queue_info = 
		{
		  (GInterfaceInitFunc) tny_send_queue_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyCamelSendQueue",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_SEND_QUEUE,
			&tny_send_queue_info);

	}

	return type;
}


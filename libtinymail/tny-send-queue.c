/* libtinymail - The Tiny Mail base library
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

#include <tny-send-queue.h>
#include <tny-folder.h>
#include <tny-msg.h>

guint tny_send_queue_signals [TNY_SEND_QUEUE_LAST_SIGNAL];


/**
 * tny_send_queue_cancel:
 * @self: A #TnySendQueue instance
 * @remove: Whether or not to also remove queued messages
 * @err: a #GError instance or NULL
 *
 * Cancels the current operation
 **/
void 
tny_send_queue_cancel (TnySendQueue *self, gboolean remove, GError **err)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_SEND_QUEUE (self));
	g_assert (TNY_SEND_QUEUE_GET_IFACE (self)->cancel_func != NULL);
#endif

	TNY_SEND_QUEUE_GET_IFACE (self)->cancel_func (self, remove, err);
	return;
}

/**
 * tny_send_queue_get_sentbox:
 * @self: A #TnySendQueue instance
 *
 * Get the folder which contains the messages that have been sent. The 
 * return value must be unreferenced after use
 *
 * Return value: a #TnyFolder instance
 **/
TnyFolder* 
tny_send_queue_get_sentbox (TnySendQueue *self)
{
	TnyFolder *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_SEND_QUEUE (self));
	g_assert (TNY_SEND_QUEUE_GET_IFACE (self)->get_sentbox_func != NULL);
#endif

	retval = TNY_SEND_QUEUE_GET_IFACE (self)->get_sentbox_func (self);

#ifdef DBC /* ensure */
	g_assert (TNY_IS_FOLDER (retval));
#endif

	return retval;
}

/**
 * tny_send_queue_get_outbox:
 * @self: A #TnySendQueue instance
 *
 * Get the folder which contains the messages that have not yet been sent. The 
 * return value must be unreferenced after use
 *
 * Return value: a #TnyFolder instance
 **/
TnyFolder* 
tny_send_queue_get_outbox (TnySendQueue *self)
{
	TnyFolder *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_SEND_QUEUE (self));
	g_assert (TNY_SEND_QUEUE_GET_IFACE (self)->get_outbox_func != NULL);
#endif

	retval = TNY_SEND_QUEUE_GET_IFACE (self)->get_outbox_func (self);

#ifdef DBC /* ensure */
	g_assert (TNY_IS_FOLDER (retval));
#endif

	return retval;
}

/**
 * tny_send_queue_add:
 * @self: A #TnySendQueue instance
 * @msg: a #TnyMsg instance
 * @err: a #GError instance or NULL
 *
 * Add a message to the send queue.
 *
 **/
void 
tny_send_queue_add (TnySendQueue *self, TnyMsg *msg, GError **err)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_SEND_QUEUE (self));
	g_assert (msg);
	g_assert (TNY_IS_MSG (msg));
	g_assert (TNY_SEND_QUEUE_GET_IFACE (self)->add_func != NULL);
#endif

	TNY_SEND_QUEUE_GET_IFACE (self)->add_func (self, msg, err);
	return;
}




static void
tny_send_queue_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) 
	{
/**
 * TnySendQueue::msg-sent
 * @self: the object on which the signal is emitted
 * @arg1: The message that got sent
 * @arg2: The current nth number of the message that got sent
 * @arg3: The total amount of messages currently being processed
 *
 * API WARNING: This API might change
 *
 * Emitted when a message got sent
 **/
		tny_send_queue_signals[TNY_SEND_QUEUE_MSG_SENT] =
		   g_signal_new ("msg_sent",
			TNY_TYPE_SEND_QUEUE,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnySendQueueIface, msg_sent),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE, 3, TNY_TYPE_MSG, G_TYPE_UINT, G_TYPE_UINT);

/**
 * TnySendQueue::error-happened
 * @self: the object on which the signal is emitted
 * @arg1: The message that was supposed to be sent or NULL
 * @arg2: a GError containing the error that happened
 * @arg3: The current nth number of the message that was supposed to be sent
 * @arg4: The total amount of messages currently being processed
 *
 * API WARNING: This API might change
 *
 * Emitted when a message didn't get sent because of an error
 **/
		tny_send_queue_signals[TNY_SEND_QUEUE_ERROR_HAPPENED] =
		   g_signal_new ("error_happened",
			TNY_TYPE_SEND_QUEUE,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnySendQueueIface, error_happened),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE, 3, TNY_TYPE_MSG, G_TYPE_POINTER, G_TYPE_UINT, G_TYPE_UINT);

		initialized = TRUE;
	}
}

GType
tny_send_queue_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnySendQueueIface),
		  tny_send_queue_base_init,   /* base_init */
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
			"TnySendQueue", &info, 0);
		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);

	}

	return type;
}


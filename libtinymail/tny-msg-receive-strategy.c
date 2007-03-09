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

#include <tny-msg-receive-strategy.h>

/* Possible future API changes:
 * tny_msg_receive_strategy_perform_get_msg will get a status callback handler.
 * Also take a look at the possible API changes for TnyFolder's get_msg_async 
 * as this would affect that API too. */

/**
 * tny_msg_receive_strategy_perform_get_msg:
 * @self: A #TnyMsgReceiveStrategy instance
 * @folder: The #TnyFolder instance from which the message will be received
 * @header: The #TnyHeader instance of the message that must be received
 * @err: A #GError instance or NULL
 *
 * Performs the receiving of a message from a folder
 *
 * Return value: the received message
 **/
TnyMsg *
tny_msg_receive_strategy_perform_get_msg (TnyMsgReceiveStrategy *self, TnyFolder *folder, TnyHeader *header, GError **err)
{
#ifdef DEBUG
	if (!TNY_MSG_RECEIVE_STRATEGY_GET_IFACE (self)->perform_get_msg_func)
		g_critical ("You must implement tny_msg_receive_strategy_get_msg\n");
#endif

	return TNY_MSG_RECEIVE_STRATEGY_GET_IFACE (self)->perform_get_msg_func (self, folder, header, err);
}

static void
tny_msg_receive_strategy_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) 
		initialized = TRUE;
}

GType
tny_msg_receive_strategy_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgReceiveStrategyIface),
		  tny_msg_receive_strategy_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyMsgReceiveStrategy", &info, 0);
	}

	return type;
}

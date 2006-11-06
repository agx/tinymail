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

#include <tny-msg-remove-strategy.h>

/**
 * tny_msg_remove_strategy_remove:
 * @self: A #TnyMsgRemoveStrategy instance
 * @folder: The #TnyFolder instance from which the message will be removed
 * @header: The #TnyHeader instance of the message that must be removed
 *
 * Performs the removal of a message from a folder
 **/
void
tny_msg_remove_strategy_remove (TnyMsgRemoveStrategy *self, TnyFolder *folder, TnyHeader *header)
{
#ifdef DEBUG
	if (!TNY_MSG_REMOVE_STRATEGY_GET_IFACE (self)->remove_func)
		g_critical ("You must implement tny_msg_remove_strategy_remove\n");
#endif

	TNY_MSG_REMOVE_STRATEGY_GET_IFACE (self)->remove_func (self, folder, header);
	return;
}

static void
tny_msg_remove_strategy_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) 
		initialized = TRUE;
}

GType
tny_msg_remove_strategy_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgRemoveStrategyIface),
		  tny_msg_remove_strategy_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyMsgRemoveStrategy", &info, 0);
	}

	return type;
}

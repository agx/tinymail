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
 * tny_msg_remove_strategy_peform_remove:
 * @self: A #TnyMsgRemoveStrategy instance
 * @folder: The #TnyFolder instance from which the message will be removed
 * @header: The #TnyHeader instance of the message that must be removed
 * @err: A #GError instance or NULL
 *
 * Performs the removal of a message from a folder
 *
 * This doesn't remove it from a #TnyList that holds the headers (for example 
 * for a header summary view) if the tny_folder_get_headers method happened 
 * before the deletion. You are responsible for refreshing your own lists.
 *
 * This method also doesn't "have" to remove the header from the folder. 
 * Depending on the implementation it might only marks it as removed (it for 
 * example sets the TNY_HEADER_FLAG_DELETED). In such a case only after
 * performing the tny_folder_sync method on the folder, it will really be
 * removed.
 *
 * In such a case this means that a tny_folder_get_headers method call will
 * still prepend the removed message to the list. It will do this until the
 * expunge  happened. You are advised to hide messages that have been marked
 * as being deleted from your summary view. In Gtk+, for the #GtkTreeView
 * component, you can do this using the #GtkTreeModelFilter tree model filtering
 * model.
 *
 * The #TnyCamelMsgRemoveStrategy implementation works this way. This 
 * implementation is also the default implementation for most #TnyFolder 
 * implementations in libtinymail-camel
 * 
 * Note that it's possible that another implementation works differently. You
 * could, for example, inherit or decorate the #TnyCamelMsgRemoveStrategy 
 * implementation by adding code that also permanently removes the message
 * in your inherited special type.
 *
 **/
void
tny_msg_remove_strategy_perform_remove (TnyMsgRemoveStrategy *self, TnyFolder *folder, TnyHeader *header, GError **err)
{
#ifdef DEBUG
	if (!TNY_MSG_REMOVE_STRATEGY_GET_IFACE (self)->perform_remove_func)
		g_critical ("You must implement tny_msg_remove_strategy_remove\n");
#endif

	TNY_MSG_REMOVE_STRATEGY_GET_IFACE (self)->perform_remove_func (self, folder, header, err);
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

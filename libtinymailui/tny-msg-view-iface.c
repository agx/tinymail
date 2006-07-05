/* libtinymailui - The Tiny Mail UI library
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

#include <tny-msg-view-iface.h>



/**
 * tny_msg_view_iface_set_unavailable:
 * @self: A #TnyMsgViewIface instance
 * @header: a #TnyMsgHeaderIface instance or NULL
 *
 * Set the view to display that the message was unavailable
 * 
 **/
void
tny_msg_view_iface_set_unavailable (TnyMsgViewIface *self, TnyMsgHeaderIface *header)
{
#ifdef DEBUG
	if (!TNY_MSG_VIEW_IFACE_GET_CLASS (self)->set_unavailable_func)
		g_critical ("You must implement tny_msg_view_iface_set_unavailable\n");
#endif

	TNY_MSG_VIEW_IFACE_GET_CLASS (self)->set_unavailable_func (self, header);
	return;
}


/**
 * tny_msg_view_iface_set_save_strategy:
 * @self: A #TnyMsgViewIface instance
 * @strategy: A TnySaveStrategyIface instace
 *
 * Set the strategy used for saving mime-parts
 * 
 **/
void
tny_msg_view_iface_set_save_strategy (TnyMsgViewIface *self, TnySaveStrategyIface *strategy)
{
#ifdef DEBUG
	if (!TNY_MSG_VIEW_IFACE_GET_CLASS (self)->set_save_strategy_func)
		g_critical ("You must implement tny_msg_view_iface_set_save_strategy\n");
#endif

	TNY_MSG_VIEW_IFACE_GET_CLASS (self)->set_save_strategy_func (self, strategy);
	return;
}




/**
 * tny_msg_view_iface_set_msg:
 * @self: A #TnyMsgViewIface instance
 * @msg: A #TnyMsgIface instace
 *
 * Set message to view
 * 
 **/
void
tny_msg_view_iface_set_msg (TnyMsgViewIface *self, TnyMsgIface *msg)
{
#ifdef DEBUG
	if (!TNY_MSG_VIEW_IFACE_GET_CLASS (self)->set_msg_func)
		g_critical ("You must implement tny_msg_view_iface_set_msg\n");
#endif

	TNY_MSG_VIEW_IFACE_GET_CLASS (self)->set_msg_func (self, msg);
	return;
}

static void
tny_msg_view_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_msg_view_iface_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgViewIfaceClass),
		  tny_msg_view_iface_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyMsgViewIface", &info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}

	return type;
}



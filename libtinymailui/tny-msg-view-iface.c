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
 * tny_msg_view_iface_clear:
 * @self: A #TnyMsgViewIface instance
 *
 * Clear the view @self (show nothing)
 * 
 * Implementors: this method should clear @self (display nothing)
 *
 **/
void
tny_msg_view_iface_clear (TnyMsgViewIface *self)
{
#ifdef DEBUG
	if (!TNY_MSG_VIEW_IFACE_GET_CLASS (self)->clear_func)
		g_critical ("You must implement tny_msg_view_iface_clear\n");
#endif

	TNY_MSG_VIEW_IFACE_GET_CLASS (self)->clear_func (self);
	return;
}


/**
 * tny_msg_view_iface_set_unavailable:
 * @self: A #TnyMsgViewIface instance
 *
 * Set the view @self to display that the message was unavailable
 * 
 * Implementors: this method should set @self to display a message like
 * "Message unavailable" or any other indication that a specific message
 * isn't available.
 *
 **/
void
tny_msg_view_iface_set_unavailable (TnyMsgViewIface *self)
{
#ifdef DEBUG
	if (!TNY_MSG_VIEW_IFACE_GET_CLASS (self)->set_unavailable_func)
		g_critical ("You must implement tny_msg_view_iface_set_unavailable\n");
#endif

	TNY_MSG_VIEW_IFACE_GET_CLASS (self)->set_unavailable_func (self);
	return;
}


/**
 * tny_msg_view_iface_set_save_strategy:
 * @self: A #TnyMsgViewIface instance
 * @strategy: A TnySaveStrategyIface instace
 *
 * Set the strategy used for saving mime-parts
 * 
 * Implementors: This method should set the strategy for saving a mime-part.
 * The user interface of the view can for example have a popup menu in its
 * attachment viewer that will have to use this strategy for saving the
 * mime-part.
 *
 * The idea is that devices can have a specific such strategy. For example a
 * strategy that sends it to another computer or a strategy that saves it to
 * a flash disk. However. In the message view component, you don't care about
 * that. You only care about the API of the save-strategy interface.
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
 * Set message of the view @self
 * 
 * Implementors: this method should cause the view @self to show the message
 * @msg to the user. This includes showing the header (for which you can
 * make a composition with a #TnyHeaderViewIface), the message body and the
 * attachments.
 *
 * You can get a list of mime-parts using the tny_msg_iface_get_parts API of
 * the #TnyMsgIface type. If the mime-part has as content type a type that your
 * viewer supports and recognises as an E-mail body (you can check the content
 * type if a mime-part using tny_mime_part_iface_content_type_is), the view
 * should show it.
 *
 * It's advised to use the #TnyStreamIface API for streaming the decoded content
 * of the mime-part to the model of your view that will display the body.
 * Examples are #TnyMsgView in libtinymailui-gtk and #TnyMozEmbedMsgView in
 * libtinymailui-mozembed. If you don't decode the content of the mime-part,
 * for example using tny_mime_part_iface_decode_to_stream, the content might be
 * encoded in a format suitable for data transfer over for example SMTP.
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



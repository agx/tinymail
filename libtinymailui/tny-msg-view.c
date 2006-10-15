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

#include <tny-msg-view.h>

/**
 * tny_msg_view_create_mime_part_view_for:
 * @self: A #TnyMsgView instance
 * @part: A #TnyMimePart instance
 *
 * Create a #TnyMimePartView instance for viewing @part
 *
 * Return value: A #TnyMimePartView instance for viewing @part
 **/
TnyMimePartView*
tny_msg_view_create_mime_part_view_for (TnyMsgView *self, TnyMimePart *part)
{
#ifdef DEBUG
	if (!TNY_MSG_VIEW_GET_IFACE (self)->create_mime_part_view_for_func)
		g_critical ("You must implement tny_msg_view_create_mime_part_view_for\n");
#endif

	return TNY_MSG_VIEW_GET_IFACE (self)->create_mime_part_view_for_func (self, part);
}

/**
 * tny_msg_view_clear:
 * @self: A #TnyMsgView instance
 *
 * Clear the view @self (show nothing)
 * 
 * Implementors: this method should clear @self (display nothing, or display
 * a picture with flowers and nude people if that is how your E-mail client
 * indicates that there's no message loaded)
 *
 **/
void
tny_msg_view_clear (TnyMsgView *self)
{
#ifdef DEBUG
	if (!TNY_MSG_VIEW_GET_IFACE (self)->clear_func)
		g_critical ("You must implement tny_msg_view_clear\n");
#endif

	TNY_MSG_VIEW_GET_IFACE (self)->clear_func (self);
	return;
}


/**
 * tny_msg_view_set_unavailable:
 * @self: A #TnyMsgView instance
 *
 * Set the view @self to display that the message was unavailable
 * 
 * Implementors: this method should set @self to display a message like
 * "Message unavailable" or any other indication that a specific message
 * isn't available.
 *
 **/
void
tny_msg_view_set_unavailable (TnyMsgView *self)
{
#ifdef DEBUG
	if (!TNY_MSG_VIEW_GET_IFACE (self)->set_unavailable_func)
		g_critical ("You must implement tny_msg_view_set_unavailable\n");
#endif

	TNY_MSG_VIEW_GET_IFACE (self)->set_unavailable_func (self);
	return;
}


/**
 * tny_msg_view_get_msg:
 * @self: A #TnyMsgView instance
 *
 * Get message of the view @self The return value must be unreferenced after
 * use.
 * 
 * Implementors: this method should return the message this view is currently
 * displaying. It should add a reference to the instance before returning. If
 * the view isn't viewing any message, it must return NULL.
 *
 * Return value: A #TnyMsg instance or NULL
 **/
TnyMsg* 
tny_msg_view_get_msg (TnyMsgView *self)
{
#ifdef DEBUG
	if (!TNY_MSG_VIEW_GET_IFACE (self)->get_msg_func)
		g_critical ("You must implement tny_msg_view_get_msg\n");
#endif

	return TNY_MSG_VIEW_GET_IFACE (self)->get_msg_func (self);
}

/**
 * tny_msg_view_set_msg:
 * @self: A #TnyMsgView instance
 * @msg: A #TnyMsg instace
 *
 * Set message of the view @self
 * 
 * Implementors: this method should cause the view @self to show the message
 * @msg to the user. This includes showing the header (for which you can
 * make a composition with a #TnyHeaderView), the message body and the
 * attachments.
 *
 * You can get a list of mime-parts using the tny_msg_get_parts API of
 * the #TnyMsg type. If the mime-part has as content type a type that your
 * viewer supports and recognises as an E-mail body (you can check the content
 * type if a mime-part using tny_mime_part_content_type_is), the view
 * should show it.
 *
 * It's advised to use the #TnyStream API for streaming the decoded content
 * of the mime-part to the model of your view that will display the body.
 * Examples are #TnyMsgView in libtinymailui-gtk and #TnyMozEmbedMsgView in
 * libtinymailui-mozembed. If you don't decode the content of the mime-part,
 * for example using tny_mime_part_decode_to_stream, the content might be
 * encoded in a format suitable for data transfer over for example SMTP.
 *
 **/
void
tny_msg_view_set_msg (TnyMsgView *self, TnyMsg *msg)
{
#ifdef DEBUG
	if (!TNY_MSG_VIEW_GET_IFACE (self)->set_msg_func)
		g_critical ("You must implement tny_msg_view_set_msg\n");
#endif

	TNY_MSG_VIEW_GET_IFACE (self)->set_msg_func (self, msg);
	return;
}

static void
tny_msg_view_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_msg_view_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgViewIface),
		  tny_msg_view_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyMsgView", &info, 0);
	}

	return type;
}



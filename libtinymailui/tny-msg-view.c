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
 * tny_msg_view_create_new_inline_viewer:
 * @self: A #TnyMsgView instance
 *
 * Create a new #TnyMsgView that can be used to display an inline message. 
 * Usually it will return a new instance of the same type as @self. The
 * returned instance must be unreferenced after use.
 *
 * Implementors: This method should create and return a new #TnyMsgView instance 
 * usually of the same type as @self. This method will be used when a
 * #TnyMsgView needs to create a #TnyMsgView instance for displaying inlined 
 * messages (like what message/rfc822 mime parts are). For example the 
 * #TnyGtkMsgView implementation will use this method to create for itself a 
 * new #TnyMsgView instance that it can embed in itself.
 *
 * Example:
 * <informalexample><programlisting>
 * static TnyMsgView*
 * tny_my_html_msg_view_create_new_inline_viewer (TnyMsgView *self)
 * {
 *    return tny_my_html_msg_view_new ();
 * }
 * </programlisting></informalexample>
 *
 * Note that if you want to pass contructor parameters, that you will have to
 * store them yourself (for example in a static global field in the .c file) and
 * repeat them in the new instance that will be created by this method.
 *
 * Return value: A #TnyMsgView instance
 **/
TnyMsgView* 
tny_msg_view_create_new_inline_viewer (TnyMsgView *self)
{
#ifdef DEBUG
	if (!TNY_MSG_VIEW_GET_IFACE (self)->create_new_inline_viewer_func)
		g_critical ("You must implement tny_msg_view_create_new_inline_viewer\n");
#endif

	return TNY_MSG_VIEW_GET_IFACE (self)->create_new_inline_viewer_func (self);
}

/**
 * tny_msg_view_create_mime_part_view_for:
 * @self: A #TnyMsgView instance
 * @part: A #TnyMimePart instance
 *
 * Create a #TnyMimePartView instance for viewing @part. The returned instance
 * must be unreferenced after use.
 *
 * Implementors: This method should create and return a new #TnyMimePartView 
 * that is suitable for displaying @part.
 *
 * Example:
 * <informalexample><programlisting>
 * static TnyMimePartView*
 * tny_my_html_msg_view_create_mime_part_view_for (TnyMsgView *self, TnyMimePart *part)
 * {
 *    TnyMimePartView *retval = NULL;
 *    g_assert (TNY_IS_MIME_PART (part));
 *    if (tny_mime_part_content_type_is (part, "text/html"))
 *    {
 *        GtkWidget *widget = (GtkWidget *) self;
 *        retval = tny_my_html_mime_part_view_new ();
 *    } else
 *        retval = TNY_GTK_MSG_VIEW_CLASS (parent_class)->create_mime_part_view_for_func (self, part);
 *    return retval;
 * }
 * </programlisting></informalexample>
 *
 * ps. For a real and complete working example take a look at the implementation of 
 * #TnyMozEmbedMsgView in libtinymailui-mozembed.
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
 * Clear @self (show nothing)
 * 
 * Implementors: this method should clear @self (display nothing and clearup)
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
 * Set @self to display that a message was unavailable
 * 
 * Implementors: this method should set @self to display a message like
 * "Message unavailable" or trigger another indication that a specific message
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
 * Get the current message of @self. If @self is not displaying any message,
 * NULL will be returned. Else the return value must be unreferenced after use.
 *
 * Implementors: this method should return the mime part this view is currently
 * viewing. It must add a reference to the instance before returning it. If the
 * view isn't viewing any mime part, it must return NULL.
 *
 * Usually this method is an alias for tny_mime_part_view_get_part of 
 * #TnyMimePartView
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
 * Set the message which view @self must display.
 * 
 * Implementors: this method should cause @self to show @msg to the user. 
 * This includes showing the header (for which you can make a composition with 
 * a #TnyHeaderView), the message body and the attachments (for which you 
 * typically use the #TnyMimePartView interface and implementations).
 *
 * You can get a list of mime parts using the tny_mime_part_get_parts API of
 * the #TnyMimePart type. You can use the tny_msg_view_create_mime_part_view_for
 * API to get an instance of a #TnyMimePartView that can view the mime part.
 * 
 * Usually this method is an alias or decorator for tny_mime_part_view_set_part
 * of #TnyMimePartView
 *
 * Example:
 * <informalexample><programlisting>
 * static void 
 * tny_my_msg_view_set_msg (TnyMsgView *self, TnyMsg *msg)
 * {
 *     TnyIterator *iterator;
 *     TnyList *list = tny_simple_list_new ();
 *     tny_msg_view_clear (self);
 *     header = tny_msg_get_header (msg);
 *     tny_header_view_set_header (priv->headerview, header);
 *     g_object_unref (G_OBJECT (header));
 *     tny_mime_part_view_set_part (TNY_MIME_PART_VIEW (self),
 *                TNY_MIME_PART (msg));
 *     tny_mime_part_get_parts (TNY_MIME_PART (msg), list);
 *     iterator = tny_list_create_iterator (list);
 *     while (!tny_iterator_is_done (iterator))
 *     {
 *         TnyMimePart *part = tny_iterator_get_current (iterator);
 *         TnyMimePartView *mpview;
 *         mpview = tny_msg_view_create_mime_part_view_for (self, part);
 *         if (mpview)
 *             tny_mime_part_view_set_part (mpview, part);
 *         g_object_unref (G_OBJECT(part));
 *         tny_iterator_next (iterator);
 *     }
 *     g_object_unref (G_OBJECT (iterator));
 *     g_object_unref (G_OBJECT (list));
 * }
 * </programlisting></informalexample>
 *
 * ps. For a real and complete working example take a look at the implementation of 
 * #TnyGtkMsgView in libtinymailui-gtk.
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

		g_type_interface_add_prerequisite (type, TNY_TYPE_MIME_PART_VIEW);

	}

	return type;
}



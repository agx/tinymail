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
 * Create a #TnyMimePartView instance for viewing @part. The returned instance
 * must be unreferenced after use.
 *
 * Implementors: This method should create and return a new #TnyMimePartView 
 * that is suitable for displaying the #TnyMimePart @part.
 *
 * Example:
 * <informalexample><programlisting>
 * static TnyMimePartView*
 * tny_gtk_msg_view_create_mime_part_view_for (TnyMsgView *self, TnyMimePart *part)
 * {
 *     TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);
 *     TnyMimePartView *retval = NULL;
 *     if (tny_mime_part_content_type_is (part, "text/*"))
 *     {
 *         retval = tny_gtk_text_mime_part_view_new ();
 *         gtk_box_pack_start (GTK_BOX (TNY_GTK_MSG_VIEW (self)->viewers), 
 *                GTK_WIDGET (retval), TRUE, TRUE, 0);
 *         gtk_widget_show (GTK_WIDGET (retval));
 *    } else if (tny_mime_part_get_content_type (part) &&
 *           tny_mime_part_is_attachment (part))
 *    {
 *         static gboolean first = TRUE;
 *         GtkTreeModel *model;
 *         gtk_widget_show (priv->attachview_sw);
 *         if (first)
 *         {
 *              model = tny_gtk_attach_list_model_new ();
 *              gtk_icon_view_set_model (priv->attachview, model);
 *              first = FALSE;
 *         } else
 *             model = gtk_icon_view_get_model (priv->attachview);
 *             retval = tny_gtk_attachment_mime_part_view_new (TNY_GTK_ATTACH_LIST_MODEL (model));
 *         }
 *         return retval;
 * }
 * </programlisting></informalexample>
 *
 * This example is the implementation of #TnyGtkMsgView. If you are planning to
 * use libtinymailui-gtk, which contains this type, it's also possible to inherit
 * from this type. Take for example another type in libtinymailui-mozembed that
 * implements one that adds support for a text/html mime part.
 *
 * Example:
 * <informalexample><programlisting>
 * static TnyMimePartView*
 * tny_moz_embed_msg_view_create_mime_part_view_for (TnyMsgView *self, TnyMimePart *part)
 * {
 *     TnyMimePartView *retval = NULL;
 *     if (tny_mime_part_content_type_is (part, "text/html"))
 *     {
 *         retval = tny_moz_embed_html_mime_part_view_new ();
 *         gtk_box_pack_start (GTK_BOX (TNY_GTK_MSG_VIEW (self)->viewers), GTK_WIDGET (retval), TRUE, TRUE, 0);
 *         gtk_widget_show (GTK_WIDGET (retval));
 *     }
 *     if (!retval)
 *         retval = TNY_GTK_MSG_VIEW_CLASS (parent_class)->create_mime_part_view_for_func (self, part);
 *     return retval;
 * }
 * static void 
 * tny_moz_embed_msg_view_class_init (TnyMozEmbedMsgViewClass *class)
 * {
 *      GObjectClass *object_class;
 *      parent_class = g_type_class_peek_parent (class);
 *      object_class = (GObjectClass*) class;
 *      object_class->finalize = tny_moz_embed_msg_view_finalize;
 *      TNY_GTK_MSG_VIEW_CLASS (class)->create_mime_part_view_for_func = tny_moz_embed_msg_view_create_mime_part_view_for;
 * }
 * </programlisting></informalexample>
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
 * Get the current message of the view @self. The returned instance must be 
 * unreferenced after use.
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
 * Set the message which view @self must display.
 * 
 * Implementors: this method should cause the view @self to show the message
 * @msg to the user. This includes showing the header (for which you can
 * make a composition with a #TnyHeaderView), the message body and the
 * attachments (for which you typically use the #TnyMimePartView interface and
 * implementations).
 *
 * You can get a list of mime-parts using the tny_msg_get_parts API of
 * the #TnyMsg type. You can use the tny_msg_view_create_mime_part_view_for
 * API to get an instance of a #TnyMimePartView that can view the mime part.
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
 *     tny_msg_get_parts (msg, list);
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



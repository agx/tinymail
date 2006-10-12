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

#include <tny-mime-part-view.h>



/**
 * tny_mime_part_view_set_save_strategy:
 * @self: A #TnyMimePartView instance
 * @strategy: A TnySaveStrategy instace
 *
 * Set the strategy used for saving mime-parts
 * 
 * Implementors: This method should set the strategy for saving a mime-part.
 * The user interface of the view can for example have a popup menu in its
 * attachment viewer that will have to use this strategy for saving the
 * mime-part.
 *
 * Example:
 * <informalexample><programlisting>
 * static void 
 * tny_my_mime_part_view_set_save_strategy (TnyMimePartView *self_i, TnySaveStrategy *strat)
 * {
 *      TnyMyMimePartView *self = TNY_MY_MIME_PART_VIEW (self_i);
 *      if (self->save_strategy)
 *            g_object_unref (G_OBJECT (self->save_strategy));
 *      self->save_strategy = g_object_ref (G_OBJECT (strat));
 * }
 * static void
 * tny_my_mime_part_view_finalize (TnyMyMimePartView *self)
 * {
 *      if (self->save_strategy))
 *		g_object_unref (G_OBJECT (self->save_strategy));
 * }
 * </programlisting></informalexample>
 *
 * <informalexample><programlisting>
 * static void 
 * tny_my_mime_part_view_on_save_clicked (TnyMimePartView *self, TnyMimePart *attachment)
 * {
 *     TnyMyMimePartView *self = TNY_MY_MIME_PART_VIEW (self_i);
 *     tny_save_strategy_save (self->save_strategy, attachment);
 * }
 * </programlisting></informalexample>
 *
 * The idea is that devices can have a specific such strategy. For example a
 * strategy that sends it to another computer or a strategy that saves it to
 * a flash disk. However. In the mime part view component, you don't care about
 * that. You only care about the API of the save-strategy interface.
 *
 **/
void 
tny_mime_part_view_set_save_strategy (TnyMimePartView *self, TnySaveStrategy *strategy)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_VIEW_GET_IFACE (self)->set_save_strategy_func)
		g_critical ("You must implement tny_mime_part_view_set_save_strategy\n");
#endif

	TNY_MIME_PART_VIEW_GET_IFACE (self)->set_save_strategy_func (self, strategy);
	return;
}

/**
 * tny_mime_part_view_can_view:
 * @self: A #TnyMimePartView instance
 * @part: a #TnyMimePart instance
 *
 * Figures out whether or not the view supports viewing a mime part 
 *
 * Return value: Whether or not the view supports viewing this mime-part
 **/
gboolean 
tny_mime_part_view_can_view (TnyMimePartView *self, TnyMimePart *part)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_VIEW_GET_IFACE (self)->can_view_func)
		g_critical ("You must implement tny_mime_part_view_is_supported\n");
#endif

	return TNY_MIME_PART_VIEW_GET_IFACE (self)->can_view_func (self, part);
}


/**
 * tny_mime_part_view_clear:
 * @self: A #TnyMimePartView instance
 *
 * Clear the view @self (show nothing)
 *
 * Implementors: this method should clear @self (display nothing, or display
 * a picture with flowers and nude people if that is how your E-mail client
 * indicates that there's no mime_part loaded)
 * 
 **/
void
tny_mime_part_view_clear (TnyMimePartView *self)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_VIEW_GET_IFACE (self)->clear_func)
		g_critical ("You must implement tny_mime_part_view_clear\n");
#endif

	TNY_MIME_PART_VIEW_GET_IFACE (self)->clear_func (self);
	return;    
}


/**
 * tny_mime_part_view_set_mime_part:
 * @self: A #TnyMimePartView instance
 * @mime_part: A #TnyMimePart instace
 *
 * Set mime_part of the view @self
 * 
 * Implementors: this method should cause the view @self to show the mime_part
 * @mime_part to the user. Often this means showing the from, to, subject, date
 * and cc labels.
 *
 * #TnyMimePartView is often used in a composition with the #TnyMsgView
 * type (the #TnyMsgView implementation contains a #TnyMimePartView).
 *
 **/
void
tny_mime_part_view_set_mime_part (TnyMimePartView *self, TnyMimePart *mime_part)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_VIEW_GET_IFACE (self)->set_mime_part_func)
		g_critical ("You must implement tny_mime_part_view_set_mime_part\n");
#endif

	TNY_MIME_PART_VIEW_GET_IFACE (self)->view_mime_part_func (self, mime_part);
	return;
}

static void
tny_mime_part_view_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_mime_part_view_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMimePartViewIface),
		  tny_mime_part_view_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyMimePartView", &info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}

	return type;
}



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

#include <tny-mime-part-saver.h>


/**
 * tny_mime_part_saver_set_save_strategy:
 * @self: A #TnyMimePartSaver instance
 * @strategy: A TnyMimePartSaveStrategy instace
 *
 * Set the strategy used for saving mime-parts
 *
 * Implementors: This method should set (store) the strategy for saving a
 * mime-part.
 *
 * Example:
 * <informalexample><programlisting>
 * static void 
 * tny_my_msg_view_set_save_strategy (TnyMimePartSaver *self_i, TnyMimePartSaveStrategy *strat)
 * {
 *      TnyMyMsgView *self = TNY_MY_MSG_VIEW (self_i);
 *      if (self->save_strategy)
 *            g_object_unref (G_OBJECT (self->save_strategy));
 *      self->save_strategy = g_object_ref (G_OBJECT (strat));
 * }
 * static void
 * tny_my_msg_view_finalize (TnyMyMsgView *self)
 * {
 *      if (self->save_strategy))
 *		g_object_unref (G_OBJECT (self->save_strategy));
 * }
 * </programlisting></informalexample>
 *
 * The idea is that devices can have a specific such strategy. For example a
 * strategy that sends it to another computer or a strategy that saves it to
 * a flash disk. However. In the message view component, you don't care about
 * that. You only care about the API of the save-strategy interface.
 *
 **/
void 
tny_mime_part_saver_set_save_strategy (TnyMimePartSaver *self, TnyMimePartSaveStrategy *strategy)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_SAVER_GET_IFACE (self)->set_save_strategy_func)
		g_critical ("You must implement tny_mime_part_saver_set_save_strategy\n");
#endif

	TNY_MIME_PART_SAVER_GET_IFACE (self)->set_save_strategy_func (self, strategy);
	return;
}



/**
 * tny_mime_part_saver_get_save_strategy:
 * @self: A #TnyMsgView instance
 *
 * Get the strategy used for saving mime-parts. The return value must be
 * unreferenced after use.
 *
 * Example:
 * <informalexample><programlisting>
 * static void 
 * tny_my_msg_view_on_save_clicked (TnyMimePartSaver *self, TnyMimePart *attachment)
 * {
 *     TnyMimePartSaveStrategy *strategy = tny_mime_part_saver_get_save_strategy (self);
 *     tny_save_strategy_save (strategy, attachment);
 *     g_object_unref (G_OBJECT (strategy));
 * }
 * </programlisting></informalexample>
 *
 * Implementors: This method should return the strategy for saving a mime-part.
 * being the implementer, you must add a reference before returning the instance.
 *
 * Example:
 * <informalexample><programlisting>
 * static *TnyMimePartSaveStrategy
 * tny_my_msg_view_get_save_strategy (TnyMimePartSaver *self_i)
 * {
 *      TnyMyMsgView *self = TNY_MY_MSG_VIEW (self_i);
 *      return TNY_MIME_PART_SAVE_STRATEGY (g_object_ref (self->mime_part_save_strategy));
 * }
 * </programlisting></informalexample>
 *
 * Also read about tny_mime_part_saver_set_mime_part_save_strategy
 *
 * Return value: the #TnyMimePartSaveStrategy for @self
 **/
TnyMimePartSaveStrategy*
tny_mime_part_saver_get_save_strategy (TnyMimePartSaver *self)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_SAVER_GET_IFACE (self)->get_save_strategy_func)
		g_critical ("You must implement tny_mime_part_saver_get_save_strategy\n");
#endif

	return TNY_MIME_PART_SAVER_GET_IFACE (self)->get_save_strategy_func (self);
}


/**
 * tny_mime_part_saver_perform_save:
 * @self: A #TnyMimePartSaver instance
 * @mime_part: A #TnyMimePart instace
 *
 * Saves @mime_part using the save strategy
 *
 **/
void
tny_mime_part_saver_perform_save (TnyMimePartSaver *self, TnyMimePart *mime_part)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_SAVER_GET_IFACE (self)->perform_save_func)
		g_critical ("You must implement tny_mime_part_saver_perform_save\n");
#endif

	TNY_MIME_PART_SAVER_GET_IFACE (self)->perform_save_func (self, mime_part);
	return;
}

static void
tny_mime_part_saver_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_mime_part_saver_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMimePartSaverIface),
		  tny_mime_part_saver_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyMimePartSaver", &info, 0);
	}

	return type;
}



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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <config.h>

#include <tny-mime-part-view.h>


/**
 * tny_mime_part_view_clear:
 * @self: A #TnyMimePartView instance
 *
 * Clear @self (show nothing)
 *
 * Implementors: this method should clear @self (display nothing and cleanup)
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
 * tny_mime_part_view_get_part:
 * @self: A #TnyMimePartView instance
 *
 * Get the current mime part of @self. If @self is not displaying any mime part,
 * NULL will be returned. Else the return value must be unreferenced after use.
 * 
 * Implementors: this method should return the mime part this view is currently
 * viewing. It must add a reference to the instance before returning it. If the 
 * view isn't viewing any mime part, it must return NULL.
 *
 * Example:
 * <informalexample><programlisting>
 * static TnyMimePart* 
 * tny_gtk_text_mime_part_view_get_part (TnyMimePartView *self)
 * {
 *      return TNY_MIME_PART (g_object_ref (priv->part));
 * }
 * </programlisting></informalexample>
 *
 * Return value: A #TnyMimePart instance or NULL
 **/
TnyMimePart*
tny_mime_part_view_get_part (TnyMimePartView *self)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_VIEW_GET_IFACE (self)->get_part_func)
		g_critical ("You must implement tny_mime_part_view_get_part\n");
#endif

	return TNY_MIME_PART_VIEW_GET_IFACE (self)->get_part_func (self);
}

/**
 * tny_mime_part_view_set_part:
 * @self: A #TnyMimePartView instance
 * @mime_part: A #TnyMimePart instace
 *
 * Set mime part which @self should display.
 * 
 * Implementors: this method should cause the view @self to show the mime part
 * @mime_part to the user. 
 *
 * Example:
 * <informalexample><programlisting>
 * static void 
 * tny_gtk_text_mime_part_view_set_part (TnyMimePartView *self, TnyMimePart *part)
 * {
 *      if (part)
 *      {
 *           GtkTextBuffer *buffer;
 *           TnyStream *dest;
 *           buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self));
 *           if (buffer &amp;&amp; GTK_IS_TEXT_BUFFER (buffer))
 *                gtk_text_buffer_set_text (buffer, "", 0);
 *           dest = tny_gtk_text_buffer_stream_new (buffer);
 *           tny_stream_reset (dest);
 *           tny_mime_part_decode_to_stream (part, dest);
 *           tny_stream_reset (dest);
 *           g_object_unref (G_OBJECT (dest));
 *           priv->part = TNY_MIME_PART (g_object_ref (part));
 *      }
 * }
 * static void
 * tny_gtk_text_mime_part_view_finalize (TnyGtkTextMimePartView *self)
 * {
 *      if (priv->part))
 *          g_object_unref (priv->part);
 * }
 * </programlisting></informalexample>
 **/
void
tny_mime_part_view_set_part (TnyMimePartView *self, TnyMimePart *mime_part)
{
#ifdef DEBUG
	if (!TNY_MIME_PART_VIEW_GET_IFACE (self)->set_part_func)
		g_critical ("You must implement tny_mime_part_view_set_part\n");
#endif

	TNY_MIME_PART_VIEW_GET_IFACE (self)->set_part_func (self, mime_part);
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
	}

	return type;
}



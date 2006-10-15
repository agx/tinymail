/* libtinymailui-gtk - The Tiny Mail UI library for Gtk+
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
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include <tny-moz-embed-msg-view.h>
#include <tny-moz-embed-html-mime-part-view.h>

static GObjectClass *parent_class = NULL;


/**
 * tny_moz_embed_msg_view_new:
 *
 * Return value: a new #TnyMsgView instance implemented for Gtk+
 **/
TnyMsgView*
tny_moz_embed_msg_view_new (void)
{
	TnyMozEmbedMsgView *self = g_object_new (TNY_TYPE_MOZ_EMBED_MSG_VIEW, NULL);

	return TNY_MSG_VIEW (self);
}

static void
tny_moz_embed_msg_view_instance_init (GTypeInstance *instance, gpointer g_class)
{
	return;
}

static void
tny_moz_embed_msg_view_finalize (GObject *object)
{
	(*parent_class->finalize) (object);

	return;
}

static TnyMimePartView*
tny_moz_embed_msg_view_create_mime_part_view_for (TnyMsgView *self, TnyMimePart *part)
{
	TnyMimePartView *retval = NULL;

	if (tny_mime_part_content_type_is (part, "text/html"))
	{
		retval = tny_moz_embed_html_mime_part_view_new ();
		gtk_box_pack_start (GTK_BOX (TNY_GTK_MSG_VIEW (self)->viewers), GTK_WIDGET (retval), TRUE, TRUE, 0);
		gtk_widget_show (GTK_WIDGET (retval));
	}

	if (!retval)
		retval = TNY_MOZ_EMBED_MSG_VIEW_GET_CLASS (self)->create_mime_part_view_for_orig_func (self, part);
	
	return retval;
}



static void 
tny_moz_embed_msg_view_class_init (TnyMozEmbedMsgViewClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_moz_embed_msg_view_finalize;

	/* Method overloading */
	class->create_mime_part_view_for_orig_func = TNY_GTK_MSG_VIEW_CLASS (class)->create_mime_part_view_for_func;	
	TNY_GTK_MSG_VIEW_CLASS (class)->create_mime_part_view_for_func = tny_moz_embed_msg_view_create_mime_part_view_for;

	return;
}

GType 
tny_moz_embed_msg_view_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMozEmbedMsgViewClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_moz_embed_msg_view_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMozEmbedMsgView),
		  0,      /* n_preallocs */
		  tny_moz_embed_msg_view_instance_init    /* instance_init */
		};

		type = g_type_register_static (TNY_TYPE_GTK_MSG_VIEW,
			"TnyMozEmbedMsgView",
			&info, 0);
	}

	return type;
}

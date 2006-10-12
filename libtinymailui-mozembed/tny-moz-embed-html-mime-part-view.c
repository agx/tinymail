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

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <string.h>
#include <gtk/gtk.h>

#include <tny-moz-embed-html-mime-part-view.h>
#include <tny-moz-embed-stream.h>

#ifdef GNOME
#include <tny-vfs-stream.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#else
#include <tny-fs-stream.h>
#endif


static GObjectClass *parent_class = NULL;

typedef struct _TnyMozEmbedHtmlMimePartViewPriv TnyMozEmbedHtmlMimePartViewPriv;

struct _TnyMozEmbedHtmlMimePartViewPriv
{
	TnyMimePart *part;
	TnySaveStrategy *save_strategy;
};

#define TNY_MOZ_EMBED_HTML_MIME_PART_VIEW_GET_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_MOZ_EMBED_HTML_MIME_PART_VIEW, TnyMozEmbedHtmlMimePartViewPriv))


static void
tny_moz_embed_html_mime_part_view_set_save_strategy (TnyMimePartView *self, TnySaveStrategy *strategy)
{
	TnyMozEmbedHtmlMimePartViewPriv *priv = TNY_MOZ_EMBED_HTML_MIME_PART_VIEW_GET_PRIVATE (self);

	if (priv->save_strategy)
		g_object_unref (G_OBJECT (priv->save_strategy));

	g_object_ref (G_OBJECT (strategy));
	priv->save_strategy = strategy;

	return;
}


static void
tny_moz_embed_html_mime_part_view_save_as_activated (GtkMenuItem *menuitem, gpointer user_data)
{
	TnyMozEmbedHtmlMimePartView *self = user_data;
	TnyMozEmbedHtmlMimePartViewPriv *priv = TNY_MOZ_EMBED_HTML_MIME_PART_VIEW_GET_PRIVATE (self);

	if (!G_LIKELY (priv->save_strategy))
	{
		g_warning (_("No save strategy for this mime part view\n"));
		return;
	}

	tny_save_strategy_save (priv->save_strategy, priv->part);

	return;
}


static gint
tny_moz_embed_html_mime_part_view_popup_handler (GtkWidget *widget, GdkEvent *event)
{	
	g_return_val_if_fail (event != NULL, FALSE);
	
	
	if (G_UNLIKELY (event->type == GDK_BUTTON_PRESS))
	{
		GtkMenu *menu;
		GdkEventButton *event_button;

		menu = GTK_MENU (widget);
		g_return_val_if_fail (widget != NULL, FALSE);
		g_return_val_if_fail (GTK_IS_MENU (widget), FALSE);

		event_button = (GdkEventButton *) event;
		if (G_LIKELY (event_button->button == 3))
		{
			gtk_menu_popup (menu, NULL, NULL, NULL, NULL, 
					  event_button->button, event_button->time);
			return TRUE;
		}
	}
	
	return FALSE;
}

	
static gpointer 
remove_html_stread_hack (gpointer data)
{
	/* Sigh, I don't know why I need this :-(. I think GtkMozEmbed postpones
	   the loading of the document to the very last moment. */

	sleep (5);

	/* This will remove the file in /tmp/ */
	g_object_unref (G_OBJECT (data));

	return NULL;
}

static void 
tny_moz_embed_html_mime_part_view_set_part (TnyMimePartView *self, TnyMimePart *part)
{
	TnyMozEmbedHtmlMimePartViewPriv *priv = TNY_MOZ_EMBED_HTML_MIME_PART_VIEW_GET_PRIVATE (self);

	if (G_LIKELY (priv->part))
		g_object_unref (G_OBJECT (priv->part));
			
    	if (part)
	{
		TnyStream *dest;

		dest = tny_moz_embed_stream_new (GTK_MOZ_EMBED (self));
		tny_stream_reset (dest);
		tny_mime_part_decode_to_stream (part, dest);

		/* This will do: g_object_unref (G_OBJECT (dest)); */
		g_thread_create (remove_html_stread_hack, dest, FALSE, NULL);
			
		g_object_ref (G_OBJECT (part));
		priv->part = part;
		
	}
    
	return;
}

static void
tny_moz_embed_html_mime_part_view_clear (TnyMimePartView *self)
{
	/* Unimplemented. Clear self */
	
    	return;
}

/**
 * tny_moz_embed_html_mime_part_view_new:
 * @save_strategy: The save strategy to use
 *
 * Return value: a new #TnyMimePartView instance implemented for Gtk+
 **/
TnyMimePartView*
tny_moz_embed_html_mime_part_view_new (TnySaveStrategy *save_strategy)
{
	TnyMozEmbedHtmlMimePartView *self = g_object_new (TNY_TYPE_MOZ_EMBED_HTML_MIME_PART_VIEW, NULL);

	tny_mime_part_view_set_save_strategy (TNY_MIME_PART_VIEW (self), save_strategy);

	return TNY_MIME_PART_VIEW (self);
}

static void
tny_moz_embed_html_mime_part_view_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyMozEmbedHtmlMimePartView *self = (TnyMozEmbedHtmlMimePartView *)instance;
	TnyMozEmbedHtmlMimePartViewPriv *priv = TNY_MOZ_EMBED_HTML_MIME_PART_VIEW_GET_PRIVATE (self);
	GtkMenu *menu = GTK_MENU (gtk_menu_new ());
	GtkWidget *mitem = gtk_menu_item_new_with_mnemonic ("Save _As");

	priv->save_strategy = NULL;
	gtk_widget_show (mitem);

	g_signal_connect (G_OBJECT (mitem), "activate", 
		G_CALLBACK (tny_moz_embed_html_mime_part_view_save_as_activated), self);

	gtk_menu_shell_append (GTK_MENU_SHELL (menu), mitem);
			
	g_signal_connect_swapped (G_OBJECT (self), "button_press_event",
		G_CALLBACK (tny_moz_embed_html_mime_part_view_popup_handler), menu);

	return;
}

static void
tny_moz_embed_html_mime_part_view_finalize (GObject *object)
{
	TnyMozEmbedHtmlMimePartView *self = (TnyMozEmbedHtmlMimePartView *)object;	
	TnyMozEmbedHtmlMimePartViewPriv *priv = TNY_MOZ_EMBED_HTML_MIME_PART_VIEW_GET_PRIVATE (self);

	if (G_LIKELY (priv->part))
		g_object_unref (G_OBJECT (priv->part));

	if (G_LIKELY (priv->save_strategy))
		g_object_unref (G_OBJECT (priv->save_strategy));
    
	(*parent_class->finalize) (object);

	return;
}

static void
tny_moz_embed_html_mime_part_view_init (gpointer g, gpointer iface_data)
{
	TnyMimePartViewIface *klass = (TnyMimePartViewIface *)g;

	klass->set_part_func = tny_moz_embed_html_mime_part_view_set_part;
	klass->set_save_strategy_func = tny_moz_embed_html_mime_part_view_set_save_strategy;
	klass->clear_func = tny_moz_embed_html_mime_part_view_clear;
	
	return;
}

static void 
tny_moz_embed_html_mime_part_view_class_init (TnyMozEmbedHtmlMimePartViewClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_moz_embed_html_mime_part_view_finalize;

	g_type_class_add_private (object_class, sizeof (TnyMozEmbedHtmlMimePartViewPriv));

	return;
}

GType 
tny_moz_embed_html_mime_part_view_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMozEmbedHtmlMimePartViewClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_moz_embed_html_mime_part_view_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMozEmbedHtmlMimePartView),
		  0,      /* n_preallocs */
		  tny_moz_embed_html_mime_part_view_instance_init,    /* instance_init */
		  NULL
		};

		static const GInterfaceInfo tny_moz_embed_html_mime_part_view_info = 
		{
		  (GInterfaceInitFunc) tny_moz_embed_html_mime_part_view_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (GTK_TYPE_MOZ_EMBED,
			"TnyMozEmbedHtmlMimePartView",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_MIME_PART_VIEW, 
			&tny_moz_embed_html_mime_part_view_info);

	}

	return type;
}


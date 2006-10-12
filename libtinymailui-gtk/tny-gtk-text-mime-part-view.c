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

#include <tny-gtk-text-mime-part-view.h>
#include <tny-gtk-text-buffer-stream.h>

#ifdef GNOME
#include <tny-vfs-stream.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#else
#include <tny-fs-stream.h>
#endif


static GObjectClass *parent_class = NULL;

typedef struct _TnyGtkTextMimePartViewPriv TnyGtkTextMimePartViewPriv;

struct _TnyGtkTextMimePartViewPriv
{
	TnyMimePart *part;
	TnySaveStrategy *save_strategy;
};

#define TNY_GTK_TEXT_MIME_PART_VIEW_GET_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_GTK_TEXT_MIME_PART_VIEW, TnyGtkTextMimePartViewPriv))

	

static void
tny_gtk_text_mime_part_view_set_save_strategy (TnyMimePartView *self, TnySaveStrategy *strategy)
{
	TnyGtkTextMimePartViewPriv *priv = TNY_GTK_TEXT_MIME_PART_VIEW_GET_PRIVATE (self);

	if (priv->save_strategy)
		g_object_unref (G_OBJECT (priv->save_strategy));

	g_object_ref (G_OBJECT (strategy));
	priv->save_strategy = strategy;

	return;
}


static void
tny_gtk_text_mime_part_view_save_as_activated (GtkMenuItem *menuitem, gpointer user_data)
{
	TnyGtkTextMimePartView *self = user_data;
	TnyGtkTextMimePartViewPriv *priv = TNY_GTK_TEXT_MIME_PART_VIEW_GET_PRIVATE (self);

	if (!G_LIKELY (priv->save_strategy))
	{
		g_warning (_("No save strategy for this mime part view\n"));
		return;
	}

	tny_save_strategy_save (priv->save_strategy, priv->part);

	return;
}


static gint
tny_gtk_text_mime_part_view_popup_handler (GtkWidget *widget, GdkEvent *event)
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

static void 
tny_gtk_text_mime_part_view_set_part (TnyMimePartView *self, TnyMimePart *part)
{
	TnyGtkTextMimePartViewPriv *priv = TNY_GTK_TEXT_MIME_PART_VIEW_GET_PRIVATE (self);

	if (G_LIKELY (priv->part))
		g_object_unref (G_OBJECT (priv->part));
    
    	if (part)
	{
		GtkTextBuffer *buffer;
		TnyStream *dest;

		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self));
		if (buffer && GTK_IS_TEXT_BUFFER (buffer))
			gtk_text_buffer_set_text (buffer, "", 0);
	    
		dest = tny_gtk_text_buffer_stream_new (buffer);
			    
		tny_stream_reset (dest);
		tny_mime_part_decode_to_stream (part, dest);
		tny_stream_reset (dest);

		g_object_unref (G_OBJECT (dest));
		
		g_object_ref (G_OBJECT (part));
		priv->part = part;
		
	}
    
	return;
}

static void
tny_gtk_text_mime_part_view_clear (TnyMimePartView *self)
{
	TnyGtkTextMimePartViewPriv *priv = TNY_GTK_TEXT_MIME_PART_VIEW_GET_PRIVATE (self);
    	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self));
	
	if (buffer && GTK_IS_TEXT_BUFFER (buffer))
		gtk_text_buffer_set_text (buffer, "", 0);
    
    	return;
}

/**
 * tny_gtk_text_mime_part_view_new:
 * @save_strategy: The save strategy to use
 *
 * Return value: a new #TnyMimePartView instance implemented for Gtk+
 **/
TnyMimePartView*
tny_gtk_text_mime_part_view_new (TnySaveStrategy *save_strategy)
{
	TnyGtkTextMimePartView *self = g_object_new (TNY_TYPE_GTK_TEXT_MIME_PART_VIEW, NULL);

	tny_mime_part_view_set_save_strategy (TNY_MIME_PART_VIEW (self), save_strategy);

	return TNY_MIME_PART_VIEW (self);
}

static void
tny_gtk_text_mime_part_view_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyGtkTextMimePartView *self = (TnyGtkTextMimePartView *)instance;
	TnyGtkTextMimePartViewPriv *priv = TNY_GTK_TEXT_MIME_PART_VIEW_GET_PRIVATE (self);
	GtkMenu *menu = GTK_MENU (gtk_menu_new ());
	GtkWidget *mitem = gtk_menu_item_new_with_mnemonic ("Save _As");

	priv->save_strategy = NULL;
	gtk_widget_show (mitem);

	g_signal_connect (G_OBJECT (mitem), "activate", 
		G_CALLBACK (tny_gtk_text_mime_part_view_save_as_activated), self);

	gtk_menu_shell_append (GTK_MENU_SHELL (menu), mitem);
			
	g_signal_connect_swapped (G_OBJECT (self), "button_press_event",
		G_CALLBACK (tny_gtk_text_mime_part_view_popup_handler), menu);

	gtk_text_view_set_editable (GTK_TEXT_VIEW (self), FALSE);

	return;
}

static void
tny_gtk_text_mime_part_view_finalize (GObject *object)
{
	TnyGtkTextMimePartView *self = (TnyGtkTextMimePartView *)object;	
	TnyGtkTextMimePartViewPriv *priv = TNY_GTK_TEXT_MIME_PART_VIEW_GET_PRIVATE (self);

	if (G_LIKELY (priv->part))
		g_object_unref (G_OBJECT (priv->part));

	if (G_LIKELY (priv->save_strategy))
		g_object_unref (G_OBJECT (priv->save_strategy));
    
	(*parent_class->finalize) (object);

	return;
}

static void
tny_gtk_text_mime_part_view_init (gpointer g, gpointer iface_data)
{
	TnyMimePartViewIface *klass = (TnyMimePartViewIface *)g;

	klass->set_part_func = tny_gtk_text_mime_part_view_set_part;
	klass->set_save_strategy_func = tny_gtk_text_mime_part_view_set_save_strategy;
	klass->clear_func = tny_gtk_text_mime_part_view_clear;
	
	return;
}

static void 
tny_gtk_text_mime_part_view_class_init (TnyGtkTextMimePartViewClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_gtk_text_mime_part_view_finalize;

	g_type_class_add_private (object_class, sizeof (TnyGtkTextMimePartViewPriv));

	return;
}

GType 
tny_gtk_text_mime_part_view_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyGtkTextMimePartViewClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_gtk_text_mime_part_view_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyGtkTextMimePartView),
		  0,      /* n_preallocs */
		  tny_gtk_text_mime_part_view_instance_init,    /* instance_init */
		  NULL
		};

		static const GInterfaceInfo tny_gtk_text_mime_part_view_info = 
		{
		  (GInterfaceInitFunc) tny_gtk_text_mime_part_view_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (GTK_TYPE_TEXT_VIEW,
			"TnyGtkTextMimePartView",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_MIME_PART_VIEW, 
			&tny_gtk_text_mime_part_view_info);

	}

	return type;
}

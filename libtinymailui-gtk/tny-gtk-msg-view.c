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

#include <tny-list-iface.h>
#include <tny-iterator-iface.h>

#include <tny-gtk-msg-view.h>
#include <tny-gtk-text-buffer-stream.h>
#include <tny-gtk-attach-list-model.h>
#include <tny-header-view-iface.h>
#include <tny-gtk-header-view.h>

#ifdef GNOME
#include <tny-vfs-stream.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#else
#include <tny-fs-stream.h>
#endif

#include "tny-gtk-attach-list-model-priv.h"

static GObjectClass *parent_class = NULL;

typedef struct _TnyGtkMsgViewPriv TnyGtkMsgViewPriv;

struct _TnyGtkMsgViewPriv
{
	TnyMsgIface *msg;
	GtkTextView *textview;
	TnyHeaderViewIface *headerview;
	GtkIconView *attachview;
	GtkWidget *attachview_sw;
	TnySaveStrategyIface *save_strategy;
};

#define TNY_GTK_MSG_VIEW_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_GTK_MSG_VIEW, TnyGtkMsgViewPriv))

static void
reload_msg (TnyMsgViewIface *self)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);
	GtkTextBuffer *buffer;
	TnyHeaderIface *header;
	TnyIteratorIface *iterator;
	gboolean first_attach = TRUE;
	GtkTreeModel *model;

	g_return_if_fail (TNY_IS_MSG_IFACE (priv->msg));

	header = TNY_HEADER_IFACE (tny_msg_iface_get_header (priv->msg));
    	g_return_if_fail (TNY_IS_HEADER_IFACE (header));
	tny_header_view_iface_set_header (priv->headerview, header);
    	g_object_unref (G_OBJECT (header));

    	buffer = gtk_text_view_get_buffer (priv->textview);
    
    	model = tny_gtk_attach_list_model_new ();
	tny_msg_iface_get_parts (priv->msg, TNY_LIST_IFACE (model));
	iterator = tny_list_iface_create_iterator (TNY_LIST_IFACE (model));

	gtk_widget_hide (priv->attachview_sw);
	gtk_text_buffer_set_text (buffer, "", 0);

    
	gtk_widget_show (GTK_WIDGET (priv->headerview));

	while (!tny_iterator_iface_is_done (iterator))
	{
		TnyMimePartIface *part = (TnyMimePartIface*)tny_iterator_iface_current (iterator);

		if (G_LIKELY (tny_mime_part_iface_content_type_is (part, "text/*")))
		{
			TnyStreamIface *dest = tny_gtk_text_buffer_stream_new (buffer);
		    
			tny_stream_iface_reset (dest);
			tny_mime_part_iface_write_to_stream (part, dest);
			tny_stream_iface_reset (dest);

		    	g_object_unref (G_OBJECT (dest));
		    
		} else if (tny_mime_part_iface_get_content_type (part) &&
			tny_mime_part_iface_is_attachment (part))
		{			
			first_attach = FALSE;
		}
		
		g_object_unref (G_OBJECT(part));
		tny_iterator_iface_next (iterator);
	}

	g_object_unref (G_OBJECT (iterator));


	if (G_LIKELY (!first_attach))
	{
		gtk_icon_view_set_model (priv->attachview, model);
		gtk_widget_show (priv->attachview_sw);
	}

	g_object_unref (G_OBJECT (model));

	return;
}

void
tny_gtk_msg_view_set_save_strategy (TnyMsgViewIface *self, TnySaveStrategyIface *strategy)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);

	if (priv->save_strategy)
		g_object_unref (G_OBJECT (priv->save_strategy));

	g_object_ref (G_OBJECT (strategy));
	priv->save_strategy = strategy;

	return;
}


static void
for_each_selected_attachment (GtkIconView *icon_view, GtkTreePath *path, gpointer user_data)
{
	TnyGtkMsgView *self = user_data;
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);
	GtkTreeModel *model = gtk_icon_view_get_model (icon_view);
	GtkTreeIter iter;

	if (!G_LIKELY (priv->save_strategy))
	{
		g_warning (_("No save strategy for this message view\n"));
		return;
	}

	if (G_LIKELY (gtk_tree_model_get_iter(model, &iter, path)))
	{
		TnyMimePartIface *part;

		gtk_tree_model_get (model, &iter, 
			TNY_GTK_ATTACH_LIST_MODEL_INSTANCE_COLUMN, 
			&part, -1);

		if (G_LIKELY (part))
		{
			tny_save_strategy_iface_save (priv->save_strategy, part);
			g_object_unref (G_OBJECT (part));
		}
	}

	return;
}


static void
tny_gtk_msg_view_save_as_activated (GtkMenuItem *menuitem, gpointer user_data)
{
	TnyGtkMsgView *self = user_data;
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);

	if (!G_LIKELY (priv->save_strategy))
	{
		g_warning (_("No save strategy for this message view\n"));
		return;
	}

	gtk_icon_view_selected_foreach (priv->attachview,
		for_each_selected_attachment, self);

	return;
}



static gint
tny_gtk_msg_view_popup_handler (GtkWidget *widget, GdkEvent *event)
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
tny_gtk_msg_view_set_unavailable (TnyMsgViewIface *self)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);
	GtkTextBuffer *buffer;

    	if (G_LIKELY (priv->msg)) 
    	{
		g_object_unref (G_OBJECT (priv->msg));
		priv->msg = NULL;
	}

	buffer = gtk_text_view_get_buffer (priv->textview);
	gtk_widget_hide (priv->attachview_sw);
	gtk_text_buffer_set_text (buffer, _("Message is unavailable"), -1);

 	tny_header_view_iface_clear (priv->headerview);
	gtk_widget_hide (GTK_WIDGET (priv->headerview));

	return;
}

static void 
tny_gtk_msg_view_set_msg (TnyMsgViewIface *self, TnyMsgIface *msg)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);

	if (G_LIKELY (priv->msg))
		g_object_unref (G_OBJECT (priv->msg));
    
    	if (msg)
	{
		g_object_ref (G_OBJECT (msg));
		priv->msg = msg;
		reload_msg (self);
	}
    
	return;
}

static void
tny_gtk_msg_view_clear (TnyMsgViewIface *self)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);

    	GtkTextBuffer *buffer = gtk_text_view_get_buffer (priv->textview);
	gtk_widget_hide (priv->attachview_sw);
	gtk_text_buffer_set_text (buffer, "", 0);
	tny_header_view_iface_set_header (priv->headerview, NULL);
	gtk_widget_hide (GTK_WIDGET (priv->headerview));
    
    	return;
}

/**
 * tny_gtk_msg_view_new:
 * @save_strategy: The save strategy to use
 *
 * Return value: a new #TnyMsgViewIface instance implemented for Gtk+
 **/
TnyMsgViewIface*
tny_gtk_msg_view_new (TnySaveStrategyIface *save_strategy)
{
	TnyGtkMsgView *self = g_object_new (TNY_TYPE_GTK_MSG_VIEW, NULL);

	tny_msg_view_iface_set_save_strategy (TNY_MSG_VIEW_IFACE (self), save_strategy);

	return TNY_MSG_VIEW_IFACE (self);
}

static void
tny_gtk_msg_view_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyGtkMsgView *self = (TnyGtkMsgView *)instance;
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);
	GtkWidget *vbox = gtk_vbox_new (FALSE, 1);
	GtkMenu *menu = GTK_MENU (gtk_menu_new ());
	GtkWidget *mitem = gtk_menu_item_new_with_mnemonic ("Save _As");

	priv->save_strategy = NULL;

	gtk_scrolled_window_set_hadjustment (GTK_SCROLLED_WINDOW (self), NULL);
	gtk_scrolled_window_set_vadjustment (GTK_SCROLLED_WINDOW (self), NULL);

	gtk_widget_show (mitem);

	g_signal_connect (G_OBJECT (mitem), "activate", 
		G_CALLBACK (tny_gtk_msg_view_save_as_activated), self);

	gtk_menu_shell_append (GTK_MENU_SHELL (menu), mitem);

	priv->attachview_sw = gtk_scrolled_window_new (NULL, NULL);

	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (self), 
			GTK_SHADOW_NONE);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (self),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (priv->attachview_sw), 
			GTK_SHADOW_NONE);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (priv->attachview_sw),
			GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	priv->attachview = GTK_ICON_VIEW (gtk_icon_view_new ());

	gtk_icon_view_set_selection_mode (priv->attachview, GTK_SELECTION_SINGLE);

	g_signal_connect_swapped (G_OBJECT (priv->attachview), "button_press_event",
		G_CALLBACK (tny_gtk_msg_view_popup_handler), menu);

	gtk_icon_view_set_text_column (priv->attachview, 
		TNY_GTK_ATTACH_LIST_MODEL_FILENAME_COLUMN);

	gtk_icon_view_set_pixbuf_column (priv->attachview, 
		TNY_GTK_ATTACH_LIST_MODEL_PIXBUF_COLUMN);

	gtk_icon_view_set_columns (priv->attachview, -1);
	gtk_icon_view_set_item_width (priv->attachview, 100);
	gtk_icon_view_set_column_spacing (priv->attachview, 10);

	priv->headerview = tny_gtk_header_view_new ();

	priv->textview = GTK_TEXT_VIEW (gtk_text_view_new ());

	gtk_text_view_set_editable (priv->textview, FALSE);

	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (priv->headerview), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (priv->textview), TRUE, TRUE, 0);	
	gtk_box_pack_start (GTK_BOX (vbox), priv->attachview_sw, FALSE, TRUE, 0);

	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (self), 
			GTK_WIDGET (vbox));

	gtk_container_add (GTK_CONTAINER (priv->attachview_sw), GTK_WIDGET (priv->attachview));

	gtk_widget_show (GTK_WIDGET (vbox));

	gtk_widget_hide (GTK_WIDGET (priv->headerview));
	gtk_widget_show (GTK_WIDGET (priv->textview));
	gtk_widget_show (GTK_WIDGET (priv->attachview));

	return;
}

static void
tny_gtk_msg_view_finalize (GObject *object)
{
	TnyGtkMsgView *self = (TnyGtkMsgView *)object;	
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);

	if (G_LIKELY (priv->msg))
		g_object_unref (G_OBJECT (priv->msg));

	if (G_LIKELY (priv->save_strategy))
		g_object_unref (G_OBJECT (priv->save_strategy));

    	/* priv->headerview will be destroyed as a GtkObject */
    
	(*parent_class->finalize) (object);

	return;
}

static void
tny_gtk_msg_view_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyMsgViewIfaceClass *klass = (TnyMsgViewIfaceClass *)g_iface;

	klass->set_msg_func = tny_gtk_msg_view_set_msg;
	klass->set_save_strategy_func = tny_gtk_msg_view_set_save_strategy;
	klass->set_unavailable_func = tny_gtk_msg_view_set_unavailable;
	klass->clear_func = tny_gtk_msg_view_clear;
    
	return;
}

static void 
tny_gtk_msg_view_class_init (TnyGtkMsgViewClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_gtk_msg_view_finalize;

	g_type_class_add_private (object_class, sizeof (TnyGtkMsgViewPriv));

	return;
}

GType 
tny_gtk_msg_view_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyGtkMsgViewClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_gtk_msg_view_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyGtkMsgView),
		  0,      /* n_preallocs */
		  tny_gtk_msg_view_instance_init,    /* instance_init */
		  NULL
		};

		static const GInterfaceInfo tny_gtk_msg_view_iface_info = 
		{
		  (GInterfaceInitFunc) tny_gtk_msg_view_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (GTK_TYPE_SCROLLED_WINDOW,
			"TnyGtkMsgView",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_MSG_VIEW_IFACE, 
			&tny_gtk_msg_view_iface_info);

	}

	return type;
}

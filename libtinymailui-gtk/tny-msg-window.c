/* libtinymail - The Tiny Mail base library
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

#include <string.h>
#include <gtk/gtk.h>
#include <tny-msg-window.h>
#include <tny-text-buffer-stream.h>
#include <tny-attach-list-model.h>
#include <tny-vfs-stream.h>

#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-utils.h>

#include "tny-attach-list-model-priv.h"

static GObjectClass *parent_class = NULL;

typedef struct _TnyMsgWindowPriv TnyMsgWindowPriv;

struct _TnyMsgWindowPriv
{
	TnyMsgIface *msg;
	GtkTextView *headerview, *textview;
	GtkIconView *attachview;
	GtkWidget *attachview_sw;
};

#define TNY_MSG_WINDOW_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_MSG_WINDOW_TYPE, TnyMsgWindowPriv))


/* TODO: refactor */
static gchar *
_get_readable_date (const time_t file_time_raw)
{
	struct tm *file_time;
	gchar readable_date[64];
	gsize readable_date_size;

	file_time = localtime (&file_time_raw);

	readable_date_size = strftime (readable_date, 63, "%Y-%m-%d, %-I:%M %p", file_time);		
	
	return g_strdup (readable_date);
}

static void
reload_msg (TnyMsgWindowIface *self)
{
	TnyMsgWindowPriv *priv = TNY_MSG_WINDOW_GET_PRIVATE (self);

	GtkTextIter hiter;
	GtkTextBuffer *headerbuffer = gtk_text_view_get_buffer (priv->headerview);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (priv->textview);
	TnyStreamIface *dest = TNY_STREAM_IFACE (tny_text_buffer_stream_new (buffer));
	TnyMsgHeaderIface *header = TNY_MSG_HEADER_IFACE (tny_msg_iface_get_header (priv->msg));
	GList *parts = (GList*)tny_msg_iface_get_parts (priv->msg);
	const gchar *str = NULL;

	TnyAttachListModel *model = TNY_ATTACH_LIST_MODEL 
			(gtk_icon_view_get_model (priv->attachview));

	gtk_widget_hide (priv->attachview_sw);

	gtk_text_buffer_create_tag (headerbuffer, "bold", 
			"weight", PANGO_WEIGHT_BOLD, NULL);

	gtk_text_buffer_set_text (headerbuffer, "", 0);

	gtk_text_buffer_get_start_iter (headerbuffer, &hiter);
	
	str = "From: ";
	gtk_text_buffer_insert (headerbuffer, &hiter, str, strlen (str));
	str = tny_msg_header_iface_get_from (header);
	gtk_text_buffer_insert (headerbuffer, &hiter, str, strlen (str));
	str = "\n";
	gtk_text_buffer_insert (headerbuffer, &hiter, str, strlen (str));

	str = "To: ";
	gtk_text_buffer_insert (headerbuffer, &hiter, str, strlen (str));
	str = tny_msg_header_iface_get_to (header);
	gtk_text_buffer_insert (headerbuffer, &hiter, str, strlen (str));
	str = "\n";
	gtk_text_buffer_insert (headerbuffer, &hiter, str, strlen (str));

	str = "Subject: ";
	gtk_text_buffer_insert (headerbuffer, &hiter, str, strlen (str));
	str = tny_msg_header_iface_get_subject (header);
	gtk_text_buffer_insert (headerbuffer, &hiter, str, strlen (str));
	str = "\n";
	gtk_text_buffer_insert (headerbuffer, &hiter, str, strlen (str));

	str = "Date: ";
	gtk_text_buffer_insert (headerbuffer, &hiter, str, strlen (str));
	str = (gchar*)_get_readable_date (tny_msg_header_iface_get_date_sent (header));
	gtk_text_buffer_insert (headerbuffer, &hiter, str, strlen (str));
	g_free ((gchar*)str);


	while (parts)
	{
		TnyMsgMimePartIface *part = parts->data;

		if (tny_msg_mime_part_iface_content_type_is (part, "text/*"))
		{
			tny_stream_iface_reset (dest);
			tny_msg_mime_part_iface_write_to_stream (part, dest);
			tny_stream_iface_reset (dest);
		} else if (tny_msg_mime_part_iface_get_content_type (part) &&
			tny_msg_mime_part_iface_get_filename (part))
		{
			tny_attach_list_model_add (model, part);
			gtk_widget_show (priv->attachview_sw);
		}

		parts = g_list_next (parts);
	}

	gtk_window_set_title (GTK_WINDOW (self), tny_msg_header_iface_get_subject (header));

	return;
}

static GnomeVFSResult
save_to_file (const gchar *uri, TnyMsgMimePartIface *part)
{
	GnomeVFSResult result;
	GnomeVFSHandle *handle;
	TnyVfsStream *stream = NULL;

	result = gnome_vfs_create (&handle, uri, 
		GNOME_VFS_OPEN_WRITE, FALSE, 0777);

	if (result != GNOME_VFS_OK)
		return result;

	stream = tny_vfs_stream_new (handle);
	tny_msg_mime_part_iface_write_to_stream (part, TNY_STREAM_IFACE (stream));

	/* This also closes the gnome-vfs handle */
	g_object_unref (G_OBJECT (stream));

	return result;
}

static void
for_each_selected_attachment (GtkIconView *icon_view, GtkTreePath *path, gpointer user_data)
{
	TnyMsgWindow *self = user_data;
	TnyMsgWindowPriv *priv = TNY_MSG_WINDOW_GET_PRIVATE (self);
	GtkTreeModel *model = gtk_icon_view_get_model (icon_view);
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter(model, &iter, path))
	{
		TnyMsgMimePartIface *part;

		gtk_tree_model_get (model, &iter, 
			TNY_ATTACH_LIST_MODEL_INSTANCE_COLUMN, 
			&part, -1);

		if (part)
		{
		
			GtkFileChooserDialog *dialog = GTK_FILE_CHOOSER_DIALOG 
				(gtk_file_chooser_dialog_new ("Save File",
				GTK_WINDOW (self), GTK_FILE_CHOOSER_ACTION_SAVE,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, 
				GTK_RESPONSE_ACCEPT, NULL));
		
			gtk_file_chooser_set_do_overwrite_confirmation 
				(GTK_FILE_CHOOSER (dialog), TRUE);
		
			gtk_file_chooser_set_current_folder 
				(GTK_FILE_CHOOSER (dialog), g_get_home_dir ());

			gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), 
				tny_msg_mime_part_iface_get_filename (part));

			if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
			{
				gchar *uri;
		
				uri = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER (dialog));

				if (uri)
				{
					save_to_file (uri, part);
					g_free (uri);
				}
			}
		
			gtk_widget_destroy (GTK_WIDGET (dialog));
		}
	}

	return;
}


static void
tny_msg_window_save_as_activated (GtkMenuItem *menuitem, gpointer user_data)
{
	TnyMsgWindow *self = user_data;
	TnyMsgWindowPriv *priv = TNY_MSG_WINDOW_GET_PRIVATE (self);

	gtk_icon_view_selected_foreach (priv->attachview,
		for_each_selected_attachment, self);

	return;
}



static gint
tny_msg_window_popup_handler (GtkWidget *widget, GdkEvent *event)
{
	GtkMenu *menu;
	GdkEventButton *event_button;
	
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_MENU (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);
	
	menu = GTK_MENU (widget);
	
	if (event->type == GDK_BUTTON_PRESS)
	{
	  event_button = (GdkEventButton *) event;
	  if (event_button->button == 3)
		{
			gtk_menu_popup (menu, NULL, NULL, NULL, NULL, 
					  event_button->button, event_button->time);
			return TRUE;
		}
	}
	
	return FALSE;
}

static void 
tny_msg_window_set_msg (TnyMsgWindowIface *self, TnyMsgIface *msg)
{
	TnyMsgWindowPriv *priv = TNY_MSG_WINDOW_GET_PRIVATE (self);

	if (priv->msg)
		g_object_unref (G_OBJECT (priv->msg));

	g_object_ref (G_OBJECT (msg));

	priv->msg = msg;

	reload_msg (self);

	return;
}


TnyMsgWindow*
tny_msg_window_new (void)
{
	TnyMsgWindow *self = g_object_new (TNY_MSG_WINDOW_TYPE, NULL);

	return self;
}

static void
tny_msg_window_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyMsgWindow *self = (TnyMsgWindow *)instance;
	TnyMsgWindowPriv *priv = TNY_MSG_WINDOW_GET_PRIVATE (self);
	GtkWidget *textview_sw = gtk_scrolled_window_new (NULL, NULL);
	GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
	GtkTreeModel *model = GTK_TREE_MODEL (tny_attach_list_model_new());
	GtkMenu *menu = GTK_MENU (gtk_menu_new ());
	GtkWidget *mitem = gtk_menu_item_new_with_mnemonic ("Save _As");
	
	gtk_widget_show (mitem);

	g_signal_connect (G_OBJECT (mitem), "activate", 
		G_CALLBACK (tny_msg_window_save_as_activated), self);

	gtk_menu_shell_append (GTK_MENU_SHELL (menu), mitem);

	priv->attachview_sw = gtk_scrolled_window_new (NULL, NULL);

	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (textview_sw), 
			GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (textview_sw),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (priv->attachview_sw), 
			GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (priv->attachview_sw),
			GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	priv->attachview = GTK_ICON_VIEW (gtk_icon_view_new_with_model (model));

	gtk_icon_view_set_selection_mode (priv->attachview, GTK_SELECTION_SINGLE);

	g_signal_connect_swapped (G_OBJECT (priv->attachview), "button_press_event",
		G_CALLBACK (tny_msg_window_popup_handler), menu);

	_tny_attach_list_model_set_screen (TNY_ATTACH_LIST_MODEL (model),
		gtk_widget_get_screen (GTK_WIDGET (priv->attachview)));

	gtk_icon_view_set_text_column (priv->attachview, 
		TNY_ATTACH_LIST_MODEL_FILENAME_COLUMN);

	gtk_icon_view_set_pixbuf_column (priv->attachview, 
		TNY_ATTACH_LIST_MODEL_PIXBUF_COLUMN);

	gtk_icon_view_set_columns (priv->attachview, -1);
	gtk_icon_view_set_item_width (priv->attachview, 100);
	gtk_icon_view_set_column_spacing (priv->attachview, 10);

	priv->headerview = GTK_TEXT_VIEW (gtk_text_view_new ());
	priv->textview = GTK_TEXT_VIEW (gtk_text_view_new ());

	gtk_text_view_set_editable (priv->headerview, FALSE);
	gtk_text_view_set_editable (priv->textview, FALSE);

	gtk_container_set_border_width (GTK_CONTAINER (self), 8);

	gtk_window_set_default_size (GTK_WINDOW (self), 300, 200);

	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (priv->headerview), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (priv->textview), TRUE, TRUE, 0);	
	gtk_box_pack_start (GTK_BOX (vbox), priv->attachview_sw, FALSE, TRUE, 0);

	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (textview_sw), 
			GTK_WIDGET (vbox));

	gtk_container_add (GTK_CONTAINER (priv->attachview_sw), GTK_WIDGET (priv->attachview));
	gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (textview_sw));


	gtk_widget_show (GTK_WIDGET (vbox));
	gtk_widget_show (GTK_WIDGET (textview_sw));

	gtk_widget_show (GTK_WIDGET (priv->headerview));
	gtk_widget_show (GTK_WIDGET (priv->textview));
	gtk_widget_show (GTK_WIDGET (priv->attachview));

	return;
}

static void
tny_msg_window_finalize (GObject *object)
{
	TnyMsgWindow *self = (TnyMsgWindow *)object;	
	TnyMsgWindowPriv *priv = TNY_MSG_WINDOW_GET_PRIVATE (self);

	if (priv->msg)
		g_object_unref (G_OBJECT (priv->msg));

	(*parent_class->finalize) (object);

	return;
}

static void
tny_msg_window_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyMsgWindowIfaceClass *klass = (TnyMsgWindowIfaceClass *)g_iface;

	klass->set_msg_func = tny_msg_window_set_msg;

	return;
}

static void 
tny_msg_window_class_init (TnyMsgWindowClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_msg_window_finalize;

	g_type_class_add_private (object_class, sizeof (TnyMsgWindowPriv));

	return;
}

GType 
tny_msg_window_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgWindowClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_msg_window_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMsgWindow),
		  0,      /* n_preallocs */
		  tny_msg_window_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_msg_window_iface_info = 
		{
		  (GInterfaceInitFunc) tny_msg_window_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (GTK_TYPE_WINDOW,
			"TnyMsgWindow",
			&info, 0);

		g_type_add_interface_static (type, TNY_MSG_WINDOW_IFACE_TYPE, 
			&tny_msg_window_iface_info);

	}

	return type;
}

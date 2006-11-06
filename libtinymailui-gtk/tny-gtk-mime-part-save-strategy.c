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
#include <tny-gtk-mime-part-save-strategy.h>
#include <tny-gtk-text-buffer-stream.h>
#include <tny-gtk-attach-list-model.h>

#include <tny-mime-part.h>

#ifdef GNOME
#include <tny-vfs-stream.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#else
#include <tny-fs-stream.h>
#endif

static GObjectClass *parent_class = NULL;

#define TNY_GTK_MIME_PART_SAVE_STRATEGY_GET_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_GTK_MIME_PART_SAVE_STRATEGY, TnyGtkMimePartSaveStrategyPriv))


#ifdef GNOME
static gboolean
gtk_save_to_file (const gchar *uri, TnyMimePart *part)
{
	GnomeVFSResult result;
	GnomeVFSHandle *handle;
	TnyStream *stream = NULL;

	result = gnome_vfs_create (&handle, uri, 
		GNOME_VFS_OPEN_WRITE, FALSE, 0777);

	if (G_UNLIKELY (result != GNOME_VFS_OK))
		return FALSE;

	stream = tny_vfs_stream_new (handle);
	tny_mime_part_decode_to_stream (part, stream);
    
	/* This also closes the handle */    
	g_object_unref (G_OBJECT (stream));

	return TRUE;
}
#else
static gboolean
gtk_save_to_file (const gchar *local_filename, TnyMimePart *part)
{
	int fd = open (local_filename, O_WRONLY | O_CREAT,  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (fd != -1)
	{
		TnyStream *stream = tny_fs_stream_new (fd);
		tny_mime_part_decode_to_stream (part, TNY_STREAM (stream));

		/* This also closes the file descriptor */
		g_object_unref (G_OBJECT (stream));
		return TRUE;
	}

	return FALSE;
}
#endif


static void
tny_gtk_mime_part_save_strategy_save (TnyMimePartSaveStrategy *self, TnyMimePart *part)
{
	TNY_GTK_MIME_PART_SAVE_STRATEGY_GET_CLASS (self)->save_func (self, part);
	return;
}

static void
tny_gtk_mime_part_save_strategy_save_default (TnyMimePartSaveStrategy *self, TnyMimePart *part)
{
	GtkFileChooserDialog *dialog;
	gboolean destr=FALSE;

	dialog = GTK_FILE_CHOOSER_DIALOG 
		(gtk_file_chooser_dialog_new (_("Save File"), NULL,
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, 
		GTK_RESPONSE_ACCEPT, NULL));

	/* gtk_file_chooser_set_do_overwrite_confirmation 
		(GTK_FILE_CHOOSER (dialog), TRUE); */

	gtk_file_chooser_set_current_folder 
		(GTK_FILE_CHOOSER (dialog), g_get_home_dir ());

#ifndef GNOME
	gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (dialog), TRUE);
#endif

	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), 
		tny_mime_part_get_filename (part));

	if (G_LIKELY (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT))
	{
		gchar *uri;
#ifdef GNOME
		uri = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER (dialog));
#else
		uri = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
#endif
		if (uri)
		{
			if (!gtk_save_to_file (uri, part))
			{
				gtk_widget_destroy (GTK_WIDGET (dialog));
				destr = TRUE;
				GtkWidget *errd;
				errd = gtk_message_dialog_new (NULL,
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_CLOSE,
					_("Saving to %s failed\n"), uri);
				gtk_dialog_run (GTK_DIALOG (errd));
				gtk_widget_destroy (GTK_WIDGET (errd));
			}

			g_free (uri);
		}
	}

	if (!destr)
		gtk_widget_destroy (GTK_WIDGET (dialog));

	return;
}


/**
 * tny_gtk_mime_part_save_strategy_new:
 *
 *
 * Return value: a new #TnyMimePartSaveStrategy instance implemented for Gtk+
 **/
TnyMimePartSaveStrategy*
tny_gtk_mime_part_save_strategy_new (void)
{
	TnyGtkMimePartSaveStrategy *self = g_object_new (TNY_TYPE_GTK_MIME_PART_SAVE_STRATEGY, NULL);

	return TNY_MIME_PART_SAVE_STRATEGY (self);
}

static void
tny_gtk_mime_part_save_strategy_instance_init (GTypeInstance *instance, gpointer g_class)
{
	return;
}

static void
tny_gtk_mime_part_save_strategy_finalize (GObject *object)
{
	(*parent_class->finalize) (object);

	return;
}

static void
tny_gtk_mime_part_save_strategy_init (gpointer g, gpointer iface_data)
{
	TnyMimePartSaveStrategyIface *klass = (TnyMimePartSaveStrategyIface *)g;

	klass->save_func = tny_gtk_mime_part_save_strategy_save;

	return;
}

static void 
tny_gtk_mime_part_save_strategy_class_init (TnyGtkMimePartSaveStrategyClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	class->save_func = tny_gtk_mime_part_save_strategy_save_default;

	object_class->finalize = tny_gtk_mime_part_save_strategy_finalize;

	return;
}

GType 
tny_gtk_mime_part_save_strategy_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyGtkMimePartSaveStrategyClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_gtk_mime_part_save_strategy_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyGtkMimePartSaveStrategy),
		  0,      /* n_preallocs */
		  tny_gtk_mime_part_save_strategy_instance_init,    /* instance_init */
		  NULL
		};

		static const GInterfaceInfo tny_gtk_mime_part_save_strategy_info = 
		{
		  (GInterfaceInitFunc) tny_gtk_mime_part_save_strategy_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyGtkMimePartSaveStrategy",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_MIME_PART_SAVE_STRATEGY, 
			&tny_gtk_mime_part_save_strategy_info);

	}

	return type;
}

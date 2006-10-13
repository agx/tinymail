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
#include <tny-list.h>

#ifdef GNOME
#include <tny-vfs-stream.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#else
#include <tny-fs-stream.h>
#endif

#include <tny-gtk-attachment-mime-part-view.h>


static GObjectClass *parent_class = NULL;

typedef struct _TnyGtkAttachmentMimePartViewPriv TnyGtkAttachmentMimePartViewPriv;

struct _TnyGtkAttachmentMimePartViewPriv
{
	TnyMimePart *part;
	TnySaveStrategy *save_strategy;
	TnyGtkAttachListModel *imodel;
};

#define TNY_GTK_ATTACHMENT_MIME_PART_VIEW_GET_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_GTK_ATTACHMENT_MIME_PART_VIEW, TnyGtkAttachmentMimePartViewPriv))


static void
tny_gtk_attachment_mime_part_view_set_save_strategy (TnyMimePartView *self, TnySaveStrategy *strategy)
{
	TnyGtkAttachmentMimePartViewPriv *priv = TNY_GTK_ATTACHMENT_MIME_PART_VIEW_GET_PRIVATE (self);

	if (priv->save_strategy)
		g_object_unref (G_OBJECT (priv->save_strategy));

	g_object_ref (G_OBJECT (strategy));
	priv->save_strategy = strategy;

	return;
}


static void
tny_gtk_attachment_mime_part_view_save_as_activated (GtkMenuItem *menuitem, gpointer user_data)
{
	TnyGtkAttachmentMimePartView *self = user_data;
	TnyGtkAttachmentMimePartViewPriv *priv = TNY_GTK_ATTACHMENT_MIME_PART_VIEW_GET_PRIVATE (self);

	if (!G_LIKELY (priv->save_strategy))
	{
		g_warning (_("No save strategy for this mime part view\n"));
		return;
	}

	tny_save_strategy_save (priv->save_strategy, priv->part);

	return;
}


static gint
tny_gtk_attachment_mime_part_view_popup_handler (GtkWidget *widget, GdkEvent *event)
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
tny_gtk_attachment_mime_part_view_set_part (TnyMimePartView *self, TnyMimePart *part)
{
	TnyGtkAttachmentMimePartViewPriv *priv = TNY_GTK_ATTACHMENT_MIME_PART_VIEW_GET_PRIVATE (self);

	if (G_LIKELY (priv->part))
		g_object_unref (G_OBJECT (priv->part));

	if (part)
	{
		tny_list_prepend (TNY_LIST (priv->imodel), G_OBJECT (part));
		g_object_ref (G_OBJECT (part));
		priv->part = part;
	}

	return;
}

static void
tny_gtk_attachment_mime_part_view_clear (TnyMimePartView *self)
{
	TnyGtkAttachmentMimePartViewPriv *priv = TNY_GTK_ATTACHMENT_MIME_PART_VIEW_GET_PRIVATE (self);

	if (priv->part)
	{
		tny_list_remove (TNY_LIST (priv->imodel), G_OBJECT (priv->part));
		g_object_unref (G_OBJECT (priv->part));
		priv->part = NULL;
	}

	return;
}

/**
 * tny_gtk_attachment_mime_part_view_new:
 * @save_strategy: The save strategy to use
 *
 * Return value: a new #TnyMimePartView instance implemented for Gtk+
 **/
TnyMimePartView*
tny_gtk_attachment_mime_part_view_new (TnySaveStrategy *save_strategy, TnyGtkAttachListModel *imodel)
{
	TnyGtkAttachmentMimePartView *self = g_object_new (TNY_TYPE_GTK_ATTACHMENT_MIME_PART_VIEW, NULL);

	tny_mime_part_view_set_save_strategy (TNY_MIME_PART_VIEW (self), save_strategy);

	g_object_ref (G_OBJECT (imodel));
	TNY_GTK_ATTACHMENT_MIME_PART_VIEW_GET_PRIVATE (self)->imodel = imodel;

	return TNY_MIME_PART_VIEW (self);
}

static void
tny_gtk_attachment_mime_part_view_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyGtkAttachmentMimePartView *self = (TnyGtkAttachmentMimePartView *)instance;
	TnyGtkAttachmentMimePartViewPriv *priv = TNY_GTK_ATTACHMENT_MIME_PART_VIEW_GET_PRIVATE (self);
	GtkMenu *menu = GTK_MENU (gtk_menu_new ());
	GtkWidget *mitem = gtk_menu_item_new_with_mnemonic ("Save _As");

	priv->imodel = NULL;
	priv->save_strategy = NULL;

	gtk_widget_show (mitem);
	g_signal_connect (G_OBJECT (mitem), "activate", 
		G_CALLBACK (tny_gtk_attachment_mime_part_view_save_as_activated), self);

	gtk_menu_shell_append (GTK_MENU_SHELL (menu), mitem);

	g_signal_connect_swapped (G_OBJECT (self), "button_press_event",
		G_CALLBACK (tny_gtk_attachment_mime_part_view_popup_handler), menu);

	return;
}

static void
tny_gtk_attachment_mime_part_view_finalize (GObject *object)
{
	TnyGtkAttachmentMimePartView *self = (TnyGtkAttachmentMimePartView *)object;	
	TnyGtkAttachmentMimePartViewPriv *priv = TNY_GTK_ATTACHMENT_MIME_PART_VIEW_GET_PRIVATE (self);

	if (priv->imodel)
		g_object_unref (G_OBJECT (priv->imodel));
	priv->imodel = NULL;

	if (G_LIKELY (priv->part))
		g_object_unref (G_OBJECT (priv->part));
	priv->part = NULL;

	if (G_LIKELY (priv->save_strategy))
		g_object_unref (G_OBJECT (priv->save_strategy));
	priv->save_strategy = NULL;

	(*parent_class->finalize) (object);

	return;
}

static void
tny_mime_part_view_init (gpointer g, gpointer iface_data)
{
	TnyMimePartViewIface *klass = (TnyMimePartViewIface *)g;

	klass->set_part_func = tny_gtk_attachment_mime_part_view_set_part;
	klass->set_save_strategy_func = tny_gtk_attachment_mime_part_view_set_save_strategy;
	klass->clear_func = tny_gtk_attachment_mime_part_view_clear;

	return;
}

static void 
tny_gtk_attachment_mime_part_view_class_init (TnyGtkAttachmentMimePartViewClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;
	object_class->finalize = tny_gtk_attachment_mime_part_view_finalize;
	g_type_class_add_private (object_class, sizeof (TnyGtkAttachmentMimePartViewPriv));

	return;
}

GType 
tny_gtk_attachment_mime_part_view_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyGtkAttachmentMimePartViewClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_gtk_attachment_mime_part_view_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyGtkAttachmentMimePartView),
		  0,      /* n_preallocs */
		  tny_gtk_attachment_mime_part_view_instance_init,    /* instance_init */
		  NULL
		};

		static const GInterfaceInfo tny_mime_part_view_info = 
		{
		  (GInterfaceInitFunc) tny_mime_part_view_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyGtkAttachmentMimePartView",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_MIME_PART_VIEW, 
			&tny_mime_part_view_info);

	}

	return type;
}

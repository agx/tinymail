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

#include <glib.h>
#include <gtk/gtk.h>
#include <libgnomeui/libgnomeui.h>

#include <tny-attach-list-model.h>

static GObjectClass *parent_class = NULL;

#include "tny-attach-list-model-priv.h"

#define TNY_ATTACH_LIST_MODEL_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_ATTACH_LIST_MODEL_TYPE, TnyAttachListModelPriv))


void
_tny_attach_list_model_set_screen (TnyAttachListModel *self, GdkScreen *screen)
{
	TnyAttachListModelPriv *priv = TNY_ATTACH_LIST_MODEL_GET_PRIVATE (self);

	priv->screen = screen;
	priv->theme = gtk_icon_theme_get_for_screen (priv->screen);

	return;
}


void
tny_attach_list_model_add (TnyAttachListModel *self, TnyMsgMimePartIface *part)
{
	GtkListStore *model = GTK_LIST_STORE (self);
	GtkTreeIter iter;
	TnyAttachListModelPriv *priv = TNY_ATTACH_LIST_MODEL_GET_PRIVATE (self);

	GdkPixbuf *pixbuf;
        gchar *icon;
	
	/* THE gnomeui-2 dependency */
	icon = gnome_icon_lookup (priv->theme, NULL, 
		tny_msg_mime_part_iface_get_filename (part), NULL, NULL,
		mime_type, 0, NULL);

	if (icon)
	{
		pixbuf = gtk_icon_theme_load_icon (priv->theme, icon, 
			ICON_SIZE_LARGE_TOOLBAR, 0, NULL);
		g_free (icon);
	}

	gtk_list_store_append (model, &iter);

	gtk_list_store_set (model, &iter,
		TNY_ATTACH_LIST_MODEL_PIXBUF_COLUMN, 
		pixbuf,
		TNY_ATTACH_LIST_MODEL_FILENAME_COLUMN, 
		tny_msg_mime_part_iface_get_filename (part),
		TNY_ATTACH_LIST_MODEL_INSTANCE_COLUMN,
		part, -1);

	return;
}

TnyAttachListModel*
tny_attach_list_model_new (void)
{
	TnyAttachListModel *self = g_object_new (TNY_ATTACH_LIST_MODEL_TYPE, NULL);

	return self;
}

static void
tny_attach_list_model_finalize (GObject *object)
{
	(*parent_class->finalize) (object);
}

static void
tny_attach_list_model_class_init (TnyAttachListModelClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_attach_list_model_finalize;

	g_type_class_add_private (object_class, sizeof (TnyAttachListModelPriv));

	return;
}

static void
tny_attach_list_model_instance_init (GTypeInstance *instance, gpointer g_class)
{
	GtkListStore *store = (GtkListStore*) instance;
	static GType types[] = { G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_POINTER };

	gtk_list_store_set_column_types (store, 
		TNY_ATTACH_LIST_MODEL_N_COLUMNS, types);

	return;
}


GType
tny_attach_list_model_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyAttachListModelClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_attach_list_model_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyAttachListModel),
		  0,      /* n_preallocs */
		  tny_attach_list_model_instance_init    /* instance_init */
		};

		type = g_type_register_static (GTK_TYPE_LIST_STORE, "TnyAttachListModel",
					    &info, 0);
	}

	return type;
}

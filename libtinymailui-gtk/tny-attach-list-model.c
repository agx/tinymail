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

#include <glib.h>
#include <gtk/gtk.h>

#ifdef GNOME
#include <libgnomeui/libgnomeui.h>
#endif

#include <tny-attach-list-model.h>
#include <tny-msg-mime-part-iface.h>
#include <tny-iterator-iface.h>
#include <tny-msg-mime-part-iface.h>
#include <tny-folder-iface.h>

#include "tny-attach-list-model-priv.h"

static GObjectClass *parent_class = NULL;


#define TNY_ATTACH_LIST_MODEL_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_ATTACH_LIST_MODEL, TnyAttachListModelPriv))

typedef void (*listaddfunc) (GtkListStore *list_store, GtkTreeIter *iter);

static void
tny_attach_list_model_add (TnyAttachListModel *self, TnyMsgMimePartIface *part, listaddfunc func)
{
	GtkListStore *model = GTK_LIST_STORE (self);
	GtkTreeIter iter;
	TnyAttachListModelPriv *priv = TNY_ATTACH_LIST_MODEL_GET_PRIVATE (self);

	static GdkPixbuf *stock_file_pixbuf = NULL;
	GdkPixbuf *pixbuf;
        gchar *icon;

	if (tny_msg_mime_part_iface_get_content_type (part) &&
			tny_msg_mime_part_iface_is_attachment (part))
	{

		if (!priv->theme)
			priv->theme = gtk_icon_theme_get_default ();

#ifdef GNOME
		icon = gnome_icon_lookup (priv->theme, NULL, 
			tny_msg_mime_part_iface_get_filename (part), NULL, NULL,
			tny_msg_mime_part_iface_get_content_type (part), 0, NULL);
#else
		icon = GTK_STOCK_FILE;
#endif

		if (G_LIKELY (icon))
		{
			pixbuf = gtk_icon_theme_load_icon (priv->theme, icon, 
				GTK_ICON_SIZE_LARGE_TOOLBAR, 0, NULL);
#ifdef GNOME
			g_free (icon);
#endif
		} else {
			if (G_UNLIKELY (!stock_file_pixbuf))
				stock_file_pixbuf = gtk_icon_theme_load_icon (priv->theme, 
					GTK_STOCK_FILE, GTK_ICON_SIZE_LARGE_TOOLBAR, 
					0, NULL);

			pixbuf = stock_file_pixbuf;
		}

		func (model, &iter);

		gtk_list_store_set (model, &iter,
			TNY_ATTACH_LIST_MODEL_PIXBUF_COLUMN, 
			pixbuf,
			TNY_ATTACH_LIST_MODEL_FILENAME_COLUMN, 
			tny_msg_mime_part_iface_get_filename (part),
			TNY_ATTACH_LIST_MODEL_INSTANCE_COLUMN,
			part, -1);
	}

	return;
}


/**
 * tny_attach_list_model_new:
 *
 *
 * Return value: a new #GtkTreeModel instance suitable for showing  
 * #TnyMsgMimePartIface instances
 **/
TnyAttachListModel*
tny_attach_list_model_new (void)
{
	TnyAttachListModel *self = g_object_new (TNY_TYPE_ATTACH_LIST_MODEL, NULL);

	return self;
}

static void
tny_attach_list_model_finalize (GObject *object)
{
	TnyAttachListModelPriv *priv = TNY_ATTACH_LIST_MODEL_GET_PRIVATE (object);
	TnyAttachListModel *me = (TnyAttachListModel*) object;

	g_mutex_free (me->iterator_lock);
	me->iterator_lock = NULL;

	if (priv->theme)
		g_object_unref (G_OBJECT (priv->theme));

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
	TnyAttachListModel *me = (TnyAttachListModel*) instance;
	TnyAttachListModelPriv *priv = TNY_ATTACH_LIST_MODEL_GET_PRIVATE (instance);
	static GType types[] = { G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_OBJECT };

	priv->theme = NULL;
	types[0] = GDK_TYPE_PIXBUF;
	me->iterator_lock = g_mutex_new ();

	gtk_list_store_set_column_types (store, 
		TNY_ATTACH_LIST_MODEL_N_COLUMNS, types);

	return;
}

static TnyIteratorIface*
tny_attach_list_model_create_iterator (TnyListIface *self)
{
	TnyAttachListModel *me = (TnyAttachListModel*)self;

	/* Return a new iterator */

	return TNY_ITERATOR_IFACE (_tny_attach_list_model_iterator_new (me));
}



static void
tny_attach_list_model_prepend (TnyListIface *self, GObject* item)
{
	TnyAttachListModel *me = (TnyAttachListModel*)self;

	g_mutex_lock (me->iterator_lock);

	/* Prepend something to the list */
	g_object_ref (G_OBJECT (item));
	me->first = g_list_prepend (me->first, item);
	tny_attach_list_model_add (me, TNY_MSG_MIME_PART_IFACE (item), 
		gtk_list_store_prepend);

	g_mutex_unlock (me->iterator_lock);
}

static void
tny_attach_list_model_append (TnyListIface *self, GObject* item)
{
	TnyAttachListModel *me = (TnyAttachListModel*)self;

	g_mutex_lock (me->iterator_lock);

	/* Append something to the list */
	g_object_ref (G_OBJECT (item));
	me->first = g_list_append (me->first, item);
	tny_attach_list_model_add (me, TNY_MSG_MIME_PART_IFACE (item), 
		gtk_list_store_append);

	g_mutex_unlock (me->iterator_lock);
}

static guint
tny_attach_list_model_length (TnyListIface *self)
{
	TnyAttachListModel *me = (TnyAttachListModel*)self;
	guint retval = 0;

	g_mutex_lock (me->iterator_lock);

	retval = me->first?g_list_length (me->first):0;

	g_mutex_unlock (me->iterator_lock);

	return retval;
}

static void
tny_attach_list_model_remove (TnyListIface *self, GObject* item)
{
	TnyAttachListModel *me = (TnyAttachListModel*)self;
	GtkTreeModel *model = GTK_TREE_MODEL (me);
	GtkTreeIter iter;

	g_return_if_fail (G_IS_OBJECT (item));
	g_return_if_fail (G_IS_OBJECT (me));

	/* Remove something from the list */

	g_mutex_lock (me->iterator_lock);

	me->first = g_list_remove (me->first, (gconstpointer)item);

	gtk_tree_model_get_iter_first (model, &iter);
	while (gtk_tree_model_iter_next (model, &iter))
	{
		TnyMsgMimePartIface *curpart;

		gtk_tree_model_get (model, &iter, 
			TNY_ATTACH_LIST_MODEL_INSTANCE_COLUMN, 
			&curpart, -1);

		if (curpart == (TnyMsgMimePartIface*)item)
		{
			gtk_list_store_remove (GTK_LIST_STORE (me), &iter);
			g_object_unref (G_OBJECT (item));

			break;
		}
	}

	g_mutex_unlock (me->iterator_lock);
}


static TnyListIface*
tny_attach_list_model_copy_the_list (TnyListIface *self)
{
	TnyAttachListModel *me = (TnyAttachListModel*)self;
	TnyAttachListModel *copy = g_object_new (TNY_TYPE_ATTACH_LIST_MODEL, NULL);

	/* This only copies the TnyListIface pieces. The result is not a
	   correct or good TnyMsgHeaderListModel. But it will be a correct
	   TnyListIface instance. It is the only thing the user of this
	   method expects.

	   The new list will point to the same instances, of course. It's
	   only a copy of the list-nodes of course. */

	g_mutex_lock (me->iterator_lock);
	GList *list_copy = g_list_copy (me->first);
	copy->first = list_copy;
	g_mutex_unlock (me->iterator_lock);

	return TNY_LIST_IFACE (copy);
}

static void 
tny_attach_list_model_foreach_in_the_list (TnyListIface *self, GFunc func, gpointer user_data)
{
	TnyAttachListModel *me = (TnyAttachListModel*)self;

	/* Foreach item in the list (without using a slower iterator) */

	g_mutex_lock (me->iterator_lock);
	g_list_foreach (me->first, func, user_data);
	g_mutex_unlock (me->iterator_lock);

	return;
}

static void
tny_list_iface_init (TnyListIfaceClass *klass)
{
	klass->length_func = tny_attach_list_model_length;
	klass->prepend_func = tny_attach_list_model_prepend;
	klass->append_func = tny_attach_list_model_append;
	klass->remove_func = tny_attach_list_model_remove;
	klass->create_iterator_func = tny_attach_list_model_create_iterator;
	klass->copy_func = tny_attach_list_model_copy_the_list;
	klass->foreach_func = tny_attach_list_model_foreach_in_the_list;

	return;
}

GType
tny_attach_list_model_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
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


		static const GInterfaceInfo tny_list_iface_info = {
			(GInterfaceInitFunc) tny_list_iface_init,
			NULL,
			NULL
		};

		g_type_add_interface_static (type, TNY_TYPE_LIST_IFACE,
					     &tny_list_iface_info);

	}

	return type;
}


GType 
tny_attach_list_model_column_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TNY_ATTACH_LIST_MODEL_PIXBUF_COLUMN, "TNY_ATTACH_LIST_MODEL_PIXBUF_COLUMN", "pixbuf" },
      { TNY_ATTACH_LIST_MODEL_FILENAME_COLUMN, "TNY_ATTACH_LIST_MODEL_FILENAME_COLUMN", "filename" },
      { TNY_ATTACH_LIST_MODEL_INSTANCE_COLUMN, "TNY_ATTACH_LIST_MODEL_INSTANCE_COLUMN", "instance" },
      { TNY_ATTACH_LIST_MODEL_N_COLUMNS, "TNY_ATTACH_LIST_MODEL_N_COLUMNS", "n" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyAttachListModelColumn", values);
  }
  return etype;
}


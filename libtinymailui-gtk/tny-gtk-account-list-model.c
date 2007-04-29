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

#include <tny-gtk-account-list-model.h>
#include <tny-mime-part.h>
#include <tny-iterator.h>
#include <tny-mime-part.h>
#include <tny-folder.h>

#include "tny-gtk-account-list-model-iterator-priv.h"


static GObjectClass *parent_class = NULL;


#define TNY_GTK_ACCOUNT_LIST_MODEL_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_GTK_ACCOUNT_LIST_MODEL, TnyGtkAccountListModelPriv))


/**
 * tny_gtk_account_list_model_new:
 *
 * Create a new #GtkTreeModel instance suitable for showing a #TnyMimePart.
 *
 * Return value: a new #GtkTreeModel instance suitable for showing a #TnyMimePart
 **/
GtkTreeModel*
tny_gtk_account_list_model_new (void)
{
	TnyGtkAccountListModel *self = g_object_new (TNY_TYPE_GTK_ACCOUNT_LIST_MODEL, NULL);

	return GTK_TREE_MODEL (self);
}

static void 
destroy_accs (gpointer item, gpointer user_data)
{
	if (item && G_IS_OBJECT (item))
		g_object_unref (G_OBJECT (item));
	return;
}

static void
tny_gtk_account_list_model_finalize (GObject *object)
{
	TnyGtkAccountListModel *me = (TnyGtkAccountListModel*) object;

	g_mutex_lock (me->iterator_lock);
	if (me->first)
	{
		g_list_foreach (me->first, destroy_accs, NULL);
		g_list_free (me->first); me->first = NULL;
	}
	g_mutex_unlock (me->iterator_lock);

	g_mutex_free (me->iterator_lock);

	(*parent_class->finalize) (object);
}

static void
tny_gtk_account_list_model_class_init (TnyGtkAccountListModelClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_gtk_account_list_model_finalize;

	return;
}

static void
tny_gtk_account_list_model_instance_init (GTypeInstance *instance, gpointer g_class)
{
	GtkListStore *store = (GtkListStore*) instance;
	TnyGtkAccountListModel *me = (TnyGtkAccountListModel*) instance;
	static GType types[] = { G_TYPE_STRING, G_TYPE_OBJECT };

	me->iterator_lock = g_mutex_new ();
	me->first = NULL;

	gtk_list_store_set_column_types (store, 
		TNY_GTK_ACCOUNT_LIST_MODEL_N_COLUMNS, types);

	return;
}

static TnyIterator*
tny_gtk_account_list_model_create_iterator (TnyList *self)
{
	TnyGtkAccountListModel *me = (TnyGtkAccountListModel*)self;

	/* Return a new iterator */

	return TNY_ITERATOR (_tny_gtk_account_list_model_iterator_new (me));
}



static void
tny_gtk_account_list_model_prepend (TnyList *self, GObject* item)
{
	TnyGtkAccountListModel *me = (TnyGtkAccountListModel*)self;
	GtkListStore *store = GTK_LIST_STORE (me);
	GtkTreeIter iter;
	TnyAccount *account = TNY_ACCOUNT (item);

	g_mutex_lock (me->iterator_lock);
	g_object_ref (G_OBJECT (item));
	me->first = g_list_prepend (me->first, item);    
	gtk_list_store_prepend (store, &iter);
	gtk_list_store_set (store, &iter, 
		TNY_GTK_ACCOUNT_LIST_MODEL_NAME_COLUMN, tny_account_get_name (account),
		TNY_GTK_ACCOUNT_LIST_MODEL_INSTANCE_COLUMN, account, -1); 
	g_mutex_unlock (me->iterator_lock);
}

static void
tny_gtk_account_list_model_append (TnyList *self, GObject* item)
{
	TnyGtkAccountListModel *me = (TnyGtkAccountListModel*)self;
	GtkListStore *store = GTK_LIST_STORE (me);
	GtkTreeIter iter;
	TnyAccount *account = TNY_ACCOUNT (item);

	g_mutex_lock (me->iterator_lock);
	g_object_ref (G_OBJECT (item)); 
	me->first = g_list_append (me->first, item);    
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, 
		TNY_GTK_ACCOUNT_LIST_MODEL_NAME_COLUMN, tny_account_get_name (account),
		TNY_GTK_ACCOUNT_LIST_MODEL_INSTANCE_COLUMN, account, -1);    
	g_mutex_unlock (me->iterator_lock);
}

static guint
tny_gtk_account_list_model_get_length (TnyList *self)
{
	TnyGtkAccountListModel *me = (TnyGtkAccountListModel*)self;
	guint retval = 0;
	GtkTreeIter iter;

	g_mutex_lock (me->iterator_lock);
	retval = me->first?g_list_length (me->first):0;
	g_mutex_unlock (me->iterator_lock);

	return retval;
}

static void
tny_gtk_account_list_model_remove (TnyList *self, GObject* item)
{
	TnyGtkAccountListModel *me = (TnyGtkAccountListModel*)self;
	GtkTreeModel *model = GTK_TREE_MODEL (me);
	GtkTreeIter iter;

	g_return_if_fail (G_IS_OBJECT (item));
	g_return_if_fail (G_IS_OBJECT (me));

	/* Remove something from the list */

	g_mutex_lock (me->iterator_lock);
	
	me->first = g_list_remove (me->first, (gconstpointer)item);

	if (gtk_tree_model_get_iter_first (model, &iter))
	  while (gtk_tree_model_iter_next (model, &iter))
	  {
		GObject *citem;

		gtk_tree_model_get (model, &iter, 
			TNY_GTK_ACCOUNT_LIST_MODEL_INSTANCE_COLUMN, 
			&citem, -1);

		if (citem == item)
		{
			gtk_list_store_remove (GTK_LIST_STORE (me), &iter);
			g_object_unref (G_OBJECT (item));
			break;
		}
		g_object_unref (G_OBJECT (citem));
	  }

	g_mutex_unlock (me->iterator_lock);
}

static void 
tny_gtk_account_list_model_foreach_in_the_list (TnyList *self, GFunc func, gpointer user_data)
{
	TnyGtkAccountListModel *me = (TnyGtkAccountListModel*)self;

	/* Foreach item in the list (without using a slower iterator) */

	g_mutex_lock (me->iterator_lock);
	g_list_foreach (me->first, func, user_data);
	g_mutex_unlock (me->iterator_lock);

	return;
}


static TnyList*
tny_gtk_account_list_model_copy_the_list (TnyList *self)
{
	TnyGtkAccountListModel *me = (TnyGtkAccountListModel*)self;
	TnyGtkAccountListModel *copy = g_object_new (TNY_TYPE_GTK_ACCOUNT_LIST_MODEL, NULL);

	g_mutex_lock (me->iterator_lock);
	GList *list_copy = g_list_copy (me->first);
	g_list_foreach (list_copy, (GFunc)g_object_ref, NULL);
	copy->first = list_copy;
	g_mutex_unlock (me->iterator_lock);    

	return TNY_LIST (copy);
}

static void
tny_list_init (TnyListIface *klass)
{
	klass->get_length_func = tny_gtk_account_list_model_get_length;
	klass->prepend_func = tny_gtk_account_list_model_prepend;
	klass->append_func = tny_gtk_account_list_model_append;
	klass->remove_func = tny_gtk_account_list_model_remove;
	klass->create_iterator_func = tny_gtk_account_list_model_create_iterator;
	klass->copy_func = tny_gtk_account_list_model_copy_the_list;
	klass->foreach_func = tny_gtk_account_list_model_foreach_in_the_list;

	return;
}

GType
tny_gtk_account_list_model_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyGtkAccountListModelClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_gtk_account_list_model_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyGtkAccountListModel),
		  0,      /* n_preallocs */
		  tny_gtk_account_list_model_instance_init,    /* instance_init */
		  NULL
		};

		type = g_type_register_static (GTK_TYPE_LIST_STORE, "TnyGtkAccountListModel",
					    &info, 0);


		static const GInterfaceInfo tny_list_info = {
			(GInterfaceInitFunc) tny_list_init,
			NULL,
			NULL
		};

		g_type_add_interface_static (type, TNY_TYPE_LIST,
					     &tny_list_info);

	}

	return type;
}

/**
 * tny_gtk_account_list_model_column_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
GType 
tny_gtk_account_list_model_column_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TNY_GTK_ACCOUNT_LIST_MODEL_NAME_COLUMN, "TNY_GTK_ACCOUNT_LIST_MODEL_NAME_COLUMN", "name" },
      { TNY_GTK_ACCOUNT_LIST_MODEL_INSTANCE_COLUMN, "TNY_GTK_ACCOUNT_LIST_MODEL_INSTANCE_COLUMN", "instance" },
      { TNY_GTK_ACCOUNT_LIST_MODEL_N_COLUMNS, "TNY_GTK_ACCOUNT_LIST_MODEL_N_COLUMNS", "n" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyGtkAccountListModelColumn", values);
  }
  return etype;
}


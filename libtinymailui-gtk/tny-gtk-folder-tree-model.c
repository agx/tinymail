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


/* This implementation is unfinished and highly unstable! */

#include <config.h>

#include <glib.h>
#include <gtk/gtk.h>

#ifdef GNOME
#include <libgnomeui/libgnomeui.h>
#endif

#include <tny-gtk-folder-tree-model.h>
#include <tny-mime-part.h>
#include <tny-iterator.h>
#include <tny-mime-part.h>
#include <tny-folder.h>

#include "tny-gtk-folder-tree-model-iterator-priv.h"


static GObjectClass *parent_class = NULL;


#define TNY_GTK_FOLDER_TREE_MODEL_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_GTK_FOLDER_TREE_MODEL, TnyGtkFolderTreeModelPriv))


/**
 * tny_gtk_folder_tree_model_new:
 *
 *
 * Return value: a new #GtkTreeModel instance suitable for showing  
 * #TnyMimePart instances
 **/
GtkTreeModel*
tny_gtk_folder_tree_model_new (void)
{
	TnyGtkFolderTreeModel *self = g_object_new (TNY_TYPE_GTK_FOLDER_TREE_MODEL, NULL);

	return GTK_TREE_MODEL (self);
}

/*
static void 
destroy_fols (gpointer item, gpointer user_data)
{
    	if (item && G_IS_OBJECT (item))
		g_object_unref (G_OBJECT (item));
    	return;
}
*/

static void
tny_gtk_folder_tree_model_finalize (GObject *object)
{
	TnyGtkFolderTreeModel *me = (TnyGtkFolderTreeModel*) object;

    /*
	g_mutex_lock (me->iterator_lock);
    	if (me->first)
	{
		g_list_foreach (me->first, destroy_fols, NULL);
		g_list_free (me->first); me->first = NULL;
	}
	g_mutex_unlock (me->iterator_lock);
    */
	(*parent_class->finalize) (object);
}

static void
tny_gtk_folder_tree_model_class_init (TnyGtkFolderTreeModelClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_gtk_folder_tree_model_finalize;

	return;
}

static void
tny_gtk_folder_tree_model_instance_init (GTypeInstance *instance, gpointer g_class)
{
	GtkListStore *store = (GtkListStore*) instance;
	TnyGtkFolderTreeModel *me = (TnyGtkFolderTreeModel*) instance;
	static GType types[] = { G_TYPE_STRING, G_TYPE_OBJECT };

	me->iterator_lock = g_mutex_new ();

	gtk_list_store_set_column_types (store, 
		TNY_GTK_FOLDER_TREE_MODEL_N_COLUMNS, types);

	return;
}

static TnyIterator*
tny_gtk_folder_tree_model_create_iterator (TnyList *self)
{
	TnyGtkFolderTreeModel *me = (TnyGtkFolderTreeModel*)self;

	/* Return a new iterator */

	return TNY_ITERATOR (_tny_gtk_folder_tree_model_iterator_new (me));
}



static void
tny_gtk_folder_tree_model_prepend (TnyList *self, GObject* item)
{
	TnyGtkFolderTreeModel *me = (TnyGtkFolderTreeModel*)self;
	GtkTreeStore *store = GTK_TREE_STORE (me);
	GtkTreeIter iter;
	TnyAccount *account = TNY_ACCOUNT (item);
    
	g_mutex_lock (me->iterator_lock);
	gtk_tree_store_prepend (store, &iter, NULL);
	gtk_tree_store_set (store, &iter, 
		TNY_GTK_FOLDER_TREE_MODEL_NAME_COLUMN, tny_account_get_name (account),
		TNY_GTK_FOLDER_TREE_MODEL_INSTANCE_COLUMN, account, -1);    
	g_mutex_unlock (me->iterator_lock);
}

static void
tny_gtk_folder_tree_model_append (TnyList *self, GObject* item)
{
	TnyGtkFolderTreeModel *me = (TnyGtkFolderTreeModel*)self;
	GtkTreeStore *store = GTK_TREE_STORE (me);
	GtkTreeIter iter;
	TnyAccount *account = TNY_ACCOUNT (item);
        
	g_mutex_lock (me->iterator_lock);
	gtk_tree_store_append (store, &iter, NULL);
	gtk_tree_store_set (store, &iter, 
		TNY_GTK_FOLDER_TREE_MODEL_NAME_COLUMN, tny_account_get_name (account),
		TNY_GTK_FOLDER_TREE_MODEL_INSTANCE_COLUMN, account, -1);    
	g_mutex_unlock (me->iterator_lock);
}

static guint
tny_gtk_folder_tree_model_get_length (TnyList *self)
{
	TnyGtkFolderTreeModel *me = (TnyGtkFolderTreeModel*)self;
	guint retval = 0;
	GtkTreeIter iter;
    
	g_mutex_lock (me->iterator_lock);
	gtk_tree_model_get_iter_first (GTK_TREE_MODEL (me), &iter);
	while (gtk_tree_model_iter_next (GTK_TREE_MODEL (me), &iter))
		retval++;
	g_mutex_unlock (me->iterator_lock);

	return retval;
}

static void
tny_gtk_folder_tree_model_remove (TnyList *self, GObject* item)
{
	TnyGtkFolderTreeModel *me = (TnyGtkFolderTreeModel*)self;
	GtkTreeModel *model = GTK_TREE_MODEL (me);
	GtkTreeIter iter;

	g_return_if_fail (G_IS_OBJECT (item));
	g_return_if_fail (G_IS_OBJECT (me));

	/* Remove something from the list */

	g_mutex_lock (me->iterator_lock);
	
	if (gtk_tree_model_get_iter_first (model, &iter))
	  while (gtk_tree_model_iter_next (model, &iter))
	  {
		GObject *citem;
	      
		gtk_tree_model_get (model, &iter, 
			TNY_GTK_FOLDER_TREE_MODEL_INSTANCE_COLUMN, 
			&citem, -1);

		if (citem == item)
		{
			gtk_tree_store_remove (GTK_TREE_STORE (me), &iter);
		    	g_object_unref (G_OBJECT (item));
			break;
		}
		g_object_unref (G_OBJECT (citem));
	  }
    
	g_mutex_unlock (me->iterator_lock);
}


typedef struct 
{
	gpointer user_data;
	GFunc func;
} ForeachHelpr;

static gboolean 
tny_gtk_folder_tree_model_foreach_in_the_list_impl (GtkTreeModel *model,
		GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	ForeachHelpr *dta = (ForeachHelpr*) data;
	TnyAccount *item;

	gtk_tree_model_get (model, iter, 
		TNY_GTK_FOLDER_TREE_MODEL_INSTANCE_COLUMN, &item, -1);
	dta->func (item, dta->user_data);

	g_object_unref (G_OBJECT (item));
	return FALSE;
}
	
static void 
tny_gtk_folder_tree_model_foreach_in_the_list (TnyList *self, GFunc func, gpointer user_data)
{
	TnyGtkFolderTreeModel *me = (TnyGtkFolderTreeModel*)self;
	ForeachHelpr *dta = g_slice_new (ForeachHelpr);

	dta->user_data = user_data;
	dta->func = func;

	/* Foreach item in the list (without using a slower iterator) */

	g_mutex_lock (me->iterator_lock);
	gtk_tree_model_foreach (GTK_TREE_MODEL (me), 
		tny_gtk_folder_tree_model_foreach_in_the_list_impl, dta);
	g_mutex_unlock (me->iterator_lock);

	g_slice_free (ForeachHelpr, dta);

	return;
}


static TnyList*
tny_gtk_folder_tree_model_copy_the_list (TnyList *self)
{
	TnyGtkFolderTreeModel *me = (TnyGtkFolderTreeModel*)self;
	TnyGtkFolderTreeModel *copy = g_object_new (TNY_TYPE_GTK_FOLDER_TREE_MODEL, NULL);

	/* This only copies the TnyList pieces. The result is not a
	   correct or good TnyHeaderListModel. But it will be a correct
	   TnyList instance. It is the only thing the user of this
	   method expects.

	   The new list will point to the same instances, of course. It's
	   only a copy of the list-nodes of course. */

	g_mutex_lock (me->iterator_lock);
	tny_gtk_folder_tree_model_foreach_in_the_list (TNY_LIST (copy), 
			(GFunc)g_object_ref, NULL);
	g_mutex_unlock (me->iterator_lock);

	return TNY_LIST (copy);
}

static void
tny_list_init (TnyListIface *klass)
{
	klass->get_length_func = tny_gtk_folder_tree_model_get_length;
	klass->prepend_func = tny_gtk_folder_tree_model_prepend;
	klass->append_func = tny_gtk_folder_tree_model_append;
	klass->remove_func = tny_gtk_folder_tree_model_remove;
	klass->create_iterator_func = tny_gtk_folder_tree_model_create_iterator;
	klass->copy_func = tny_gtk_folder_tree_model_copy_the_list;
	klass->foreach_func = tny_gtk_folder_tree_model_foreach_in_the_list;

	return;
}

GType
tny_gtk_folder_tree_model_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyGtkFolderTreeModelClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_gtk_folder_tree_model_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyGtkFolderTreeModel),
		  0,      /* n_preallocs */
		  tny_gtk_folder_tree_model_instance_init,    /* instance_init */
		  NULL
		};

		type = g_type_register_static (GTK_TYPE_TREE_STORE, "TnyGtkFolderTreeModel",
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


GType 
tny_gtk_folder_tree_model_column_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TNY_GTK_FOLDER_TREE_MODEL_NAME_COLUMN, "TNY_GTK_FOLDER_TREE_MODEL_NAME_COLUMN", "name" },
      { TNY_GTK_FOLDER_TREE_MODEL_INSTANCE_COLUMN, "TNY_GTK_FOLDER_TREE_MODEL_INSTANCE_COLUMN", "instance" },
      { TNY_GTK_FOLDER_TREE_MODEL_N_COLUMNS, "TNY_GTK_FOLDER_TREE_MODEL_N_COLUMNS", "n" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyGtkFolderTreeModelColumn", values);
  }
  return etype;
}


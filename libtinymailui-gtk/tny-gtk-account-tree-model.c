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

#include <tny-list.h>
#include <tny-iterator.h>
#include <tny-store-account.h>
#include <tny-folder.h>
#include <tny-folder-store.h>
#include <tny-simple-list.h>

#include <tny-gtk-account-tree-model.h>

#include "tny-gtk-account-tree-model-iterator-priv.h"

static GObjectClass *parent_class = NULL;
static void recurse_folders_async (TnyGtkAccountTreeModel *self, TnyFolderStore *store, GtkTreeIter *parent_tree_iter);

typedef void (*treeaddfunc) (GtkTreeStore *tree_store, GtkTreeIter *iter, GtkTreeIter *parent);


typedef struct {
	GtkTreeIter *parent_tree_iter;
	TnyGtkAccountTreeModel *self;
} AsyncHelpr;

static void
recurse_get_folders_callback (TnyFolderStore *self, TnyList *folders, gpointer user_data)
{
	AsyncHelpr *hlrp = user_data;
	TnyIterator *iter = tny_list_create_iterator (folders);

	while (!tny_iterator_is_done (iter))
	{
		GtkTreeStore *model = GTK_TREE_STORE (hlrp->self);
		TnyFolderStore *folder = (TnyFolderStore*) tny_iterator_get_current (iter);
		GtkTreeIter *tree_iter = gtk_tree_iter_copy (hlrp->parent_tree_iter);
	    
		gtk_tree_store_append (model, tree_iter, hlrp->parent_tree_iter);

 		gtk_tree_store_set (model, tree_iter,
			TNY_GTK_ACCOUNT_TREE_MODEL_NAME_COLUMN, 
			tny_folder_get_name (TNY_FOLDER (folder)),
			TNY_GTK_ACCOUNT_TREE_MODEL_UNREAD_COLUMN, 
			tny_folder_get_unread_count (TNY_FOLDER (folder)),
			TNY_GTK_ACCOUNT_TREE_MODEL_TYPE_COLUMN,
			tny_folder_get_folder_type (TNY_FOLDER (folder)),
			TNY_GTK_ACCOUNT_TREE_MODEL_INSTANCE_COLUMN,
			folder, -1);

		recurse_folders_async (hlrp->self, folder, tree_iter);

		g_object_unref (G_OBJECT (folder));

		tny_iterator_next (iter);
	}

	g_object_unref (G_OBJECT (iter));
	g_object_unref (G_OBJECT (folders));

	gtk_tree_iter_free (hlrp->parent_tree_iter);
	g_slice_free (AsyncHelpr, hlrp);

}


static void
recurse_folders_async (TnyGtkAccountTreeModel *self, TnyFolderStore *store, GtkTreeIter *parent_tree_iter)
{
	AsyncHelpr *hlrp = g_slice_new0 (AsyncHelpr);
	TnyList *folders = tny_simple_list_new ();

	hlrp->self = self;
	hlrp->parent_tree_iter = parent_tree_iter;

	tny_folder_store_get_folders_async (store, folders, recurse_get_folders_callback, self->query, hlrp);
}

static void
recurse_folders_sync (TnyGtkAccountTreeModel *self, TnyFolderStore *store, GtkTreeIter *parent_tree_iter)
{
	TnyIterator *iter;
	TnyList *folders = tny_simple_list_new ();

	tny_folder_store_get_folders (store, folders, self->query);
	iter = tny_list_create_iterator (folders);

	while (!tny_iterator_is_done (iter))
	{
		GtkTreeStore *model = GTK_TREE_STORE (self);
		TnyFolderStore *folder = (TnyFolderStore*) tny_iterator_get_current (iter);
		GtkTreeIter tree_iter;
	    
		gtk_tree_store_append (model, &tree_iter, parent_tree_iter);

 		gtk_tree_store_set (model, &tree_iter,
			TNY_GTK_ACCOUNT_TREE_MODEL_NAME_COLUMN, 
			tny_folder_get_name (TNY_FOLDER (folder)),
			TNY_GTK_ACCOUNT_TREE_MODEL_UNREAD_COLUMN, 
			tny_folder_get_unread_count (TNY_FOLDER (folder)),
			TNY_GTK_ACCOUNT_TREE_MODEL_TYPE_COLUMN,
			tny_folder_get_folder_type (TNY_FOLDER (folder)),
			TNY_GTK_ACCOUNT_TREE_MODEL_INSTANCE_COLUMN,
			folder, -1);
	    
		recurse_folders_sync (self, folder, &tree_iter);
	    
 		g_object_unref (G_OBJECT (folder));

		tny_iterator_next (iter);	    
	}

	g_object_unref (G_OBJECT (iter));
	g_object_unref (G_OBJECT (folders));
}


static void
tny_gtk_account_tree_model_add (TnyGtkAccountTreeModel *self, TnyStoreAccount *account, treeaddfunc func)
{
	GtkTreeStore *model = GTK_TREE_STORE (self);
	TnyList *folders = tny_simple_list_new ();
	GtkTreeIter name_iter;
    
	func (model, &name_iter, NULL);

	gtk_tree_store_set (model, &name_iter,
		TNY_GTK_ACCOUNT_TREE_MODEL_NAME_COLUMN, 
		tny_account_get_name (TNY_ACCOUNT (account)),
		TNY_GTK_ACCOUNT_TREE_MODEL_UNREAD_COLUMN, 0,
		TNY_GTK_ACCOUNT_TREE_MODEL_TYPE_COLUMN, TNY_FOLDER_TYPE_ROOT,
		TNY_GTK_ACCOUNT_TREE_MODEL_INSTANCE_COLUMN,
		account, -1);

	recurse_folders_sync (self, TNY_FOLDER_STORE (account), &name_iter);

    	g_object_unref (G_OBJECT (folders));
    
	return;
}

static void
tny_gtk_account_tree_model_add_async (TnyGtkAccountTreeModel *self, TnyStoreAccount *account, treeaddfunc func)
{
	GtkTreeStore *model = GTK_TREE_STORE (self);
	GtkTreeIter first, *name_iter;
	gboolean need_add = TRUE;
    
	if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (self), &first))
	{
		func (model, &first, NULL);
		need_add = FALSE;
	}

	name_iter = gtk_tree_iter_copy (&first);

	if (need_add)
		func (model, name_iter, NULL);

	gtk_tree_store_set (model, name_iter,
		TNY_GTK_ACCOUNT_TREE_MODEL_NAME_COLUMN, 
		tny_account_get_name (TNY_ACCOUNT (account)),
		TNY_GTK_ACCOUNT_TREE_MODEL_UNREAD_COLUMN, 0,
		TNY_GTK_ACCOUNT_TREE_MODEL_TYPE_COLUMN, TNY_FOLDER_TYPE_ROOT,
		TNY_GTK_ACCOUNT_TREE_MODEL_INSTANCE_COLUMN,
		account, -1);

	recurse_folders_async (self, TNY_FOLDER_STORE (account), name_iter);
    
	return;
}

/**
 * tny_gtk_account_tree_model_new:
 * @async: Whether or not this component should attempt to asynchronously fill the tree
 * @query: the #TnyFolderStoreQuery that will be used to retrieve the
 * folders of each account
 *
 * Return value: a new #GtkTreeModel instance suitable for showing  
 * #TnyAccount instances
 **/
GtkTreeModel*
tny_gtk_account_tree_model_new (gboolean async, TnyFolderStoreQuery *query)
{
	TnyGtkAccountTreeModel *self = g_object_new (TNY_TYPE_GTK_ACCOUNT_TREE_MODEL, NULL);
	self->is_async = async;
	if (query) self->query = g_object_ref (G_OBJECT (query));
	return GTK_TREE_MODEL (self);
}

static void 
destroy_accounts (gpointer item, gpointer user_data)
{
    	if (item && G_IS_OBJECT (item))
		g_object_unref (G_OBJECT (item));
    	return;
}

static void
tny_gtk_account_tree_model_finalize (GObject *object)
{
	TnyGtkAccountTreeModel *me = (TnyGtkAccountTreeModel*) object;

	g_mutex_lock (me->iterator_lock);
    	if (me->first)
	{
		g_list_foreach (me->first, destroy_accounts, NULL);
		g_list_free (me->first); me->first = NULL;
	}
	g_mutex_unlock (me->iterator_lock);
    
	g_mutex_free (me->iterator_lock);
	me->iterator_lock = NULL;

	if (me->query)
		g_object_unref (G_OBJECT (me->query));

	(*parent_class->finalize) (object);
}

static void
tny_gtk_account_tree_model_class_init (TnyGtkAccountTreeModelClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_gtk_account_tree_model_finalize;

	return;
}

static void
tny_gtk_account_tree_model_instance_init (GTypeInstance *instance, gpointer g_class)
{
	GtkTreeStore *store = (GtkTreeStore*) instance;
	TnyGtkAccountTreeModel *me = (TnyGtkAccountTreeModel*) instance;
	static GType types[] = { G_TYPE_STRING, G_TYPE_UINT, G_TYPE_INT, G_TYPE_OBJECT };

	me->iterator_lock = g_mutex_new ();

	gtk_tree_store_set_column_types (store, 
		TNY_GTK_ACCOUNT_TREE_MODEL_N_COLUMNS, types);

	return;
}



static TnyIterator*
tny_gtk_account_tree_model_create_iterator (TnyList *self)
{
	TnyGtkAccountTreeModel *me = (TnyGtkAccountTreeModel*)self;

	/* Return a new iterator */

	return TNY_ITERATOR (_tny_gtk_account_tree_model_iterator_new (me));
}



static void
tny_gtk_account_tree_model_prepend (TnyList *self, GObject* item)
{
	TnyGtkAccountTreeModel *me = (TnyGtkAccountTreeModel*)self;

	g_mutex_lock (me->iterator_lock);

	/* Prepend something to the list */
	g_object_ref (G_OBJECT (item));
	me->first = g_list_prepend (me->first, item);
    
    	if (me->is_async)
		tny_gtk_account_tree_model_add_async (me, TNY_STORE_ACCOUNT (item), 
			gtk_tree_store_prepend);
	else
		tny_gtk_account_tree_model_add (me, TNY_STORE_ACCOUNT (item), 
			gtk_tree_store_prepend);
	
	g_mutex_unlock (me->iterator_lock);
}

static void
tny_gtk_account_tree_model_append (TnyList *self, GObject* item)
{
	TnyGtkAccountTreeModel *me = (TnyGtkAccountTreeModel*)self;

	g_mutex_lock (me->iterator_lock);

	/* Append something to the list */
	g_object_ref (G_OBJECT (item));
	me->first = g_list_append (me->first, item);
    
	if (me->is_async)
		tny_gtk_account_tree_model_add_async (me, TNY_STORE_ACCOUNT (item), 
			gtk_tree_store_append);
	else
		tny_gtk_account_tree_model_add (me, TNY_STORE_ACCOUNT (item), 
			gtk_tree_store_append);
    
	g_mutex_unlock (me->iterator_lock);
}

static guint
tny_gtk_account_tree_model_get_length (TnyList *self)
{
	TnyGtkAccountTreeModel *me = (TnyGtkAccountTreeModel*)self;
	guint retval = 0;

	g_mutex_lock (me->iterator_lock);

	retval = me->first?g_list_length (me->first):0;

	g_mutex_unlock (me->iterator_lock);

	return retval;
}

static void
tny_gtk_account_tree_model_remove (TnyList *self, GObject* item)
{
	TnyGtkAccountTreeModel *me = (TnyGtkAccountTreeModel*)self;
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
			TNY_GTK_ACCOUNT_TREE_MODEL_INSTANCE_COLUMN, 
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


static TnyList*
tny_gtk_account_tree_model_copy_the_list (TnyList *self)
{
	TnyGtkAccountTreeModel *me = (TnyGtkAccountTreeModel*)self;
	TnyGtkAccountTreeModel *copy = g_object_new (TNY_TYPE_GTK_ACCOUNT_TREE_MODEL, NULL);

	/* This only copies the TnyList pieces. The result is not a
	   correct or good TnyHeaderListModel. But it will be a correct
	   TnyList instance. It is the only thing the user of this
	   method expects.

	   The new list will point to the same instances, of course. It's
	   only a copy of the list-nodes of course. */

	g_mutex_lock (me->iterator_lock);
	GList *list_copy = g_list_copy (me->first);
	g_list_foreach (list_copy, (GFunc)g_object_ref, NULL);
	copy->first = list_copy;
	g_mutex_unlock (me->iterator_lock);

	return TNY_LIST (copy);
}

static void 
tny_gtk_account_tree_model_foreach_in_the_list (TnyList *self, GFunc func, gpointer user_data)
{
	TnyGtkAccountTreeModel *me = (TnyGtkAccountTreeModel*)self;

	/* Foreach item in the list (without using a slower iterator) */

	g_mutex_lock (me->iterator_lock);
	g_list_foreach (me->first, func, user_data);
	g_mutex_unlock (me->iterator_lock);

	return;
}


static void
tny_list_init (TnyListIface *klass)
{
	klass->get_length_func = tny_gtk_account_tree_model_get_length;
	klass->prepend_func = tny_gtk_account_tree_model_prepend;
	klass->append_func = tny_gtk_account_tree_model_append;
	klass->remove_func = tny_gtk_account_tree_model_remove;
	klass->create_iterator_func = tny_gtk_account_tree_model_create_iterator;
	klass->copy_func = tny_gtk_account_tree_model_copy_the_list;
	klass->foreach_func = tny_gtk_account_tree_model_foreach_in_the_list;

	return;
}

GType
tny_gtk_account_tree_model_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyGtkAccountTreeModelClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_gtk_account_tree_model_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyGtkAccountTreeModel),
		  0,      /* n_preallocs */
		  tny_gtk_account_tree_model_instance_init    /* instance_init */
		};

		type = g_type_register_static (GTK_TYPE_TREE_STORE, "TnyGtkAccountTreeModel",
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
tny_gtk_account_tree_model_column_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TNY_GTK_ACCOUNT_TREE_MODEL_NAME_COLUMN, "TNY_GTK_ACCOUNT_TREE_MODEL_NAME_COLUMN", "name" },
      { TNY_GTK_ACCOUNT_TREE_MODEL_UNREAD_COLUMN, "TNY_GTK_ACCOUNT_TREE_MODEL_UNREAD_COLUMN", "unread" },
      { TNY_GTK_ACCOUNT_TREE_MODEL_TYPE_COLUMN, "TNY_GTK_ACCOUNT_TREE_MODEL_TYPE_COLUMN", "type" },
      { TNY_GTK_ACCOUNT_TREE_MODEL_INSTANCE_COLUMN, "TNY_GTK_ACCOUNT_TREE_MODEL_INSTANCE_COLUMN", "instance" },
      { TNY_GTK_ACCOUNT_TREE_MODEL_N_COLUMNS, "TNY_GTK_ACCOUNT_TREE_MODEL_N_COLUMNS", "n" },
      { 0, NULL, NULL }
     };
    etype = g_enum_register_static ("TnyGtkAccountTreeModelColumn", values);
  }
  return etype;
}


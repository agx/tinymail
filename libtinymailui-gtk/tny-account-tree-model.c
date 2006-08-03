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

#include <tny-list-iface.h>
#include <tny-iterator-iface.h>
#include <tny-account-tree-model.h>
#include <tny-store-account-iface.h>
#include <tny-msg-folder-iface.h>

#include "tny-account-tree-model-priv.h"

static GObjectClass *parent_class = NULL;

typedef void (*treeaddfunc) (GtkTreeStore *tree_store, GtkTreeIter *iter, GtkTreeIter *parent);

static void
fill_treemodel_recursive (TnyAccountTreeModel *self, TnyListIface *folders, GtkTreeIter *parent_iter, TnyStoreAccountIface *account)
{

  if (folders && tny_list_iface_length (folders) > 0)
  {
	GtkTreeStore *model = GTK_TREE_STORE (self);
	TnyIteratorIface *iterator = tny_list_iface_create_iterator (folders);
	gboolean next = tny_iterator_iface_has_first (iterator);

	while (next)
	{
		GtkTreeIter iter;
		TnyMsgFolderIface *folder = (TnyMsgFolderIface*)tny_iterator_iface_current (iterator);
		TnyListIface *more_folders;

		more_folders = (TnyListIface*)tny_msg_folder_iface_get_folders (folder);

		gtk_tree_store_append (model, &iter, parent_iter);

 		gtk_tree_store_set (model, &iter,
			TNY_ACCOUNT_TREE_MODEL_NAME_COLUMN, 
			tny_msg_folder_iface_get_name (folder),
			TNY_ACCOUNT_TREE_MODEL_UNREAD_COLUMN, 
			tny_msg_folder_iface_get_unread_count (folder),
			TNY_ACCOUNT_TREE_MODEL_TYPE_COLUMN,
			tny_msg_folder_iface_get_folder_type (folder),
			TNY_ACCOUNT_TREE_MODEL_INSTANCE_COLUMN,
			folder, -1);

		/* TODO: Observe the FOLDERS_RELOADED signal and add 
		   to the model if a subfolder got added or subscribed */

		/* g_signal_connect (G_OBJECT (folder), "folders_reloaded",
			G_CALLBACK (folders_reloaded), self+account); */

		tny_msg_folder_iface_uncache (folder);

		if (tny_list_iface_length (more_folders) > 0)
			fill_treemodel_recursive (self, more_folders, &iter, account);

		next = tny_iterator_iface_has_next (iterator);

		if (next)
			tny_iterator_iface_next (iterator);
	}

	g_object_unref (G_OBJECT (iterator));
  }
}


static void
tny_account_tree_model_add (TnyAccountTreeModel *self, TnyStoreAccountIface *account, treeaddfunc func)
{
	GtkTreeStore *model = GTK_TREE_STORE (self);
	TnyListIface *folders;
	GtkTreeIter name_iter;

	folders = tny_store_account_iface_get_folders (account, 
		TNY_STORE_ACCOUNT_FOLDER_TYPE_SUBSCRIBED);

	func (model, &name_iter, NULL);

	gtk_tree_store_set (model, &name_iter,
		TNY_ACCOUNT_TREE_MODEL_NAME_COLUMN, 
		tny_account_iface_get_name (TNY_ACCOUNT_IFACE (account)),
		TNY_ACCOUNT_TREE_MODEL_UNREAD_COLUMN, 0,
		TNY_ACCOUNT_TREE_MODEL_TYPE_COLUMN, -1,
		TNY_ACCOUNT_TREE_MODEL_INSTANCE_COLUMN,
		NULL, -1);

	fill_treemodel_recursive (self, (TnyListIface*)folders, 
		&name_iter, account);

	return;
}

/**
 * tny_account_tree_model_new:
 *
 *
 * Return value: a new #GtkTreeModel instance suitable for showing  
 * #TnyAccountIface instances
 **/
TnyAccountTreeModel*
tny_account_tree_model_new (void)
{
	TnyAccountTreeModel *self = g_object_new (TNY_TYPE_ACCOUNT_TREE_MODEL, NULL);

	return self;
}

static void
tny_account_tree_model_finalize (GObject *object)
{
	TnyAccountTreeModel *me = (TnyAccountTreeModel*) object;

	g_mutex_free (me->iterator_lock);
	me->iterator_lock = NULL;

	(*parent_class->finalize) (object);
}

static void
tny_account_tree_model_class_init (TnyAccountTreeModelClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_account_tree_model_finalize;

	return;
}

static void
tny_account_tree_model_instance_init (GTypeInstance *instance, gpointer g_class)
{
	GtkTreeStore *store = (GtkTreeStore*) instance;
	TnyAccountTreeModel *me = (TnyAccountTreeModel*) instance;
	static GType types[] = { G_TYPE_STRING, G_TYPE_UINT, G_TYPE_INT, G_TYPE_OBJECT };

	me->iterator_lock = g_mutex_new ();

	gtk_tree_store_set_column_types (store, 
		TNY_ACCOUNT_TREE_MODEL_N_COLUMNS, types);

	return;
}



static TnyIteratorIface*
tny_account_tree_model_create_iterator (TnyListIface *self)
{
	TnyAccountTreeModel *me = (TnyAccountTreeModel*)self;

	/* Return a new iterator */

	return TNY_ITERATOR_IFACE (_tny_account_tree_model_iterator_new (me));
}



static void
tny_account_tree_model_prepend (TnyListIface *self, GObject* item)
{
	TnyAccountTreeModel *me = (TnyAccountTreeModel*)self;

	g_mutex_lock (me->iterator_lock);

	/* Prepend something to the list */
	g_object_ref (G_OBJECT (item));
	me->first = g_list_prepend (me->first, item);
	tny_account_tree_model_add (me, TNY_STORE_ACCOUNT_IFACE (item), 
		gtk_tree_store_prepend);

	g_mutex_unlock (me->iterator_lock);
}

static void
tny_account_tree_model_append (TnyListIface *self, GObject* item)
{
	TnyAccountTreeModel *me = (TnyAccountTreeModel*)self;

	g_mutex_lock (me->iterator_lock);

	/* Append something to the list */
	g_object_ref (G_OBJECT (item));
	me->first = g_list_append (me->first, item);
	tny_account_tree_model_add (me, TNY_STORE_ACCOUNT_IFACE (item), 
		gtk_tree_store_append);

	g_mutex_unlock (me->iterator_lock);
}

static guint
tny_account_tree_model_length (TnyListIface *self)
{
	TnyAccountTreeModel *me = (TnyAccountTreeModel*)self;
	guint retval = 0;

	g_mutex_lock (me->iterator_lock);

	retval = me->first?g_list_length (me->first):0;

	g_mutex_unlock (me->iterator_lock);

	return retval;
}

static void
tny_account_tree_model_remove (TnyListIface *self, GObject* item)
{
	TnyAccountTreeModel *me = (TnyAccountTreeModel*)self;
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
		TnyAccountIface *curaccount;

		gtk_tree_model_get (model, &iter, 
			TNY_ACCOUNT_TREE_MODEL_INSTANCE_COLUMN, 
			&curaccount, -1);

		if (curaccount == (TnyAccountIface*)item)
		{
			gtk_tree_store_remove (GTK_TREE_STORE (me), &iter);
			g_object_unref (G_OBJECT (item));

			break;
		}
	}

	g_mutex_unlock (me->iterator_lock);
}


static TnyListIface*
tny_account_tree_model_copy_the_list (TnyListIface *self)
{
	TnyAccountTreeModel *me = (TnyAccountTreeModel*)self;
	TnyAccountTreeModel *copy = g_object_new (TNY_TYPE_ACCOUNT_TREE_MODEL, NULL);

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
tny_account_tree_model_foreach_in_the_list (TnyListIface *self, GFunc func, gpointer user_data)
{
	TnyAccountTreeModel *me = (TnyAccountTreeModel*)self;

	/* Foreach item in the list (without using a slower iterator) */

	g_mutex_lock (me->iterator_lock);
	g_list_foreach (me->first, func, user_data);
	g_mutex_unlock (me->iterator_lock);

	return;
}


static void
tny_list_iface_init (TnyListIfaceClass *klass)
{
	klass->length_func = tny_account_tree_model_length;
	klass->prepend_func = tny_account_tree_model_prepend;
	klass->append_func = tny_account_tree_model_append;
	klass->remove_func = tny_account_tree_model_remove;
	klass->create_iterator_func = tny_account_tree_model_create_iterator;
	klass->copy_func = tny_account_tree_model_copy_the_list;
	klass->foreach_func = tny_account_tree_model_foreach_in_the_list;

	return;
}

GType
tny_account_tree_model_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyAccountTreeModelClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_account_tree_model_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyAccountTreeModel),
		  0,      /* n_preallocs */
		  tny_account_tree_model_instance_init    /* instance_init */
		};

		type = g_type_register_static (GTK_TYPE_TREE_STORE, "TnyAccountTreeModel",
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
tny_account_tree_model_column_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TNY_ACCOUNT_TREE_MODEL_NAME_COLUMN, "TNY_ACCOUNT_TREE_MODEL_NAME_COLUMN", "name" },
      { TNY_ACCOUNT_TREE_MODEL_UNREAD_COLUMN, "TNY_ACCOUNT_TREE_MODEL_UNREAD_COLUMN", "unread" },
      { TNY_ACCOUNT_TREE_MODEL_TYPE_COLUMN, "TNY_ACCOUNT_TREE_MODEL_TYPE_COLUMN", "type" },
      { TNY_ACCOUNT_TREE_MODEL_INSTANCE_COLUMN, "TNY_ACCOUNT_TREE_MODEL_INSTANCE_COLUMN", "instance" },
      { TNY_ACCOUNT_TREE_MODEL_N_COLUMNS, "TNY_ACCOUNT_TREE_MODEL_N_COLUMNS", "n" },
      { 0, NULL, NULL }
     };
    etype = g_enum_register_static ("TnyAccountTreeModelColumn", values);
  }
  return etype;
}

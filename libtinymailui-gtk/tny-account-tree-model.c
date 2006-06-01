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

static GObjectClass *parent_class = NULL;

/*
static void 
folders_reloaded (gpointer user_data)
{
	TnyAccountTreeModel *self = user_data;

	return;
}
*/

static void
fill_treemodel_recursive (TnyAccountTreeModel *self, TnyListIface *folders, GtkTreeIter *parent_iter, TnyStoreAccountIface *account)
{

  if (tny_list_iface_length (folders) > 0)
  {
	GtkTreeStore *model = GTK_TREE_STORE (self);
	TnyIteratorIface *iterator = tny_list_iface_create_iterator (folders);
	gboolean next = TRUE;

	while (next)
	{
		GtkTreeIter iter;
		TnyMsgFolderIface *folder = tny_iterator_iface_current (iterator);
		TnyListIface *more_folders = (TnyListIface*)tny_msg_folder_iface_get_folders (folder);

		gtk_tree_store_append (model, &iter, parent_iter);

		gtk_tree_store_set (model, &iter,
			TNY_ACCOUNT_TREE_MODEL_NAME_COLUMN, 
			tny_msg_folder_iface_get_name (folder),
			TNY_ACCOUNT_TREE_MODEL_UNREAD_COLUMN, 
			tny_msg_folder_iface_get_unread_count (folder),
			TNY_ACCOUNT_TREE_MODEL_TYPE_COLUMN, 0, /* TODO */
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

/**
 * tny_account_tree_model_add:
 * @self: A #TnyAccountTreeModel instance
 * @account: A #TnyAccountIface instance
 *
 * Add an account to the list model
 *
 **/
void
tny_account_tree_model_add (TnyAccountTreeModel *self, TnyStoreAccountIface *account)
{
	GtkTreeStore *model = GTK_TREE_STORE (self);
	const TnyListIface *folders;
	GtkTreeIter name_iter;

	folders = tny_store_account_iface_get_folders (account, 
		TNY_STORE_ACCOUNT_FOLDER_TYPE_SUBSCRIBED);

	gtk_tree_store_append (model, &name_iter, NULL);

	gtk_tree_store_set (model, &name_iter,
		TNY_ACCOUNT_TREE_MODEL_NAME_COLUMN, 
		tny_account_iface_get_name (TNY_ACCOUNT_IFACE (account)),
		TNY_ACCOUNT_TREE_MODEL_UNREAD_COLUMN, 0,
		TNY_ACCOUNT_TREE_MODEL_TYPE_COLUMN, -1, /* TODO */
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
	static GType types[] = { G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_POINTER };

	gtk_tree_store_set_column_types (store, 
		TNY_ACCOUNT_TREE_MODEL_N_COLUMNS, types);

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
	}

	return type;
}

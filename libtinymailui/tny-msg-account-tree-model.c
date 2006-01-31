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

#include <tny-msg-account-tree-model.h>
#include <tny-msg-account-iface.h>
#include <tny-msg-folder-iface.h>

static GObjectClass *parent_class = NULL;


static void
fill_treemodel_recursive (TnyMsgAccountTreeModel *self, const GList *folders, GtkTreeIter *parent_iter)
{
	GtkTreeStore *model = GTK_TREE_STORE (self);

	while (folders)
	{
		GtkTreeIter iter;
		TnyMsgFolderIface *folder = folders->data;
		const GList *more_folders = tny_msg_folder_iface_get_folders (folder);

		gtk_tree_store_append (model, &iter, parent_iter);

		gtk_tree_store_set (model, &iter,
			TNY_MSG_ACCOUNT_TREE_MODEL_NAME_COLUMN, 
			tny_msg_folder_iface_get_name (folder),
			TNY_MSG_ACCOUNT_TREE_MODEL_UNREAD_COLUMN, 0,
			TNY_MSG_ACCOUNT_TREE_MODEL_INSTANCE_COLUMN,
			folder, -1);

		if (more_folders && g_list_length ((GList*)more_folders) > 0)
			fill_treemodel_recursive (self, more_folders, &iter);

		folders = g_list_next (folders);
	}

}

void
tny_msg_account_tree_model_add (TnyMsgAccountTreeModel *self, TnyMsgAccountIface *account)
{
	GtkTreeStore *model = GTK_TREE_STORE (self);
	const GList *folders = tny_msg_account_iface_get_folders (account);

	fill_treemodel_recursive (self, folders, NULL);

	return;
}

TnyMsgAccountTreeModel*
tny_msg_account_tree_model_new (void)
{
	TnyMsgAccountTreeModel *self = g_object_new (TNY_MSG_ACCOUNT_TREE_MODEL_TYPE, NULL);

	return self;
}

static void
tny_msg_account_tree_model_finalize (GObject *object)
{
	(*parent_class->finalize) (object);
}

static void
tny_msg_account_tree_model_class_init (TnyMsgAccountTreeModelClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_msg_account_tree_model_finalize;

	/* g_type_class_add_private (object_class, sizeof (TnyMsgAccountTreeModelPriv)); */

	return;
}

static void
tny_msg_account_tree_model_instance_init (GTypeInstance *instance, gpointer g_class)
{
	GtkTreeStore *store = (GtkTreeStore*) instance;
	static GType types[] = { G_TYPE_STRING, G_TYPE_INT, G_TYPE_POINTER };

	gtk_tree_store_set_column_types (store, 
		TNY_MSG_ACCOUNT_TREE_MODEL_N_COLUMNS, types);

	return;
}


GType
tny_msg_account_tree_model_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgAccountTreeModelClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_msg_account_tree_model_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMsgAccountTreeModel),
		  0,      /* n_preallocs */
		  tny_msg_account_tree_model_instance_init    /* instance_init */
		};

		type = g_type_register_static (GTK_TYPE_TREE_STORE, "TnyMsgAccountTreeModel",
					    &info, 0);
	}

	return type;
}

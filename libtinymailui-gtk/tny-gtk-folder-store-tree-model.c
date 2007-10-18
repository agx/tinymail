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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <config.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>

#include <tny-list.h>
#include <tny-iterator.h>
#include <tny-folder.h>
#include <tny-folder-store.h>
#include <tny-simple-list.h>
#include <tny-merge-folder.h>

#include <tny-store-account.h>
#include <tny-folder-store-change.h>
#include <tny-folder-store-observer.h>
#include <tny-folder-change.h>
#include <tny-folder-observer.h>

#include <tny-gtk-folder-store-tree-model.h>

#include "tny-gtk-folder-store-tree-model-iterator-priv.h"

static GObjectClass *parent_class = NULL;

typedef void (*treeaddfunc) (GtkTreeStore *tree_store, GtkTreeIter *iter, GtkTreeIter *parent);

static void
recurse_folders_sync (TnyGtkFolderStoreTreeModel *self, TnyFolderStore *store, GtkTreeIter *parent_tree_iter)
{
	TnyIterator *iter;
	TnyList *folders = tny_simple_list_new ();
	TnyGtkFolderStoreTreeModel *me = (TnyGtkFolderStoreTreeModel*) self;

	/* TODO add error checking and reporting here */
	tny_folder_store_get_folders (store, folders, self->query, NULL);
	iter = tny_list_create_iterator (folders);

	while (!tny_iterator_is_done (iter))
	{
		GtkTreeModel *mmodel = (GtkTreeModel *) self;
		GtkTreeStore *model = GTK_TREE_STORE (self);
		GObject *instance = G_OBJECT (tny_iterator_get_current (iter));
		GtkTreeIter tree_iter;
		TnyFolder *folder = NULL;
		TnyFolderStore *folder_store = NULL;
		GtkTreeIter miter;
		gboolean found = FALSE;
		GObject *mark_for_removal = NULL;

		if (instance && (TNY_IS_FOLDER (instance) || TNY_IS_MERGE_FOLDER (instance)))
			folder = TNY_FOLDER (instance);

		if (instance && (TNY_IS_FOLDER_STORE (instance)))
			folder_store = TNY_FOLDER_STORE (instance);

		/* We check whether we have this folder already in the tree, or 
		 * whether it's a brand new one. If it's a new one, we'll add
		 * it, of course */

		if (gtk_tree_model_iter_children (mmodel, &miter, parent_tree_iter))
		  do
		  {
			GObject *citem = NULL;
			TnyIterator *niter = NULL;
			
			gtk_tree_model_get (mmodel, &miter, 
				TNY_GTK_FOLDER_STORE_TREE_MODEL_INSTANCE_COLUMN, 
				&citem, -1);

			if (citem == instance)
			{
				found = TRUE;
				break;
			}
			if (citem)
				g_object_unref (citem);

			/* We search whether this folder that we have in the 
			 * model, still exists in the actual list. Because if
			 * not, it probably got removed remotely (and we need
			 * to get rid of it in the model now) */

			niter = tny_list_create_iterator (folders);
			while (!tny_iterator_is_done (niter))
			{
				TnyFolder *ifound = TNY_FOLDER (tny_iterator_get_current (niter));
				if (citem == (GObject *) ifound) {
					if (mark_for_removal)
						g_object_unref (mark_for_removal);
					mark_for_removal = g_object_ref (ifound);
				}
				g_object_unref (ifound);
				tny_iterator_next (niter);
			}
			g_object_unref (niter);

		  } while (gtk_tree_model_iter_next (mmodel, &miter));

		/* It was not found, so let's start adding it to the model! */

		if (!found)
		{
			gtk_tree_store_append (model, &tree_iter, parent_tree_iter);

			/* Making self both a folder-store as a folder observer
			 * of this folder. This way we'll get notified when 
			 * both a removal and a creation happens. Also when a 
			 * rename happens: that's a removal and a creation. */

			if (folder) {
				tny_folder_add_observer (folder, TNY_FOLDER_OBSERVER (self));
				me->folder_observables = g_list_prepend (me->folder_observables, folder);
			}

			if (folder_store) {
				tny_folder_store_add_observer (folder_store, TNY_FOLDER_STORE_OBSERVER (self));
				me->store_observables = g_list_prepend (me->store_observables, folder_store);
			}


			/* This adds a reference count to folder too. When it gets removed, that
			   reference count is decreased automatically by the gtktreestore infra-
			   structure. */

			if (folder)
			{
				TnyFolder *folder = TNY_FOLDER (instance);
				
				gtk_tree_store_set (model, &tree_iter,
					TNY_GTK_FOLDER_STORE_TREE_MODEL_NAME_COLUMN, 
					tny_folder_get_name (TNY_FOLDER (folder)),
					TNY_GTK_FOLDER_STORE_TREE_MODEL_UNREAD_COLUMN, 
					tny_folder_get_unread_count (TNY_FOLDER (folder)),
					TNY_GTK_FOLDER_STORE_TREE_MODEL_ALL_COLUMN, 
					tny_folder_get_all_count (TNY_FOLDER (folder)),
					TNY_GTK_FOLDER_STORE_TREE_MODEL_TYPE_COLUMN,
					tny_folder_get_folder_type (TNY_FOLDER (folder)),
					TNY_GTK_FOLDER_STORE_TREE_MODEL_INSTANCE_COLUMN,
					folder, -1);
			}

			/* it's a store by itself, so keep on recursing */
			if (folder_store)
				recurse_folders_sync (self, folder_store, &tree_iter);

			/* We're a folder, we'll request a status, since we've
			 * set self to be a folder observers of folder, we'll 
			 * receive the status update. This makes it possible to
			 * instantly get the unread and total counts, of course.
			 * 
			 * Note that the initial value is read from the cache in
			 * case the account is set offline, in TnyCamelFolder. 
			 * In case the account is online a STAT for POP or a 
			 * STATUS for IMAP is asked during the poke-status impl.
			 *
			 * This means that no priv->folder must be loaded, no
			 * memory peak will happen, few data must be transmitted
			 * in case we're online. Which is perfect! */

			if (folder)
				tny_folder_poke_status (TNY_FOLDER (folder));

			if (mark_for_removal)
				g_object_unref (mark_for_removal);

		} else {
			if (mark_for_removal) {
				printf ("We need to remove %s\n", 
					tny_folder_get_id (TNY_FOLDER (mark_for_removal)));
				g_object_unref (mark_for_removal);
			}
		}

		g_object_unref (instance);

		tny_iterator_next (iter);
	}

	g_object_unref (iter);
	g_object_unref (folders);
}


static const gchar*
get_root_name (TnyFolderStore *folder_store)
{
	const gchar *root_name;
	if (TNY_IS_ACCOUNT (folder_store))
		root_name = tny_account_get_name (TNY_ACCOUNT (folder_store));
	else
		root_name = _("Folder bag");
	return root_name;
}


static void
get_folders_cb (TnyFolderStore *fstore, gboolean cancelled, TnyList *list, GError *err, gpointer user_data)
{
	TnyGtkFolderStoreTreeModel *self = (TnyGtkFolderStoreTreeModel *) user_data;
	GtkTreeModel *model = GTK_TREE_MODEL (self);
	GtkTreeIter iter;
	GtkTreeIter name_iter;
	gboolean found = FALSE;

	g_object_unref (list);

	/* Note that the very first time, this code will pull all folder info.
	 * Note that when it runs, you'll see LIST commands happen on the IMAP
	 * socket. Especially the first time. You'll also see STATUS commands 
	 * happen. This is, indeed, normal behaviour when this component is used.*/

	/* But first, let's find-back that damn account so that we have the
	 * actual iter behind it in the model. */

	if (gtk_tree_model_get_iter_first (model, &iter))
	  do 
	  {
		GObject *citem;

		gtk_tree_model_get (model, &iter, 
			TNY_GTK_FOLDER_STORE_TREE_MODEL_INSTANCE_COLUMN, 
			&citem, -1);
		if (citem == (GObject *) fstore)
		{
			name_iter = iter;
			found = TRUE;
			break;
		}
		g_object_unref (G_OBJECT (citem));

	  } while (gtk_tree_model_iter_next (model, &iter));

	/* We found it, so we'll now just recursively start asking for all its
	 * folders and subfolders. The recurse_folders_sync can indeed cope with
	 * folders that already exist (it wont add them a second time). */

	if (found)
		recurse_folders_sync (self, fstore, &name_iter);

	g_object_unref (self);

	return;
}

static void 
tny_gtk_folder_store_tree_model_on_constatus_changed (TnyAccount *account, TnyConnectionStatus status, TnyGtkFolderStoreTreeModel *self)
{
	TnyList *list = NULL;

	/* This callback handler deals with connection status changes. In case
	 * we got connected, we can expect that, at least sometimes, new folders
	 * might have arrived. We'll need to scan for those, so we'll recursively
	 * start asking the account about its folders. */

	if (status == TNY_CONNECTION_STATUS_RECONNECTING)
		return;

	list = tny_simple_list_new ();
	tny_folder_store_get_folders_async (TNY_FOLDER_STORE (account), 
		list, get_folders_cb, NULL, NULL, g_object_ref (self));

	return;
}



static void 
tny_gtk_folder_store_tree_model_on_changed (TnyAccount *account, TnyGtkFolderStoreTreeModel *self)
{
	GtkTreeModel *model = GTK_TREE_MODEL (self);
	GtkTreeIter iter;
	GtkTreeIter name_iter;
	gboolean found = FALSE;

	if (gtk_tree_model_get_iter_first (model, &iter))
	  do 
	  {
		GObject *citem;

		gtk_tree_model_get (model, &iter, 
			TNY_GTK_FOLDER_STORE_TREE_MODEL_INSTANCE_COLUMN, 
			&citem, -1);
		if (citem == (GObject *) account)
		{
			name_iter = iter;
			found = TRUE;
			break;
		}
		g_object_unref (citem);

	  } while (gtk_tree_model_iter_next (model, &iter));

	if (found) {
		gtk_tree_store_set (GTK_TREE_STORE (model), &name_iter,
			TNY_GTK_FOLDER_STORE_TREE_MODEL_NAME_COLUMN, 
			get_root_name (TNY_FOLDER_STORE (account)), -1);
	}
}

typedef struct
{
	TnyGtkFolderStoreTreeModel *self;
	TnyAccount *account;
} AccNotYetReadyInfo;

static gboolean
account_was_not_yet_ready_idle (gpointer user_data)
{
	AccNotYetReadyInfo *info = (AccNotYetReadyInfo *) user_data;
	gboolean repeat = TRUE;

	if (tny_account_is_ready (info->account))
	{
		g_signal_connect (info->account, "connection-status-changed",
			G_CALLBACK (tny_gtk_folder_store_tree_model_on_constatus_changed), info->self);

		g_signal_connect (info->account, "changed",
			G_CALLBACK (tny_gtk_folder_store_tree_model_on_changed), info->self);

		tny_gtk_folder_store_tree_model_on_constatus_changed (info->account, 
			tny_account_get_connection_status (info->account), info->self);

		tny_gtk_folder_store_tree_model_on_changed (info->account, 
			info->self);

		repeat = FALSE;
	}

	return repeat;
}

static void 
account_was_not_yet_ready_destroy (gpointer user_data)
{
	AccNotYetReadyInfo *info = (AccNotYetReadyInfo *) user_data;

	g_object_unref (info->account);
	g_object_unref (info->self);

	g_slice_free (AccNotYetReadyInfo, user_data);
}

static void
tny_gtk_folder_store_tree_model_add_i (TnyGtkFolderStoreTreeModel *self, TnyFolderStore *folder_store, treeaddfunc func, const gchar *root_name)
{
	GtkTreeStore *model = GTK_TREE_STORE (self);
	TnyList *folders = tny_simple_list_new ();
	GtkTreeIter name_iter;

	func (model, &name_iter, NULL);

	gtk_tree_store_set (model, &name_iter,
		TNY_GTK_FOLDER_STORE_TREE_MODEL_NAME_COLUMN, root_name,
		TNY_GTK_FOLDER_STORE_TREE_MODEL_UNREAD_COLUMN, 0,
		TNY_GTK_FOLDER_STORE_TREE_MODEL_ALL_COLUMN, 0,
		TNY_GTK_FOLDER_STORE_TREE_MODEL_TYPE_COLUMN, TNY_FOLDER_TYPE_ROOT,
		TNY_GTK_FOLDER_STORE_TREE_MODEL_INSTANCE_COLUMN,
		folder_store, -1);

	/* In case we added a store account, it's possible that the account 
	 * will have "the account just got connected" events happening. Accounts
	 * that just got connected might have new folders for us to know about.
	 * That's why we'll handle connection changes. */

	if (TNY_IS_STORE_ACCOUNT (folder_store)) {
		if (!tny_account_is_ready (TNY_ACCOUNT (folder_store))) 
		{
			AccNotYetReadyInfo *info = g_slice_new (AccNotYetReadyInfo);

			info->account = TNY_ACCOUNT (g_object_ref (folder_store));
			info->self = TNY_GTK_FOLDER_STORE_TREE_MODEL (g_object_ref (self));

			g_timeout_add_full (G_PRIORITY_HIGH, 100, 
				account_was_not_yet_ready_idle, 
				info, account_was_not_yet_ready_destroy);

		} else {
			g_signal_connect (folder_store, "connection-status-changed",
				G_CALLBACK (tny_gtk_folder_store_tree_model_on_constatus_changed), self);
			g_signal_connect (folder_store, "changed",
				G_CALLBACK (tny_gtk_folder_store_tree_model_on_changed), self);
		}
	}

	/* Being online at startup is being punished by this: it'll have to
	 * wait for the connection operation to finish before this queued item
	 * will get its turn. TNY TODO: figure out how we can avoid this and
	 * already return the offline folders (if we had those from a session
	 * before this one) */

	tny_folder_store_get_folders_async (TNY_FOLDER_STORE (folder_store), 
		folders, get_folders_cb, NULL, NULL, g_object_ref (self));

	/* recurse_folders_sync (self, TNY_FOLDER_STORE (folder_store), &name_iter);  */


	/* Add an observer for the root folder store, so that we can observe 
	 * the actual account too. */

	tny_folder_store_add_observer (folder_store, TNY_FOLDER_STORE_OBSERVER (self));
	self->store_observables = g_list_prepend (self->store_observables, folder_store);

	/* g_object_unref (G_OBJECT (folders)); */

	return;
}


/**
 * tny_gtk_folder_store_tree_model_new:
 * @query: the #TnyFolderStoreQuery that will be used to retrieve the folders of each folder_store
 *
 * Create a new #GtkTreeModel instance suitable for showing  
 * #TnyFolderStore instances
 * 
 * Return value: a new #GtkTreeModel instance suitable for showing  
 * #TnyFolderStore instances
 **/
GtkTreeModel*
tny_gtk_folder_store_tree_model_new (TnyFolderStoreQuery *query)
{
	TnyGtkFolderStoreTreeModel *self = g_object_new (TNY_TYPE_GTK_FOLDER_STORE_TREE_MODEL, NULL);

	if (query) 
		self->query = g_object_ref (G_OBJECT (query));

	return GTK_TREE_MODEL (self);
}

static void 
destroy_folder_stores (gpointer item, gpointer user_data)
{
	if (item && G_IS_OBJECT (item))
		g_object_unref (G_OBJECT (item));
	return;
}

static void 
unregister_folder_observerable (gpointer item, gpointer user_data)
{
	TnyFolder *f = (TnyFolder *) item;
	tny_folder_remove_observer (f, TNY_FOLDER_OBSERVER (user_data));
}

static void 
unregister_store_observerable (gpointer item, gpointer user_data)
{
	TnyFolderStore *fstore = (TnyFolderStore *) item;
	tny_folder_store_remove_observer (fstore, TNY_FOLDER_STORE_OBSERVER (user_data));
}

static void
tny_gtk_folder_store_tree_model_finalize (GObject *object)
{
	TnyGtkFolderStoreTreeModel *me = (TnyGtkFolderStoreTreeModel*) object;

	g_mutex_lock (me->iterator_lock);
	if (me->first)
	{
		g_list_foreach (me->first, destroy_folder_stores, NULL);
		g_list_free (me->first); me->first = NULL;
	}
	g_mutex_unlock (me->iterator_lock);

	if (me->folder_observables)
	{
		g_list_foreach (me->folder_observables, unregister_folder_observerable, me);
		g_list_free (me->store_observables);
	}

	if (me->store_observables)
	{
		g_list_foreach (me->store_observables, unregister_store_observerable, me);
		g_list_free (me->store_observables);
	}

	g_mutex_free (me->iterator_lock);
	me->iterator_lock = NULL;

	if (me->query)
		g_object_unref (me->query);

	(*parent_class->finalize) (object);
}

static void
tny_gtk_folder_store_tree_model_class_init (TnyGtkFolderStoreTreeModelClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_gtk_folder_store_tree_model_finalize;

	return;
}

static void
tny_gtk_folder_store_tree_model_instance_init (GTypeInstance *instance, gpointer g_class)
{
	GtkTreeStore *store = (GtkTreeStore*) instance;
	TnyGtkFolderStoreTreeModel *me = (TnyGtkFolderStoreTreeModel*) instance;
	static GType types[] = { G_TYPE_STRING, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_INT, G_TYPE_OBJECT };

	me->iterator_lock = g_mutex_new ();
	me->folder_observables = NULL;
	me->store_observables = NULL;

	gtk_tree_store_set_column_types (store, 
		TNY_GTK_FOLDER_STORE_TREE_MODEL_N_COLUMNS, types);

	return;
}



static TnyIterator*
tny_gtk_folder_store_tree_model_create_iterator (TnyList *self)
{
	TnyGtkFolderStoreTreeModel *me = (TnyGtkFolderStoreTreeModel*)self;

	/* Return a new iterator */

	return TNY_ITERATOR (_tny_gtk_folder_store_tree_model_iterator_new (me));
}


/**
 * tny_gtk_folder_store_tree_model_prepend:
 * @self: A #TnyGtkFolderStoreTreeModel instance
 * @item: A #TnyFolderStore instance to add
 * @root_name: The node's root name 
 *
 * Prepends an item to the model
 **/
void
tny_gtk_folder_store_tree_model_prepend (TnyGtkFolderStoreTreeModel *self, TnyFolderStore* item, const gchar *root_name)
{
	TnyGtkFolderStoreTreeModel *me = (TnyGtkFolderStoreTreeModel*)self;

	g_mutex_lock (me->iterator_lock);

	/* Prepend something to the list */
	g_object_ref (item);
	me->first = g_list_prepend (me->first, item);

	tny_gtk_folder_store_tree_model_add_i (me, TNY_FOLDER_STORE (item), 
		gtk_tree_store_prepend, root_name);

	g_mutex_unlock (me->iterator_lock);
}


static void
tny_gtk_folder_store_tree_model_prepend_i (TnyList *self, GObject* item)
{
	TnyGtkFolderStoreTreeModel *me = (TnyGtkFolderStoreTreeModel*)self;

	g_mutex_lock (me->iterator_lock);

	/* Prepend something to the list */
	g_object_ref (item);
	me->first = g_list_prepend (me->first, item);

	tny_gtk_folder_store_tree_model_add_i (me, TNY_FOLDER_STORE (item), 
		gtk_tree_store_prepend, get_root_name (TNY_FOLDER_STORE (item)));

	g_mutex_unlock (me->iterator_lock);
}

/**
 * tny_gtk_folder_store_tree_model_append:
 * @self: A #TnyGtkFolderStoreTreeModel instance
 * @item: A #TnyFolderStore instance to add
 * @root_name: The node's root name 
 *
 * Appends an item to the model
 **/
void
tny_gtk_folder_store_tree_model_append (TnyGtkFolderStoreTreeModel *self, TnyFolderStore* item, const gchar *root_name)
{
	TnyGtkFolderStoreTreeModel *me = (TnyGtkFolderStoreTreeModel*)self;

	g_mutex_lock (me->iterator_lock);

	/* Append something to the list */
	g_object_ref (item);
	me->first = g_list_append (me->first, item);

	tny_gtk_folder_store_tree_model_add_i (me, TNY_FOLDER_STORE (item), 
		gtk_tree_store_append, root_name);

	g_mutex_unlock (me->iterator_lock);
}

static void
tny_gtk_folder_store_tree_model_append_i (TnyList *self, GObject* item)
{
	TnyGtkFolderStoreTreeModel *me = (TnyGtkFolderStoreTreeModel*)self;

	g_mutex_lock (me->iterator_lock);

	/* Append something to the list */
	g_object_ref (item);
	me->first = g_list_append (me->first, item);

	tny_gtk_folder_store_tree_model_add_i (me, TNY_FOLDER_STORE (item), 
		gtk_tree_store_append, get_root_name (TNY_FOLDER_STORE (item)));

	g_mutex_unlock (me->iterator_lock);
}

static guint
tny_gtk_folder_store_tree_model_get_length (TnyList *self)
{
	TnyGtkFolderStoreTreeModel *me = (TnyGtkFolderStoreTreeModel*)self;
	guint retval = 0;

	g_mutex_lock (me->iterator_lock);

	retval = me->first?g_list_length (me->first):0;

	g_mutex_unlock (me->iterator_lock);

	return retval;
}

static void
tny_gtk_folder_store_tree_model_remove (TnyList *self, GObject* item)
{
	TnyGtkFolderStoreTreeModel *me = (TnyGtkFolderStoreTreeModel*)self;
	GtkTreeModel *model = GTK_TREE_MODEL (me);
	GtkTreeIter iter;

	g_return_if_fail (G_IS_OBJECT (item));
	g_return_if_fail (G_IS_OBJECT (me));

	/* Remove something from the list */

	g_mutex_lock (me->iterator_lock);

	me->first = g_list_remove (me->first, (gconstpointer)item);

	/* This doesn't have to be recursive as only the first-level folders are
	   actually really part of the list. */

	if (gtk_tree_model_get_iter_first (model, &iter))
	  do 
	  {
		GObject *citem = NULL;
		gtk_tree_model_get (model, &iter, 
			TNY_GTK_FOLDER_STORE_TREE_MODEL_INSTANCE_COLUMN, 
			&citem, -1);
		if (citem == item)
		{
			/* This removes a reference count */
			gtk_tree_store_remove (GTK_TREE_STORE (me), &iter);
			g_object_unref (G_OBJECT (item));
			break;
		}
		if (citem)
			g_object_unref (G_OBJECT (citem));

	  } while (gtk_tree_model_iter_next (model, &iter));

	g_mutex_unlock (me->iterator_lock);
}


static TnyList*
tny_gtk_folder_store_tree_model_copy_the_list (TnyList *self)
{
	TnyGtkFolderStoreTreeModel *me = (TnyGtkFolderStoreTreeModel*)self;
	TnyGtkFolderStoreTreeModel *copy = g_object_new (TNY_TYPE_GTK_FOLDER_STORE_TREE_MODEL, NULL);
	GList *list_copy = NULL;

	/* This only copies the TnyList pieces. The result is not a correct or good
	   TnyHeaderListModel. But it will be a correct TnyList instance. It's the 
	   only thing the user of this method expects (that is the contract of it).

	   The new list will point to the same instances, of course. It's only a 
	   copy of the list-nodes of course. */

	g_mutex_lock (me->iterator_lock);
	list_copy = g_list_copy (me->first);
	g_list_foreach (list_copy, (GFunc)g_object_ref, NULL);
	copy->first = list_copy;
	g_mutex_unlock (me->iterator_lock);

	return TNY_LIST (copy);
}

static void 
tny_gtk_folder_store_tree_model_foreach_in_the_list (TnyList *self, GFunc func, gpointer user_data)
{
	TnyGtkFolderStoreTreeModel *me = (TnyGtkFolderStoreTreeModel*)self;

	/* Foreach item in the list (without using a slower iterator) */

	g_mutex_lock (me->iterator_lock);
	g_list_foreach (me->first, func, user_data);
	g_mutex_unlock (me->iterator_lock);

	return;
}


static gboolean 
updater (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data1)
{
	TnyFolderType type = TNY_FOLDER_TYPE_UNKNOWN;
	TnyFolderChange *change = user_data1;
	TnyFolder *changed_folder = tny_folder_change_get_folder (change);
	TnyFolderChangeChanged changed = tny_folder_change_get_changed (change);

	/* This updater will get the folder out of the model, compare with 
	 * the changed_folder pointer, and if there's a match it will update
	 * the values of the model with the values from the folder. */

	gtk_tree_model_get (model, iter, 
		TNY_GTK_FOLDER_STORE_TREE_MODEL_TYPE_COLUMN, 
		&type, -1);

	if (type != TNY_FOLDER_TYPE_ROOT) 
	{
		TnyFolder *folder;
		gint unread, total;

		gtk_tree_model_get (model, iter, 
			TNY_GTK_FOLDER_STORE_TREE_MODEL_INSTANCE_COLUMN, 
			&folder, -1);

		if (changed & TNY_FOLDER_CHANGE_CHANGED_ALL_COUNT)
			total = tny_folder_change_get_new_all_count (change);
		else
			total = tny_folder_get_all_count (folder);

		if (changed & TNY_FOLDER_CHANGE_CHANGED_UNREAD_COUNT)
			unread = tny_folder_change_get_new_unread_count (change);
		else
			unread = tny_folder_get_unread_count (folder);

		if (folder == changed_folder)
		{

			/* TNY TODO: This is not enough: Subfolders will be incorrect because the
			   the full_name of the subfolders will still be the old full_name!*/

			gtk_tree_store_set (GTK_TREE_STORE (model), iter,
				TNY_GTK_FOLDER_STORE_TREE_MODEL_NAME_COLUMN, 
				tny_folder_get_name (TNY_FOLDER (folder)),
				TNY_GTK_FOLDER_STORE_TREE_MODEL_UNREAD_COLUMN, 
				unread,
				TNY_GTK_FOLDER_STORE_TREE_MODEL_ALL_COLUMN, 
				total, -1);
		}

		g_object_unref (folder);
	}

	g_object_unref (changed_folder);

	return FALSE;
}


static gboolean 
deleter (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data1)
{
	gboolean retval = FALSE;
	TnyFolderType type = TNY_FOLDER_TYPE_UNKNOWN;
	GObject *folder = user_data1;

	/* The deleter will compare all folders in the model with the deleted 
	 * folder @folder, and if there's a match it will delete the folder's
	 * row from the model. */

	gtk_tree_model_get (model, iter, 
		TNY_GTK_FOLDER_STORE_TREE_MODEL_TYPE_COLUMN, 
		&type, -1);

	if (type != TNY_FOLDER_TYPE_ROOT) 
	{
		GObject *fol = NULL;

		gtk_tree_model_get (model, iter, 
			TNY_GTK_FOLDER_STORE_TREE_MODEL_INSTANCE_COLUMN, 
			&fol, -1);

		if (fol == folder) {
			gtk_tree_store_remove (GTK_TREE_STORE (model), iter);
			retval = TRUE;
		}

		if (fol)
			g_object_unref (fol);
	}

	return retval;
}


static gboolean
find_store_iter (GtkTreeModel *model, GtkTreeIter *iter, GtkTreeIter *f, gpointer user_data)
{
	/* Finding the iter for a given folder */

	do
	{
		GtkTreeIter child;
		TnyFolderType type = TNY_FOLDER_TYPE_UNKNOWN;
		TnyFolderStore *fol;
		gboolean found = FALSE;

		gtk_tree_model_get (model, iter, 
			TNY_GTK_FOLDER_STORE_TREE_MODEL_TYPE_COLUMN, 
			&type, -1);

		gtk_tree_model_get (model, iter, 
			TNY_GTK_FOLDER_STORE_TREE_MODEL_INSTANCE_COLUMN, 
			&fol, -1);
		
		if (fol == user_data)
			found = TRUE;
		
		if (fol)
			g_object_unref (fol);

		if (found) {
			*f = *iter;
			return TRUE;
		}

		if (gtk_tree_model_iter_children (model, &child, iter))
		{
			if (find_store_iter (model, &child, f, user_data))
				return TRUE;
		}

	} while (gtk_tree_model_iter_next (model, iter));

	return FALSE;
}



static void
tny_gtk_folder_store_tree_model_folder_obsr_update (TnyFolderObserver *self, TnyFolderChange *change)
{
	TnyFolderChangeChanged changed = tny_folder_change_get_changed (change);
	GtkTreeModel *model = (GtkTreeModel *) self;

	if (changed & TNY_FOLDER_CHANGE_CHANGED_FOLDER_RENAME ||
		changed & TNY_FOLDER_CHANGE_CHANGED_ALL_COUNT || 
		changed & TNY_FOLDER_CHANGE_CHANGED_UNREAD_COUNT)
	{
		gtk_tree_model_foreach (model, updater, change);
	}

	return;
}


static void
tny_gtk_folder_store_tree_model_store_obsr_update (TnyFolderStoreObserver *self, TnyFolderStoreChange *change)
{
	TnyFolderStoreChangeChanged changed = tny_folder_store_change_get_changed (change);
	GtkTreeModel *model = GTK_TREE_MODEL (self);
	TnyGtkFolderStoreTreeModel *me = (TnyGtkFolderStoreTreeModel*) self;

	if (changed & TNY_FOLDER_STORE_CHANGE_CHANGED_CREATED_FOLDERS)
	{
		TnyList *created = tny_simple_list_new ();
		TnyIterator *miter;

		tny_folder_store_change_get_created_folders (change, created);
		miter = tny_list_create_iterator (created);

		while (!tny_iterator_is_done (miter))
		{
			TnyFolder *folder = TNY_FOLDER (tny_iterator_get_current (miter));
			tny_folder_add_observer (TNY_FOLDER (folder), TNY_FOLDER_OBSERVER (self));
			tny_folder_store_add_observer (TNY_FOLDER_STORE (folder), TNY_FOLDER_STORE_OBSERVER (self));
			g_object_unref (folder);
			tny_iterator_next (miter);
		}
		g_object_unref (miter);
		g_object_unref (created);
	}

	if (changed & TNY_FOLDER_STORE_CHANGE_CHANGED_CREATED_FOLDERS)
	{
		TnyFolderStore *parentstore = tny_folder_store_change_get_folder_store (change);
		GtkTreeIter first, iter;

		if (gtk_tree_model_get_iter_first (model, &first) && 
			find_store_iter (model, &first, &iter, parentstore))
		{
			TnyList *created = tny_simple_list_new ();
			TnyIterator *miter;

			tny_folder_store_change_get_created_folders (change, created);
			miter = tny_list_create_iterator (created);

			while (!tny_iterator_is_done (miter))
			{
				GtkTreeIter newiter;
				TnyFolder *folder = TNY_FOLDER (tny_iterator_get_current (miter));

				me->folder_observables = g_list_prepend (me->folder_observables, folder);
				me->store_observables = g_list_prepend (me->store_observables, folder);

				gtk_tree_store_append (GTK_TREE_STORE (model), &newiter, &iter);

				/* This adds a reference count to folder_store too. When it gets 
				   removed, that reference count is decreased automatically by 
				   the gtktreestore infrastructure. */

				gtk_tree_store_set (GTK_TREE_STORE (model), &newiter,
					TNY_GTK_FOLDER_STORE_TREE_MODEL_NAME_COLUMN, 
					tny_folder_get_name (TNY_FOLDER (folder)),
					TNY_GTK_FOLDER_STORE_TREE_MODEL_UNREAD_COLUMN, 
					tny_folder_get_unread_count (TNY_FOLDER (folder)),
					TNY_GTK_FOLDER_STORE_TREE_MODEL_ALL_COLUMN, 
					tny_folder_get_all_count (TNY_FOLDER (folder)),
					TNY_GTK_FOLDER_STORE_TREE_MODEL_TYPE_COLUMN,
					tny_folder_get_folder_type (TNY_FOLDER (folder)),
					TNY_GTK_FOLDER_STORE_TREE_MODEL_INSTANCE_COLUMN,
					folder, -1);

				g_object_unref (folder);
				tny_iterator_next (miter);
			}
			g_object_unref (miter);
			g_object_unref (created);
		}
		g_object_unref (parentstore);
	}

	if (changed & TNY_FOLDER_STORE_CHANGE_CHANGED_REMOVED_FOLDERS)
	{
		TnyList *removed = tny_simple_list_new ();
		TnyIterator *miter;

		tny_folder_store_change_get_removed_folders (change, removed);
		miter = tny_list_create_iterator (removed);

		while (!tny_iterator_is_done (miter))
		{
			TnyFolder *folder = TNY_FOLDER (tny_iterator_get_current (miter));
			gtk_tree_model_foreach (model, deleter, folder);
			g_object_unref (folder);
			tny_iterator_next (miter);
		}
		g_object_unref (miter);
		g_object_unref (removed);
	}


	return;
}

static void
tny_folder_store_observer_init (TnyFolderStoreObserverIface *klass)
{
	klass->update_func = tny_gtk_folder_store_tree_model_store_obsr_update;
}

static void
tny_folder_observer_init (TnyFolderObserverIface *klass)
{
	klass->update_func = tny_gtk_folder_store_tree_model_folder_obsr_update;
}

static void
tny_list_init (TnyListIface *klass)
{
	klass->get_length_func = tny_gtk_folder_store_tree_model_get_length;
	klass->prepend_func = tny_gtk_folder_store_tree_model_prepend_i;
	klass->append_func = tny_gtk_folder_store_tree_model_append_i;
	klass->remove_func = tny_gtk_folder_store_tree_model_remove;
	klass->create_iterator_func = tny_gtk_folder_store_tree_model_create_iterator;
	klass->copy_func = tny_gtk_folder_store_tree_model_copy_the_list;
	klass->foreach_func = tny_gtk_folder_store_tree_model_foreach_in_the_list;

	return;
}

/**
 * tny_gtk_folder_store_tree_model_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
GType
tny_gtk_folder_store_tree_model_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyGtkFolderStoreTreeModelClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_gtk_folder_store_tree_model_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyGtkFolderStoreTreeModel),
		  0,      /* n_preallocs */
		  tny_gtk_folder_store_tree_model_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_list_info = {
			(GInterfaceInitFunc) tny_list_init,
			NULL,
			NULL
		};

		static const GInterfaceInfo tny_folder_store_observer_info = {
			(GInterfaceInitFunc) tny_folder_store_observer_init,
			NULL,
			NULL
		};

		static const GInterfaceInfo tny_folder_observer_info = {
			(GInterfaceInitFunc) tny_folder_observer_init,
			NULL,
			NULL
		};

		type = g_type_register_static (GTK_TYPE_TREE_STORE, "TnyGtkFolderStoreTreeModel",
					    &info, 0);

		g_type_add_interface_static (type, TNY_TYPE_LIST,
					     &tny_list_info);
		g_type_add_interface_static (type, TNY_TYPE_FOLDER_STORE_OBSERVER,
					     &tny_folder_store_observer_info);
		g_type_add_interface_static (type, TNY_TYPE_FOLDER_OBSERVER,
					     &tny_folder_observer_info);

	}

	return type;
}


/**
 * tny_gtk_folder_store_tree_model_column_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
GType
tny_gtk_folder_store_tree_model_column_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TNY_GTK_FOLDER_STORE_TREE_MODEL_NAME_COLUMN, "TNY_GTK_FOLDER_STORE_TREE_MODEL_NAME_COLUMN", "name" },
      { TNY_GTK_FOLDER_STORE_TREE_MODEL_UNREAD_COLUMN, "TNY_GTK_FOLDER_STORE_TREE_MODEL_UNREAD_COLUMN", "unread" },
      { TNY_GTK_FOLDER_STORE_TREE_MODEL_ALL_COLUMN, "TNY_GTK_FOLDER_STORE_TREE_MODEL_ALL_COLUMN", "all" },
      { TNY_GTK_FOLDER_STORE_TREE_MODEL_TYPE_COLUMN, "TNY_GTK_FOLDER_STORE_TREE_MODEL_TYPE_COLUMN", "type" },
      { TNY_GTK_FOLDER_STORE_TREE_MODEL_INSTANCE_COLUMN, "TNY_GTK_FOLDER_STORE_TREE_MODEL_INSTANCE_COLUMN", "instance" },
      { TNY_GTK_FOLDER_STORE_TREE_MODEL_N_COLUMNS, "TNY_GTK_FOLDER_STORE_TREE_MODEL_N_COLUMNS", "n" },
      { 0, NULL, NULL }
     };
    etype = g_enum_register_static ("TnyGtkFolderStoreTreeModelColumn", values);
  }
  return etype;
}


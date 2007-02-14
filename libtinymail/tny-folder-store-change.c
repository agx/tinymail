/* libtinymail - The Tiny Mail base library
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

#include <tny-folder-store-change.h>
#include <tny-simple-list.h>

static GObjectClass *parent_class = NULL;

typedef struct _TnyFolderStoreChangePriv TnyFolderStoreChangePriv;

struct _TnyFolderStoreChangePriv
{
	TnyList *created, *removed, *renamed;
	GMutex *lock;
	TnyFolderStore *folderstore;
	TnyFolderStoreChangeChanged changed;
};

#define TNY_FOLDER_STORE_CHANGE_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_FOLDER_STORE_CHANGE, TnyFolderStoreChangePriv))


/**
 * tny_folder_store_change_get_changed:
 * @self: a #TnyFolderStoreChange instance
 *
 * Get an enum with all the changed things
 **/
TnyFolderStoreChangeChanged 
tny_folder_store_change_get_changed  (TnyFolderStoreChange *self)
{
	TnyFolderStoreChangePriv *priv = TNY_FOLDER_STORE_CHANGE_GET_PRIVATE (self);

	return priv->changed;
}


/**
 * tny_folder_store_change_add_created_folder:
 * @self: a #TnyFolderStoreChange instance
 * @folder: the folder to add to the changeset
 *
 * Add @folder to the changeset of created folders
 **/
void 
tny_folder_store_change_add_created_folder (TnyFolderStoreChange *self, TnyFolder *folder)
{
	TnyFolderStoreChangePriv *priv = TNY_FOLDER_STORE_CHANGE_GET_PRIVATE (self);

	g_mutex_lock (priv->lock);

	if (!priv->created)
		priv->created = tny_simple_list_new ();
	tny_list_prepend (priv->created, G_OBJECT (folder));
	priv->changed |= TNY_FOLDER_STORE_CHANGE_CHANGED_CREATED_FOLDERS;

	g_mutex_unlock (priv->lock);

	return;
}



/**
 * tny_folder_store_change_add_renamed_folder:
 * @self: a #TnyFolderStoreChange instance
 * @folder: the folder to add to the changeset
 *
 * Add @folder to the changeset of renamed folders
 **/
void 
tny_folder_store_change_add_renamed_folder (TnyFolderStoreChange *self, TnyFolder *folder)
{
	TnyFolderStoreChangePriv *priv = TNY_FOLDER_STORE_CHANGE_GET_PRIVATE (self);

	g_mutex_lock (priv->lock);

	if (!priv->renamed)
		priv->renamed = tny_simple_list_new ();
	tny_list_prepend (priv->renamed, G_OBJECT (folder));
	priv->changed |= TNY_FOLDER_STORE_CHANGE_CHANGED_RENAMED_FOLDERS;

	g_mutex_unlock (priv->lock);

	return;
}

/**
 * tny_folder_store_change_add_removed_header:
 * @self: a #TnyFolderStoreChange instance
 * @folder: the folder to add to the changeset
 *
 * Add @folder to the changeset of removed folders
 **/
void 
tny_folder_store_change_add_removed_folder (TnyFolderStoreChange *self, TnyFolder *folder)
{
	TnyFolderStoreChangePriv *priv = TNY_FOLDER_STORE_CHANGE_GET_PRIVATE (self);

	g_mutex_lock (priv->lock);

	if (!priv->removed)
		priv->removed = tny_simple_list_new ();
	tny_list_prepend (priv->removed, G_OBJECT (folder));
	priv->changed |= TNY_FOLDER_STORE_CHANGE_CHANGED_REMOVED_FOLDERS;

	g_mutex_unlock (priv->lock);

	return;
}

/**
 * tny_folder_store_change_get_created_folders:
 * @self: a #TnyFolderStoreChange instance
 * @folders: the #TnyList where the created folders will be put it
 *
 * Get the created folders in this changeset
 **/
void 
tny_folder_store_change_get_created_folders (TnyFolderStoreChange *self, TnyList *folders)
{
	TnyFolderStoreChangePriv *priv = TNY_FOLDER_STORE_CHANGE_GET_PRIVATE (self);
	TnyIterator *iter;

	g_assert (TNY_IS_LIST (folders));

	g_mutex_lock (priv->lock);

	if (!priv->created)
	{
		g_mutex_unlock (priv->lock);
		return;
	}

	iter = tny_list_create_iterator (priv->created);

	while (!tny_iterator_is_done (iter))
	{
		GObject *folder = tny_iterator_get_current (iter);
		tny_list_prepend (folders, folder);
		g_object_unref (folder);
		tny_iterator_next (iter);
	}

	g_object_unref (G_OBJECT (iter));

	g_mutex_unlock (priv->lock);

	return;
}



/**
 * tny_folder_store_change_get_renamed_folders:
 * @self: a #TnyFolderStoreChange instance
 * @folders: the #TnyList where the renamed folders will be put it
 *
 * Get the renamed folders in this changeset
 **/
void 
tny_folder_store_change_get_renamed_folders (TnyFolderStoreChange *self, TnyList *folders)
{
	TnyFolderStoreChangePriv *priv = TNY_FOLDER_STORE_CHANGE_GET_PRIVATE (self);
	TnyIterator *iter;

	g_assert (TNY_IS_LIST (folders));

	g_mutex_lock (priv->lock);

	if (!priv->created)
	{
		g_mutex_unlock (priv->lock);
		return;
	}

	iter = tny_list_create_iterator (priv->renamed);

	while (!tny_iterator_is_done (iter))
	{
		GObject *folder = tny_iterator_get_current (iter);
		tny_list_prepend (folders, folder);
		g_object_unref (folder);
		tny_iterator_next (iter);
	}

	g_object_unref (G_OBJECT (iter));

	g_mutex_unlock (priv->lock);

	return;
}


/**
 * tny_folder_store_change_get_removed_folders:
 * @self: a #TnyFolderStoreChange instance
 * @folders: the #TnyList where the removed folders will be put it
 *
 * Get the removed folders in this changeset
 **/
void 
tny_folder_store_change_get_removed_folders (TnyFolderStoreChange *self, TnyList *folders)
{
	TnyFolderStoreChangePriv *priv = TNY_FOLDER_STORE_CHANGE_GET_PRIVATE (self);
	TnyIterator *iter;

	g_assert (TNY_IS_LIST (folders));

	g_mutex_lock (priv->lock);

	if (!priv->removed)
	{
		g_mutex_unlock (priv->lock);
		return;
	}

	iter = tny_list_create_iterator (priv->removed);

	while (!tny_iterator_is_done (iter))
	{
		GObject *folder = tny_iterator_get_current (iter);
		tny_list_prepend (folders, folder);
		g_object_unref (folder);
		tny_iterator_next (iter);
	}

	g_object_unref (G_OBJECT (iter));

	g_mutex_unlock (priv->lock);

	return;
}

/**
 * tny_folder_store_change_reset:
 * @self: a #TnyFolderStoreChange instance
 *
 * Reset the state of @self
 **/
void 
tny_folder_store_change_reset (TnyFolderStoreChange *self)
{
	TnyFolderStoreChangePriv *priv = TNY_FOLDER_STORE_CHANGE_GET_PRIVATE (self);

	g_mutex_lock (priv->lock);

	priv->changed = 0;
	if (priv->created)
		g_object_unref (G_OBJECT (priv->created));
	if (priv->removed)
		g_object_unref (G_OBJECT (priv->removed));
	if (priv->renamed)
		g_object_unref (G_OBJECT (priv->removed));
	priv->created = NULL;
	priv->removed = NULL;
	priv->renamed = NULL;

	g_mutex_unlock (priv->lock);
}


/**
 * tny_folder_store_change_new:
 * @folderstore: a #TnyFolderStore instance
 *
 * Creates a changeset for @folderstore
 *
 * Return value: a new #TnyFolderStoreChange instance
 **/
TnyFolderStoreChange*
tny_folder_store_change_new (TnyFolderStore *folderstore)
{
	TnyFolderStoreChange *self = g_object_new (TNY_TYPE_FOLDER_STORE_CHANGE, NULL);
	TnyFolderStoreChangePriv *priv = TNY_FOLDER_STORE_CHANGE_GET_PRIVATE (self);

	priv->folderstore = TNY_FOLDER_STORE (g_object_ref (G_OBJECT (folderstore)));

	return self;
}

/**
 * tny_folder_store_change_get_folder_store:
 * @self: a #TnyFolderStoreChange instance
 *
 * Get the folderstore of @self. The return value of this method must be unreferenced 
 * after use
 *
 * Return value: the #TnyFolderStore instance related to this changeset
 **/
TnyFolderStore* 
tny_folder_store_change_get_folder_store (TnyFolderStoreChange *self)
{
	TnyFolderStoreChangePriv *priv = TNY_FOLDER_STORE_CHANGE_GET_PRIVATE (self);
	TnyFolderStore *retval = NULL;

	g_mutex_lock (priv->lock);
	if (priv->folderstore)
		retval = TNY_FOLDER_STORE (g_object_ref (G_OBJECT (priv->folderstore)));
	g_mutex_unlock (priv->lock);

	return retval;
}


static void
tny_folder_store_change_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyFolderStoreChange *self = (TnyFolderStoreChange *)instance;
	TnyFolderStoreChangePriv *priv = TNY_FOLDER_STORE_CHANGE_GET_PRIVATE (self);

	priv->lock = g_mutex_new ();

	g_mutex_lock (priv->lock);

	priv->changed = 0;
	priv->created = NULL;
	priv->removed = NULL;
	priv->renamed = NULL;
	priv->folderstore = NULL;

	g_mutex_unlock (priv->lock);

	return;
}

static void
tny_folder_store_change_finalize (GObject *object)
{
	TnyFolderStoreChange *self = (TnyFolderStoreChange *)object;	
	TnyFolderStoreChangePriv *priv = TNY_FOLDER_STORE_CHANGE_GET_PRIVATE (self);

	g_mutex_lock (priv->lock);

	if (priv->renamed)
		g_object_unref (G_OBJECT (priv->renamed));
	if (priv->created)
		g_object_unref (G_OBJECT (priv->created));
	if (priv->removed)
		g_object_unref (G_OBJECT (priv->removed));
	priv->created = NULL;
	priv->removed = NULL;
	priv->renamed = NULL;

	if (priv->folderstore)
		g_object_unref (G_OBJECT (priv->folderstore));

	g_mutex_unlock (priv->lock);

	g_mutex_free (priv->lock);

	(*parent_class->finalize) (object);

	return;
}


static void 
tny_folder_store_change_class_init (TnyFolderStoreChangeClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;
	object_class->finalize = tny_folder_store_change_finalize;
	g_type_class_add_private (object_class, sizeof (TnyFolderStoreChangePriv));

	return;
}

GType 
tny_folder_store_change_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyFolderStoreChangeClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_folder_store_change_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyFolderStoreChange),
		  0,      /* n_preallocs */
		  tny_folder_store_change_instance_init,   /* instance_init */
		  NULL
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyFolderStoreChange",
			&info, 0);
	}

	return type;
}


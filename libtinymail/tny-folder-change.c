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

#include <tny-folder-change.h>
#include <tny-simple-list.h>

static GObjectClass *parent_class = NULL;

typedef struct _TnyFolderChangePriv TnyFolderChangePriv;

struct _TnyFolderChangePriv
{
	TnyList *added, *removed;
	guint new_unread_count;
	guint new_all_count;
	GMutex *lock;
	TnyFolder *folder;
	gchar *oldname, *newname;
	TnyFolderChangeChanged changed;
};

#define TNY_FOLDER_CHANGE_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_FOLDER_CHANGE, TnyFolderChangePriv))


/**
 * tny_folder_change_get_changed:
 * @self: a #TnyFolderChange instance
 *
 * Get an enum with all the changed things
 **/
TnyFolderChangeChanged 
tny_folder_change_get_changed  (TnyFolderChange *self)
{
	TnyFolderChangePriv *priv = TNY_FOLDER_CHANGE_GET_PRIVATE (self);

	return priv->changed;
}

/**
 * tny_folder_change_set_new_all_count:
 * @self: a #TnyFolderChange instance
 * @new_all_count: the new all count of the folder
 *
 * Set the all count of the changeset
 **/
void 
tny_folder_change_set_new_all_count (TnyFolderChange *self, guint new_all_count)
{
	TnyFolderChangePriv *priv = TNY_FOLDER_CHANGE_GET_PRIVATE (self);

	priv->new_all_count = new_all_count;

	return;
}

/**
 * tny_folder_change_set_new_unread_count:
 * @self: a #TnyFolderChange instance
 * @new_unread_count: the new unread count of the folder
 *
 * Set the unread count of the changeset
 **/
void 
tny_folder_change_set_new_unread_count (TnyFolderChange *self, guint new_unread_count)
{
	TnyFolderChangePriv *priv = TNY_FOLDER_CHANGE_GET_PRIVATE (self);

	priv->new_unread_count = new_unread_count;
	priv->changed |= TNY_FOLDER_CHANGE_CHANGED_UNREAD_COUNT;

	return;
}


/**
 * tny_folder_change_get_new_unread_count:
 * @self: a #TnyFolderChange instance
 *
 * Get the unread count of the changeset
 *
 * Return value: the new unread-count
 **/
guint 
tny_folder_change_get_new_unread_count (TnyFolderChange *self)
{
	TnyFolderChangePriv *priv = TNY_FOLDER_CHANGE_GET_PRIVATE (self);

	return priv->new_unread_count;
}

/**
 * tny_folder_change_get_new_all_count:
 * @self: a #TnyFolderChange instance
 *
 * Get the all count of the changeset
 *
 * Return value: the new all-count
 **/
guint 
tny_folder_change_get_new_all_count (TnyFolderChange *self)
{
	TnyFolderChangePriv *priv = TNY_FOLDER_CHANGE_GET_PRIVATE (self);

	return priv->new_all_count;
}

/**
 * tny_folder_change_add_added_header:
 * @self: a #TnyFolderChange instance
 * @header: the header to add to the changeset
 *
 * Add @header to the changeset of added headers
 **/
void 
tny_folder_change_add_added_header (TnyFolderChange *self, TnyHeader *header)
{
	TnyFolderChangePriv *priv = TNY_FOLDER_CHANGE_GET_PRIVATE (self);

	g_mutex_lock (priv->lock);

	if (!priv->added)
		priv->added = tny_simple_list_new ();
	tny_list_prepend (priv->added, G_OBJECT (header));
	priv->changed |= TNY_FOLDER_CHANGE_CHANGED_ADDED_HEADERS;

	g_mutex_unlock (priv->lock);

	return;
}

/**
 * tny_folder_change_add_removed_header:
 * @self: a #TnyFolderChange instance
 * @header: the header to add to the changeset
 *
 * Add @header to the changeset of removed headers
 **/
void 
tny_folder_change_add_removed_header (TnyFolderChange *self, TnyHeader *header)
{
	TnyFolderChangePriv *priv = TNY_FOLDER_CHANGE_GET_PRIVATE (self);

	g_mutex_lock (priv->lock);

	if (!priv->removed)
		priv->removed = tny_simple_list_new ();
	tny_list_prepend (priv->removed, G_OBJECT (header));
	priv->changed |= TNY_FOLDER_CHANGE_CHANGED_REMOVED_HEADERS;

	g_mutex_unlock (priv->lock);

	return;
}

/**
 * tny_folder_change_get_added_headers:
 * @self: a #TnyFolderChange instance
 * @headers: the #TnyList where the added headers will be put it
 *
 * Get the added headers in this changeset
 **/
void 
tny_folder_change_get_added_headers (TnyFolderChange *self, TnyList *headers)
{
	TnyFolderChangePriv *priv = TNY_FOLDER_CHANGE_GET_PRIVATE (self);
	TnyIterator *iter;

	g_assert (TNY_IS_LIST (headers));

	g_mutex_lock (priv->lock);

	if (!priv->added)
	{
		g_mutex_unlock (priv->lock);
		return;
	}

	iter = tny_list_create_iterator (priv->added);

	while (!tny_iterator_is_done (iter))
	{
		GObject *header = tny_iterator_get_current (iter);
		tny_list_prepend (headers, header);
		g_object_unref (header);
		tny_iterator_next (iter);
	}

	g_object_unref (G_OBJECT (iter));

	g_mutex_unlock (priv->lock);

	return;
}


/**
 * tny_folder_change_get_removed_headers:
 * @self: a #TnyFolderChange instance
 * @headers: the #TnyList where the removed headers will be put it
 *
 * Get the removed headers in this changeset
 **/
void 
tny_folder_change_get_removed_headers (TnyFolderChange *self, TnyList *headers)
{
	TnyFolderChangePriv *priv = TNY_FOLDER_CHANGE_GET_PRIVATE (self);
	TnyIterator *iter;

	g_assert (TNY_IS_LIST (headers));

	g_mutex_lock (priv->lock);

	if (!priv->removed)
	{
		g_mutex_unlock (priv->lock);
		return;
	}

	iter = tny_list_create_iterator (priv->removed);

	while (!tny_iterator_is_done (iter))
	{
		GObject *header = tny_iterator_get_current (iter);
		tny_list_prepend (headers, header);
		g_object_unref (header);
		tny_iterator_next (iter);
	}

	g_object_unref (G_OBJECT (iter));

	g_mutex_unlock (priv->lock);

	return;
}

/**
 * tny_folder_change_reset:
 * @self: a #TnyFolderChange instance
 *
 * Reset the state of @self
 **/
void 
tny_folder_change_reset (TnyFolderChange *self)
{
	TnyFolderChangePriv *priv = TNY_FOLDER_CHANGE_GET_PRIVATE (self);

	g_mutex_lock (priv->lock);

	priv->changed = 0;
	priv->new_unread_count = 0;
	priv->new_all_count = 0;
	if (priv->added)
		g_object_unref (G_OBJECT (priv->added));
	if (priv->removed)
		g_object_unref (G_OBJECT (priv->removed));
	priv->added = NULL;
	priv->removed = NULL;

	g_mutex_unlock (priv->lock);
}

/**
 * tny_folder_change_get_rename:
 * @self: a #TnyFolderChange instance
 * @oldname: a pointer to a string
 *
 * Get the new name of the folder in case of a rename. This will return NULL
 * of nu rename happened. You can pass a pointer if you need the old folder
 * name too.
 *
 * You must not free the return value nor the @oldname pointer. It's handled
 * internally in the TnyFolderChange type.
 **/
const gchar *
tny_folder_change_get_rename (TnyFolderChange *self, const gchar **oldname)
{
	const gchar *retval = NULL;
	TnyFolderChangePriv *priv = TNY_FOLDER_CHANGE_GET_PRIVATE (self);

	g_mutex_lock (priv->lock);

	if (priv->changed & TNY_FOLDER_CHANGE_CHANGED_FOLDER_RENAME)
	{
		retval = priv->newname;
		if (oldname)
			*oldname = priv->oldname;
	}

	g_mutex_unlock (priv->lock);

	return retval;
}

/**
 * tny_folder_change_set_rename:
 * @self: a #TnyFolderChange instance
 * @newname: the new name of the folder
 *
 * Mark the change in such a way that the user can know that a rename has 
 * happened. The TnyFolderChange will copy your @newname internally, so you
 * can do whatever you want with what you passed afterwards.
 **/
void 
tny_folder_change_set_rename (TnyFolderChange *self, const gchar *newname)
{
	TnyFolderChangePriv *priv = TNY_FOLDER_CHANGE_GET_PRIVATE (self);

	g_mutex_lock (priv->lock);
	priv->changed |= TNY_FOLDER_CHANGE_CHANGED_FOLDER_RENAME;
	if (priv->newname)
		g_free (priv->newname);
	priv->newname = g_strdup (newname);
	g_mutex_unlock (priv->lock);
}


/**
 * tny_folder_change_new:
 * @folder: a #TnyFolder instance
 *
 * Creates a changeset for @folder
 *
 * Return value: a new #TnyFolderChange instance
 **/
TnyFolderChange*
tny_folder_change_new (TnyFolder *folder)
{
	TnyFolderChange *self = g_object_new (TNY_TYPE_FOLDER_CHANGE, NULL);
	TnyFolderChangePriv *priv = TNY_FOLDER_CHANGE_GET_PRIVATE (self);

	priv->folder = TNY_FOLDER (g_object_ref (G_OBJECT (folder)));
	priv->oldname = g_strdup (tny_folder_get_name (folder));

	return self;
}

/**
 * tny_folder_change_get_folder:
 * @self: a #TnyFolderChange instance
 *
 * Get the folder of @self. The return value of this method must be unreferenced 
 * after use
 *
 * Return value: the #TnyFolder instance related to this changeset
 **/
TnyFolder* 
tny_folder_change_get_folder (TnyFolderChange *self)
{
	TnyFolderChangePriv *priv = TNY_FOLDER_CHANGE_GET_PRIVATE (self);
	TnyFolder *retval = NULL;

	g_mutex_lock (priv->lock);
	if (priv->folder)
		retval = TNY_FOLDER (g_object_ref (G_OBJECT (priv->folder)));
	g_mutex_unlock (priv->lock);

	return retval;
}


static void
tny_folder_change_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyFolderChange *self = (TnyFolderChange *)instance;
	TnyFolderChangePriv *priv = TNY_FOLDER_CHANGE_GET_PRIVATE (self);

	priv->lock = g_mutex_new ();

	g_mutex_lock (priv->lock);

	priv->changed = 0;
	priv->new_unread_count = 0;
	priv->new_all_count = 0;
	priv->added = NULL;
	priv->removed = NULL;
	priv->folder = NULL;
	priv->oldname = NULL;
	priv->newname = NULL;

	g_mutex_unlock (priv->lock);

	return;
}

static void
tny_folder_change_finalize (GObject *object)
{
	TnyFolderChange *self = (TnyFolderChange *)object;	
	TnyFolderChangePriv *priv = TNY_FOLDER_CHANGE_GET_PRIVATE (self);

	g_mutex_lock (priv->lock);

	if (priv->added)
		g_object_unref (G_OBJECT (priv->added));
	if (priv->removed)
		g_object_unref (G_OBJECT (priv->removed));
	priv->added = NULL;
	priv->removed = NULL;

	if (priv->oldname)
		g_free (priv->oldname);
	if (priv->newname)
		g_free (priv->newname);

	if (priv->folder)
		g_object_unref (G_OBJECT (priv->folder));

	g_mutex_unlock (priv->lock);

	g_mutex_free (priv->lock);

	(*parent_class->finalize) (object);

	return;
}


static void 
tny_folder_change_class_init (TnyFolderChangeClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;
	object_class->finalize = tny_folder_change_finalize;
	g_type_class_add_private (object_class, sizeof (TnyFolderChangePriv));

	return;
}

GType 
tny_folder_change_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyFolderChangeClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_folder_change_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyFolderChange),
		  0,      /* n_preallocs */
		  tny_folder_change_instance_init,   /* instance_init */
		  NULL
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyFolderChange",
			&info, 0);
	}

	return type;
}


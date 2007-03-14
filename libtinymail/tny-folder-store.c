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

#ifdef DBC
#include <string.h>
#endif

#include <tny-folder-store.h>
#include <tny-folder-store-observer.h>

/* Possible future API changes:
 * tny_folder_store_find_folder for finding a folder using an url_string, maybe 
 * also a tny_folder_store_set_find_folder_strategy and a 
 * tny_folder_store_get_find_folder_strategy if in future alternative ways to 
 * find a folder are to be specified and developed */

/**
 * tny_folder_store_add_observer:
 * @self: a #TnyFolder instance
 * @observer: a #TnyFolderStoreObserver instance
 *
 * Add @observer to the list of interested observers for the 
 * event that could happen.
 *
 **/
void 
tny_folder_store_add_observer (TnyFolderStore *self, TnyFolderStoreObserver *observer)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER_STORE (self));
	g_assert (observer);
	g_assert (TNY_IS_FOLDER_STORE_OBSERVER (observer));
	g_assert (TNY_FOLDER_STORE_GET_IFACE (self)->add_observer_func != NULL);
#endif

	TNY_FOLDER_STORE_GET_IFACE (self)->add_observer_func (self, observer);

#ifdef DBC /* ensure */
	/* TNY TODO: Check whether it's really added */
#endif

	return;
}


/**
 * tny_folder_store_remove_observer:
 * @self: a #TnyFolderStore instance
 * @observer: a #TnyFolderStoreObserver instance
 *
 * Remove @observer from the list of interested observers for the 
 * event that could happen.
 *
 **/
void 
tny_folder_store_remove_observer (TnyFolderStore *self, TnyFolderStoreObserver *observer)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER_STORE (self));
	g_assert (observer);
	g_assert (TNY_IS_FOLDER_STORE_OBSERVER (observer));
	g_assert (TNY_FOLDER_STORE_GET_IFACE (self)->remove_observer_func != NULL);
#endif

	TNY_FOLDER_STORE_GET_IFACE (self)->remove_observer_func (self, observer);

#ifdef DBC /* ensure */
	/* TNY TODO: Check whether it's really removed */
#endif

	return;

}

/**
 * tny_folder_store_remove_folder:
 * @self: a #TnyFolderStore object
 * @folder: The folder to remove
 * @err: a #GError object or NULL
 *
 * Removes a folder represented by @folder from the folder store @self. You are
 * responsible for unreferencing the @folder instance yourself. This method will
 * not do this for you, leaving the @folder instance in an unusable state. The 
 * id of the @folder instance will be blanked once really deleted from the
 * service. All the #TnyFolderObservers and #TnyFolderStoreObservers of @folder,
 * but of course not of @self, will automatically be unsubscribed.
 * 
 * Example:
 * <informalexample><programlisting>
 * static void
 * my_remove_a_folder (TnyFolderStore *store, TnyFolder *remfol, GError **err)
 * {
 *     tny_folder_store_remove_folder (store, remfol, err);
 *     g_object_unref (G_OBJECT (remfol));
 * }
 * </programlisting></informalexample>
 *
 **/
void 
tny_folder_store_remove_folder (TnyFolderStore *self, TnyFolder *folder, GError **err)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER_STORE (self));
	g_assert (folder);
	g_assert (TNY_IS_FOLDER (folder));
	g_assert (TNY_FOLDER_STORE_GET_IFACE (self)->remove_folder_func != NULL);
#endif

	TNY_FOLDER_STORE_GET_IFACE (self)->remove_folder_func (self, folder, err);

#ifdef DBC /* ensure */
	/* Checking this is something for a unit test */
#endif

	return;
}

/**
 * tny_folder_store_create_folder:
 * @self: a #TnyFolderStore object
 * @name: The folder name to create
 * @err: a #GError object or NULL
 *
 * Creates a new folder in @self. If not NULL, the value returned is the newly 
 * created folder instance and must be unreferenced after use.
 *
 * Example:
 * <informalexample><programlisting>
 * TnyFolderStore *store = ...
 * TnyFolder *createfol;
 * createfol = tny_folder_store_create_folder (store, "Test", NULL);
 * if (createfol) g_object_unref (G_OBJECT (createfol));
 * </programlisting></informalexample>
 * 
 * Return value: A new folder instance representing the folder that was created or NULL in case of failure
 *
 **/
TnyFolder*
tny_folder_store_create_folder (TnyFolderStore *self, const gchar *name, GError **err)
{
	TnyFolder *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER_STORE (self));
	g_assert (name);
	g_assert (strlen (name) > 0);
	g_assert (TNY_FOLDER_STORE_GET_IFACE (self)->create_folder_func != NULL);
#endif

	retval = TNY_FOLDER_STORE_GET_IFACE (self)->create_folder_func (self, name, err);

#ifdef DBC /* ensure */
	if (retval)
		g_assert (TNY_IS_FOLDER (retval));
#endif

	return retval;
}

/**
 * tny_folder_store_get_folders:
 * @self: a #TnyFolderStore object
 * @list: A #TnyList to fillup
 * @query: A #TnyFolderStoreQuery object or NULL
 * @err: a #GError object or NULL
 *
 * Get a list of child folders from @self. You can use @query to limit the list 
 * of folders with only folders that match a query or NULL if you don't want
 * to limit the list at all.
 * 
 * Example:
 * <informalexample><programlisting>
 * TnyFolderStore *store = ...
 * TnyIterator *iter; TnyFolderStoreQuery *query = ...
 * TnyList *folders = tny_simple_list_new ();
 * tny_folder_store_get_folders (store, folders, query, NULL);
 * iter = tny_list_create_iterator (folders);
 * while (!tny_iterator_is_done (iter))
 * {
 *     TnyFolder *folder = TNY_FOLDER (tny_iterator_get_current (iter));
 *     g_print ("%s\n", tny_folder_get_name (folder));
 *     g_object_unref (G_OBJECT (folder));
 *     tny_iterator_next (iter);
 * }
 * g_object_unref (G_OBJECT (iter));
 * g_object_unref (G_OBJECT (folders)); 
 * </programlisting></informalexample>
 **/
void 
tny_folder_store_get_folders (TnyFolderStore *self, TnyList *list, TnyFolderStoreQuery *query, GError **err)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER_STORE (self));
	g_assert (list);
	g_assert (TNY_IS_LIST (list));
	if (query)
		g_assert (TNY_IS_FOLDER_STORE_QUERY (query));
	g_assert (TNY_FOLDER_STORE_GET_IFACE (self)->get_folders_func != NULL);
#endif

	TNY_FOLDER_STORE_GET_IFACE (self)->get_folders_func (self, list, query, err);

#ifdef DBC /* ensure */
#endif

	return;
}

/**
 * tny_folder_store_get_folders_async:
 * @self: a #TnyFolderStore object
 * @list: A #TnyList to fillup
 * @callback: The callback handler
 * @query: A #TnyFolderStoreQuery object
 * @user_data: user data for the callback
 *
 * Get a list of child folders from the folder store @self and call back when 
 * finished. You can use @query to limit the list of folders with only folders 
 * that match a query or NULL if you don't want to limit the list at all.
 *
 * Example:
 * <informalexample><programlisting>
 * static void 
 * callback (TnyFolderStore *self, TnyList *list, GError **err, gpointer user_data)
 * {
 *     TnyIterator *iter = tny_list_create_iterator (list);
 *     while (!tny_iterator_is_done (iter))
 *     {
 *         TnyFolderStore *folder = tny_iterator_get_current (iter);
 *         TnyList *folders = tny_simple_list_new ();
 *         g_print ("%s\n", tny_folder_get_name (TNY_FOLDER (folder)));
 *         tny_folder_store_get_folders_async (folder,
 *             folders, callback, NULL, NULL);
 *         g_object_unref (G_OBJECT (folder));
 *         tny_iterator_next (iter);
 *     }
 *     g_object_unref (G_OBJECT (iter));
 *     g_object_unref (G_OBJECT (list));
 * } 
 * static void
 * get_all_folders (TnyStoreAccount *account)
 * {
 *     TnyList *folders;
 *     folders = tny_simple_list_new ();
 *     tny_folder_store_get_folders_async (TNY_FOLDER_STORE (account),
 *         folders, callback, NULL, NULL);
 * }
 * </programlisting></informalexample>
 *
 * If you want to use this functionality, you are advised to let your application 
 * use a #GMainLoop. All Gtk+ applications have this once gtk_main () is called.
 * 
 * When using a #GMainLoop, this method will callback using g_idle_add_full.
 * Without a #GMainLoop, which the libtinymail-camel implementations detect
 * using (g_main_depth > 0), the callbacks will happen in a worker thread at an 
 * unknown moment in time (check your locking).
 *
 * When using Gtk+ the callback doesn't need gdk_threads_enter and 
 * gdk_threads_leave in Gtk+.
 *
 **/
void 
tny_folder_store_get_folders_async (TnyFolderStore *self, TnyList *list, TnyGetFoldersCallback callback, TnyFolderStoreQuery *query, gpointer user_data)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER_STORE (self));
	g_assert (list);
	g_assert (callback);
	g_assert (TNY_IS_LIST (list));
	if (query)
		g_assert (TNY_IS_FOLDER_STORE_QUERY (query));
	g_assert (TNY_FOLDER_STORE_GET_IFACE (self)->get_folders_async_func != NULL);
#endif

	TNY_FOLDER_STORE_GET_IFACE (self)->get_folders_async_func (self, list, callback, query, user_data);

#ifdef DBC /* ensure */
#endif

	return;
}



static void
tny_folder_store_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_folder_store_get_type (void)
{
	static GType type = 0;
	
	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyFolderStoreIface),
		  tny_folder_store_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL,    /* instance_init */
		  NULL
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyFolderStore", &info, 0);
	}

	return type;
}



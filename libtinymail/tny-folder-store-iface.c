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

#include <tny-folder-store-iface.h>


/**
 * tny_folder_store_iface_remove_folder:
 * @self: a #TnyFolderStoreIface object
 * @folder: The folder to remove
 *
 * Removes a folder represented by @folder from the folder store @self
 * 
 **/
void 
tny_folder_store_iface_remove_folder (TnyFolderStoreIface *self, TnyFolderIface *folder)
{
#ifdef DEBUG
	if (!TNY_FOLDER_STORE_IFACE_GET_CLASS (self)->remove_folder_func)
		g_critical ("You must implement tny_folder_store_iface_remove_folder\n");
#endif

	TNY_FOLDER_STORE_IFACE_GET_CLASS (self)->remove_folder_func (self, folder);
	return;
}

/**
 * tny_folder_store_iface_create_folder:
 * @self: a #TnyFolderStoreIface object
 * @name: The folder name to create
 *
 * Creates a new folder in folder store @self
 * 
 * Return value: A new folder instance representing the folder that was created
 *
 **/
TnyFolderIface *
tny_folder_store_iface_create_folder (TnyFolderStoreIface *self, const gchar *name)
{
#ifdef DEBUG
	if (!TNY_FOLDER_STORE_IFACE_GET_CLASS (self)->create_folder_func)
		g_critical ("You must implement tny_folder_store_iface_create_folder\n");
#endif

	return TNY_FOLDER_STORE_IFACE_GET_CLASS (self)->create_folder_func (self, name);
}

/**
 * tny_folder_store_iface_get_folders:
 * @self: a #TnyFolderStoreIface object
 * @list: A #TnyListIface to fillup
 * @query: A #TnyFolderStoreQuery object or NULL
 *
 * Get a list of child folders from the folder store @self. You can use @query to 
 * limit the list of folders with only folders that match the query.
 * 
 **/
void 
tny_folder_store_iface_get_folders (TnyFolderStoreIface *self, TnyListIface *list, TnyFolderStoreQuery *query)
{
#ifdef DEBUG
	if (!TNY_FOLDER_STORE_IFACE_GET_CLASS (self)->get_folders_func)
		g_critical ("You must implement tny_folder_store_iface_get_folders\n");
#endif

	TNY_FOLDER_STORE_IFACE_GET_CLASS (self)->get_folders_func (self, list, query);
	return;
}

/**
 * tny_folder_store_iface_get_folders_async:
 * @self: a #TnyFolderStoreIface object
 * @list: A #TnyListIface to fillup
 * @callback: The callback handler
 * @status_callback: A callback for status notifications
 * @query: A #TnyFolderStoreQuery object
 * @user_data: user data for the callback
 *
 * Get a list of child folders from the folder store @self and call back when 
 * finished. 
 *
 * If you want to use this functionality, it's advised to let your application 
 * use the #GMainLoop. All Gtk+ applications have this once gtk_main () is
 * called.
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
tny_folder_store_iface_get_folders_async (TnyFolderStoreIface *self, TnyListIface *list, TnyGetFoldersCallback callback, TnyFolderStoreQuery *query, gpointer user_data)
{
#ifdef DEBUG
	if (!TNY_FOLDER_STORE_IFACE_GET_CLASS (self)->get_folders_async_func)
		g_critical ("You must implement tny_folder_store_iface_get_folders_async\n");
#endif

	TNY_FOLDER_STORE_IFACE_GET_CLASS (self)->get_folders_async_func (self, list, callback, query, user_data);
	return;
}



static void
tny_folder_store_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_folder_store_iface_get_type (void)
{
	static GType type = 0;
	
	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyFolderStoreIfaceClass),
		  tny_folder_store_iface_base_init,   /* base_init */
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
			"TnyFolderStoreIface", &info, 0);
	    
	    g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}

	return type;
}



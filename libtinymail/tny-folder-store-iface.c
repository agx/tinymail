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
 * Removes folder from self
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
 * Creates a new folder in self
 * 
 * Return value: A new folder instance representing the folder that was created
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
 * @query: A #TnyFolderStoreQuery object
 *
 * Get a list of child folders from this folder store
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
 * Get a list of child folders from this folder store and call back when 
 * finished. 
 *
 * If you want to use this functionality, your application needs to use a glib 
 * main event loop (#GMainLoop). All Gtk+ and GNOME applications use this
 * automatically.
 *
 * The callback and the status_callback don't need gdk_threads_enter and
 * gdk_threads_leave because they are both invoked using g_timeout_add in the
 * main event loop of your application.
 *
 * DOC TODO: Document callback and status_callback
 * 
 **/
void 
tny_folder_store_iface_get_folders_async (TnyFolderStoreIface *self, TnyListIface *list, TnyGetFoldersCallback callback, TnyGetFoldersStatusCallback statuscb, TnyFolderStoreQuery *query, gpointer user_data)
{
#ifdef DEBUG
	if (!TNY_FOLDER_STORE_IFACE_GET_CLASS (self)->get_folders_async_func)
		g_critical ("You must implement tny_folder_store_iface_get_folders_async\n");
#endif

	TNY_FOLDER_STORE_IFACE_GET_CLASS (self)->get_folders_async_func (self, list, callback, statuscb, query, user_data);
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



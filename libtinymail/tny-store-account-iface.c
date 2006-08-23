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

#include <tny-store-account-iface.h>


/**
 * tny_store_account_iface_unsubscribe:
 * @self: a #TnyStoreAccountIface object
 * @folder: The folder to unsubscribe from
 *
 * WARNING: This API might soon change
 *
 * Unsubscribe from a folder
 * 
 **/
void
tny_store_account_iface_unsubscribe (TnyStoreAccountIface *self, TnyFolderIface *folder)
{
#ifdef DEBUG
	if (!TNY_STORE_ACCOUNT_IFACE_GET_CLASS (self)->unsubscribe_func)
		g_critical ("You must implement tny_store_account_iface_unsubscribe\n");
#endif

	TNY_STORE_ACCOUNT_IFACE_GET_CLASS (self)->unsubscribe_func (self, folder);
	return;
}

/**
 * tny_store_account_iface_subscribe:
 * @self: a #TnyStoreAccountIface object
 * @folder: The folder to subscribe to
 *
 * WARNING: This API might soon change
 *
 * Subscribe to a folder
 * 
 **/
void
tny_store_account_iface_subscribe (TnyStoreAccountIface *self, TnyFolderIface *folder)
{
#ifdef DEBUG
	if (!TNY_STORE_ACCOUNT_IFACE_GET_CLASS (self)->subscribe_func)
		g_critical ("You must implement tny_store_account_iface_subscribe\n");
#endif

	TNY_STORE_ACCOUNT_IFACE_GET_CLASS (self)->subscribe_func (self, folder);
	return;
}

/**
 * tny_store_account_iface_get_folders:
 * @self: a #TnyStoreAccountIface object
 * @type: Either all or only subscribed folders
 * 
 * WARNING: This API might soon change
 *
 * Get the folders of a storage account
 * 
 * Return value: A read-only #TnyListIface which contains #TnyFolderIface instances
 *
 **/
TnyListIface*
tny_store_account_iface_get_folders (TnyStoreAccountIface *self, TnyStoreAccountFolderType type)
{
#ifdef DEBUG
	if (!TNY_STORE_ACCOUNT_IFACE_GET_CLASS (self)->get_folders_func)
		g_critical ("You must implement tny_store_account_iface_get_folders\n");
#endif

	return TNY_STORE_ACCOUNT_IFACE_GET_CLASS (self)->get_folders_func (self, type);
}


static void
tny_store_account_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_store_account_iface_get_type (void)
{
	static GType type = 0;
	
	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyStoreAccountIfaceClass),
		  tny_store_account_iface_base_init,   /* base_init */
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
			"TnyStoreAccountIface", &info, 0);

		g_type_interface_add_prerequisite (type, TNY_TYPE_ACCOUNT_IFACE);
	}

	return type;
}

GType
tny_store_account_folder_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TNY_STORE_ACCOUNT_FOLDER_TYPE_SUBSCRIBED, "TNY_STORE_ACCOUNT_FOLDER_TYPE_SUBSCRIBED", "subscribed" },
      { TNY_STORE_ACCOUNT_FOLDER_TYPE_ALL, "TNY_STORE_ACCOUNT_FOLDER_TYPE_ALL", "all" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyStoreAccountFolderType", values);
  }
  return etype;
}

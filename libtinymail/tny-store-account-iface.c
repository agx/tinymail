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

#include <tny-store-account-iface.h>


/**
 * tny_store_account_iface_get_folders:
 * @self: a #TnyStoreAccountIface object
 * 
 * Get the folders of a storage account
 * 
 * Return value: A read-only GList which contains TnyFolderIface instances
 **/
const GList*
tny_store_account_iface_get_folders (TnyStoreAccountIface *self)
{
	return TNY_STORE_ACCOUNT_IFACE_GET_CLASS (self)->get_folders_func (self);
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
	if (type == 0) 
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
		  NULL    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyStoreAccountIface", &info, 0);

		g_type_interface_add_prerequisite (type, TNY_TYPE_ACCOUNT_IFACE);
	}

	return type;
}

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

#include <tny-store-account.h>
#include <tny-folder-store.h>
#include <tny-folder.h>

guint tny_store_account_signals [TNY_STORE_ACCOUNT_LAST_SIGNAL];


/**
 * tny_store_account_delete_cache:
 * @self: a #TnyStoreAccount object
 *
 * Delete the cache of a store account
 **/
void 
tny_store_account_delete_cache (TnyStoreAccount *self)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_STORE_ACCOUNT (self));
	g_assert (TNY_STORE_ACCOUNT_GET_IFACE (self)->delete_cache_func != NULL);
#endif

	TNY_STORE_ACCOUNT_GET_IFACE (self)->delete_cache_func (self);

	return;
}


/**
 * tny_store_account_find_folder:
 * @self: a #TnyStoreAccount object
 * @url_string: the url-string of the folder to find
 * @err: a #GError object
 *
 * Try to find the folder in @self that corresponds to @url_string. If this 
 * method does not return NULL, the returned value is the found folder and
 * must be unreferenced after use.
 *
 * Implementors: when implementing a #TnyStoreAccount, you must implement
 * this method. Let it return the folder that corresponds to @url_string or 
 * let it return NULL.
 *
 * This method can be used to resolve url-strings to #TnyAccount instances.
 * See tny_folder_get_url_string() for details of the @url-string syntax.
 *
 * Return value: the found account or NULL.
 **/
TnyFolder* 
tny_store_account_find_folder (TnyStoreAccount *self, const gchar *url_string, GError **err)
{
	TnyFolder *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_STORE_ACCOUNT (self));
	g_assert (url_string);
	g_assert (strlen (url_string) > 0);
	g_assert (strstr (url_string, "://"));
	g_assert (TNY_STORE_ACCOUNT_GET_IFACE (self)->find_folder_func != NULL);
#endif

	retval = TNY_STORE_ACCOUNT_GET_IFACE (self)->find_folder_func (self, url_string, err);

#ifdef DBC /* ensure */
	if (retval)
		g_assert (TNY_IS_FOLDER (retval));
#endif

	return retval;
}

/**
 * tny_store_account_unsubscribe:
 * @self: a #TnyStoreAccount object
 * @folder: The folder to unsubscribe from
 *
 * API WARNING: This API might change
 *
 * Unsubscribe from a folder
 * 
 **/
void
tny_store_account_unsubscribe (TnyStoreAccount *self, TnyFolder *folder)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_STORE_ACCOUNT (self));
	g_assert (folder);
	g_assert (TNY_IS_FOLDER (folder));
	g_assert (TNY_STORE_ACCOUNT_GET_IFACE (self)->unsubscribe_func != NULL);
#endif

	TNY_STORE_ACCOUNT_GET_IFACE (self)->unsubscribe_func (self, folder);
	return;
}

/**
 * tny_store_account_subscribe:
 * @self: a #TnyStoreAccount object
 * @folder: The folder to subscribe to
 *
 * API WARNING: This API might change
 *
 * Subscribe to a folder
 * 
 **/
void
tny_store_account_subscribe (TnyStoreAccount *self, TnyFolder *folder)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_STORE_ACCOUNT (self));
	g_assert (folder);
	g_assert (TNY_IS_FOLDER (folder));
	g_assert (TNY_STORE_ACCOUNT_GET_IFACE (self)->subscribe_func != NULL);
#endif

	TNY_STORE_ACCOUNT_GET_IFACE (self)->subscribe_func (self, folder);
	return;
}


static void
tny_store_account_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		
/** 
 * TnyStoreAccount::subscription-changed
 * @self: the object on which the signal is emitted
 * @arg1: the #TnyFolder of the folder whose subscription has changed
 * @user_data: user data set when the signal handler was connected
 *
 * Emitted when the subscription of a folder change
 **/
		tny_store_account_signals[TNY_STORE_ACCOUNT_SUBSCRIPTION_CHANGED] =
			g_signal_new ("subscription_changed",
				      TNY_TYPE_STORE_ACCOUNT,
				      G_SIGNAL_RUN_FIRST,
				      G_STRUCT_OFFSET (TnyStoreAccountIface, subscription_changed),
				      NULL, NULL,
				      g_cclosure_marshal_VOID__POINTER,
				      G_TYPE_NONE, 1, TNY_TYPE_FOLDER);


		initialized = TRUE;
	}
}

/**
 * tny_store_account_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
GType
tny_store_account_get_type (void)
{
	static GType type = 0;
	
	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyStoreAccountIface),
		  tny_store_account_base_init,   /* base_init */
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
			"TnyStoreAccount", &info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
		g_type_interface_add_prerequisite (type, TNY_TYPE_FOLDER_STORE);
		g_type_interface_add_prerequisite (type, TNY_TYPE_ACCOUNT);

	}

	return type;
}

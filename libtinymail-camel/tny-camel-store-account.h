#ifndef TNY_CAMEL_STORE_ACCOUNT_H
#define TNY_CAMEL_STORE_ACCOUNT_H

/* libtinymail-camel - The Tiny Mail base library for Camel
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

#include <glib.h>
#include <glib-object.h>

#include <tny-folder.h>
#include <tny-camel-account.h>
#include <tny-store-account.h>

G_BEGIN_DECLS

#define TNY_TYPE_CAMEL_STORE_ACCOUNT             (tny_camel_store_account_get_type ())
#define TNY_CAMEL_STORE_ACCOUNT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_CAMEL_STORE_ACCOUNT, TnyCamelStoreAccount))
#define TNY_CAMEL_STORE_ACCOUNT_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_CAMEL_STORE_ACCOUNT, TnyCamelStoreAccountClass))
#define TNY_IS_CAMEL_STORE_ACCOUNT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_CAMEL_STORE_ACCOUNT))
#define TNY_IS_CAMEL_STORE_ACCOUNT_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_CAMEL_STORE_ACCOUNT))
#define TNY_CAMEL_STORE_ACCOUNT_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_CAMEL_STORE_ACCOUNT, TnyCamelStoreAccountClass))

typedef struct _TnyCamelStoreAccount TnyCamelStoreAccount;
typedef struct _TnyCamelStoreAccountClass TnyCamelStoreAccountClass;

struct _TnyCamelStoreAccount
{
	TnyCamelAccount parent;
};

struct _TnyCamelStoreAccountClass 
{
	TnyCamelAccountClass parent;

	/* virtual methods */
	void (*get_folders_async_func) (TnyFolderStore *self, TnyList *list, TnyFolderStoreQuery *query, TnyGetFoldersCallback callback, TnyStatusCallback status_callback, gpointer user_data);
	void (*get_folders_func) (TnyFolderStore *self, TnyList *list, TnyFolderStoreQuery *query, GError **err);
	void (*remove_folder_func) (TnyFolderStore *self, TnyFolder *folder, GError **err);
	TnyFolder* (*create_folder_func) (TnyFolderStore *self, const gchar *name, GError **err);
	void (*create_folder_async_func) (TnyFolderStore *self, const gchar *name, TnyCreateFolderCallback callback, TnyStatusCallback status_callback, gpointer user_data);
	void (*add_observer_func) (TnyFolderStore *self, TnyFolderStoreObserver *observer);
	void (*remove_observer_func) (TnyFolderStore *self, TnyFolderStoreObserver *observer);

	TnyFolder * (*find_folder_func) (TnyStoreAccount *self, const gchar *url_string, GError **err);
	void (*delete_cache_func) (TnyStoreAccount *self);

	/* protected virtual methods*/
	TnyFolder * (*factor_folder_func) (TnyCamelStoreAccount *self, const gchar *full_name, gboolean *was_new);
};

GType tny_camel_store_account_get_type (void);
TnyStoreAccount* tny_camel_store_account_new (void);

TnyFolder* tny_camel_store_account_factor_folder (TnyCamelStoreAccount *self, const gchar *full_name, gboolean *was_new);

G_END_DECLS

#endif


#ifndef TNY_FOLDER_STORE_H
#define TNY_FOLDER_STORE_H

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

#include <glib.h>
#include <glib-object.h>
#include <tny-shared.h>

#include <tny-account.h>
#include <tny-folder.h>
#include <tny-list.h>
#include <tny-folder-store-query.h>

G_BEGIN_DECLS

#define TNY_TYPE_FOLDER_STORE             (tny_folder_store_get_type ())
#define TNY_FOLDER_STORE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_FOLDER_STORE, TnyFolderStore))
#define TNY_IS_FOLDER_STORE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_FOLDER_STORE))
#define TNY_FOLDER_STORE_GET_IFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_FOLDER_STORE, TnyFolderStoreIface))

#ifndef TNY_SHARED_H
typedef struct _TnyFolderStore TnyFolderStore;
typedef struct _TnyFolderStoreIface TnyFolderStoreIface;
#endif

struct _TnyFolderStoreIface
{
   GTypeInterface parent;

   void (*remove_folder_func) (TnyFolderStore *self, TnyFolder *folder, GError **err);
   TnyFolder* (*create_folder_func) (TnyFolderStore *self, const gchar *name, GError **err);
   void (*get_folders_func) (TnyFolderStore *self, TnyList *list, TnyFolderStoreQuery *query, GError **err);
   void (*get_folders_async_func) (TnyFolderStore *self, TnyList *list, TnyGetFoldersCallback callback, TnyFolderStoreQuery *query, gpointer user_data);
};

GType tny_folder_store_get_type (void);

void tny_folder_store_remove_folder (TnyFolderStore *self, TnyFolder *folder, GError **err);
TnyFolder *tny_folder_store_create_folder (TnyFolderStore *self, const gchar *name, GError **err);
void tny_folder_store_get_folders (TnyFolderStore *self, TnyList *list, TnyFolderStoreQuery *query, GError **err);
void tny_folder_store_get_folders_async (TnyFolderStore *self, TnyList *list, TnyGetFoldersCallback callback, TnyFolderStoreQuery *query, gpointer user_data);

G_END_DECLS

#endif

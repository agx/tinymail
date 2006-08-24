#ifndef TNY_FOLDER_STORE_IFACE_H
#define TNY_FOLDER_STORE_IFACE_H

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

#include <tny-account-iface.h>
#include <tny-folder-iface.h>
#include <tny-list-iface.h>
#include <tny-folder-store-query.h>

G_BEGIN_DECLS

#define TNY_TYPE_FOLDER_STORE_IFACE             (tny_folder_store_iface_get_type ())
#define TNY_FOLDER_STORE_IFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_FOLDER_STORE_IFACE, TnyFolderStoreIface))
#define TNY_FOLDER_STORE_IFACE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_FOLDER_STORE_IFACE, TnyFolderStoreIfaceClass))
#define TNY_IS_FOLDER_STORE_IFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_FOLDER_STORE_IFACE))
#define TNY_IS_FOLDER_STORE_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_FOLDER_STORE_IFACE))
#define TNY_FOLDER_STORE_IFACE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_FOLDER_STORE_IFACE, TnyFolderStoreIfaceClass))


struct _TnyFolderStoreIfaceClass
{
   GTypeInterface parent;

   void (*remove_folder_func) (TnyFolderStoreIface *self, TnyFolderIface *folder);
   TnyFolderIface* (*create_folder_func) (TnyFolderStoreIface *self, const gchar *name);
   void (*get_folders_func) (TnyFolderStoreIface *self, TnyListIface *list, TnyFolderStoreQuery *query);
   void (*get_folders_async_func) (TnyFolderStoreIface *self, TnyListIface *list, TnyGetFoldersCallback callback, TnyGetFoldersStatusCallback statuscb, TnyFolderStoreQuery *query, gpointer user_data);
};

GType tny_folder_store_iface_get_type (void);

void tny_folder_store_iface_remove_folder (TnyFolderStoreIface *self, TnyFolderIface *folder);
TnyFolderIface *tny_folder_store_iface_create_folder (TnyFolderStoreIface *self, const gchar *name);
void tny_folder_store_iface_get_folders (TnyFolderStoreIface *self, TnyListIface *list, TnyFolderStoreQuery *query);
void tny_folder_store_iface_get_folders_async (TnyFolderStoreIface *self, TnyListIface *list, TnyGetFoldersCallback callback, TnyGetFoldersStatusCallback statuscb, TnyFolderStoreQuery *query, gpointer user_data);

G_END_DECLS

#endif

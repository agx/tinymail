#ifndef TNY_STORE_ACCOUNT_IFACE_H
#define TNY_STORE_ACCOUNT_IFACE_H

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
#include <tny-msg-folder-iface.h>
#include <tny-list-iface.h>

G_BEGIN_DECLS

#define TNY_TYPE_STORE_ACCOUNT_IFACE             (tny_store_account_iface_get_type ())
#define TNY_STORE_ACCOUNT_IFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_STORE_ACCOUNT_IFACE, TnyStoreAccountIface))
#define TNY_STORE_ACCOUNT_IFACE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_STORE_ACCOUNT_IFACE, TnyStoreAccountIfaceClass))
#define TNY_IS_STORE_ACCOUNT_IFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_STORE_ACCOUNT_IFACE))
#define TNY_IS_STORE_ACCOUNT_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_STORE_ACCOUNT_IFACE))
#define TNY_STORE_ACCOUNT_IFACE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_STORE_ACCOUNT_IFACE, TnyStoreAccountIfaceClass))

#define TNY_TYPE_STORE_ACCOUNT_FOLDER_TYPE (tny_store_account_folder_type_get_type())

enum _TnyStoreAccountFolderType
{
	TNY_STORE_ACCOUNT_FOLDER_TYPE_SUBSCRIBED,
	TNY_STORE_ACCOUNT_FOLDER_TYPE_ALL
};

struct _TnyStoreAccountIfaceClass
{
	GTypeInterface parent;

	const TnyListIface* 
	     (*get_folders_func)           (TnyStoreAccountIface *self, TnyStoreAccountFolderType type);
	void (*subscribe_func)             (TnyStoreAccountIface *self, TnyMsgFolderIface *folder);
	void (*unsubscribe_func)           (TnyStoreAccountIface *self, TnyMsgFolderIface *folder);
};

GType tny_store_account_iface_get_type (void);
GType tny_store_account_folder_type_get_type (void);

const TnyListIface* tny_store_account_iface_get_folders (TnyStoreAccountIface *self, TnyStoreAccountFolderType type);
void tny_store_account_iface_subscribe (TnyStoreAccountIface *self, TnyMsgFolderIface *folder);
void tny_store_account_iface_unsubscribe (TnyStoreAccountIface *self, TnyMsgFolderIface *folder);


G_END_DECLS

#endif

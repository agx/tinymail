#ifndef TNY_CAMEL_FOLDER_H
#define TNY_CAMEL_FOLDER_H

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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <glib.h>
#include <glib-object.h>

#include <tny-msg.h>
#include <tny-header.h>

#include <tny-msg-remove-strategy.h>
#include <tny-msg-receive-strategy.h>

#include <camel/camel-folder.h>

G_BEGIN_DECLS

#define TNY_TYPE_CAMEL_FOLDER             (tny_camel_folder_get_type ())
#define TNY_CAMEL_FOLDER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_CAMEL_FOLDER, TnyCamelFolder))
#define TNY_CAMEL_FOLDER_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_CAMEL_FOLDER, TnyCamelFolderClass))
#define TNY_IS_CAMEL_FOLDER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_CAMEL_FOLDER))
#define TNY_IS_CAMEL_FOLDER_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_CAMEL_FOLDER))
#define TNY_CAMEL_FOLDER_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_CAMEL_FOLDER, TnyCamelFolderClass))

typedef struct _TnyCamelFolder TnyCamelFolder;
typedef struct _TnyCamelFolderClass TnyCamelFolderClass;

struct _TnyCamelFolder
{
	GObject parent;
};

struct _TnyCamelFolderClass 
{
	GObjectClass parent;

	/* virtual methods */
	void (*add_msg_func) (TnyFolder *self, TnyMsg *msg, GError **err);
	void (*remove_msg_func) (TnyFolder *self, TnyHeader *header, GError **err);
	void (*sync_func) (TnyFolder *self, gboolean expunge, GError **err);
	TnyMsgRemoveStrategy* (*get_msg_remove_strategy_func) (TnyFolder *self);
	void (*set_msg_remove_strategy_func) (TnyFolder *self, TnyMsgRemoveStrategy *st);
	TnyMsgReceiveStrategy* (*get_msg_receive_strategy_func) (TnyFolder *self);
	void (*set_msg_receive_strategy_func) (TnyFolder *self, TnyMsgReceiveStrategy *st);
	TnyMsg* (*get_msg_func) (TnyFolder *self, TnyHeader *header, GError **err);
	TnyMsg* (*find_msg_func) (TnyFolder *self, const gchar *url_string, GError **err);
	void (*get_msg_async_func) (TnyFolder *self, TnyHeader *header, TnyGetMsgCallback callback, TnyStatusCallback status_callback, gpointer user_data);
	void (*get_headers_func) (TnyFolder *self, TnyList *headers, gboolean refresh, GError **err);
	const gchar* (*get_name_func) (TnyFolder *self);
	const gchar* (*get_id_func) (TnyFolder *self);
	TnyAccount* (*get_account_func) (TnyFolder *self);
	void (*set_name_func) (TnyFolder *self, const gchar *name, GError **err);
	TnyFolderType (*get_folder_type_func) (TnyFolder *self);
	guint (*get_all_count_func) (TnyFolder *self);
	guint (*get_unread_count_func) (TnyFolder *self);
	gboolean (*is_subscribed_func) (TnyFolder *self);
	void (*refresh_async_func) (TnyFolder *self, TnyRefreshFolderCallback callback, TnyStatusCallback status_callback, gpointer user_data);
	void (*refresh_func) (TnyFolder *self, GError **err);
	void (*transfer_msgs_func) (TnyFolder *self, TnyList *headers, TnyFolder *folder_dst, gboolean delete_originals, GError **err);
	void (*transfer_msgs_async_func) (TnyFolder *self, TnyList *header_list, TnyFolder *folder_dst, gboolean delete_originals, TnyTransferMsgsCallback callback, gpointer user_data);
	TnyFolder* (*copy_func) (TnyFolder *self, TnyFolderStore *into, const gchar *new_name, gboolean del, GError **err);
	void (*poke_status_func) (TnyFolder *self);
	void (*add_observer_func) (TnyFolder *self, TnyFolderObserver *observer);
	void (*remove_observer_func) (TnyFolder *self, TnyFolderObserver *observer);
	TnyFolderStats* (*get_stats_func) (TnyFolder *self);
	gchar* (*get_url_string_func) (TnyFolder *self);
	TnyFolderCaps (*get_caps_func) (TnyFolder *self);

	void (*get_folders_async_func) (TnyFolderStore *self, TnyList *list, TnyGetFoldersCallback callback, TnyFolderStoreQuery *query, gpointer user_data);
	void (*get_folders_func) (TnyFolderStore *self, TnyList *list, TnyFolderStoreQuery *query, GError **err);
	void (*remove_folder_func) (TnyFolderStore *self, TnyFolder *folder, GError **err);
	TnyFolder* (*create_folder_func) (TnyFolderStore *self, const gchar *name, GError **err);
	TnyFolderStore* (*get_folder_store_func) (TnyFolder *self);
	void (*add_store_observer_func) (TnyFolderStore *self, TnyFolderStoreObserver *observer);
	void (*remove_store_observer_func) (TnyFolderStore *self, TnyFolderStoreObserver *observer);

};

GType tny_camel_folder_get_type (void);


CamelFolder* tny_camel_folder_get_folder (TnyCamelFolder *self);

G_END_DECLS

#endif


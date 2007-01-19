#ifndef TNY_FOLDER_H
#define TNY_FOLDER_H

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

#include <tny-shared.h>
#include <tny-msg.h>
#include <tny-header.h>
#include <tny-account.h>
#include <tny-msg-remove-strategy.h>
#include <tny-list.h>

G_BEGIN_DECLS

#define TNY_TYPE_FOLDER             (tny_folder_get_type ())
#define TNY_FOLDER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_FOLDER, TnyFolder))
#define TNY_IS_FOLDER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_FOLDER))
#define TNY_FOLDER_GET_IFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_FOLDER, TnyFolderIface))

#ifndef TNY_SHARED_H
typedef enum _TnyFolderType TnyFolderType;
typedef struct _TnyFolder TnyFolder;
typedef struct _TnyFolderIface TnyFolderIface;
#endif

enum _TnyFolderSignal
{
	TNY_FOLDER_FOLDER_INSERTED,
	TNY_FOLDER_FOLDERS_RELOADED,
	TNY_FOLDER_LAST_SIGNAL
};

extern guint tny_folder_signals[TNY_FOLDER_LAST_SIGNAL];

#define TNY_TYPE_FOLDER_TYPE (tny_folder_type_get_type())

enum _TnyFolderType
{
	TNY_FOLDER_TYPE_UNKNOWN,
	TNY_FOLDER_TYPE_NORMAL,
	TNY_FOLDER_TYPE_INBOX,
	TNY_FOLDER_TYPE_OUTBOX,
	TNY_FOLDER_TYPE_TRASH,
	TNY_FOLDER_TYPE_JUNK,
	TNY_FOLDER_TYPE_SENT,
	TNY_FOLDER_TYPE_ROOT,
	TNY_FOLDER_TYPE_NOTES,
	TNY_FOLDER_TYPE_DRAFTS,
	TNY_FOLDER_TYPE_CONTACTS,
	TNY_FOLDER_TYPE_CALENDAR
};

struct _TnyFolderIface
{
	GTypeInterface parent;
	
	/* Methods */
	void (*remove_msg_func) (TnyFolder *self, TnyHeader *header, GError **err);
	void (*add_msg_func) (TnyFolder *self, TnyMsg *msg, GError **err);
	void (*expunge_func) (TnyFolder *self, GError **err);
	TnyMsgRemoveStrategy* (*get_msg_remove_strategy_func) (TnyFolder *self);
	void (*set_msg_remove_strategy_func) (TnyFolder *self, TnyMsgRemoveStrategy *st);
	TnyMsgReceiveStrategy* (*get_msg_receive_strategy_func) (TnyFolder *self);
	void (*set_msg_receive_strategy_func) (TnyFolder *self, TnyMsgReceiveStrategy *st);
	TnyMsg* (*get_msg_func) (TnyFolder *self, TnyHeader *header, GError **err);
	void (*get_msg_async_func) (TnyFolder *self, TnyHeader *header, TnyGetMsgCallback callback, gpointer user_data);
	void (*get_headers_func) (TnyFolder *self, TnyList *headers, gboolean refresh, GError **err);
	const gchar* (*get_name_func) (TnyFolder *self);
	const gchar* (*get_id_func) (TnyFolder *self);
	TnyAccount* (*get_account_func) (TnyFolder *self);
	void (*set_name_func) (TnyFolder *self, const gchar *name, GError **err);
	TnyFolderType (*get_folder_type_func) (TnyFolder *self);
	guint (*get_all_count_func) (TnyFolder *self);
	guint (*get_unread_count_func) (TnyFolder *self);
	gboolean (*is_subscribed_func) (TnyFolder *self);
	void (*refresh_async_func) (TnyFolder *self, TnyRefreshFolderCallback callback, TnyRefreshFolderStatusCallback status_callback, gpointer user_data);
	void (*refresh_func) (TnyFolder *self, GError **err);
	void (*transfer_msgs_func) (TnyFolder *self, TnyList *header_list, TnyFolder *folder_dst, gboolean delete_originals, GError **err);
	void (*transfer_msgs_async_func) (TnyFolder *self, TnyList *header_list, TnyFolder *folder_dst, gboolean delete_originals, TnyTransferMsgsCallback callback, gpointer user_data);
	TnyFolder* (*copy_func) (TnyFolder *self, TnyFolderStore *into, const gchar *new_name, gboolean del, GError **err);
	void (*poke_status_func) (TnyFolder *self);
	void (*add_observer_func) (TnyFolder *self, TnyFolderObserver *observer);
	void (*remove_observer_func) (TnyFolder *self, TnyFolderObserver *observer);

};

GType tny_folder_get_type (void);
GType tny_folder_type_get_type (void);

TnyMsgRemoveStrategy* tny_folder_get_msg_remove_strategy (TnyFolder *self);
void tny_folder_set_msg_remove_strategy (TnyFolder *self, TnyMsgRemoveStrategy *st);
TnyMsgReceiveStrategy* tny_folder_get_msg_receive_strategy (TnyFolder *self);
void tny_folder_set_msg_receive_strategy (TnyFolder *self, TnyMsgReceiveStrategy *st);
void tny_folder_remove_msg (TnyFolder *self, TnyHeader *header, GError **err);
void tny_folder_add_msg (TnyFolder *self, TnyMsg *msg, GError **err);
void tny_folder_expunge (TnyFolder *self, GError **err);
TnyMsg* tny_folder_get_msg (TnyFolder *self, TnyHeader *header, GError **err);
void tny_folder_get_msg_async (TnyFolder *self, TnyHeader *header, TnyGetMsgCallback callback, gpointer user_data);
void tny_folder_get_headers (TnyFolder *self, TnyList *headers, gboolean refresh, GError **err);
TnyAccount* tny_folder_get_account (TnyFolder *self);
const gchar* tny_folder_get_id (TnyFolder *self);
const gchar* tny_folder_get_name (TnyFolder *self);
void tny_folder_set_name (TnyFolder *self, const gchar *name, GError **err);
TnyFolderType tny_folder_get_folder_type (TnyFolder *self);
guint tny_folder_get_all_count (TnyFolder *self);
guint tny_folder_get_unread_count (TnyFolder *self);
gboolean tny_folder_is_subscribed (TnyFolder *self);
void tny_folder_refresh_async (TnyFolder *self, TnyRefreshFolderCallback callback, TnyRefreshFolderStatusCallback status_callback, gpointer user_data);
void tny_folder_refresh (TnyFolder *self, GError **err);
void tny_folder_transfer_msgs (TnyFolder *self, TnyList *header_list, TnyFolder *folder_dst, gboolean delete_originals, GError **err);
void tny_folder_transfer_msgs_async (TnyFolder *self, TnyList *header_list, TnyFolder *folder_dst, gboolean delete_originals, TnyTransferMsgsCallback callback, gpointer user_data);
TnyFolder* tny_folder_copy (TnyFolder *self, TnyFolderStore *into, const gchar *new_name, gboolean del, GError **err);
void tny_folder_poke_status (TnyFolder *self);
void tny_folder_add_observer (TnyFolder *self, TnyFolderObserver *observer);
void tny_folder_remove_observer (TnyFolder *self, TnyFolderObserver *observer);


G_END_DECLS

#endif

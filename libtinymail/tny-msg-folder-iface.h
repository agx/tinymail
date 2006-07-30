#ifndef TNY_MSG_FOLDER_IFACE_H
#define TNY_MSG_FOLDER_IFACE_H

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
#include <tny-msg-iface.h>
#include <tny-msg-header-iface.h>
#include <tny-account-iface.h>
#include <tny-list-iface.h>

G_BEGIN_DECLS

#define TNY_TYPE_MSG_FOLDER_IFACE             (tny_msg_folder_iface_get_type ())
#define TNY_MSG_FOLDER_IFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_MSG_FOLDER_IFACE, TnyMsgFolderIface))
#define TNY_MSG_FOLDER_IFACE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_MSG_FOLDER_IFACE, TnyMsgFolderIfaceClass))
#define TNY_IS_MSG_FOLDER_IFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_MSG_FOLDER_IFACE))
#define TNY_IS_MSG_FOLDER_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_MSG_FOLDER_IFACE))
#define TNY_MSG_FOLDER_IFACE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_MSG_FOLDER_IFACE, TnyMsgFolderIfaceClass))

#ifndef TNY_MSG_FOLDER_IFACE_C
extern guint *tny_msg_folder_iface_signals;
#endif

enum
{
	TNY_MSG_FOLDER_IFACE_FOLDER_INSERTED,
	TNY_MSG_FOLDER_IFACE_FOLDERS_RELOADED,
	TNY_MSG_FOLDER_IFACE_LAST_SIGNAL
};

#define TNY_TYPE_MSG_FOLDER_TYPE (tny_msg_folder_type_get_type())

enum _TnyMsgFolderType
{
	TNY_MSG_FOLDER_TYPE_UNKNOWN,
	TNY_MSG_FOLDER_TYPE_NORMAL,
	TNY_MSG_FOLDER_TYPE_INBOX,
	TNY_MSG_FOLDER_TYPE_OUTBOX,
	TNY_MSG_FOLDER_TYPE_TRASH,
	TNY_MSG_FOLDER_TYPE_JUNK,
	TNY_MSG_FOLDER_TYPE_SENT
};
	

/* TODO: Removing folders, Moving messages */

struct _TnyMsgFolderIfaceClass
{
	GTypeInterface parent;
	
	/* Signals */
	void (*folder_inserted)            (TnyMsgFolderIface *self, TnyMsgFolderIface *folder);
	void (*folders_reloaded)           (TnyMsgFolderIface *self);


	/* Methods */
	void (*remove_message_func) (TnyMsgFolderIface *self, TnyMsgHeaderIface *header);
	void (*expunge_func)        (TnyMsgFolderIface *self);

	TnyListIface*
		      (*get_folders_func)  (TnyMsgFolderIface *self);

	TnyMsgIface*  
                       (*get_message_func)  (TnyMsgFolderIface *self, TnyMsgHeaderIface *header);
	
	void	       (*get_headers_func)  (TnyMsgFolderIface *self, TnyListIface *headers, gboolean refresh);

	const gchar*   (*get_name_func)    (TnyMsgFolderIface *self);
	const gchar*   (*get_id_func)      (TnyMsgFolderIface *self);
	TnyAccountIface*   
                       (*get_account_func) (TnyMsgFolderIface *self);

	void           (*set_name_func)    (TnyMsgFolderIface *self, const gchar *name);
	void           (*set_id_func)      (TnyMsgFolderIface *self, const gchar *id);
	void           (*set_account_func) (TnyMsgFolderIface *self, TnyAccountIface *account);
	TnyMsgFolderType (*get_folder_type_func) (TnyMsgFolderIface *self);

	guint           (*get_all_count_func)     (TnyMsgFolderIface *self);	
	guint           (*get_unread_count_func)  (TnyMsgFolderIface *self);

	void           (*uncache_func)     (TnyMsgFolderIface *self);
	gboolean       (*has_cache_func)   (TnyMsgFolderIface *self);

	void           (*set_subscribed_func)    (TnyMsgFolderIface *self, gboolean subscribed);
	gboolean       (*get_subscribed_func)    (TnyMsgFolderIface *self);

	void           (*refresh_async_func) (TnyMsgFolderIface *self, TnyRefreshFolderCallback callback, TnyRefreshFolderStatusCallback status_callback, gpointer user_data);
	void           (*refresh_func) (TnyMsgFolderIface *self);
};

GType tny_msg_folder_iface_get_type (void);
GType tny_msg_folder_type_get_type (void);

void tny_msg_folder_iface_remove_message (TnyMsgFolderIface *self, TnyMsgHeaderIface *header);
void tny_msg_folder_iface_expunge (TnyMsgFolderIface *self);

TnyListIface* tny_msg_folder_iface_get_folders (TnyMsgFolderIface *self);

TnyMsgIface* tny_msg_folder_iface_get_message (TnyMsgFolderIface *self, TnyMsgHeaderIface *header);
void tny_msg_folder_iface_get_headers (TnyMsgFolderIface *self, TnyListIface *headers, gboolean refresh);

TnyAccountIface* tny_msg_folder_iface_get_account (TnyMsgFolderIface *self);
const gchar* tny_msg_folder_iface_get_id (TnyMsgFolderIface *self);
const gchar* tny_msg_folder_iface_get_name (TnyMsgFolderIface *self);

void tny_msg_folder_iface_set_account (TnyMsgFolderIface *self, TnyAccountIface *account);
void tny_msg_folder_iface_set_id (TnyMsgFolderIface *self, const gchar *id);
void tny_msg_folder_iface_set_name (TnyMsgFolderIface *self, const gchar *name);

TnyMsgFolderType tny_msg_folder_iface_get_folder_type (TnyMsgFolderIface *self);

guint tny_msg_folder_iface_get_all_count (TnyMsgFolderIface *self);
guint tny_msg_folder_iface_get_unread_count (TnyMsgFolderIface *self);
void tny_msg_folder_iface_uncache (TnyMsgFolderIface *self);

void tny_msg_folder_iface_set_subscribed (TnyMsgFolderIface *self, gboolean subscribed);
gboolean tny_msg_folder_iface_get_subscribed (TnyMsgFolderIface *self);

void tny_msg_folder_iface_refresh_async (TnyMsgFolderIface *self, TnyRefreshFolderCallback callback, TnyRefreshFolderStatusCallback status_callback, gpointer user_data);
void tny_msg_folder_iface_refresh (TnyMsgFolderIface *self);

G_END_DECLS

#endif

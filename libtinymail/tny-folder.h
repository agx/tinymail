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
#include <tny-list.h>

G_BEGIN_DECLS

#define TNY_TYPE_FOLDER             (tny_folder_get_type ())
#define TNY_FOLDER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_FOLDER, TnyFolder))
#define TNY_IS_FOLDER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_FOLDER))
#define TNY_FOLDER_GET_IFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_FOLDER, TnyFolderIface))

#ifndef TNY_FOLDER_C
extern guint *tny_folder_signals;
#endif

#ifndef TNY_SHARED_H
typedef enum _TnyFolderType TnyFolderType;
typedef struct _TnyFolder TnyFolder;
typedef struct _TnyFolderIface TnyFolderIface;
#endif

enum
{
	TNY_FOLDER_FOLDER_INSERTED,
	TNY_FOLDER_FOLDERS_RELOADED,
	TNY_FOLDER_LAST_SIGNAL
};

#define TNY_TYPE_FOLDER_TYPE (tny_folder_type_get_type())

enum _TnyFolderType
{
	TNY_FOLDER_TYPE_UNKNOWN,
	TNY_FOLDER_TYPE_NORMAL,
	TNY_FOLDER_TYPE_INBOX,
	TNY_FOLDER_TYPE_OUTBOX,
	TNY_FOLDER_TYPE_TRASH,
	TNY_FOLDER_TYPE_JUNK,
	TNY_FOLDER_TYPE_SENT
};
	
/* TODO: Moving messages */

struct _TnyFolderIface
{
	GTypeInterface parent;
	
	/* Signals */
	void (*folder_inserted) (TnyFolder *self, TnyFolder *folder);
	void (*folders_reloaded) (TnyFolder *self);

	/* Methods */
	void (*remove_message_func) (TnyFolder *self, TnyHeader *header);
	void (*expunge_func) (TnyFolder *self);
	TnyMsg* (*get_message_func) (TnyFolder *self, TnyHeader *header);
	void (*get_headers_func) (TnyFolder *self, TnyList *headers, gboolean refresh);
	const gchar* (*get_name_func) (TnyFolder *self);
	const gchar* (*get_id_func) (TnyFolder *self);
	TnyStoreAccount* (*get_account_func) (TnyFolder *self);
	void (*set_name_func) (TnyFolder *self, const gchar *name);
	TnyFolderType (*get_folder_type_func) (TnyFolder *self);
	guint (*get_all_count_func) (TnyFolder *self);	
	guint (*get_unread_count_func) (TnyFolder *self);
	void (*set_subscribed_func) (TnyFolder *self, gboolean subscribed);
	gboolean (*get_subscribed_func) (TnyFolder *self);
	void (*refresh_async_func) (TnyFolder *self, TnyRefreshFolderCallback callback, TnyRefreshFolderStatusCallback status_callback, gpointer user_data);
	void (*refresh_func) (TnyFolder *self);
};

GType tny_folder_get_type (void);
GType tny_folder_type_get_type (void);

void tny_folder_remove_message (TnyFolder *self, TnyHeader *header);
void tny_folder_expunge (TnyFolder *self);
TnyMsg* tny_folder_get_message (TnyFolder *self, TnyHeader *header);
void tny_folder_get_headers (TnyFolder *self, TnyList *headers, gboolean refresh);
TnyStoreAccount* tny_folder_get_account (TnyFolder *self);
const gchar* tny_folder_get_id (TnyFolder *self);
const gchar* tny_folder_get_name (TnyFolder *self);
void tny_folder_set_name (TnyFolder *self, const gchar *name);
TnyFolderType tny_folder_get_folder_type (TnyFolder *self);
guint tny_folder_get_all_count (TnyFolder *self);
guint tny_folder_get_unread_count (TnyFolder *self);
void tny_folder_set_subscribed (TnyFolder *self, gboolean subscribed);
gboolean tny_folder_get_subscribed (TnyFolder *self);
void tny_folder_refresh_async (TnyFolder *self, TnyRefreshFolderCallback callback, TnyRefreshFolderStatusCallback status_callback, gpointer user_data);
void tny_folder_refresh (TnyFolder *self);

G_END_DECLS

#endif

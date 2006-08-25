#ifndef TNY_FOLDER_IFACE_H
#define TNY_FOLDER_IFACE_H

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
#include <tny-header-iface.h>
#include <tny-account-iface.h>
#include <tny-list-iface.h>

G_BEGIN_DECLS

#define TNY_TYPE_FOLDER_IFACE             (tny_folder_iface_get_type ())
#define TNY_FOLDER_IFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_FOLDER_IFACE, TnyFolderIface))
#define TNY_FOLDER_IFACE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_FOLDER_IFACE, TnyFolderIfaceClass))
#define TNY_IS_FOLDER_IFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_FOLDER_IFACE))
#define TNY_IS_FOLDER_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_FOLDER_IFACE))
#define TNY_FOLDER_IFACE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_FOLDER_IFACE, TnyFolderIfaceClass))

#ifndef TNY_FOLDER_IFACE_C
extern guint *tny_folder_iface_signals;
#endif

enum
{
	TNY_FOLDER_IFACE_FOLDER_INSERTED,
	TNY_FOLDER_IFACE_FOLDERS_RELOADED,
	TNY_FOLDER_IFACE_LAST_SIGNAL
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
	

/* TODO: Removing folders, Moving messages */

struct _TnyFolderIfaceClass
{
	GTypeInterface parent;
	
	/* Signals */
	void (*folder_inserted)            (TnyFolderIface *self, TnyFolderIface *folder);
	void (*folders_reloaded)           (TnyFolderIface *self);


	/* Methods */
	void (*remove_message_func) (TnyFolderIface *self, TnyHeaderIface *header);
	void (*expunge_func)        (TnyFolderIface *self);

	TnyMsgIface*  
                       (*get_message_func)  (TnyFolderIface *self, TnyHeaderIface *header);
	
	void	       (*get_headers_func)  (TnyFolderIface *self, TnyListIface *headers, gboolean refresh);

	const gchar*   (*get_name_func)    (TnyFolderIface *self);
	const gchar*   (*get_id_func)      (TnyFolderIface *self);
	TnyAccountIface*   
                       (*get_account_func) (TnyFolderIface *self);

	void           (*set_name_func)    (TnyFolderIface *self, const gchar *name);
	void           (*set_account_func) (TnyFolderIface *self, TnyAccountIface *account);
	TnyFolderType (*get_folder_type_func) (TnyFolderIface *self);

	guint           (*get_all_count_func)     (TnyFolderIface *self);	
	guint           (*get_unread_count_func)  (TnyFolderIface *self);

	void           (*set_subscribed_func)    (TnyFolderIface *self, gboolean subscribed);
	gboolean       (*get_subscribed_func)    (TnyFolderIface *self);

	void           (*refresh_async_func) (TnyFolderIface *self, TnyRefreshFolderCallback callback, TnyRefreshFolderStatusCallback status_callback, gpointer user_data);
	void           (*refresh_func) (TnyFolderIface *self);
};

GType tny_folder_iface_get_type (void);
GType tny_folder_type_get_type (void);

void tny_folder_iface_remove_message (TnyFolderIface *self, TnyHeaderIface *header);
void tny_folder_iface_expunge (TnyFolderIface *self);

TnyMsgIface* tny_folder_iface_get_message (TnyFolderIface *self, TnyHeaderIface *header);
void tny_folder_iface_get_headers (TnyFolderIface *self, TnyListIface *headers, gboolean refresh);

TnyAccountIface* tny_folder_iface_get_account (TnyFolderIface *self);
const gchar* tny_folder_iface_get_id (TnyFolderIface *self);
const gchar* tny_folder_iface_get_name (TnyFolderIface *self);

void tny_folder_iface_set_account (TnyFolderIface *self, TnyAccountIface *account);
void tny_folder_iface_set_name (TnyFolderIface *self, const gchar *name);

TnyFolderType tny_folder_iface_get_folder_type (TnyFolderIface *self);

guint tny_folder_iface_get_all_count (TnyFolderIface *self);
guint tny_folder_iface_get_unread_count (TnyFolderIface *self);

void tny_folder_iface_set_subscribed (TnyFolderIface *self, gboolean subscribed);
gboolean tny_folder_iface_get_subscribed (TnyFolderIface *self);

void tny_folder_iface_refresh_async (TnyFolderIface *self, TnyRefreshFolderCallback callback, TnyRefreshFolderStatusCallback status_callback, gpointer user_data);
void tny_folder_iface_refresh (TnyFolderIface *self);

G_END_DECLS

#endif

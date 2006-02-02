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
#include <tny-msg-account-iface.h>

G_BEGIN_DECLS

#define TNY_MSG_FOLDER_IFACE_TYPE             (tny_msg_folder_iface_get_type ())
#define TNY_MSG_FOLDER_IFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_MSG_FOLDER_IFACE_TYPE, TnyMsgFolderIface))
#define TNY_MSG_FOLDER_IFACE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_MSG_FOLDER_IFACE_TYPE, TnyMsgFolderIfaceClass))
#define TNY_IS_MSG_FOLDER_IFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_MSG_FOLDER_IFACE_TYPE))
#define TNY_IS_MSG_FOLDER_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_MSG_FOLDER_IFACE_TYPE))
#define TNY_MSG_FOLDER_IFACE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_MSG_FOLDER_IFACE_TYPE, TnyMsgFolderIfaceClass))

struct _TnyMsgFolderIfaceClass
{
	GTypeInterface parent;

	const GList*  (*get_folders_func)  (TnyMsgFolderIface *self);
	void          (*add_folder_func)   (TnyMsgFolderIface *self, TnyMsgFolderIface *folder);

	const TnyMsgIface*  
                       (*get_message_func)  (TnyMsgFolderIface *self, TnyMsgHeaderIface *header);
	const GList*   (*get_headers_func)  (TnyMsgFolderIface *self);

	const gchar*   (*get_name_func)    (TnyMsgFolderIface *self);
	const gchar*   (*get_id_func)      (TnyMsgFolderIface *self);
	const TnyMsgAccountIface*   
                       (*get_account_func) (TnyMsgFolderIface *self);

	void           (*set_name_func)    (TnyMsgFolderIface *self, const gchar *name);
	void           (*set_id_func)      (TnyMsgFolderIface *self, const gchar *id);
	void           (*set_account_func) (TnyMsgFolderIface *self, const TnyMsgAccountIface *account);
	
	void           (*uncache_func)     (TnyMsgFolderIface *self);
	const gboolean (*has_cache_func)   (TnyMsgFolderIface *self);
};

GType          tny_msg_folder_iface_get_type     (void);

const GList*   tny_msg_folder_iface_get_folders  (TnyMsgFolderIface *self);
void           tny_msg_folder_iface_add_folder   (TnyMsgFolderIface *self, TnyMsgFolderIface *folder);

const TnyMsgIface*
               tny_msg_folder_iface_get_message  (TnyMsgFolderIface *self, TnyMsgHeaderIface *header);
const GList*   tny_msg_folder_iface_get_headers  (TnyMsgFolderIface *self);

const TnyMsgAccountIface*  
               tny_msg_folder_iface_get_account  (TnyMsgFolderIface *self);
const gchar*   tny_msg_folder_iface_get_id       (TnyMsgFolderIface *self);
const gchar*   tny_msg_folder_iface_get_name     (TnyMsgFolderIface *self);

void           tny_msg_folder_iface_set_account  (TnyMsgFolderIface *self, const TnyMsgAccountIface *account);
void           tny_msg_folder_iface_set_id       (TnyMsgFolderIface *self, const gchar *id);
void           tny_msg_folder_iface_set_name     (TnyMsgFolderIface *self, const gchar *name);

void           tny_msg_folder_iface_uncache       (TnyMsgFolderIface *self);
const gboolean tny_msg_folder_iface_has_cache     (TnyMsgFolderIface *self);

G_END_DECLS

#endif

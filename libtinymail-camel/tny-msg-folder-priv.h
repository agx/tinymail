#ifndef TNY_MSG_FOLDER_PRIV_H
#define TNY_MSG_FOLDER_PRIV_H

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
#include <camel/camel-folder.h>
#include <tny-account-iface.h>
#include <tny-msg-folder-iface.h>

typedef struct _TnyMsgFolderPriv TnyMsgFolderPriv;

struct _TnyMsgFolderPriv
{
	GPtrArray *cached_uids;
	gboolean loaded;

	GType headers_list_type;

	GMutex *cached_msgs_lock;
	GHashTable *cached_msgs;

	guint folder_changed_id;

	GMutex *folder_lock;
	CamelFolder *folder;

	gchar *folder_name;
	TnyAccountIface *account;

	GMutex *folders_lock;
	TnyListIface *folders;
	guint cached_length, unread_length;

	gboolean subscribed;
	gboolean has_summary_cap;

	gchar *cached_name;
};

CamelFolder* _tny_msg_folder_get_camel_folder (TnyMsgFolderIface *self);

void _tny_msg_folder_set_subscribed_priv (TnyMsgFolderIface *self, gboolean subscribed);
void _tny_msg_folder_set_name_priv (TnyMsgFolderIface *self, const gchar *name);

#define TNY_MSG_FOLDER_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_MSG_FOLDER, TnyMsgFolderPriv))


#endif

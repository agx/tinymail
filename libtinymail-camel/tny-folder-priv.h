#ifndef TNY_FOLDER_PRIV_H
#define TNY_FOLDER_PRIV_H

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
#include <camel/camel-store.h>
#include <tny-account-iface.h>
#include <tny-folder-iface.h>

typedef struct _TnyFolderPriv TnyFolderPriv;

struct _TnyFolderPriv
{
	gboolean loaded;

	GType headers_list_type;

	guint folder_changed_id;
	guint headers_managed;
    
	GMutex *folder_lock;
	CamelFolder *folder;

	gchar *folder_name;
	TnyAccountIface *account;

	GMutex *folders_lock;
	TnyListIface *folders;
	guint cached_length, unread_length;

	gboolean subscribed;
	gboolean has_summary_cap;
    
	CamelFolderInfo *iter;
	gboolean iter_parented;
    
	gchar *cached_name;
	TnyFolderType cached_folder_type;
};

CamelFolder* _tny_folder_get_camel_folder (TnyFolderIface *self);

void _tny_folder_set_id (TnyFolder *self, const gchar *id);
void _tny_folder_set_subscribed (TnyFolder *self, gboolean subscribed);
void _tny_folder_set_name (TnyFolder *self, const gchar *name);
void _tny_folder_set_folder_type (TnyFolder *folder, CamelFolderInfo *folder_info);
void _tny_folder_set_unread_count (TnyFolder *self, guint len);
void _tny_folder_set_all_count (TnyFolder *self, guint len);
void _tny_folder_check_uncache (TnyFolder *self, TnyFolderPriv *priv);
void _tny_folder_set_iter (TnyFolder *folder, CamelFolderInfo *iter);

#define TNY_FOLDER_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_FOLDER, TnyFolderPriv))


#endif

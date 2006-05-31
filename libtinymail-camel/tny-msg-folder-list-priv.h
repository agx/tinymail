#ifndef TNY_MSG_FOLDER_LIST_PRIV_H
#define TNY_MSG_FOLDER_LIST_PRIV_H

/* libtinymailui-gtk - The Tiny Mail UI library for Gtk+
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

#include <tny-list-iface.h>
#include <tny-iterator-iface.h>
#include <tny-msg-folder-iface.h>

G_BEGIN_DECLS

#define TNY_TYPE_MSG_FOLDER_LIST            (_tny_msg_folder_list_get_type ())
#define TNY_MSG_FOLDER_LIST(obj)            (GTK_CHECK_CAST ((obj), TNY_TYPE_MSG_FOLDER_LIST, TnyMsgFolderList))
#define TNY_MSG_FOLDER_LIST_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), TNY_TYPE_MSG_FOLDER_LIST, TnyMsgFolderListClass))
#define TNY_IS_MSG_FOLDER_LIST(obj)         (GTK_CHECK_TYPE ((obj), TNY_TYPE_MSG_FOLDER_LIST))
#define TNY_IS_MSG_FOLDER_LIST_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((obj), TNY_TYPE_MSG_FOLDER_LIST))
#define TNY_MSG_FOLDER_LIST_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), TNY_TYPE_MSG_FOLDER_LIST, TnyMsgFolderListClass))

typedef struct _TnyMsgFolderList TnyMsgFolderList;
typedef struct _TnyMsgFolderListClass TnyMsgFolderListClass;

struct _TnyMsgFolderList 
{
	GObject parent;
	GMutex *iterator_lock;
	GList *first;
	TnyMsgFolderIface *pfolder;
};

struct _TnyMsgFolderListClass 
{
	GObjectClass parent;
};

GType         _tny_msg_folder_list_get_type         (void);
TnyListIface* _tny_msg_folder_list_new              (TnyMsgFolderIface *pfolder);
void          _tny_msg_folder_list_set_folder       (TnyMsgFolderList *self, TnyMsgFolderIface *pfolder);
void          _tny_msg_folder_list_intern_prepend   (TnyMsgFolderList *self, TnyMsgFolderIface *item);

G_END_DECLS

#endif

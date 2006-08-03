#ifndef TNY_FOLDER_LIST_PRIV_H
#define TNY_FOLDER_LIST_PRIV_H

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
#include <tny-folder-iface.h>

G_BEGIN_DECLS

#define TNY_TYPE_FOLDER_LIST             (_tny_folder_list_get_type ())
#define TNY_FOLDER_LIST(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_FOLDER_LIST, TnyFolderList))
#define TNY_FOLDER_LIST_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_FOLDER_LIST, TnyFolderListClass))
#define TNY_IS_FOLDER_LIST(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_FOLDER_LIST))
#define TNY_IS_FOLDER_LIST_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_FOLDER_LIST))
#define TNY_FOLDER_LIST_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_FOLDER_LIST, TnyFolderListClass))


typedef struct _TnyFolderList TnyFolderList;
typedef struct _TnyFolderListClass TnyFolderListClass;

struct _TnyFolderList 
{
	GObject parent;
	GMutex *iterator_lock;
	GList *first;
	TnyFolderIface *pfolder;
};

struct _TnyFolderListClass 
{
	GObjectClass parent;
};

GType         _tny_folder_list_get_type         (void);
TnyListIface* _tny_folder_list_new              (TnyFolderIface *pfolder);
void          _tny_folder_list_set_folder       (TnyFolderList *self, TnyFolderIface *pfolder);
void          _tny_folder_list_intern_prepend   (TnyFolderList *self, TnyFolderIface *item);

G_END_DECLS

#endif

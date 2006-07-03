#ifndef TNY_MSG_FOLDER_LIST_ITERATOR_PRIV_H
#define TNY_MSG_FOLDER_LIST_ITERATOR_PRIV_H

/* libtinymail- The Tiny Mail base library
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

#include <tny-shared.h>

#include <tny-list-iface.h>
#include <tny-iterator-iface.h>

G_BEGIN_DECLS

#define TNY_TYPE_MSG_FOLDER_LIST_ITERATOR             (_tny_msg_folder_list_iterator_get_type ())
#define TNY_MSG_FOLDER_LIST_ITERATOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_MSG_FOLDER_LIST_ITERATOR, TnyMsgFolderListIterator))
#define TNY_MSG_FOLDER_LIST_ITERATOR_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_MSG_FOLDER_LIST_ITERATOR, TnyMsgFolderListIteratorClass))
#define TNY_IS_MSG_FOLDER_LIST_ITERATOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_MSG_FOLDER_LIST_ITERATOR))
#define TNY_IS_MSG_FOLDER_LIST_ITERATOR_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_MSG_FOLDER_LIST_ITERATOR))
#define TNY_MSG_FOLDER_LIST_ITERATOR_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_MSG_FOLDER_LIST_ITERATOR, TnyMsgFolderListIteratorClass))

typedef struct _TnyMsgFolderListIterator TnyMsgFolderListIterator;
typedef struct _TnyMsgFolderListIteratorClass TnyMsgFolderListIteratorClass;

struct _TnyMsgFolderListIterator
{
	GObject parent;
	TnyMsgFolderList *model;
	GList *current;
};

struct _TnyMsgFolderListIteratorClass 
{
	GObjectClass parent;
};

GType _tny_msg_folder_list_iterator_get_type (void);
void _tny_msg_folder_list_iterator_set_model (TnyMsgFolderListIterator *self, TnyMsgFolderList *model);
TnyMsgFolderListIterator* _tny_msg_header_list_iterator_new (TnyMsgFolderList *model);

G_END_DECLS

#endif


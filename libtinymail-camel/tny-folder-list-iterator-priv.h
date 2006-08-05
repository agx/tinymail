#ifndef TNY_FOLDER_LIST_ITERATOR_PRIV_H
#define TNY_FOLDER_LIST_ITERATOR_PRIV_H

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

#define TNY_TYPE_FOLDER_LIST_ITERATOR             (_tny_folder_list_iterator_get_type ())
#define TNY_FOLDER_LIST_ITERATOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_FOLDER_LIST_ITERATOR, TnyFolderListIterator))
#define TNY_FOLDER_LIST_ITERATOR_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_FOLDER_LIST_ITERATOR, TnyFolderListIteratorClass))
#define TNY_IS_FOLDER_LIST_ITERATOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_FOLDER_LIST_ITERATOR))
#define TNY_IS_FOLDER_LIST_ITERATOR_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_FOLDER_LIST_ITERATOR))
#define TNY_FOLDER_LIST_ITERATOR_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_FOLDER_LIST_ITERATOR, TnyFolderListIteratorClass))

typedef struct _TnyFolderListIterator TnyFolderListIterator;
typedef struct _TnyFolderListIteratorClass TnyFolderListIteratorClass;

struct _TnyFolderListIterator
{
	GObject parent;
	TnyFolderList *model;
	GList *current;
};

struct _TnyFolderListIteratorClass 
{
	GObjectClass parent;
};

GType _tny_folder_list_iterator_get_type (void);
void _tny_folder_list_iterator_set_model (TnyFolderListIterator *self, TnyFolderList *model);
TnyFolderListIterator* _tny_header_list_iterator_new (TnyFolderList *model);

G_END_DECLS

#endif


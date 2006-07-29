#ifndef TNY_ACCOUNT_TREE_MODEL_H
#define TNY_ACCOUNT_TREE_MODEL_H

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

#include <glib.h>
#include <gtk/gtk.h>
#include <tny-store-account-iface.h>
#include <tny-list-iface.h>

G_BEGIN_DECLS

#define TNY_TYPE_ACCOUNT_TREE_MODEL             (tny_account_tree_model_get_type ())
#define TNY_ACCOUNT_TREE_MODEL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_ACCOUNT_TREE_MODEL, TnyAccountTreeModel))
#define TNY_ACCOUNT_TREE_MODEL_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_ACCOUNT_TREE_MODEL, TnyAccountTreeModelClass))
#define TNY_ACCOUNT_IS_TREE_MODEL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_ACCOUNT_TREE_MODEL))
#define TNY_ACCOUNT_IS_TREE_MODEL_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_ACCOUNT_TREE_MODEL))
#define TNY_ACCOUNT_TREE_MODEL_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_ACCOUNT_TREE_MODEL, TnyAccountTreeModelClass))

/* Implements GtkTreeModelIface and TnyListIface */

typedef struct _TnyAccountTreeModel TnyAccountTreeModel;
typedef struct _TnyAccountTreeModelClass TnyAccountTreeModelClass;

enum 
{
	TNY_ACCOUNT_TREE_MODEL_NAME_COLUMN,
	TNY_ACCOUNT_TREE_MODEL_UNREAD_COLUMN,
	TNY_ACCOUNT_TREE_MODEL_TYPE_COLUMN,
	TNY_ACCOUNT_TREE_MODEL_INSTANCE_COLUMN,
	TNY_ACCOUNT_TREE_MODEL_N_COLUMNS
};

struct _TnyAccountTreeModel
{
	GtkTreeStore parent;
	GList *first;
	GMutex *iterator_lock;
};

struct _TnyAccountTreeModelClass
{
	GtkTreeStoreClass parent_class;
};

GType                   tny_account_tree_model_get_type  (void);
TnyAccountTreeModel*    tny_account_tree_model_new       (void);

G_END_DECLS

#endif

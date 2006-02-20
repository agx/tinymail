#ifndef TNY_ATTACH_LIST_MODEL_H
#define TNY_ATTACH_LIST_MODEL_H

/* libtinymailui - The Tiny Mail UI library
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
#include <tny-account-iface.h>

G_BEGIN_DECLS

#define TNY_ATTACH_LIST_MODEL_TYPE            (tny_attach_list_model_get_type ())
#define TNY_ATTACH_LIST_MODEL(obj)            (GTK_CHECK_CAST ((obj), TNY_ATTACH_LIST_MODEL_TYPE, TnyAttachListModel))
#define TNY_ATTACH_LIST_MODEL_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), TNY_ATTACH_LIST_MODEL_TYPE, TnyAttachListModelClass))
#define TNY_ATTACH_IS_LIST_MODEL(obj)         (GTK_CHECK_TYPE ((obj), TNY_ATTACH_LIST_MODEL_TYPE))
#define TNY_ATTACH_IS_LIST_MODEL_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((obj), TNY_ATTACH_LIST_MODEL_TYPE))
#define TNY_ATTACH_LIST_MODEL_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), TNY_ATTACH_LIST_MODEL_TYPE, TnyAttachListModelClass))

typedef struct _TnyAttachListModel TnyAttachListModel;
typedef struct _TnyAttachListModelClass TnyAttachListModelClass;

enum 
{
	TNY_ATTACH_LIST_MODEL_PIXBUF_COLUMN,
	TNY_ATTACH_LIST_MODEL_FILENAME_COLUMN,
	TNY_ATTACH_LIST_MODEL_INSTANCE_COLUMN,
	TNY_ATTACH_LIST_MODEL_N_COLUMNS
};

struct _TnyAttachListModel
{
	GtkListStore parent;
};

struct _TnyAttachListModelClass
{
	GtkListStoreClass parent_class;
};


GType                   tny_attach_list_model_get_type  (void);
TnyAttachListModel*     tny_attach_list_model_new       (void);

void                    tny_attach_list_model_add       (TnyAttachListModel *self, TnyMsgMimePartIface *part);

G_END_DECLS

#endif

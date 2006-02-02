#ifndef TNY_MSG_HEADER_LIST_MODEL_H
#define TNY_MSG_HEADER_LIST_MODEL_H

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

#include <gtk/gtktreemodel.h>
#include <tny-msg-header-iface.h>

G_BEGIN_DECLS

#define TNY_MSG_HEADER_TYPE_LIST_MODEL            (tny_msg_header_list_model_get_type ())
#define TNY_MSG_HEADER_LIST_MODEL(obj)            (GTK_CHECK_CAST ((obj), TNY_MSG_HEADER_TYPE_LIST_MODEL, TnyMsgHeaderListModel))
#define TNY_MSG_HEADER_LIST_MODEL_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), TNY_MSG_HEADER_TYPE_LIST_MODEL, TnyMsgHeaderListModelClass))
#define TNY_MSG_HEADER_IS_LIST_MODEL(obj)         (GTK_CHECK_TYPE ((obj), TNY_MSG_HEADER_TYPE_LIST_MODEL))
#define TNY_MSG_HEADER_IS_LIST_MODEL_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((obj), TNY_MSG_HEADER_TYPE_LIST_MODEL))
#define TNY_MSG_HEADER_LIST_MODEL_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), TNY_MSG_HEADER_TYPE_LIST_MODEL, TnyMsgHeaderListModelClass))

typedef struct _TnyMsgHeaderListModel TnyMsgHeaderListModel;
typedef struct _TnyMsgHeaderListModelClass TnyMsgHeaderListModelClass;

enum 
{
	TNY_MSG_HEADER_LIST_MODEL_FROM_COLUMN,
	TNY_MSG_HEADER_LIST_MODEL_TO_COLUMN,
	TNY_MSG_HEADER_LIST_MODEL_SUBJECT_COLUMN,
	TNY_MSG_HEADER_LIST_MODEL_INSTANCE_COLUMN,
	TNY_MSG_HEADER_LIST_MODEL_NUM_COLUMNS
};


GType         tny_msg_header_list_model_get_type  (void);
GtkTreeModel* tny_msg_header_list_model_new       (void);
void          tny_msg_header_list_model_add       (TnyMsgHeaderListModel *self, TnyMsgHeaderIface *header);
void          tny_msg_header_list_model_inject    (TnyMsgHeaderListModel *self, GList *headers);

G_END_DECLS

#endif

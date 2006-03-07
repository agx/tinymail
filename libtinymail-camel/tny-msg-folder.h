#ifndef TNY_MSG_FOLDER_H
#define TNY_MSG_FOLDER_H

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
#include <glib-object.h>

#include <tny-msg-iface.h>
#include <tny-msg-header-iface.h>

#include <camel/camel-folder.h>

G_BEGIN_DECLS

#define TNY_TYPE_MSG_FOLDER             (tny_msg_folder_get_type ())
#define TNY_MSG_FOLDER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_MSG_FOLDER, TnyMsgFolder))
#define TNY_MSG_FOLDER_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_MSG_FOLDER, TnyMsgFolderClass))
#define TNY_IS_MSG_FOLDER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_MSG_FOLDER))
#define TNY_IS_MSG_FOLDER_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_MSG_FOLDER))
#define TNY_MSG_FOLDER_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_MSG_FOLDER, TnyMsgFolderClass))

typedef struct _TnyMsgFolder TnyMsgFolder;
typedef struct _TnyMsgFolderClass TnyMsgFolderClass;

struct _TnyMsgFolder
{
	GObject parent;
};

struct _TnyMsgFolderClass 
{
	GObjectClass parent;
};

GType           tny_msg_folder_get_type         (void);

TnyMsgFolder*   tny_msg_folder_new_with_folder  (CamelFolder *camel_folder);
TnyMsgFolder*   tny_msg_folder_new              (void);

void            tny_msg_folder_set_folder       (TnyMsgFolder *self, CamelFolder *camel_folder);
CamelFolder*    tny_msg_folder_get_folder       (TnyMsgFolder *self);

G_END_DECLS

#endif


#ifndef TNY_FOLDER_H
#define TNY_FOLDER_H

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
#include <tny-header-iface.h>

#include <camel/camel-folder.h>

G_BEGIN_DECLS

#define TNY_TYPE_FOLDER             (tny_folder_get_type ())
#define TNY_FOLDER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_FOLDER, TnyFolder))
#define TNY_FOLDER_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_FOLDER, TnyFolderClass))
#define TNY_IS_FOLDER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_FOLDER))
#define TNY_IS_FOLDER_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_FOLDER))
#define TNY_FOLDER_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_FOLDER, TnyFolderClass))

typedef struct _TnyFolder TnyFolder;
typedef struct _TnyFolderClass TnyFolderClass;

struct _TnyFolder
{
	GObject parent;
};

struct _TnyFolderClass 
{
	GObjectClass parent;
};

GType           tny_folder_get_type         (void);

TnyFolder*   tny_folder_new_with_folder  (CamelFolder *camel_folder);
TnyFolder*   tny_folder_new              (void);

void            tny_folder_set_folder       (TnyFolder *self, CamelFolder *camel_folder);
CamelFolder*    tny_folder_get_folder       (TnyFolder *self);

G_END_DECLS

#endif


#ifndef TNY_CAMEL_POP_FOLDER_H
#define TNY_CAMEL_POP_FOLDER_H

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

#include <tny-folder-iface.h>
#include <tny-camel-folder.h>
#include <tny-camel-account.h>
#include <tny-store-account-iface.h>

G_BEGIN_DECLS

#define TNY_TYPE_CAMEL_POP_FOLDER             (tny_camel_pop_folder_get_type ())
#define TNY_CAMEL_POP_FOLDER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_CAMEL_POP_FOLDER, TnyCamelPOPFolder))
#define TNY_CAMEL_POP_FOLDER_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_CAMEL_POP_FOLDER, TnyCamelPOPFolderClass))
#define TNY_IS_CAMEL_POP_FOLDER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_CAMEL_POP_FOLDER))
#define TNY_IS_CAMEL_POP_FOLDER_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_CAMEL_POP_FOLDER))
#define TNY_CAMEL_POP_FOLDER_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_CAMEL_POP_FOLDER, TnyCamelPOPFolderClass))

typedef struct _TnyCamelPOPFolder TnyCamelPOPFolder;
typedef struct _TnyCamelPOPFolderClass TnyCamelPOPFolderClass;

struct _TnyCamelPOPFolder
{
	TnyCamelFolder parent;
};

struct _TnyCamelPOPFolderClass 
{
	TnyCamelFolderClass parent;
};

GType tny_camel_pop_folder_get_type (void);
TnyFolderIface* tny_camel_pop_folder_new (void);

G_END_DECLS

#endif


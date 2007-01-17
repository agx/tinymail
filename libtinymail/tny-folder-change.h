#ifndef TNY_FOLDER_CHANGE_H
#define TNY_FOLDER_CHANGE_H

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
#include <tny-folder.h>

G_BEGIN_DECLS

#define TNY_TYPE_FOLDER_CHANGE             (tny_folder_change_get_type ())
#define TNY_FOLDER_CHANGE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_FOLDER_CHANGE, TnyFolderChange))
#define TNY_FOLDER_CHANGE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_FOLDER_CHANGE, TnyFolderChangeClass))
#define TNY_IS_FOLDER_CHANGE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_FOLDER_CHANGE))
#define TNY_IS_FOLDER_CHANGE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_FOLDER_CHANGE))
#define TNY_FOLDER_CHANGE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_FOLDER_CHANGE, TnyFolderChangeClass))

#ifndef TNY_SHARED_H
typedef struct _TnyFolderChange TnyFolderChange;
typedef struct _TnyFolderChangeClass TnyFolderChangeClass;
#endif

struct _TnyFolderChange
{
	GObject parent;
};

struct _TnyFolderChangeClass 
{
	GObjectClass parent;
};

GType  tny_folder_change_get_type (void);
TnyFolderChange* tny_folder_change_new (TnyFolder *folder);

void tny_folder_change_set_new_all_count (TnyFolderChange *self, guint new_all_count);
void tny_folder_change_set_new_unread_count (TnyFolderChange *self, guint new_unread_count);
guint tny_folder_change_get_new_unread_count (TnyFolderChange *self);
guint tny_folder_change_get_new_all_count (TnyFolderChange *self);
void tny_folder_change_add_added_header (TnyFolderChange *self, TnyHeader *header);
void tny_folder_change_add_removed_header (TnyFolderChange *self, TnyHeader *header);
void tny_folder_change_get_added_headers (TnyFolderChange *self, TnyList *headers);
void tny_folder_change_get_removed_headers (TnyFolderChange *self, TnyList *headers);
void tny_folder_change_reset (TnyFolderChange *self);
TnyFolder* tny_folder_change_get_folder (TnyFolderChange *self);

G_END_DECLS

#endif


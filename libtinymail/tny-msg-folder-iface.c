/* libtinymail - The Tiny Mail base library
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

#include <tny-msg-folder-iface.h>
#include <tny-msg-header-iface.h>


const TnyAccountIface*  
tny_msg_folder_iface_get_account (TnyMsgFolderIface *self)
{
	return TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_account_func (self);
}

void
tny_msg_folder_iface_set_account (TnyMsgFolderIface *self, const TnyAccountIface *account)
{
	TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->set_account_func (self, account);

	return;
}

const GList*
tny_msg_folder_iface_get_folders (TnyMsgFolderIface *self)
{
	return TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_folders_func (self);
}

void
tny_msg_folder_iface_add_folder (TnyMsgFolderIface *self, TnyMsgFolderIface *folder)
{
	TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->add_folder_func (self, folder);

	return;
}

const TnyMsgIface*
tny_msg_folder_iface_get_message (TnyMsgFolderIface *self, const TnyMsgHeaderIface *header)
{
	return TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_message_func (self, header);
}

const GList*
tny_msg_folder_iface_get_headers (TnyMsgFolderIface *self)
{
	return TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_headers_func (self);
}

const gchar*
tny_msg_folder_iface_get_id (TnyMsgFolderIface *self)
{
	return TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_id_func (self);
}

const gchar*
tny_msg_folder_iface_get_name (TnyMsgFolderIface *self)
{
	return TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_name_func (self);
}

void
tny_msg_folder_iface_set_id (TnyMsgFolderIface *self, const gchar *id)
{
	TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->set_id_func (self, id);
	return;
}

void
tny_msg_folder_iface_set_name (TnyMsgFolderIface *self, const gchar *name)
{
	TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->set_name_func (self, name);
	return;
}

void
tny_msg_folder_iface_uncache (TnyMsgFolderIface *self)
{
	if (TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->uncache_func != NULL)
		TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->uncache_func (self);
	return;
}

const gboolean
tny_msg_folder_iface_has_cache (TnyMsgFolderIface *self)
{
	if (TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->has_cache_func != NULL)
		TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->has_cache_func (self);
	return;
}

static void
tny_msg_folder_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_msg_folder_iface_get_type (void)
{
	static GType type = 0;
	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgFolderIfaceClass),
		  tny_msg_folder_iface_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyMsgFolderIface", &info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}

	return type;
}

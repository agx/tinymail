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

#include <tny-msg-header-iface.h>
#include <tny-msg-folder-iface.h>

const gchar*
tny_msg_header_iface_get_id (TnyMsgHeaderIface *self)
{
	return TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_id_func (self);
}

const gchar* 
tny_msg_header_iface_get_from (TnyMsgHeaderIface *self)
{
	return TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_from_func (self);
}

const gchar*
tny_msg_header_iface_get_subject (TnyMsgHeaderIface *self)
{
	return TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_subject_func (self);
}


const gchar* 
tny_msg_header_iface_get_to (TnyMsgHeaderIface *self)
{
	return TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_to_func (self);
}

const TnyMsgFolderIface* 
tny_msg_header_iface_get_folder (TnyMsgHeaderIface *self)
{
	return TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_folder_func (self);
}


void
tny_msg_header_iface_set_folder (TnyMsgHeaderIface *self, const TnyMsgFolderIface *folder)
{
	TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_folder_func (self, folder);
	return;
}

void
tny_msg_header_iface_set_id (TnyMsgHeaderIface *self, const gchar *id)
{
	TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_id_func (self, id);
	return;
}

void
tny_msg_header_iface_set_from (TnyMsgHeaderIface *self, const gchar *from)
{
	TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_from_func (self, from);
	return;
}

void
tny_msg_header_iface_set_to (TnyMsgHeaderIface *self, const gchar *to)
{
	TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_to_func (self, to);
	return;
}

void
tny_msg_header_iface_set_subject (TnyMsgHeaderIface *self, const gchar *subject)
{
	TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_subject_func (self, subject);
	return;
}

void
tny_msg_header_iface_uncache (TnyMsgHeaderIface *self)
{
	if (TNY_MSG_HEADER_IFACE_GET_CLASS (self)->uncache_func != NULL)
		TNY_MSG_HEADER_IFACE_GET_CLASS (self)->uncache_func (self);
	return;
}

const gboolean
tny_msg_header_iface_has_cache (TnyMsgHeaderIface *self)
{
	if (TNY_MSG_HEADER_IFACE_GET_CLASS (self)->has_cache_func != NULL)
		TNY_MSG_HEADER_IFACE_GET_CLASS (self)->has_cache_func (self);
	return;
}


static void
tny_msg_header_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_msg_header_iface_get_type (void)
{
	static GType type = 0;
	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgHeaderIfaceClass),
		  tny_msg_header_iface_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};

		type = g_type_register_static (G_TYPE_INTERFACE,
			"TnyMsgHeaderIface", &info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}

	return type;
}

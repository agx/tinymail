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

#include <config.h>

#include <tny-msg-iface.h>


/**
 * tny_msg_iface_get_folder:
 * @self: a #TnyMsgIface object
 * 
 * Get the parent folder of this message
 *
 * Return value: The parent folder of this message
 **/
TnyFolderIface* 
tny_msg_iface_get_folder (TnyMsgIface *self)
{
#ifdef DEBUG
	if (!TNY_MSG_IFACE_GET_CLASS (self)->get_folder_func)
		g_critical ("You must implement tny_msg_iface_get_folder\n");
#endif

	return TNY_MSG_IFACE_GET_CLASS (self)->get_folder_func (self);
}


/**
 * tny_msg_iface_get_parts:
 * @self: a #TnyMsgIface object
 * @list: a #TnyListIface object
 * 
 * Get a read-only list of mime-parts of this message
 *
 **/
void
tny_msg_iface_get_parts (TnyMsgIface *self, TnyListIface *list)
{
#ifdef DEBUG
	if (!TNY_MSG_IFACE_GET_CLASS (self)->get_parts_func)
		g_critical ("You must implement tny_msg_iface_get_parts\n");
#endif

	TNY_MSG_IFACE_GET_CLASS (self)->get_parts_func (self, list);
	return;
}


/**
 * tny_msg_iface_get_header:
 * @self: a #TnyMsgIface object
 * 
 * Get the header of the message
 *
 * Return value: The header of the message
 **/
TnyHeaderIface*
tny_msg_iface_get_header (TnyMsgIface *self)
{
#ifdef DEBUG
	if (!TNY_MSG_IFACE_GET_CLASS (self)->get_header_func)
		g_critical ("You must implement tny_msg_iface_get_header\n");
#endif

	return TNY_MSG_IFACE_GET_CLASS (self)->get_header_func (self);
}


/**
 * tny_msg_iface_add_part:
 * @self: a #TnyMsgIface object
 * @part: the mime-part to add
 * 
 * Add a mime-part to a message
 *
 * Return value: The id of the added mime-part
 **/
gint
tny_msg_iface_add_part (TnyMsgIface *self, TnyMimePartIface *part)
{
#ifdef DEBUG
	if (!TNY_MSG_IFACE_GET_CLASS (self)->add_part_func)
		g_critical ("You must implement tny_msg_iface_add_part\n");
#endif

	return TNY_MSG_IFACE_GET_CLASS (self)->add_part_func (self, part);
}

/**
 * tny_msg_iface_del_part:
 * @self: a #TnyMsgIface object
 * @id: the mime-part id to delete
 * 
 * Delete a mime-part from a message
 *
 **/
void
tny_msg_iface_del_part (TnyMsgIface *self, gint id)
{
#ifdef DEBUG
	if (!TNY_MSG_IFACE_GET_CLASS (self)->del_part_func)
		g_critical ("You must implement tny_msg_iface_del_part\n");
#endif

	TNY_MSG_IFACE_GET_CLASS (self)->del_part_func (self, id);
	return;
}


/**
 * tny_msg_iface_set_header:
 * @self: a #TnyMsgIface object
 * @header: the header to set
 * 
 * Set the header of a message
 *
 **/
void
tny_msg_iface_set_header (TnyMsgIface *self, TnyHeaderIface *header)
{
#ifdef DEBUG
	if (!TNY_MSG_IFACE_GET_CLASS (self)->set_header_func)
		g_critical ("You must implement tny_msg_iface_set_header\n");
#endif

	TNY_MSG_IFACE_GET_CLASS (self)->set_header_func (self, header);
	return;
}

static void
tny_msg_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_msg_iface_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0)) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgIfaceClass),
		  tny_msg_iface_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyMsgIface", &info, 0);

		g_type_interface_add_prerequisite (type, TNY_TYPE_MIME_PART_IFACE); 
	}

	return type;
}



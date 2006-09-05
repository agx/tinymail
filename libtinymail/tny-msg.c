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

#include <tny-msg.h>


/**
 * tny_msg_get_folder:
 * @self: a #TnyMsg object
 * 
 * Get the parent folder of this message. The returned folder object should be
 * unreferenced after use.
 *
 * Return value: The parent folder of this message or NULL if none
 **/
TnyFolder* 
tny_msg_get_folder (TnyMsg *self)
{
#ifdef DEBUG
	if (!TNY_MSG_GET_IFACE (self)->get_folder_func)
		g_critical ("You must implement tny_msg_get_folder\n");
#endif

	return TNY_MSG_GET_IFACE (self)->get_folder_func (self);
}


/**
 * tny_msg_get_parts:
 * @self: a #TnyMsg object
 * @list: a #TnyList object
 * 
 * Get a read-only list of mime-parts of this message.
 *
 **/
void
tny_msg_get_parts (TnyMsg *self, TnyList *list)
{
#ifdef DEBUG
	if (!TNY_MSG_GET_IFACE (self)->get_parts_func)
		g_critical ("You must implement tny_msg_get_parts\n");
#endif

	TNY_MSG_GET_IFACE (self)->get_parts_func (self, list);
	return;
}


/**
 * tny_msg_get_header:
 * @self: a #TnyMsg object
 * 
 * Get the header of the message. The returned header object should be 
 * unreferenced after use.
 *
 * Return value: The header of the message
 **/
TnyHeader*
tny_msg_get_header (TnyMsg *self)
{
#ifdef DEBUG
	if (!TNY_MSG_GET_IFACE (self)->get_header_func)
		g_critical ("You must implement tny_msg_get_header\n");
#endif

	return TNY_MSG_GET_IFACE (self)->get_header_func (self);
}


/**
 * tny_msg_add_part:
 * @self: a #TnyMsg object
 * @part: the mime-part to add
 * 
 * Add a mime-part to a message
 *
 * Return value: The id of the added mime-part
 *
 **/
gint
tny_msg_add_part (TnyMsg *self, TnyMimePart *part)
{
#ifdef DEBUG
	if (!TNY_MSG_GET_IFACE (self)->add_part_func)
		g_critical ("You must implement tny_msg_add_part\n");
#endif

	return TNY_MSG_GET_IFACE (self)->add_part_func (self, part);
}

/**
 * tny_msg_del_part:
 * @self: a #TnyMsg object
 * @part: the mime-part to delete
 * 
 * Delete a mime-part from a message
 *
 **/
void
tny_msg_del_part (TnyMsg *self, TnyMimePart *part)
{
#ifdef DEBUG
	if (!TNY_MSG_GET_IFACE (self)->del_part_func)
		g_critical ("You must implement tny_msg_del_part\n");
#endif

	TNY_MSG_GET_IFACE (self)->del_part_func (self, part);
	return;
}


/**
 * tny_msg_set_header:
 * @self: a #TnyMsg object
 * @header: the header to set
 * 
 * Set the header of a message
 *
 **/
void
tny_msg_set_header (TnyMsg *self, TnyHeader *header)
{
#ifdef DEBUG
	if (!TNY_MSG_GET_IFACE (self)->set_header_func)
		g_critical ("You must implement tny_msg_set_header\n");
#endif

	TNY_MSG_GET_IFACE (self)->set_header_func (self, header);
	return;
}

static void
tny_msg_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_msg_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0)) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgIface),
		  tny_msg_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL,   /* instance_init */
		  NULL
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyMsg", &info, 0);

		g_type_interface_add_prerequisite (type, TNY_TYPE_MIME_PART); 
	}

	return type;
}



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

#ifdef DBC
#include <string.h>
#endif

#include <tny-msg.h>
#include <tny-header.h>
#include <tny-folder.h>


/**
 * tny_msg_get_url_string:
 * @self: a #TnyMsg object
 * 
 * Get the url_string @self or NULL if it's impossible to determine the url 
 * string of @self. If not NULL, the returned value must be freed after use.
 *
 * The url string is specified in RFC 1808 and looks for example like this:
 * imap://user@hostname/INBOX/folder/000 where 000 is the UID of the message on
 * the IMAP server. Note that it doesn't necessarily contain the password of the
 * IMAP account.
 * 
 * Return value: The url string or NULL.
 **/
gchar* 
tny_msg_get_url_string (TnyMsg *self)
{
	gchar *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_MSG (self));
	g_assert (TNY_MSG_GET_IFACE (self)->get_url_string_func != NULL);
#endif

	retval = TNY_MSG_GET_IFACE (self)->get_url_string_func (self);

#ifdef DBC /* ensure */
	if (retval) {
		g_assert (strlen (retval) > 0);
		g_assert (strstr (retval, "://") != NULL);
	}
#endif

	return retval;
}

/**
 * tny_msg_get_folder:
 * @self: a #TnyMsg object
 * 
 * Get the parent folder of @self. If not NULL, the returned value must be
 * unreferenced after use.
 *
 * Return value: The parent folder of this message or NULL if none
 **/
TnyFolder* 
tny_msg_get_folder (TnyMsg *self)
{
	TnyFolder *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_MSG (self));
	g_assert (TNY_MSG_GET_IFACE (self)->get_folder_func != NULL);
#endif

	retval = TNY_MSG_GET_IFACE (self)->get_folder_func (self);

#ifdef DBC /* ensure */
	if (retval)
		g_assert (TNY_IS_FOLDER (retval));
#endif

	return retval;
}




/**
 * tny_msg_get_header:
 * @self: a #TnyMsg object
 * 
 * Get the header of @self. The returned header object must be 
 * unreferenced after use.
 *
 * Return value: The header of the message
 **/
TnyHeader*
tny_msg_get_header (TnyMsg *self)
{
	TnyHeader *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_MSG (self));
	g_assert (TNY_MSG_GET_IFACE (self)->get_header_func != NULL);
#endif

	retval = TNY_MSG_GET_IFACE (self)->get_header_func (self);


#ifdef DBC /* ensure */
	g_assert (TNY_IS_HEADER (retval));
#endif

	return retval;
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
		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);

	}

	return type;
}



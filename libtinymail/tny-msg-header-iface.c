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


/**
 * tny_msg_header_iface_set_bcc:
 * @self: a #TnyMsgHeaderIface object
 * @to: the reply-to header
 * 
 * Set the reply-to header
 * 
 **/
void
tny_msg_header_iface_set_replyto (TnyMsgHeaderIface *self, const gchar *to)
{
	TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_replyto_func (self, to);
	return;
}

/**
 * tny_msg_header_iface_get_replyto:
 * @self: a #TnyMsgHeaderIface object
 * 
 * 
 * 
 * Return value: reply-to header
 **/
const gchar*
tny_msg_header_iface_get_replyto (TnyMsgHeaderIface *self)
{
	return TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_replyto_func (self);
}

/**
 * tny_msg_header_iface_set_bcc:
 * @self: a #TnyMsgHeaderIface object
 * @from: the bcc header in a comma separated list
 * 
 * Set the bcc header. Also look at the to header for more information
 * about the formatting.
 * 
 **/
void
tny_msg_header_iface_set_bcc (TnyMsgHeaderIface *self, const gchar *bcc)
{
	TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_bcc_func (self, bcc);
	return;
}

/**
 * tny_msg_header_iface_set_cc:
 * @self: a #TnyMsgHeaderIface object
 * @from: the cc header in a comma separated list
 * 
 * Set the cc header. Also look at the to header for more information
 * about the formatting.
 * 
 **/
void
tny_msg_header_iface_set_cc (TnyMsgHeaderIface *self, const gchar *cc)
{
	TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_cc_func (self, cc);
	return;
}

/**
 * tny_msg_header_iface_set_from:
 * @self: a #TnyMsgHeaderIface object
 * @from: the from header
 * 
 * Set the from header
 * 
 **/
void
tny_msg_header_iface_set_from (TnyMsgHeaderIface *self, const gchar *from)
{
	TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_from_func (self, from);
	return;
}

/**
 * tny_msg_header_iface_set_subject:
 * @self: a #TnyMsgHeaderIface object
 * @subject: the subject header
 * 
 * Set the subject header
 * 
 **/
void
tny_msg_header_iface_set_subject (TnyMsgHeaderIface *self, const gchar *subject)
{
	TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_subject_func (self, subject);
	return;
}

/**
 * tny_msg_header_iface_set_to:
 * @self: a #TnyMsgHeaderIface object
 * @to: the to header in a comma separated list
 * 
 * Set the to header.
 *
 * The format is a comma separated list like this:
 * 
 * So there's no quotes nor anything special.
 * 
 **/
void
tny_msg_header_iface_set_to (TnyMsgHeaderIface *self, const gchar *to)
{
	TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_to_func (self, to);
	return;
}

/**
 * tny_msg_header_iface_get_cc:
 * @self: a #TnyMsgHeaderIface object
 * 
 * 
 * 
 * Return value: cc header
 **/
const gchar*
tny_msg_header_iface_get_cc (TnyMsgHeaderIface *self)
{
	return TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_cc_func (self);
}

/**
 * tny_msg_header_iface_get_bcc:
 * @self: a #TnyMsgHeaderIface object
 * 
 * 
 * 
 * Return value: bcc header
 **/
const gchar*
tny_msg_header_iface_get_bcc (TnyMsgHeaderIface *self)
{
	return TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_bcc_func (self);
}

/**
 * tny_msg_header_iface_get_date_received:
 * @self: a #TnyMsgHeaderIface object
 * 
 * 
 * 
 * Return value: date received header
 **/
const time_t
tny_msg_header_iface_get_date_received (TnyMsgHeaderIface *self)
{
	return TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_date_received_func (self);
}

/**
 * tny_msg_header_iface_get_date_sent:
 * @self: a #TnyMsgHeaderIface object
 * 
 * 
 * 
 * Return value: date sent header
 **/
const time_t
tny_msg_header_iface_get_date_sent (TnyMsgHeaderIface *self)
{
	return TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_date_sent_func (self);
}


/**
 * tny_msg_header_iface_get_id:
 * @self: a #TnyMsgHeaderIface object
 * 
 * Get an unique id of the message of which self is a message header.
 * 
 * Return value: Unique follow-up uid
 **/
const gchar*
tny_msg_header_iface_get_uid (TnyMsgHeaderIface *self)
{
	return TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_uid_func (self);
}

/**
 * tny_msg_header_iface_get_message_id:
 * @self: a #TnyMsgHeaderIface object
 * 
 * Get an unique id of the message of which self is a message header.
 * 
 * Return value: message-id header
 **/
const gchar*
tny_msg_header_iface_get_message_id (TnyMsgHeaderIface *self)
{
	return TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_message_id_func (self);
}

/**
 * tny_msg_header_iface_get_from:
 * @self: a #TnyMsgHeaderIface object
 * 
 * Get the from header
 * 
 * Return value: from header
 **/
const gchar* 
tny_msg_header_iface_get_from (TnyMsgHeaderIface *self)
{
	return TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_from_func (self);
}

/**
 * tny_msg_header_iface_get_subject:
 * @self: a #TnyMsgHeaderIface object
 * 
 * Get the subject header
 * 
 * Return value: subject header
 **/
const gchar*
tny_msg_header_iface_get_subject (TnyMsgHeaderIface *self)
{
	return TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_subject_func (self);
}


/**
 * tny_msg_header_iface_get_to:
 * @self: a #TnyMsgHeaderIface object
 * 
 * Get the to header
 * 
 * Return value: to header
 **/
const gchar* 
tny_msg_header_iface_get_to (TnyMsgHeaderIface *self)
{
	return TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_to_func (self);
}

/**
 * tny_msg_header_iface_get_folder:
 * @self: a #TnyMsgHeaderIface object
 * 
 * Get a reference to the parent folder where this message header is located
 * 
 * Return value: The folder of the message header
 **/
const TnyMsgFolderIface* 
tny_msg_header_iface_get_folder (TnyMsgHeaderIface *self)
{
	return TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_folder_func (self);
}


/**
 * tny_msg_header_iface_set_folder:
 * @self: a #TnyMsgHeaderIface object
 * 
 * Set the reference to the parent folder where this message header is located
 * 
 **/
void
tny_msg_header_iface_set_folder (TnyMsgHeaderIface *self, const TnyMsgFolderIface *folder)
{
	TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_folder_func (self, folder);
	return;
}

/**
 * tny_msg_header_iface_set_message_id:
 * @self: a #TnyMsgHeaderIface object
 * @id: an unique follow-up uid
 * 
 *
 **/
void
tny_msg_header_iface_set_uid (TnyMsgHeaderIface *self, const gchar *id)
{
	TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_uid_func (self, id);
	return;
}


/**
 * tny_msg_header_iface_uncache:
 * @self: a #TnyMsgHeaderIface object
 * 
 * If it's possible to uncache this instance, uncache it
 * 
 **/
void
tny_msg_header_iface_uncache (TnyMsgHeaderIface *self)
{
	if (TNY_MSG_HEADER_IFACE_GET_CLASS (self)->uncache_func != NULL)
		TNY_MSG_HEADER_IFACE_GET_CLASS (self)->uncache_func (self);
	return;
}

/**
 * tny_msg_header_iface_has_cache:
 * @self: a #TnyMsgHeaderIface object
 * 
 * If it's possible to uncache this instance, return whether or not it has a cache
 * 
 * Return value: Whether or not this instance has a cache
 **/
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

	if (G_UNLIKELY(type == 0))
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

		/* g_type_interface_add_prerequisite (type, G_TYPE_OBJECT); */
	}

	return type;
}

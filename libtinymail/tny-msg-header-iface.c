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
#ifdef DEBUG
	if (!TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_replyto_func)
		g_critical ("You must implement tny_msg_header_iface_set_replyto\n");
#endif	

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
#ifdef DEBUG
	if (!TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_replyto_func)
		g_critical ("You must implement tny_msg_header_iface_get_replyto\n");
#endif
	return TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_replyto_func (self);
}

/**
 * tny_msg_header_iface_set_bcc:
 * @self: a #TnyMsgHeaderIface object
 * @bcc: the bcc header in a comma separated list
 * 
 * Set the bcc header. Also look at the to header for more information
 * about the formatting.
 * 
 **/
void
tny_msg_header_iface_set_bcc (TnyMsgHeaderIface *self, const gchar *bcc)
{
#ifdef DEBUG
	if (!TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_bcc_func)
		g_critical ("You must implement tny_msg_header_iface_set_bcc\n");
#endif

	TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_bcc_func (self, bcc);
	return;
}

/**
 * tny_msg_header_iface_set_cc:
 * @self: a #TnyMsgHeaderIface object
 * @cc: the cc header in a comma separated list
 * 
 * Set the cc header. Also look at the to header for more information
 * about the formatting.
 * 
 **/
void
tny_msg_header_iface_set_cc (TnyMsgHeaderIface *self, const gchar *cc)
{
#ifdef DEBUG
	if (!TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_cc_func)
		g_critical ("You must implement tny_msg_header_iface_set_cc\n");
#endif

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
#ifdef DEBUG
	if (!TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_from_func)
		g_critical ("You must implement tny_msg_header_iface_set_from\n");
#endif

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
#ifdef DEBUG
	if (!TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_subject_func)
		g_critical ("You must implement tny_msg_header_iface_set_subject\n");
#endif

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
#ifdef DEBUG
	if (!TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_to_func)
		g_critical ("You must implement tny_msg_header_iface_set_to\n");
#endif

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
#ifdef DEBUG
	if (!TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_cc_func)
		g_critical ("You must implement tny_msg_header_iface_get_cc\n");
#endif

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
#ifdef DEBUG
	if (!TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_bcc_func)
		g_critical ("You must implement tny_msg_header_iface_get_bcc\n");
#endif

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
time_t
tny_msg_header_iface_get_date_received (TnyMsgHeaderIface *self)
{
#ifdef DEBUG
	if (!TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_date_received_func)
		g_critical ("You must implement tny_msg_header_iface_get_date_received\n");
#endif

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
time_t
tny_msg_header_iface_get_date_sent (TnyMsgHeaderIface *self)
{
#ifdef DEBUG
	if (!TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_date_sent_func)
		g_critical ("You must implement tny_msg_header_iface_get_date_sent\n");
#endif

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
#ifdef DEBUG
	if (!TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_uid_func)
		g_critical ("You must implement tny_msg_header_iface_get_uid\n");
#endif

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
#ifdef DEBUG
	if (!TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_message_id_func)
		g_critical ("You must implement tny_msg_header_iface_get_message_id\n");
#endif

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
#ifdef DEBUG
	if (!TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_from_func)
		g_critical ("You must implement tny_msg_header_iface_get_from\n");
#endif

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
#ifdef DEBUG
	if (!TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_subject_func)
		g_critical ("You must implement tny_msg_header_iface_get_subject\n");
#endif

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
#ifdef DEBUG
	if (!TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_to_func)
		g_critical ("You must implement tny_msg_header_iface_get_to\n");
#endif

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
#ifdef DEBUG
	if (!TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_folder_func)
		g_critical ("You must implement tny_msg_header_iface_get_folder\n");
#endif

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
#ifdef DEBUG
	if (!TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_folder_func)
		g_critical ("You must implement tny_msg_header_iface_set_folder\n");
#endif

	TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_folder_func (self, folder);
	return;
}

/**
 * tny_msg_header_iface_get_flags:
 * @self: a #TnyMsgHeaderIface object
 * 
 * Get message information flags.
 * 
 * Return value: flag bitmask
 **/
TnyMsgHeaderFlags
tny_msg_header_iface_get_flags (TnyMsgHeaderIface *self)
{
#ifdef DEBUG
	if (!TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_flags_func)
		g_critical ("You must implement tny_msg_header_iface_get_flags\n");
#endif

	return TNY_MSG_HEADER_IFACE_GET_CLASS (self)->get_flags_func (self);
}

/**
 * tny_msg_header_iface_set_flags:
 * @self: a #TnyMsgHeaderIface object
 * @mask: A #TnyMsgHeaderFlags bitmask of flags to set.
 * 
 * Modify message flags.
 * 
 **/
void 
tny_msg_header_iface_set_flags (TnyMsgHeaderIface *self, TnyMsgHeaderFlags mask)
{
#ifdef DEBUG
	if (!TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_flags_func)
		g_critical ("You must implement tny_msg_header_iface_set_flags\n");
#endif

	return TNY_MSG_HEADER_IFACE_GET_CLASS (self)->set_flags_func (self, mask);
}

/**
 * tny_msg_header_iface_unset_flags:
 * @self: a #TnyMsgHeaderIface object
 * @mask: A #TnyMsgHeaderFlags bitmask of flags to clear.
 * 
 * Modify message flags.
 * 
 **/
void 
tny_msg_header_iface_unset_flags (TnyMsgHeaderIface *self, TnyMsgHeaderFlags mask)
{
#ifdef DEBUG
	if (!TNY_MSG_HEADER_IFACE_GET_CLASS (self)->unset_flags_func)
		g_critical ("You must implement tny_msg_header_iface_unset_flags\n");
#endif

	return TNY_MSG_HEADER_IFACE_GET_CLASS (self)->unset_flags_func (self, mask);
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


GType
tny_msg_header_flags_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TNY_MSG_HEADER_FLAG_ANSWERED, "TNY_MSG_HEADER_FLAG_ANSWERED", "answered" },
      { TNY_MSG_HEADER_FLAG_DELETED, "TNY_MSG_HEADER_FLAG_DELETED", "deleted" },
      { TNY_MSG_HEADER_FLAG_DRAFT, "TNY_MSG_HEADER_FLAG_DRAFT", "draft" },
      { TNY_MSG_HEADER_FLAG_FLAGGED, "TNY_MSG_HEADER_FLAG_FLAGGED", "flagged" },
      { TNY_MSG_HEADER_FLAG_SEEN, "TNY_MSG_HEADER_FLAG_SEEN", "seen" },
      { TNY_MSG_HEADER_FLAG_ATTACHMENTS, "TNY_MSG_HEADER_FLAG_ATTACHMENTS", "attachments" },
      { TNY_MSG_HEADER_FLAG_ANSWERED_ALL, "TNY_MSG_HEADER_FLAG_ANSWERED_ALL", "answered_all" },
      { TNY_MSG_HEADER_FLAG_JUNK, "TNY_MSG_HEADER_FLAG_junk", "JUNK" },
      { TNY_MSG_HEADER_FLAG_SECURE, "TNY_MSG_HEADER_FLAG_SECURE", "secure" },
      { TNY_MSG_HEADER_FLAG_FOLDER_FLAGGED, "TNY_MSG_HEADER_FLAG_FLAGGED", "flagged" },
      { TNY_MSG_HEADER_FLAG_JUNK_LEARN, "TNY_MSG_HEADER_FLAG_LEARN", "learn" },
      { TNY_MSG_HEADER_FLAG_USER, "TNY_MSG_HEADER_FLAG_USER", "user" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyMsgHeaderFlags", values);
  }
  return etype;
}

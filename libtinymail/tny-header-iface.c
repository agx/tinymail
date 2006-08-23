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

#include <tny-header-iface.h>
#include <tny-folder-iface.h>


/**
 * tny_header_iface_set_bcc:
 * @self: a #TnyHeaderIface object
 * @to: the reply-to header
 * 
 * Set the reply-to header
 * 
 **/
void
tny_header_iface_set_replyto (TnyHeaderIface *self, const gchar *to)
{
#ifdef DEBUG
	if (!TNY_HEADER_IFACE_GET_CLASS (self)->set_replyto_func)
		g_critical ("You must implement tny_header_iface_set_replyto\n");
#endif	

	TNY_HEADER_IFACE_GET_CLASS (self)->set_replyto_func (self, to);
	return;
}

/**
 * tny_header_iface_get_replyto:
 * @self: a #TnyHeaderIface object
 * 
 * Get the reply-to header
 * 
 * Return value: reply-to header
 **/
const gchar*
tny_header_iface_get_replyto (TnyHeaderIface *self)
{
#ifdef DEBUG
	if (!TNY_HEADER_IFACE_GET_CLASS (self)->get_replyto_func)
		g_critical ("You must implement tny_header_iface_get_replyto\n");
#endif
	return TNY_HEADER_IFACE_GET_CLASS (self)->get_replyto_func (self);
}

/**
 * tny_header_iface_set_bcc:
 * @self: a #TnyHeaderIface object
 * @bcc: the bcc header in a comma separated list
 * 
 * Set the bcc header. Also look at the to header for more information
 * about formatting.
 * 
 **/
void
tny_header_iface_set_bcc (TnyHeaderIface *self, const gchar *bcc)
{
#ifdef DEBUG
	if (!TNY_HEADER_IFACE_GET_CLASS (self)->set_bcc_func)
		g_critical ("You must implement tny_header_iface_set_bcc\n");
#endif

	TNY_HEADER_IFACE_GET_CLASS (self)->set_bcc_func (self, bcc);
	return;
}

/**
 * tny_header_iface_set_cc:
 * @self: a #TnyHeaderIface object
 * @cc: the cc header in a comma separated list
 * 
 * Set the cc header. Also look at the to header for more information
 * about formatting.
 * 
 **/
void
tny_header_iface_set_cc (TnyHeaderIface *self, const gchar *cc)
{
#ifdef DEBUG
	if (!TNY_HEADER_IFACE_GET_CLASS (self)->set_cc_func)
		g_critical ("You must implement tny_header_iface_set_cc\n");
#endif

	TNY_HEADER_IFACE_GET_CLASS (self)->set_cc_func (self, cc);
	return;
}

/**
 * tny_header_iface_set_from:
 * @self: a #TnyHeaderIface object
 * @from: the from header
 * 
 * Set the from header
 * 
 **/
void
tny_header_iface_set_from (TnyHeaderIface *self, const gchar *from)
{
#ifdef DEBUG
	if (!TNY_HEADER_IFACE_GET_CLASS (self)->set_from_func)
		g_critical ("You must implement tny_header_iface_set_from\n");
#endif

	TNY_HEADER_IFACE_GET_CLASS (self)->set_from_func (self, from);
	return;
}

/**
 * tny_header_iface_set_subject:
 * @self: a #TnyHeaderIface object
 * @subject: the subject header
 * 
 * Set the subject header
 * 
 **/
void
tny_header_iface_set_subject (TnyHeaderIface *self, const gchar *subject)
{
#ifdef DEBUG
	if (!TNY_HEADER_IFACE_GET_CLASS (self)->set_subject_func)
		g_critical ("You must implement tny_header_iface_set_subject\n");
#endif

	TNY_HEADER_IFACE_GET_CLASS (self)->set_subject_func (self, subject);
	return;
}

/**
 * tny_header_iface_set_to:
 * @self: a #TnyHeaderIface object
 * @to: the to header in a comma separated list
 * 
 * Set the to header.
 *
 * The format is a comma separated list like this:
 * Full name &gt;user@domain&lt;, Full name &gt;user@domain&lt;
 *
 * There are no quotes nor anything special. Just commas.
 * 
 **/
void
tny_header_iface_set_to (TnyHeaderIface *self, const gchar *to)
{
#ifdef DEBUG
	if (!TNY_HEADER_IFACE_GET_CLASS (self)->set_to_func)
		g_critical ("You must implement tny_header_iface_set_to\n");
#endif

	TNY_HEADER_IFACE_GET_CLASS (self)->set_to_func (self, to);
	return;
}

/**
 * tny_header_iface_get_cc:
 * @self: a #TnyHeaderIface object
 * 
 * Get the CC header
 * 
 * Return value: cc header
 **/
const gchar*
tny_header_iface_get_cc (TnyHeaderIface *self)
{
#ifdef DEBUG
	if (!TNY_HEADER_IFACE_GET_CLASS (self)->get_cc_func)
		g_critical ("You must implement tny_header_iface_get_cc\n");
#endif

	return TNY_HEADER_IFACE_GET_CLASS (self)->get_cc_func (self);
}

/**
 * tny_header_iface_get_bcc:
 * @self: a #TnyHeaderIface object
 * 
 * Get the BCC header
 * 
 * Return value: bcc header
 **/
const gchar*
tny_header_iface_get_bcc (TnyHeaderIface *self)
{
#ifdef DEBUG
	if (!TNY_HEADER_IFACE_GET_CLASS (self)->get_bcc_func)
		g_critical ("You must implement tny_header_iface_get_bcc\n");
#endif

	return TNY_HEADER_IFACE_GET_CLASS (self)->get_bcc_func (self);
}

/**
 * tny_header_iface_get_date_received:
 * @self: a #TnyHeaderIface object
 * 
 * Get the Date Received header as a time_t
 * 
 * Return value: date received header
 **/
time_t
tny_header_iface_get_date_received (TnyHeaderIface *self)
{
#ifdef DEBUG
	if (!TNY_HEADER_IFACE_GET_CLASS (self)->get_date_received_func)
		g_critical ("You must implement tny_header_iface_get_date_received\n");
#endif

	return TNY_HEADER_IFACE_GET_CLASS (self)->get_date_received_func (self);
}

/**
 * tny_header_iface_get_date_sent:
 * @self: a #TnyHeaderIface object
 * 
 * Get the Date Sent header as a time_t
 * 
 * Return value: date sent header
 **/
time_t
tny_header_iface_get_date_sent (TnyHeaderIface *self)
{
#ifdef DEBUG
	if (!TNY_HEADER_IFACE_GET_CLASS (self)->get_date_sent_func)
		g_critical ("You must implement tny_header_iface_get_date_sent\n");
#endif

	return TNY_HEADER_IFACE_GET_CLASS (self)->get_date_sent_func (self);
}


/**
 * tny_header_iface_get_id:
 * @self: a #TnyHeaderIface object
 * 
 * Get an unique id of the message of which self is a message header. The 
 * returned value should not be freed.
 * 
 * Return value: Unique follow-up uid as a read-only string
 *
 **/
const gchar*
tny_header_iface_get_uid (TnyHeaderIface *self)
{
#ifdef DEBUG
	if (!TNY_HEADER_IFACE_GET_CLASS (self)->get_uid_func)
		g_critical ("You must implement tny_header_iface_get_uid\n");
#endif

	return TNY_HEADER_IFACE_GET_CLASS (self)->get_uid_func (self);
}

/**
 * tny_header_iface_get_message_id:
 * @self: a #TnyHeaderIface object
 * 
 * Get an unique id of the message of which self is a message header. The 
 * returned value should not be freed.
 * 
 * Return value: message-id header as a read-only string
 *
 **/
const gchar*
tny_header_iface_get_message_id (TnyHeaderIface *self)
{
#ifdef DEBUG
	if (!TNY_HEADER_IFACE_GET_CLASS (self)->get_message_id_func)
		g_critical ("You must implement tny_header_iface_get_message_id\n");
#endif

	return TNY_HEADER_IFACE_GET_CLASS (self)->get_message_id_func (self);
}

/**
 * tny_header_iface_get_from:
 * @self: a #TnyHeaderIface object
 * 
 * Get the from header. The returned value should not be freed.
 * 
 * Return value: from header as a read-only string.
 *
 **/
const gchar* 
tny_header_iface_get_from (TnyHeaderIface *self)
{
#ifdef DEBUG
	if (!TNY_HEADER_IFACE_GET_CLASS (self)->get_from_func)
		g_critical ("You must implement tny_header_iface_get_from\n");
#endif

	return TNY_HEADER_IFACE_GET_CLASS (self)->get_from_func (self);
}

/**
 * tny_header_iface_get_subject:
 * @self: a #TnyHeaderIface object
 * 
 * Get the subject header. The returned value should not be freed.
 * 
 * Return value: subject header as a read-only string.
 *
 **/
const gchar*
tny_header_iface_get_subject (TnyHeaderIface *self)
{
#ifdef DEBUG
	if (!TNY_HEADER_IFACE_GET_CLASS (self)->get_subject_func)
		g_critical ("You must implement tny_header_iface_get_subject\n");
#endif

	return TNY_HEADER_IFACE_GET_CLASS (self)->get_subject_func (self);
}


/**
 * tny_header_iface_get_to:
 * @self: a #TnyHeaderIface object
 * 
 * Get the to header. The returned value should not be freed.
 * 
 * Return value: to header as a read-only string
 *
 **/
const gchar* 
tny_header_iface_get_to (TnyHeaderIface *self)
{
#ifdef DEBUG
	if (!TNY_HEADER_IFACE_GET_CLASS (self)->get_to_func)
		g_critical ("You must implement tny_header_iface_get_to\n");
#endif

	return TNY_HEADER_IFACE_GET_CLASS (self)->get_to_func (self);
}

/**
 * tny_header_iface_get_folder:
 * @self: a #TnyHeaderIface object
 * 
 * Get the parent folder where this message header is located.
 * The returned folder object should be unreferenced after use.
 * 
 * Return value: The folder of the message header
 *
 **/
TnyFolderIface* 
tny_header_iface_get_folder (TnyHeaderIface *self)
{
#ifdef DEBUG
	if (!TNY_HEADER_IFACE_GET_CLASS (self)->get_folder_func)
		g_critical ("You must implement tny_header_iface_get_folder\n");
#endif

	return TNY_HEADER_IFACE_GET_CLASS (self)->get_folder_func (self);
}



/**
 * tny_header_iface_get_flags:
 * @self: a #TnyHeaderIface object
 * 
 * Get message information flags.
 * 
 * Return value: flag bitmask
 **/
TnyHeaderFlags
tny_header_iface_get_flags (TnyHeaderIface *self)
{
#ifdef DEBUG
	if (!TNY_HEADER_IFACE_GET_CLASS (self)->get_flags_func)
		g_critical ("You must implement tny_header_iface_get_flags\n");
#endif

	return TNY_HEADER_IFACE_GET_CLASS (self)->get_flags_func (self);
}

/**
 * tny_header_iface_set_flags:
 * @self: a #TnyHeaderIface object
 * @mask: A #TnyHeaderFlags bitmask of flags to set.
 * 
 * Modify message flags.
 * 
 **/
void 
tny_header_iface_set_flags (TnyHeaderIface *self, TnyHeaderFlags mask)
{
#ifdef DEBUG
	if (!TNY_HEADER_IFACE_GET_CLASS (self)->set_flags_func)
		g_critical ("You must implement tny_header_iface_set_flags\n");
#endif

	return TNY_HEADER_IFACE_GET_CLASS (self)->set_flags_func (self, mask);
}

/**
 * tny_header_iface_unset_flags:
 * @self: a #TnyHeaderIface object
 * @mask: A #TnyHeaderFlags bitmask of flags to clear.
 * 
 * Modify message flags.
 * 
 **/
void 
tny_header_iface_unset_flags (TnyHeaderIface *self, TnyHeaderFlags mask)
{
#ifdef DEBUG
	if (!TNY_HEADER_IFACE_GET_CLASS (self)->unset_flags_func)
		g_critical ("You must implement tny_header_iface_unset_flags\n");
#endif

	return TNY_HEADER_IFACE_GET_CLASS (self)->unset_flags_func (self, mask);
}


static void
tny_header_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_header_iface_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyHeaderIfaceClass),
		  tny_header_iface_base_init,   /* base_init */
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
			"TnyHeaderIface", &info, 0);

		/* g_type_interface_add_prerequisite (type, G_TYPE_OBJECT); */
	}

	return type;
}


GType
tny_header_flags_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TNY_HEADER_FLAG_ANSWERED, "TNY_HEADER_FLAG_ANSWERED", "answered" },
      { TNY_HEADER_FLAG_DELETED, "TNY_HEADER_FLAG_DELETED", "deleted" },
      { TNY_HEADER_FLAG_DRAFT, "TNY_HEADER_FLAG_DRAFT", "draft" },
      { TNY_HEADER_FLAG_FLAGGED, "TNY_HEADER_FLAG_FLAGGED", "flagged" },
      { TNY_HEADER_FLAG_SEEN, "TNY_HEADER_FLAG_SEEN", "seen" },
      { TNY_HEADER_FLAG_ATTACHMENTS, "TNY_HEADER_FLAG_ATTACHMENTS", "attachments" },
      { TNY_HEADER_FLAG_ANSWERED_ALL, "TNY_HEADER_FLAG_ANSWERED_ALL", "answered_all" },
      { TNY_HEADER_FLAG_JUNK, "TNY_HEADER_FLAG_JUNK", "junk" },
      { TNY_HEADER_FLAG_SECURE, "TNY_HEADER_FLAG_SECURE", "secure" },
      { TNY_HEADER_FLAG_FOLDER_FLAGGED, "TNY_HEADER_FLAG_FLAGGED", "flagged" },
      { TNY_HEADER_FLAG_JUNK_LEARN, "TNY_HEADER_FLAG_LEARN", "learn" },
      { TNY_HEADER_FLAG_USER, "TNY_HEADER_FLAG_USER", "user" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyHeaderFlags", values);
  }
  return etype;
}

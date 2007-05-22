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

#include <tny-header.h>
#include <tny-folder.h>


/**
 * tny_header_set_replyto:
 * @self: a #TnyHeader object
 * @to: the reply-to header
 * 
 * Set the reply-to header
 **/
void
tny_header_set_replyto (TnyHeader *self, const gchar *to)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_HEADER (self));
	g_assert (to);
	g_assert (strlen (to) > 0);
	g_assert (TNY_HEADER_GET_IFACE (self)->set_replyto_func != NULL);
#endif

	TNY_HEADER_GET_IFACE (self)->set_replyto_func (self, to);

#ifdef DBC /* ensure */
	g_assert (!strcmp (tny_header_get_replyto (self), to));
#endif

	return;
}

/**
 * tny_header_get_replyto:
 * @self: a #TnyHeader object
 * 
 * Get the reply-to header
 * 
 * Return value: reply-to header, or NULL if not found
 **/
const gchar*
tny_header_get_replyto (TnyHeader *self)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_HEADER (self));
	g_assert (TNY_HEADER_GET_IFACE (self)->get_replyto_func != NULL);
#endif

	return TNY_HEADER_GET_IFACE (self)->get_replyto_func (self);
}

/**
 * tny_header_set_bcc:
 * @self: a #TnyHeader object
 * @bcc: the bcc header in a comma separated list
 * 
 * Set the bcc header. Look at the to header for more information
 * about formatting.
 * 
 **/
void
tny_header_set_bcc (TnyHeader *self, const gchar *bcc)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_HEADER (self));
	g_assert (bcc);
	g_assert (strlen (bcc) > 0);
	g_assert (TNY_HEADER_GET_IFACE (self)->set_bcc_func != NULL);
#endif

	TNY_HEADER_GET_IFACE (self)->set_bcc_func (self, bcc);

#ifdef DBC /* ensure */
	g_assert (!strcmp (tny_header_get_bcc (self), bcc));
#endif

	return;
}

/**
 * tny_header_set_cc:
 * @self: a #TnyHeader object
 * @cc: the cc header in a comma separated list
 * 
 * Set the cc header. Look at the to header for more information
 * about formatting.
 * 
 **/
void
tny_header_set_cc (TnyHeader *self, const gchar *cc)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_HEADER (self));
	g_assert (cc);
	g_assert (strlen (cc) > 0);
	g_assert (TNY_HEADER_GET_IFACE (self)->set_cc_func != NULL);
#endif

	TNY_HEADER_GET_IFACE (self)->set_cc_func (self, cc);

#ifdef DBC /* ensure */
	g_assert (!strcmp (tny_header_get_cc (self), cc));
#endif

	return;
}

/**
 * tny_header_set_from:
 * @self: a #TnyHeader object
 * @from: the from header
 * 
 * Set the from header
 * 
 **/
void
tny_header_set_from (TnyHeader *self, const gchar *from)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_HEADER (self));
	g_assert (from);
	g_assert (strlen (from) > 0);
	g_assert (TNY_HEADER_GET_IFACE (self)->set_from_func != NULL);
#endif

	TNY_HEADER_GET_IFACE (self)->set_from_func (self, from);

#ifdef DBC /* ensure */
	g_assert (!strcmp (tny_header_get_from (self), from));
#endif

	return;
}

/**
 * tny_header_set_subject:
 * @self: a #TnyHeader object
 * @subject: the subject header
 * 
 * Set the subject header
 * 
 **/
void
tny_header_set_subject (TnyHeader *self, const gchar *subject)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_HEADER (self));
	g_assert (subject);
	g_assert (strlen (subject) > 0);
	g_assert (TNY_HEADER_GET_IFACE (self)->set_subject_func != NULL);
#endif

	TNY_HEADER_GET_IFACE (self)->set_subject_func (self, subject);

#ifdef DBC /* ensure */
	g_assert (!strcmp (tny_header_get_subject (self), subject));
#endif

	return;
}

/**
 * tny_header_set_to:
 * @self: a #TnyHeader object
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
tny_header_set_to (TnyHeader *self, const gchar *to)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_HEADER (self));
	g_assert (to);
	g_assert (strlen (to) > 0);
	g_assert (TNY_HEADER_GET_IFACE (self)->set_to_func != NULL);
#endif

	TNY_HEADER_GET_IFACE (self)->set_to_func (self, to);

#ifdef DBC /* ensure */
	/* TNY TODO: A nice check would be one that checks whether the new to
	 * matches the just-set to, just comparing is not good enough, though:
	 * The implementation is allowed to change the formatting slightly. */
	g_assert (tny_header_get_to (self) != NULL);
#endif

	return;
}

/**
 * tny_header_get_cc:
 * @self: a #TnyHeader object
 * 
 * Get the CC header. The returned value should not be freed.
 * 
 * Return value: cc header as a read-only string, or NULL if not found
 *
 **/
const gchar*
tny_header_get_cc (TnyHeader *self)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_HEADER (self));
	g_assert (TNY_HEADER_GET_IFACE (self)->get_cc_func != NULL);
#endif

	return TNY_HEADER_GET_IFACE (self)->get_cc_func (self);
}

/**
 * tny_header_get_bcc:
 * @self: a #TnyHeader object
 * 
 * Get the BCC header. The returned value should not be freed.
 * 
 * Return value: bcc header as a read-only string, or NULL if not found
 *
 **/
const gchar*
tny_header_get_bcc (TnyHeader *self)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_HEADER (self));
	g_assert (TNY_HEADER_GET_IFACE (self)->get_bcc_func != NULL);
#endif

	return TNY_HEADER_GET_IFACE (self)->get_bcc_func (self);
}

/**
 * tny_header_get_date_received:
 * @self: a #TnyHeader object
 * 
 * Get the Date Received header as a time_t
 * 
 * Return value: date received header
 **/
time_t
tny_header_get_date_received (TnyHeader *self)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_HEADER (self));
	g_assert (TNY_HEADER_GET_IFACE (self)->get_date_received_func != NULL);
#endif

	return TNY_HEADER_GET_IFACE (self)->get_date_received_func (self);
}

/**
 * tny_header_get_date_sent:
 * @self: a #TnyHeader object
 * 
 * Get the Date Sent header as a time_t
 * 
 * Return value: date sent header
 **/
time_t
tny_header_get_date_sent (TnyHeader *self)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_HEADER (self));
	g_assert (TNY_HEADER_GET_IFACE (self)->get_date_sent_func != NULL);
#endif

	return TNY_HEADER_GET_IFACE (self)->get_date_sent_func (self);
}


/**
 * tny_header_get_uid:
 * @self: a #TnyHeader object
 * 
 * Get an unique id of the message of which self is a message header. The 
 * returned value should not be freed.
 * 
 * Return value: Unique follow-up uid as a read-only string, or NULL if not found
 *
 **/
const gchar*
tny_header_get_uid (TnyHeader *self)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_HEADER (self));
	g_assert (TNY_HEADER_GET_IFACE (self)->get_uid_func != NULL);
#endif

	return TNY_HEADER_GET_IFACE (self)->get_uid_func (self);
}

/**
 * tny_header_get_message_id:
 * @self: a #TnyHeader object
 * 
 * Get an unique id of the message of which self is a message header. The 
 * returned value should not be freed.
 * 
 * Return value: message-id header as a read-only string, or NULL if not found
 *
 **/
const gchar*
tny_header_get_message_id (TnyHeader *self)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_HEADER (self));
	g_assert (TNY_HEADER_GET_IFACE (self)->get_message_id_func != NULL);
#endif

	return TNY_HEADER_GET_IFACE (self)->get_message_id_func (self);
}


/**
 * tny_header_get_message_size:
 * @self: a #TnyHeader object
 * 
 * Get the expected message size
 * 
 * Return value: expected message size
 *
 **/
guint
tny_header_get_message_size (TnyHeader *self)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_HEADER (self));
	g_assert (TNY_HEADER_GET_IFACE (self)->get_message_size_func != NULL);
#endif


	return TNY_HEADER_GET_IFACE (self)->get_message_size_func (self);
}


/**
 * tny_header_get_from:
 * @self: a #TnyHeader object
 * 
 * Get the from header. The returned value should not be freed.
 * 
 * Return value: from header as a read-only string, or NULL if not found
 *
 **/
const gchar* 
tny_header_get_from (TnyHeader *self)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_HEADER (self));
	g_assert (TNY_HEADER_GET_IFACE (self)->get_from_func != NULL);
#endif

	return TNY_HEADER_GET_IFACE (self)->get_from_func (self);
}

/**
 * tny_header_get_subject:
 * @self: a #TnyHeader object
 * 
 * Get the subject header. The returned value should not be freed.
 * 
 * Return value: subject header as a read-only string, or NULL if not found
 *
 **/
const gchar*
tny_header_get_subject (TnyHeader *self)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_HEADER (self));
	g_assert (TNY_HEADER_GET_IFACE (self)->get_subject_func != NULL);
#endif

	return TNY_HEADER_GET_IFACE (self)->get_subject_func (self);
}


/**
 * tny_header_get_to:
 * @self: a #TnyHeader object
 * 
 * Get the to header. The returned value should not be freed.
 * 
 * Return value: to header as a read-only string, or NULL if not found
 *
 **/
const gchar* 
tny_header_get_to (TnyHeader *self)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_HEADER (self));
	g_assert (TNY_HEADER_GET_IFACE (self)->get_to_func != NULL);
#endif

	return TNY_HEADER_GET_IFACE (self)->get_to_func (self);
}

/**
 * tny_header_get_folder:
 * @self: a #TnyHeader object
 * 
 * Get the parent folder where this message header is located. This method can
 * return NULL in case the folder isn't know. The returned value,i if not NULL,
 * must be unreferenced after use.
 * 
 * Return value: The folder of the message header or NULL
 *
 **/
TnyFolder* 
tny_header_get_folder (TnyHeader *self)
{
	TnyFolder *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_HEADER (self));
	g_assert (TNY_HEADER_GET_IFACE (self)->get_folder_func != NULL);
#endif

	retval = TNY_HEADER_GET_IFACE (self)->get_folder_func (self);

#ifdef DBC /* ensure */
	if (retval)
		g_assert (TNY_IS_FOLDER (retval));
#endif

	return retval;
}



/**
 * tny_header_get_flags:
 * @self: a #TnyHeader object
 * 
 * Get message information flags.
 * 
 * Return value: flag bitmask
 **/
TnyHeaderFlags
tny_header_get_flags (TnyHeader *self)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_HEADER (self));
	g_assert (TNY_HEADER_GET_IFACE (self)->get_flags_func != NULL);
#endif

	return TNY_HEADER_GET_IFACE (self)->get_flags_func (self);
}

/**
 * tny_header_set_flags:
 * @self: a #TnyHeader object
 * @mask: A #TnyHeaderFlags bitmask of flags to set.
 * 
 * Modify message flags. Modifying the TNY_HEADER_FLAG_SEEN will trigger the 
 * notification of folder observers if @self was originated from a folder.
 * 
 **/
void 
tny_header_set_flags (TnyHeader *self, TnyHeaderFlags mask)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_HEADER (self));
	g_assert (TNY_HEADER_GET_IFACE (self)->set_flags_func != NULL);
#endif

	TNY_HEADER_GET_IFACE (self)->set_flags_func (self, mask);

#ifdef DBC /* ensure */
	/* TNY TODO: A check that ensures that all bits in mask are set */
#endif

	return;
}

/**
 * tny_header_unset_flags:
 * @self: a #TnyHeader object
 * @mask: A #TnyHeaderFlags bitmask of flags to clear.
 * 
 * Reset message flags. Modifying the TNY_HEADER_FLAG_SEEN will trigger the 
 * notification of folder observers if @self was originated from a folder.
 * 
 **/
void 
tny_header_unset_flags (TnyHeader *self, TnyHeaderFlags mask)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_HEADER (self));
	g_assert (TNY_HEADER_GET_IFACE (self)->unset_flags_func != NULL);
#endif

	TNY_HEADER_GET_IFACE (self)->unset_flags_func (self, mask);

#ifdef DBC /* ensure */
	/* TNY TODO: A check that ensures that all bits in mask are unset */
#endif

	return;
}


static void
tny_header_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_header_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyHeaderIface),
		  tny_header_base_init,   /* base_init */
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
			"TnyHeader", &info, 0);
	}

	return type;
}

/**
 * tny_header_flags_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
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
      { TNY_HEADER_FLAG_CACHED, "TNY_HEADER_FLAG_CACHED", "cached" },
      { TNY_HEADER_FLAG_PARTIAL, "TNY_HEADER_FLAG_PARTIAL", "partial" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyHeaderFlags", values);
  }
  return etype;
}

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

#include <tny-msg-folder-iface.h>
#include <tny-msg-header-iface.h>

guint *tny_msg_folder_iface_signals;


/**
 * tny_msg_folder_iface_expunge:
 * @self: a TnyMsgFolderIface object
 *
 * Sync changes made to a folder to its backing store, 
 * expunging deleted messages as well.
 **/
void 
tny_msg_folder_iface_expunge (TnyMsgFolderIface *self)
{
#ifdef DEBUG
	if (!TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->expunge_func)
		g_critical ("You must implement tny_msg_folder_iface_expunge\n");
#endif
	TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->expunge_func (self);
	return;
}

/**
 * tny_msg_folder_iface_remove_message:
 * @self: a TnyMsgFolderIface object
 * @header: the header of the message to remove
 *
 * Remove a message from a folder. This doesn't remove it from a #TnyListIface 
 * that would hold the headers (for example for a header summary view).
 **/
void 
tny_msg_folder_iface_remove_message (TnyMsgFolderIface *self, TnyMsgHeaderIface *header)
{
#ifdef DEBUG
	if (!TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->remove_message_func)
		g_critical ("You must implement tny_msg_folder_iface_remove_message\n");
#endif

	TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->remove_message_func (self, header);
	return;
}


/**
 * tny_msg_folder_iface_refresh_async:
 * @self: a TnyMsgFolderIface object
 * @callback: The callback handler (happens in the GMainLoop)
 * @status_callback: A callback for status notifications (in-thread)
 * @user_data: user data for the callback
 *
 * Refresh the folder and call back when finished
 * 
 **/
void
tny_msg_folder_iface_refresh_async (TnyMsgFolderIface *self, TnyRefreshFolderCallback callback, TnyRefreshFolderStatusCallback status_callback, gpointer user_data)
{
#ifdef DEBUG
	if (!TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->refresh_async_func)
		g_critical ("You must implement tny_msg_folder_iface_refresh_async\n");
#endif

	TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->refresh_async_func (self, callback, status_callback, user_data);
	return;
}



/**
 * tny_msg_folder_iface_refresh:
 * @self: a TnyMsgFolderIface object
 *
 * Refresh the folder
 * 
 **/
void
tny_msg_folder_iface_refresh (TnyMsgFolderIface *self)
{
#ifdef DEBUG
	if (!TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->refresh_func)
		g_critical ("You must implement tny_msg_folder_iface_refresh\n");
#endif

	TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->refresh_func (self);
	return;
}

/**
 * tny_msg_folder_iface_set_subscribed:
 * @self: a TnyMsgFolderIface object
 * @subscribed: Whether or not to subscribe to the folder
 *
 * Set the subscribed status of this folder.
 * 
 **/
void
tny_msg_folder_iface_set_subscribed (TnyMsgFolderIface *self, gboolean subscribed)
{
#ifdef DEBUG
	if (!TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->set_subscribed_func)
		g_critical ("You must implement tny_msg_folder_iface_set_subscribed\n");
#endif

	TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->set_subscribed_func (self, subscribed);
	return;
}

/**
 * tny_msg_folder_iface_get_subscribed:
 * @self: a TnyMsgFolderIface object
 * 
 * Get the subscribed status of this folder.
 * 
 * Return value: subscribe status
 **/
gboolean
tny_msg_folder_iface_get_subscribed (TnyMsgFolderIface *self)
{
#ifdef DEBUG
	if (!TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_subscribed_func)
		g_critical ("You must implement tny_msg_folder_iface_get_subscribed\n");
#endif

	return TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_subscribed_func (self);
}

/**
 * tny_msg_folder_iface_get_unread_count:
 * @self: a TnyMsgFolderIface object
 * 
 * Get the amount of unread messages in this folder.
 * 
 * Return value: amount of unread messages
 **/
guint
tny_msg_folder_iface_get_unread_count (TnyMsgFolderIface *self)
{
#ifdef DEBUG
	if (!TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_unread_count_func)
		g_critical ("You must implement tny_msg_folder_iface_get_unread_count\n");
#endif

	return TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_unread_count_func (self);
}


guint
tny_msg_folder_iface_get_all_count (TnyMsgFolderIface *self)
{
#ifdef DEBUG
	if (!TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_all_count_func)
		g_critical ("You must implement tny_msg_folder_iface_get_all_count\n");
#endif

	return TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_all_count_func (self);
}

/**
 * tny_msg_folder_iface_get_account:
 * @self: a TnyMsgFolderIface object
 * 
 * Get a reference to the parent account of this folder
 * 
 * Return value: the account of this folder
 **/
TnyAccountIface*  
tny_msg_folder_iface_get_account (TnyMsgFolderIface *self)
{
#ifdef DEBUG
	if (!TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_account_func)
		g_critical ("You must implement tny_msg_folder_iface_get_account\n");
#endif

	return TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_account_func (self);
}

/**
 * tny_msg_folder_iface_set_account:
 * @self: a TnyMsgFolderIface object
 * 
 * Set the parent of this folder
 * 
 **/
void
tny_msg_folder_iface_set_account (TnyMsgFolderIface *self, TnyAccountIface *account)
{
#ifdef DEBUG
	if (!TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->set_account_func)
		g_critical ("You must implement tny_msg_folder_iface_set_account\n");
#endif

	TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->set_account_func (self, account);

	return;
}

/**
 * tny_msg_folder_iface_get_folders:
 * @self: a TnyMsgFolderIface object
 * 
 * Get the child folders of this folder
 * 
 * Return value: A read-only #TnyListIface with TnyMsgFolderIface instances
 **/
TnyListIface*
tny_msg_folder_iface_get_folders (TnyMsgFolderIface *self)
{
#ifdef DEBUG
	if (!TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_folders_func)
		g_critical ("You must implement tny_msg_folder_iface_get_folders\n");
#endif

	return TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_folders_func (self);
}


/**
 * tny_msg_folder_iface_get_message:
 * @self: a TnyMsgFolderIface object
 * @header: the header of the message to get
 * 
 * Get a message in the folder by header
 * 
 * Return value: The message instance or NULL on failure
 **/
TnyMsgIface*
tny_msg_folder_iface_get_message (TnyMsgFolderIface *self, TnyMsgHeaderIface *header)
{
#ifdef DEBUG
	if (!TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_message_func)
		g_critical ("You must implement tny_msg_folder_iface_get_message\n");
#endif

	return TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_message_func (self, header);
}


/**
 * tny_msg_folder_iface_get_headers:
 * @self: a TnyMsgFolderIface object
 * @headers: A #TnyListIface instance where the headers will be put
 * @refresh: whether or not to synchronize with the server first
 * 
 * Get a list of message header instances that are in this folder
 * 
 **/
void
tny_msg_folder_iface_get_headers (TnyMsgFolderIface *self, TnyListIface *headers, gboolean refresh)
{
#ifdef DEBUG
	if (!TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_headers_func)
		g_critical ("You must implement tny_msg_folder_iface_get_headers\n");
#endif

	return TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_headers_func (self, headers, refresh);
}

/**
 * tny_msg_folder_iface_get_id:
 * @self: a TnyMsgFolderIface object
 * 
 * Get an unique id for this folder (unique per account)
 * 
 * Return value: A unique id
 **/
const gchar*
tny_msg_folder_iface_get_id (TnyMsgFolderIface *self)
{
#ifdef DEBUG
	if (!TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_id_func)
		g_critical ("You must implement tny_msg_folder_iface_get_id\n");
#endif

	return TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_id_func (self);
}

/**
 * tny_msg_folder_iface_get_name:
 * @self: a TnyMsgFolderIface object
 * 
 * Get the displayable name of this folder
 * 
 * Return value: The folder name
 **/
const gchar*
tny_msg_folder_iface_get_name (TnyMsgFolderIface *self)
{
#ifdef DEBUG
	if (!TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_name_func)
		g_critical ("You must implement tny_msg_folder_iface_get_name\n");
#endif

	return TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_name_func (self);
}

/**
 * tny_msg_folder_iface_set_id:
 * @self: a TnyMsgFolderIface object
 * @id: an unique id
 * 
 * Set the unique id for this folder (unique per account)
 * 
 **/
void
tny_msg_folder_iface_set_id (TnyMsgFolderIface *self, const gchar *id)
{
#ifdef DEBUG
	if (!TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->set_id_func)
		g_critical ("You must implement tny_msg_folder_iface_set_id\n");
#endif

	TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->set_id_func (self, id);
	return;
}

/**
 * tny_msg_folder_iface_set_name:
 * @self: a TnyMsgFolderIface object
 * @name: an unique id
 * 
 * Set the displayable name of this folder
 * 
 **/
void
tny_msg_folder_iface_set_name (TnyMsgFolderIface *self, const gchar *name)
{
#ifdef DEBUG
	if (!TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->set_name_func)
		g_critical ("You must implement tny_msg_folder_iface_set_name\n");
#endif

	TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->set_name_func (self, name);
	return;
}



/**
 * tny_msg_folder_iface_get_folder_type:
 * @self: a TnyMsgFolderIface object
 * 
 * Get the type of the folder (Inbox, Outbox etc.) 
 * 
 **/
TnyMsgFolderType tny_msg_folder_iface_get_folder_type  (TnyMsgFolderIface *self)
{
#ifdef DEBUG
	if (!TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_folder_type_func)
		g_critical ("You must implement tny_msg_folder_iface_get_folder_type\n");
#endif

	return TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->get_folder_type_func (self);
}



/**
 * tny_msg_folder_iface_uncache:
 * @self: a TnyMsgFolderIface object
 * 
 * If it's possible to uncache this instance, uncache it
 * 
 **/
void
tny_msg_folder_iface_uncache (TnyMsgFolderIface *self)
{
	if (TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->uncache_func != NULL)
		TNY_MSG_FOLDER_IFACE_GET_CLASS (self)->uncache_func (self);
	return;
}

/**
 * tny_msg_folder_iface_has_cache:
 * @self: a TnyMsgFolderIface object
 * 
 * If it's possible to uncache this instance, return whether or not it has a cache
 * 
 * Return value: Whether or not this instance has a cache
 **/
gboolean
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

	if (!initialized) 
	{

		tny_msg_folder_iface_signals = g_new0 (guint, TNY_MSG_FOLDER_IFACE_LAST_SIGNAL);

/**
 * TnyMsgFolderIface::folder_inserted:
 * @self: the object on which the signal is emitted
 * @arg1: The folder that got inserted
 * @folder: the #TnyMsgFolderIface as the added folder
 *
 * Emitted when a folder gets added to the folder
 **/
		tny_msg_folder_iface_signals[TNY_MSG_FOLDER_IFACE_FOLDER_INSERTED] =
		   g_signal_new ("folder_inserted",
			TNY_TYPE_MSG_FOLDER_IFACE,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnyMsgFolderIfaceClass, folder_inserted),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE, 1, TNY_TYPE_MSG_FOLDER_IFACE);

/**
 * TnyMsgFolderIface::folders_reloaded:
 * @self: the object on which the signal is emitted
 *
 * Emitted when the folder gets reloaded.
 */
		tny_msg_folder_iface_signals[TNY_MSG_FOLDER_IFACE_FOLDERS_RELOADED] =
		   g_signal_new ("folders_reloaded",
			TNY_TYPE_MSG_FOLDER_IFACE,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnyMsgFolderIfaceClass, folders_reloaded),
			NULL, NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE, 0);


		initialized = TRUE;
	}
}

GType
tny_msg_folder_iface_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
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


GType
tny_msg_folder_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TNY_MSG_FOLDER_TYPE_UNKNOWN, "TNY_MSG_FOLDER_TYPE_UNKNOWN", "unknown" },
      { TNY_MSG_FOLDER_TYPE_NORMAL, "TNY_MSG_FOLDER_TYPE_NORMAL", "normal" },
      { TNY_MSG_FOLDER_TYPE_INBOX, "TNY_MSG_FOLDER_TYPE_INBOX", "inbox" },
      { TNY_MSG_FOLDER_TYPE_OUTBOX, "TNY_MSG_FOLDER_TYPE_OUTBOX", "outbox" },
      { TNY_MSG_FOLDER_TYPE_TRASH, "TNY_MSG_FOLDER_TYPE_TRASH", "trash" },
      { TNY_MSG_FOLDER_TYPE_JUNK, "TNY_MSG_FOLDER_TYPE_JUNK", "junk" },
      { TNY_MSG_FOLDER_TYPE_SENT, "TNY_MSG_FOLDER_TYPE_SENT", "sent" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyMsgFolderType", values);
  }
  return etype;
}

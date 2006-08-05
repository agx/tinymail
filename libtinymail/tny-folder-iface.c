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

#include <tny-folder-iface.h>
#include <tny-header-iface.h>

guint *tny_folder_iface_signals;


/**
 * tny_folder_iface_expunge:
 * @self: a TnyFolderIface object
 *
 * Sync changes made to a folder to its backing store, 
 * expunging deleted messages as well.
 **/
void 
tny_folder_iface_expunge (TnyFolderIface *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_IFACE_GET_CLASS (self)->expunge_func)
		g_critical ("You must implement tny_folder_iface_expunge\n");
#endif
	TNY_FOLDER_IFACE_GET_CLASS (self)->expunge_func (self);
	return;
}

/**
 * tny_folder_iface_remove_message:
 * @self: a TnyFolderIface object
 * @header: the header of the message to remove
 *
 * Remove a message from a folder. This doesn't remove it from a #TnyListIface 
 * that would hold the headers (for example for a header summary view).
 **/
void 
tny_folder_iface_remove_message (TnyFolderIface *self, TnyHeaderIface *header)
{
#ifdef DEBUG
	if (!TNY_FOLDER_IFACE_GET_CLASS (self)->remove_message_func)
		g_critical ("You must implement tny_folder_iface_remove_message\n");
#endif

	TNY_FOLDER_IFACE_GET_CLASS (self)->remove_message_func (self, header);
	return;
}


/**
 * tny_folder_iface_refresh_async:
 * @self: a TnyFolderIface object
 * @callback: The callback handler (happens in the GMainLoop)
 * @status_callback: A callback for status notifications (in-thread)
 * @user_data: user data for the callback
 *
 * Refresh the folder and call back when finished
 * 
 **/
void
tny_folder_iface_refresh_async (TnyFolderIface *self, TnyRefreshFolderCallback callback, TnyRefreshFolderStatusCallback status_callback, gpointer user_data)
{
#ifdef DEBUG
	if (!TNY_FOLDER_IFACE_GET_CLASS (self)->refresh_async_func)
		g_critical ("You must implement tny_folder_iface_refresh_async\n");
#endif

	TNY_FOLDER_IFACE_GET_CLASS (self)->refresh_async_func (self, callback, status_callback, user_data);
	return;
}



/**
 * tny_folder_iface_refresh:
 * @self: a TnyFolderIface object
 *
 * Refresh the folder
 * 
 **/
void
tny_folder_iface_refresh (TnyFolderIface *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_IFACE_GET_CLASS (self)->refresh_func)
		g_critical ("You must implement tny_folder_iface_refresh\n");
#endif

	TNY_FOLDER_IFACE_GET_CLASS (self)->refresh_func (self);
	return;
}

/**
 * tny_folder_iface_set_subscribed:
 * @self: a TnyFolderIface object
 * @subscribed: Whether or not to subscribe to the folder
 *
 * Set the subscribed status of this folder.
 * 
 **/
void
tny_folder_iface_set_subscribed (TnyFolderIface *self, gboolean subscribed)
{
#ifdef DEBUG
	if (!TNY_FOLDER_IFACE_GET_CLASS (self)->set_subscribed_func)
		g_critical ("You must implement tny_folder_iface_set_subscribed\n");
#endif

	TNY_FOLDER_IFACE_GET_CLASS (self)->set_subscribed_func (self, subscribed);
	return;
}

/**
 * tny_folder_iface_get_subscribed:
 * @self: a TnyFolderIface object
 * 
 * Get the subscribed status of this folder.
 * 
 * Return value: subscribe status
 **/
gboolean
tny_folder_iface_get_subscribed (TnyFolderIface *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_IFACE_GET_CLASS (self)->get_subscribed_func)
		g_critical ("You must implement tny_folder_iface_get_subscribed\n");
#endif

	return TNY_FOLDER_IFACE_GET_CLASS (self)->get_subscribed_func (self);
}

/**
 * tny_folder_iface_get_unread_count:
 * @self: a TnyFolderIface object
 * 
 * Get the amount of unread messages in this folder.
 * 
 * Return value: amount of unread messages
 **/
guint
tny_folder_iface_get_unread_count (TnyFolderIface *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_IFACE_GET_CLASS (self)->get_unread_count_func)
		g_critical ("You must implement tny_folder_iface_get_unread_count\n");
#endif

	return TNY_FOLDER_IFACE_GET_CLASS (self)->get_unread_count_func (self);
}


guint
tny_folder_iface_get_all_count (TnyFolderIface *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_IFACE_GET_CLASS (self)->get_all_count_func)
		g_critical ("You must implement tny_folder_iface_get_all_count\n");
#endif

	return TNY_FOLDER_IFACE_GET_CLASS (self)->get_all_count_func (self);
}

/**
 * tny_folder_iface_get_account:
 * @self: a TnyFolderIface object
 * 
 * Get a reference to the parent account of this folder
 * 
 * Return value: the account of this folder
 **/
TnyAccountIface*  
tny_folder_iface_get_account (TnyFolderIface *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_IFACE_GET_CLASS (self)->get_account_func)
		g_critical ("You must implement tny_folder_iface_get_account\n");
#endif

	return TNY_FOLDER_IFACE_GET_CLASS (self)->get_account_func (self);
}

/**
 * tny_folder_iface_set_account:
 * @self: a TnyFolderIface object
 * 
 * Set the parent of this folder
 * 
 **/
void
tny_folder_iface_set_account (TnyFolderIface *self, TnyAccountIface *account)
{
#ifdef DEBUG
	if (!TNY_FOLDER_IFACE_GET_CLASS (self)->set_account_func)
		g_critical ("You must implement tny_folder_iface_set_account\n");
#endif

	TNY_FOLDER_IFACE_GET_CLASS (self)->set_account_func (self, account);

	return;
}

/**
 * tny_folder_iface_get_folders:
 * @self: a TnyFolderIface object
 * 
 * Get the child folders of this folder
 * 
 * Return value: A read-only #TnyListIface with TnyFolderIface instances
 **/
TnyListIface*
tny_folder_iface_get_folders (TnyFolderIface *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_IFACE_GET_CLASS (self)->get_folders_func)
		g_critical ("You must implement tny_folder_iface_get_folders\n");
#endif

	return TNY_FOLDER_IFACE_GET_CLASS (self)->get_folders_func (self);
}


/**
 * tny_folder_iface_get_message:
 * @self: a TnyFolderIface object
 * @header: the header of the message to get
 * 
 * Get a message in the folder by header
 * 
 * Return value: The message instance or NULL on failure
 **/
TnyMsgIface*
tny_folder_iface_get_message (TnyFolderIface *self, TnyHeaderIface *header)
{
#ifdef DEBUG
	if (!TNY_FOLDER_IFACE_GET_CLASS (self)->get_message_func)
		g_critical ("You must implement tny_folder_iface_get_message\n");
#endif

	return TNY_FOLDER_IFACE_GET_CLASS (self)->get_message_func (self, header);
}


/**
 * tny_folder_iface_get_headers:
 * @self: a TnyFolderIface object
 * @headers: A #TnyListIface instance where the headers will be put
 * @refresh: whether or not to synchronize with the server first
 * 
 * Get a list of message header instances that are in this folder
 * 
 **/
void
tny_folder_iface_get_headers (TnyFolderIface *self, TnyListIface *headers, gboolean refresh)
{
#ifdef DEBUG
	if (!TNY_FOLDER_IFACE_GET_CLASS (self)->get_headers_func)
		g_critical ("You must implement tny_folder_iface_get_headers\n");
#endif

	return TNY_FOLDER_IFACE_GET_CLASS (self)->get_headers_func (self, headers, refresh);
}

/**
 * tny_folder_iface_get_id:
 * @self: a TnyFolderIface object
 * 
 * Get an unique id for this folder (unique per account)
 * 
 * Return value: A unique id
 **/
const gchar*
tny_folder_iface_get_id (TnyFolderIface *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_IFACE_GET_CLASS (self)->get_id_func)
		g_critical ("You must implement tny_folder_iface_get_id\n");
#endif

	return TNY_FOLDER_IFACE_GET_CLASS (self)->get_id_func (self);
}

/**
 * tny_folder_iface_get_name:
 * @self: a TnyFolderIface object
 * 
 * Get the displayable name of this folder
 * 
 * Return value: The folder name
 **/
const gchar*
tny_folder_iface_get_name (TnyFolderIface *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_IFACE_GET_CLASS (self)->get_name_func)
		g_critical ("You must implement tny_folder_iface_get_name\n");
#endif

	return TNY_FOLDER_IFACE_GET_CLASS (self)->get_name_func (self);
}

/**
 * tny_folder_iface_set_id:
 * @self: a TnyFolderIface object
 * @id: an unique id
 * 
 * Set the unique id for this folder (unique per account)
 * 
 **/
void
tny_folder_iface_set_id (TnyFolderIface *self, const gchar *id)
{
#ifdef DEBUG
	if (!TNY_FOLDER_IFACE_GET_CLASS (self)->set_id_func)
		g_critical ("You must implement tny_folder_iface_set_id\n");
#endif

	TNY_FOLDER_IFACE_GET_CLASS (self)->set_id_func (self, id);
	return;
}

/**
 * tny_folder_iface_set_name:
 * @self: a TnyFolderIface object
 * @name: an unique id
 * 
 * Set the displayable name of this folder
 * 
 **/
void
tny_folder_iface_set_name (TnyFolderIface *self, const gchar *name)
{
#ifdef DEBUG
	if (!TNY_FOLDER_IFACE_GET_CLASS (self)->set_name_func)
		g_critical ("You must implement tny_folder_iface_set_name\n");
#endif

	TNY_FOLDER_IFACE_GET_CLASS (self)->set_name_func (self, name);
	return;
}



/**
 * tny_folder_iface_get_folder_type:
 * @self: a TnyFolderIface object
 * 
 * Get the type of the folder (Inbox, Outbox etc.) 
 * 
 **/
TnyFolderType tny_folder_iface_get_folder_type  (TnyFolderIface *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_IFACE_GET_CLASS (self)->get_folder_type_func)
		g_critical ("You must implement tny_folder_iface_get_folder_type\n");
#endif

	return TNY_FOLDER_IFACE_GET_CLASS (self)->get_folder_type_func (self);
}



/**
 * tny_folder_iface_uncache:
 * @self: a TnyFolderIface object
 * 
 * If it's possible to uncache this instance, uncache it
 * 
 **/
void
tny_folder_iface_uncache (TnyFolderIface *self)
{
	if (TNY_FOLDER_IFACE_GET_CLASS (self)->uncache_func != NULL)
		TNY_FOLDER_IFACE_GET_CLASS (self)->uncache_func (self);
	return;
}

/**
 * tny_folder_iface_has_cache:
 * @self: a TnyFolderIface object
 * 
 * If it's possible to uncache this instance, return whether or not it has a cache
 * 
 * Return value: Whether or not this instance has a cache
 **/
gboolean
tny_folder_iface_has_cache (TnyFolderIface *self)
{
	if (TNY_FOLDER_IFACE_GET_CLASS (self)->has_cache_func != NULL)
		TNY_FOLDER_IFACE_GET_CLASS (self)->has_cache_func (self);
	return;
}

static void
tny_folder_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) 
	{

		tny_folder_iface_signals = g_new0 (guint, TNY_FOLDER_IFACE_LAST_SIGNAL);

/**
 * TnyFolderIface::folder_inserted:
 * @self: the object on which the signal is emitted
 * @arg1: The folder that got inserted
 * @folder: the #TnyFolderIface as the added folder
 *
 * Emitted when a folder gets added to the folder
 **/
		tny_folder_iface_signals[TNY_FOLDER_IFACE_FOLDER_INSERTED] =
		   g_signal_new ("folder_inserted",
			TNY_TYPE_FOLDER_IFACE,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnyFolderIfaceClass, folder_inserted),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE, 1, TNY_TYPE_FOLDER_IFACE);

/**
 * TnyFolderIface::folders_reloaded:
 * @self: the object on which the signal is emitted
 *
 * Emitted when the folder gets reloaded.
 */
		tny_folder_iface_signals[TNY_FOLDER_IFACE_FOLDERS_RELOADED] =
		   g_signal_new ("folders_reloaded",
			TNY_TYPE_FOLDER_IFACE,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnyFolderIfaceClass, folders_reloaded),
			NULL, NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE, 0);


		initialized = TRUE;
	}
}

GType
tny_folder_iface_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyFolderIfaceClass),
		  tny_folder_iface_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyFolderIface", &info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}

	return type;
}


GType
tny_folder_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TNY_FOLDER_TYPE_UNKNOWN, "TNY_FOLDER_TYPE_UNKNOWN", "unknown" },
      { TNY_FOLDER_TYPE_NORMAL, "TNY_FOLDER_TYPE_NORMAL", "normal" },
      { TNY_FOLDER_TYPE_INBOX, "TNY_FOLDER_TYPE_INBOX", "inbox" },
      { TNY_FOLDER_TYPE_OUTBOX, "TNY_FOLDER_TYPE_OUTBOX", "outbox" },
      { TNY_FOLDER_TYPE_TRASH, "TNY_FOLDER_TYPE_TRASH", "trash" },
      { TNY_FOLDER_TYPE_JUNK, "TNY_FOLDER_TYPE_JUNK", "junk" },
      { TNY_FOLDER_TYPE_SENT, "TNY_FOLDER_TYPE_SENT", "sent" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyFolderType", values);
  }
  return etype;
}

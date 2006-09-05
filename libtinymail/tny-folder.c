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

#include <tny-folder.h>
#include <tny-folder-store.h>
#include <tny-header.h>

guint *tny_folder_signals;


/**
 * tny_folder_expunge:
 * @self: a TnyFolder object
 *
 * Sync changes made to a folder to its backing store, expunging deleted 
 * messages (the ones marked with TNY_HEADER_FLAG_DELETED) as well.
 *
 **/
void 
tny_folder_expunge (TnyFolder *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->expunge_func)
		g_critical ("You must implement tny_folder_expunge\n");
#endif
	TNY_FOLDER_GET_IFACE (self)->expunge_func (self);
	return;
}

/**
 * tny_folder_remove_message:
 * @self: a TnyFolder object
 * @header: the header of the message to remove
 *
 * Remove a message from a folder. This doesn't remove it from a #TnyList 
 * that holds the headers (for example for a header summary view) after doing
 * the tny_folder_get_headers method.
 *
 * This method also doesn't truely remove the header from the folder. It only
 * marks it as removed (it sets the TNY_HEADER_FLAG_DELETED). If you perform
 * tny_folder_expunge on the folder, it will really be removed.
 *
 * This means that the tny_folder_get_headers method will still prepend the
 * removed message in the list until the expunge happened. You are advised to
 * hide messages that have been marked as being deleted from your summary view.
 * 
 * In Gtk+ for the #GtkTreeView component, you can do this using the 
 * #GtkTreeModelFilter tree model filtering model.
 *
 **/
void 
tny_folder_remove_message (TnyFolder *self, TnyHeader *header)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->remove_message_func)
		g_critical ("You must implement tny_folder_remove_message\n");
#endif

	TNY_FOLDER_GET_IFACE (self)->remove_message_func (self, header);
	return;
}


/**
 * tny_folder_refresh_async:
 * @self: a TnyFolder object
 * @callback: The callback handler
 * @status_callback: A callback for status notifications
 * @user_data: user data for the callback
 *
 * Refresh the folder and call back when finished.
 *
 * If you want to use this functionality, it's advised to let your application 
 * use the #GMainLoop. All Gtk+ applications have this once gtk_main () is
 * called.
 * 
 * When using a #GMainLoop this method will callback using g_idle_add_full.
 * Without a #GMainLoop, which the libtinymail-camel implementation detects
 * using (g_main_depth > 0), the callbacks will happen in a worker thread at an
 * unknown moment in time (check your locking).
 *
 * When using Gtk+, the callback doesn't need gdk_threads_enter and 
 * gdk_threads_leave in Gtk+.
 *
 **/
void
tny_folder_refresh_async (TnyFolder *self, TnyRefreshFolderCallback callback, TnyRefreshFolderStatusCallback status_callback, gpointer user_data)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->refresh_async_func)
		g_critical ("You must implement tny_folder_refresh_async\n");
#endif

	TNY_FOLDER_GET_IFACE (self)->refresh_async_func (self, callback, status_callback, user_data);
	return;
}



/**
 * tny_folder_refresh:
 * @self: a TnyFolder object
 *
 * Refresh the folder. This gets the summary information from the E-Mail service
 * and writes it to the the on-disk cache or updates it.
 *
 * After this method, tny_folder_get_all_count and 
 * tny_folder_get_unread_count are guaranteed to be correct.
 *
 * Also read about tny_folder_get_headers.
 *
 **/
void
tny_folder_refresh (TnyFolder *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->refresh_func)
		g_critical ("You must implement tny_folder_refresh\n");
#endif

	TNY_FOLDER_GET_IFACE (self)->refresh_func (self);
	return;
}

/**
 * tny_folder_set_subscribed:
 * @self: a TnyFolder object
 * @subscribed: Whether or not to subscribe to the folder
 *
 * WARNING: This API might soon change
 *
 * Set the subscribed status of this folder.
 * 
 **/
void
tny_folder_set_subscribed (TnyFolder *self, gboolean subscribed)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->set_subscribed_func)
		g_critical ("You must implement tny_folder_set_subscribed\n");
#endif

	TNY_FOLDER_GET_IFACE (self)->set_subscribed_func (self, subscribed);
	return;
}

/**
 * tny_folder_get_subscribed:
 * @self: a TnyFolder object
 * 
 * WARNING: This API might soon change
 *
 * Get the subscribed status of this folder.
 * 
 * Return value: subscribe status
 **/
gboolean
tny_folder_get_subscribed (TnyFolder *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->get_subscribed_func)
		g_critical ("You must implement tny_folder_get_subscribed\n");
#endif

	return TNY_FOLDER_GET_IFACE (self)->get_subscribed_func (self);
}

/**
 * tny_folder_get_unread_count:
 * @self: a TnyFolder object
 * 
 * Get the amount of unread messages in this folder. The value is only
 * garuanteed to be correct after tny_folder_refresh.
 * 
 * Return value: amount of unread messages
 *
 **/
guint
tny_folder_get_unread_count (TnyFolder *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->get_unread_count_func)
		g_critical ("You must implement tny_folder_get_unread_count\n");
#endif

	return TNY_FOLDER_GET_IFACE (self)->get_unread_count_func (self);
}

/**
 * tny_folder_get_all_count:
 * @self: a TnyFolder object
 * 
 * Get the amount of messages in this folder. The value is only
 * garuanteed to be correct after tny_folder_refresh.
 * 
 * Return value: amount of messages
 *
 **/
guint
tny_folder_get_all_count (TnyFolder *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->get_all_count_func)
		g_critical ("You must implement tny_folder_get_all_count\n");
#endif

	return TNY_FOLDER_GET_IFACE (self)->get_all_count_func (self);
}

/**
 * tny_folder_get_account:
 * @self: a TnyFolder object
 * 
 * Get a reference to the parent account of this folder
 * 
 * Return value: the account of this folder
 *
 **/
TnyStoreAccount*  
tny_folder_get_account (TnyFolder *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->get_account_func)
		g_critical ("You must implement tny_folder_get_account\n");
#endif

	return TNY_FOLDER_GET_IFACE (self)->get_account_func (self);
}



/**
 * tny_folder_get_message:
 * @self: a TnyFolder object
 * @header: the header of the message to get
 * 
 * Get a message in the folder identified by a header
 * 
 * Return value: The message instance or NULL on failure
 *
 **/
TnyMsg*
tny_folder_get_message (TnyFolder *self, TnyHeader *header)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->get_message_func)
		g_critical ("You must implement tny_folder_get_message\n");
#endif

	return TNY_FOLDER_GET_IFACE (self)->get_message_func (self, header);
}


/**
 * tny_folder_get_headers:
 * @self: a TnyFolder object
 * @headers: A #TnyList instance where the headers will be put
 * @refresh: whether or not to synchronize with the server first
 * 
 * Get a list of message header instances that are in this folder. Also read
 * about tny_folder_refresh.
 *
 **/
void
tny_folder_get_headers (TnyFolder *self, TnyList *headers, gboolean refresh)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->get_headers_func)
		g_critical ("You must implement tny_folder_get_headers\n");
#endif

	return TNY_FOLDER_GET_IFACE (self)->get_headers_func (self, headers, refresh);
}

/**
 * tny_folder_get_id:
 * @self: a TnyFolder object
 * 
 * Get an unique id for this folder (unique per account). The ID will be a 
 * a "/" separated string like "INBOX/parent-folder/folder" depending on the
 * service type (the example is for IMAP using the libtinymail-camel 
 * implementation).
 *
 * The ID is guaranteed to be unique per account. You should not free the result
 * of this method.
 * 
 * Return value: A unique id
 *
 **/
const gchar*
tny_folder_get_id (TnyFolder *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->get_id_func)
		g_critical ("You must implement tny_folder_get_id\n");
#endif

	return TNY_FOLDER_GET_IFACE (self)->get_id_func (self);
}

/**
 * tny_folder_get_name:
 * @self: a TnyFolder object
 * 
 * Get a displayable name for this folder. You should not free the result of 
 * this method.
 * 
 * Return value: The folder name
 *
 **/
const gchar*
tny_folder_get_name (TnyFolder *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->get_name_func)
		g_critical ("You must implement tny_folder_get_name\n");
#endif

	return TNY_FOLDER_GET_IFACE (self)->get_name_func (self);
}


/**
 * tny_folder_set_name:
 * @self: a TnyFolder object
 * @name: a new name for the folder
 * 
 * Rename a folder. Most services require the name to be unique in the 
 * parent folder.
 *
 **/
void
tny_folder_set_name (TnyFolder *self, const gchar *name)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->set_name_func)
		g_critical ("You must implement tny_folder_set_name\n");
#endif

	TNY_FOLDER_GET_IFACE (self)->set_name_func (self, name);
	return;
}



/**
 * tny_folder_get_folder_type:
 * @self: a TnyFolder object
 * 
 * Get the type of the folder (Inbox, Outbox etc.) 
 * 
 * Return value: The folder type as  a #TnyFolderType enum
 *
 **/
TnyFolderType 
tny_folder_get_folder_type  (TnyFolder *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->get_folder_type_func)
		g_critical ("You must implement tny_folder_get_folder_type\n");
#endif

	return TNY_FOLDER_GET_IFACE (self)->get_folder_type_func (self);
}




static void
tny_folder_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) 
	{

		tny_folder_signals = g_new0 (guint, TNY_FOLDER_LAST_SIGNAL);

/**
 * TnyFolder::folder_inserted:
 * @self: the object on which the signal is emitted
 * @arg1: The folder that got inserted
 * @folder: the #TnyFolder as the added folder
 *
 * Emitted when a folder gets added to the folder
 **/
		tny_folder_signals[TNY_FOLDER_FOLDER_INSERTED] =
		   g_signal_new ("folder_inserted",
			TNY_TYPE_FOLDER,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnyFolderIface, folder_inserted),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE, 1, TNY_TYPE_FOLDER);

/**
 * TnyFolder::folders_reloaded:
 * @self: the object on which the signal is emitted
 *
 * Emitted when the folder gets reloaded.
 */
		tny_folder_signals[TNY_FOLDER_FOLDERS_RELOADED] =
		   g_signal_new ("folders_reloaded",
			TNY_TYPE_FOLDER,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnyFolderIface, folders_reloaded),
			NULL, NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE, 0);


		initialized = TRUE;
	}
}

GType
tny_folder_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyFolderIface),
		  tny_folder_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL,    /* instance_init */
		  NULL
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyFolder", &info, 0);

		g_type_interface_add_prerequisite (type, TNY_TYPE_FOLDER_STORE);
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

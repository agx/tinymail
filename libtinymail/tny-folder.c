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

#include <tny-folder-store.h>
#include <tny-header.h>

#include <tny-folder.h>
guint tny_folder_signals [TNY_FOLDER_LAST_SIGNAL];


/**
 * tny_folder_get_msg_remove_strategy:
 * @self: a TnyFolder object
 *
 * Get the strategy for removing a message. The return value of this method
 * must be unreferenced after use.
 *
 * Implementors: This method must return the strategy for removing a message.
 * being the implementer, you must add a reference before returning the instance.
 *
 * Return value: the strategy for removing a message
 **/
TnyMsgRemoveStrategy* 
tny_folder_get_msg_remove_strategy (TnyFolder *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->get_msg_remove_strategy_func)
		g_critical ("You must implement tny_folder_get_msg_remove_strategy\n");
#endif
	return TNY_FOLDER_GET_IFACE (self)->get_msg_remove_strategy_func (self);
}

/**
 * tny_folder_set_msg_remove_strategy:
 * @self: a TnyFolder object
 * @st: a #TnyMsgRemoveStrategy object
 *
 * Set the strategy for removing a message
 *
 * Implementors: This method must set (store) the strategy for removing a
 * message.
 *
 * The idea is that devices can have a specific such strategy. For example a
 * strategy that removes it immediately from both local cache and a service
 * or a strategy that does nothing but flag the message for removal upon next
 * expunge or a strategy that does nothing.
 *
 * For more information take a look at tny_msg_remove_strategy_peform_remove
 * of #TnyMsgRemoveStrategy.
 *
 **/
void 
tny_folder_set_msg_remove_strategy (TnyFolder *self, TnyMsgRemoveStrategy *st)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->set_msg_remove_strategy_func)
		g_critical ("You must implement tny_folder_set_msg_remove_strategy\n");
#endif
	TNY_FOLDER_GET_IFACE (self)->set_msg_remove_strategy_func (self, st);
	return;
}

/**
 * tny_folder_expunge:
 * @self: a #TnyFolder object
 * @err: a #GError object or NULL
 *
 * Persist changes made to a folder to its backing store, expunging deleted 
 * messages (the ones marked with TNY_HEADER_FLAG_DELETED) as well.
 *
 * Example:
 * <informalexample><programlisting>
 * TnyHeader *header = ...
 * TnyFolder *folder = tny_header_get_folder (header);
 * tny_folder_remove_msg (folder, header, NULL);
 * tny_list_remove (TNY_LIST (mymodel), G_OBJECT (header));
 * g_object_unref (G_OBJECT (header));
 * tny_folder_expunge (folder);
 * g_object_unref (G_OBJECT (folder));
 * </programlisting></informalexample>
 **/
void 
tny_folder_expunge (TnyFolder *self, GError **err)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->expunge_func)
		g_critical ("You must implement tny_folder_expunge\n");
#endif
	TNY_FOLDER_GET_IFACE (self)->expunge_func (self, err);
	return;
}

/**
 * tny_folder_add_msg:
 * @self: a #TnyFolder object
 * @msg: a #TnyMsg object
 * @err: a #GError object or NULL
 *
 * Add a message to a folder.
 **/
void 
tny_folder_add_msg (TnyFolder *self, TnyMsg *msg, GError **err)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->add_msg_func)
		g_critical ("You must implement tny_folder_add_msg\n");
#endif

	TNY_FOLDER_GET_IFACE (self)->add_msg_func (self, msg, err);
	return;
}

/**
 * tny_folder_remove_msg:
 * @self: a TnyFolder object
 * @header: the header of the message to remove
 * @err: a #GError object or NULL
 *
 * Remove a message from a folder. It will use a #TnyMsgRemoveStrategy to 
 * perform the removal itself. For more details, check out the documentation
 * of the #TnyMsgRemoveStrategy type and the implementation that you activated
 * using tny_folder_set_msg_remove_strategy. The default implementation for
 * libtinymail-camel is the #TnyCamelMsgRemoveStrategy.
 *
 * Example:
 * <informalexample><programlisting>
 * static void
 * on_header_view_key_press_event (GtkTreeView *header_view, GdkEventKey *event, gpointer user_data)
 * {
 *   if (event->keyval == GDK_Delete)
 *   {
 *       GtkTreeSelection *selection;
 *       GtkTreeModel *model;
 *       GtkTreeIter iter;
 *       selection = gtk_tree_view_get_selection (header_view);
 *       if (gtk_tree_selection_get_selected (selection, &amp;model, &amp;iter))
 *       {
 *          TnyHeader *header;
 *          gtk_tree_model_get (model, &amp;iter, 
 *                TNY_GTK_HEADER_LIST_MODEL_INSTANCE_COLUMN, 
 *                &amp;header, -1);
 *          TnyFolder *folder = tny_header_get_folder (header);
 *          tny_folder_remove_msg (folder, header, NULL);
 *          tny_list_remove (TNY_LIST (model), G_OBJECT (header));
 *          g_object_unref (G_OBJECT (folder));
 *          g_object_unref (G_OBJECT (header));
 *       }
 *   }
 * }
 * </programlisting></informalexample>
 *
 **/
void 
tny_folder_remove_msg (TnyFolder *self, TnyHeader *header, GError **err)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->remove_msg_func)
		g_critical ("You must implement tny_folder_remove_msg\n");
#endif

	TNY_FOLDER_GET_IFACE (self)->remove_msg_func (self, header, err);
	return;
}


/**
 * tny_folder_refresh_async:
 * @self: a TnyFolder object
 * @callback: The callback handler
 * @status_callback: A callback for status notifications
 * @user_data: user data for the callback
 *
 * Refresh @self and call back when finished. This gets the summary information
 * from the E-Mail service, writes it to the the on-disk cache and/or updates
 * it.
 *
 * After this method, tny_folder_get_all_count and 
 * tny_folder_get_unread_count are guaranteed to be correct.
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
 * When using Gtk+, the callback doesn't need the gdk_threads_enter and 
 * gdk_threads_leave guards (because it happens in the #GMainLoop).
 *
 * Example:
 * <informalexample><programlisting>
 * static void
 * status_update_cb (TnyFolder *folder, const gchar *what, gint status, gpointer user_data)
 * {
 *     g_print (".");
 * }
 * static void
 * folder_refresh_cb (TnyFolder *folder, gboolean cancelled, GError **err, gpointer user_data)
 * {
 *     if (!cancelled)
 *     {
 *         TnyList *headers = tny_simple_list_new ();
 *         TnyIterator *iter;
 *         g_print ("done\nHeaders for %s are:", 
 *                tny_folder_get_name (folder));
 *         tny_folder_get_headers (folder, headers, FALSE);
 *         iter = tny_list_create_iterator (headers);
 *         while (!tny_iterator_is_done (iter))
 *         {
 *             TnyHeader *header = tny_iterator_current (iter);
 *             g_print ("\t%s\n", tny_header_get_subject (header));
 *             g_object_unref (G_OBJECT (header));
 *             tny_iterator_next (iter);
 *         }
 *         g_object_unref (G_OBJECT (headers));
 *     }
 * }
 * TnyFolder *folder = ...
 * g_print ("Getting headers ");
 * tny_folder_refresh_async (folder, 
 *          folder_refresh_cb, 
 *          status_update_cb, NULL); 
 * </programlisting></informalexample>
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
 * @err: a #GError object or NULL
 *
 * Refresh the folder. This gets the summary information from the E-Mail
 * service, writes it to the the on-disk cache and/or updates it.
 *
 * After this method, tny_folder_get_all_count and 
 * tny_folder_get_unread_count are guaranteed to be correct.
 *
 * Also read about tny_folder_get_headers.
 *
 **/
void
tny_folder_refresh (TnyFolder *self, GError **err)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->refresh_func)
		g_critical ("You must implement tny_folder_refresh\n");
#endif

	TNY_FOLDER_GET_IFACE (self)->refresh_func (self, err);
	return;
}


/**
 * tny_folder_is_subscribed:
 * @self: a TnyFolder object
 * 
 * Get the subscribtion status of this folder.
 * 
 * Return value: subscribe status
 **/
gboolean
tny_folder_is_subscribed (TnyFolder *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->is_subscribed_func)
		g_critical ("You must implement tny_folder_is_subscribed\n");
#endif

	return TNY_FOLDER_GET_IFACE (self)->is_subscribed_func (self);
}

/**
 * tny_folder_get_unread_count:
 * @self: a TnyFolder object
 * 
 * Get the amount of unread messages in this folder. The value is only
 * garuanteed to be correct after tny_folder_refresh or after the callback of
 * a tny_folder_refresh_async happened.
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
 * Get the amount of messages in this folder. The value is only garuanteed to be
 * correct after tny_folder_refresh or after the callback of a 
 * tny_folder_refresh_async happened.
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
 * Get a the parent account of this folder. You must unreference the return
 * value after use.
 * 
 * Return value: the account of this folder
 *
 **/
TnyAccount*  
tny_folder_get_account (TnyFolder *self)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->get_account_func)
		g_critical ("You must implement tny_folder_get_account\n");
#endif

	return TNY_FOLDER_GET_IFACE (self)->get_account_func (self);
}

/**
 * tny_folder_transfer_msgs:
 * @self: the TnyFolder where the headers are stored
 * @header_list: a list of TnyHeader objects
 * @folder_dst: the TnyFolder where the msgs will be transfered
 * @delete_originals: if TRUE then move msgs, else copy them
 * @err: a #GError object or NULL
 * 
 * Transfers messages of which the headers are in @header_list from @self to 
 * @folder_dst. They could be moved or just copied depending on the value of 
 * the @delete_originals argument
 *
 **/
void 
tny_folder_transfer_msgs (TnyFolder *self, TnyList *headers, TnyFolder *folder_dst, gboolean delete_originals, GError **err)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->transfer_msgs_func)
		g_critical ("You must implement tny_folder_transfer_msgs\n");
#endif

	return TNY_FOLDER_GET_IFACE (self)->transfer_msgs_func (self, headers, folder_dst, delete_originals, err);
}


/**
 * tny_folder_get_msg:
 * @self: a #TnyFolder object
 * @header: the header of the message to get
 * @err: a #GError object or NULL
 * 
 * Get a message in @self identified by @header. You must unreference the
 * return value after use.
 * 
 * Example:
 * <informalexample><programlisting>
 * TnyMsgView *message_view = tny_platform_factory_new_msg_view (platfact);
 * TnyFolder *folder = ...
 * TnyHeader *header = ...
 * TnyMsg *message = tny_folder_get_msg (folder, header, NULL);
 * tny_msg_view_set_msg (message_view, message);
 * g_object_unref (G_OBJECT (message));
 * </programlisting></informalexample>
 *
 * Return value: The message instance or NULL on failure
 *
 **/
TnyMsg*
tny_folder_get_msg (TnyFolder *self, TnyHeader *header, GError **err)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->get_msg_func)
		g_critical ("You must implement tny_folder_get_msg\n");
#endif

	return TNY_FOLDER_GET_IFACE (self)->get_msg_func (self, header, err);
}




/**
 * tny_folder_get_msg_async:
 * @self: a #TnyFolder object
 * @header: a #TnyHeader object
 * @callback: The callback handler
 * @user_data: user data for the callback
 *
 * Get a message in @self identified by @header. You must unreference the
 * return value after use.
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
 * When using Gtk+, the callback doesn't need the gdk_threads_enter and 
 * gdk_threads_leave guards (because it happens in the #GMainLoop).
 *
 * Example:
 * <informalexample><programlisting>
 * static void
 * folder_get_msg_cb (TnyFolder *folder, TnyMsg *msg, GError **err, gpointer user_data)
 * {
 *       TnyMsgView *message_view = user_data;
 *       tny_msg_view_set_msg (message_view, message);
 * }
 * TnyMsgView *message_view = tny_platform_factory_new_msg_view (platfact);
 * TnyFolder *folder = ...; TnyHeader *header = ...;
 * tny_folder_get_msg_async (folder, header,
 *          folder_get_msg_cb, message_view); 
 * </programlisting></informalexample>
 **/
void
tny_folder_get_msg_async (TnyFolder *self, TnyHeader *header, TnyGetMsgCallback callback, gpointer user_data)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->get_msg_async_func)
		g_critical ("You must implement tny_folder_get_msg_async\n");
#endif

	TNY_FOLDER_GET_IFACE (self)->get_msg_async_func (self, header, callback, user_data);
	return;
}

/**
 * tny_folder_get_headers:
 * @self: a TnyFolder object
 * @headers: A #TnyList instance where the headers will be put
 * @refresh: whether or not to synchronize with the service first
 * @err: a #GError object or NULL
 * 
 * Get the list of message header instances that are in @self. Also read
 * about tny_folder_refresh.
 *
 * Example:
 * <informalexample><programlisting>
 * TnyList *headers = tny_simple_list_new ();
 * TnyFolder *folder = ...;
 * TnyIterator *iter; 
 * tny_folder_get_headers (folder, headers, TRUE, NULL);
 * iter = tny_list_create_iterator (headers);
 * while (!tny_iterator_is_done (iter))
 * {
 *     TnyHeader *header = tny_iterator_get_current (iter);
 *     g_print ("%s\n", tny_header_get_subject (header));
 *     g_object_unref (G_OBJECT (header));
 *     tny_iterator_next (iter);
 * }
 * g_object_unref (G_OBJECT (iter));
 * g_object_unref (G_OBJECT (headers)); 
 * </programlisting></informalexample>
 **/
void
tny_folder_get_headers (TnyFolder *self, TnyList *headers, gboolean refresh, GError **err)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->get_headers_func)
		g_critical ("You must implement tny_folder_get_headers\n");
#endif

	return TNY_FOLDER_GET_IFACE (self)->get_headers_func (self, headers, refresh, err);
}

/**
 * tny_folder_get_id:
 * @self: a TnyFolder object
 * 
 * Get an unique id of @self (unique per account). The ID will be a "/" 
 * separated string like (but not guaranteed) "INBOX/parent-folder/folder"
 * depending on the service type (the example is for IMAP using the 
 * libtinymail-camel implementation).
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
 * Get the displayable name @of self. You should not free the result of 
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
 * @err: a #GError object or NULL
 * 
 * Rename a folder. Most services require the name to be unique in the 
 * parent folder.
 *
 **/
void
tny_folder_set_name (TnyFolder *self, const gchar *name, GError **err)
{
#ifdef DEBUG
	if (!TNY_FOLDER_GET_IFACE (self)->set_name_func)
		g_critical ("You must implement tny_folder_set_name\n");
#endif

	TNY_FOLDER_GET_IFACE (self)->set_name_func (self, name, err);
	return;
}

/**
 * tny_folder_get_folder_type:
 * @self: a TnyFolder object
 * 
 * Get the type of @self (Inbox, Outbox etc.) 
 * 
 * Return value: The folder type as a #TnyFolderType enum
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
	static gboolean tny_folder_initialized = FALSE;

	if (!tny_folder_initialized) 
		tny_folder_initialized = TRUE;
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
		  NULL,   /* instance_init */
		  NULL
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyFolder", &info, 0);

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

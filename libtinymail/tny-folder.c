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

#include <tny-folder-store.h>
#include <tny-header.h>
#include <tny-list.h>
#include <tny-folder-stats.h>
#include <tny-msg-receive-strategy.h>
#include <tny-folder-observer.h>

#include <tny-folder.h>
guint tny_folder_signals [TNY_FOLDER_LAST_SIGNAL];

/* Possible future API changes:
 * tny_folder_get_msg_async will get a status callback handler. Also take a look
 * at the possible API changes for TnyMsgReceiveStrategy as this would affect
 * that API too. 
 *
 * tny_folder_find_msg for finding a message using an url_string, maybe also a 
 * tny_folder_set_find_msg_strategy and a tny_folder_get_find_msg_strategy if
 * in future alternative ways to find a message are to be specified and 
 * developed */

/**
 * tny_folder_get_caps:
 * @self: a #TnyFolder object
 * 
 * Get the capabilities of @self.
 * 
 * Return value: The capabilities of the folder
 **/

TnyFolderCaps 
tny_folder_get_caps (TnyFolder *self)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (TNY_FOLDER_GET_IFACE (self)->get_caps_func != NULL);
#endif

	return TNY_FOLDER_GET_IFACE (self)->get_caps_func (self);
}

/**
 * tny_folder_get_url_string:
 * @self: a #TnyFolder object
 * 
 * Get the url_string @self or NULL if it's impossible to determine the url 
 * string of @self. If not NULL, the returned value must be freed after use.
 *
 * The url string is specified in RFC 1808 and looks for example like this:
 * imap://user@hostname/INBOX/folder. Note that it doesn't necessarily contain 
 * the password of the IMAP account.
 * 
 * Return value: The url string or NULL.
 **/
gchar* 
tny_folder_get_url_string (TnyFolder *self)
{
	gchar *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (TNY_FOLDER_GET_IFACE (self)->get_url_string_func != NULL);
#endif

	retval = TNY_FOLDER_GET_IFACE (self)->get_url_string_func (self);

#ifdef DBC /* ensure */
	if (retval) {
		g_assert (strlen (retval) > 0);
		g_assert (strstr (retval, "://") != NULL);
	}
#endif

	return retval;
}

/**
 * tny_folder_get_stats:
 * @self: a TnyFolder object
 *
 * Get some statistics of the folder @self. You must unreference the return 
 * value after use.
 *
 * Return value: some stats of the folder
 **/
TnyFolderStats* 
tny_folder_get_stats (TnyFolder *self)
{
	TnyFolderStats *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (TNY_FOLDER_GET_IFACE (self)->get_stats_func != NULL);
#endif

	retval = TNY_FOLDER_GET_IFACE (self)->get_stats_func (self);

#ifdef DBC /* ensure */
	g_assert (TNY_IS_FOLDER_STATS (retval));
#endif

	return retval;
}

/**
 * tny_folder_add_observer:
 * @self: a #TnyFolder instance
 * @observer: a #TnyFolderObserver instance
 *
 * Add @observer to the list of interested observers for the 
 * event that could happen caused by a tny_folder_poke_recent_changes or
 * other urgent changes.
 *
 **/
void 
tny_folder_add_observer (TnyFolder *self, TnyFolderObserver *observer)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (observer);
	g_assert (TNY_IS_FOLDER_OBSERVER (observer));
	g_assert (TNY_FOLDER_GET_IFACE (self)->add_observer_func != NULL);
#endif

	TNY_FOLDER_GET_IFACE (self)->add_observer_func (self, observer);
	return;
}


/**
 * tny_folder_remove_observer:
 * @self: a #TnyFolder instance
 * @observer: a #TnyFolderObserver instance
 *
 * Remove @observer from the list of interested observers for the 
 * event that could happen caused by a tny_folder_poke_recent_changes or
 * other urgent changes.
 *
 **/
void 
tny_folder_remove_observer (TnyFolder *self, TnyFolderObserver *observer)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (observer);
	g_assert (TNY_IS_FOLDER_OBSERVER (observer));
	g_assert (TNY_FOLDER_GET_IFACE (self)->remove_observer_func != NULL);
#endif

	TNY_FOLDER_GET_IFACE (self)->remove_observer_func (self, observer);
	return;

}

/**
 * tny_folder_poke_status:
 * @self: a TnyFolder object
 *
 * Poke for the status, this might ignite the event that a !TnyFolderChange is 
 * to be sent to the observers (tny_folder_add_observer and !TnyFolderObserver).
 *
 * This functionality is to be used for E-mail services that don't do what for
 * example the Push-IMAP (P-IMAP) protocol promises (the pushing of changes, as
 * described in draft-maes-lemonade-p-imap-12).
 **/
void 
tny_folder_poke_status (TnyFolder *self)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (TNY_FOLDER_GET_IFACE (self)->poke_status_func != NULL);
#endif

	TNY_FOLDER_GET_IFACE (self)->poke_status_func (self);
	return;
}


/**
 * tny_folder_get_msg_receive_strategy:
 * @self: a TnyFolder object
 *
 * Get the strategy for receiving a message. The return value of this method
 * must be unreferenced after use.
 *
 * Implementors: This method must return the strategy for receiving a message.
 * being the implementer, you must add a reference before returning the instance.
 *
 * Return value: the strategy for receiving a message
 **/
TnyMsgReceiveStrategy* 
tny_folder_get_msg_receive_strategy (TnyFolder *self)
{
	TnyMsgReceiveStrategy *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (TNY_FOLDER_GET_IFACE (self)->get_msg_receive_strategy_func != NULL);
#endif

	retval = TNY_FOLDER_GET_IFACE (self)->get_msg_receive_strategy_func (self);

#ifdef DBC /* ensure */
	g_assert (TNY_IS_MSG_RECEIVE_STRATEGY (retval));
#endif

	return retval;
}

/**
 * tny_folder_set_msg_receive_strategy:
 * @self: a TnyFolder object
 * @st: a #TnyMsgReceiveStrategy object
 *
 * Set the strategy for receiving a message
 *
 * Implementors: This method must set (store) the strategy for receiving a
 * message.
 *
 * The idea is that devices can have a specific such strategy. For example a
 * strategy that receives the message fully from the service or a strategy that
 * receives it partially from the service (only the body)
 *
 * For more information take a look at tny_msg_receive_strategy_peform_get_msg
 * of #TnyMsgReceiveStrategy.
 *
 **/
void 
tny_folder_set_msg_receive_strategy (TnyFolder *self, TnyMsgReceiveStrategy *st)
{
#ifdef DBC /* require */
	TnyMsgReceiveStrategy *test;
	g_assert (TNY_IS_FOLDER (self));
	g_assert (st);
	g_assert (TNY_IS_MSG_RECEIVE_STRATEGY (st));
	g_assert (TNY_FOLDER_GET_IFACE (self)->set_msg_receive_strategy_func != NULL);
#endif

	TNY_FOLDER_GET_IFACE (self)->set_msg_receive_strategy_func (self, st);

#ifdef DBC /* ensure */
	test = tny_folder_get_msg_receive_strategy (self);
	g_assert (test);
	g_assert (TNY_IS_MSG_RECEIVE_STRATEGY (test));
	g_assert (test == st);
	g_object_unref (G_OBJECT (test));
#endif

	return;
}



/**
 * tny_folder_copy:
 * @self: a #TnyFolder object
 * @into: a #TnyFolderStore object
 * @new_name: the new name in @into
 * @del: whether or not to delete the original location
 * @err: a #GError object or NULL
 *
 * Copies @self to @into giving the new folder the name @new_name. Returns the
 * newly created folder in @into, which will carry the name @new_name.
 *
 * Implementors: The return value must be the new folder in @into carrying the 
 * name @new_name. Invoking the tny_folder_get_folder_store API on the return 
 * value must return the @into instance. The implementation must copy all 
 * messages from @self to @into. In case @del was TRUE, the messages that got
 * successfully copied must be removed from @self. If @new_name already exists
 * in @into and @err is not NULL, then an error must be set in @err and no 
 * further action may be performed.
 * 
 * Return value: a new folder instance to whom was copied
 **/
TnyFolder* 
tny_folder_copy (TnyFolder *self, TnyFolderStore *into, const gchar *new_name, gboolean del, GError **err)
{
	TnyFolder *retval;

#ifdef DBC /* require */
	TnyFolderStore *test;
	g_assert (TNY_IS_FOLDER (self));
	g_assert (TNY_IS_FOLDER_STORE (into));
	g_assert (new_name);
	g_assert (strlen (new_name) > 0);
	g_assert (TNY_FOLDER_GET_IFACE (self)->copy_func != NULL);
#endif

	retval = TNY_FOLDER_GET_IFACE (self)->copy_func (self, into, new_name, del, err);

#ifdef DBC /* ensure */
	g_assert (retval);
	g_assert (!strcmp (tny_folder_get_name (retval), new_name));
	test = tny_folder_get_folder_store (retval);
	g_assert (test);
	g_assert (TNY_IS_FOLDER_STORE (test));
	g_assert (test == into);
	g_object_unref (G_OBJECT (test));
#endif

	return retval;
}

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
	TnyMsgRemoveStrategy *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (TNY_FOLDER_GET_IFACE (self)->get_msg_remove_strategy_func != NULL);
#endif

	retval = TNY_FOLDER_GET_IFACE (self)->get_msg_remove_strategy_func (self);

#ifdef DBC /* ensure */
	g_assert (TNY_IS_MSG_REMOVE_STRATEGY (retval));
#endif

	return retval;
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
#ifdef DBC /* require */
	TnyMsgRemoveStrategy *test;
	g_assert (TNY_IS_FOLDER (self));
	g_assert (st);
	g_assert (TNY_IS_MSG_REMOVE_STRATEGY (st));
	g_assert (TNY_FOLDER_GET_IFACE (self)->set_msg_remove_strategy_func != NULL);
#endif

	TNY_FOLDER_GET_IFACE (self)->set_msg_remove_strategy_func (self, st);

#ifdef DBC /* ensure */
	test = tny_folder_get_msg_remove_strategy (self);
	g_assert (test);
	g_assert (TNY_IS_MSG_REMOVE_STRATEGY (test));
	g_assert (test == st);
	g_object_unref (G_OBJECT (test));
#endif

	return;
}

/**
 * tny_folder_sync:
 * @self: a #TnyFolder object
 * @expunge: whether or not to also expunge deleted messages
 * @err: a #GError object or NULL
 *
 * Persist changes made to a folder to its backing store, expunging deleted 
 * messages (the ones marked with TNY_HEADER_FLAG_DELETED) as well if @expunge
 * is TRUE.
 *
 * Example:
 * <informalexample><programlisting>
 * TnyHeader *header = ...
 * TnyFolder *folder = tny_header_get_folder (header);
 * tny_folder_remove_msg (folder, header, NULL);
 * tny_list_remove (TNY_LIST (mymodel), G_OBJECT (header));
 * g_object_unref (G_OBJECT (header));
 * tny_folder_sync (folder, TRUE, NULL);
 * g_object_unref (G_OBJECT (folder));
 * </programlisting></informalexample>
 **/
void 
tny_folder_sync (TnyFolder *self, gboolean expunge, GError **err)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (TNY_FOLDER_GET_IFACE (self)->sync_func != NULL);
#endif

	TNY_FOLDER_GET_IFACE (self)->sync_func (self, expunge, err);
	return;
}

/**
 * tny_folder_add_msg:
 * @self: a #TnyFolder object
 * @msg: a #TnyMsg object
 * @err: a #GError object or NULL
 *
 * Add a message to a folder. It's recommended to destroy @msg afterwards as 
 * after receiving the message from the folder again, the instance wont be the
 * same anymore and property like the tny_msg_get_id might have changed and
 * assigned too.
 * 
 * Folder observers of @self will get a header-added trigger caused by this
 * action.
 **/
void 
tny_folder_add_msg (TnyFolder *self, TnyMsg *msg, GError **err)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (msg);
	g_assert (TNY_IS_MSG (msg));
	g_assert (TNY_FOLDER_GET_IFACE (self)->add_msg_func != NULL);
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
 * Folder observers of @self will get a header-removed trigger caused by this
 * action.
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
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (header);
	g_assert (TNY_IS_HEADER (header));
	g_assert (TNY_FOLDER_GET_IFACE (self)->remove_msg_func != NULL);
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
 * status_update_cb (TnyFolder *folder, const gchar *what, gint sofar, gint oftotal, gpointer user_data)
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
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (callback);
	g_assert (status_callback);
	g_assert (TNY_FOLDER_GET_IFACE (self)->refresh_async_func != NULL);
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
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (TNY_FOLDER_GET_IFACE (self)->refresh_func != NULL);
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
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (TNY_FOLDER_GET_IFACE (self)->is_subscribed_func != NULL);
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
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (TNY_FOLDER_GET_IFACE (self)->get_unread_count_func != NULL);
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
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (TNY_FOLDER_GET_IFACE (self)->get_all_count_func != NULL);
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
	TnyAccount *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (TNY_FOLDER_GET_IFACE (self)->get_account_func != NULL);
#endif

	retval = TNY_FOLDER_GET_IFACE (self)->get_account_func (self);


#ifdef DBC /* ensure */
	if (retval)
		g_assert (TNY_IS_ACCOUNT (retval));
#endif

	return retval;
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
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (headers);
	g_assert (TNY_IS_LIST (headers));
	g_assert (folder_dst);
	g_assert  (TNY_IS_FOLDER (folder_dst));
	g_assert (TNY_FOLDER_GET_IFACE (self)->transfer_msgs_func != NULL);
#endif

	TNY_FOLDER_GET_IFACE (self)->transfer_msgs_func (self, headers, folder_dst, delete_originals, err);
	return;
}

/**
 * tny_folder_transfer_msgs_async:
 * @self: the TnyFolder where the headers are stored
 * @header_list: a list of TnyHeader objects
 * @folder_dst: the TnyFolder where the msgs will be transfered
 * @delete_originals: if TRUE then move msgs, else copy them
 * @callback: The callback handler
 * @user_data: user data for the callback
 * 
 * Transfers messages of which the headers are in @header_list from @self to 
 * @folder_dst. They could be moved or just copied depending on the value of 
 * the @delete_originals argument
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
 **/
void 
tny_folder_transfer_msgs_async (TnyFolder *self, TnyList *header_list, TnyFolder *folder_dst, gboolean delete_originals, TnyTransferMsgsCallback callback, gpointer user_data)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (header_list);
	g_assert (TNY_IS_LIST (header_list));
	g_assert (folder_dst);
	g_assert (TNY_IS_FOLDER (folder_dst));
	g_assert (callback);
	g_assert (TNY_FOLDER_GET_IFACE (self)->transfer_msgs_async_func != NULL);
#endif

	TNY_FOLDER_GET_IFACE (self)->transfer_msgs_async_func (self, header_list, folder_dst, delete_originals, callback, user_data);
	return;
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
	TnyMsg *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (header);
	g_assert (TNY_IS_HEADER (header));
	g_assert (TNY_FOLDER_GET_IFACE (self)->get_msg_func != NULL);
#endif

	retval = TNY_FOLDER_GET_IFACE (self)->get_msg_func (self, header, err);

#ifdef DBC /* ensure */
	if (retval)
		g_assert (TNY_IS_MSG (retval));
#endif

	return retval;
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
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (header);
	g_assert (TNY_IS_HEADER (header));
	g_assert (callback);
	g_assert (TNY_FOLDER_GET_IFACE (self)->get_msg_async_func != NULL);
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
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (headers);
	g_assert (TNY_IS_LIST (headers));
	g_assert (TNY_FOLDER_GET_IFACE (self)->get_headers_func != NULL);
#endif

	TNY_FOLDER_GET_IFACE (self)->get_headers_func (self, headers, refresh, err);
	return;
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
	const gchar *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (TNY_FOLDER_GET_IFACE (self)->get_id_func != NULL);
#endif

	retval = TNY_FOLDER_GET_IFACE (self)->get_id_func (self);

#ifdef DBC /* ensure */
	g_assert (retval);
	g_assert (strlen (retval) > 0);
#endif

	return retval;
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
	const gchar *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (TNY_FOLDER_GET_IFACE (self)->get_name_func != NULL);
#endif

	retval = TNY_FOLDER_GET_IFACE (self)->get_name_func (self);

#ifdef DBC /* ensure */
	g_assert (retval);
	g_assert (strlen (retval) > 0);
#endif

	return retval;
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
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (name);
	g_assert (strlen (name) > 0);
	g_assert (TNY_FOLDER_GET_IFACE (self)->set_name_func != NULL);
#endif

	TNY_FOLDER_GET_IFACE (self)->set_name_func (self, name, err);

#ifdef DBC /* ensure */
	g_assert (!strcmp (tny_folder_get_name (self), name));
#endif

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
#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (TNY_FOLDER_GET_IFACE (self)->get_folder_type_func != NULL);
#endif

	return TNY_FOLDER_GET_IFACE (self)->get_folder_type_func (self);
}


/**
 * tny_folder_get_folder_store:
 * @self: a TnyFolder object
 * 
 * Get a the parent account of this folder. You must unreference the
 * return value after use. Note that not every folder strictly has to
 * be inside a folder store.
 * 
 * Return value: the folder store of this folder, or NULL if the
 * folder is not inside a folder store
 *
 **/
TnyFolderStore* 
tny_folder_get_folder_store (TnyFolder *self)
{
	TnyFolderStore* retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_FOLDER (self));
	g_assert (TNY_FOLDER_GET_IFACE (self)->get_folder_store_func != NULL);
#endif

	retval = TNY_FOLDER_GET_IFACE (self)->get_folder_store_func (self);

#ifdef DBC /* require */
	if (retval)
		g_assert (TNY_IS_FOLDER_STORE (retval));
#endif

	return retval;
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

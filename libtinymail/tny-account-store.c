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

#include <tny-account.h>
#include <tny-transport-account.h>
#include <tny-store-account.h>
#include <tny-device.h>
#include <tny-list.h>

#include <tny-account-store.h>
guint tny_account_store_signals [TNY_ACCOUNT_STORE_LAST_SIGNAL];


/**
 * tny_account_store_find_account:
 * @self: a #TnyAccountStore object
 * @url_string: the url-string of the account to find
 *
 * Try to find the account in @self that corresponds to @url_string. If this 
 * method does not return NULL, the returned value is the found account and
 * must be unreferenced after use.
 *
 * Implementors: when implementing a platform-specific library, you must
 * implement this method. Let it return the account that corresponds to
 * @url_string or let it return NULL. Also see tny_account_matches_url_string
 * at #TnyAccount.
 *
 * This method can be used to resolve url-strings to #TnyAccount instances.
 *
 * Return value: the found account or NULL.
 **/
TnyAccount* 
tny_account_store_find_account (TnyAccountStore *self, const gchar *url_string)
{
	TnyAccount *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT_STORE (self));
	g_assert (url_string);
	g_assert (strlen (url_string) > 0);
	g_assert (strstr (url_string, "://"));
	g_assert (TNY_ACCOUNT_STORE_GET_IFACE (self)->find_account_func != NULL);
#endif

	retval = TNY_ACCOUNT_STORE_GET_IFACE (self)->find_account_func (self, url_string);

#ifdef DBC /* ensure */
	if (retval) {
		g_assert (TNY_IS_ACCOUNT (retval));
		g_assert (tny_account_matches_url_string (retval, url_string));
	}
#endif

	return retval;
}

/**
 * tny_account_store_alert:
 * @self: a #TnyAccountStore object
 * @type: the message type (severity)
 * @prompt: the prompt
 *
 * This jump-to-the-ui method implements showing a message dialog with @prompt
 * as prompt and @type as message type. It will return TRUE if the reply was 
 * affirmative or FALSE if not.
 *
 * Implementors: when implementing a platform-specific library, you must
 * implement this method. For example in Gtk+ by using the #GtkDialog API. The
 * implementation will for example be used to ask the user about accepting SSL
 * certificates. The two possible answers that must be supported are 
 * "Yes" and "No" which must result in a TRUE or a FALSE return value.
 *
 * Example implementation for Gtk+:
 * <informalexample><programlisting>
 * static gboolean
 * tny_gnome_account_store_alert (TnyAccountStore *self, TnyAlertType type, const gchar *prompt)
 * {
 *     GtkMessageType gtktype;
 *     gboolean retval = FALSE;
 *     GtkWidget *dialog;
 *     switch (type)
 *     {
 *         case TNY_ALERT_TYPE_INFO:
 *         gtktype = GTK_MESSAGE_INFO;
 *         break;
 *         case TNY_ALERT_TYPE_WARNING:
 *         gtktype = GTK_MESSAGE_WARNING;
 *         break;
 *         case TNY_ALERT_TYPE_ERROR:
 *         default:
 *         gtktype = GTK_MESSAGE_ERROR;
 *         break;
 *     }
 *     dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
 *            gtktype, GTK_BUTTONS_YES_NO, prompt);
 *     if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_YES)
 *            retval = TRUE;
 *     gtk_widget_destroy (dialog);
 *     return retval;
 * }
 * </programlisting></informalexample>
 *
 * Return value: Whether the user pressed Ok/Yes (TRUE) or Cancel/No (FALSE)
 *
 **/
gboolean 
tny_account_store_alert (TnyAccountStore *self, TnyAlertType type, const gchar *prompt)
{
	gboolean retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT_STORE (self));
	g_assert (prompt);
	g_assert (strlen (prompt) > 0);
	g_assert (TNY_ACCOUNT_STORE_GET_IFACE (self)->alert_func != NULL);
#endif

	retval = TNY_ACCOUNT_STORE_GET_IFACE (self)->alert_func (self, type, prompt);

#ifdef DBC /* ensure */
#endif

	return retval;
}

/**
 * tny_account_store_get_device:
 * @self: a #TnyAccountStore object
 *
 * This method returns a #TnyDevice instance. You must unreference the return
 * value after use.
 *
 * Implementors: when implementing a platform-specific library, you must
 * implement this method by letting it return a #TnyDevice instance. You must
 * add a reference count before returning.
 *
 * Return value: the device attached to this account store
 *
 **/
TnyDevice* 
tny_account_store_get_device (TnyAccountStore *self)
{
	TnyDevice *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT_STORE (self));
	g_assert (TNY_ACCOUNT_STORE_GET_IFACE (self)->get_device_func != NULL);
#endif

	retval = TNY_ACCOUNT_STORE_GET_IFACE (self)->get_device_func (self);

#ifdef DBC /* ensure */
	g_assert (retval);
	g_assert (TNY_IS_DEVICE (retval));
#endif

	return retval;
}

/**
 * tny_account_store_get_cache_dir:
 * @self: a #TnyAccountStore object
 * 
 * Get the local path that will be used for storing the disk cache
 *
 * Implementors: when implementing a platform-specific library, you must
 * implement this method. You can for example let it return the path to some
 * folder in $HOME on the file system.
 *
 * Note that the callers of this method will not free the result. The
 * implementor of a #TnyAccountStore is responsible for freeing it up. For
 * example when destroying @self (in its finalize method).
 *
 * Return value: the local path that will be used for storing the disk cache
 *
 **/
const gchar*
tny_account_store_get_cache_dir (TnyAccountStore *self)
{
	const gchar *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT_STORE (self));
	g_assert (TNY_ACCOUNT_STORE_GET_IFACE (self)->get_cache_dir_func != NULL);
#endif

	retval = TNY_ACCOUNT_STORE_GET_IFACE (self)->get_cache_dir_func (self);

#ifdef DBC /* ensure */
	g_assert (retval);
	g_assert (strlen (retval) > 0);
#endif

	return retval;
}


/**
 * tny_account_store_get_accounts:
 * @self: a #TnyAccountStore object
 * @list: a #TnyList instance that will be filled with #TnyAccount instances
 * @types: a #TnyGetAccountsRequestType that describes which account types are needed
 * 
 * Get a read-only list of accounts in the store. You must not change @list 
 * except for referencing and unreferencing.
 *
 * Example:
 * <informalexample><programlisting>
 * TnyList *list = tny_simple_list_new ();
 * tny_account_store_get_accounts (astore, list, TNY_ACCOUNT_STORE_STORE_ACCOUNTS);
 * TnyIterator *iter = tny_list_create_iterator (list);
 * while (!tny_iterator_is_done (iter))
 * {
 *    TnyStoreAccount *cur = TNY_STORE_ACCOUNT (tny_iterator_get_current (iter));
 *    printf ("%s\n", tny_store_account_get_name (cur));
 *    g_object_unref (G_OBJECT (cur));
 *    tny_iterator_next (iter);
 * }
 * g_object_unref (G_OBJECT (iter));
 * g_object_unref (G_OBJECT (list));
 * </programlisting></informalexample>
 *
 * Implementors: when implementing a platform-specific library, you must 
 * implement this method.
 *
 * It is allowed to cache the list (but not required). If you are implementing
 * an account store for account implementations from libtinymail-camel, you must
 * register the created accounts with a #TnySessionCamel instance using the 
 * libtinymail-camel specific tny_session_camel_set_current_accounts API.
 *
 * The implementation must fillup @list with available accounts. Note that if
 * you cache the list, you must add a reference to each account added to the
 * list (else they will be unreferenced and if the reference count would reach
 * zero, an account would no longer be correctly cached).
 *
 * With libtinymail-camel each created account must also be informed about the
 * #TnySessionCamel instance being used. Read more about this at
 * tny_camel_account_set_session of the libtinymail-camel specific 
 * #TnyCamelAccount.
 *
 * There are multiple samples of #TnyAccountStore implementations in
 * libtinymail-gnome-desktop, libtinymail-olpc, libtinymail-gpe, 
 * libtinymail-maemo and tests/shared/account-store.c which is being used by
 * the unit tests and the normal tests.
 *
 * The get_pass and forget_pass functionality of the example below is usually
 * implemented by utilizing a #TnyPasswordGetter that is returned by the 
 * #TnyPlatformFactory, that is usually available from the #TnyAccountStore. 
 *
 * Example (that uses a cache):
 * <informalexample><programlisting>
 * static TnyCamelSession *session = NULL;
 * static TnyList *accounts = NULL;
 * static gchar* 
 * account_get_pass_func (TnyAccount *account, const gchar *prompt, gboolean *cancel)
 * {
 *      return g_strdup ("the password");
 * }
 * static void
 * account_forget_pass_func (TnyAccount *account)
 * {
 *      g_print ("Password was incorrect\n");
 * }
 * static void
 * tny_my_account_store_get_accounts (TnyAccountStore *self, TnyList *list, TnyGetAccountsRequestType types)
 * {
 *    TnyIterator *iter;
 *    if (session == NULL)
 *        session = tny_session_camel_new (TNY_ACCOUNT_STORE (self));
 *    if (accounts == NULL)
 *    {
 *        accounts = tny_simple_list_new ();
 *        for (... each account ... )
 *        {
 *           TnyAccount *account = TNY_ACCOUNT (tny_camel_store_account_new ());
 *           tny_camel_account_set_session (TNY_CAMEL_ACCOUNT (account), session);
 *           tny_account_set_proto (account, "imap");
 *           tny_account_set_name (account, "account i");
 *           tny_account_set_user (account, "user of account i");
 *           tny_account_set_hostname (account, "server.domain of account i");
 *           tny_account_set_id (account, "i");
 *           tny_account_set_forget_pass_func (account, account_forget_pass_func);
 *           tny_account_set_pass_func (account, account_get_pass_func);
 *           tny_list_prepend (accounts, account);
 *           g_object_unref (G_OBJECT (account));
 *        }
 *    }
 *    iter = tny_list_create_iterator (accounts);
 *    while (tny_iterator_is_done (iter))
 *    {
 *        GObject *cur = tny_iterator_get_current (iter);
 *        tny_list_prepend (list, cur);
 *        g_object_unref (cur);
 *        tny_iterator_next (iter);
 *    }
 *    g_object_unref (G_OBJECT (iter));
 * }
 * </programlisting></informalexample>
 **/
void
tny_account_store_get_accounts (TnyAccountStore *self, TnyList *list, TnyGetAccountsRequestType types)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT_STORE (self));
	g_assert (list);
	g_assert (TNY_IS_LIST (list));
	g_assert (TNY_ACCOUNT_STORE_GET_IFACE (self)->get_accounts_func != NULL);
#endif

	TNY_ACCOUNT_STORE_GET_IFACE (self)->get_accounts_func (self, list, types);

#ifdef DBC /* ensure */
#endif

	return;
}


static void
tny_account_store_base_init (gpointer g_class)
{
	static gboolean tny_account_store_initialized = FALSE;

	if (!tny_account_store_initialized) 
	{
/**
 * TnyAccountStore::account-changed
 * @self: the object on which the signal is emitted
 * @arg1: the #TnyAccount of the account that changed
 * @user_data: user data set when the signal handler was connected
 *
 * API WARNING: This API might change
 *
 * Emitted when an account in the store changed
 */
		tny_account_store_signals[TNY_ACCOUNT_STORE_ACCOUNT_CHANGED] =
		   g_signal_new ("account_changed",
			TNY_TYPE_ACCOUNT_STORE,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnyAccountStoreIface, account_changed),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE, 1, TNY_TYPE_ACCOUNT);

/**
 * TnyAccountStore::account-inserted
 * @self: the object on which the signal is emitted
 * @arg1: the #TnyAccount of the account that got inserted
 * @user_data: user data set when the signal handler was connected.
 *
 * API WARNING: This API might change
 *
 * Emitted when an account is added to the store
 */
		tny_account_store_signals[TNY_ACCOUNT_STORE_ACCOUNT_INSERTED] =
		   g_signal_new ("account_inserted",
			TNY_TYPE_ACCOUNT_STORE,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnyAccountStoreIface, account_inserted),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE, 1, TNY_TYPE_ACCOUNT);

/**
 * TnyAccountStore::account-removed
 * @self: the object on which the signal is emitted
 * @arg1: the #TnyAccount of the account that got removed
 * @user_data: user data set when the signal handler was connected.
 *
 * API WARNING: This API might change
 *
 * Emitted when an account is removed from the store
 */
		tny_account_store_signals[TNY_ACCOUNT_STORE_ACCOUNT_REMOVED] =
		   g_signal_new ("account_removed",
			TNY_TYPE_ACCOUNT_STORE,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnyAccountStoreIface, account_removed),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE, 1, TNY_TYPE_ACCOUNT);

/**
 * TnyAccountStore::accounts-reloaded
 * @self: the object on which the signal is emitted
 *
 * API WARNING: This API might change
 *
 * Emitted when the store reloads the accounts
 */
		tny_account_store_signals[TNY_ACCOUNT_STORE_ACCOUNTS_RELOADED] =
		   g_signal_new ("accounts_reloaded",
			TNY_TYPE_ACCOUNT_STORE,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnyAccountStoreIface, accounts_reloaded),
			NULL, NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE, 0);

		tny_account_store_initialized = TRUE;
	}
	return;
}


/**
 * tny_account_store_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
GType
tny_account_store_get_type (void)
{
	static GType type = 0;
	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyAccountStoreIface),
		  tny_account_store_base_init,   /* base_init */
		  NULL,   /*    base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL,    /* instance_init */
		  NULL
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyAccountStore", &info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);

	}
	return type;
}

/**
 * tny_alert_type_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
GType
tny_alert_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TNY_ALERT_TYPE_INFO, "TNY_ALERT_TYPE_INFO", "info" },
      { TNY_ALERT_TYPE_WARNING, "TNY_ALERT_TYPE_WARNING", "warning" },
      { TNY_ALERT_TYPE_ERROR, "TNY_ALERT_TYPE_ERROR", "error" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyAlertType", values);
  }
  return etype;
}

/**
 * tny_get_accounts_request_type_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
GType
tny_get_accounts_request_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TNY_ACCOUNT_STORE_TRANSPORT_ACCOUNTS, "TNY_ACCOUNT_STORE_TRANSPORT_ACCOUNTS", "transport" },
      { TNY_ACCOUNT_STORE_STORE_ACCOUNTS, "TNY_ACCOUNT_STORE_STORE_ACCOUNTS", "store" },
      { TNY_ACCOUNT_STORE_BOTH, "TNY_ACCOUNT_STORE_BOTH", "both" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyGetAccountsRequestType", values);
  }
  return etype;
}


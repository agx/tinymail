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

#include <tny-account.h>

#ifndef TNY_ACCOUNT_STORE_C
#define TNY_ACCOUNT_STORE_C
#endif

#include <tny-account-store.h>

#ifdef TNY_ACCOUNT_STORE_C
#undef TNY_ACCOUNT_STORE_C
#endif

guint *tny_account_store_signals = NULL;


/**
 * tny_account_store_alert:
 * @self: a #TnyAccountTransport object
 * @type: the message type (severity)
 * @prompt: the prompt
 *
 * This jump-to-the-ui method implements showing a message dialog with prompt
 * as prompt. It will return TRUE if the reply was affirmative. Or FALSE if not.
 *
 * Implementors: when implementing a platform-specific library, you must
 * implement this method. For example in Gtk+ by using the GtkDialog API.
 *
 * Return value: Whether the user pressed Ok/Yes (TRUE) or Cancel/No (FALSE)
 *
 **/
gboolean 
tny_account_store_alert (TnyAccountStore *self, TnyAlertType type, const gchar *prompt)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_STORE_GET_IFACE (self)->alert_func)
		g_critical ("You must implement tny_account_store_alert\n");
#endif
	return TNY_ACCOUNT_STORE_GET_IFACE (self)->alert_func (self, type, prompt);
}

/**
 * tny_account_store_get_device:
 * @self: a #TnyAccountTransport object
 *
 * This method returns a #TnyDevice instance
 *
 * Implementors: when implementing a platform-specific library, you must
 * implement this method and a #TnyDevice implementation.
 *
 * Return value: the device attached to this account store
 *
 **/
TnyDevice* 
tny_account_store_get_device (TnyAccountStore *self)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_STORE_GET_IFACE (self)->get_device_func)
		g_critical ("You must implement tny_account_store_get_device\n");
#endif
	return TNY_ACCOUNT_STORE_GET_IFACE (self)->get_device_func (self);
}

/**
 * tny_account_store_get_cache_dir:
 * @self: a #TnyAccountTransport object
 * 
 * Get the local path that will be used for storing the disk cache
 *
 * Implementors: when implementing a platform-specific library, you must
 * implement this method. You can for example let it return the path to some
 * folder on the file system.
 *
 * Note that the callers of this method will not free the result. You
 * are responsible of freeing it up. For example when destroying the 
 * #TnyAccountStore implementation instance.
 * 
 * Return value: the local path that will be used for storing the disk cache
 *
 **/
const gchar*
tny_account_store_get_cache_dir (TnyAccountStore *self)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_STORE_GET_IFACE (self)->get_cache_dir_func)
		g_critical ("You must implement tny_account_store_get_cache_dir\n");
#endif
	return TNY_ACCOUNT_STORE_GET_IFACE (self)->get_cache_dir_func (self);
}


/**
 * tny_account_store_get_accounts:
 * @self: a #TnyAccountTransport object
 * @list: a #TnyList instance that will be filled with #TnyAccount instances
 * @types: a #TnyGetAccountsRequestType that describes which account types are needed
 * 
 * Get a read-only list of accounts in the store
 *
 * Implementors: when implementing a platform-specific library, you must 
 * implement this method.
 * 
 * It is allowed to cache the list (but not required). If you are implementing
 * an account store for account implementations from libtinymail-camel, you must
 * register the created accounts with a #TnySessionCamel instance using the 
 * libtinymail-camel specific tny_session_camel_set_current_accounts API.
 *
 * The implementation must fillup @list with instances to the available accounts.
 * Note that if you cache the list, you must add a reference to each account
 * added to the list (they will be unreferenced and if the reference count
 * would reaches zero, an account would no longer be cached).
 *
 * With libtinymail-camel each created account must also be informed about the
 * #TnySessionCamel instance being used. Read more about this at
 * tny_account_set_session of the libtinymail-camel specific #TnyAccount.
 *
 * There are multiple samples of #TnyAccountStore implementations in
 * libtinymail-gnome-desktop, libtinymail-olpc, libtinymail-gpe, 
 * libtinymail-maemo and tests/shared/account-store.c which is being used by
 * the unit tests and the normal tests.
 *
 **/
void
tny_account_store_get_accounts (TnyAccountStore *self, TnyList *list, TnyGetAccountsRequestType types)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_STORE_GET_IFACE (self)->get_accounts_func)
		g_critical ("You must implement tny_account_store_get_accounts\n");
#endif
	TNY_ACCOUNT_STORE_GET_IFACE (self)->get_accounts_func (self, list, types);

	return;
}


/**
 * tny_account_store_add_transport_account:
 * @self: a #TnyAccountTransport object
 * @account: the account to add
 * 
 * Add a transport account to the store
 * 
 **/
void
tny_account_store_add_transport_account (TnyAccountStore *self, TnyTransportAccount *account)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_STORE_GET_IFACE (self)->add_transport_account_func)
		g_critical ("You must implement tny_account_store_add_transport_account\n");
#endif

	TNY_ACCOUNT_STORE_GET_IFACE (self)->add_transport_account_func (self, account);
	return;
}


/**
 * tny_account_store_add_store_account:
 * @self: a #TnyAccountStore object
 * @account: the account to add
 * 
 * Add a storage account to the store
 * 
 **/
void
tny_account_store_add_store_account (TnyAccountStore *self, TnyStoreAccount *account)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_STORE_GET_IFACE (self)->add_store_account_func)
		g_critical ("You must implement tny_account_store_add_store_account\n");
#endif
	TNY_ACCOUNT_STORE_GET_IFACE (self)->add_store_account_func (self, account);
	return;
}


static void
tny_account_store_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) 
	{

		tny_account_store_signals = g_new0 (guint, TNY_ACCOUNT_STORE_LAST_SIGNAL);

/**
 * TnyAccountStore::account_changed:
 * @self: the object on which the signal is emitted
 * @arg1: the #TnyAccount of the account that changed
 * @user_data: user data set when the signal handler was connected
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
 * TnyAccountStore::account_inserted:
 * @self: the object on which the signal is emitted
 * @arg1: the #TnyAccount of the account that got inserted
 * @user_data: user data set when the signal handler was connected.
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
 * TnyAccountStore::account_removed:
 * @self: the object on which the signal is emitted
 * @arg1: the #TnyAccount of the account that got removed
 * @user_data: user data set when the signal handler was connected.
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
 * TnyAccountStore::accounts_reloaded:
 * @self: the object on which the signal is emitted
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

		initialized = TRUE;
	}

	return;
}

static void
tny_account_store_base_finalize (gpointer g_class)
{
	if (tny_account_store_signals)
		g_free (tny_account_store_signals);

	return;
}

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
		  tny_account_store_base_finalize,   /* base_finalize */
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
	}

	return type;
}

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


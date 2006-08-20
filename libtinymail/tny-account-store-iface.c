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

#include <tny-account-iface.h>

#ifndef TNY_ACCOUNT_STORE_IFACE_C
#define TNY_ACCOUNT_STORE_IFACE_C
#endif

#include <tny-account-store-iface.h>

#ifdef TNY_ACCOUNT_STORE_IFACE_C
#undef TNY_ACCOUNT_STORE_IFACE_C
#endif

guint *tny_account_store_iface_signals = NULL;


/**
 * tny_account_store_iface_alert:
 * @self: a #TnyAccountTransportIface object
 * @type: the message type (severity)
 * @prompt: the prompt
 *
 * This jump-to-the-ui method implements showing a message dialog with prompt
 * as prompt. It will return TRUE if the reply was affirmative. Or FALSE if not.
 * 
 * Return value: Whether the user pressed Ok/Yes (TRUE) or Cancel/No (FALSE)
 **/
gboolean 
tny_account_store_iface_alert (TnyAccountStoreIface *self, TnyAlertType type, const gchar *prompt)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_STORE_IFACE_GET_CLASS (self)->alert_func)
		g_critical ("You must implement tny_account_store_iface_alert\n");
#endif
	return TNY_ACCOUNT_STORE_IFACE_GET_CLASS (self)->alert_func (self, type, prompt);
}

/**
 * tny_account_store_iface_get_device:
 * @self: a #TnyAccountTransportIface object
 * 
 * Return value: the device attached to this account store
 **/
TnyDeviceIface* 
tny_account_store_iface_get_device (TnyAccountStoreIface *self)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_STORE_IFACE_GET_CLASS (self)->get_device_func)
		g_critical ("You must implement tny_account_store_iface_get_device\n");
#endif
	return TNY_ACCOUNT_STORE_IFACE_GET_CLASS (self)->get_device_func (self);
}

/**
 * tny_account_store_iface_get_cache_dir:
 * @self: a #TnyAccountTransportIface object
 * 
 * Get the local path that will be used for storing the disk cache
 * 
 * Return value: the local path that will be used for storing the disk cache
 **/
const gchar*
tny_account_store_iface_get_cache_dir (TnyAccountStoreIface *self)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_STORE_IFACE_GET_CLASS (self)->get_cache_dir_func)
		g_critical ("You must implement tny_account_store_iface_get_cache_dir\n");
#endif
	return TNY_ACCOUNT_STORE_IFACE_GET_CLASS (self)->get_cache_dir_func (self);
}


/**
 * tny_account_store_iface_get_accounts:
 * @self: a #TnyAccountTransportIface object
 * @list: a #TnyListIface instance that will be filled with #TnyAccountIface instances
 * @types: a #TnyGetAccountsRequestType that describes which account types are needed
 * 
 * Get a read-only list of accounts in the store
 * 
 **/
void
tny_account_store_iface_get_accounts (TnyAccountStoreIface *self, TnyListIface *list, TnyGetAccountsRequestType types)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_STORE_IFACE_GET_CLASS (self)->get_accounts_func)
		g_critical ("You must implement tny_account_store_iface_get_accounts\n");
#endif
	TNY_ACCOUNT_STORE_IFACE_GET_CLASS (self)->get_accounts_func (self, list, types);

	return;
}


/**
 * tny_account_store_iface_add_transport_account:
 * @self: a #TnyAccountTransportIface object
 * @account: the account to add
 * 
 * Add a transport account to the store
 * 
 **/
void
tny_account_store_iface_add_transport_account (TnyAccountStoreIface *self, TnyTransportAccountIface *account)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_STORE_IFACE_GET_CLASS (self)->add_transport_account_func)
		g_critical ("You must implement tny_account_store_iface_add_transport_account\n");
#endif

	TNY_ACCOUNT_STORE_IFACE_GET_CLASS (self)->add_transport_account_func (self, account);
	return;
}


/**
 * tny_account_store_iface_add_store_account:
 * @self: a #TnyAccountStoreIface object
 * @account: the account to add
 * 
 * Add a storage account to the store
 * 
 **/
void
tny_account_store_iface_add_store_account (TnyAccountStoreIface *self, TnyStoreAccountIface *account)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_STORE_IFACE_GET_CLASS (self)->add_store_account_func)
		g_critical ("You must implement tny_account_store_iface_add_store_account\n");
#endif
	TNY_ACCOUNT_STORE_IFACE_GET_CLASS (self)->add_store_account_func (self, account);
	return;
}


static void
tny_account_store_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) 
	{

		tny_account_store_iface_signals = g_new0 (guint, TNY_ACCOUNT_STORE_IFACE_LAST_SIGNAL);

/**
 * TnyAccountStoreIface::account_changed:
 * @self: the object on which the signal is emitted
 * @arg1: the #TnyAccountIface of the account that changed
 * @user_data: user data set when the signal handler was connected
 *
 * Emitted when an account in the store changed
 */
		tny_account_store_iface_signals[TNY_ACCOUNT_STORE_IFACE_ACCOUNT_CHANGED] =
		   g_signal_new ("account_changed",
			TNY_TYPE_ACCOUNT_STORE_IFACE,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnyAccountStoreIfaceClass, account_changed),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE, 1, TNY_TYPE_ACCOUNT_IFACE);

/**
 * TnyAccountStoreIface::account_inserted:
 * @self: the object on which the signal is emitted
 * @arg1: the #TnyAccountIface of the account that got inserted
 * @user_data: user data set when the signal handler was connected.
 *
 * Emitted when an account is added to the store
 */


		tny_account_store_iface_signals[TNY_ACCOUNT_STORE_IFACE_ACCOUNT_INSERTED] =
		   g_signal_new ("account_inserted",
			TNY_TYPE_ACCOUNT_STORE_IFACE,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnyAccountStoreIfaceClass, account_inserted),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE, 1, TNY_TYPE_ACCOUNT_IFACE);

/**
 * TnyAccountStoreIface::account_removed:
 * @self: the object on which the signal is emitted
 * @arg1: the #TnyAccountIface of the account that got removed
 * @user_data: user data set when the signal handler was connected.
 *
 * Emitted when an account is removed from the store
 */
		tny_account_store_iface_signals[TNY_ACCOUNT_STORE_IFACE_ACCOUNT_REMOVED] =
		   g_signal_new ("account_removed",
			TNY_TYPE_ACCOUNT_STORE_IFACE,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnyAccountStoreIfaceClass, account_removed),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE, 1, TNY_TYPE_ACCOUNT_IFACE);

/**
 * TnyAccountStoreIface::accounts_reloaded:
 * @self: the object on which the signal is emitted
 *
 * Emitted when the store reloads the accounts
 */
		tny_account_store_iface_signals[TNY_ACCOUNT_STORE_IFACE_ACCOUNTS_RELOADED] =
		   g_signal_new ("accounts_reloaded",
			TNY_TYPE_ACCOUNT_STORE_IFACE,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnyAccountStoreIfaceClass, accounts_reloaded),
			NULL, NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE, 0);

		initialized = TRUE;
	}

	return;
}

static void
tny_account_store_iface_base_finalize (gpointer g_class)
{
	if (tny_account_store_iface_signals)
		g_free (tny_account_store_iface_signals);

	return;
}

GType
tny_account_store_iface_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyAccountStoreIfaceClass),
		  tny_account_store_iface_base_init,   /* base_init */
		  tny_account_store_iface_base_finalize,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL,    /* instance_init */
		  NULL
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyAccountStoreIface", &info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
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
      { TNY_ACCOUNT_STORE_IFACE_TRANSPORT_ACCOUNTS, "TNY_ACCOUNT_STORE_IFACE_TRANSPORT_ACCOUNTS", "transport" },
      { TNY_ACCOUNT_STORE_IFACE_STORE_ACCOUNTS, "TNY_ACCOUNT_STORE_IFACE_STORE_ACCOUNTS", "store" },
      { TNY_ACCOUNT_STORE_IFACE_BOTH, "TNY_ACCOUNT_STORE_IFACE_BOTH", "both" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyGetAccountsRequestType", values);
  }
  return etype;
}


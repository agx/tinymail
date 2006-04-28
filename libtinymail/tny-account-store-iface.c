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
	return TNY_ACCOUNT_STORE_IFACE_GET_CLASS (self)->get_cache_dir_func (self);
}


/**
 * tny_account_store_iface_get_transport_accounts:
 * @self: a #TnyAccountTransportIface object
 * 
 * Get a read-only list of transport accounts in the store
 * 
 * Return value: A read-only GList which contains TnyTransportAccountIface instances
 **/
const GList*
tny_account_store_iface_get_transport_accounts (TnyAccountStoreIface *self)
{
	return TNY_ACCOUNT_STORE_IFACE_GET_CLASS (self)->get_transport_accounts_func (self);
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
	TNY_ACCOUNT_STORE_IFACE_GET_CLASS (self)->add_transport_account_func (self, account);
	return;
}


/**
 * tny_account_store_iface_get_store_accounts:
 * @self: a #TnyAccountStoreIface object
 * 
 * Get a read-only list of storage accounts in the store
 * 
 * Return value: A read-only GList which contains TnyStoreAccountIface instances
 **/
const GList*
tny_account_store_iface_get_store_accounts (TnyAccountStoreIface *self)
{
	return TNY_ACCOUNT_STORE_IFACE_GET_CLASS (self)->get_store_accounts_func (self);
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
 * @account: the #TnyAccountIface of the account that changed
 *
 * The "account_changed" signal is emitted when an account in the store changed
 **/
		tny_account_store_iface_signals[ACCOUNT_CHANGED] =
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
 * @account: the #TnyAccountIface of the account that got inserted
 *
 * The "account_inserted" signal is emitted when an account is added to the
 * store
 **/


		tny_account_store_iface_signals[ACCOUNT_INSERTED] =
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
 * @account: the #TnyAccountIface of the account that got removed
 *
 * The "account_removed" signal is emitted when an account is removed from the
 * store
 **/
		tny_account_store_iface_signals[ACCOUNT_REMOVED] =
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
 * The "accounts_reloaded" signal is emitted when the store reloaded the 
 * accounts
 **/
		tny_account_store_iface_signals[ACCOUNTS_RELOADED] =
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
	if (type == 0) 
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
		  NULL    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyAccountStoreIface", &info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}

	return type;
}



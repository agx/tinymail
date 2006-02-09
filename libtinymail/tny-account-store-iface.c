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

#ifndef TNY_ACCOUNT_STORE_IFACE_C
#define TNY_ACCOUNT_STORE_IFACE_C
#endif

#include <tny-account-store-iface.h>

#ifdef TNY_ACCOUNT_STORE_IFACE_C
#undef TNY_ACCOUNT_STORE_IFACE_C
#endif

guint *tny_account_store_iface_signals = NULL;

const GList*
tny_account_store_iface_get_accounts (TnyAccountStoreIface *self)
{
	return TNY_ACCOUNT_STORE_IFACE_GET_CLASS (self)->get_accounts_func (self);
}

void
tny_account_store_iface_add_account (TnyAccountStoreIface *self, TnyAccountIface *account)
{
	TNY_ACCOUNT_STORE_IFACE_GET_CLASS (self)->add_account_func (self, account);
	return;
}


static void
tny_account_store_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) 
	{

		tny_account_store_iface_signals = g_new0 (guint, LAST_SIGNAL);

		tny_account_store_iface_signals[ACCOUNT_CHANGED] =
		   g_signal_new ("account_changed",
			TNY_ACCOUNT_STORE_IFACE_TYPE,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnyAccountStoreIfaceClass, account_changed),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE, 1, TNY_ACCOUNT_STORE_IFACE_TYPE);

		tny_account_store_iface_signals[ACCOUNT_INSERTED] =
		   g_signal_new ("account_inserted",
			TNY_ACCOUNT_STORE_IFACE_TYPE,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnyAccountStoreIfaceClass, account_inserted),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE, 1, TNY_ACCOUNT_STORE_IFACE_TYPE);

		tny_account_store_iface_signals[ACCOUNT_REMOVED] =
		   g_signal_new ("account_removed",
			TNY_ACCOUNT_STORE_IFACE_TYPE,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnyAccountStoreIfaceClass, account_removed),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE, 1, TNY_ACCOUNT_STORE_IFACE_TYPE);

		tny_account_store_iface_signals[ACCOUNTS_RELOADED] =
		   g_signal_new ("accounts_reloaded",
			TNY_ACCOUNT_STORE_IFACE_TYPE,
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



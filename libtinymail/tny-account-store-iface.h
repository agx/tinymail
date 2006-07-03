#ifndef TNY_ACCOUNT_STORE_IFACE_H
#define TNY_ACCOUNT_STORE_IFACE_H

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

#include <glib.h>
#include <glib-object.h>
#include <tny-shared.h>

G_BEGIN_DECLS

#define TNY_TYPE_ACCOUNT_STORE_IFACE             (tny_account_store_iface_get_type ())
#define TNY_ACCOUNT_STORE_IFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_ACCOUNT_STORE_IFACE, TnyAccountStoreIface))
#define TNY_ACCOUNT_STORE_IFACE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_ACCOUNT_STORE_IFACE, TnyAccountStoreIfaceClass))
#define TNY_IS_ACCOUNT_STORE_IFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_ACCOUNT_STORE_IFACE))
#define TNY_IS_ACCOUNT_STORE_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_ACCOUNT_STORE_IFACE))
#define TNY_ACCOUNT_STORE_IFACE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_ACCOUNT_STORE_IFACE, TnyAccountStoreIfaceClass))

enum
{
	TNY_ACCOUNT_STORE_IFACE_ACCOUNT_CHANGED,
	TNY_ACCOUNT_STORE_IFACE_ACCOUNT_INSERTED,
	TNY_ACCOUNT_STORE_IFACE_ACCOUNT_REMOVED,
	TNY_ACCOUNT_STORE_IFACE_ACCOUNTS_RELOADED,
	TNY_ACCOUNT_STORE_IFACE_LAST_SIGNAL
};

enum _TnyAlertType
{
	TNY_ALERT_TYPE_INFO,
	TNY_ALERT_TYPE_WARNING,
	TNY_ALERT_TYPE_ERROR
};

#ifndef TNY_ACCOUNT_STORE_IFACE_C
extern guint *tny_account_store_iface_signals;
#endif

struct _TnyAccountStoreIfaceClass
{
	GTypeInterface parent;

	/* Signals */
	void (*account_changed)                                 (TnyAccountStoreIface *self, TnyAccountIface *account);
	void (*account_inserted)                                (TnyAccountStoreIface *self, TnyAccountIface *account);
	void (*account_removed)                                 (TnyAccountStoreIface *self, TnyAccountIface *account);
	void (*accounts_reloaded)                               (TnyAccountStoreIface *self);

	/* Methods */

/*
void 
tny_account_store_iface_get_accounts (TnyAccountStoreIface *self, TnyListIface *list, TnyGetAccountsRequestType type)

enum 
{
	ONLY_TRANSPORT_ACCOUNTS,
	ONLY_STORE_ACCOUNTS,
	BOTH
} TnyGetACcountsRequestType;
*/

	const GList*  (*get_store_accounts_func)                (TnyAccountStoreIface *self);
	void          (*add_store_account_func)                 (TnyAccountStoreIface *self, TnyStoreAccountIface *account);

	const GList*  (*get_transport_accounts_func)            (TnyAccountStoreIface *self);
	void          (*add_transport_account_func)             (TnyAccountStoreIface *self, TnyTransportAccountIface *account);

	const gchar*  (*get_cache_dir_func)                     (TnyAccountStoreIface *self);

	const TnyDeviceIface* 
		(*get_device_func)				(TnyAccountStoreIface *self);

	gboolean (*alert_func)					(TnyAccountStoreIface *self, TnyAlertType type, const gchar *prompt);
};

GType         tny_account_store_iface_get_type                  (void);

const GList*  tny_account_store_iface_get_store_accounts        (TnyAccountStoreIface *self);
void          tny_account_store_iface_add_store_account         (TnyAccountStoreIface *self, TnyStoreAccountIface *account);


const GList*  tny_account_store_iface_get_transport_accounts    (TnyAccountStoreIface *self);
void          tny_account_store_iface_add_transport_account     (TnyAccountStoreIface *self, TnyTransportAccountIface *account);

const gchar*  tny_account_store_iface_get_cache_dir             (TnyAccountStoreIface *self);

const TnyDeviceIface* 
		tny_account_store_iface_get_device		(TnyAccountStoreIface *self);

gboolean 	tny_account_store_iface_alert 			(TnyAccountStoreIface *self, TnyAlertType type, const gchar *prompt);

G_END_DECLS

#endif

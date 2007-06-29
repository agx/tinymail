#ifndef TNY_ACCOUNT_H
#define TNY_ACCOUNT_H

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
#include <tny-status.h>

G_BEGIN_DECLS

#define TNY_TYPE_ACCOUNT             (tny_account_get_type ())
#define TNY_ACCOUNT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_ACCOUNT, TnyAccount))
#define TNY_IS_ACCOUNT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_ACCOUNT))
#define TNY_ACCOUNT_GET_IFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_ACCOUNT, TnyAccountIface))

#define TNY_TYPE_ACCOUNT_TYPE (tny_account_type_get_type())

#ifndef TNY_SHARED_H
typedef enum _TnyAccountType TnyAccountType;
typedef struct _TnyAccount TnyAccount;
typedef struct _TnyAccountIface TnyAccountIface;
typedef enum _TnyAccountSignal TnyAccountSignal;
typedef enum _TnyConnectionStatus TnyConnectionStatus;
#endif

enum _TnyConnectionStatus
{
	TNY_CONNECTION_STATUS_DISCONNECTED,
	TNY_CONNECTION_STATUS_DISCONNECTED_BROKEN,
	TNY_CONNECTION_STATUS_CONNECTED_BROKEN,
	TNY_CONNECTION_STATUS_CONNECTED,
	TNY_CONNECTION_STATUS_RECONNECTING,
	TNY_CONNECTION_STATUS_INIT
};

enum _TnyAccountType
{
	TNY_ACCOUNT_TYPE_STORE,
	TNY_ACCOUNT_TYPE_TRANSPORT,
	TNY_ACCOUNT_TYPE_COMBINED
};

enum _TnyAccountSignal
{
	TNY_ACCOUNT_CONNECTION_STATUS_CHANGED,
	TNY_ACCOUNT_LAST_SIGNAL
};

extern guint tny_account_signals [TNY_ACCOUNT_LAST_SIGNAL];

struct _TnyAccountIface
{
	GTypeInterface parent;

	/* Methods */
	TnyConnectionStatus (*get_connection_status_func)(TnyAccount *self);
	void (*set_id_func) (TnyAccount *self, const gchar *id);
	void (*set_name_func) (TnyAccount *self, const gchar *name);
	void (*set_secure_auth_mech_func) (TnyAccount *self, const gchar *mech);
	void (*set_proto_func) (TnyAccount *self, const gchar *proto);
	void (*set_user_func) (TnyAccount *self, const gchar *user);
	void (*set_hostname_func) (TnyAccount *self, const gchar *host);
	void (*set_port_func) (TnyAccount *self, guint port);
	void (*set_url_string_func) (TnyAccount *self, const gchar *url_string);
	void (*set_pass_func_func) (TnyAccount *self, TnyGetPassFunc get_pass_func);
	void (*set_forget_pass_func_func) (TnyAccount *self, TnyForgetPassFunc get_forget_pass_func);
	TnyGetPassFunc (*get_pass_func_func) (TnyAccount *self);
	TnyForgetPassFunc (*get_forget_pass_func_func) (TnyAccount *self);
	const gchar* (*get_id_func) (TnyAccount *self);
	const gchar* (*get_name_func) (TnyAccount *self);
	const gchar* (*get_secure_auth_mech_func) (TnyAccount *self);
	const gchar* (*get_proto_func) (TnyAccount *self);
	const gchar* (*get_user_func) (TnyAccount *self);
	const gchar* (*get_hostname_func) (TnyAccount *self);
	guint (*get_port_func) (TnyAccount *self);
	gchar* (*get_url_string_func) (TnyAccount *self);
	TnyAccountType (*get_account_type_func) (TnyAccount *self);
	void (*cancel_func) (TnyAccount *self);
	gboolean (*matches_url_string_func) (TnyAccount *self, const gchar *url_string);
	void (*start_operation_func) (TnyAccount *self, TnyStatusDomain domain, TnyStatusCode code, TnyStatusCallback status_callback, gpointer status_user_data);
	void (*stop_operation_func) (TnyAccount *self, gboolean *canceled);

	/* Signals*/
	void (*connection_status_changed) (TnyAccount *self, TnyConnectionStatus status);
};

GType tny_account_get_type (void);
GType tny_account_type_get_type (void);

TnyConnectionStatus tny_account_get_connection_status (TnyAccount *self);
void tny_account_set_id (TnyAccount *self, const gchar *id);
void tny_account_set_name (TnyAccount *self, const gchar *name);
void tny_account_set_secure_auth_mech (TnyAccount *self, const gchar *mech);
void tny_account_set_proto (TnyAccount *self, const gchar *proto);
void tny_account_set_user (TnyAccount *self, const gchar *user);
void tny_account_set_hostname (TnyAccount *self, const gchar *host);
void tny_account_set_port (TnyAccount *self, guint port);
void tny_account_set_pass_func (TnyAccount *self, TnyGetPassFunc get_pass_func);
void tny_account_set_url_string (TnyAccount *self, const gchar *url_string);
TnyAccountType tny_account_get_account_type (TnyAccount *self);
const gchar* tny_account_get_id (TnyAccount *self);
const gchar* tny_account_get_name (TnyAccount *self);
const gchar* tny_account_get_secure_auth_mech (TnyAccount *self);
const gchar* tny_account_get_proto (TnyAccount *self);
const gchar* tny_account_get_user (TnyAccount *self);
const gchar* tny_account_get_hostname (TnyAccount *self);
guint tny_account_get_port (TnyAccount *self);
gchar* tny_account_get_url_string (TnyAccount *self);
TnyGetPassFunc tny_account_get_pass_func (TnyAccount *self);
void tny_account_set_forget_pass_func (TnyAccount *self, TnyForgetPassFunc forget_pass_func);
TnyForgetPassFunc tny_account_get_forget_pass_func (TnyAccount *self);
void tny_account_cancel (TnyAccount *self);
gboolean tny_account_matches_url_string (TnyAccount *self, const gchar *url_string);
void tny_account_start_operation (TnyAccount *self, TnyStatusDomain domain, TnyStatusCode code, TnyStatusCallback status_callback, gpointer status_user_data);
void tny_account_stop_operation (TnyAccount *self, gboolean *canceled);

G_END_DECLS

#endif

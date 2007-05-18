#ifndef TNY_CAMEL_ACCOUNT_H
#define TNY_CAMEL_ACCOUNT_H

/* libtinymail-camel - The Tiny Mail base library for Camel
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
#include <tny-camel-shared.h>
#include <tny-account.h>

G_BEGIN_DECLS

#define TNY_TYPE_CAMEL_ACCOUNT             (tny_camel_account_get_type ())
#define TNY_CAMEL_ACCOUNT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_CAMEL_ACCOUNT, TnyCamelAccount))
#define TNY_CAMEL_ACCOUNT_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_CAMEL_ACCOUNT, TnyCamelAccountClass))
#define TNY_IS_CAMEL_ACCOUNT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_CAMEL_ACCOUNT))
#define TNY_IS_ACAMEL_CCOUNT_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_CAMEL_ACCOUNT))
#define TNY_CAMEL_ACCOUNT_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_CAMEL_ACCOUNT, TnyCamelAccountClass))

/* This is an abstract type, you cannot (should not) instantiate it */

typedef struct _TnyCamelAccount TnyCamelAccount;
typedef struct _TnyCamelAccountClass TnyCamelAccountClass;

struct _TnyCamelAccount
{
	GObject parent;
};

struct _TnyCamelAccountClass 
{
	GObjectClass parent;

	/* Virtual methods */
	gboolean (*is_connected_func)(TnyAccount *self);
	void (*set_id_func) (TnyAccount *self, const gchar *id);
	void (*set_name_func) (TnyAccount *self, const gchar *name);
	void (*set_secure_auth_mech_func) (TnyAccount *self, const gchar *name);
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
	void (*try_connect_func) (TnyAccount *self, GError **err);
	void (*cancel_func) (TnyAccount *self);
	gboolean (*matches_url_string_func) (TnyAccount *self, const gchar *url_string);

	void (*add_option_func) (TnyCamelAccount *self, const gchar *option);
	void (*set_online_func) (TnyCamelAccount *self, gboolean online, GError **err);

	/* Abstract methods */
	void (*prepare_func) (TnyCamelAccount *self);
};

GType tny_camel_account_get_type (void);

void tny_camel_account_add_option (TnyCamelAccount *self, const gchar *option);
void tny_camel_account_set_session (TnyCamelAccount *self, TnySessionCamel *session);
void tny_camel_account_set_online (TnyCamelAccount *self, gboolean online, GError **err);


typedef void (*TnyCamelGetSupportedSecureAuthCallback) (TnyCamelAccount *self, gboolean cancelled, TnyList *auth_types, GError **err, gpointer user_data);
void tny_camel_account_get_supported_secure_authentication(TnyCamelAccount *self, TnyCamelGetSupportedSecureAuthCallback callback, TnyStatusCallback status_callback, gpointer user_data);

G_END_DECLS

#endif


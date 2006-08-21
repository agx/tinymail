#ifndef TNY_ACCOUNT_PRIV_H
#define TNY_ACCOUNT_PRIV_H

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

#include <tny-session-camel.h>

typedef struct _TnyAccountPriv TnyAccountPriv;

struct _TnyAccountPriv
{
	TnySessionCamel *session;
	GStaticRecMutex *service_lock;
	CamelService *service;
	CamelException *ex;
	gchar *url_string, *id, *user, *host, *proto;
	TnyGetPassFunc get_pass_func;
	TnyForgetPassFunc forget_pass_func;
	gboolean pass_func_set, forget_pass_func_set;
	CamelProviderType type;
	CamelOperation *cancel;
	GMutex *cancel_lock;
	gboolean connected, inuse_spin;
	gchar *name; GList *options;
	TnyAccountType account_type;
};

const CamelService* _tny_account_get_service (TnyAccount *self);
const gchar* _tny_account_get_url_string (TnyAccount *self);
void _tny_account_start_camel_operation (TnyAccountIface *self, CamelOperationStatusFunc func, gpointer user_data, const gchar *what);
void _tny_account_stop_camel_operation (TnyAccountIface *self);

#define TNY_ACCOUNT_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_ACCOUNT, TnyAccountPriv))

#endif

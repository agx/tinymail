#ifndef TNY_SESSION_CAMEL_H
#define TNY_SESSION_CAMEL_H

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
#include <camel/camel-session.h>
#include <tny-shared.h>
#include <tny-camel-shared.h>
#include <tny-lockable.h>

G_BEGIN_DECLS

#define TNY_TYPE_SESSION_CAMEL     (tny_session_camel_get_type ())
#define TNY_SESSION_CAMEL(obj)     (CAMEL_CHECK_CAST((obj), TNY_TYPE_SESSION_CAMEL, TnySessionCamel))
#define TNY_SESSION_CAMEL_CLASS(k) (CAMEL_CHECK_CLASS_CAST ((k), TNY_TYPE_SESSION_CAMEL, TnySessionCamelClass))
#define TNY_IS_SESSION_CAMEL(o)    (CAMEL_CHECK_TYPE((o), TNY_TYPE_SESSION_CAMEL))

struct _TnySessionCamel
{
	CamelSession parent_object;

	gpointer device;
	gpointer account_store;
	gboolean interactive, prev_constat, first_switch;
	guint connchanged_signal;
	GList *current_accounts;
	gchar *camel_dir;
	gboolean in_auth_function;
	TnyLockable *ui_lock;
};

struct _TnySessionCamelClass
{
	CamelSessionClass parent_class;
};

CamelType tny_session_camel_get_type (void);

void tny_session_camel_set_pass_func (TnySessionCamel *self, TnyAccount *account, TnyGetPassFunc get_pass_func);
void tny_session_camel_set_forget_pass_func (TnySessionCamel *self, TnyAccount *account, TnyForgetPassFunc get_forget_pass_func);
void tny_session_camel_set_account_store (TnySessionCamel *self, TnyAccountStore *account_store);
void tny_session_camel_set_device (TnySessionCamel *self, TnyDevice *device);

void tny_session_camel_set_ui_locker (TnySessionCamel *self, TnyLockable *ui_lock);

G_END_DECLS

#endif

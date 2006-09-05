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

	/* This is an abstract method (as it's not implemented by 
	TnyCamelAccount, it's also not virtual) */
    
	void (*reconnect_func) (TnyCamelAccount *self);
};

GType tny_camel_account_get_type (void);

void tny_camel_account_add_option (TnyCamelAccount *self, const gchar *option);
void tny_camel_account_set_session (TnyCamelAccount *self, TnySessionCamel *session);
void tny_camel_account_set_online_status (TnyCamelAccount *self, gboolean offline);

G_END_DECLS

#endif


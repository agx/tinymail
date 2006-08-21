#ifndef TNY_ACCOUNT_IFACE_H
#define TNY_ACCOUNT_IFACE_H

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

#define TNY_TYPE_ACCOUNT_IFACE             (tny_account_iface_get_type ())
#define TNY_ACCOUNT_IFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_ACCOUNT_IFACE, TnyAccountIface))
#define TNY_ACCOUNT_IFACE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_ACCOUNT_IFACE, TnyAccountIfaceClass))
#define TNY_IS_ACCOUNT_IFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_ACCOUNT_IFACE))
#define TNY_IS_ACCOUNT_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_ACCOUNT_IFACE))
#define TNY_ACCOUNT_IFACE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_ACCOUNT_IFACE, TnyAccountIfaceClass))

#define TNY_TYPE_ACCOUNT_TYPE (tny_account_type_get_type())

enum _TnyAccountType
{
	TNY_ACCOUNT_TYPE_STORE,
	TNY_ACCOUNT_TYPE_TRANSPORT
};

struct _TnyAccountIfaceClass
{
	GTypeInterface parent;

	gboolean (*is_connected_func)(TnyAccountIface *self);
        void (*set_id_func)                (TnyAccountIface *self, const gchar *id);
        void (*set_name_func)              (TnyAccountIface *self, const gchar *name);
        void (*set_proto_func)             (TnyAccountIface *self, const gchar *proto);
        void (*set_user_func)              (TnyAccountIface *self, const gchar *user);
        void (*set_hostname_func)          (TnyAccountIface *self, const gchar *host);
        void (*set_url_string_func)        (TnyAccountIface *self, const gchar *url_string);
	void (*set_account_type_func)      (TnyAccountIface *self, TnyAccountType type);

        void (*set_pass_func_func)         (TnyAccountIface *self, TnyGetPassFunc get_pass_func);
        void (*set_forget_pass_func_func)  (TnyAccountIface *self, TnyForgetPassFunc get_forget_pass_func);
        TnyGetPassFunc     (*get_pass_func_func)        (TnyAccountIface *self);
	TnyForgetPassFunc  (*get_forget_pass_func_func) (TnyAccountIface *self);

        const gchar*    (*get_id_func )              (TnyAccountIface *self);
        const gchar*    (*get_name_func )            (TnyAccountIface *self);
        const gchar*    (*get_proto_func )           (TnyAccountIface *self);
        const gchar*    (*get_user_func)             (TnyAccountIface *self);
        const gchar*    (*get_hostname_func)         (TnyAccountIface *self);
        const gchar*    (*get_url_string_func)       (TnyAccountIface *self);
	TnyAccountType  (*get_account_type_func)     (TnyAccountIface *self);
};

GType tny_account_iface_get_type (void);
GType tny_account_type_get_type (void);

gboolean tny_account_iface_is_connected (TnyAccountIface *self);

void         tny_account_iface_set_id          (TnyAccountIface *self, const gchar *id);

void         tny_account_iface_set_name       (TnyAccountIface *self, const gchar *name);
void         tny_account_iface_set_proto       (TnyAccountIface *self, const gchar *proto);
void         tny_account_iface_set_user        (TnyAccountIface *self, const gchar *user);
void         tny_account_iface_set_hostname    (TnyAccountIface *self, const gchar *host);
void         tny_account_iface_set_pass_func   (TnyAccountIface *self, TnyGetPassFunc get_pass_func);
void         tny_account_iface_set_url_string  (TnyAccountIface *self, const gchar *url_string);

void           tny_account_iface_set_account_type  (TnyAccountIface *self, TnyAccountType type);
TnyAccountType tny_account_iface_get_account_type  (TnyAccountIface *self);

const gchar* tny_account_iface_get_id (TnyAccountIface *self);

const gchar* tny_account_iface_get_name (TnyAccountIface *self);
const gchar* tny_account_iface_get_proto (TnyAccountIface *self);
const gchar* tny_account_iface_get_user (TnyAccountIface *self);
const gchar* tny_account_iface_get_hostname (TnyAccountIface *self);
const gchar* tny_account_iface_get_url_string (TnyAccountIface *self);

TnyGetPassFunc tny_account_iface_get_pass_func (TnyAccountIface *self);

void tny_account_iface_set_forget_pass_func (TnyAccountIface *self, TnyForgetPassFunc get_forget_pass_func);
TnyForgetPassFunc tny_account_iface_get_forget_pass_func (TnyAccountIface *self);

G_END_DECLS

#endif

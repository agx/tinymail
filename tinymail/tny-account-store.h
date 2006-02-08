#ifndef TNY_ACCOUNT_STORE_H
#define TNY_ACCOUNT_STORE_H

/*
 * Copyright (C) 2006-2007 Philip Van Hoof <pvanhoof@gnome.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with self program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <glib.h>
#include <glib-object.h>
#include <tny-shared.h>

G_BEGIN_DECLS

#define TNY_ACCOUNT_STORE_TYPE             (tny_account_store_get_type ())
#define TNY_ACCOUNT_STORE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_ACCOUNT_STORE_TYPE, TnyAccountStore))
#define TNY_ACCOUNT_STORE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_ACCOUNT_STORE_TYPE, TnyAccountStoreClass))
#define TNY_IS_ACCOUNT_STORE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_ACCOUNT_STORE_TYPE))
#define TNY_IS_ACCOUNT_STORE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_ACCOUNT_STORE_TYPE))
#define TNY_ACCOUNT_STORE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_ACCOUNT_STORE_TYPE, TnyAccountStoreClass))

typedef struct _TnyAccountStore TnyAccountStore;
typedef struct _TnyAccountStoreClass TnyAccountStoreClass;

struct _TnyAccountStore
{
	GObject parent;
};

struct _TnyAccountStoreClass
{
	GObjectClass parent;
};

GType               tny_account_store_get_type       (void);
TnyAccountStore*    tny_account_store_get_instance   (void);

const GList*        tny_account_store_get_accounts   (TnyAccountStore *self);
void                tny_account_store_add_account    (TnyAccountStore *self, TnyAccountIface *account);

G_END_DECLS

#endif

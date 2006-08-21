#ifndef TNY_MEMTEST_ACCOUNT_STORE_H
#define TNY_MEMTEST_ACCOUNT_STORE_H

/* tinymail - Tiny Mail
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
#include <tny-account-store.h>
#include <tny-camel-shared.h>

G_BEGIN_DECLS

#define TNY_TYPE_MEMTEST_ACCOUNT_STORE             (tny_memtest_account_store_get_type ())
#define TNY_MEMTEST_ACCOUNT_STORE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_MEMTEST_ACCOUNT_STORE, TnyMemTestAccountStore))
#define TNY_MEMTEST_ACCOUNT_STORE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_MEMTEST_ACCOUNT_STORE, TnyMemTestAccountStoreClass))
#define TNY_IS_MEMTEST_ACCOUNT_STORE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_MEMTEST_ACCOUNT_STORE))
#define TNY_IS_MEMTEST_ACCOUNT_STORE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_MEMTEST_ACCOUNT_STORE))
#define TNY_MEMTEST_ACCOUNT_STORE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_MEMTEST_ACCOUNT_STORE, TnyMemTestAccountStoreClass))

typedef struct _TnyMemTestAccountStore TnyMemTestAccountStore;
typedef struct _TnyMemTestAccountStoreClass TnyMemTestAccountStoreClass;

struct _TnyMemTestAccountStore
{
	TnyAccountStore parent;
};

struct _TnyMemTestAccountStoreClass
{
	TnyAccountStoreClass parent;
};

GType               tny_memtest_account_store_get_type       (void);
TnyMemTestAccountStore*    tny_memtest_account_store_new            (void);
TnySessionCamel*    tny_memtest_account_store_get_session    (TnyMemTestAccountStore *self);

G_END_DECLS

#endif

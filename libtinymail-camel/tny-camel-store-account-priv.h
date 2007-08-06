#ifndef TNY_CAMEL_STORE_ACCOUNT_PRIV_H
#define TNY_CAMEL_STORE_ACCOUNT_PRIV_H

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
#include <tny-camel-store-account.h>

#include "tny-camel-queue-priv.h"
#include "tny-session-camel-priv.h"

typedef struct _TnyCamelStoreAccountPriv TnyCamelStoreAccountPriv;

struct _TnyCamelStoreAccountPriv
{
	CamelStore *iter_store;
	CamelFolderInfo *iter;
	GList *managed_folders;
	TnyList *sobservers;
	gboolean cant_reuse_iter;
	GStaticRecMutex *factory_lock, *obs_lock;
	TnyCamelQueue *queue, *msg_queue;
};

#define TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_CAMEL_STORE_ACCOUNT, TnyCamelStoreAccountPriv))

void _tny_camel_store_account_emit_conchg_signal (TnyCamelStoreAccount *self);
void _tny_camel_store_account_queue_going_online (TnyCamelStoreAccount *self, TnySessionCamel *session, gboolean online, go_online_callback_func err_func, gpointer user_data);

#endif

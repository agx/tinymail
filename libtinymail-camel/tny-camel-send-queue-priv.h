#ifndef TNY_CAMEL_SEND_QUEUE_PRIV_H
#define TNY_CAMEL_SEND_QUEUE_PRIV_H

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

#include <tny-camel-transport-account.h>
#include <tny-folder.h>

typedef struct _TnyCamelSendQueuePriv TnyCamelSendQueuePriv;

struct _TnyCamelSendQueuePriv
{
	TnyTransportAccount *trans_account;
	TnyFolder *sentbox_cache, *outbox_cache;
	guint total;
	gint signal;
	GThread *thread;
	GMutex *todo_lock, *sending_lock; 
	gboolean do_continue, is_running;
};

#endif

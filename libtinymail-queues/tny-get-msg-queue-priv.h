#ifndef TNY_GET_MSG_QUEUE_PRIV_H
#define TNY_GET_MSG_QUEUE_PRIV_H

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

typedef struct _TnyGetMsgQueuePriv TnyGetMsgQueuePriv;

struct _TnyGetMsgQueuePriv
{
	OAsyncWorker *queue;
	GMutex *lock;
	TnyFolder *folder;
};


#define TNY_GET_MSG_QUEUE_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_GET_MSG_QUEUE, TnyGetMsgQueuePriv))


#endif

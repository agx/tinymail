#ifndef TNY_CAMEL_MSG_PRIV_H
#define TNY_CAMEL_MSG_PRIV_H

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

#include <camel/camel.h>
#include <camel/camel-stream-buffer.h>

typedef struct _TnyCamelMsgPriv TnyCamelMsgPriv;

struct _TnyCamelMsgPriv
{
	GMutex *message_lock;
	GMutex *header_lock;
	TnyHeader *header;
	GMutex *parts_lock;
	GMutex *folder_lock;
	TnyFolder *folder;
};

CamelMimeMessage* _tny_camel_msg_get_camel_mime_message (TnyCamelMsg *self);
void _tny_camel_msg_set_camel_mime_message (TnyCamelMsg *self, CamelMimeMessage *message);
void _tny_camel_msg_set_folder (TnyCamelMsg *self, TnyFolder *folder);

#endif

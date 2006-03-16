#ifndef TNY_MSG_PRIV_H
#define TNY_MSG_PRIV_H

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

typedef struct _TnyMsgPriv TnyMsgPriv;

struct _TnyMsgPriv
{
	GMutex *message_lock;
	CamelMimeMessage *message;
	GMutex *header_lock;
	TnyMsgHeaderIface *header;
	GMutex *parts_lock;
	GList *parts;
	GMutex *folder_lock;
	TnyMsgFolderIface *folder;
};

void  _tny_msg_set_camel_mime_message    (TnyMsg *self, CamelMimeMessage *message);
void  _tny_msg_header_set_not_uncachable (TnyMsgHeader *self);

#endif

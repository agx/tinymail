#ifndef TNY_CAMEL_HEADER_PRIV_H
#define TNY_CAMEL_HEADER_PRIV_H

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
#include <camel/camel-folder-summary.h>
#include <tny-camel-folder.h>

#include "tny-camel-folder-priv.h"

typedef struct _WriteInfo WriteInfo;
struct _WriteInfo
{
	CamelMimeMessage *msg;
	gchar *mime_from;
};

typedef struct _MemInfo MemInfo;
struct _MemInfo
{
	gchar *uid, *bcc, *cc, *message_id, 
		  *from, *to, *subject, *replyto;
	time_t date_received, date_sent;
	guint message_size;
	TnyHeaderFlags flags;
};

#pragma pack(1)
struct _TnyCamelHeader
{
	GObject parent;
	gpointer info;
	TnyCamelFolder *folder;
	guchar write:2;
#ifdef HEALTHY_CHECK
 	guchar healthy:1;
#endif
};

struct _TnyCamelHeaderClass 
{
	GObjectClass parent_class;
};

void _tny_camel_header_set_camel_message_info (TnyCamelHeader *self, CamelMessageInfo *camel_message_info, gboolean knowit);
void _tny_camel_header_set_folder (TnyCamelHeader *self, TnyCamelFolder *folder, TnyCamelFolderPriv *tpriv);
void _tny_camel_header_set_camel_mime_message (TnyCamelHeader *self, CamelMimeMessage *camel_mime_message);
void _tny_camel_header_set_as_memory (TnyCamelHeader *self, MemInfo *info);

#endif

#ifndef TNY_HEADER_PRIV_H
#define TNY_HEADER_PRIV_H

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
#include <tny-folder.h>
#include "tny-folder-priv.h"

typedef struct _WriteInfo WriteInfo;
struct _WriteInfo
{
	CamelMimeMessage *msg;
	gchar *mime_from;
};

#pragma pack(1) /* Size will be 21 in stead of 24 */
struct _TnyHeader 
{
	GObject parent;		
	gpointer info;
	TnyFolderIface *folder;
	guchar write:1;
};

struct _TnyHeaderClass 
{
	GObjectClass parent_class;
};

void _tny_header_set_camel_message_info (TnyHeader *self, CamelMessageInfo *camel_message_info);
CamelMimeMessage* _tny_header_get_camel_mime_message (TnyHeader *self);
void _tny_header_set_folder (TnyHeader *self, TnyFolder *folder, TnyFolderPriv *tpriv);

#endif

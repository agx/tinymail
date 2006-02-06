#ifndef TNY_SHARED_H
#define TNY_SHARED_H

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

G_BEGIN_DECLS

/* I used forward typedefs to make it possible to cross-reference */

typedef struct _TnyMsgIface TnyMsgIface;
typedef struct _TnyMsgIfaceClass TnyMsgIfaceClass;
typedef struct _TnyMsgFolderIface TnyMsgFolderIface;
typedef struct _TnyMsgFolderIfaceClass TnyMsgFolderIfaceClass;
typedef struct _TnyMsgHeaderIface TnyMsgHeaderIface;
typedef struct _TnyMsgHeaderIfaceClass TnyMsgHeaderIfaceClass;
typedef struct _TnyMsgBodyIface TnyMsgBodyIface;
typedef struct _TnyMsgBodyIfaceClass TnyMsgBodyIfaceClass;
typedef struct _TnyMsgAttachmentIface TnyMsgAttachmentIface;
typedef struct _TnyMsgAttachmentIfaceClass TnyMsgAttachmentIfaceClass;
typedef struct _TnyMsgAccountIface TnyMsgAccountIface;
typedef struct _TnyMsgAccountIfaceClass TnyMsgAccountIfaceClass;
typedef struct _TnyMsgHeaderProxy TnyMsgHeaderProxy;
typedef struct _TnyMsgHeaderProxyClass TnyMsgHeaderProxyClass;
typedef gchar* (*GetPassFunc) (TnyMsgAccountIface *self);

G_END_DECLS

#endif

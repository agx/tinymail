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

typedef struct _TnyMsgIface TnyMsgIface;
typedef struct _TnyMsgIfaceClass TnyMsgIfaceClass;
typedef struct _TnyMsgFolderIface TnyMsgFolderIface;
typedef struct _TnyMsgFolderIfaceClass TnyMsgFolderIfaceClass;
typedef struct _TnyMsgHeaderIface TnyMsgHeaderIface;
typedef struct _TnyMsgHeaderIfaceClass TnyMsgHeaderIfaceClass;
typedef struct _TnyMsgMimePartIface TnyMsgMimePartIface;
typedef struct _TnyMsgMimePartIfaceClass TnyMsgMimePartIfaceClass;
typedef struct _TnyAccountIface TnyAccountIface;
typedef struct _TnyAccountIfaceClass TnyAccountIfaceClass;
typedef struct _TnyAccountStoreIface TnyAccountStoreIface;
typedef struct _TnyStreamIface TnyStreamIface;
typedef struct _TnyStreamIfaceClass TnyStreamIfaceClass;
typedef struct _TnyAccountStoreIfaceClass TnyAccountStoreIfaceClass;
typedef struct _TnyMsgHeaderProxy TnyMsgHeaderProxy;
typedef struct _TnyMsgHeaderProxyClass TnyMsgHeaderProxyClass;
typedef gchar* (*GetPassFunc) (TnyAccountIface *self, const gchar *prompt);
typedef void (*ForgetPassFunc) (TnyAccountIface *self);

G_END_DECLS

#endif

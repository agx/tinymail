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
typedef struct _TnyDeviceIface TnyDeviceIface;
typedef struct _TnyDeviceIfaceClass TnyDeviceIfaceClass;
typedef struct _TnyStoreAccountIface TnyStoreAccountIface;
typedef struct _TnyStoreAccountIfaceClass TnyStoreAccountIfaceClass;
typedef struct _TnyTransportAccountIface TnyTransportAccountIface;
typedef struct _TnyTransportAccountIfaceClass TnyTransportAccountIfaceClass;
typedef struct _TnyAccountStoreIface TnyAccountStoreIface;
typedef struct _TnyStreamIface TnyStreamIface;
typedef struct _TnyStreamIfaceClass TnyStreamIfaceClass;
typedef struct _TnyAccountStoreIfaceClass TnyAccountStoreIfaceClass;
typedef struct _TnyMsgHeaderProxy TnyMsgHeaderProxy;
typedef struct _TnyMsgHeaderProxyClass TnyMsgHeaderProxyClass;
typedef gchar* (*TnyGetPassFunc) (TnyAccountIface *self, const gchar *domain, const gchar *prompt, const gchar *item, gboolean *cancel);
typedef void (*TnyForgetPassFunc) (TnyAccountIface *self, const gchar *domain, const gchar *item);
typedef enum _TnyStoreAccountFolderType TnyStoreAccountFolderType;
typedef void (*TnyGetHeadersCallback) (TnyMsgFolderIface *self, gboolean cancelled, gpointer user_data);
typedef void (*TnyGetHeadersStatusCallback) (TnyMsgFolderIface *self, const gchar *what, gint status, gpointer user_data);
typedef enum _TnyMsgHeaderFlags TnyMsgHeaderFlags;

#ifndef G_LIKELY
#define G_LIKELY(expr) (expr)
#endif
#ifndef G_UNLIKELY
#define G_UNLIKELY(expr) (expr)
#endif

G_END_DECLS

#endif

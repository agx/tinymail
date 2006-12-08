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

typedef struct _TnyAccountStore TnyAccountStore;
typedef struct _TnyAccountStoreIface TnyAccountStoreIface;
typedef struct _TnyList TnyList;
typedef struct _TnyIterator TnyIterator;
typedef struct _TnyListIface TnyListIface;
typedef struct _TnyIteratorIface TnyIteratorIface;
typedef struct _TnyMsg TnyMsg;
typedef struct _TnyMsgIface TnyMsgIface;
typedef struct _TnyFolder TnyFolder;
typedef struct _TnyFolderIface TnyFolderIface;
typedef struct _TnyHeader TnyHeader;
typedef struct _TnyHeaderIface TnyHeaderIface;
typedef struct _TnyMimePart TnyMimePart;
typedef struct _TnyMimePartIface TnyMimePartIface;
typedef struct _TnyAccount TnyAccount;
typedef struct _TnyAccountIface TnyAccountIface;
typedef struct _TnyDevice TnyDevice;
typedef struct _TnyDeviceIface TnyDeviceIface;
typedef struct _TnyStoreAccount TnyStoreAccount;
typedef struct _TnyStoreAccountIface TnyStoreAccountIface;
typedef struct _TnyTransportAccount TnyTransportAccount;
typedef struct _TnyTransportAccountIface TnyTransportAccountIface;
typedef struct _TnyStream TnyStream;
typedef struct _TnyStreamIface TnyStreamIface;
typedef gchar* (*TnyGetPassFunc) (TnyAccount *self, const gchar *prompt, gboolean *cancel);
typedef void (*TnyForgetPassFunc) (TnyAccount *self);
typedef void (*TnyRefreshFolderCallback) (TnyFolder *self, gboolean cancelled, GError **err, gpointer user_data);
typedef void (*TnyGetMsgCallback) (TnyFolder *folder, TnyMsg *msg, GError **err, gpointer user_data);
typedef void (*TnyTransferMsgsCallback) (TnyFolder *folder, GError **err, gpointer user_data);
typedef void (*TnyRefreshFolderStatusCallback) (TnyFolder *self, const gchar *what, gint status, gpointer user_data);
typedef enum _TnyHeaderFlags TnyHeaderFlags;
typedef enum _TnyAlertType TnyAlertType;
typedef enum _TnyFolderType TnyFolderType;
typedef struct _TnySimpleList TnySimpleList;
typedef struct _TnySimpleListClass TnySimpleListClass;
typedef enum _TnyAccountType TnyAccountType;
typedef enum _TnyGetAccountsRequestType TnyGetAccountsRequestType;
typedef struct _TnyFsStream TnyFsStream;
typedef struct _TnyFsStreamClass TnyFsStreamClass;
typedef struct _TnyFolderStore TnyFolderStore;
typedef struct _TnyFolderStoreIface TnyFolderStoreIface;
typedef struct _TnyFolderStoreQuery TnyFolderStoreQuery;
typedef struct _TnyFolderStoreQueryClass TnyFolderStoreQueryClass;
typedef enum _TnyFolderStoreQueryOption TnyFolderStoreQueryOption;
typedef struct _TnyFolderStoreQueryItem TnyFolderStoreQueryItem;
typedef struct _TnyFolderStoreQueryItemClass TnyFolderStoreQueryItemClass;
typedef void (*TnyGetFoldersCallback) (TnyFolderStore *self, TnyList *list, GError **err, gpointer user_data);
typedef enum _TnyFolderSignal TnyFolderSignal;
typedef enum _TnyDeviceSignal TnyDeviceSignal;
typedef enum _TnyAccountStoreSignal TnyAccountStoreSignal;
typedef struct _TnyMsgRemoveStrategy TnyMsgRemoveStrategy;
typedef struct _TnyMsgRemoveStrategyIface TnyMsgRemoveStrategyIface;
typedef struct _TnySendQueue TnySendQueue;
typedef struct _TnySendQueueIface TnySendQueueIface;
typedef enum _TnyError TnyError;
typedef enum _TnyErrorDomain TnyErrorDomain;

G_END_DECLS

#endif

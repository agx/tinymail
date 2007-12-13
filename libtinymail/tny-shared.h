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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <glib.h>
#include <glib-object.h>

#ifdef DEBUG
#define tny_debug g_print
#else
#define tny_debug(o, ...)	
#endif

G_BEGIN_DECLS

/* GTK+ uses G_PRIORITY_HIGH_IDLE + 10 for resizing operations,
 * and G_PRIORITY_HIGH_IDLE + 20 for redrawing operations;
 * this makes sure that status callbacks happen after redraws, so we don't
 * get a lot of notifications but very little visual feedback */
#define TNY_PRIORITY_LOWER_THAN_GTK_REDRAWS G_PRIORITY_HIGH_IDLE + 30

typedef struct _TnyStatus TnyStatus;
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
typedef struct _TnySimpleList TnySimpleList;
typedef struct _TnySimpleListClass TnySimpleListClass;
typedef struct _TnyFsStream TnyFsStream;
typedef struct _TnyFsStreamClass TnyFsStreamClass;
typedef struct _TnyFolderStore TnyFolderStore;
typedef struct _TnyFolderStoreIface TnyFolderStoreIface;
typedef struct _TnyFolderStoreQuery TnyFolderStoreQuery;
typedef struct _TnyFolderStoreQueryClass TnyFolderStoreQueryClass;
typedef struct _TnyFolderStoreQueryItem TnyFolderStoreQueryItem;
typedef struct _TnyFolderStoreQueryItemClass TnyFolderStoreQueryItemClass;
typedef struct _TnyMsgRemoveStrategy TnyMsgRemoveStrategy;
typedef struct _TnyMsgRemoveStrategyIface TnyMsgRemoveStrategyIface;
typedef struct _TnySendQueue TnySendQueue;
typedef struct _TnySendQueueIface TnySendQueueIface;
typedef struct _TnyMsgReceiveStrategy TnyMsgReceiveStrategy;
typedef struct _TnyMsgReceiveStrategyIface TnyMsgReceiveStrategyIface;
typedef struct _TnyPair TnyPair;
typedef struct _TnyPairClass TnyPairClass;
typedef struct _TnyLockable TnyLockable;
typedef struct _TnyLockableIface TnyLockableIface;
typedef struct _TnyNoopLockable TnyNoopLockable;
typedef struct _TnyNoopLockableClass TnyNoopLockableClass;
typedef struct _TnyFolderObserver TnyFolderObserver;
typedef struct _TnyFolderObserverIface TnyFolderObserverIface;
typedef struct _TnyFolderChange TnyFolderChange;
typedef struct _TnyFolderChangeClass TnyFolderChangeClass;
typedef struct _TnyFolderMonitor TnyFolderMonitor;
typedef struct _TnyFolderMonitorClass TnyFolderMonitorClass;
typedef struct _TnyFolderStoreChange TnyFolderStoreChange;
typedef struct _TnyFolderStoreChangeClass TnyFolderStoreChangeClass;
typedef struct _TnyFolderStoreObserver TnyFolderStoreObserver;
typedef struct _TnyFolderStoreObserverIface TnyFolderStoreObserverIface;
typedef struct _TnyFolderStats TnyFolderStats;
typedef struct _TnyFolderStatsClass TnyFolderStatsClass;
typedef struct _TnyPasswordGetter TnyPasswordGetter;
typedef struct _TnyPasswordGetterIface TnyPasswordGetterIface;
typedef struct _TnyCombinedAccount TnyCombinedAccount;
typedef struct _TnyCombinedAccountClass TnyCombinedAccountClass;
typedef struct _TnyConnectionStrategy TnyConnectionStrategy;
typedef struct _TnyConnectionStrategyIface TnyConnectionStrategyIface;

typedef gchar* (*TnyGetPassFunc) (TnyAccount *self, const gchar *prompt, gboolean *cancel);
typedef void (*TnyForgetPassFunc) (TnyAccount *self);
typedef void (*TnyFolderCallback) (TnyFolder *self, gboolean canceled, GError *err, gpointer user_data);
typedef void (*TnyCreateFolderCallback) (TnyFolderStore *self, gboolean canceled, TnyFolder *new_folder, GError *err, gpointer user_data);
typedef void (*TnyMimePartCallback) (TnyMimePart *self, TnyStream *stream, gboolean canceled, GError *err, gpointer user_data);
typedef void (*TnyGetHeadersCallback) (TnyFolder *self, gboolean canceled, TnyList *headers, GError *err, gpointer user_data);
typedef void (*TnyGetMsgCallback) (TnyFolder *folder, gboolean canceled, TnyMsg *msg, GError *err, gpointer user_data);
typedef void (*TnyTransferMsgsCallback) (TnyFolder *folder, gboolean canceled, GError *err, gpointer user_data);
typedef void (*TnyStatusCallback) (GObject *self, TnyStatus *status, gpointer user_data);
typedef void (*TnyGetFoldersCallback) (TnyFolderStore *self, gboolean canceled, TnyList *list, GError *err, gpointer user_data);
typedef void (*TnyCopyFolderCallback) (TnyFolder *self, gboolean canceled, TnyFolderStore *into, TnyFolder *new_folder, GError *err, gpointer user_data);
typedef void (*TnySendQueueAddCallback) (TnySendQueue *self, gboolean canceled, TnyMsg *msg, GError *err, gpointer user_data);

G_END_DECLS

#endif

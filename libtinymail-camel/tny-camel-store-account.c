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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <config.h>
#include <glib/gi18n-lib.h>
#include <glib.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <string.h>

#include <tny-list.h>
#include <tny-account.h>
#include <tny-store-account.h>
#include <tny-camel-store-account.h>
#include <tny-folder-store-change.h>
#include <tny-folder-store-observer.h>
#include <tny-simple-list.h>

#include <tny-folder.h>
#include <tny-folder-store.h>
#include <tny-merge-folder.h>
#include <tny-camel-folder.h>
#include <tny-error.h>

#include <camel/camel.h>
#include <camel/camel-session.h>
#include <camel/camel-store.h>
#include <camel/camel-disco-store.h>

#ifndef CAMEL_FOLDER_TYPE_SENT
#define CAMEL_FOLDER_TYPE_SENT (5 << 10)
#endif

#include <tny-folder.h>
#include <tny-status.h>
#define TINYMAIL_ENABLE_PRIVATE_API
#include "tny-common-priv.h"
#undef TINYMAIL_ENABLE_PRIVATE_API

#include "tny-camel-account-priv.h"
#include "tny-camel-store-account-priv.h"
#include "tny-camel-folder-priv.h"
#include "tny-camel-common-priv.h"

#include <tny-camel-shared.h>
#include <tny-account-store.h>


static GObjectClass *parent_class = NULL;



typedef struct { 
	GObject *self;
	GObject *change; 
} NotFolObInIdleInfo;

static void 
do_notify_in_idle_destroy (gpointer user_data)
{
	NotFolObInIdleInfo *info = (NotFolObInIdleInfo *) user_data;
	g_object_unref (info->self);
	g_object_unref (info->change);
	g_slice_free (NotFolObInIdleInfo, info);
}

static void
notify_folder_store_observers_about (TnyFolderStore *self, TnyFolderStoreChange *change)
{
	TnyCamelAccountPriv *apriv = NULL;
	GList *list = NULL;

	GStaticRecMutex *obs_lock;
	GList *sobs;

	if (TNY_IS_CAMEL_STORE_ACCOUNT (self)) {
		TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);
		apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

		obs_lock = priv->obs_lock;
		sobs = priv->sobs;
	} else if (TNY_IS_CAMEL_FOLDER (self)) {
		TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
		apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (priv->account);

		obs_lock = priv->obs_lock;
		sobs = priv->sobs;
	} else {
		g_return_if_reached ();
	}

	g_static_rec_mutex_lock (obs_lock);
	if (!sobs) {
		g_static_rec_mutex_unlock (obs_lock);
		return;
	}
	list = g_list_copy (sobs);
	g_static_rec_mutex_unlock (obs_lock);

	while (list)
	{
		TnyFolderStoreObserver *observer = TNY_FOLDER_STORE_OBSERVER (list->data);
		tny_lockable_lock (apriv->session->priv->ui_lock);
		tny_folder_store_observer_update (observer, change);
		tny_lockable_unlock (apriv->session->priv->ui_lock);
		list = g_list_next (list);
	}
	g_list_free (list);

	return;
}

static gboolean 
notify_folder_store_observers_about_idle (gpointer user_data)
{
	NotFolObInIdleInfo *info = (NotFolObInIdleInfo *) user_data;
	notify_folder_store_observers_about (TNY_FOLDER_STORE (info->self), 
		TNY_FOLDER_STORE_CHANGE (info->change));
	return FALSE;
}

static void
notify_folder_store_observers_about_in_idle (TnyFolderStore *self, TnyFolderStoreChange *change)
{
	NotFolObInIdleInfo *info = g_slice_new (NotFolObInIdleInfo);
	info->self = g_object_ref (self);
	info->change = g_object_ref (change);
	g_idle_add_full (G_PRIORITY_HIGH, notify_folder_store_observers_about_idle,
		info, do_notify_in_idle_destroy);
}

static gboolean
connection_status_idle (gpointer data)
{
	TnyAccount *self = (TnyAccount *) data;
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	tny_lockable_lock (apriv->session->priv->ui_lock);
	g_signal_emit (G_OBJECT (self), 
		tny_account_signals [TNY_ACCOUNT_CONNECTION_STATUS_CHANGED], 
			0, apriv->status);
	tny_lockable_unlock (apriv->session->priv->ui_lock);

	return FALSE;
}



static void
connection_status_idle_destroy (gpointer data)
{
	g_object_unref (data);
}

static void 
tny_camel_store_account_delete_cache (TnyStoreAccount *self)
{
	TNY_CAMEL_STORE_ACCOUNT_GET_CLASS (self)->delete_cache_func (self);
}

static void 
tny_camel_store_account_delete_cache_default (TnyStoreAccount *self)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	CamelStore *store = CAMEL_STORE (apriv->service);
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	char *str;

	priv->deleted = TRUE;
	priv->cant_reuse_iter = TRUE;
	apriv->delete_this = camel_store_delete_cache (store);

	return;
}

static void
tny_camel_store_account_do_emit (TnyCamelStoreAccount *self)
{
	g_idle_add_full (G_PRIORITY_HIGH, 
				connection_status_idle, 
				g_object_ref (self), 
				connection_status_idle_destroy);
}


static void 
disconnection (CamelService *service, gboolean suc, TnyAccount *self)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);

	priv->cant_reuse_iter = TRUE;

	if (!service->reconnecting) 
	{

		/* If we are not reconnecting and we got disconnected ... */

		gboolean emit = FALSE;

		if (apriv->status != TNY_CONNECTION_STATUS_DISCONNECTED)
			emit = TRUE;
		apriv->status = TNY_CONNECTION_STATUS_DISCONNECTED;

		if (emit)
			tny_camel_store_account_do_emit (TNY_CAMEL_STORE_ACCOUNT (self));
	}

	/* The failure of a reconnect is handled in the connection handler */


#ifdef DEBUG
	g_print ("TNY_DEBUG: %s disconnected %s\n", 
		tny_account_get_name (self), 
		service->reconnecting?"reconnecting":"");
#endif
}


static void 
connection (CamelService *service, gboolean suc, TnyAccount *self)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);
	gboolean emit = FALSE;

	priv->cant_reuse_iter = TRUE;

	if (!service->reconnecting)
	{
		/* If we are not reconnecting, but normally connecting */

		if (suc) {
			if (apriv->status != TNY_CONNECTION_STATUS_CONNECTED)
				emit = TRUE;
			apriv->status = TNY_CONNECTION_STATUS_CONNECTED;
		} else {
			if (apriv->status != TNY_CONNECTION_STATUS_CONNECTED_BROKEN)
				emit = TRUE;
			apriv->status = TNY_CONNECTION_STATUS_CONNECTED_BROKEN;
		}
	}

	if (service->reconnecting && !suc) 
	{
		/* In case reconnecting failed */

		if (apriv->status != TNY_CONNECTION_STATUS_CONNECTED_BROKEN)
			emit = TRUE;
		apriv->status = TNY_CONNECTION_STATUS_CONNECTED_BROKEN;

#ifdef DEBUG
		g_print ("TNY_DEBUG: %s reconnecting failed\n", 
			tny_account_get_name (self));
#endif

	}


	if (CAMEL_IS_DISCO_STORE (service) && !service->reconnecting)
	{

		if (camel_disco_store_status ((CamelDiscoStore *) service) == CAMEL_DISCO_STORE_ONLINE)
		{

			/* In case the account's status is online and ... */

#ifdef DEBUG
			g_print ("TNY_DEBUG: %s %s %s\n", 
				tny_account_get_name (self), 
				suc?"connected":"failed connecting",
				service->reconnecting?"reconnecting":"");
#endif

			if (suc) {
				/* Connecting succeeded */

				if (apriv->status != TNY_CONNECTION_STATUS_CONNECTED)
					emit = TRUE;
				apriv->status = TNY_CONNECTION_STATUS_CONNECTED;
			} else {
				/* Connecting failed */

				if (apriv->status != TNY_CONNECTION_STATUS_CONNECTED_BROKEN)
					emit = TRUE;
				apriv->status = TNY_CONNECTION_STATUS_CONNECTED_BROKEN;
			}

		} else {

			/* In case the account's status is offline and ... */

#ifdef DEBUG
			g_print ("TNY_DEBUG: %s %s\n", 
				tny_account_get_name (self),
				suc?"ready for offline operation":
					"not ready for offline operation");
#endif


			if (suc) {
				/* Preparing the offline cache succeeded */
				emit = FALSE; /* Let disconnection handle it */
				/*if (apriv->status != TNY_CONNECTION_STATUS_DISCONNECTED)
					emit = TRUE;
				apriv->status = TNY_CONNECTION_STATUS_DISCONNECTED;*/
			} else {

				/* Preparing the offline cache failed */

				if (apriv->status != TNY_CONNECTION_STATUS_DISCONNECTED_BROKEN)
					emit = TRUE;
				apriv->status = TNY_CONNECTION_STATUS_DISCONNECTED_BROKEN;
			}
		}

	}

	if (emit)
		tny_camel_store_account_do_emit (TNY_CAMEL_STORE_ACCOUNT (self));

}

void 
_tny_camel_store_account_emit_conchg_signal (TnyCamelStoreAccount *self)
{
	/* tny_camel_store_account_do_emit (self); */
	return;
}

static void 
reconnection (CamelService *service, gboolean suc, TnyAccount *self)
{
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);

	priv->cant_reuse_iter = TRUE;

#ifdef DEBUG
	g_print ("TNY_DEBUG: %s reconnecting %s\n", 
		tny_account_get_name (self), 
		suc?"succeeded":"failed");
#endif

}

static void 
reconnecting (CamelService *service, gboolean suc, TnyAccount *self)
{
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);

	priv->cant_reuse_iter = TRUE;

#ifdef DEBUG
	g_print ("TNY_DEBUG: %s reconnecting\n", 
		tny_account_get_name (self));
#endif

}


static void 
tny_camel_store_account_prepare (TnyCamelAccount *self, gboolean recon_if, gboolean reservice)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);

	priv->cant_reuse_iter = TRUE;

	_tny_camel_account_refresh (self, recon_if);

	g_static_rec_mutex_lock (apriv->service_lock);

	if (apriv->session && apriv->url_string)
	{
	  if (camel_exception_is_set (apriv->ex))
		camel_exception_clear (apriv->ex);

	  if (!apriv->service && reservice)
	  {
		if (apriv->service && CAMEL_IS_SERVICE (apriv->service))
		{
			camel_object_unref (CAMEL_OBJECT (apriv->service));
			apriv->service = NULL;
		} 

		apriv->service = camel_session_get_service
			((CamelSession*) apriv->session, apriv->url_string, 
			apriv->type, apriv->ex);

		if (apriv->service && !camel_exception_is_set (apriv->ex)) 
		{
			apriv->service->data = self;
			apriv->service->connecting = (con_op) connection;
			apriv->service->disconnecting = (con_op) disconnection;
			apriv->service->reconnecter = (con_op) reconnecting;
			apriv->service->reconnection = (con_op) reconnection;
	

		} else if (camel_exception_is_set (apriv->ex) && apriv->service)
		{
			g_warning ("Must cleanup service pointer\n");
			apriv->service = NULL;
		}
	  }
	} else {
		camel_exception_set (apriv->ex, CAMEL_EXCEPTION_SYSTEM,
			"Session not yet set, use tny_camel_account_set_session");
	}

	g_static_rec_mutex_unlock (apriv->service_lock);
}

static void 
tny_camel_store_account_try_connect (TnyAccount *self, GError **err)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);

	priv->cant_reuse_iter = TRUE;

	if (!apriv->url_string || !apriv->service || !CAMEL_IS_SERVICE (apriv->service))
	{
		if (camel_exception_is_set (apriv->ex))
		{
			g_set_error (err, TNY_ACCOUNT_ERROR, 
				TNY_ACCOUNT_ERROR_TRY_CONNECT,
				camel_exception_get_description (apriv->ex));
			camel_exception_clear (apriv->ex);
		} else {
			g_set_error (err, TNY_ACCOUNT_ERROR, 
				TNY_ACCOUNT_ERROR_TRY_CONNECT,
				"Account not yet fully configured. "
				"This problem indicates a bug in the software.");
		}

		return;
	}

	if (apriv->pass_func_set && apriv->forget_pass_func_set)
	{
		CamelException ex = CAMEL_EXCEPTION_INITIALISER;

		if (camel_exception_is_set (apriv->ex))
			camel_exception_clear (apriv->ex);

		g_static_rec_mutex_lock (apriv->service_lock);

		/* camel_service_connect can launch GUI things */
		if (!camel_service_connect (apriv->service, &ex))
		{
			if (camel_exception_is_set (&ex))
			{
				g_set_error (err, TNY_ACCOUNT_ERROR, 
					TNY_ACCOUNT_ERROR_TRY_CONNECT,
					camel_exception_get_description (&ex));
				camel_exception_clear (&ex);
			} else {
				g_set_error (err, TNY_ACCOUNT_ERROR, 
					TNY_ACCOUNT_ERROR_TRY_CONNECT,
					"Unknown error while connecting");
			}
		} else {
			tny_camel_store_account_do_emit (TNY_CAMEL_STORE_ACCOUNT (self));
		}

		g_static_rec_mutex_unlock (apriv->service_lock);

	} else {
			g_set_error (err, TNY_ACCOUNT_ERROR, 
				TNY_ACCOUNT_ERROR_TRY_CONNECT,
				"Get and Forget password functions not yet set. "
				"This problem indicates a bug in the software.");
	}

	return;
}

/* This utility function performs folder subscriptions/unsubscriptions
 * since the code for both operations is almost the same. If the
 * subscribe parameter is TRUE, then we're asking for a folder
 * subscription, else for a folder unsubscription */

typedef struct {
	TnySessionCamel *session;
	GObject *self, *folder;
} SetSubsInfo;

static gboolean 
set_subscription_idle (gpointer user_data)
{
	SetSubsInfo *info = (SetSubsInfo *) user_data;

	tny_lockable_lock (info->session->priv->ui_lock);
	g_signal_emit (info->self, 
		tny_store_account_signals [TNY_STORE_ACCOUNT_SUBSCRIPTION_CHANGED], 
		0, info->folder);
	tny_lockable_unlock (info->session->priv->ui_lock);

	return FALSE;
}

static void
set_subscription_destroy (gpointer user_data)
{
	SetSubsInfo *info = (SetSubsInfo *) user_data;
	g_object_unref (info->self);
	g_object_unref (info->folder);
	camel_object_unref (info->session);
	g_slice_free (SetSubsInfo, info);
	return;
}

static void
set_subscription (TnyStoreAccount *self, TnyFolder *folder, gboolean subscribe)
{
	TnyCamelAccountPriv *apriv;
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	CamelStore *store;
	CamelFolder *cfolder;
	const gchar *folder_full_name;
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);

	priv->cant_reuse_iter = TRUE;

	apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	if (apriv->service == NULL || !CAMEL_IS_SERVICE (apriv->service))
	{
		g_error ("Account not ready in set_subscription\n");
		return;
	}

	store = CAMEL_STORE (apriv->service);

	if (!camel_store_supports_subscriptions (store)) 
		goto cleanup;

	/* Retrieve the folder full name */
	cfolder = _tny_camel_folder_get_folder (TNY_CAMEL_FOLDER (folder));
	folder_full_name = camel_folder_get_full_name (cfolder);

	if (folder_full_name == NULL)
		goto cleanup;

	if (camel_store_folder_subscribed (store, folder_full_name) == subscribe)
		goto cleanup;

	/* Subscribe or unsubscribe */
	if (subscribe)
		camel_store_subscribe_folder (store, folder_full_name, &ex);
	else
		camel_store_unsubscribe_folder (store, folder_full_name, &ex);

	if (camel_exception_is_set (&ex)) 
	{
		g_warning ("Could not %s folder %s: %s\n",
			   subscribe ? "subscribe to" : "unsubscribe",
			   tny_folder_get_name (folder), 
			   camel_exception_get_description (&ex));
		camel_exception_clear (&ex);
	} else 
	{
		SetSubsInfo *info = g_slice_new (SetSubsInfo);

		/* Sync */
		_tny_camel_folder_set_subscribed (TNY_CAMEL_FOLDER (folder), subscribe);

		info->session = apriv->session;
		camel_object_ref (info->session);
		info->self = g_object_ref (self);
		info->folder = g_object_ref (folder);

		g_idle_add_full (G_PRIORITY_DEFAULT, set_subscription_idle, info,
			set_subscription_destroy);

	}
	camel_object_unref (cfolder);

cleanup:

	return;
}

static void
tny_camel_store_account_subscribe (TnyStoreAccount *self, TnyFolder *folder)
{
	g_assert (TNY_IS_CAMEL_FOLDER (folder));

	set_subscription (self, folder, TRUE);
}

static void
tny_camel_store_account_unsubscribe (TnyStoreAccount *self, TnyFolder *folder)
{
	g_assert (TNY_IS_CAMEL_FOLDER (folder));

	set_subscription (self, folder, FALSE);
}


/**
 * tny_camel_store_account_new:
 * 
 * Create a new #TnyStoreAccount instance implemented for Camel
 *
 * Return value: A new #TnyStoreAccount instance implemented for Camel
 **/
TnyStoreAccount*
tny_camel_store_account_new (void)
{
	TnyCamelStoreAccount *self = g_object_new (TNY_TYPE_CAMEL_STORE_ACCOUNT, NULL);

	return TNY_STORE_ACCOUNT (self);
}

static void
tny_camel_store_account_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyCamelStoreAccount *self = (TnyCamelStoreAccount *)instance;
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	priv->deleted = FALSE;
	priv->queue = _tny_camel_queue_new (self);
	priv->msg_queue = _tny_camel_queue_new (self);
	apriv->type = CAMEL_PROVIDER_STORE;
	apriv->account_type = TNY_ACCOUNT_TYPE_STORE;
	priv->managed_folders = NULL;
	priv->sobs = NULL;
	priv->iter = NULL;
	priv->cant_reuse_iter = TRUE;
	priv->factory_lock = g_new0 (GStaticRecMutex, 1);
	g_static_rec_mutex_init (priv->factory_lock);
	priv->obs_lock = g_new0 (GStaticRecMutex, 1);
	g_static_rec_mutex_init (priv->obs_lock);

	return;
}

static void
foreach_managed_folder (gpointer data, gpointer user_data)
{
	if (data && TNY_IS_CAMEL_FOLDER (data))
	{
		TnyCamelFolder *folder = (TnyCamelFolder*) data;

		TNY_CAMEL_FOLDER_GET_PRIVATE (folder)->iter = NULL;
		TNY_CAMEL_FOLDER_GET_PRIVATE (folder)->iter_parented = FALSE;
	}

	return;
}

static void 
notify_store_observer_del (gpointer user_data, GObject *observer)
{
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (user_data);
	g_static_rec_mutex_lock (priv->obs_lock);
	priv->sobs = g_list_remove (priv->sobs, observer);
	g_static_rec_mutex_unlock (priv->obs_lock);
}


static void
tny_camel_store_account_finalize (GObject *object)
{
	TnyCamelStoreAccount *self = (TnyCamelStoreAccount *)object;
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);

	if (priv->sobs) {
		GList *copy = priv->sobs;
		while (copy) {
			g_object_weak_unref ((GObject *) copy->data, notify_store_observer_del, self);
			copy = g_list_next (copy);
		}
		g_list_free (priv->sobs);
		priv->sobs = NULL;
	}

	g_list_foreach (priv->managed_folders, foreach_managed_folder, self);
	g_list_free (priv->managed_folders);
	priv->managed_folders = NULL;

	if (priv->iter_store && CAMEL_IS_STORE (priv->iter_store))
	{
		camel_store_free_folder_info (priv->iter_store, priv->iter);
		camel_object_unref (CAMEL_OBJECT (priv->iter_store));
	}

	/* g_static_rec_mutex_free (priv->factory_lock); */
	g_free (priv->factory_lock);
	priv->factory_lock = NULL;

	/* g_static_rec_mutex_free (priv->obs_lock); */
	g_free (priv->obs_lock);
	priv->obs_lock = NULL;

	g_object_unref (priv->queue);
	g_object_unref (priv->msg_queue);

	(*parent_class->finalize) (object);

	return;
}

static void 
tny_camel_store_account_remove_folder (TnyFolderStore *self, TnyFolder *folder, GError **err)
{
	TNY_CAMEL_STORE_ACCOUNT_GET_CLASS (self)->remove_folder_func (self, folder, err);
}

static void 
tny_camel_store_account_remove_folder_actual (TnyFolderStore *self, TnyFolder *folder, TnyFolderStoreChange *change, GError **err)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	TnyCamelStoreAccountPriv *aspriv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	TnyCamelFolder *cfol = TNY_CAMEL_FOLDER (folder);
	TnyCamelFolderPriv *cpriv = TNY_CAMEL_FOLDER_GET_PRIVATE (cfol);
	CamelStore *store;
	CamelException subex = CAMEL_EXCEPTION_INITIALISER;

	g_assert (TNY_IS_CAMEL_FOLDER (folder));

	store = CAMEL_STORE (apriv->service);

	if (camel_exception_is_set (&ex)) 
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_REMOVE_FOLDER,
				camel_exception_get_description (&ex));
		camel_exception_clear (&ex);
		_tny_session_stop_operation (apriv->session);
		return;
	}

	g_assert (CAMEL_IS_STORE (store));

	g_static_rec_mutex_lock (aspriv->factory_lock);

	if (!cpriv->folder_name)
	{
		g_warning ("Trying to remove an invalid folder\n");
		g_static_rec_mutex_unlock (aspriv->factory_lock);
		return;
	}

	/* g_static_rec_mutex_lock (cpriv->obs_lock); */
	_tny_camel_folder_freeup_observers (cfol, cpriv);
	/* g_static_rec_mutex_unlock (cpriv->obs_lock); */

	if (camel_store_supports_subscriptions (store))
		camel_store_subscribe_folder (store, cpriv->folder_name, &subex);

	camel_store_delete_folder (store, cpriv->folder_name, &ex);

	if (camel_exception_is_set (&ex)) 
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_REMOVE_FOLDER,
				camel_exception_get_description (&ex));
		camel_exception_clear (&ex);
	} else 
	{
		if (aspriv->iter)
		{
			/* Known memleak
			camel_store_free_folder_info (aspriv->iter_store, aspriv->iter); */
			aspriv->iter = NULL;
		}

		if (camel_store_supports_subscriptions (store))
			camel_store_unsubscribe_folder (store, cpriv->folder_name, &subex);
		tny_folder_store_change_add_removed_folder (change, folder);

		/* g_free (cpriv->folder_name); 
		cpriv->folder_name = NULL; */

		aspriv->managed_folders = 
			g_list_remove (aspriv->managed_folders, cfol);

	}


	g_static_rec_mutex_unlock (aspriv->factory_lock);

	return;
}


static GList *
recurse_remove (TnyFolderStore *from, TnyFolder *folder, GList *changes, GError **err)
{
	GStaticRecMutex *lock;
	GError *nerr = NULL;
	TnyFolderStoreChange *change = NULL;

	if (TNY_IS_FOLDER (from)) {
		TnyCamelFolderPriv *fpriv = TNY_CAMEL_FOLDER_GET_PRIVATE (from);
		lock = fpriv->folder_lock;
	} else {
		TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (from);
		lock = apriv->account_lock;
	}
	g_static_rec_mutex_lock (lock);

	if (TNY_IS_FOLDER_STORE (folder))
	{
		TnyList *folders = tny_simple_list_new ();
		TnyIterator *iter;

		tny_folder_store_get_folders (TNY_FOLDER_STORE (folder), 
				folders, NULL, &nerr);

		if (nerr != NULL)
		{
			g_object_unref (folders);
			goto exception;
		}

		iter = tny_list_create_iterator (folders);
		while (!tny_iterator_is_done (iter))
		{
			TnyFolder *cur = TNY_FOLDER (tny_iterator_get_current (iter));

			changes = recurse_remove (TNY_FOLDER_STORE (folder), cur, changes, &nerr);

			if (nerr != NULL)
			{
				g_object_unref (cur);
				g_object_unref (iter);
				g_object_unref (folders);
				goto exception;
			}

			g_object_unref (cur);
			tny_iterator_next (iter);
		}
		g_object_unref (iter);
		g_object_unref (folders);
	}

	tny_debug ("tny_folder_store_remove: actual removal of %s\n", 
			tny_folder_get_name (folder));

	change = tny_folder_store_change_new (from);
	if (TNY_IS_ACCOUNT (from))
		tny_camel_store_account_remove_folder_actual (from, folder, change, &nerr);
	else if (TNY_IS_FOLDER (from))
		_tny_camel_folder_remove_folder_actual (from, folder, change, &nerr);
	changes = g_list_append (changes, change);

exception:

	if (nerr != NULL)
		g_propagate_error (err, nerr);

	g_static_rec_mutex_unlock (lock);

	return changes;
}


static gboolean
notify_folder_store_observers_about_remove_in_idle (gpointer user_data)
{
	GList *list = (GList *) user_data;

	while (list) {
		TnyFolderStoreChange *change = (TnyFolderStoreChange *) list->data;
		TnyFolderStore *store = tny_folder_store_change_get_folder_store (change);
		notify_folder_store_observers_about (store, change);

		g_object_unref (store);
		g_object_unref (change);
		list = g_list_next (list);
	}
	return FALSE;
}

static void
notify_folder_store_observers_about_remove_in_idle_destroy (gpointer user_data)
{
	g_list_free ((GList *)user_data);
}

static void 
tny_camel_store_account_remove_folder_default (TnyFolderStore *self, TnyFolder *folder, GError **err)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	GError *nerr = NULL;
	GList *changes = NULL;

	if (!_tny_session_check_operation (apriv->session, TNY_ACCOUNT (self), err,
			TNY_FOLDER_STORE_ERROR, TNY_FOLDER_STORE_ERROR_REMOVE_FOLDER))
		return;

	if (apriv->service == NULL || !CAMEL_IS_SERVICE (apriv->service))
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_REMOVE_FOLDER,
				"Account not ready for this operation (%s)."
				"This problem indicates a bug in the software.",
				camel_exception_get_description (apriv->ex));
		_tny_session_stop_operation (apriv->session);
		return;
	}

	changes = recurse_remove (self, folder, changes, &nerr);

	if (changes) {
		g_idle_add_full (G_PRIORITY_HIGH, 
			notify_folder_store_observers_about_remove_in_idle, changes,
			notify_folder_store_observers_about_remove_in_idle_destroy);
	}

	if (nerr != NULL)
		g_propagate_error (err, nerr);

	_tny_session_stop_operation (apriv->session);

	return;
}

static TnyFolder*
tny_camel_store_account_create_folder (TnyFolderStore *self, const gchar *name, GError **err)
{
	return TNY_CAMEL_STORE_ACCOUNT_GET_CLASS (self)->create_folder_func (self, name, err);
}


static TnyFolder*
tny_camel_store_account_create_folder_default (TnyFolderStore *self, const gchar *name, GError **err)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);    
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	TnyFolder *folder; CamelFolderInfo *info; CamelStore *store;
	TnyFolderStoreChange *change;
	CamelException subex = CAMEL_EXCEPTION_INITIALISER;
	gboolean was_new = FALSE;

	if (!_tny_session_check_operation (apriv->session, TNY_ACCOUNT (self), err,
			TNY_FOLDER_STORE_ERROR, TNY_FOLDER_STORE_ERROR_CREATE_FOLDER))
		return NULL;

	if (!name || strlen (name) <= 0)
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_CREATE_FOLDER,
				"Failed to create folder with no name");
		_tny_session_stop_operation (apriv->session);
		return NULL;
	}

	g_assert (CAMEL_IS_SESSION (apriv->session));

	if (apriv->service == NULL || !CAMEL_IS_SERVICE (apriv->service))
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_CREATE_FOLDER,
				"Account not ready for this operation (%s). "
				"This problem indicates a bug in the softare.",
				camel_exception_get_description (apriv->ex));
		_tny_session_stop_operation (apriv->session);
		return NULL;
	}

	store = CAMEL_STORE (apriv->service);

	if (camel_exception_is_set (&ex)) 
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_CREATE_FOLDER,
				camel_exception_get_description (&ex));
		camel_exception_clear (&ex);
		_tny_session_stop_operation (apriv->session);
		return NULL;
	}

	g_assert (CAMEL_IS_STORE (store));

	info = camel_store_create_folder (store, NULL, name, &ex);

	if (camel_exception_is_set (&ex)) 
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_CREATE_FOLDER,
				camel_exception_get_description (&ex));
		camel_exception_clear (&ex);

		if (CAMEL_IS_OBJECT (store))
		{
			if (info && CAMEL_IS_STORE (store))
				camel_store_free_folder_info (store, info);
		}
		_tny_session_stop_operation (apriv->session);
		return NULL;
	}

	g_assert (info != NULL);

	if (camel_store_supports_subscriptions (store))
		camel_store_subscribe_folder (store, info->full_name, &subex);

	folder = tny_camel_store_account_factor_folder  
		(TNY_CAMEL_STORE_ACCOUNT (self), info->full_name, &was_new);

	_tny_camel_folder_set_folder_info (self, TNY_CAMEL_FOLDER (folder), info);
	_tny_camel_folder_set_parent (TNY_CAMEL_FOLDER (folder), self);

	/* So that the next call to get_folders() includes the newly-created
	 * folder. */
	priv->cant_reuse_iter = TRUE;

	change = tny_folder_store_change_new (self);
	tny_folder_store_change_add_created_folder (change, folder);
	notify_folder_store_observers_about_in_idle (self, change);
	g_object_unref (change);

	_tny_session_stop_operation (apriv->session);

	return folder;
}




static void
tny_camel_store_account_get_folders (TnyFolderStore *self, TnyList *list, TnyFolderStoreQuery *query, GError **err)
{
	TNY_CAMEL_STORE_ACCOUNT_GET_CLASS (self)->get_folders_func (self, list, query, err);
}

/**
 * tny_camel_store_account_factor_folder:
 * @self: a valid #TnyCamelStoreAccount instance
 * @full_name: the name of the folder to create
 * @was_new: whether or not a new instance got created (by reference)
 * 
 * Factor a new TnyFolder instance. It's possible that an existing one is reused
 * In that case will a reference be added to the instance. If an existing
 * instance was not available, a new one will be created and remembered for in 
 * case a new request happens.
 * 
 * Return value: A #TnyCamelTransportAccount instance or NULL
 **/
TnyFolder *
tny_camel_store_account_factor_folder (TnyCamelStoreAccount *self, const gchar *full_name, gboolean *was_new)
{
	return TNY_CAMEL_STORE_ACCOUNT_GET_CLASS (self)->factor_folder_func (self, full_name, was_new);
}

static TnyFolder * 
tny_camel_store_account_factor_folder_default (TnyCamelStoreAccount *self, const gchar *full_name, gboolean *was_new)
{
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);
	TnyCamelFolder *folder = NULL;

	GList *copy = NULL;

	g_static_rec_mutex_lock (priv->factory_lock);

	copy = priv->managed_folders;
	while (copy)
	{
		TnyFolder *fnd = (TnyFolder*) copy->data;
		const gchar *name = tny_folder_get_id (fnd);

		if (name && full_name && !strcmp (name, full_name))
		{
			folder = TNY_CAMEL_FOLDER (g_object_ref (G_OBJECT (fnd)));
			*was_new = FALSE;
			break;
		}
		copy = g_list_next (copy);
	}

	if (!folder) {
		folder = TNY_CAMEL_FOLDER (_tny_camel_folder_new ());
		priv->managed_folders = g_list_prepend (priv->managed_folders, folder);

		*was_new = TRUE;
	}

	g_static_rec_mutex_unlock (priv->factory_lock);

	return (TnyFolder *) folder;
}

static void 
tny_camel_store_account_get_folders_default (TnyFolderStore *self, TnyList *list, TnyFolderStoreQuery *query, GError **err)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);    
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;    
	CamelFolderInfo *iter=NULL; guint32 flags; CamelStore *store;

	g_assert (TNY_IS_LIST (list));
	g_assert (CAMEL_IS_SESSION (apriv->session));


	if (!_tny_session_check_operation (apriv->session, TNY_ACCOUNT (self), err, 
			TNY_FOLDER_STORE_ERROR, TNY_FOLDER_STORE_ERROR_GET_FOLDERS))
		return;

	if (query != NULL)
		g_assert (TNY_IS_FOLDER_STORE_QUERY (query));

	if (apriv->service == NULL || !CAMEL_IS_SERVICE (apriv->service))
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_GET_FOLDERS,
				"Account not ready for this operation (%s). "
				"This problem indicates a bug in the software.",
				camel_exception_get_description (apriv->ex));
		_tny_session_stop_operation (apriv->session);
		return;
	}

	store = CAMEL_STORE (apriv->service);

	if (camel_exception_is_set (&ex))
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
			TNY_FOLDER_STORE_ERROR_GET_FOLDERS,
			camel_exception_get_description (&ex));
		camel_exception_clear (&ex);
		_tny_session_stop_operation (apriv->session);
		return;
	}

	g_assert (CAMEL_IS_STORE (store));

	flags = CAMEL_STORE_FOLDER_INFO_FAST | CAMEL_STORE_FOLDER_INFO_NO_VIRTUAL |
		CAMEL_STORE_FOLDER_INFO_RECURSIVE;

	if (!camel_session_is_online ((CamelSession*) apriv->session))
		flags |= CAMEL_STORE_FOLDER_INFO_SUBSCRIBED;

	/*if (!priv->iter)*/

	iter = priv->iter;

	if (!iter || priv->cant_reuse_iter)
		iter = camel_store_get_folder_info (store, "", flags, &ex);

	/*else
		iter = priv->iter;*/

	if (camel_exception_is_set (&ex))
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
			TNY_FOLDER_STORE_ERROR_GET_FOLDERS,
			camel_exception_get_description (&ex));
		camel_exception_clear (&ex);

		_tny_session_stop_operation (apriv->session);

		if (iter == NULL)
			return;
	}

	priv->iter = iter;
	priv->cant_reuse_iter = FALSE;

	camel_object_ref (CAMEL_OBJECT (store));
	priv->iter_store = store;

	if (iter)
	{
	  while (iter)
	  {
		/* Also take a look at camel-maildir-store.c:525 */
		if (!(iter->flags & CAMEL_FOLDER_VIRTUAL) && _tny_folder_store_query_passes (query, iter))
		{
			gboolean was_new = FALSE;

			TnyCamelFolder *folder = (TnyCamelFolder *) tny_camel_store_account_factor_folder (
				TNY_CAMEL_STORE_ACCOUNT (self), 
				iter->full_name, &was_new);

			if (was_new && folder != NULL)
				_tny_camel_folder_set_folder_info (self, folder, iter);

			if (folder != NULL)
			{
				const gchar *name = tny_folder_get_name (TNY_FOLDER(folder));
				/* TNY TODO: Temporary fix for empty root folders */
				if (name && strlen(name) > 0)
					tny_list_prepend (list, G_OBJECT (folder));
				g_object_unref (G_OBJECT (folder));
			}
		}
		iter = iter->next;
	  }
	}

	_tny_session_stop_operation (apriv->session);

	return;
}


typedef struct {
	GError *err;
	TnyFolderStore *self;
	TnyList *list;
	TnyGetFoldersCallback callback;
	TnyFolderStoreQuery *query;
	gpointer user_data;
	guint depth; 
	TnySessionCamel *session;
	gboolean cancelled;

	GCond* condition;
	gboolean had_callback;
	GMutex *mutex;

} GetFoldersInfo;


static void
tny_camel_store_account_get_folders_async_destroyer (gpointer thr_user_data)
{
	GetFoldersInfo *info = thr_user_data;

	/* gidle reference */
	g_object_unref (info->self);
	g_object_unref (info->list);

	if (info->err)
		g_error_free (info->err);

	g_mutex_lock (info->mutex);
	g_cond_broadcast (info->condition);
	info->had_callback = TRUE;
	g_mutex_unlock (info->mutex);

	return;
}

static gboolean
tny_camel_store_account_get_folders_async_callback (gpointer thr_user_data)
{
	GetFoldersInfo *info = thr_user_data;
	if (info->callback) {
		tny_lockable_lock (info->session->priv->ui_lock);
		info->callback (info->self, info->cancelled, info->list, info->err, info->user_data);
		tny_lockable_unlock (info->session->priv->ui_lock);
	}
	return FALSE;
}


/**
 * When using a #GMainLoop this method will execute a callback using
 * g_idle_add_full.  Note that without a #GMainLoop, the callbacks
 * could happen in a worker thread (depends on who call it) at an
 * unknown moment in time (check your locking in this case).
 */
static void
execute_callback (gint depth, 
		  gint priority,
		  GSourceFunc idle_func,
		  gpointer data, 
		  GDestroyNotify destroy_func)
{
	/* if (depth > 0){ */
		g_idle_add_full (priority, idle_func, data, destroy_func);
	/* } else {
		idle_func (data);
		destroy_func (data);
	} */
}


static gpointer 
tny_camel_store_account_get_folders_async_thread (gpointer thr_user_data)
{
	GetFoldersInfo *info = (GetFoldersInfo*) thr_user_data;

	tny_folder_store_get_folders (TNY_FOLDER_STORE (info->self),
		info->list, info->query, &info->err);

	info->cancelled = FALSE;
	if (info->err != NULL) {
		if (camel_strstrcase (info->err->message, "cancel") != NULL)
			info->cancelled = TRUE;
	}

	if (info->query)
		g_object_unref (G_OBJECT (info->query));

	info->mutex = g_mutex_new ();
	info->condition = g_cond_new ();
	info->had_callback = FALSE;

	execute_callback (info->depth, G_PRIORITY_DEFAULT, 
		tny_camel_store_account_get_folders_async_callback, info, 
		tny_camel_store_account_get_folders_async_destroyer);

	/* Wait on the queue for the mainloop callback to be finished */
	g_mutex_lock (info->mutex);
	if (!info->had_callback)
		g_cond_wait (info->condition, info->mutex);
	g_mutex_unlock (info->mutex);

	g_mutex_free (info->mutex);
	g_cond_free (info->condition);

	g_slice_free (GetFoldersInfo, info);

	return NULL;
}

static void
tny_camel_store_account_get_folders_async_cancelled_destroyer (gpointer thr_user_data)
{
	GetFoldersInfo *info = thr_user_data;
	/* gidle references */
	g_object_unref (info->self);
	if (info->query)
		g_object_unref (info->query);
	g_object_unref (info->list);
	if (info->err)
		g_error_free (info->err);
	g_slice_free (GetFoldersInfo, info);
	return;
}

static gboolean
tny_camel_store_account_get_folders_async_cancelled_callback (gpointer thr_user_data)
{
	GetFoldersInfo *info = thr_user_data;
	if (info->callback) {
		tny_lockable_lock (info->session->priv->ui_lock);
		info->callback (info->self, TRUE, info->list, info->err, info->user_data);
		tny_lockable_unlock (info->session->priv->ui_lock);
	}
	return FALSE;
}

static void
tny_camel_store_account_get_folders_async (TnyFolderStore *self, TnyList *list, TnyGetFoldersCallback callback, TnyFolderStoreQuery *query, TnyStatusCallback status_callback, gpointer user_data)
{
	TNY_CAMEL_STORE_ACCOUNT_GET_CLASS (self)->get_folders_async_func (self, list, callback, query, status_callback, user_data);
}

static void 
tny_camel_store_account_get_folders_async_default (TnyFolderStore *self, TnyList *list, TnyGetFoldersCallback callback, TnyFolderStoreQuery *query, TnyStatusCallback status_callback, gpointer user_data)
{
	GetFoldersInfo *info;
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);

	g_assert (TNY_IS_LIST (list));

	/* Idle info for the callbacks */
	info = g_slice_new0 (GetFoldersInfo);
	info->session = apriv->session;
	info->err = NULL;
	info->self = self;
	info->list = list;
	info->callback = callback;
	info->user_data = user_data;
	info->query = query;
	info->depth = g_main_depth ();

	/* thread reference */
	g_object_ref (info->self);
	g_object_ref (info->list); 
	if (info->query)
		g_object_ref (info->query);

	_tny_camel_queue_launch_wflags (priv->queue, 
		tny_camel_store_account_get_folders_async_thread,
		tny_camel_store_account_get_folders_async_cancelled_callback,
		tny_camel_store_account_get_folders_async_cancelled_destroyer, 
		&info->cancelled, info, TNY_CAMEL_QUEUE_NORMAL_ITEM|
			TNY_CAMEL_QUEUE_PRIORITY_ITEM, __FUNCTION__);

	return;
}


static void
tny_camel_store_account_add_observer (TnyFolderStore *self, TnyFolderStoreObserver *observer)
{
	TNY_CAMEL_STORE_ACCOUNT_GET_CLASS (self)->add_observer_func (self, observer);
}

static void
tny_camel_store_account_add_observer_default (TnyFolderStore *self, TnyFolderStoreObserver *observer)
{
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);

	g_assert (TNY_IS_FOLDER_STORE_OBSERVER (observer));

	g_static_rec_mutex_lock (priv->obs_lock);
	priv->sobs = g_list_prepend (priv->sobs, observer);
	g_object_weak_ref (G_OBJECT (observer), notify_store_observer_del, self);
	g_static_rec_mutex_unlock (priv->obs_lock);

	return;
}



static void
tny_camel_store_account_remove_observer (TnyFolderStore *self, TnyFolderStoreObserver *observer)
{
	TNY_CAMEL_STORE_ACCOUNT_GET_CLASS (self)->remove_observer_func (self, observer);
}

static void
tny_camel_store_account_remove_observer_default (TnyFolderStore *self, TnyFolderStoreObserver *observer)
{
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);
	GList *found = NULL;

	g_assert (TNY_IS_FOLDER_STORE_OBSERVER (observer));

	g_static_rec_mutex_lock (priv->obs_lock);

	if (!priv->sobs) {
		g_static_rec_mutex_unlock (priv->obs_lock);
		return;
	}

	found = g_list_find (priv->sobs, observer);
	if (found) {
		priv->sobs = g_list_remove_link (priv->sobs, found);
		g_object_weak_unref (found->data, notify_store_observer_del, self);
		g_list_free (found);
	}

	g_static_rec_mutex_unlock (priv->obs_lock);

	return;
}

static TnyFolder*
tny_camel_store_account_find_folder (TnyStoreAccount *self, const gchar *url_string, GError **err)
{
	return TNY_CAMEL_STORE_ACCOUNT_GET_CLASS (self)->find_folder_func (self, url_string, err);
}


static int
hex_to_int (gchar c)
{
	return  c >= '0' && c <= '9' ? c - '0'
		: c >= 'A' && c <= 'F' ? c - 'A' + 10
		: c >= 'a' && c <= 'f' ? c - 'a' + 10
		: -1;
}

static int
unescape_character (const char *scanner)
{
	int first_digit;
	int second_digit;

	first_digit = hex_to_int (*scanner++);
	if (first_digit < 0) {
		return -1;
	}

	second_digit = hex_to_int (*scanner++);
	if (second_digit < 0) {
		return -1;
	}

	return (first_digit << 4) | second_digit;
}


static char *
unescape_string (const gchar *escaped_string,
		const gchar *illegal_characters)
{
	const gchar *in;
	gchar *out, *result;
	gint character;

	if (escaped_string == NULL) {
		return NULL;
	}

	result = g_malloc (strlen (escaped_string) + 1);

	out = result;
	for (in = escaped_string; *in != '\0'; in++) {
		character = *in;
		if (*in == '%') {
			character = unescape_character (in + 1);

			/* Check for an illegal character. We consider '\0' illegal here. */
			if (character <= 0
			    || (illegal_characters != NULL
				&& strchr (illegal_characters, (char)character) != NULL)) {
				g_free (result);
				return NULL;
			}
			in += 2;
		}
		*out++ = (char)character;
	}

	*out = '\0';
	g_assert (out - result <= strlen (escaped_string));
	return result;

}

static TnyFolder*
tny_camel_store_account_find_folder_default (TnyStoreAccount *self, const gchar *url_string_in, GError **err)
{
	TnyFolder *retval = NULL;
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;    
	CamelFolderInfo *iter = NULL; guint32 flags; CamelStore *store;
	char *str = NULL, *nnstr = NULL;
	char *url_string = NULL;

	g_assert (CAMEL_IS_SESSION (apriv->session));

	if (!_tny_session_check_operation (apriv->session, TNY_ACCOUNT (self), err, 
			TNY_FOLDER_STORE_ERROR, TNY_FOLDER_STORE_ERROR_GET_FOLDERS))
		return NULL;

	if (apriv->service == NULL || !CAMEL_IS_SERVICE (apriv->service))
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_GET_FOLDERS,
				"Account not ready for this operation (%s)."
				"This problem indicates a bug in the software.",
				camel_exception_get_description (apriv->ex));
		_tny_session_stop_operation (apriv->session);
		return NULL;
	}

 	store = CAMEL_STORE (apriv->service);

	if (camel_exception_is_set (&ex))
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
			TNY_FOLDER_STORE_ERROR_GET_FOLDERS,
			camel_exception_get_description (&ex));
		camel_exception_clear (&ex);
		_tny_session_stop_operation (apriv->session);
		return NULL;
	}

	g_assert (CAMEL_IS_STORE (store));

	flags = CAMEL_STORE_FOLDER_INFO_FAST | CAMEL_STORE_FOLDER_INFO_NO_VIRTUAL;

	if (!camel_session_is_online ((CamelSession*) apriv->session))
		flags |= CAMEL_STORE_FOLDER_INFO_SUBSCRIBED;

	url_string = g_strdup (url_string_in);

	if (camel_strstrcase (url_string, "maildir"))
	{
		str = strchr (url_string, '#');
		if (str && strlen (str) > 1)
		{
			char *nstr;

			nnstr = unescape_string (str, NULL);
			if (nnstr && strlen (nnstr) > 1)
			{
				nnstr++;
				nstr = strchr (nnstr, '/');
				if (nstr)
					*nstr = '\0';
			}
			str = nnstr;
		} else 
			str = NULL;
	} else {
		str = strchr (url_string, '/');

		if (str && strlen (str) > 1) {
			str++;
			str = strchr (str, '/');
		} else 
			str = NULL;

		if (str && strlen (str) > 1) {
			str++;
			str = strchr (str, '/');
		} else
			str = NULL;

		if (str && strlen (str) > 1) {
			char *nstr;
			str++;
			nstr = strchr (str, '/');
			if (nstr)
				*nstr = '\0';
		} else 
			str = NULL;
	}

	if (str) 
	{
		iter = camel_store_get_folder_info (store, str, flags, &ex);

		if (nnstr) {
			/* g_free (nnstr); known leak */
			nnstr = NULL;
		}
	} else {
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
			TNY_FOLDER_STORE_ERROR_GET_FOLDERS,
			"Invalid URL string");
		_tny_session_stop_operation (apriv->session);

		if (nnstr) {
			/* g_free (nnstr); known leak */
			nnstr = NULL;
		}

		g_free (url_string);
		return NULL;
	}

	if (camel_exception_is_set (&ex))
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
			TNY_FOLDER_STORE_ERROR_GET_FOLDERS,
			camel_exception_get_description (&ex));
		camel_exception_clear (&ex);

		if (CAMEL_IS_OBJECT (store))
		{
			if (iter && CAMEL_IS_STORE (store))
				camel_store_free_folder_info (store, iter);
		}
		_tny_session_stop_operation (apriv->session);
		g_free (url_string);
		return NULL;
	}

	if (iter)
	{
		gboolean was_new = FALSE;

		TnyCamelFolder *folder = (TnyCamelFolder *) tny_camel_store_account_factor_folder (
			TNY_CAMEL_STORE_ACCOUNT (self), 
			iter->full_name, &was_new);

		if (was_new && folder != NULL)
			_tny_camel_folder_set_folder_info (TNY_FOLDER_STORE (self), folder, iter);

		retval = (TnyFolder *) folder;
	}

	_tny_session_stop_operation (apriv->session);

	g_free (url_string);

	return retval;
}



typedef struct {
	TnySessionCamel *session;
	TnyCamelStoreAccount *self;
	gboolean online;
	go_online_callback_func done_func;
	gpointer user_data;
} GoingOnlineInfo;

static gpointer 
tny_camel_store_account_queue_going_online_thread (gpointer thr_user_data)
{
	GoingOnlineInfo *info = (GoingOnlineInfo*) thr_user_data;
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (info->self);
	GError *err = NULL;

	/* So this happens in the queue thread. There's only one such thread
	 * for each TnyCamelStoreAccount active at any given moment in time. 
	 * This means that operations on the queue, while this is happening,
	 * must wait. */

	/* The info->done_func points to on_account_connect_done in the 
	 * TnySessionCamel implementation. Or to the set_online_done handler.
	 * Go take a look! */

	apriv->is_connecting = TRUE;

	/* This is one of those functions that our hero, Rob, is going to 
	 * refactor/rename to something that does make sense :-). Isn't that
	 * true Rob? */

	g_static_rec_mutex_lock (apriv->service_lock);
	if (apriv->service) {
		CamelException mex = CAMEL_EXCEPTION_INITIALISER;
		camel_service_disconnect (apriv->service, FALSE, &mex);
	}
	g_static_rec_mutex_unlock (apriv->service_lock);

	_tny_camel_account_try_connect (TNY_CAMEL_ACCOUNT (info->self), 
			FALSE, &err);

	if (err) {
		info->done_func (info->session, TNY_CAMEL_ACCOUNT (info->self), err, info->user_data);
		g_error_free (err);
	} else {

		/* This one actually sets the account to online. Although Rob 
		 * is probably going to improve all this. Right *poke*? */

		_tny_camel_account_set_online (TNY_CAMEL_ACCOUNT (info->self), 
			info->online, &err);

		if (err) {
			info->done_func (info->session, 
				TNY_CAMEL_ACCOUNT (info->self), err, info->user_data);
			g_error_free (err);
		} else
			info->done_func (info->session, 
				TNY_CAMEL_ACCOUNT (info->self), NULL, info->user_data);

	}

	apriv->is_connecting = FALSE;

	/* Thread reference */
	camel_object_unref (info->session);
	g_object_unref (info->self);

	return NULL;
}

/* This is that protected implementation that will request to go online, on the
 * queue of this account @self. Each TnyCamelStoreAccount has two queues: One 
 * queue is dedicated to getting messages. The other, the default queue, is
 * dedicated to any other operation. Including getting itself connected with an 
 * actual server (that can be both a POP or an IMAP server, depending on the 
 * url-string and/or proto setting of @self). */

static gboolean 
cancelled_conn (gpointer user_data)
{
	GoingOnlineInfo *info = (GoingOnlineInfo *) user_data;
	GError *err = NULL;

	g_set_error (&err, TNY_ACCOUNT_ERROR, TNY_ACCOUNT_ERROR_TRY_CONNECT_USER_CANCEL, 
		"cancel");

	info->done_func (info->session, 
		TNY_CAMEL_ACCOUNT (info->self), err, info->user_data);
	if (err)
		g_error_free (err);

	return FALSE;
}

static void 
cancelled_conn_destroy (gpointer user_data)
{
	GoingOnlineInfo *info = (GoingOnlineInfo *) user_data;
	g_object_unref (info->self);
	camel_object_unref (info->session);
	g_slice_free (GoingOnlineInfo, info);
	return;
}

void
_tny_camel_store_account_queue_going_online (TnyCamelStoreAccount *self, TnySessionCamel *session, gboolean online, go_online_callback_func done_func, gpointer user_data)
{
	GoingOnlineInfo *info = NULL;
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);

	/* Idle info for the callbacks */
	info = g_slice_new0 (GoingOnlineInfo);

	info->session = session;
	info->self = self;
	info->done_func = done_func;
	info->online = online;
	info->user_data = user_data;

	/* thread reference */
	g_object_ref (info->self);
	camel_object_ref (info->session);


	/* It's indeed a very typical queue operation */

	_tny_camel_queue_cancel_remove_items (priv->queue,
		TNY_CAMEL_QUEUE_RECONNECT_ITEM);

	_tny_camel_queue_launch_wflags (priv->queue, 
		tny_camel_store_account_queue_going_online_thread,
		cancelled_conn, cancelled_conn_destroy, NULL,
		 info, TNY_CAMEL_QUEUE_RECONNECT_ITEM|
			TNY_CAMEL_QUEUE_CANCELLABLE_ITEM,
		__FUNCTION__);

	return;
}

static void
tny_folder_store_init (gpointer g, gpointer iface_data)
{
	TnyFolderStoreIface *klass = (TnyFolderStoreIface *)g;

	klass->remove_folder_func = tny_camel_store_account_remove_folder;
	klass->create_folder_func = tny_camel_store_account_create_folder;
	klass->get_folders_func = tny_camel_store_account_get_folders;
	klass->get_folders_async_func = tny_camel_store_account_get_folders_async;
	klass->add_observer_func = tny_camel_store_account_add_observer;
	klass->remove_observer_func = tny_camel_store_account_remove_observer;

	return;
}

static void
tny_store_account_init (gpointer g, gpointer iface_data)
{
	TnyStoreAccountIface *klass = (TnyStoreAccountIface *)g;

	klass->subscribe_func = tny_camel_store_account_subscribe;
	klass->unsubscribe_func = tny_camel_store_account_unsubscribe;
	klass->find_folder_func = tny_camel_store_account_find_folder;
	klass->delete_cache_func = tny_camel_store_account_delete_cache;

	return;
}


static void 
tny_camel_store_account_class_init (TnyCamelStoreAccountClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_camel_store_account_finalize;

	TNY_CAMEL_ACCOUNT_CLASS (class)->try_connect_func = tny_camel_store_account_try_connect;
	TNY_CAMEL_ACCOUNT_CLASS (class)->prepare_func = tny_camel_store_account_prepare;

	class->get_folders_async_func = tny_camel_store_account_get_folders_async_default;
	class->get_folders_func = tny_camel_store_account_get_folders_default;
	class->create_folder_func = tny_camel_store_account_create_folder_default;
	class->remove_folder_func = tny_camel_store_account_remove_folder_default;
	class->add_observer_func = tny_camel_store_account_add_observer_default;
	class->remove_observer_func = tny_camel_store_account_remove_observer_default;
	class->find_folder_func = tny_camel_store_account_find_folder_default;
	class->delete_cache_func = tny_camel_store_account_delete_cache_default;

	/* Protected default implementation */
	class->factor_folder_func = tny_camel_store_account_factor_folder_default;

	g_type_class_add_private (object_class, sizeof (TnyCamelStoreAccountPriv));

	return;
}

/**
 * tny_camel_store_account_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
GType 
tny_camel_store_account_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY (!_camel_type_init_done))
	{
		if (!g_thread_supported ()) 
			g_thread_init (NULL);

		camel_type_init ();
		_camel_type_init_done = TRUE;
	}

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyCamelStoreAccountClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_camel_store_account_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyCamelStoreAccount),
		  0,      /* n_preallocs */
		  tny_camel_store_account_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_store_account_info = 
		{
		  (GInterfaceInitFunc) tny_store_account_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		static const GInterfaceInfo tny_folder_store_info = 
		{
		  (GInterfaceInitFunc) tny_folder_store_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (TNY_TYPE_CAMEL_ACCOUNT,
			"TnyCamelStoreAccount",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_FOLDER_STORE, 
			&tny_folder_store_info);  

		g_type_add_interface_static (type, TNY_TYPE_STORE_ACCOUNT, 
			&tny_store_account_info);

	}

	return type;
}


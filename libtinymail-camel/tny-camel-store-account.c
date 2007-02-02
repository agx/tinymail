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

#include <config.h>
#include <glib/gi18n-lib.h>
#include <glib.h>
#include <string.h>

#include <tny-list.h>
#include <tny-account.h>
#include <tny-store-account.h>
#include <tny-camel-store-account.h>

#include <tny-folder.h>
#include <tny-folder-store.h>
#include <tny-camel-folder.h>
#include <tny-error.h>

#include <camel/camel.h>
#include <camel/camel-session.h>
#include <camel/camel-store.h>

#ifndef CAMEL_FOLDER_TYPE_SENT
#define CAMEL_FOLDER_TYPE_SENT (5 << 10)
#endif

#include <tny-folder.h>

#include "tny-camel-account-priv.h"
#include "tny-camel-store-account-priv.h"
#include "tny-camel-folder-priv.h"
#include "tny-camel-common-priv.h"

#include <tny-camel-shared.h>
#include <tny-account-store.h>


static GObjectClass *parent_class = NULL;

static void 
tny_camel_store_account_prepare (TnyCamelAccount *self)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	if (!apriv->custom_url_string)
	{
		CamelURL *url = NULL;
		GList *options = apriv->options;
		gchar *proto;

		if (apriv->proto == NULL)
			return;

		proto = g_strdup_printf ("%s://", apriv->proto); 

		if (camel_exception_is_set (apriv->ex))
			camel_exception_clear (apriv->ex);

		url = camel_url_new (proto, apriv->ex);
		g_free (proto);

		if (!url)
			return;

		camel_url_set_protocol (url, apriv->proto); 
		camel_url_set_user (url, apriv->user);
		camel_url_set_host (url, apriv->host);

		if (apriv->port != -1)
			camel_url_set_port (url, (int)apriv->port);

		if (apriv->mech)
			camel_url_set_authmech (url, apriv->mech);

		while (options)
		{
			gchar *ptr, *dup = g_strdup (options->data);
			gchar *option, *value;
			ptr = strchr (dup, '=');
			if (ptr) {
				ptr++;
				value = g_strdup (ptr); ptr--;
				*ptr = '\0'; option = dup;
			} else {
				option = dup;
				value = g_strdup ("1");
			}
			camel_url_set_param (url, option, value);
			g_free (value);
			g_free (dup);
			options = g_list_next (options);
		}

		if (G_LIKELY (apriv->url_string))
			g_free (apriv->url_string);

		apriv->url_string = camel_url_to_string (url, 0);
		camel_url_free (url);
	}

	g_static_rec_mutex_lock (apriv->service_lock);
	if (apriv->session)
	{
		CamelService *oservice = apriv->service;

		if (camel_exception_is_set (apriv->ex))
			camel_exception_clear (apriv->ex);

		if (apriv->service && CAMEL_IS_OBJECT (apriv->service))
			camel_object_unref (CAMEL_OBJECT (apriv->service));

		apriv->service = camel_session_get_service
			((CamelSession*) apriv->session, apriv->url_string, 
			apriv->type, apriv->ex);

		if (apriv->service)
			apriv->service->data = self;

	} else {
		camel_exception_set (apriv->ex, CAMEL_EXCEPTION_SYSTEM,
			_("Session not yet set, use tny_camel_account_set_session"));
	}

	g_static_rec_mutex_unlock (apriv->service_lock);
}

static void 
tny_camel_store_account_try_connect (TnyAccount *self, GError **err)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

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
				_("Account not yet fully configured"));
		}

		return;
	}

	if (apriv->pass_func_set && apriv->forget_pass_func_set)
	{
		CamelException ex = CAMEL_EXCEPTION_INITIALISER;
		apriv->connected = FALSE;

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
					_("Unknown error while connecting"));
			}
		} else {
			apriv->connected = TRUE;
			/* tny_camel_account_set_online (self, apriv->connected); */
		}

		g_static_rec_mutex_unlock (apriv->service_lock);

	} else {
			g_set_error (err, TNY_ACCOUNT_ERROR, 
				TNY_ACCOUNT_ERROR_TRY_CONNECT,
				_("Get and Forget password functions not yet set"));
	}

	return;
}

/* This utility function performs folder subscriptions/unsubscriptions
 * since the code for both operations is almost the same. If the
 * subscribe parameter is TRUE, then we're asking for a folder
 * subscription, else for a folder unsubscription */

static void
set_subscription (TnyStoreAccount *self, TnyFolder *folder, gboolean subscribe)
{
	TnyCamelAccountPriv *apriv;
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	CamelStore *store;
	CamelFolder *cfolder;
	const gchar *folder_full_name;

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
	cfolder = tny_camel_folder_get_folder (TNY_CAMEL_FOLDER (folder));
	folder_full_name = camel_folder_get_full_name (cfolder);

	if (camel_store_folder_subscribed (store, folder_full_name) == subscribe)
		goto cleanup;

	/* Subscribe or unsubscribe */
	if (subscribe)
		camel_store_subscribe_folder (store, folder_full_name, &ex);
	else
		camel_store_unsubscribe_folder (store, folder_full_name, &ex);

	if (camel_exception_is_set (&ex)) 
	{
		g_warning (N_("Could not %s folder %s: %s\n"),
			   subscribe ? _("subscribe to") : _("unsubscribe"),
			   tny_folder_get_name (folder), 
			   camel_exception_get_description (&ex));
		camel_exception_clear (&ex);
	} else {
		/* Sync */
		_tny_camel_folder_set_subscribed (TNY_CAMEL_FOLDER (folder), subscribe);
		
		g_signal_emit (self, 
			       tny_store_account_signals [TNY_STORE_ACCOUNT_SUBSCRIPTION_CHANGED], 
			       0, folder);
	}
	camel_object_unref (CAMEL_OBJECT (cfolder));

cleanup:
    	camel_object_unref (CAMEL_OBJECT (store));
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

	apriv->type = CAMEL_PROVIDER_STORE;
	apriv->connected = FALSE;
	apriv->account_type = TNY_ACCOUNT_TYPE_STORE;
	priv->managed_folders = NULL;

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
tny_camel_store_account_finalize (GObject *object)
{
	TnyCamelStoreAccount *self = (TnyCamelStoreAccount *)object;
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);

	g_list_foreach (priv->managed_folders, foreach_managed_folder, self);
	g_list_free (priv->managed_folders);
	priv->managed_folders = NULL;

	if (priv->iter_store && CAMEL_IS_STORE (priv->iter_store))
	{
		camel_store_free_folder_info (priv->iter_store, priv->iter);
		camel_object_unref (CAMEL_OBJECT (priv->iter_store));
	}

	/* Disco store ? */

	(*parent_class->finalize) (object);

	return;
}

static void 
tny_camel_store_account_remove_folder (TnyFolderStore *self, TnyFolder *folder, GError **err)
{
	TNY_CAMEL_STORE_ACCOUNT_GET_CLASS (self)->remove_folder_func (self, folder, err);
}

static void 
tny_camel_store_account_remove_folder_default (TnyFolderStore *self, TnyFolder *folder, GError **err)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	TnyCamelFolder *cfol = TNY_CAMEL_FOLDER (folder);
	TnyCamelFolderPriv *cpriv = TNY_CAMEL_FOLDER_GET_PRIVATE (cfol);
	CamelStore *store;

	g_assert (TNY_IS_CAMEL_FOLDER (folder));

	if (!_tny_session_check_operation (apriv->session, err, 
			TNY_FOLDER_STORE_ERROR, TNY_FOLDER_STORE_ERROR_REMOVE_FOLDER))
		return;

	if (apriv->service == NULL || !CAMEL_IS_SERVICE (apriv->service))
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_REMOVE_FOLDER,
				_("Account not ready for this operation (%s)"),
				camel_exception_get_description (apriv->ex));
		return;
	}

	store = CAMEL_STORE (apriv->service);

	if (camel_exception_is_set (&ex)) 
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_REMOVE_FOLDER,
				camel_exception_get_description (&ex));
		camel_exception_clear (&ex);

		if (store && CAMEL_IS_OBJECT (store))
				camel_object_unref (CAMEL_OBJECT (store));

		return;
	}

	g_assert (CAMEL_IS_STORE (store));
	g_assert (cpriv->folder_name != NULL);

	/* Unsubscribe : camel should do it by itself but it does not do it */

	if (camel_store_supports_subscriptions (store) &&  
			camel_store_folder_subscribed (store, cpriv->folder_name))
		camel_store_unsubscribe_folder (store, cpriv->folder_name, NULL);

	camel_store_delete_folder (store, cpriv->folder_name, &ex);

	if (camel_exception_is_set (&ex)) 
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_REMOVE_FOLDER,
				camel_exception_get_description (&ex));
		camel_exception_clear (&ex);
	}

	g_free (cpriv->folder_name); 
	cpriv->folder_name = NULL;

	camel_object_unref (CAMEL_OBJECT (store));


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
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	TnyFolder *folder; CamelFolderInfo *info; CamelStore *store;

	if (!_tny_session_check_operation (apriv->session, err, 
			TNY_FOLDER_STORE_ERROR, TNY_FOLDER_STORE_ERROR_CREATE_FOLDER))
		return;

	if (!name || strlen (name) <= 0)
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_CREATE_FOLDER,
				_("Failed to create folder with no name"));

		return NULL;
	}

	g_assert (CAMEL_IS_SESSION (apriv->session));

	if (apriv->service == NULL || !CAMEL_IS_SERVICE (apriv->service))
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_CREATE_FOLDER,
				_("Account not ready for this operation (%s)"),
				camel_exception_get_description (apriv->ex));
		return NULL;
	}

	store = CAMEL_STORE (apriv->service);

	if (camel_exception_is_set (&ex)) 
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_CREATE_FOLDER,
				camel_exception_get_description (&ex));
		camel_exception_clear (&ex);

		if (store && CAMEL_IS_OBJECT (store))
				camel_object_unref (CAMEL_OBJECT (store));

		return NULL;
	}

	g_assert (CAMEL_IS_STORE (store));

	info = camel_store_create_folder (store, "/", name, &ex);

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
			camel_object_unref (CAMEL_OBJECT (store));
		}

		return NULL;
	}

	g_assert (info != NULL);

	folder = tny_camel_folder_new ();
	_tny_camel_folder_set_id (TNY_CAMEL_FOLDER (folder), info->full_name);
	camel_store_free_folder_info (store, info);

	camel_object_unref (CAMEL_OBJECT (store));

	return folder;
}

static void
tny_camel_store_account_get_folders (TnyFolderStore *self, TnyList *list, TnyFolderStoreQuery *query, GError **err)
{
	TNY_CAMEL_STORE_ACCOUNT_GET_CLASS (self)->get_folders_func (self, list, query, err);
}


static void 
tny_camel_store_account_get_folders_default (TnyFolderStore *self, TnyList *list, TnyFolderStoreQuery *query, GError **err)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);    
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;    
	CamelFolderInfo *iter; guint32 flags; CamelStore *store;

	g_assert (TNY_IS_LIST (list));
	g_assert (CAMEL_IS_SESSION (apriv->session));

	if (!_tny_session_check_operation (apriv->session, err, 
			TNY_FOLDER_STORE_ERROR, TNY_FOLDER_STORE_ERROR_GET_FOLDERS))
		return;

	if (query != NULL)
		g_assert (TNY_IS_FOLDER_STORE_QUERY (query));

	if (apriv->service == NULL || !CAMEL_IS_SERVICE (apriv->service))
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_GET_FOLDERS,
				_("Account not ready for this operation (%s)"),
				camel_exception_get_description (apriv->ex));
		return;
	}

	store = CAMEL_STORE (apriv->service);

	if (camel_exception_is_set (&ex))
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
			TNY_FOLDER_STORE_ERROR_GET_FOLDERS,
			camel_exception_get_description (&ex));
		camel_exception_clear (&ex);

		if (store && CAMEL_IS_OBJECT (store))
			camel_object_unref (CAMEL_OBJECT (store));

		return;
	}

	g_assert (CAMEL_IS_STORE (store));

	flags = CAMEL_STORE_FOLDER_INFO_FAST | CAMEL_STORE_FOLDER_INFO_NO_VIRTUAL |
		CAMEL_STORE_FOLDER_INFO_RECURSIVE;

	if (!camel_session_is_online ((CamelSession*) apriv->session))
		flags |= CAMEL_STORE_FOLDER_INFO_SUBSCRIBED;

	iter = camel_store_get_folder_info (store, "", flags, &ex);

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
			camel_object_unref (CAMEL_OBJECT (store));
		}

		return;
	}

	priv->iter = iter;
	priv->iter_store = store;

	if (iter)
	{
	  while (iter)
	  {
		if (_tny_folder_store_query_passes (query, iter))
		{
			TnyCamelFolder *folder = TNY_CAMEL_FOLDER (tny_camel_folder_new ());

			_tny_camel_folder_set_id (folder, iter->full_name);
			_tny_camel_folder_set_folder_type (folder, iter);
			_tny_camel_folder_set_unread_count (folder, iter->unread);
			_tny_camel_folder_set_all_count (folder, iter->total);
			_tny_camel_folder_set_name (folder, iter->name);
			_tny_camel_folder_set_iter (folder, iter);
			priv->managed_folders = g_list_prepend (priv->managed_folders, folder);
			_tny_camel_folder_set_account (folder, TNY_ACCOUNT (self));

			tny_list_prepend (list, G_OBJECT (folder));

			g_object_unref (G_OBJECT (folder));
		}
		iter = iter->next;
	  }
	}
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
} GetFoldersInfo;


static void
tny_camel_store_account_get_folders_async_destroyer (gpointer thr_user_data)
{
	GetFoldersInfo *info = thr_user_data;

	/* gidle reference */
	g_object_unref (G_OBJECT (info->self));
	g_object_unref (G_OBJECT (info->list));

	if (info->err)
		g_error_free (info->err);

	g_slice_free (GetFoldersInfo, info);

	return;
}

static gboolean
tny_camel_store_account_get_folders_async_callback (gpointer thr_user_data)
{
	GetFoldersInfo *info = thr_user_data;

	if (info->callback)
		info->callback (info->self, info->list,&info->err, info->user_data);

	return FALSE;
}

static gpointer 
tny_camel_store_account_get_folders_async_thread (gpointer thr_user_data)
{
	GetFoldersInfo *info = (GetFoldersInfo*) thr_user_data;
	GError *err = NULL;

	tny_folder_store_get_folders (TNY_FOLDER_STORE (info->self),
		info->list, info->query, &err);

	if (err != NULL)
		info->err = g_error_copy ((const GError *) err);
	else
		info->err = NULL;

	if (info->query)
		g_object_unref (G_OBJECT (info->query));

	if (info->callback)
	{
		if (info->depth > 0)
		{
			g_idle_add_full (G_PRIORITY_HIGH, 
				tny_camel_store_account_get_folders_async_callback, 
				info, tny_camel_store_account_get_folders_async_destroyer);
		} else {
			tny_camel_store_account_get_folders_async_callback (info);
			tny_camel_store_account_get_folders_async_destroyer (info);
		}
	} else {
		g_object_unref (G_OBJECT (info->self));
		g_object_unref (G_OBJECT (info->list));
	}


	g_thread_exit (NULL);

	return NULL;
}

static void
tny_camel_store_account_get_folders_async (TnyFolderStore *self, TnyList *list, TnyGetFoldersCallback callback, TnyFolderStoreQuery *query, gpointer user_data)
{
	TNY_CAMEL_STORE_ACCOUNT_GET_CLASS (self)->get_folders_async_func (self, list, callback, query, user_data);
}

static void 
tny_camel_store_account_get_folders_async_default (TnyFolderStore *self, TnyList *list, TnyGetFoldersCallback callback, TnyFolderStoreQuery *query, gpointer user_data)
{
	GetFoldersInfo *info;
	GThread *thread;
	GError *err = NULL;
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	g_assert (TNY_IS_LIST (list));

	if (!_tny_session_check_operation (apriv->session, &err, 
			TNY_FOLDER_STORE_ERROR, TNY_FOLDER_STORE_ERROR_GET_FOLDERS))
	{
		if (callback)
			callback (self, list, &err, user_data);
		g_error_free (err);
		return;
	}

	info = g_slice_new0 (GetFoldersInfo);
	info->err = NULL;
	info->self = self;
	info->list = list;
	info->callback = callback;
	info->user_data = user_data;
	info->query = query;
	info->depth = g_main_depth ();

	/* thread reference */
	g_object_ref (G_OBJECT (info->self));
	g_object_ref (G_OBJECT (info->list)); 
	if (info->query)
		g_object_ref (G_OBJECT (info->query));

	thread = g_thread_create (tny_camel_store_account_get_folders_async_thread,
			info, FALSE, NULL);    

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

	return;
}

static void
tny_store_account_init (gpointer g, gpointer iface_data)
{
	TnyStoreAccountIface *klass = (TnyStoreAccountIface *)g;

	klass->subscribe_func = tny_camel_store_account_subscribe;
	klass->unsubscribe_func = tny_camel_store_account_unsubscribe;

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

	g_type_class_add_private (object_class, sizeof (TnyCamelStoreAccountPriv));

	return;
}

GType 
tny_camel_store_account_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY (!camel_type_init_done))
	{
		if (!g_thread_supported ()) 
			g_thread_init (NULL);

		camel_type_init ();
		camel_type_init_done = TRUE;
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


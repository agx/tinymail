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

/* TODO: Refactor to a TnyIMAPStoreAccount, TnyPOPStoreAccount and 
TnyNNTPStoreAccount. Maybe also add a TnyExchangeStoreAccount? This file can 
stay as an abstract TnyStoreAccount type. */

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
report_error (TnyCamelAccountPriv *priv)
{
	if (G_UNLIKELY (priv->service == NULL))
	{
		g_error (_("Couldn't get service %s: %s\n"), priv->url_string,
			   camel_exception_get_description (priv->ex));
		camel_exception_clear (priv->ex);
		return;
	}
}


static void 
tny_camel_store_account_reconnect (TnyCamelAccount *self)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	if (G_LIKELY (priv->session) && G_UNLIKELY (priv->proto) && 
		G_UNLIKELY (priv->user) && G_UNLIKELY (priv->host))
	{
		if (!priv->url_string)
		{
			CamelURL *url = NULL;
			GList *options = priv->options;

			gchar *proto = g_strdup_printf ("%s://", priv->proto); 

			url = camel_url_new (proto, priv->ex);
			g_free (proto);
		
			camel_url_set_protocol (url, priv->proto); 
			camel_url_set_user (url, priv->user);
			camel_url_set_host (url, priv->host);
		
			while (options)
			{
				gchar *ptr, *dup = g_strdup (options->data);
				gchar *option, *value;

				ptr = strchr (dup, '=');

				if (ptr) 
				{
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

			if (G_LIKELY (priv->url_string))
				g_free (priv->url_string);

			priv->url_string = camel_url_to_string (url, 0);

			camel_url_free (url);

			priv->service = camel_session_get_service
				((CamelSession*) priv->session, priv->url_string, 
				priv->type, priv->ex);

			if (priv->service == NULL)
				report_error (priv);
		}
	} else if (G_LIKELY (priv->session) && (priv->url_string))
	{
		/* un officially supported provider */

		priv->service = camel_session_get_service
			((CamelSession*) priv->session, priv->url_string, 
			priv->type, priv->ex);

		if (priv->service == NULL)
			report_error (priv);
	}

	if ( G_LIKELY (priv->service) && G_UNLIKELY (priv->pass_func_set)
		&& G_UNLIKELY (priv->forget_pass_func_set) )
	{
		priv->connected = FALSE;

		if (!camel_service_connect (priv->service, priv->ex))
		{
			g_warning (_("Not connected with %s: %s\n"), priv->url_string,
				   camel_exception_get_description (priv->ex));
			camel_exception_clear (priv->ex);
			/* camel_service_cancel_connect (priv->service);
			camel_service_disconnect (priv->service, FALSE, priv->ex); */
		} else {
			priv->connected = TRUE;
		}
	}

	return;
}

/**
 * This utility function performs folder subscriptions/unsubscriptions
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

	/* Get store */
	g_static_rec_mutex_lock (apriv->service_lock);
	store = camel_session_get_store ((CamelSession*) apriv->session, 
			apriv->url_string, &ex);
	g_static_rec_mutex_unlock (apriv->service_lock);

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

	if (camel_exception_is_set (&ex)) {
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

	(*parent_class->finalize) (object);

	return;
}

static void 
tny_camel_store_account_remove_folder (TnyFolderStore *self, TnyFolder *folder)
{
	TNY_CAMEL_STORE_ACCOUNT_GET_CLASS (self)->remove_folder_func (self, folder);
}

static void 
tny_camel_store_account_remove_folder_default (TnyFolderStore *self, TnyFolder *folder)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	TnyCamelFolder *cfol = TNY_CAMEL_FOLDER (folder);
	TnyCamelFolderPriv *cpriv = TNY_CAMEL_FOLDER_GET_PRIVATE (cfol);
	CamelStore *store;

	g_assert (TNY_IS_CAMEL_FOLDER (folder));

	store = camel_session_get_store ((CamelSession*) apriv->session, 
			apriv->url_string, &ex);

	if (store)
	{
		if (cpriv->folder_name)
		{
			/* Unsubscribe : camel should do it by itself
			   but it does not do it */
			if (camel_store_supports_subscriptions (store) &&
			    camel_store_folder_subscribed (store, cpriv->folder_name))
				camel_store_unsubscribe_folder (store, cpriv->folder_name, NULL);

			camel_store_delete_folder (store, cpriv->folder_name, &ex);
			if (camel_exception_is_set (&ex))
				g_critical ("Could not delete folder %s (%s)\n",
				cpriv->folder_name, camel_exception_get_description (&ex));
			g_free (cpriv->folder_name); cpriv->folder_name = NULL;
		}

		camel_object_unref (CAMEL_OBJECT (store));
	} else 
		g_critical ("Store not available for %s (%s)\n", 
			apriv->url_string, camel_exception_get_description (&ex));

	/* TODO: error handling using 'ex' */

	return;
}

static TnyFolder*
tny_camel_store_account_create_folder (TnyFolderStore *self, const gchar *name)
{
	return TNY_CAMEL_STORE_ACCOUNT_GET_CLASS (self)->create_folder_func (self, name);
}


static TnyFolder*
tny_camel_store_account_create_folder_default (TnyFolderStore *self, const gchar *name)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;    
	TnyFolder *folder = tny_camel_folder_new ();
	CamelFolderInfo *info;
	CamelStore *store;

	store = camel_session_get_store ((CamelSession*) apriv->session, 
			apriv->url_string, &ex);

	if (store)
	{
		info = camel_store_create_folder (store, "/", name, &ex);
		if (info)
		{
			_tny_camel_folder_set_id (TNY_CAMEL_FOLDER (folder), info->full_name);
			camel_store_free_folder_info (store, info);
		} else
			g_critical ("Failed to create folder %s (%s)\n", name,
				camel_exception_get_description (&ex));

		camel_object_unref (CAMEL_OBJECT (store));
	} else 
		g_critical ("Store not available for %s (%s)\n", 
			apriv->url_string, camel_exception_get_description (&ex));

	/* TODO: Error handling using 'ex' */

    	return folder;
}

static void
tny_camel_store_account_get_folders (TnyFolderStore *self, TnyList *list, TnyFolderStoreQuery *query)
{
	TNY_CAMEL_STORE_ACCOUNT_GET_CLASS (self)->get_folders_func (self, list, query);
}


static void 
tny_camel_store_account_get_folders_default (TnyFolderStore *self, TnyList *list, TnyFolderStoreQuery *query)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);    
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;    
	CamelFolderInfo *iter;
	guint32 flags;
	CamelStore *store;

	g_assert (TNY_IS_LIST (list));
	if (query)
		g_assert (TNY_IS_FOLDER_STORE_QUERY (query));

	store = camel_session_get_store ((CamelSession*) apriv->session, 
			apriv->url_string, &ex);

	if (!store)
	{
		g_critical ("Store not available for %s (%s)\n", 
			apriv->url_string, camel_exception_get_description (&ex));

		return;
	}

	flags = CAMEL_STORE_FOLDER_INFO_FAST | CAMEL_STORE_FOLDER_INFO_NO_VIRTUAL |
		CAMEL_STORE_FOLDER_INFO_RECURSIVE;

	if (!camel_session_is_online ((CamelSession*) apriv->session))
		flags |= CAMEL_STORE_FOLDER_INFO_SUBSCRIBED;

	/*_tny_camel_account_start_camel_operation (TNY_CAMEL_ACCOUNT (self), 
					NULL, NULL, NULL); */
	iter = camel_store_get_folder_info (store, "", flags, &ex);
	/*_tny_camel_account_stop_camel_operation (TNY_CAMEL_ACCOUNT (self));*/

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
			_tny_camel_folder_set_account (folder, TNY_STORE_ACCOUNT (self));

			tny_list_prepend (list, G_OBJECT (folder));
		}
		iter = iter->next;
	  }
	}
	return;
}


typedef struct {
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

	g_slice_free (GetFoldersInfo, info);

	return;
}

static gboolean
tny_camel_store_account_get_folders_async_callback (gpointer thr_user_data)
{
	GetFoldersInfo *info = thr_user_data;

	if (info->callback)
		info->callback (info->self, info->list, info->user_data);

	return FALSE;
}

static gpointer 
tny_camel_store_account_get_folders_async_thread (gpointer thr_user_data)
{
	GetFoldersInfo *info = (GetFoldersInfo*) thr_user_data;

	tny_folder_store_get_folders (TNY_FOLDER_STORE (info->self),
		info->list, info->query);

	if (info->query)
		g_object_unref (G_OBJECT (info->query));

	/* thread reference */
	g_object_unref (G_OBJECT (info->self));
	g_object_unref (G_OBJECT (info->list));

	if (info->callback)
	{
		/* gidle reference */
		g_object_ref (G_OBJECT (info->self));
		g_object_ref (G_OBJECT (info->list));

		if (info->depth > 0)
		{
			g_idle_add_full (G_PRIORITY_HIGH, 
				tny_camel_store_account_get_folders_async_callback, 
				info, tny_camel_store_account_get_folders_async_destroyer);
		} else {
			tny_camel_store_account_get_folders_async_callback (info);
			tny_camel_store_account_get_folders_async_destroyer (info);
		}
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

	g_assert (TNY_IS_LIST (list));

	info = g_slice_new0 (GetFoldersInfo);

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

	((TnyCamelAccountClass*)class)->reconnect_func = tny_camel_store_account_reconnect;

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


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

/* TODO: Refactor to a TnyPOPStoreAccount, TnyPOPStoreAccount and 
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
#include <tny-camel-pop-store-account.h>

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
#include "tny-camel-pop-store-account-priv.h"

#include <tny-camel-shared.h>
#include <tny-account-store.h>
#include <tny-error.h>

static GObjectClass *parent_class = NULL;

static TnyFolder*
create_maildir (TnyCamelAccount *self, TnyCamelAccountPriv *apriv, const gchar *name, const gchar *url_string)
{
	CamelStore *store = (CamelStore*) apriv->service;
	CamelSession *session = (CamelSession*) apriv->session;
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	gchar *full_path = (gchar*) url_string;
	CamelStore *mdstore = NULL;

	mdstore = camel_session_get_store(session, full_path, &ex);

	if (!camel_exception_is_set (&ex) && mdstore)
	{
		CamelFolder *cfolder = NULL;

		cfolder = camel_store_get_folder (mdstore, name, CAMEL_STORE_FOLDER_CREATE, &ex);
		if (!camel_exception_is_set (&ex) && cfolder)
		{
			CamelFolderInfo *iter;

			/* camel_object_unref (CAMEL_OBJECT (cfolder)); */

			iter = camel_store_get_folder_info (mdstore, name, 
					CAMEL_STORE_FOLDER_INFO_FAST|CAMEL_STORE_FOLDER_INFO_NO_VIRTUAL,&ex);

			if (!camel_exception_is_set (&ex) && iter)
			{

				/* This MUST be a TnyCamelPOPFolder (as that one overrides some 
				   important methods) */

				TnyCamelFolder *folder = TNY_CAMEL_FOLDER (tny_camel_pop_folder_new ());
				TnyCamelFolderPriv *fpriv = TNY_CAMEL_FOLDER_GET_PRIVATE (folder);

				_tny_camel_folder_set_id (folder, iter->full_name);
				_tny_camel_folder_set_folder_type (folder, iter);
				_tny_camel_folder_set_unread_count (folder, iter->unread);
				_tny_camel_folder_set_all_count (folder, iter->total);
				_tny_camel_folder_set_name (folder, iter->name);
				_tny_camel_folder_set_iter (folder, iter);
				_tny_camel_folder_set_account (folder, TNY_ACCOUNT (self));

				fpriv->store = mdstore;

				g_free (full_path);

				return TNY_FOLDER (folder);

			} else if (iter && CAMEL_IS_STORE (mdstore))
				camel_store_free_folder_info (mdstore, iter);

		} else 
		{
			g_critical (_("Can't create folder \"%s\" in %s"), name, full_path);
			if (cfolder && CAMEL_IS_OBJECT (cfolder))
				camel_object_unref (CAMEL_OBJECT (cfolder));
		}
	} else 
	{
		g_critical (_("Can't create store on %s"), full_path);
		if (store && CAMEL_IS_OBJECT (mdstore))
			camel_object_unref (CAMEL_OBJECT (mdstore));
	}

	return NULL;
}


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
tny_camel_pop_store_account_reconnect (TnyCamelAccount *self)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	TnyCamelPopStoreAccountPriv *priv = TNY_CAMEL_POP_STORE_ACCOUNT_GET_PRIVATE (self);
	CamelSession *session = (CamelSession*) apriv->session;

	g_mutex_lock (priv->lock);

	if (G_LIKELY (apriv->session) && G_UNLIKELY (apriv->user) 
		&& G_UNLIKELY (apriv->host))
	{
		if (!apriv->url_string)
		{
			gchar *name = NULL;

			if (G_LIKELY (apriv->url_string))
				g_free (apriv->url_string);

			name = g_strdup_printf ("%s@%s", apriv->user, apriv->host);
			apriv->url_string = g_strdup_printf ("maildir://%s/mail/pop/%s/maildir",
				session->storage_path, name);
			if (priv->inbox != NULL)
				g_object_unref (G_OBJECT (priv->inbox));
			priv->inbox = create_maildir (self, apriv, name, apriv->url_string);
			g_free (name);

			/* camel_session_get_service can launch GUI things */

			g_static_rec_mutex_lock (apriv->service_lock);
			apriv->service = camel_session_get_service (session, 
				apriv->url_string, apriv->type, apriv->ex);
			if (apriv->service == NULL)
				report_error (apriv);
			g_static_rec_mutex_unlock (apriv->service_lock);

			/* TODO: Handle priv->ex using GError */

		} else if (G_LIKELY (apriv->session) && (apriv->url_string))
		{
			/* camel_session_get_service can launch GUI things */

			g_static_rec_mutex_lock (apriv->service_lock);
			apriv->service = camel_session_get_service (session, 
				apriv->url_string, apriv->type, apriv->ex);
			if (apriv->service == NULL)
				report_error (apriv);
			g_static_rec_mutex_unlock (apriv->service_lock);
			
			/* TODO: Handle priv->ex using GError */
		}
	}

	g_mutex_unlock (priv->lock);

	return;
}

static const gchar* 
tny_camel_pop_store_account_get_url_string (TnyAccount *self)
{
	g_warning ("You can't use the tny_account_get_url_string API on POP accounts");
	return NULL;
}

static void
tny_camel_pop_store_account_set_url_string (TnyAccount *self, const gchar *url_string)
{
	g_warning ("You can't use the tny_account_set_url_string API on POP accounts");
}

static void
tny_camel_pop_store_account_add_option (TnyCamelAccount *self, const gchar *option)
{
	g_warning ("You can't use the tny_camel_account_add_option API on POP accounts");
}

static void 
tny_camel_pop_store_account_get_folders (TnyFolderStore *self, TnyList *list, TnyFolderStoreQuery *query, GError **err)
{
	TnyCamelPopStoreAccountPriv *priv = TNY_CAMEL_POP_STORE_ACCOUNT_GET_PRIVATE (self);

	g_assert (list != NULL);
	g_assert (TNY_IS_LIST (list));

	if (!priv->inbox)
		tny_camel_pop_store_account_reconnect (TNY_CAMEL_ACCOUNT (self));

	if (priv->inbox)
	{
		TnyCamelFolderPriv *fpriv = TNY_CAMEL_FOLDER_GET_PRIVATE (priv->inbox);

		if (!fpriv || !fpriv->iter)
			goto errorh;

		/* There's only one folder, would be silly if the developer filters it
		   away. But the developer being silly doesn't mean that we shouldn't
		   filter it away, in that case. Right? :-) */

		if (_tny_folder_store_query_passes (query, fpriv->iter))
			tny_list_prepend (list, g_object_ref (G_OBJECT (priv->inbox)));
	} else
		goto errorh;

	return;

errorh:

	/* TODO: maybe use the error from the url_string and service getting? */
	g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_GET_FOLDERS,
				"Can't get the INBOX folder");

	return;
}

static void
tny_camel_pop_store_account_get_folders_async (TnyFolderStore *self, TnyList *list, TnyGetFoldersCallback callback, TnyFolderStoreQuery *query, gpointer user_data)
{
	GError *err = NULL;

	/* There's no need to do this really async, it's just setting some 
	   pointers right. 

	   But maybe it could use a gidle in case there's a GMainLoop being used?

	   With using gidle being the point that the callback should happen in the
	   same thread as the caller, while this also causes the callback to happen
	   in that thread, I personally don't see a point in using gidle here.

	   As usual I'm of course interested in other peoples opinion on this. */

	tny_camel_pop_store_account_get_folders (self, list, query, &err);

	/* So, I just call the callback here  ... */

	if (callback)
		callback (self, list, &err, user_data);

	if (err != NULL)
		g_error_free (err);
}



static void 
tny_camel_pop_store_account_remove_folder (TnyFolderStore *self, TnyFolder *folder, GError **err)
{
	g_warning ("You can't use the tny_store_account_remove_folder API on POP accounts");
}

static TnyFolder* 
tny_camel_pop_store_account_create_folder (TnyFolderStore *self, const gchar *name, GError **err)
{
	g_warning ("You can't use the tny_store_account_create_folder API on POP accounts");
}


/**
 * tny_camel_pop_store_account_new:
 * 
 *
 * Return value: A new POP #TnyStoreAccount instance implemented for Camel
 **/
TnyStoreAccount*
tny_camel_pop_store_account_new (void)
{
	TnyCamelPOPStoreAccount *self = g_object_new (TNY_TYPE_CAMEL_POP_STORE_ACCOUNT, NULL);

	return TNY_STORE_ACCOUNT (self);
}

static void
tny_camel_pop_store_account_finalize (GObject *object)
{
    
    	/* The abstract CamelStoreAccount finalizes everything correctly */
    
	(*parent_class->finalize) (object);

	return;
}


static void 
tny_camel_pop_store_account_class_init (TnyCamelPOPStoreAccountClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	TNY_CAMEL_ACCOUNT_CLASS (class)->reconnect_func = tny_camel_pop_store_account_reconnect;
	TNY_CAMEL_ACCOUNT_CLASS (class)->get_url_string_func = tny_camel_pop_store_account_get_url_string;
	TNY_CAMEL_ACCOUNT_CLASS (class)->set_url_string_func = tny_camel_pop_store_account_set_url_string;
	TNY_CAMEL_ACCOUNT_CLASS (class)->add_option_func = tny_camel_pop_store_account_add_option;

	TNY_CAMEL_STORE_ACCOUNT_CLASS (class)->get_folders_async_func = tny_camel_pop_store_account_get_folders_async;
	TNY_CAMEL_STORE_ACCOUNT_CLASS (class)->get_folders_func = tny_camel_pop_store_account_get_folders;
	TNY_CAMEL_STORE_ACCOUNT_CLASS (class)->remove_folder_func = tny_camel_pop_store_account_remove_folder;
	TNY_CAMEL_STORE_ACCOUNT_CLASS (class)->create_folder_func = tny_camel_pop_store_account_create_folder;


   	/* TODO: implement custom get_folders and get_folders_async for POP */
	/* TODO: implement a TnyCamelPOPFolder (that uses the local provider of Camel) */
	/* TODO: implement do-nothing create_folder and remove_folder for POP */
    
	object_class->finalize = tny_camel_pop_store_account_finalize;

	g_type_class_add_private (object_class, sizeof (TnyCamelPopStoreAccountPriv));

	return;
}


static void
tny_camel_pop_store_account_instance_init (GTypeInstance *instance, gpointer g_class)
{
	/* The abstract CamelStoreAccount initializes everything correctly */
    
	return;
}

GType 
tny_camel_pop_store_account_get_type (void)
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
		  sizeof (TnyCamelPOPStoreAccountClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_camel_pop_store_account_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyCamelPOPStoreAccount),
		  0,      /* n_preallocs */
		  tny_camel_pop_store_account_instance_init    /* instance_init */
		};
	    
		type = g_type_register_static (TNY_TYPE_CAMEL_STORE_ACCOUNT,
			"TnyCamelPOPStoreAccount",
			&info, 0);	    
	}

	return type;
}


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

static void 
tny_camel_pop_store_account_set_online_status (TnyCamelAccount *self, gboolean offline, GError **err)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;

	if (!priv->service || !CAMEL_IS_SERVICE (priv->service))
	{
		if (camel_exception_is_set (priv->ex))
		{
			g_set_error (err, TNY_ACCOUNT_ERROR, 
				TNY_ACCOUNT_ERROR_TRY_CONNECT,
				camel_exception_get_description (priv->ex));
		} else {
			g_set_error (err, TNY_ACCOUNT_ERROR, 
				TNY_ACCOUNT_ERROR_TRY_CONNECT,
				_("Account not yet fully configured"));
		}

		return;
	}

	if (offline)
		camel_service_disconnect (priv->service, TRUE, &ex);
	else
		camel_service_connect (priv->service, &ex);

	if (camel_exception_is_set (&ex))
	{
		g_set_error (err, TNY_ACCOUNT_ERROR, 
			TNY_ACCOUNT_ERROR_TRY_CONNECT,
			camel_exception_get_description (&ex));
	}

	return;
}

static void 
tny_camel_pop_store_account_get_folders (TnyFolderStore *self, TnyList *list, TnyFolderStoreQuery *query, GError **err)
{

	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);    
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;    
	CamelStore *store;
	TnyCamelFolder *folder;

	g_assert (TNY_IS_LIST (list));

	if (!_tny_session_check_operation (apriv->session, err, 
			TNY_FOLDER_STORE_ERROR, TNY_FOLDER_STORE_ERROR_GET_FOLDERS))
		return;

	g_assert (CAMEL_IS_STORE (apriv->service));

	store = CAMEL_STORE (apriv->service);

	folder = TNY_CAMEL_FOLDER (tny_camel_pop_folder_new ());

	_tny_camel_folder_set_id (folder, "INBOX");
	/* _tny_camel_folder_set_unread_count (folder, iter->unread);
	_tny_camel_folder_set_all_count (folder, iter->total); */
	_tny_camel_folder_set_name (folder, "Inbox");
	priv->managed_folders = g_list_prepend (priv->managed_folders, folder);
	_tny_camel_folder_set_account (folder, TNY_ACCOUNT (self));
	tny_list_prepend (list, G_OBJECT (folder));

	g_object_unref (G_OBJECT (folder));

	return;
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
	TnyCamelPopStoreAccountPriv *priv = TNY_CAMEL_POP_STORE_ACCOUNT_GET_PRIVATE (object);

	g_mutex_lock (priv->lock);
	if (priv->inbox)
		g_object_unref (G_OBJECT (priv->inbox));
	g_mutex_unlock (priv->lock);

	g_mutex_free (priv->lock);

	(*parent_class->finalize) (object);

	return;
}


static void 
tny_camel_pop_store_account_class_init (TnyCamelPOPStoreAccountClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	TNY_CAMEL_ACCOUNT_CLASS (class)->set_online_status_func = tny_camel_pop_store_account_set_online_status;

	TNY_CAMEL_STORE_ACCOUNT_CLASS (class)->get_folders_func = tny_camel_pop_store_account_get_folders;
	TNY_CAMEL_STORE_ACCOUNT_CLASS (class)->remove_folder_func = tny_camel_pop_store_account_remove_folder;
	TNY_CAMEL_STORE_ACCOUNT_CLASS (class)->create_folder_func = tny_camel_pop_store_account_create_folder;

	object_class->finalize = tny_camel_pop_store_account_finalize;

	g_type_class_add_private (object_class, sizeof (TnyCamelPopStoreAccountPriv));

	return;
}

/**
 * tny_camel_pop_store_account_get_delete_originals:
 * @self: a TnyCamelPOPStoreAccount
 *
 * Get the delete originals property of @self.
 *
 * Return value: Whether or not to delete original messages from the service
 **/
gboolean 
tny_camel_pop_store_account_get_delete_originals (TnyCamelPOPStoreAccount *self)
{
	TnyCamelPopStoreAccountPriv *priv = TNY_CAMEL_POP_STORE_ACCOUNT_GET_PRIVATE (self);

	return priv->delete_originals;
}

/**
 * tny_camel_pop_store_account_set_delete_originals:
 * @self: a TnyCamelPOPStoreAccount
 * @delete_originals: Whether or not to delete original messages from the service
 *
 * Set the delete originals property of @self.
 *
 **/
void 
tny_camel_pop_store_account_set_delete_originals (TnyCamelPOPStoreAccount *self, gboolean delete_originals)
{
	TnyCamelPopStoreAccountPriv *priv = TNY_CAMEL_POP_STORE_ACCOUNT_GET_PRIVATE (self);

	priv->delete_originals = delete_originals;
}

static void
tny_camel_pop_store_account_instance_init (GTypeInstance *instance, gpointer g_class)
{
	
	TnyCamelPopStoreAccountPriv *priv = TNY_CAMEL_POP_STORE_ACCOUNT_GET_PRIVATE (instance);

	priv->lock = g_mutex_new ();
	priv->inbox = NULL;
	priv->delete_originals = FALSE;

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


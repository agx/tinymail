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

#include <glib.h>

#include <string.h>

#include <tny-account-iface.h>
#include <tny-store-account-iface.h>
#include <tny-store-account.h>

#include <tny-msg-folder-iface.h>
#include <tny-msg-folder.h>

#include <camel/camel.h>
#include <camel/camel-session.h>
#include <camel/camel-store.h>


static GObjectClass *parent_class = NULL;

#include "tny-account-priv.h"
#include "tny-store-account-priv.h"
#include "tny-msg-folder-priv.h"

#include <tny-camel-shared.h>
#include <tny-account-store-iface.h>

#define TNY_STORE_ACCOUNT_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_STORE_ACCOUNT, TnyStoreAccountPriv))



static void 
tny_store_account_reconnect (TnyAccount *self)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);

	if (priv->session && priv->proto && priv->user && priv->host)
	{
		CamelURL *url = NULL;
		gchar *proto = g_strdup_printf ("%s://", priv->proto); 

		url = camel_url_new (proto, priv->ex);
		g_free (proto);
	
		camel_url_set_protocol (url, priv->proto); 

		camel_url_set_user (url, priv->user);
		camel_url_set_host (url, priv->host);
	
		if (G_LIKELY (priv->url_string))
			g_free (priv->url_string);

		priv->url_string = camel_url_to_string (url, 0);

		if (G_UNLIKELY (priv->service))
			camel_object_unref (CAMEL_OBJECT (priv->service));
	
		priv->service = camel_session_get_service 
			(CAMEL_SESSION (priv->session), priv->url_string, 
			priv->type, priv->ex);
	
		if (G_UNLIKELY (priv->service == NULL))
		{
			g_error ("couldn't get service %s: %s\n", priv->url_string,
				   camel_exception_get_description (priv->ex));
			camel_exception_clear (priv->ex);
			return;
		}

		camel_url_free (url);
	}

	if (G_LIKELY (priv->service) && G_UNLIKELY (priv->pass_func_set) 
		&& G_UNLIKELY (priv->proto) && G_UNLIKELY (priv->user) 
		&& G_UNLIKELY (priv->host))
		camel_service_connect (priv->service, priv->ex);

	return;
}


static void
destroy_folder (gpointer data, gpointer user_data)
{
	g_object_unref (G_OBJECT (data));
}

static void 
tny_store_account_clear_folders (TnyStoreAccountPriv *priv)
{
	if (G_LIKELY (priv->folders))
	{
		g_mutex_lock (priv->folders_lock);
		g_list_foreach (priv->folders, destroy_folder, NULL);
		g_mutex_unlock (priv->folders_lock);
	}

	if (G_UNLIKELY (priv->ufolders))
	{
		g_mutex_lock (priv->folders_lock);
		g_list_foreach (priv->ufolders, destroy_folder, NULL);
		g_mutex_unlock (priv->folders_lock);
	}

	g_mutex_lock (priv->folders_lock);
	priv->folders = NULL;
	priv->ufolders = NULL;
	g_mutex_unlock (priv->folders_lock);

}

static void 
fill_folders_recursive (TnyStoreAccountIface *self, CamelStore *store, TnyMsgFolderIface *parent, CamelFolderInfo *iter, TnyStoreAccountFolderType type)
{
	TnyStoreAccountPriv *priv = TNY_STORE_ACCOUNT_GET_PRIVATE (self);

	while (iter)
	{
		gboolean subscribed = TRUE;
		TnyMsgFolderIface *iface = TNY_MSG_FOLDER_IFACE (
			tny_msg_folder_new ());

		tny_msg_folder_iface_set_id (iface, iter->full_name);
		tny_msg_folder_iface_set_account (iface, TNY_ACCOUNT_IFACE (self));

		_tny_msg_folder_set_name_priv (iface, iter->name);

		if (G_UNLIKELY (type == TNY_STORE_ACCOUNT_FOLDER_TYPE_ALL))
			subscribed = camel_store_folder_subscribed (store, iter->full_name);

		/* Sync */
		_tny_msg_folder_set_subscribed_priv (iface, subscribed);

		if (G_LIKELY (parent))
		{
			tny_msg_folder_iface_add_folder (parent, iface);
			/* _add_folder parents to the folder by reffing */
			g_object_unref (G_OBJECT (iface)); 

		} else 
		{
			g_mutex_lock (priv->folders_lock);
			priv->folders = g_list_append (priv->folders, iface);
			g_mutex_unlock (priv->folders_lock);

			/* No unref keeps current folder the parent ref */
		}

		fill_folders_recursive (self, store, iface, iter->child, type);

		iter = iter->next;
	}
}

static const GList*
tny_store_account_get_folders (TnyStoreAccountIface *self, TnyStoreAccountFolderType type)
{
	TnyStoreAccountPriv *priv = TNY_STORE_ACCOUNT_GET_PRIVATE (self);
	TnyAccountPriv *apriv = TNY_ACCOUNT_GET_PRIVATE (self);

	const GList *retval;
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	CamelStore *store;

	tny_store_account_clear_folders (priv);

	g_static_rec_mutex_lock (apriv->service_lock);
	store = camel_session_get_store (CAMEL_SESSION (apriv->session), 
			apriv->url_string, &ex);
	g_static_rec_mutex_unlock (apriv->service_lock);

	if (G_LIKELY (g_ascii_strcasecmp (tny_account_iface_get_proto (TNY_ACCOUNT_IFACE (self)), "pop") != 0))
	{
		CamelFolderInfo *info;

		switch (type)
		{
			case TNY_STORE_ACCOUNT_FOLDER_TYPE_ALL:
				info = camel_store_get_folder_info 
					(store, "", CAMEL_STORE_FOLDER_INFO_FAST |
						CAMEL_STORE_FOLDER_INFO_NO_VIRTUAL |
						CAMEL_STORE_FOLDER_INFO_RECURSIVE, &ex);
				break;

			case TNY_STORE_ACCOUNT_FOLDER_TYPE_SUBSCRIBED:
			default:
				info = camel_store_get_folder_info 
					(store, "", CAMEL_STORE_FOLDER_INFO_SUBSCRIBED |
						CAMEL_STORE_FOLDER_INFO_RECURSIVE | 
						CAMEL_STORE_FOLDER_INFO_NO_VIRTUAL |
						CAMEL_STORE_FOLDER_INFO_FAST, &ex);

				break;
		}

		fill_folders_recursive (self, store, NULL, info, type);

		/* Tell the observers that they should reload */
		if (G_LIKELY (priv->folders) && G_LIKELY (priv->folders->data))
			g_signal_emit (priv->folders->data, tny_msg_folder_iface_signals [FOLDERS_RELOADED], 0);

		camel_store_free_folder_info (store, info);

		g_mutex_lock (priv->folders_lock);
		retval = priv->folders;
		g_mutex_unlock (priv->folders_lock);

	} else {
		TnyMsgFolderIface *inbox = TNY_MSG_FOLDER_IFACE (tny_msg_folder_new ());
		CamelException ex = CAMEL_EXCEPTION_INITIALISER;
		CamelFolder *folder = camel_store_get_inbox (store, &ex);

		/* TODO: Implement subscription logic for POP */

		g_object_ref (G_OBJECT (inbox));

		tny_msg_folder_set_folder (TNY_MSG_FOLDER (inbox), folder);
		tny_msg_folder_iface_set_account (inbox, TNY_ACCOUNT_IFACE (self));
		tny_msg_folder_iface_set_name (TNY_MSG_FOLDER_IFACE (inbox), "INBOX");

		g_mutex_lock (priv->folders_lock);
		priv->folders = g_list_append (priv->folders, inbox);
		g_mutex_unlock (priv->folders_lock);

		tny_msg_folder_iface_uncache (inbox);
		g_signal_emit (inbox, tny_msg_folder_iface_signals [FOLDERS_RELOADED], 0);

		g_mutex_lock (priv->folders_lock);
		retval = priv->folders;
		g_mutex_unlock (priv->folders_lock);
	}


	return retval;
}

static void
tny_store_account_subscribe (TnyStoreAccountIface *self, TnyMsgFolderIface *folder)
{
	TnyAccountPriv *apriv = TNY_ACCOUNT_GET_PRIVATE (self);
	TnyAccountStoreIface *astore = (TnyAccountStoreIface *)tny_account_iface_get_account_store (TNY_ACCOUNT_IFACE (self));

	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	CamelStore *store;

	g_static_rec_mutex_lock (apriv->service_lock);
	store = camel_session_get_store (CAMEL_SESSION (apriv->session), 
			apriv->url_string, &ex);
	g_static_rec_mutex_unlock (apriv->service_lock);

	camel_store_subscribe_folder (store, tny_msg_folder_iface_get_name (folder), &ex);

	tny_store_account_get_folders (self, TNY_STORE_ACCOUNT_FOLDER_TYPE_SUBSCRIBED);

	if (astore)
		g_signal_emit (astore, tny_account_store_iface_signals [ACCOUNTS_RELOADED], 0);

	/* Sync */
	_tny_msg_folder_set_subscribed_priv (folder, TRUE);

	return;
}

static void
tny_store_account_unsubscribe (TnyStoreAccountIface *self, TnyMsgFolderIface *folder)
{
	TnyAccountPriv *apriv = TNY_ACCOUNT_GET_PRIVATE (self);
	TnyAccountStoreIface *astore = (TnyAccountStoreIface *)tny_account_iface_get_account_store (TNY_ACCOUNT_IFACE (self));

	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	CamelStore *store;

	g_static_rec_mutex_lock (apriv->service_lock);
	store = camel_session_get_store (CAMEL_SESSION (apriv->session), 
			apriv->url_string, &ex);
	g_static_rec_mutex_unlock (apriv->service_lock);

	camel_store_unsubscribe_folder (store, tny_msg_folder_iface_get_name (folder), &ex);

	tny_store_account_get_folders (self, TNY_STORE_ACCOUNT_FOLDER_TYPE_SUBSCRIBED);

	if (astore)
		g_signal_emit (astore, tny_account_store_iface_signals [ACCOUNTS_RELOADED], 0);

	/* Sync */
	_tny_msg_folder_set_subscribed_priv (folder, FALSE);

	return;
}


/**
 * tny_store_account_new:
 * 
 *
 * Return value: A new #TnyStoreAccountIface instance implemented for Camel
 **/
TnyStoreAccount*
tny_store_account_new (void)
{
	TnyStoreAccount *self = g_object_new (TNY_TYPE_STORE_ACCOUNT, NULL);

	return self;
}

static void
tny_store_account_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyStoreAccount *self = (TnyStoreAccount *)instance;
	TnyStoreAccountPriv *priv = TNY_STORE_ACCOUNT_GET_PRIVATE (self);
	TnyAccountPriv *apriv = TNY_ACCOUNT_GET_PRIVATE (self);

	apriv->type = CAMEL_PROVIDER_STORE;
	priv->folders = NULL;

	priv->folders_lock = g_mutex_new ();

	return;
}


static void
tny_store_account_finalize (GObject *object)
{
	TnyStoreAccount *self = (TnyStoreAccount *)object;	
	TnyStoreAccountPriv *priv = TNY_STORE_ACCOUNT_GET_PRIVATE (self);

	tny_store_account_clear_folders (priv);

	g_mutex_free (priv->folders_lock);

	(*parent_class->finalize) (object);

	return;
}

static void
tny_store_account_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyStoreAccountIfaceClass *klass = (TnyStoreAccountIfaceClass *)g_iface;

	klass->get_folders_func = tny_store_account_get_folders;
	klass->subscribe_func = tny_store_account_subscribe;
	klass->unsubscribe_func = tny_store_account_unsubscribe;

	return;
}


static void 
tny_store_account_class_init (TnyStoreAccountClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_store_account_finalize;

	((TnyAccountClass*)class)->reconnect_func = tny_store_account_reconnect;

	g_type_class_add_private (object_class, sizeof (TnyStoreAccountPriv));

	return;
}

GType 
tny_store_account_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY (!camel_type_init_done))
	{
		camel_type_init ();
		camel_type_init_done = TRUE;
	}

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyStoreAccountClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_store_account_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyStoreAccount),
		  0,      /* n_preallocs */
		  tny_store_account_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_store_account_iface_info = 
		{
		  (GInterfaceInitFunc) tny_store_account_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (TNY_TYPE_ACCOUNT,
			"TnyStoreAccount",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_STORE_ACCOUNT_IFACE, 
			&tny_store_account_iface_info);
	}

	return type;
}


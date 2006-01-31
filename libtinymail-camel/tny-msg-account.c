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
#include <glib.h>

#include <tny-msg-account-iface.h>
#include <tny-msg-account.h>
#include <tny-msg-folder-iface.h>
#include <tny-msg-folder.h>

#include <camel/camel.h>
#include <camel/camel-session.h>
#include <camel/camel-store.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <tny-camel-session.h>

#include "/home/pvanhoof/personal.h"

static GObjectClass *parent_class = NULL;

#include "tny-msg-account-priv.h"

#define TNY_MSG_ACCOUNT_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_MSG_ACCOUNT_TYPE, TnyMsgAccountPriv))


static void 
fill_folders_recursive (TnyMsgAccountIface *self, TnyMsgFolderIface *parent, CamelFolderInfo *iter)
{
	TnyMsgAccountPriv *priv = TNY_MSG_ACCOUNT_GET_PRIVATE (self);

	while (iter)
	{
		TnyMsgFolderIface *iface = TNY_MSG_FOLDER_IFACE (
			tny_msg_folder_new ());


		g_print ("Full name: %s\n", iter->full_name);
		tny_msg_folder_iface_set_id (iface, iter->full_name);
		tny_msg_folder_iface_set_account (iface, self);

		if (parent)
			tny_msg_folder_iface_add_folder (parent, iface);
		else
			priv->folders = g_list_append (priv->folders, iface);

		fill_folders_recursive (self, iface, iter->child);

		iter = iter->next;
	}
}

const GList*
tny_msg_account_get_folders (TnyMsgAccountIface *self)
{
	TnyMsgAccountPriv *priv = TNY_MSG_ACCOUNT_GET_PRIVATE (self);

	CamelStore *store = camel_session_get_store (session, priv->url_string, priv->ex);
	CamelFolderInfo *info = camel_store_get_folder_info 
		(store, "", CAMEL_STORE_FOLDER_INFO_SUBSCRIBED |
			CAMEL_STORE_FOLDER_INFO_RECURSIVE, priv->ex);

	fill_folders_recursive (self, NULL, info);
	
	camel_store_free_folder_info (store, info);

	return priv->folders;
}

const CamelService*
_tny_msg_account_get_service (TnyMsgAccount *self)
{
	TnyMsgAccountPriv *priv = TNY_MSG_ACCOUNT_GET_PRIVATE (self);

	return (const CamelService *)priv->service;
}

const gchar*
_tny_msg_account_get_url_string (TnyMsgAccount *self)
{
	TnyMsgAccountPriv *priv = TNY_MSG_ACCOUNT_GET_PRIVATE (self);

	return (const gchar*)priv->url_string;
}


TnyMsgAccount*
tny_msg_account_new (void)
{
	TnyMsgAccount *self = g_object_new (TNY_MSG_ACCOUNT_TYPE, NULL);

	return self;
}

static void
tny_msg_account_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyMsgAccount *self = (TnyMsgAccount *)instance;
	TnyMsgAccountPriv *priv = TNY_MSG_ACCOUNT_GET_PRIVATE (self);

	CamelURL *url = NULL;

	priv->ex = camel_exception_new ();
	camel_exception_init (priv->ex);

	url = camel_url_new (PERSONAL_PROTOCOL "://", priv->ex);
	camel_url_set_protocol (url, PERSONAL_PROTOCOL);
	camel_url_set_user (url, PERSONAL_USERNAME);
	camel_url_set_host (url, PERSONAL_HOSTNAME);

	if (!session)
		tny_camel_session_prepare ();

	priv->url_string = camel_url_to_string (url, 0);	

	priv->service = camel_session_get_service 
		(session, priv->url_string, 
		CAMEL_PROVIDER_STORE, priv->ex);

	if (priv->service == NULL) {
		g_print ("couldn't get service %s: %s\n", priv->url_string,
			   camel_exception_get_description (priv->ex));
		camel_exception_clear (priv->ex);
		return;
	}

	camel_service_connect (priv->service, priv->ex);

	if (camel_exception_get_id (priv->ex) != CAMEL_EXCEPTION_NONE)
	{
		g_print ("Camel connection failure (%s)\n", camel_exception_get_description (priv->ex));
	} else {
		g_print ("Connected?!\n");
	}

	return;
}

static void
tny_msg_account_finalize (GObject *object)
{
	TnyMsgAccount *self = (TnyMsgAccount *)object;	
	TnyMsgAccountPriv *priv = TNY_MSG_ACCOUNT_GET_PRIVATE (self);

	camel_exception_free (priv->ex);

	(*parent_class->finalize) (object);

	return;
}

static void
tny_msg_account_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyMsgAccountIfaceClass *klass = (TnyMsgAccountIfaceClass *)g_iface;

	klass->get_folders_func = tny_msg_account_get_folders;

	return;
}

static void 
tny_msg_account_class_init (TnyMsgAccountClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_msg_account_finalize;

	g_type_class_add_private (object_class, sizeof (TnyMsgAccountPriv));

	return;
}

GType 
tny_msg_account_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgAccountClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_msg_account_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMsgAccount),
		  0,      /* n_preallocs */
		  tny_msg_account_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_msg_account_iface_info = 
		{
		  (GInterfaceInitFunc) tny_msg_account_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyMsgAccount",
			&info, 0);

		g_type_add_interface_static (type, TNY_MSG_ACCOUNT_IFACE_TYPE, 
			&tny_msg_account_iface_info);
	}

	return type;
}


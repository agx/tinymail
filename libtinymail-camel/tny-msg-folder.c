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

#include <tny-msg-folder-iface.h>
#include <tny-msg-folder.h>
#include <tny-msg-iface.h>
#include <tny-msg-header-iface.h>
#include <tny-msg.h>
#include <tny-msg-header.h>
#include <tny-msg-account-iface.h>
#include <tny-msg-account.h>

#include <camel/camel-folder.h>
#include <camel/camel.h>
#include <camel/camel-session.h>
#include <camel/camel-store.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <tny-camel-session.h>
#include "tny-msg-account-priv.h"
#include "tny-msg-folder-priv.h"
#include <camel/camel.h>
#include <camel/camel-folder-summary.h>

static GObjectClass *parent_class = NULL;


#define TNY_MSG_FOLDER_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_MSG_FOLDER_TYPE, TnyMsgFolderPriv))


static void
load_folder (TnyMsgFolderPriv *priv)
{
	if (!priv->folder)
	{
		CamelException ex;
		CamelStore *store = (CamelStore*) _tny_msg_account_get_service 
			(TNY_MSG_ACCOUNT (priv->account));

		priv->folder = camel_store_get_folder (store, priv->folder_name, 0, &ex);

		g_print ("Get folder (%s) to (%s)\n", 
			priv->folder_name, camel_folder_get_name (priv->folder));
	}

	return;
}

CamelFolder*
_tny_msg_folder_get_camel_folder (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	return priv->folder;
}

const GList*
tny_msg_folder_get_folders (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	return priv->folders;
}

void
tny_msg_folder_add_folder (TnyMsgFolderIface *self, TnyMsgFolderIface *folder)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	priv->folders = g_list_append (priv->folders, folder);

	return;
}

const TnyMsgAccountIface*  
tny_msg_folder_get_account (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	return priv->account;
}

void
tny_msg_folder_set_account (TnyMsgFolderIface *self, const TnyMsgAccountIface *account)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	priv->account = TNY_MSG_ACCOUNT_IFACE (account);

	return;
}

static void
add_message_with_uid (gpointer data, gpointer user_data)
{
	const char *uid = (const char*)data;
	TnyMsgFolderIface *self = user_data;
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	TnyMsgHeaderIface *header = TNY_MSG_HEADER_IFACE (tny_msg_header_new ());

	tny_msg_header_iface_set_folder (header, self);
	tny_msg_header_iface_set_id (header, uid);

	priv->cached_hdrs = g_list_append (priv->cached_hdrs, header);

	return;
}


static const GList*
tny_msg_folder_get_headers (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	load_folder (priv);

	/* TODO: Cache this on disk, compare it. Don't just cache in memory */

	if (!priv->cached_hdrs)
	{
		GPtrArray *uids = NULL;
		CamelException ex;

		camel_folder_refresh_info (priv->folder, &ex);
		uids = camel_folder_get_uids (priv->folder);
		g_ptr_array_foreach (uids, add_message_with_uid, self);
		camel_folder_free_uids (priv->folder, uids);
	}

	return priv->cached_hdrs;
}

static void
destroy_cached_key (gpointer data)
{
	/* data is a const */
	return;
}


static void
destroy_cached_value (gpointer data)
{
	/* Data is a TnyMsgIface or a TnyMsgHeaderIface */
	g_object_unref (G_OBJECT (data));
	return;
}


static const TnyMsgIface*
tny_msg_folder_get_message (TnyMsgFolderIface *self, TnyMsgHeaderIface *header)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	TnyMsgIface *message = NULL;
	const gchar *id = tny_msg_header_iface_get_id (header);

	load_folder (priv);

	if (!priv->cached_msgs)
	{
		priv->cached_msgs = g_hash_table_new_full 
			(g_str_hash, g_str_equal, destroy_cached_key,
			destroy_cached_value);
	} else {
		message = g_hash_table_lookup (priv->cached_msgs, id);
	}
	
	if (!message)
	{
		CamelException ex;
		CamelMimeMessage *camel_message = camel_folder_get_message  
			(priv->folder, (const char *) id, &ex);

		if (camel_exception_get_id (&ex) == CAMEL_EXCEPTION_NONE)
		{
			message = TNY_MSG_IFACE (tny_msg_new ());
			_tny_msg_set_camel_mime_message (TNY_MSG (message), camel_message);
			g_hash_table_insert (priv->cached_msgs, (gpointer)id, message);
		}
	}

	return message;
}


static const gchar*
tny_msg_folder_get_name (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	const gchar *name = NULL;
	
	load_folder (priv);

	name = camel_folder_get_name (priv->folder);

	return name;
}

static const gchar*
tny_msg_folder_get_id (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	return priv->folder_name;
}

static void
tny_msg_folder_set_id (TnyMsgFolderIface *self, const gchar *id)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	if (priv->folder_name)
		g_free (priv->folder_name);

	priv->folder_name = g_strdup (id);

	return;
}

static void
tny_msg_folder_set_name (TnyMsgFolderIface *self, const gchar *name)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	load_folder (priv);

	camel_folder_rename (priv->folder, name);

	return;
}

void
tny_msg_folder_set_folder (TnyMsgFolder *self, CamelFolder *camel_folder)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	
	if (priv->folder)
		camel_object_unref (priv->folder);

	camel_object_ref (camel_folder);

	priv->folder = camel_folder;

	return;
}

CamelFolder*
tny_msg_folder_get_folder (TnyMsgFolder *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	return priv->folder;
}


TnyMsgFolder*
tny_msg_folder_new_with_folder  (CamelFolder *camel_folder)
{
	TnyMsgFolder *self = g_object_new (TNY_MSG_FOLDER_TYPE, NULL);
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (self);

	tny_msg_folder_set_folder (self, camel_folder);

	return self;
}



TnyMsgFolder*
tny_msg_folder_new (void)
{
	TnyMsgFolder *self = g_object_new (TNY_MSG_FOLDER_TYPE, NULL);

	return self;
}


static void
tny_msg_folder_finalize (GObject *object)
{
	TnyMsgFolder *self = (TnyMsgFolder*) object;
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (self);

	if (priv->folder)
		camel_object_unref (priv->folder);

	if (priv->cached_msgs)
		g_hash_table_destroy (priv->cached_msgs);

	if (priv->cached_hdrs)
		g_list_foreach (priv->cached_hdrs, (GFunc)g_object_unref, NULL);

	(*parent_class->finalize) (object);

	return;
}


static void
tny_msg_folder_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyMsgFolderIfaceClass *klass = (TnyMsgFolderIfaceClass *)g_iface;

	klass->get_headers_func = tny_msg_folder_get_headers;
	klass->get_message_func = tny_msg_folder_get_message;

	klass->set_id_func = tny_msg_folder_set_id;
	klass->get_id_func = tny_msg_folder_get_id;

	klass->set_name_func = tny_msg_folder_set_name;
	klass->get_name_func = tny_msg_folder_get_name;

	klass->has_cache_func = NULL;
	klass->uncache_func = NULL;

	klass->add_folder_func = tny_msg_folder_add_folder;
	klass->get_folders_func = tny_msg_folder_get_folders;

	klass->get_account_func = tny_msg_folder_get_account;
	klass->set_account_func = tny_msg_folder_set_account;

	return;
}

static void 
tny_msg_folder_class_init (TnyMsgFolderIfaceClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_msg_folder_finalize;

	g_type_class_add_private (object_class, sizeof (TnyMsgFolderPriv));

	return;
}

static void
tny_msg_folder_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyMsgFolder *self = (TnyMsgFolder *)instance;
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (self);

	priv->folder = NULL;
	priv->folders = NULL;
	priv->cached_hdrs = NULL;
	priv->cached_msgs = NULL;

	return;
}

GType 
tny_msg_folder_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgFolderClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_msg_folder_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMsgFolder),
		  0,      /* n_preallocs */
		  tny_msg_folder_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_msg_folder_iface_info = 
		{
		  (GInterfaceInitFunc) tny_msg_folder_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyMsgFolder",
			&info, 0);

		g_type_add_interface_static (type, TNY_MSG_FOLDER_IFACE_TYPE, 
			&tny_msg_folder_iface_info);
	}

	return type;
}

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

#include <tny-msg-folder-iface.h>
#include <tny-msg-folder.h>
#include <tny-msg-iface.h>
#include <tny-msg-header-iface.h>
#include <tny-msg.h>
#include <tny-msg-header.h>
#include <tny-account-iface.h>
#include <tny-account.h>

#include <camel/camel-folder.h>
#include <camel/camel.h>
#include <camel/camel-session.h>
#include <camel/camel-store.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <tny-session-camel.h>

#include "tny-account-priv.h"
#include "tny-msg-folder-priv.h"

#include <tny-camel-shared.h>

#include <camel/camel.h>
#include <camel/camel-folder-summary.h>

static GObjectClass *parent_class = NULL;


#define TNY_MSG_FOLDER_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_MSG_FOLDER, TnyMsgFolderPriv))

typedef struct
{
	GList *list;
} RelaxedData;

static void
tny_msg_folder_relaxed_destroy (gpointer data)
{
	RelaxedData *d = data;

	g_list_free (d->list);
	d->list = NULL;

	g_free (d);

	return;
}

static gboolean
tny_msg_folder_relaxed_freeer (gpointer data)
{
	RelaxedData *d = data;
	GList *list = d->list;
	gint count = 0;

	/* g_print ("removing 5 (%d)\n", g_list_length (list)); */

	while ((count < 5) && list)
	{
		GList *element = list;

		if (element && element->data)
			g_object_unref (G_OBJECT (element->data));

		list = g_list_remove_link (list, element);

		g_list_free (element);

		/* g_print ("rcount:%d lcount:%d\n", count, g_list_length (list)); */

		count++;
	}

	d->list = list;

	if (count <= 1)
		return FALSE;
	
	return TRUE;
}


static void 
tny_msg_folder_hdr_cache_uncacher (TnyMsgFolderPriv *priv)
{
	if (priv->cached_hdrs)
		g_list_foreach (priv->cached_hdrs, (GFunc)tny_msg_header_iface_uncache,NULL);
}

static void 
tny_msg_folder_hdr_cache_remover (TnyMsgFolderPriv *priv)
{
	if (priv->cached_hdrs)
	{
		RelaxedData *d = g_new (RelaxedData, 1);

		d->list = priv->cached_hdrs;
		priv->cached_hdrs = NULL;
		g_idle_add_full (G_PRIORITY_LOW, tny_msg_folder_relaxed_freeer, 
			d, tny_msg_folder_relaxed_destroy);
	}
} 

static void 
unload_folder (TnyMsgFolderPriv *priv)
{
	/* The uncacher is faster but removes less data,
	   the remover is for the user as fast but uses an 
	   idle function. So in reality it is a lot slower,
	   but the user doesn't notice it. */

	/* tny_msg_folder_hdr_cache_uncacher (priv); */

	tny_msg_folder_hdr_cache_remover (priv); 

	if (priv->folder)
		camel_object_unref (CAMEL_OBJECT (priv->folder));

	priv->folder = NULL;
}

static void
load_folder (TnyMsgFolderPriv *priv)
{
	if (!priv->folder)
	{
		CamelException ex;
		CamelStore *store = (CamelStore*) _tny_account_get_service 
			(TNY_ACCOUNT (priv->account));

		priv->folder = camel_store_get_folder 
			(store, priv->folder_name, 0, &ex);
		priv->unread_length = (guint)
			camel_folder_get_unread_message_count (priv->folder);
	}

	return;
}

CamelFolder*
_tny_msg_folder_get_camel_folder (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	return priv->folder;
}

static const GList*
tny_msg_folder_get_folders (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	return priv->folders;
}


static guint
tny_msg_folder_get_unread_count (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	return priv->unread_length;
}

static guint
tny_msg_folder_get_all_count (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	return priv->cached_length;
}

static void
tny_msg_folder_add_folder (TnyMsgFolderIface *self, TnyMsgFolderIface *folder)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	priv->folders = g_list_append (priv->folders, folder);

	/* Tell the observers */
	g_signal_emit (self, tny_msg_folder_iface_signals [FOLDER_INSERTED], 0, folder);

	return;
}

static const TnyAccountIface*  
tny_msg_folder_get_account (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	return priv->account;
}

static void
tny_msg_folder_set_account (TnyMsgFolderIface *self, const TnyAccountIface *account)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	priv->account = TNY_ACCOUNT_IFACE (account);

	return;
}

typedef struct 
{ /* This is a speedup trick */
	TnyMsgFolderIface *self;
	TnyMsgFolderPriv *priv;
} FldAndPriv;

static void
add_message_with_uid (gpointer data, gpointer user_data)
{
	FldAndPriv *ptr = user_data;
	const char *uid = (const char*)data;

	/* Unpack speedup trick */
	TnyMsgFolderIface *self = ptr->self;
	TnyMsgFolderPriv *priv = ptr->priv;

	TnyMsgHeaderIface *header = TNY_MSG_HEADER_IFACE (tny_msg_header_new ());

	tny_msg_header_iface_set_folder (header, self);
	tny_msg_header_iface_set_uid (header, uid);

	/* Uncertain (the _new is already a ref, right?) */
	/* g_object_ref (G_OBJECT (header)) */

	priv->cached_hdrs = g_list_append (priv->cached_hdrs, header);
	priv->cached_length++;

	/* TODO: If unread */
	/* priv->unread_length++; */

	return;
}


static const GList*
tny_msg_folder_get_headers (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	load_folder (priv);

	if (!priv->cached_hdrs)
	{
		GPtrArray *uids = NULL;
		CamelException ex;
		FldAndPriv *ptr = g_new (FldAndPriv, 1);

		/* Prepare speedup trick */
		ptr->self = self;
		ptr->priv = priv;

		camel_folder_refresh_info (priv->folder, &ex);
		uids = camel_folder_get_uids (priv->folder);

		priv->cached_length = 0;
		g_ptr_array_foreach (uids, add_message_with_uid, ptr);

		/* Cleanup speedup trick */
		g_free (ptr);

		/* Speedup trick, also check tny-msg-header.c */
		priv->cached_uids = uids;

		/* The trick is not to free the uid's GPtrArray now, but
		 * in stead keep it during the livetime of the folder.
		 * The TnyMsgHeader instances have a reference to the uid's
		 * in the array. 
		 *
		 * The idea is that if a folder get's finalized, first all its
		 * msg-header instances are also finalized (invalid). So we
		 * can keep the uid pointers here, and simply assign it (and
		 * not strdup them for each msg-header instance).
		 *
		 * Just make sure that, if using this trick, you don't free the
		 * uid pointer in the msg-header instance. Free it here.
		 */

		/* So we postphone the freeing to the finalize 
		camel_folder_free_uids (priv->folder, uids); */

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
tny_msg_folder_get_message (TnyMsgFolderIface *self, const TnyMsgHeaderIface *header)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	TnyMsgIface *message = NULL;

	const gchar *id = tny_msg_header_iface_get_uid (TNY_MSG_HEADER_IFACE (header));

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
		CamelException *ex = camel_exception_new ();
		camel_exception_init (ex);

		CamelMimeMessage *camel_message = camel_folder_get_message  
			(priv->folder, (const char *) id, ex);

		if (camel_exception_get_id (ex) == CAMEL_EXCEPTION_NONE)
		{
			message = TNY_MSG_IFACE (tny_msg_new ());

			tny_msg_iface_set_folder (message, self);
			tny_msg_iface_set_header (message, TNY_MSG_HEADER_IFACE (header));
			_tny_msg_set_camel_mime_message (TNY_MSG (message), camel_message);
			g_hash_table_insert (priv->cached_msgs, (gpointer)id, message);
		}

		camel_exception_free (ex);
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

gint s=0;
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

/**
 * tny_msg_folder_set_folder:
 * @self: A #TnyMsgFolder object
 * @camel_folder: The #CamelFolder instance to play proxy for
 *
 **/
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

/**
 * tny_msg_folder_get_folder:
 * @self: A #TnyMsgFolder object
 *
 * Return value: The CamelFolder instance to play proxy for
 **/
CamelFolder*
tny_msg_folder_get_folder (TnyMsgFolder *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	return priv->folder;
}

/**
 * tny_msg_folder_new_with_folder:
 * @camel_folder: CamelFolder instance to play proxy for 
 *
 * The #TnyMsgFolder implementation is actually a proxy for #CamelFolder
 *
 * Return value: A new #TnyMsgFolderIface instance implemented for Camel
 **/
TnyMsgFolder*
tny_msg_folder_new_with_folder (CamelFolder *camel_folder)
{
	TnyMsgFolder *self = g_object_new (TNY_TYPE_MSG_FOLDER, NULL);
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (self);

	tny_msg_folder_set_folder (self, camel_folder);

	return self;
}


/**
 * tny_msg_folder_new:
 * 
 * The #TnyMsgFolder implementation is actually a proxy for #CamelFolder.
 * You need to set the #CamelFolder after using this constructor using
 * tny_msg_folder_set_folder
 *
 * Return value: A new #TnyMsgFolderIface instance implemented for Camel
 **/
TnyMsgFolder*
tny_msg_folder_new (void)
{
	TnyMsgFolder *self = g_object_new (TNY_TYPE_MSG_FOLDER, NULL);

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

	tny_msg_folder_hdr_cache_remover (priv);

	/* Speedup trick, also check tny-msg-header.c */

	if (priv->cached_uids)
		camel_folder_free_uids (priv->folder, priv->cached_uids);

	priv->cached_uids = NULL;

	(*parent_class->finalize) (object);

	return;
}

static void
tny_msg_folder_uncache (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (self);

	if (priv->folder != NULL)
		unload_folder (priv);

	return;
}

static const gboolean
tny_msg_folder_has_cache (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (self);

	return (priv->folder != NULL);
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

	klass->has_cache_func = tny_msg_folder_has_cache;
	klass->uncache_func = tny_msg_folder_uncache;

	klass->add_folder_func = tny_msg_folder_add_folder;
	klass->get_folders_func = tny_msg_folder_get_folders;

	klass->get_unread_count_func = tny_msg_folder_get_unread_count;
	klass->get_all_count_func = tny_msg_folder_get_all_count;

	klass->get_account_func = tny_msg_folder_get_account;
	klass->set_account_func = tny_msg_folder_set_account;

	return;
}

static void 
tny_msg_folder_class_init (TnyMsgFolderClass *class)
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

	if (!camel_type_init_done)
	{
		camel_type_init ();
		camel_type_init_done = TRUE;
	}

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

		g_type_add_interface_static (type, TNY_TYPE_MSG_FOLDER_IFACE, 
			&tny_msg_folder_iface_info);
	}

	return type;
}

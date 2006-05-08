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
#include <string.h>
#include <tny-msg-folder-iface.h>
#include <tny-msg-folder.h>
#include <tny-msg-iface.h>
#include <tny-msg-header-iface.h>
#include <tny-msg.h>
#include <tny-msg-header.h>
#include <tny-store-account-iface.h>
#include <tny-store-account.h>

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
	GFunc relaxed_func;
	GMutex *lock;
	gboolean free_lock;
} RelaxedData;

static void
tny_msg_folder_relaxed_data_destroyer (gpointer data)
{
	RelaxedData *d = data;

	g_mutex_lock (d->lock);

	g_list_free (d->list);
	d->list = NULL;
	g_free (d);

	g_mutex_unlock (d->lock);

	if (d->free_lock)
		g_mutex_free (d->lock);

	return;
}


static gboolean
tny_msg_folder_relaxed_performer (gpointer data)
{
	RelaxedData *d = data;
	GList *list = d->list;
	gint count = 0;

	while (G_LIKELY ((count < 5) && list))
	{
		GList *element = list;
		if (G_LIKELY (element && element->data))
			d->relaxed_func (element->data, NULL);
		list = g_list_remove_link (list, element);
		g_list_free (element);
		count++;
	}

	d->list = list;

	if (G_UNLIKELY (count <= 1))
		return FALSE;
	
	return TRUE;
}


static void 
tny_msg_folder_hdr_cache_uncacher (TnyMsgFolderPriv *priv)
{
	if (G_LIKELY (priv->cached_hdrs))
	{
		RelaxedData *d = g_new (RelaxedData, 1);

		d->relaxed_func = (GFunc)tny_msg_header_iface_uncache;

		d->free_lock = TRUE;
		d->lock = g_mutex_new ();

		g_mutex_lock (priv->cached_hdrs_lock);
		d->list = g_list_copy (priv->cached_hdrs);
		g_mutex_unlock (priv->cached_hdrs_lock);

		g_idle_add_full (G_PRIORITY_LOW, tny_msg_folder_relaxed_performer, 
			d, tny_msg_folder_relaxed_data_destroyer);
	}
}

static void 
tny_msg_folder_hdr_cache_remover (TnyMsgFolderPriv *priv)
{
	if (G_LIKELY (priv->cached_hdrs))
	{
		RelaxedData *d = g_new (RelaxedData, 1);

		d->relaxed_func = (GFunc)g_object_unref;
		g_mutex_lock (priv->cached_hdrs_lock);
		d->free_lock = FALSE;
		d->lock = priv->cached_hdrs_lock;
		d->list = priv->cached_hdrs;
		priv->cached_hdrs = NULL;
		/* Speedup trick, also check tny-msg-header.c */
		if (G_LIKELY (priv->folder && priv->cached_uids))
			camel_folder_free_uids (priv->folder, priv->cached_uids);
		priv->cached_uids = NULL;
		g_mutex_unlock (priv->cached_hdrs_lock);

		g_idle_add_full (G_PRIORITY_LOW, tny_msg_folder_relaxed_performer, 
			d, tny_msg_folder_relaxed_data_destroyer);
	}
} 

static void 
unload_folder (TnyMsgFolderPriv *priv)
{

#ifdef PERFORMANCE_MODE
	tny_msg_folder_hdr_cache_uncacher (priv);
#else
	tny_msg_folder_hdr_cache_remover (priv);
#endif

	g_mutex_lock (priv->folder_lock);

	if (G_LIKELY (priv->folder))
		camel_object_unref (CAMEL_OBJECT (priv->folder));
	priv->folder = NULL;

	g_mutex_lock (priv->cached_msgs_lock);
	if (G_LIKELY (priv->cached_msgs))
		g_hash_table_destroy (priv->cached_msgs);
	priv->cached_msgs = NULL;
	g_mutex_unlock (priv->cached_msgs_lock);

	g_mutex_unlock (priv->folder_lock);

	return;
}

static void
load_folder_no_lock (TnyMsgFolderPriv *priv)
{
	if (G_LIKELY (!priv->folder))
	{
		CamelException ex = CAMEL_EXCEPTION_INITIALISER;
		CamelStore *store = (CamelStore*) _tny_account_get_service 
			(TNY_ACCOUNT (priv->account));

		priv->folder = camel_store_get_folder 
			(store, priv->folder_name, 0, &ex);
		priv->has_summary_cap = camel_folder_has_summary_capability (priv->folder);

		if (G_LIKELY (priv->folder) && G_LIKELY (priv->has_summary_cap))
		{
			priv->unread_length = (guint)
				camel_folder_get_unread_message_count (priv->folder);
		}
	}
	
	return;
}


static void
load_folder (TnyMsgFolderPriv *priv)
{
	g_mutex_lock (priv->folder_lock);
	load_folder_no_lock (priv);
	g_mutex_unlock (priv->folder_lock);
}

CamelFolder*
_tny_msg_folder_get_camel_folder (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	CamelFolder *retval;

	g_mutex_lock (priv->folder_lock);
	retval = priv->folder;
	g_mutex_unlock (priv->folder_lock);

	return retval;
}

static const GList*
tny_msg_folder_get_folders (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	const GList *retval;

	g_mutex_lock (priv->folder_lock);
	retval = priv->folders;
	g_mutex_unlock (priv->folder_lock);

	return retval;
}


static const gboolean
tny_msg_folder_get_subscribed (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	gboolean retval;

	g_mutex_lock (priv->folder_lock);
	retval = priv->subscribed;
	g_mutex_unlock (priv->folder_lock);

	return (const gboolean)retval;
}

void /* Only internally used */
_tny_msg_folder_set_subscribed_priv (TnyMsgFolderIface *self, gboolean subscribed)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	g_mutex_lock (priv->folder_lock);
	priv->subscribed = subscribed;
	g_mutex_unlock (priv->folder_lock);

	return;
}

static void
tny_msg_folder_set_subscribed (TnyMsgFolderIface *self, const gboolean subscribed)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	/* These will synchronize me using _tny_msg_folder_set_subscribed_priv */

	if (G_LIKELY (subscribed))
		tny_store_account_iface_subscribe 
			(TNY_STORE_ACCOUNT_IFACE (priv->account), self);
	else
		tny_store_account_iface_unsubscribe
			(TNY_STORE_ACCOUNT_IFACE (priv->account), self);

	return;
}

static guint
tny_msg_folder_get_unread_count (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	guint retval;

	g_mutex_lock (priv->folder_lock);
	retval = priv->unread_length;
	g_mutex_unlock (priv->folder_lock);

	return retval;
}

static guint
tny_msg_folder_get_all_count (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	guint retval;

	g_mutex_lock (priv->folder_lock);
	retval = priv->cached_length;
	g_mutex_unlock (priv->folder_lock);

	return retval;
}

static void
tny_msg_folder_add_folder (TnyMsgFolderIface *self, TnyMsgFolderIface *folder)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	/* We reference, so when reparenting folders, unref your instances */

	g_object_ref (G_OBJECT (folder));

	g_mutex_lock (priv->folders_lock);
	priv->folders = g_list_append (priv->folders, folder);
	g_mutex_unlock (priv->folders_lock);

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

	/* No need to reference, would be a cross reference */
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
	TnyMsgHeaderIface *header = NULL;
	FldAndPriv *ptr = user_data;
	const char *uid = (const char*)data;

	/* Unpack speedup trick */
	TnyMsgFolderIface *self = ptr->self;
	TnyMsgFolderPriv *priv = ptr->priv;

	/* Proxy instantiation */
	header = TNY_MSG_HEADER_IFACE (tny_msg_header_new ());

	tny_msg_header_set_use_summary (TNY_MSG_HEADER (header), 
		priv->has_summary_cap);

	tny_msg_header_iface_set_folder (header, self);
	tny_msg_header_iface_set_uid (header, uid);

	g_mutex_lock (priv->cached_hdrs_lock);
	priv->cached_hdrs = g_list_append (priv->cached_hdrs, header);
	priv->cached_length++;
	/* TODO: If unread */
	/* priv->unread_length++; */
	g_mutex_unlock (priv->cached_hdrs_lock);


	return;
}

typedef struct 
{
	TnyMsgFolderIface *self;
	TnyGetHeadersCallback callback;
	TnyGetHeadersStatusCallback status_callback;
	gpointer user_data;
	gboolean cancelled;
} RefreshHeadersInfo;


static void
destroy_header (gpointer data, gpointer user_data)
{
	g_object_unref (G_OBJECT (data));
	data=NULL;
}

static void
tny_msg_folder_refresh_headers_async_destroyer (gpointer thr_user_data)
{
	g_free (thr_user_data);
}

static gboolean
tny_msg_folder_refresh_headers_async_callback (gpointer thr_user_data)
{
	RefreshHeadersInfo *info = thr_user_data;
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (info->self));

	if (info->callback)
		info->callback (info->self, info->cancelled, info->user_data);

	return FALSE;
}


typedef struct{
	RefreshHeadersInfo *minfo;
	gchar *what;
	gint pc;
} ProgressInfo;

static void
destroy_progress_idle (gpointer data)
{
	ProgressInfo *info = data;
	g_free (info->what);
	g_free (info->minfo);
	g_free (data);

	return;
}

static gboolean
progress_func (gpointer data)
{
	ProgressInfo *info = data;
	const gchar *what = (const gchar*)info->what;
	RefreshHeadersInfo *minfo = info->minfo;
	gint pc = info->pc;

	if (minfo && minfo->status_callback)
		minfo->status_callback (minfo->self, (const gchar*)what, (gint)pc, minfo->user_data);

	return FALSE;
} 


static void
tny_msg_folder_refresh_headers_async_status (struct _CamelOperation *op, const char *what, int pc, void *thr_user_data)
{
	RefreshHeadersInfo *oinfo = thr_user_data;
	ProgressInfo *info = g_new0 (ProgressInfo, 1);

	/* Camel will shredder what and thr_user_data, so we need to copy it */

	info->what = g_strdup (what);
	info->minfo = g_new0 (RefreshHeadersInfo ,1);
	info->minfo->callback = oinfo->callback;
	info->minfo->cancelled = oinfo->cancelled;
	info->minfo->self = oinfo->self;
	info->minfo->status_callback = oinfo->status_callback;
	info->minfo->user_data = oinfo->user_data;
	info->pc = pc;

	g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
		progress_func, info, destroy_progress_idle);

	return;
}


static gpointer 
tny_msg_folder_refresh_headers_async_thread (gpointer thr_user_data)
{
	RefreshHeadersInfo *info = thr_user_data;
	TnyMsgFolderIface *self = info->self;
	gpointer user_data = info->user_data;
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	TnyAccountPriv *apriv = TNY_ACCOUNT_GET_PRIVATE (priv->account);
	gchar *str;

	CamelException *ex = camel_exception_new ();

	camel_exception_init (ex);

	g_mutex_lock (priv->folder_lock);
	load_folder_no_lock (priv);

	str = g_strdup_printf ("Reading folder `%s'", priv->folder->full_name);

	info->cancelled = FALSE;

	_tny_account_start_camel_operation (TNY_ACCOUNT_IFACE (priv->account), 
		tny_msg_folder_refresh_headers_async_status, info, str);

	g_free (str);

	camel_folder_refresh_info (priv->folder, ex);
	camel_exception_free (ex);

	info->cancelled = camel_operation_cancel_check (apriv->cancel);

	_tny_account_stop_camel_operation (TNY_ACCOUNT_IFACE (priv->account));

	g_list_foreach (priv->cached_hdrs, destroy_header, NULL);
	g_list_free (priv->cached_hdrs);
	priv->cached_hdrs = NULL;
	g_mutex_unlock (priv->folder_lock);

	if (info->callback)
		g_idle_add_full (G_PRIORITY_HIGH, tny_msg_folder_refresh_headers_async_callback, 
			info, tny_msg_folder_refresh_headers_async_destroyer);


	g_thread_exit (NULL);

	return NULL;
}

static void
tny_msg_folder_refresh_headers_async (TnyMsgFolderIface *self, TnyGetHeadersCallback callback, TnyGetHeadersStatusCallback status_callback, gpointer user_data)
{
	RefreshHeadersInfo *info = g_new0 (RefreshHeadersInfo, 1);
	GThread *thread;
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	info->self = self;
	info->callback = callback;
	info->status_callback = status_callback;
	info->user_data = user_data;

	thread = g_thread_create (tny_msg_folder_refresh_headers_async_thread,
			info, FALSE, NULL);
}

static const GList*
tny_msg_folder_get_headers (TnyMsgFolderIface *self, gboolean refresh)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	load_folder (priv);

	g_mutex_lock (priv->cached_hdrs_lock);

	if (G_UNLIKELY (!priv->cached_hdrs))
	{
		GPtrArray *uids = NULL;
		CamelException ex;
		FldAndPriv *ptr = NULL;

		priv->cached_length = 0;
		priv->cached_hdrs = NULL;
		g_mutex_unlock (priv->cached_hdrs_lock);

		ptr = g_new (FldAndPriv, 1);

		/* Prepare speedup trick */
		ptr->self = self;
		ptr->priv = priv;

		/* This one consumes time (use the async stuff) */

		if (refresh)
			camel_folder_refresh_info (priv->folder, &ex);

		/* */
		uids = camel_folder_get_uids (priv->folder);

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

	} else {
		g_mutex_unlock (priv->cached_hdrs_lock);
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

	g_mutex_lock (priv->cached_msgs_lock);

	if (G_UNLIKELY (!priv->cached_msgs))
	{
		/* Questionable: Do we really want a message cache anyway? */

		priv->cached_msgs = g_hash_table_new_full 
			(g_str_hash, g_str_equal, destroy_cached_key,
			destroy_cached_value);
		g_mutex_unlock (priv->cached_msgs_lock);
	} else {
		message = g_hash_table_lookup (priv->cached_msgs, id);
		g_mutex_unlock (priv->cached_msgs_lock);
	}
	
	if (G_LIKELY (!message))
	{
		CamelException *ex = camel_exception_new ();
		camel_exception_init (ex);

		CamelMimeMessage *camel_message = NULL;

		g_mutex_lock (priv->folder_lock);

		/* TODO: We can reuse the message instance in the header 
		   if not using the summary capabilities. */

		_tny_account_start_camel_operation (TNY_ACCOUNT_IFACE (priv->account), 
					NULL, NULL, NULL);

		camel_message = camel_folder_get_message  
			(priv->folder, (const char *) id, ex);

		_tny_account_stop_camel_operation (TNY_ACCOUNT_IFACE (priv->account));

		g_mutex_unlock (priv->folder_lock);

		if (camel_exception_get_id (ex) == CAMEL_EXCEPTION_NONE)
		{
			message = TNY_MSG_IFACE (tny_msg_new ());

			tny_msg_iface_set_folder (message, self);
			tny_msg_iface_set_header (message, TNY_MSG_HEADER_IFACE (header));
			_tny_msg_set_camel_mime_message (TNY_MSG (message), camel_message);

			g_mutex_lock (priv->cached_msgs_lock);
			g_hash_table_insert (priv->cached_msgs, (gpointer)id, message);
			g_mutex_unlock (priv->cached_msgs_lock);
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
	
	if (G_UNLIKELY (!priv->cached_name))
	{
		load_folder (priv);
		name = camel_folder_get_name (priv->folder);
	} else
		name = priv->cached_name;

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

	if (G_UNLIKELY (priv->folder_name))
		g_free (priv->folder_name);

	priv->folder_name = g_strdup (id);

	return;
}


void
_tny_msg_folder_set_name_priv (TnyMsgFolderIface *self, const gchar *name)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	if (G_UNLIKELY (priv->cached_name))
		g_free (priv->cached_name);

	priv->cached_name = g_strdup (name);

	return;
}

static void
tny_msg_folder_set_name (TnyMsgFolderIface *self, const gchar *name)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	load_folder (priv);

	camel_folder_rename (priv->folder, name);

	if (G_UNLIKELY (priv->cached_name))
		g_free (priv->cached_name);

	priv->cached_name = g_strdup (name);

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

	g_mutex_lock (priv->folder_lock);
	
	if (G_UNLIKELY (priv->folder))
		camel_object_unref (priv->folder);

	camel_object_ref (camel_folder);

	tny_msg_folder_set_id (TNY_MSG_FOLDER_IFACE (self), 
		camel_folder_get_full_name (camel_folder));

	priv->folder = camel_folder;

	g_mutex_unlock (priv->folder_lock);

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
	CamelFolder *retval = NULL;

	g_mutex_lock (priv->folder_lock);
	retval = priv->folder;
	g_mutex_unlock (priv->folder_lock);

	return retval;
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


static int count=0;

static void
destroy_folder (gpointer data, gpointer user_data)
{
	g_object_unref (G_OBJECT (data));
}

static void
tny_msg_folder_finalize (GObject *object)
{
	TnyMsgFolder *self = (TnyMsgFolder*) object;
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (self);

	if (G_LIKELY (priv->folders))
	{
		g_mutex_lock (priv->folders_lock);
		g_list_foreach (priv->folders, destroy_folder, NULL);
		g_mutex_unlock (priv->folders_lock);
	}

	tny_msg_folder_hdr_cache_remover (priv);

	g_mutex_lock (priv->cached_hdrs_lock);
	if (G_LIKELY (priv->folder))
	{
		g_mutex_lock (priv->folder_lock);
		camel_object_unref (priv->folder);
		g_mutex_unlock (priv->folder_lock);
	}
	g_mutex_unlock (priv->cached_hdrs_lock);

	if (G_LIKELY (priv->cached_name))
		g_free (priv->cached_name);



	g_mutex_free (priv->cached_hdrs_lock);
	priv->cached_hdrs_lock = NULL;

	g_mutex_free (priv->cached_msgs_lock);
	priv->cached_msgs_lock = NULL;

	g_mutex_free (priv->folder_lock);
	priv->folder_lock = NULL;

	g_mutex_free (priv->folders_lock);
	priv->folders_lock = NULL;

	(*parent_class->finalize) (object);

	return;
}

static void
tny_msg_folder_uncache (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (self);

	if (G_LIKELY (priv->folder != NULL))
		unload_folder (priv);

	return;
}

static const gboolean
tny_msg_folder_has_cache (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (self);
	gboolean retval;

	g_mutex_lock (priv->folder_lock);
	retval = (priv->folder != NULL);
	g_mutex_unlock (priv->folder_lock);

	return retval;
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
	klass->get_subscribed_func = tny_msg_folder_get_subscribed;
	klass->set_subscribed_func = tny_msg_folder_set_subscribed;
	klass->refresh_headers_async_func = tny_msg_folder_refresh_headers_async;

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

	priv->cached_hdrs_lock = g_mutex_new ();
	priv->cached_msgs_lock = g_mutex_new ();
	priv->folder_lock = g_mutex_new ();
	priv->folders_lock = g_mutex_new ();

	priv->cached_name = NULL;

	return;
}

GType 
tny_msg_folder_get_type (void)
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

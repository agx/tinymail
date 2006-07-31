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

#include <string.h>
#include <tny-msg-folder-iface.h>
#include <tny-msg-folder.h>
#include <tny-msg-iface.h>
#include <tny-msg-header-iface.h>
#include <tny-msg.h>
#include <tny-msg-header.h>
#include <tny-store-account-iface.h>
#include <tny-store-account.h>
#include <tny-list-iface.h>

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
#include "tny-msg-folder-list-priv.h"
#include <tny-camel-shared.h>

#include <camel/camel.h>
#include <camel/camel-folder-summary.h>

static GObjectClass *parent_class = NULL;


static void 
folder_changed (TnyMsgFolderIface *self, CamelFolderChangeInfo *info, gpointer user_data)
{
	/* TnyMsgFolderPriv *priv = user_data; */

	/* When we know a new message got added to this folder */

/*
        info->uid_added
        info->uid_removed
        info->uid_changed
        info->uid_recent
*/

}

static void
unload_folder_no_lock (TnyMsgFolderPriv *priv, gboolean destroy)
{
	if (G_LIKELY (priv->folder))
	{
		if (((CamelObject*)priv->folder)->ref_count)
			if (priv->folder_changed_id != 0)
				camel_object_remove_event(priv->folder, priv->folder_changed_id);

		while (((CamelObject*)priv->folder)->ref_count >= 1)
			camel_object_unref (CAMEL_OBJECT (priv->folder));

		priv->folder = NULL;
	}

	priv->cached_length = 0;
	priv->cached_folder_type = TNY_MSG_FOLDER_TYPE_UNKNOWN;
	priv->loaded = FALSE;

	return;
}

static void 
unload_folder (TnyMsgFolderPriv *priv, gboolean destroy)
{
	g_mutex_lock (priv->folder_lock);
	unload_folder_no_lock (priv, destroy);
	g_mutex_unlock (priv->folder_lock);
}



static void
load_folder_no_lock (TnyMsgFolderPriv *priv)
{
	if (!priv->folder && !priv->loaded)
	{
		CamelFolderInfo *folder_info;
		CamelException ex = CAMEL_EXCEPTION_INITIALISER;
		CamelStore *store = (CamelStore*) _tny_account_get_service 
			(TNY_ACCOUNT (priv->account));

		priv->folder = camel_store_get_folder 
			(store, priv->folder_name, 0, &ex);

		priv->folder_changed_id = camel_object_hook_event (priv->folder, 
			"folder_changed", (CamelObjectEventHookFunc)folder_changed, 
			priv);

		priv->has_summary_cap = camel_folder_has_summary_capability (priv->folder);
		
		if (G_LIKELY (priv->folder) && G_LIKELY (priv->has_summary_cap))
		{
			priv->unread_length = (guint)
				camel_folder_get_unread_message_count (priv->folder);
		}

		priv->loaded = TRUE;
	}
	
	return;
}


static void
load_folder (TnyMsgFolderPriv *priv)
{
	g_mutex_lock (priv->folder_lock);
	load_folder_no_lock (priv);
	g_mutex_unlock (priv->folder_lock);

	return;
}


static void 
tny_msg_folder_remove_message (TnyMsgFolderIface *self, TnyMsgHeaderIface *header)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	const gchar *id;

	g_mutex_lock (priv->folder_lock);

	if (!priv->folder || !priv->loaded)
		load_folder_no_lock (priv);

	id = tny_msg_header_iface_get_uid (TNY_MSG_HEADER_IFACE (header));
	camel_folder_delete_message (priv->folder, id);

	g_mutex_unlock (priv->folder_lock);

	return;
}

static void 
tny_msg_folder_expunge (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;

	g_mutex_lock (priv->folder_lock);

	if (!priv->folder || !priv->loaded)
		load_folder_no_lock (priv);
	
	camel_folder_sync (priv->folder, TRUE, &ex);

	g_mutex_unlock (priv->folder_lock);

	return;
}


CamelFolder*
_tny_msg_folder_get_camel_folder (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	CamelFolder *retval;

	/* g_mutex_lock (priv->folder_lock); */

	if (!priv->folder || !priv->loaded)
		load_folder_no_lock (priv);
	retval = priv->folder;

	/* g_mutex_unlock (priv->folder_lock); */

	return retval;
}

static TnyListIface*
tny_msg_folder_get_folders (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	TnyListIface *retval;

	g_mutex_lock (priv->folder_lock);
	if (!priv->folders)
		priv->folders = _tny_msg_folder_list_new (self);
	retval = (TnyListIface*)priv->folders;
	g_mutex_unlock (priv->folder_lock);

	return retval;
}


static gboolean
tny_msg_folder_get_subscribed (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	gboolean retval;

	g_mutex_lock (priv->folder_lock);
	retval = priv->subscribed;
	g_mutex_unlock (priv->folder_lock);

	return retval;
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
tny_msg_folder_set_subscribed (TnyMsgFolderIface *self, gboolean subscribed)
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

void
_tny_msg_folder_set_unread_count (TnyMsgFolder *self, guint len)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	priv->unread_length = len;
	return;
}

void
_tny_msg_folder_set_all_count (TnyMsgFolder *self, guint len)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	priv->cached_length = len;
	return;
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


static TnyAccountIface*  
tny_msg_folder_get_account (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	return priv->account;
}

static void
tny_msg_folder_set_account (TnyMsgFolderIface *self, TnyAccountIface *account)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	/* No need to reference, would be a cross reference */
	priv->account = TNY_ACCOUNT_IFACE (account);

	return;
}

typedef struct 
{ 	/* This is a speedup trick */
	TnyMsgFolderIface *self;
	TnyMsgFolderPriv *priv;
	TnyListIface *headers;
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
	TnyListIface *headers = ptr->headers;
	CamelFolder *cfol = _tny_msg_folder_get_camel_folder (self);
	CamelMessageInfo *mi = camel_folder_get_message_info (cfol, uid);

	/* TODO: Proxy instantiation (happens a lot, could use a pool) */
	header = TNY_MSG_HEADER_IFACE (tny_msg_header_new ());

	tny_msg_header_iface_set_folder (header, self);
	_tny_msg_header_set_camel_message_info (header, mi);

	/* Get rid of the reference already. I know this is ugly */
	camel_folder_free_message_info (cfol, mi);

	tny_list_iface_prepend (headers, (GObject*)header);
	priv->cached_length++;
	/* TODO: If unread -- priv->unread_length++; */

	return;
}

typedef struct 
{
	TnyMsgFolderIface *self;
	TnyRefreshFolderCallback callback;
	TnyRefreshFolderStatusCallback status_callback;
	gpointer user_data;
	gboolean cancelled;

} RefreshFolderInfo;


static void
destroy_header (gpointer data, gpointer user_data)
{
	g_object_unref (G_OBJECT (data));
	data=NULL;

	return;
}

static void
tny_msg_folder_refresh_async_destroyer (gpointer thr_user_data)
{

	TnyMsgFolderIface *self = ((RefreshFolderInfo*)thr_user_data)->self;

	/* As promised */
	g_object_unref (G_OBJECT (self));

	g_free (thr_user_data);

	return;
}

static gboolean
tny_msg_folder_refresh_async_callback (gpointer thr_user_data)
{
	RefreshFolderInfo *info = thr_user_data;
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (info->self));

	if (info->callback)
		info->callback (info->self, info->cancelled, info->user_data);

	return FALSE;
}


typedef struct
{
	RefreshFolderInfo *minfo;
	gchar *what;
	gint pc;

} ProgressInfo;

static void
destroy_progress_idle (gpointer data)
{
	ProgressInfo *info = data;

	/* As promised */
	g_object_unref (G_OBJECT (info->minfo->self));

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
	RefreshFolderInfo *minfo = info->minfo;
	gint pc = info->pc;

	if (minfo && minfo->status_callback)
	{
		minfo->status_callback (minfo->self, 
			(const gchar*)what, (gint)pc, minfo->user_data);
	}

	return FALSE;
} 


static void
tny_msg_folder_refresh_async_status (struct _CamelOperation *op, const char *what, int pc, void *thr_user_data)
{
	RefreshFolderInfo *oinfo = thr_user_data;
	ProgressInfo *info = g_new0 (ProgressInfo, 1);

	/* Camel will shredder what and thr_user_data, so we need to copy it */

	info->what = g_strdup (what);
	info->minfo = g_new0 (RefreshFolderInfo ,1);
	info->minfo->callback = oinfo->callback;
	info->minfo->cancelled = oinfo->cancelled;
	info->minfo->self = oinfo->self;
	info->minfo->status_callback = oinfo->status_callback;
	info->minfo->user_data = oinfo->user_data;
	info->pc = pc;

	/* Removed in the destroyer */
	g_object_ref (G_OBJECT (info->minfo->self));

	g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
		progress_func, info, destroy_progress_idle);

	return;
}


static gpointer 
tny_msg_folder_refresh_async_thread (gpointer thr_user_data)
{
	RefreshFolderInfo *info = thr_user_data;
	TnyMsgFolderIface *self = info->self;
	gpointer user_data = info->user_data;
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	TnyAccountPriv *apriv = TNY_ACCOUNT_GET_PRIVATE (priv->account);
	gchar *str;
	CamelException *ex = camel_exception_new ();

	camel_exception_init (ex);

	g_mutex_lock (priv->folder_lock);

	load_folder_no_lock (priv); 

	info->cancelled = FALSE;
	str = g_strdup_printf (_("Reading folder `%s'"), priv->folder->full_name);
	_tny_account_start_camel_operation (TNY_ACCOUNT_IFACE (priv->account), 
		tny_msg_folder_refresh_async_status, info, str);
	g_free (str);
	camel_folder_refresh_info (priv->folder, ex);
	if (G_LIKELY (priv->folder) && G_LIKELY (priv->has_summary_cap))
		priv->unread_length = (guint)camel_folder_get_unread_message_count (priv->folder);
	camel_exception_free (ex);
	info->cancelled = camel_operation_cancel_check (apriv->cancel);
	_tny_account_stop_camel_operation (TNY_ACCOUNT_IFACE (priv->account));


	g_mutex_unlock (priv->folder_lock);

	if (info->callback)
	{
		/* Removed in the destroyer */
		g_object_ref (G_OBJECT (self));

		g_idle_add_full (G_PRIORITY_HIGH, 
			tny_msg_folder_refresh_async_callback, 
			info, tny_msg_folder_refresh_async_destroyer);

	}

	g_thread_exit (NULL);

	return NULL;
}

static void
tny_msg_folder_refresh_async (TnyMsgFolderIface *self, TnyRefreshFolderCallback callback, TnyRefreshFolderStatusCallback status_callback, gpointer user_data)
{
	RefreshFolderInfo *info = g_new0 (RefreshFolderInfo, 1);
	GThread *thread;
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	info->self = self;
	info->callback = callback;
	info->status_callback = status_callback;
	info->user_data = user_data;

	thread = g_thread_create (tny_msg_folder_refresh_async_thread,
			info, FALSE, NULL);

	return;
}

static void 
tny_msg_folder_refresh (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	TnyAccountPriv *apriv = TNY_ACCOUNT_GET_PRIVATE (priv->account);
	gchar *str;
	CamelException *ex = camel_exception_new ();

	camel_exception_init (ex);

	g_mutex_lock (priv->folder_lock);

	load_folder_no_lock (priv);

	_tny_account_start_camel_operation (TNY_ACCOUNT_IFACE (priv->account), 
		NULL, NULL, NULL);
	camel_folder_refresh_info (priv->folder, ex);
	if (G_LIKELY (priv->folder) && G_LIKELY (priv->has_summary_cap))
		priv->unread_length = (guint)camel_folder_get_unread_message_count (priv->folder);
	camel_exception_free (ex);
	_tny_account_stop_camel_operation (TNY_ACCOUNT_IFACE (priv->account));

	g_mutex_unlock (priv->folder_lock);

	return;
}

static void
tny_msg_folder_get_headers (TnyMsgFolderIface *self, TnyListIface *headers, gboolean refresh)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	GPtrArray *uids = NULL;
	CamelException ex;
	FldAndPriv *ptr = NULL;


	g_mutex_lock (priv->folder_lock);

	g_object_ref (G_OBJECT (headers));

	load_folder_no_lock (priv);

	ptr = g_new (FldAndPriv, 1);
	ptr->self = self;
	ptr->priv = priv;
	ptr->headers = headers;

	if (refresh)
	{
		camel_folder_refresh_info (priv->folder, &ex);
		if (G_LIKELY (priv->folder) && G_LIKELY (priv->has_summary_cap))
			priv->unread_length = (guint)camel_folder_get_unread_message_count (priv->folder);
	}
	priv->cached_length = 0;
	uids = camel_folder_get_uids (priv->folder);
	g_ptr_array_foreach (uids, add_message_with_uid, ptr);
	g_free (ptr);

	camel_folder_free_uids (priv->folder, uids); 

	g_object_unref (G_OBJECT (headers));

	g_mutex_unlock (priv->folder_lock);


	return;
}


static TnyMsgIface*
tny_msg_folder_get_message (TnyMsgFolderIface *self, TnyMsgHeaderIface *header)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));
	TnyMsgIface *message = NULL;
	const gchar *id;

	g_mutex_lock (priv->folder_lock);

	id = tny_msg_header_iface_get_uid (TNY_MSG_HEADER_IFACE (header));

	load_folder_no_lock (priv);

	CamelException *ex = camel_exception_new ();
	camel_exception_init (ex);

	CamelMimeMessage *camel_message = NULL;


	/* TODO: We can reuse the message instance in the header 
	   if not using the summary capabilities. */

	_tny_account_start_camel_operation (TNY_ACCOUNT_IFACE (priv->account), 
					NULL, NULL, NULL);

	camel_message = camel_folder_get_message  
			(priv->folder, (const char *) id, ex);

	_tny_account_stop_camel_operation (TNY_ACCOUNT_IFACE (priv->account));
	if (camel_exception_get_id (ex) == CAMEL_EXCEPTION_NONE)
	{
		message = TNY_MSG_IFACE (tny_msg_new ());

		_tny_msg_set_folder (message, self);
		tny_msg_iface_set_header (message, TNY_MSG_HEADER_IFACE (header));
		_tny_msg_set_camel_mime_message (TNY_MSG (message), camel_message);
	} else {
		if (camel_message)
			camel_object_unref (CAMEL_OBJECT (camel_message));
	}

	camel_exception_free (ex);

	g_mutex_unlock (priv->folder_lock);

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


static TnyMsgFolderType
tny_msg_folder_get_folder_type (TnyMsgFolderIface *self)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	return priv->cached_folder_type;
}

void
_tny_msg_folder_set_folder_type (TnyMsgFolder *self, TnyMsgFolderType type)
{
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (TNY_MSG_FOLDER (self));

	priv->cached_folder_type = type;

	return;
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

	g_mutex_lock (priv->folder_lock);

	unload_folder_no_lock (priv, TRUE);

	if (G_UNLIKELY (priv->folder_name))
		g_free (priv->folder_name);

	priv->folder_name = g_strdup (id);

	g_mutex_unlock (priv->folder_lock);

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

	
	tny_msg_folder_set_id (TNY_MSG_FOLDER_IFACE (self), 
		camel_folder_get_full_name (camel_folder));

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

static void
destroy_folder (gpointer data, gpointer user_data)
{
	if (data)
		g_object_unref (G_OBJECT (data));
	return;
}

static void
tny_msg_folder_finalize (GObject *object)
{
	TnyMsgFolder *self = (TnyMsgFolder*) object;
	TnyMsgFolderPriv *priv = TNY_MSG_FOLDER_GET_PRIVATE (self);

	g_mutex_lock (priv->folders_lock);
	g_mutex_lock (priv->folder_lock);

	if (G_LIKELY (priv->folders))
	{
		g_object_unref (G_OBJECT (priv->folders));
		priv->folders = NULL;
	}

	unload_folder_no_lock (priv, TRUE);

	if (G_LIKELY (priv->folder))
	{
		camel_object_unref (priv->folder);
		priv->folder = NULL;
	}

	if (G_LIKELY (priv->cached_name))
		g_free (priv->cached_name);
	priv->cached_name = NULL;


	g_mutex_unlock (priv->folder_lock);
	g_mutex_unlock (priv->folders_lock);

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
		unload_folder (priv, FALSE);

	return;
}

static gboolean
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
	klass->get_folder_type_func = tny_msg_folder_get_folder_type;
	klass->has_cache_func = tny_msg_folder_has_cache;
	klass->uncache_func = tny_msg_folder_uncache;
	klass->get_folders_func = tny_msg_folder_get_folders;
	klass->get_unread_count_func = tny_msg_folder_get_unread_count;
	klass->get_all_count_func = tny_msg_folder_get_all_count;
	klass->get_account_func = tny_msg_folder_get_account;
	klass->set_account_func = tny_msg_folder_set_account;
	klass->get_subscribed_func = tny_msg_folder_get_subscribed;
	klass->set_subscribed_func = tny_msg_folder_set_subscribed;
	klass->refresh_async_func = tny_msg_folder_refresh_async;
	klass->refresh_func = tny_msg_folder_refresh;
	klass->remove_message_func = tny_msg_folder_remove_message;
	klass->expunge_func = tny_msg_folder_expunge;

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

	priv->loaded = FALSE;
	priv->folder_changed_id = 0;
	priv->folder = NULL;
	priv->folders = NULL;
	priv->folder_lock = g_mutex_new ();
	priv->folders_lock = g_mutex_new ();
	priv->cached_name = NULL;
	priv->cached_folder_type = TNY_MSG_FOLDER_TYPE_UNKNOWN;
	
	return;
}

GType 
tny_msg_folder_get_type (void)
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

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

#include <tny-folder-store-iface.h>
#include <tny-folder-iface.h>
#include <tny-camel-folder.h>
#include <tny-msg-iface.h>
#include <tny-header-iface.h>
#include <tny-camel-msg.h>
#include <tny-camel-header.h>
#include <tny-store-account-iface.h>
#include <tny-camel-store-account.h>
#include <tny-list-iface.h>

#include <camel/camel-folder.h>
#include <camel/camel.h>
#include <camel/camel-session.h>
#include <camel/camel-store.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <tny-session-camel.h>
#include "tny-camel-account-priv.h"
#include "tny-camel-store-account-priv.h"
#include "tny-camel-folder-priv.h"
#include "tny-camel-header-priv.h"
#include "tny-camel-msg-priv.h"
#include "tny-camel-common-priv.h"
#include <tny-camel-shared.h>

#include <camel/camel-folder-summary.h>

static GObjectClass *parent_class = NULL;


static void 
folder_changed (TnyFolderIface *self, CamelFolderChangeInfo *info, gpointer user_data)
{
	/* TnyFolderPriv *priv = user_data; 
	
	When we know a new message got added to this folder 
	
        info->uid_added
        info->uid_removed
        info->uid_changed
        info->uid_recent */
}

static void
unload_folder_no_lock (TnyCamelFolderPriv *priv, gboolean destroy)
{
	if (G_LIKELY (priv->folder))
	{
		if (((CamelObject*)priv->folder)->ref_count)
			if (priv->folder_changed_id != 0)
				camel_object_remove_event (priv->folder, priv->folder_changed_id);

		while (((CamelObject*)priv->folder)->ref_count >= 1)
			camel_object_unref (CAMEL_OBJECT (priv->folder));

		priv->folder = NULL;
	}

	priv->cached_length = 0;
	priv->cached_folder_type = TNY_FOLDER_TYPE_UNKNOWN;
	priv->loaded = FALSE;

	return;
}

static void 
unload_folder (TnyCamelFolderPriv *priv, gboolean destroy)
{
	g_mutex_lock (priv->folder_lock);
	unload_folder_no_lock (priv, destroy);
	g_mutex_unlock (priv->folder_lock);
}


static void
load_folder_no_lock (TnyCamelFolderPriv *priv)
{
	if (!priv->folder && !priv->loaded)
	{
		CamelException ex = CAMEL_EXCEPTION_INITIALISER;
		CamelStore *store = (CamelStore*) _tny_camel_account_get_service 
			(TNY_CAMEL_ACCOUNT (priv->account));

		priv->folder = camel_store_get_folder 
			(store, priv->folder_name, 0, &ex);

    	    	priv->cached_length = camel_folder_get_message_count (priv->folder);

		priv->folder_changed_id = camel_object_hook_event (priv->folder, 
			"folder_changed", (CamelObjectEventHookFunc)folder_changed, 
			priv);

		priv->has_summary_cap = camel_folder_has_summary_capability (priv->folder);
		
		if (G_LIKELY (priv->folder) && G_LIKELY (priv->has_summary_cap))
			priv->unread_length = (guint)
				camel_folder_get_unread_message_count (priv->folder);

		priv->loaded = TRUE;
	}
	
	return;
}


static void
load_folder (TnyCamelFolderPriv *priv)
{
	g_mutex_lock (priv->folder_lock);
	load_folder_no_lock (priv);
	g_mutex_unlock (priv->folder_lock);

	return;
}


static void 
tny_camel_folder_remove_message (TnyFolderIface *self, TnyHeaderIface *header)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	const gchar *id;

	g_mutex_lock (priv->folder_lock);

	if (!priv->folder || !priv->loaded)
		load_folder_no_lock (priv);

	id = tny_header_iface_get_uid (TNY_HEADER_IFACE (header));
	camel_folder_delete_message (priv->folder, id);

	g_mutex_unlock (priv->folder_lock);

	return;
}

static void 
tny_camel_folder_expunge (TnyFolderIface *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;

	g_mutex_lock (priv->folder_lock);

	if (!priv->folder || !priv->loaded)
		load_folder_no_lock (priv);
	
	camel_folder_sync (priv->folder, TRUE, &ex);

	g_mutex_unlock (priv->folder_lock);

	return;
}


CamelFolder*
_tny_camel_folder_get_camel_folder (TnyCamelFolder *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	CamelFolder *retval;

	if (!priv->folder || !priv->loaded)
		load_folder_no_lock (priv);
	retval = priv->folder;

	return retval;
}


static gboolean
tny_camel_folder_get_subscribed (TnyFolderIface *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	gboolean retval;

	g_mutex_lock (priv->folder_lock);
	retval = priv->subscribed;
	g_mutex_unlock (priv->folder_lock);

	return retval;
}

void
_tny_camel_folder_set_subscribed (TnyCamelFolder *self, gboolean subscribed)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	g_mutex_lock (priv->folder_lock);
	priv->subscribed = subscribed;
	g_mutex_unlock (priv->folder_lock);

	return;
}

static void
tny_camel_folder_set_subscribed (TnyFolderIface *self, gboolean subscribed)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	/* These will synchronize me using _tny_folder_set_subscribed_priv */

	if (G_LIKELY (subscribed))
		tny_store_account_iface_subscribe 
			(TNY_STORE_ACCOUNT_IFACE (priv->account), self);
	else
		tny_store_account_iface_unsubscribe
			(TNY_STORE_ACCOUNT_IFACE (priv->account), self);

	return;
}

static guint
tny_camel_folder_get_unread_count (TnyFolderIface *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	guint retval;

	g_mutex_lock (priv->folder_lock);
	retval = priv->unread_length;
	g_mutex_unlock (priv->folder_lock);

	return retval;
}

void
_tny_camel_folder_set_unread_count (TnyCamelFolder *self, guint len)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	priv->unread_length = len;
	return;
}

void
_tny_camel_folder_set_all_count (TnyCamelFolder *self, guint len)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	priv->cached_length = len;
	return;
}


static guint
tny_camel_folder_get_all_count (TnyFolderIface *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	guint retval;

	g_mutex_lock (priv->folder_lock);
	retval = priv->cached_length;
	g_mutex_unlock (priv->folder_lock);

	return retval;
}


static TnyStoreAccountIface*  
tny_camel_folder_get_account (TnyFolderIface *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	return priv->account;
}

void
_tny_camel_folder_set_account (TnyCamelFolder *self, TnyStoreAccountIface *account)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	priv->account = TNY_STORE_ACCOUNT_IFACE (account);

	return;
}

typedef struct 
{ 	/* This is a speedup trick */
	TnyFolderIface *self;
	TnyCamelFolderPriv *priv;
	TnyListIface *headers;
} FldAndPriv;

static void
add_message_with_uid (gpointer data, gpointer user_data)
{
	TnyHeaderIface *header = NULL;
	FldAndPriv *ptr = user_data;
	const char *uid = (const char*)data;

	/* Unpack speedup trick */
	TnyFolderIface *self = ptr->self;
	TnyCamelFolderPriv *priv = ptr->priv;
	TnyListIface *headers = ptr->headers;
	CamelFolder *cfol = _tny_camel_folder_get_camel_folder (TNY_CAMEL_FOLDER (self));
	CamelMessageInfo *mi = camel_folder_get_message_info (cfol, uid);

	/* TODO: Proxy instantiation (happens a lot, could use a pool) */
	header = tny_camel_header_new ();

	_tny_camel_header_set_folder (TNY_CAMEL_HEADER (header), TNY_CAMEL_FOLDER (self), priv);
	_tny_camel_header_set_camel_message_info (TNY_CAMEL_HEADER (header), mi, FALSE);

	/* Get rid of the reference already. I know this is ugly */
	camel_folder_free_message_info (cfol, mi);

	tny_list_iface_prepend (headers, (GObject*)header);
	g_object_unref (G_OBJECT (header));

	priv->cached_length++;
	/* TODO: If unread -- priv->unread_length++; */

	return;
}

typedef struct 
{
	TnyFolderIface *self;
	TnyRefreshFolderCallback callback;
	TnyRefreshFolderStatusCallback status_callback;
	gpointer user_data;
	gboolean cancelled;
    	guint depth;
} RefreshFolderInfo;



static void
tny_camel_folder_refresh_async_destroyer (gpointer thr_user_data)
{

	TnyFolderIface *self = ((RefreshFolderInfo*)thr_user_data)->self;

	/* gidle reference */
	g_object_unref (G_OBJECT (self));

	g_free (thr_user_data);

	return;
}

static gboolean
tny_camel_folder_refresh_async_callback (gpointer thr_user_data)
{
	RefreshFolderInfo *info = thr_user_data;

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

	/* gidle reference */
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
tny_camel_folder_refresh_async_status (struct _CamelOperation *op, const char *what, int pc, void *thr_user_data)
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

	/* gidle reference */
	g_object_ref (G_OBJECT (info->minfo->self));

    	if (oinfo->depth > 0)
	{
		g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
			progress_func, info, destroy_progress_idle);
	} else {
		progress_func (info);
		destroy_progress_idle (info);
	}
    
	return;
}


static gpointer 
tny_camel_folder_refresh_async_thread (gpointer thr_user_data)
{
	RefreshFolderInfo *info = thr_user_data;
	TnyFolderIface *self = info->self;
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (priv->account);
	gchar *str;
	CamelException *ex = camel_exception_new ();

	camel_exception_init (ex);

	g_mutex_lock (priv->folder_lock);

	load_folder_no_lock (priv); 

	info->cancelled = FALSE;
	str = g_strdup_printf (_("Reading folder `%s'"), priv->folder->full_name);
	_tny_camel_account_start_camel_operation (TNY_CAMEL_ACCOUNT (priv->account), 
		tny_camel_folder_refresh_async_status, info, str);
	g_free (str);
	camel_folder_refresh_info (priv->folder, ex);
       	priv->cached_length = camel_folder_get_message_count (priv->folder);

	if (G_LIKELY (priv->folder) && G_LIKELY (priv->has_summary_cap))
		priv->unread_length = (guint)camel_folder_get_unread_message_count (priv->folder);
	camel_exception_free (ex);
	info->cancelled = camel_operation_cancel_check (apriv->cancel);
	_tny_camel_account_stop_camel_operation (TNY_CAMEL_ACCOUNT (priv->account));

	g_mutex_unlock (priv->folder_lock);

      	/* thread reference */
	g_object_unref (G_OBJECT (self));

	if (info->callback)
	{
		/* gidle reference */
		g_object_ref (G_OBJECT (self));

		if (info->depth > 0)
		{
			g_idle_add_full (G_PRIORITY_HIGH, 
				tny_camel_folder_refresh_async_callback, 
				info, tny_camel_folder_refresh_async_destroyer);
		} else {
			tny_camel_folder_refresh_async_callback (info);
			tny_camel_folder_refresh_async_destroyer (info);
		}
	}
  
	g_thread_exit (NULL);

	return NULL;
}

static void
tny_camel_folder_refresh_async (TnyFolderIface *self, TnyRefreshFolderCallback callback, TnyRefreshFolderStatusCallback status_callback, gpointer user_data)
{
	RefreshFolderInfo *info = g_new0 (RefreshFolderInfo, 1);
	GThread *thread;
    
	info->self = self;
	info->callback = callback;
	info->status_callback = status_callback;
	info->user_data = user_data;
	info->depth = g_main_depth ();
    
	/* thread reference */
	g_object_ref (G_OBJECT (self));

	thread = g_thread_create (tny_camel_folder_refresh_async_thread,
			info, FALSE, NULL);

	return;
}

static void 
tny_camel_folder_refresh (TnyFolderIface *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	CamelException *ex = camel_exception_new ();

	camel_exception_init (ex);

	g_mutex_lock (priv->folder_lock);

	load_folder_no_lock (priv);

	_tny_camel_account_start_camel_operation (TNY_CAMEL_ACCOUNT (priv->account), 
		NULL, NULL, NULL);
	camel_folder_refresh_info (priv->folder, ex);
	_tny_camel_account_stop_camel_operation (TNY_CAMEL_ACCOUNT (priv->account));

    	priv->cached_length = camel_folder_get_message_count (priv->folder);    
	if (G_LIKELY (priv->folder) && G_LIKELY (priv->has_summary_cap))
		priv->unread_length = (guint)camel_folder_get_unread_message_count (priv->folder);
	camel_exception_free (ex);	

	g_mutex_unlock (priv->folder_lock);

	return;
}

static void
tny_camel_folder_get_headers (TnyFolderIface *self, TnyListIface *headers, gboolean refresh)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
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
        	priv->cached_length = camel_folder_get_message_count (priv->folder);
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
tny_camel_folder_get_message (TnyFolderIface *self, TnyHeaderIface *header)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	TnyMsgIface *message = NULL;
    	CamelMimeMessage *camel_message = NULL;
	const gchar *id;
	CamelException *ex = camel_exception_new ();
    
	g_mutex_lock (priv->folder_lock);

	id = tny_header_iface_get_uid (TNY_HEADER_IFACE (header));
	load_folder_no_lock (priv);
	camel_exception_init (ex);

	_tny_camel_account_start_camel_operation (TNY_CAMEL_ACCOUNT (priv->account), 
					NULL, NULL, NULL);
	camel_message = camel_folder_get_message (priv->folder, (const char *) id, ex);    
	_tny_camel_account_stop_camel_operation (TNY_CAMEL_ACCOUNT (priv->account));

	if (camel_exception_get_id (ex) == CAMEL_EXCEPTION_NONE)
	{
	    	TnyHeaderIface *nheader = TNY_HEADER_IFACE (tny_camel_header_new ());
	    
	    	/* I don't reuse the header because that would keep a reference
		   on it. Meaning that the CamelFolder can't be destroyed (the
		   fpriv->headers_managed stuff in tny-header.c). The 
		   TnyCamelHeader type can also work with a CamelMimeMessage, 
		   so why not use that. Right? */
	    
		message = tny_camel_msg_new ();
		
		_tny_camel_msg_set_folder (TNY_CAMEL_MSG (message), self);
		_tny_camel_msg_set_camel_mime_message (TNY_CAMEL_MSG (message), camel_message); 
		/* Also check out tny-msg.c: tny_msg_finalize (read the stupid hack) */
		_tny_camel_header_set_camel_mime_message (TNY_CAMEL_HEADER (nheader), camel_message);
		tny_msg_iface_set_header (message, nheader);
		g_object_unref (G_OBJECT (nheader));  
	    
	} else {
		if (camel_message)
			camel_object_unref (CAMEL_OBJECT (camel_message));
	    	message = NULL;
	}

	camel_exception_free (ex);

	g_mutex_unlock (priv->folder_lock);

	return message;
}


static const gchar*
tny_camel_folder_get_name (TnyFolderIface *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	const gchar *name = NULL;
	
	if (G_UNLIKELY (!priv->cached_name))
	{
		load_folder (priv);
		name = camel_folder_get_name (priv->folder);
	} else
		name = priv->cached_name;

	return name;
}


static TnyFolderType
tny_camel_folder_get_folder_type (TnyFolderIface *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	return priv->cached_folder_type;
}


static const gchar*
tny_camel_folder_get_id (TnyFolderIface *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	return priv->folder_name;
}

void
_tny_camel_folder_set_id (TnyCamelFolder *self, const gchar *id)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	g_mutex_lock (priv->folder_lock);

	unload_folder_no_lock (priv, TRUE);

	if (G_UNLIKELY (priv->folder_name))
		g_free (priv->folder_name);

	priv->folder_name = g_strdup (id);

	g_mutex_unlock (priv->folder_lock);

	return;
}


void
_tny_camel_folder_set_name (TnyCamelFolder *self, const gchar *name)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	if (G_UNLIKELY (priv->cached_name))
		g_free (priv->cached_name);

	priv->cached_name = g_strdup (name);

	return;
}

static void
tny_camel_folder_set_name (TnyFolderIface *self, const gchar *name)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	load_folder (priv);

	camel_folder_rename (priv->folder, name);

	if (G_UNLIKELY (priv->cached_name))
		g_free (priv->cached_name);

	priv->cached_name = g_strdup (name);

	return;
}

/**
 * tny_camel_folder_set_folder:
 * @self: A #TnyCamelFolder object
 * @camel_folder: The #CamelFolder instance to play proxy for
 *
 **/
void
tny_camel_folder_set_folder (TnyCamelFolder *self, CamelFolder *camel_folder)
{
	_tny_camel_folder_set_id (self, camel_folder_get_full_name (camel_folder));
    
	return;
}

/**
 * tny_camel_folder_get_folder:
 * @self: A #TnyCamelFolder object
 *
 * Return value: The CamelFolder instance to play proxy for
 **/
CamelFolder*
tny_camel_folder_get_folder (TnyCamelFolder *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	CamelFolder *retval = NULL;

	g_mutex_lock (priv->folder_lock);
	retval = priv->folder;
	g_mutex_unlock (priv->folder_lock);

	return retval;
}

/**
 * tny_camel_folder_new_with_folder:
 * @camel_folder: CamelFolder instance to play proxy for 
 *
 * The #TnyCamelFolder implementation is actually a proxy for #CamelFolder
 *
 * Return value: A new #TnyFolderIface instance implemented for Camel
 **/
TnyFolderIface*
tny_camel_folder_new_with_folder (CamelFolder *camel_folder)
{
	TnyCamelFolder *self = g_object_new (TNY_TYPE_CAMEL_FOLDER, NULL);
    
	tny_camel_folder_set_folder (self, camel_folder);

	return TNY_FOLDER_IFACE (self);
}


/**
 * tny_camel_folder_new:
 * 
 * The #TnyCamelFolder implementation is actually a proxy for #CamelFolder.
 * You need to set the #CamelFolder after using this constructor using
 * tny_camel_folder_set_folder
 *
 * Return value: A new #TnyFolderIface instance implemented for Camel
 **/
TnyFolderIface*
tny_camel_folder_new (void)
{
	TnyCamelFolder *self = g_object_new (TNY_TYPE_CAMEL_FOLDER, NULL);

	return TNY_FOLDER_IFACE (self);
}


static void
tny_camel_folder_finalize (GObject *object)
{
	TnyCamelFolder *self = (TnyCamelFolder*) object;
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	g_mutex_lock (priv->folder_lock);

    	if (priv->account)
	{
		TnyCamelStoreAccountPriv *apriv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (priv->account);
		apriv->managed_folders = g_list_remove (apriv->managed_folders, self);
	}
    
	if (!priv->iter_parented && priv->iter)
	{
    		CamelStore *store = (CamelStore*) _tny_camel_account_get_service (TNY_CAMEL_ACCOUNT (priv->account));
		camel_store_free_folder_info (store, priv->iter);
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

	g_mutex_free (priv->folder_lock);
	priv->folder_lock = NULL;


	(*parent_class->finalize) (object);

	return;
}

static void
tny_camel_folder_uncache (TnyCamelFolder *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	if (G_LIKELY (priv->folder != NULL))
		unload_folder (priv, FALSE);

	return;
}


void 
_tny_camel_folder_check_uncache (TnyCamelFolder *self, TnyCamelFolderPriv *priv)
{    
	if (priv->headers_managed == 0)
		tny_camel_folder_uncache (self);
}


static void
tny_folder_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyFolderIfaceClass *klass = (TnyFolderIfaceClass *)g_iface;

	klass->get_headers_func = tny_camel_folder_get_headers;
	klass->get_message_func = tny_camel_folder_get_message;
	klass->get_id_func = tny_camel_folder_get_id;
	klass->set_name_func = tny_camel_folder_set_name;
	klass->get_name_func = tny_camel_folder_get_name;
	klass->get_folder_type_func = tny_camel_folder_get_folder_type;
	klass->get_unread_count_func = tny_camel_folder_get_unread_count;
	klass->get_all_count_func = tny_camel_folder_get_all_count;
	klass->get_account_func = tny_camel_folder_get_account;
	klass->get_subscribed_func = tny_camel_folder_get_subscribed;
	klass->set_subscribed_func = tny_camel_folder_set_subscribed;
	klass->refresh_async_func = tny_camel_folder_refresh_async;
	klass->refresh_func = tny_camel_folder_refresh;
	klass->remove_message_func = tny_camel_folder_remove_message;
	klass->expunge_func = tny_camel_folder_expunge;

	return;
}


static void 
tny_camel_folder_remove_folder (TnyFolderStoreIface *self, TnyFolderIface *folder)
{
	/* TODO */
    
       	g_critical ("TODO: The remove_folder method is unimplemented in this TnyFolderStoreIface implementation (TnyFolder)\n");

	return;
}

static TnyFolderIface*
tny_camel_folder_create_folder (TnyFolderStoreIface *self, const gchar *name)
{
	/* TODO */
    
       	g_critical ("TODO: The create_folder method is unimplemented in this TnyFolderStoreIface implementation (TnyFolder)\n");

	return TNY_FOLDER_IFACE (tny_camel_folder_new ());
}


void 
_tny_camel_folder_set_folder_type (TnyCamelFolder *folder, CamelFolderInfo *folder_info)
{
    	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (folder);

	if (!folder_info)
		priv->cached_folder_type = TNY_FOLDER_TYPE_NORMAL;
	else {
		switch (folder_info->flags & CAMEL_FOLDER_TYPE_MASK) 
		{
			case CAMEL_FOLDER_TYPE_INBOX:
				priv->cached_folder_type = TNY_FOLDER_TYPE_INBOX;
			break;
			case CAMEL_FOLDER_TYPE_OUTBOX:
				priv->cached_folder_type = TNY_FOLDER_TYPE_OUTBOX; 
			break;
			case CAMEL_FOLDER_TYPE_TRASH:
				priv->cached_folder_type = TNY_FOLDER_TYPE_TRASH; 
			break;
			case CAMEL_FOLDER_TYPE_JUNK:
				priv->cached_folder_type = TNY_FOLDER_TYPE_JUNK; 
			break;
			case CAMEL_FOLDER_TYPE_SENT:
				priv->cached_folder_type = TNY_FOLDER_TYPE_SENT; 
			break;
			default:
				priv->cached_folder_type = TNY_FOLDER_TYPE_NORMAL;
			break;
		}
	}
}

void 
_tny_camel_folder_set_iter (TnyCamelFolder *folder, CamelFolderInfo *iter)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (folder);
    
	priv->iter = iter;
    	priv->iter_parented = TRUE;
    
	return;
}

static void
tny_camel_folder_get_folders (TnyFolderStoreIface *self, TnyListIface *list, TnyFolderStoreQuery *query)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->get_folders_func (self, list, query);
}

static void 
tny_camel_folder_get_folders_default (TnyFolderStoreIface *self, TnyListIface *list, TnyFolderStoreQuery *query)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	TnyCamelStoreAccountPriv *apriv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (priv->account);
	CamelFolderInfo *iter;

	if (!priv->iter && priv->iter_parented)
	{
		CamelStore *store = (CamelStore*) _tny_camel_account_get_service (TNY_CAMEL_ACCOUNT (priv->account));
		CamelException ex = CAMEL_EXCEPTION_INITIALISER;    
		priv->iter = camel_store_get_folder_info (store, priv->folder_name, 0, &ex);
		priv->iter_parented = FALSE;
	} 
    
	iter = priv->iter;
    
    	if (iter)
	{			
	  iter = iter->child;
		
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
			
			apriv->managed_folders = g_list_prepend (apriv->managed_folders, folder);
			
			_tny_camel_folder_set_account (folder,
						 TNY_STORE_ACCOUNT_IFACE (priv->account));

			tny_list_iface_prepend (list, G_OBJECT (folder));
		}	    
		iter = iter->next;
	  }
	}
    
	return;
}


typedef struct {
    TnyFolderStoreIface *self;
    TnyListIface *list;
    TnyGetFoldersCallback callback;
    TnyFolderStoreQuery *query;
    gpointer user_data;
    guint depth;
} GetFoldersInfo;


static void
tny_camel_folder_get_folders_async_destroyer (gpointer thr_user_data)
{
	GetFoldersInfo *info = thr_user_data;
    
	/* gidle reference */
	g_object_unref (G_OBJECT (info->self));
	g_object_unref (G_OBJECT (info->list));

	g_free (info);

	return;
}

static gboolean
tny_camel_folder_get_folders_async_callback (gpointer thr_user_data)
{
	GetFoldersInfo *info = thr_user_data;

	if (info->callback)
		info->callback (info->self, info->list, info->user_data);

	return FALSE;
}

static gpointer 
tny_camel_folder_get_folders_async_thread (gpointer thr_user_data)
{
	GetFoldersInfo *info = (GetFoldersInfo*) thr_user_data;
    
	tny_folder_store_iface_get_folders (TNY_FOLDER_STORE_IFACE (info->self),
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
				tny_camel_folder_get_folders_async_callback, 
				info, tny_camel_folder_get_folders_async_destroyer);
		} else {
			tny_camel_folder_get_folders_async_callback (info);
			tny_camel_folder_get_folders_async_destroyer (info);
		}
	}


	g_thread_exit (NULL);
    
	return NULL;
}

static void 
tny_camel_folder_get_folders_async (TnyFolderStoreIface *self, TnyListIface *list, TnyGetFoldersCallback callback, TnyFolderStoreQuery *query, gpointer user_data)
{
    	TNY_CAMEL_FOLDER_GET_CLASS (self)->get_folders_async_func (self, list, callback, query, user_data);
}

static void 
tny_camel_folder_get_folders_async_default (TnyFolderStoreIface *self, TnyListIface *list, TnyGetFoldersCallback callback, TnyFolderStoreQuery *query, gpointer user_data)
{
	GetFoldersInfo *info = g_new0 (GetFoldersInfo, 1);
	GThread *thread;

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
    
	thread = g_thread_create (tny_camel_folder_get_folders_async_thread,
			info, FALSE, NULL);    

	return;
}


static void
tny_folder_store_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyFolderStoreIfaceClass *klass = (TnyFolderStoreIfaceClass *)g_iface;

	klass->remove_folder_func = tny_camel_folder_remove_folder;
	klass->create_folder_func = tny_camel_folder_create_folder;
	klass->get_folders_func = tny_camel_folder_get_folders;
	klass->get_folders_async_func = tny_camel_folder_get_folders_async;
					
    	return;
}

static void 
tny_camel_folder_class_init (TnyCamelFolderClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;
	object_class->finalize = tny_camel_folder_finalize;

    	class->get_folders_async_func = tny_camel_folder_get_folders_async_default;
	class->get_folders_func = tny_camel_folder_get_folders_default;
    
	g_type_class_add_private (object_class, sizeof (TnyCamelFolderPriv));

	return;
}



static void
tny_camel_folder_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyCamelFolder *self = (TnyCamelFolder *)instance;
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

    	priv->iter = NULL;
	priv->iter_parented = FALSE;
	priv->headers_managed = 0;
	priv->loaded = FALSE;
	priv->folder_changed_id = 0;
	priv->folder = NULL;
	priv->folder_lock = g_mutex_new ();
	priv->cached_name = NULL;
	priv->cached_folder_type = TNY_FOLDER_TYPE_UNKNOWN;
	
	return;
}

GType 
tny_camel_folder_get_type (void)
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
		  sizeof (TnyCamelFolderClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_camel_folder_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyCamelFolder),
		  0,      /* n_preallocs */
		  tny_camel_folder_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_folder_iface_info = 
		{
		  (GInterfaceInitFunc) tny_folder_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		static const GInterfaceInfo tny_folder_store_iface_info = 
		{
		  (GInterfaceInitFunc) tny_folder_store_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};
	    
		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyCamelFolder",
			&info, 0);

       		g_type_add_interface_static (type, TNY_TYPE_FOLDER_STORE_IFACE, 
			&tny_folder_store_iface_info);
	    
		g_type_add_interface_static (type, TNY_TYPE_FOLDER_IFACE, 
			&tny_folder_iface_info);
	}

	return type;
}

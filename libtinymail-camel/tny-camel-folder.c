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

#include <tny-folder-store.h>
#include <tny-folder.h>
#include <tny-camel-folder.h>
#include <tny-msg.h>
#include <tny-header.h>
#include <tny-camel-msg.h>
#include <tny-camel-header.h>
#include <tny-store-account.h>
#include <tny-camel-store-account.h>
#include <tny-list.h>
#include <tny-error.h>

#include <camel/camel-folder.h>
#include <camel/camel.h>
#include <camel/camel-session.h>
#include <camel/camel-store.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <tny-camel-msg-remove-strategy.h>
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

/*
static void 
folder_changed (TnyFolder *self, CamelFolderChangeInfo *info, gpointer user_data)
{
	TnyFolderPriv *priv = user_data; 
	
	When we know a new message got added to this folder 

	info->uid_added
	info->uid_removed
	info->uid_changed
	info->uid_recent 
}
*/

#ifdef HEALTHY_CHECK
static void
pos_header_check (gpointer data, gpointer udata)
{
	/* Note: TNY_IS_CAMEL_HEADER crashes if there's a message referenced (
	   this might imply that a header is still referenced and therefore not
	   destroyed) .. I don't know, it's definitely a bug. Also check #1 on 
	   the trac's tickets. */

	if (data)
	{
		TnyCamelHeader *hdr = data;
		hdr->healthy = 0;
	}
}
#endif

static void
unload_folder_no_lock (TnyCamelFolderPriv *priv, gboolean destroy)
{

	if (priv->folder && !CAMEL_IS_FOLDER (priv->folder))
	{

#ifdef HEALTHY_CHECK
		g_mutex_lock (priv->poshdr_lock);
		if (priv->possible_headers) 
		{
			g_list_foreach (priv->possible_headers, pos_header_check, NULL);
			g_list_free (priv->possible_headers);
		}
		priv->possible_headers = NULL;
		g_mutex_unlock (priv->poshdr_lock);
#endif

		if (CAMEL_IS_OBJECT (priv->folder))
		{
			g_critical ("Killing invalid CamelObject (should be a Camelfolder) at 0x%x\n", priv->folder);
			while (((CamelObject*)priv->folder)->ref_count >= 1)
				camel_object_unref (CAMEL_OBJECT (priv->folder));
		} else
			g_critical ("Corrupted CamelFolder instance at 0x%x (I can't recover from this state, therefore I will leak)\n", priv->folder);
	}

	if (G_LIKELY (priv->folder) && CAMEL_IS_FOLDER (priv->folder))
	{
		/*
		if (((CamelObject*)priv->folder)->ref_count)
			if (priv->folder_changed_id != 0)
				camel_object_remove_event (priv->folder, priv->folder_changed_id);
		*/

#ifdef HEALTHY_CHECK
		g_mutex_lock (priv->poshdr_lock);
		if (priv->possible_headers) 
		{
			g_list_foreach (priv->possible_headers, pos_header_check, NULL);
			g_list_free (priv->possible_headers);
		}
		priv->possible_headers = NULL;
		g_mutex_unlock (priv->poshdr_lock);
#endif

		while (((CamelObject*)priv->folder)->ref_count >= 1)
			camel_object_unref (CAMEL_OBJECT (priv->folder));

	}

	priv->folder = NULL;
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


static gboolean
load_folder_no_lock (TnyCamelFolderPriv *priv)
{
	if (priv->folder && !CAMEL_IS_FOLDER (priv->folder))
		unload_folder_no_lock (priv, FALSE);

	if (!priv->folder && !priv->loaded && priv->folder_name)
	{
		CamelException ex = CAMEL_EXCEPTION_INITIALISER;
		CamelStore *store = (CamelStore*) _tny_camel_account_get_service 
			(TNY_CAMEL_ACCOUNT (priv->account));

#ifdef HEALTHY_CHECK
		g_mutex_lock (priv->poshdr_lock);
		if (priv->possible_headers)
		{
			g_list_foreach (priv->possible_headers, pos_header_check, NULL);
			g_list_free (priv->possible_headers);
		}
		priv->possible_headers = NULL;
		g_mutex_unlock (priv->poshdr_lock);
#endif

		priv->folder = camel_store_get_folder 
			(store, priv->folder_name, 0, &ex);

		if (!priv->folder || camel_exception_is_set (&ex) || !CAMEL_IS_FOLDER (priv->folder))
		{
			if (priv->folder)
				while (((CamelObject*)priv->folder)->ref_count >= 1)
					camel_object_unref (CAMEL_OBJECT (priv->folder));

			priv->folder = NULL;
			priv->loaded = FALSE;
			return FALSE;
		}

		priv->subscribed = 
			camel_store_folder_subscribed (store,
						       camel_folder_get_full_name (priv->folder));
		priv->cached_length = camel_folder_get_message_count (priv->folder);

		/* priv->folder_changed_id = camel_object_hook_event (priv->folder, 
			"folder_changed", (CamelObjectEventHookFunc)folder_changed, 
			priv); */

		priv->has_summary_cap = camel_folder_has_summary_capability (priv->folder);

		if (G_LIKELY (priv->folder) && G_LIKELY (priv->has_summary_cap))
			priv->unread_length = (guint)
				camel_folder_get_unread_message_count (priv->folder);

		priv->loaded = TRUE;
	}
	
	return TRUE;
}


static gboolean
load_folder (TnyCamelFolderPriv *priv)
{
	gboolean retval;

	g_mutex_lock (priv->folder_lock);
	retval = load_folder_no_lock (priv);
	g_mutex_unlock (priv->folder_lock);

	return retval;
}


static void 
tny_camel_folder_add_msg (TnyFolder *self, TnyMsg *msg, GError **err)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->add_msg_func (self, msg, err);
	return;
}

static void 
tny_camel_folder_add_msg_default (TnyFolder *self, TnyMsg *msg, GError **err)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	CamelMimeMessage *message;
	CamelException *ex;

	g_assert (TNY_IS_CAMEL_MSG (msg));

	g_mutex_lock (priv->folder_lock);

	if (!priv->folder || !priv->loaded || !CAMEL_IS_FOLDER (priv->folder))
		if (!load_folder_no_lock (priv))
		{
			g_mutex_unlock (priv->folder_lock);
			return;
		}

	ex = camel_exception_new ();
	camel_exception_init (ex);

	message = _tny_camel_msg_get_camel_mime_message (TNY_CAMEL_MSG (msg));
	camel_folder_append_message (priv->folder, message, NULL, NULL, ex);

	if (camel_exception_is_set (ex))
	{
		g_set_error (err, TNY_FOLDER_ERROR, 
			TNY_FOLDER_ERROR_ADD_MSG,
			camel_exception_get_description (ex));
	} else {

		/* TODO: emit a folder-msg-inserted signal */
	
	}

	camel_exception_free (ex);

	g_mutex_unlock (priv->folder_lock);

	return;
}


static void 
tny_camel_folder_remove_msg (TnyFolder *self, TnyHeader *header, GError **err)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->remove_msg_func (self, header, err);
	return;
}

static void 
tny_camel_folder_remove_msg_default (TnyFolder *self, TnyHeader *header, GError **err)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	g_assert (TNY_IS_HEADER (header));

	if (!priv->remove_strat)
		return;

	g_mutex_lock (priv->folder_lock);

	if (!priv->folder || !priv->loaded || !CAMEL_IS_FOLDER (priv->folder))
		if (!load_folder_no_lock (priv))
		{
			g_mutex_unlock (priv->folder_lock);
			return;
		}

	tny_msg_remove_strategy_perform_remove (priv->remove_strat, self, header, err);

	g_mutex_unlock (priv->folder_lock);

	return;
}

static void 
tny_camel_folder_expunge (TnyFolder *self, GError **err)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->expunge_func (self, err);
	return;
}

static void 
tny_camel_folder_expunge_default (TnyFolder *self, GError **err)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;

	g_mutex_lock (priv->folder_lock);

	if (!priv->folder || !priv->loaded || !CAMEL_IS_FOLDER (priv->folder))
		if (!load_folder_no_lock (priv))
		{
			g_mutex_unlock (priv->folder_lock);
			return;
		}

	camel_folder_sync (priv->folder, TRUE, &ex);

	if (camel_exception_is_set (&ex))
	{
		g_set_error (err, TNY_FOLDER_ERROR, 
			TNY_FOLDER_ERROR_EXPUNGE,
			camel_exception_get_description (&ex));
	}

	g_mutex_unlock (priv->folder_lock);

	return;
}


CamelFolder*
_tny_camel_folder_get_camel_folder (TnyCamelFolder *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	CamelFolder *retval;

	if (!priv->folder || !priv->loaded || !CAMEL_IS_FOLDER (priv->folder))
		if (!load_folder_no_lock (priv))
			return NULL;

	retval = priv->folder;

	return retval;
}


static gboolean
tny_camel_folder_is_subscribed (TnyFolder *self)
{
	return TNY_CAMEL_FOLDER_GET_CLASS (self)->is_subscribed_func (self);
}

static gboolean
tny_camel_folder_is_subscribed_default (TnyFolder *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	gboolean retval;

	g_mutex_lock (priv->folder_lock);

	if (!priv->folder || !priv->loaded || !CAMEL_IS_FOLDER (priv->folder))
	{
		CamelStore *store;
		CamelFolder *cfolder;

		if (!load_folder_no_lock (priv)) 
		{
			g_mutex_unlock (priv->folder_lock);
			return;
		}
		store = (CamelStore*) _tny_camel_account_get_service 
			(TNY_CAMEL_ACCOUNT (priv->account));
		cfolder = _tny_camel_folder_get_camel_folder (TNY_CAMEL_FOLDER (self));
		priv->subscribed = camel_store_folder_subscribed (store, 
								  camel_folder_get_full_name (cfolder));
	}

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


static guint
tny_camel_folder_get_unread_count (TnyFolder *self)
{
	return TNY_CAMEL_FOLDER_GET_CLASS (self)->get_unread_count_func (self);
}

static guint
tny_camel_folder_get_unread_count_default (TnyFolder *self)
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
tny_camel_folder_get_all_count (TnyFolder *self)
{
	return TNY_CAMEL_FOLDER_GET_CLASS (self)->get_all_count_func (self);
}

static guint
tny_camel_folder_get_all_count_default (TnyFolder *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	guint retval;

	g_mutex_lock (priv->folder_lock);
	retval = priv->cached_length;
	g_mutex_unlock (priv->folder_lock);

	return retval;
}


static TnyStoreAccount*  
tny_camel_folder_get_account (TnyFolder *self)
{
	return TNY_CAMEL_FOLDER_GET_CLASS (self)->get_account_func (self);
}

static TnyStoreAccount*  
tny_camel_folder_get_account_default (TnyFolder *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	return TNY_STORE_ACCOUNT (g_object_ref (priv->account));
}

void
_tny_camel_folder_set_account (TnyCamelFolder *self, TnyStoreAccount *account)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	g_assert (TNY_IS_CAMEL_STORE_ACCOUNT (account));

	priv->account = TNY_STORE_ACCOUNT (account);

	return;
}

typedef struct 
{ 	/* This is a speedup trick */
	TnyFolder *self;
	TnyCamelFolderPriv *priv;
	TnyList *headers;
} FldAndPriv;

static void
add_message_with_uid (gpointer data, gpointer user_data)
{
	TnyHeader *header = NULL;
	FldAndPriv *ptr = user_data;
	const char *uid = (const char*)data;

	/* Unpack speedup trick */
	TnyFolder *self = ptr->self;
	TnyCamelFolderPriv *priv = ptr->priv;
	TnyList *headers = ptr->headers;
	CamelFolder *cfol = _tny_camel_folder_get_camel_folder (TNY_CAMEL_FOLDER (self));
	CamelMessageInfo *mi = camel_folder_get_message_info (cfol, uid);
	CamelMessageFlags flags = camel_message_info_flags (mi);

	/* TODO: Proxy instantiation (happens a lot, could use a pool) */
	header = tny_camel_header_new ();

	_tny_camel_header_set_folder (TNY_CAMEL_HEADER (header), TNY_CAMEL_FOLDER (self), priv);
	_tny_camel_header_set_camel_message_info (TNY_CAMEL_HEADER (header), mi, FALSE);

	/* Get rid of the reference already. I know this is ugly */
	camel_folder_free_message_info (cfol, mi);

	tny_list_prepend (headers, (GObject*)header);

#ifdef HEALTHY_CHECK
	g_mutex_lock (priv->poshdr_lock);
	priv->possible_headers = g_list_prepend (priv->possible_headers, header);    
	g_mutex_unlock (priv->poshdr_lock);
#endif

	if (!(flags & CAMEL_MESSAGE_SEEN))
		priv->unread_length++;

	g_object_unref (G_OBJECT (header));

	priv->cached_length++;

	return;
}

typedef struct 
{
	TnyFolder *self;
	TnyRefreshFolderCallback callback;
	TnyRefreshFolderStatusCallback status_callback;
	gpointer user_data;
	gboolean cancelled;
	guint depth;
	GError *err;
} RefreshFolderInfo;



static void
tny_camel_folder_refresh_async_destroyer (gpointer thr_user_data)
{
	RefreshFolderInfo *info = thr_user_data;
	TnyFolder *self = info->self;

	/* gidle reference */
	g_object_unref (G_OBJECT (self));
	if (info->err)
		g_error_free (info->err);

	g_slice_free (RefreshFolderInfo, thr_user_data);

	return;
}

static gboolean
tny_camel_folder_refresh_async_callback (gpointer thr_user_data)
{
	RefreshFolderInfo *info = thr_user_data;

	if (info->callback)
		info->callback (info->self, info->cancelled, &info->err, info->user_data);

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

	g_slice_free (RefreshFolderInfo, info->minfo);
	g_slice_free (ProgressInfo, data);

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


/**
 * tny_camel_folder_refresh_async_status:
 *
 * This is non-public API documentation
 *
 * The reason why we need to copy thist stuff is because it seems Camel has some
 * of its stuff allocated on the stack. When you do g_idle tricks, it's possible
 * that by the time the g_idle happens, the stack allocation is already killed.
 *
 * Another possibility is that Camel simply by then has freed the memory of it.
 * You simply copy stuff, it's a little bit dirty, but just make sure that in 
 * the idle callback you free it, right? Leaking wouldn't be smart here.
 **/
static void
tny_camel_folder_refresh_async_status (struct _CamelOperation *op, const char *what, int pc, void *thr_user_data)
{
	RefreshFolderInfo *oinfo = thr_user_data;
	ProgressInfo *info = g_slice_new (ProgressInfo);

	/* Camel will shredder what and thr_user_data, so we need to copy it */

	info->what = g_strdup (what);
	info->minfo = g_slice_new (RefreshFolderInfo);
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
	TnyFolder *self = info->self;
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (priv->account);
	gchar *str;
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	GError *err = NULL;

	g_mutex_lock (priv->folder_lock);

	if (!load_folder_no_lock (priv))
	{
		tny_camel_folder_refresh_async_destroyer (info);
		g_mutex_unlock (priv->folder_lock);
		g_thread_exit (NULL);
		return NULL;
	}

	info->cancelled = FALSE;
	str = g_strdup_printf (_("Reading folder `%s'"), priv->folder->full_name);
	_tny_camel_account_start_camel_operation (TNY_CAMEL_ACCOUNT (priv->account), 
		tny_camel_folder_refresh_async_status, info, str);
	g_free (str);
	camel_folder_refresh_info (priv->folder, &ex);

	info->err = NULL;

	if (camel_exception_is_set (&ex))
	{
		g_set_error (&err, TNY_FOLDER_ERROR, 
			TNY_FOLDER_ERROR_REFRESH,
			camel_exception_get_description (&ex));
		if (err != NULL)
			info->err = g_error_copy ((const GError *) &err);
	}

	priv->cached_length = camel_folder_get_message_count (priv->folder);

	if (G_LIKELY (priv->folder) && CAMEL_IS_FOLDER (priv->folder) && G_LIKELY (priv->has_summary_cap))
		priv->unread_length = (guint)camel_folder_get_unread_message_count (priv->folder);

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
tny_camel_folder_refresh_async (TnyFolder *self, TnyRefreshFolderCallback callback, TnyRefreshFolderStatusCallback status_callback, gpointer user_data)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->refresh_async_func (self, callback, status_callback, user_data);
	return;
}

/**
 * tny_camel_folder_refresh_async_default:
 *
 * This is non-public API documentation
 *
 * It's actually very simple: just store all the interesting info in a struct 
 * launch a thread and keep that struct-instance around. In the callbacks,
 * which you stored as function pointers, recover that info and pass it to the
 * user of the _async method.
 *
 * Important is to add and remove references. You don't want the reference to
 * become zero while doing stuff on the instance in the background, don't you?
 **/
static void
tny_camel_folder_refresh_async_default (TnyFolder *self, TnyRefreshFolderCallback callback, TnyRefreshFolderStatusCallback status_callback, gpointer user_data)
{
	RefreshFolderInfo *info = g_slice_new (RefreshFolderInfo);
	GThread *thread;

	info->err = NULL;
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
tny_camel_folder_refresh (TnyFolder *self, GError **err)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->refresh_func (self, err);
	return;
}

static void 
tny_camel_folder_refresh_default (TnyFolder *self, GError **err)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	CamelException *ex = camel_exception_new ();

	camel_exception_init (ex);

	g_mutex_lock (priv->folder_lock);

	if (!load_folder_no_lock (priv))
	{
		g_mutex_unlock (priv->folder_lock);
		camel_exception_free (ex);
		return;
	}

	_tny_camel_account_start_camel_operation (TNY_CAMEL_ACCOUNT (priv->account), 
		NULL, NULL, NULL);
	camel_folder_refresh_info (priv->folder, ex);
	_tny_camel_account_stop_camel_operation (TNY_CAMEL_ACCOUNT (priv->account));

	priv->cached_length = camel_folder_get_message_count (priv->folder);    
	if (G_LIKELY (priv->folder) && CAMEL_IS_FOLDER (priv->folder) && G_LIKELY (priv->has_summary_cap))
		priv->unread_length = (guint)camel_folder_get_unread_message_count (priv->folder);

	if (camel_exception_is_set (ex))
	{
		g_set_error (err, TNY_FOLDER_ERROR, 
			TNY_FOLDER_ERROR_REFRESH,
			camel_exception_get_description (ex));
	}

	camel_exception_free (ex);

	g_mutex_unlock (priv->folder_lock);

	return;
}

static void
tny_camel_folder_get_headers (TnyFolder *self, TnyList *headers, gboolean refresh, GError **err)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->get_headers_func (self, headers, refresh, err);
	return;
}

static void
tny_camel_folder_get_headers_default (TnyFolder *self, TnyList *headers, gboolean refresh, GError **err)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	GPtrArray *uids = NULL;
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	FldAndPriv *ptr = NULL;

	g_assert (TNY_IS_LIST (headers));

	g_mutex_lock (priv->folder_lock);

	if (!load_folder_no_lock (priv))
	{
		g_mutex_unlock (priv->folder_lock);
		return;
	}

	g_object_ref (G_OBJECT (headers));

	ptr = g_slice_new (FldAndPriv);
	ptr->self = self;
	ptr->priv = priv;
	ptr->headers = headers;

	if (refresh && priv->folder && CAMEL_IS_FOLDER (priv->folder))
	{
		camel_folder_refresh_info (priv->folder, &ex);

		if (camel_exception_is_set (&ex))
		{
			g_set_error (err, TNY_FOLDER_ERROR, 
				TNY_FOLDER_ERROR_REFRESH,
				camel_exception_get_description (&ex));
		} else 
		{
			priv->cached_length = camel_folder_get_message_count (priv->folder);
			if (G_LIKELY (priv->folder) && CAMEL_IS_FOLDER (priv->folder) && G_LIKELY (priv->has_summary_cap))
				priv->unread_length = (guint)camel_folder_get_unread_message_count (priv->folder);
		}
	}


	if (priv->folder && CAMEL_IS_FOLDER (priv->folder))
		uids = camel_folder_get_uids (priv->folder);

	if (uids)
	{
		priv->cached_length = 0;
		priv->unread_length = 0;
		g_ptr_array_foreach (uids, add_message_with_uid, ptr);
	}

	g_slice_free (FldAndPriv, ptr);

	if (uids)
		camel_folder_free_uids (priv->folder, uids); 

	g_object_unref (G_OBJECT (headers));
	g_mutex_unlock (priv->folder_lock);

	return;
}


static TnyMsg*
tny_camel_folder_get_msg (TnyFolder *self, TnyHeader *header, GError **err)
{
	return TNY_CAMEL_FOLDER_GET_CLASS (self)->get_msg_func (self, header, err);
}

static TnyMsg*
tny_camel_folder_get_msg_default (TnyFolder *self, TnyHeader *header, GError **err)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	TnyMsg *message = NULL;
	CamelMimeMessage *camel_message = NULL;
	const gchar *id;
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;

	g_assert (TNY_IS_HEADER (header));

	g_mutex_lock (priv->folder_lock);

	id = tny_header_get_uid (TNY_HEADER (header));

	if (!load_folder_no_lock (priv))
	{
		g_mutex_unlock (priv->folder_lock);
		return;
	}

	message = NULL;
	camel_message = camel_folder_get_message (priv->folder, (const char *) id, &ex);

	if (camel_exception_is_set (&ex))
	{
		g_set_error (err, TNY_FOLDER_ERROR, 
			TNY_FOLDER_ERROR_GET_MSG,
			camel_exception_get_description (&ex));
	} else 
	{
		if (camel_message && CAMEL_IS_OBJECT (camel_message))
		{
			TnyCamelHeader *nheader = TNY_CAMEL_HEADER (tny_camel_header_new ());

			message = tny_camel_msg_new ();
			_tny_camel_msg_set_folder (TNY_CAMEL_MSG (message), self);
			_tny_camel_mime_part_set_part (TNY_CAMEL_MIME_PART (message), 
				CAMEL_MIME_PART (camel_message)); 
			_tny_camel_header_set_camel_mime_message (nheader, camel_message);
			_tny_camel_msg_set_header (TNY_CAMEL_MSG (message), nheader);
			g_object_unref (G_OBJECT (nheader));
		}
	}

	if (camel_message && CAMEL_IS_OBJECT (camel_message))
		camel_object_unref (CAMEL_OBJECT (camel_message));

	g_mutex_unlock (priv->folder_lock);

	return message;
}


static const gchar*
tny_camel_folder_get_name (TnyFolder *self)
{
	return TNY_CAMEL_FOLDER_GET_CLASS (self)->get_name_func (self);
}

static const gchar*
tny_camel_folder_get_name_default (TnyFolder *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	const gchar *name = NULL;

	if (G_UNLIKELY (!priv->cached_name))
	{
		if (!load_folder (priv))
			return NULL;
		name = camel_folder_get_name (priv->folder);
	} else
		name = priv->cached_name;

	return name;
}

static TnyFolderType
tny_camel_folder_get_folder_type (TnyFolder *self)
{
	return TNY_CAMEL_FOLDER_GET_CLASS (self)->get_folder_type_func (self);
}

static TnyFolderType
tny_camel_folder_get_folder_type_default (TnyFolder *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	return priv->cached_folder_type;
}

static const gchar*
tny_camel_folder_get_id (TnyFolder *self)
{
	return TNY_CAMEL_FOLDER_GET_CLASS (self)->get_id_func (self);
}

static const gchar*
tny_camel_folder_get_id_default (TnyFolder *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	return priv->folder_name;
}

static void
tny_camel_folder_transfer_msgs (TnyFolder *self, TnyList *headers, TnyFolder *folder_dst, gboolean delete_originals, GError **err)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->transfer_msgs_func (self, headers, folder_dst, delete_originals, err);
}

/* TODO: provide an _async version of this. Requested by djcb. */
static void
tny_camel_folder_transfer_msgs_default (TnyFolder *self, TnyList *headers, TnyFolder *folder_dst, gboolean delete_originals, GError **err)
{
	TnyFolder *folder_src = self;
	TnyCamelFolderPriv *priv_src, *priv_dst;
	TnyIterator *iter;
	CamelException *ex;
	CamelFolder *cfol_src, *cfol_dst;
	GPtrArray *uids, *transferred_uids = NULL;
	guint list_length;

	g_assert (TNY_IS_LIST (headers));
	g_assert (TNY_IS_FOLDER (folder_src));
	g_assert (TNY_IS_FOLDER (folder_dst));

	list_length = tny_list_get_length (headers);

	if (list_length < 1) 
		return;

	/* Get privates */
	priv_src = TNY_CAMEL_FOLDER_GET_PRIVATE (folder_src);
	priv_dst = TNY_CAMEL_FOLDER_GET_PRIVATE (folder_dst);

	g_mutex_lock (priv_src->folder_lock);
	g_mutex_lock (priv_dst->folder_lock);

	/* Get camel folders */
	cfol_src = _tny_camel_folder_get_camel_folder (TNY_CAMEL_FOLDER (folder_src));
	cfol_dst = _tny_camel_folder_get_camel_folder (TNY_CAMEL_FOLDER (folder_dst));

	/* Create uids */
	uids = g_ptr_array_sized_new (list_length);
	iter = tny_list_create_iterator (headers);

	while (!tny_iterator_is_done (iter)) 
	{
		TnyHeader *header;

		header = TNY_HEADER (tny_iterator_get_current (iter));
		g_ptr_array_add (uids, (gpointer) tny_header_get_uid (header));
		g_object_unref (G_OBJECT (header));
		tny_iterator_next (iter);
	}

	ex = camel_exception_new ();
	camel_exception_init (ex);

	camel_folder_transfer_messages_to (cfol_src, uids, cfol_dst, 
			&transferred_uids, delete_originals, ex);

	if (camel_exception_is_set (ex)) 
	{
		g_set_error (err, TNY_FOLDER_ERROR, 
			TNY_FOLDER_ERROR_TRANSFER_MSGS,
			camel_exception_get_description (ex));
	} else 
	{
		if (delete_originals)
			camel_folder_sync (cfol_src, TRUE, ex);
	
		if (camel_exception_is_set (ex))
			g_warning ("Expunging messages failed: %s\n",
				   camel_exception_get_description (ex));
	}

	camel_exception_free (ex);

	if (transferred_uids) 
		g_ptr_array_free (transferred_uids, FALSE);

	g_ptr_array_free (uids, FALSE);

	g_mutex_unlock (priv_dst->folder_lock);
	g_mutex_unlock (priv_src->folder_lock);

	return;
}


void
_tny_camel_folder_set_id (TnyCamelFolder *self, const gchar *id)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	g_mutex_lock (priv->folder_lock);

	/* unload_folder_no_lock (priv, TRUE); */

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
tny_camel_folder_set_name (TnyFolder *self, const gchar *name, GError **err)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->set_name_func (self, name, err);
	return;
}

static void
tny_camel_folder_set_name_default (TnyFolder *self, const gchar *name, GError **err)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	gchar *full_name;
	CamelFolder *cfolder;
	CamelFolderInfo *parent_info;
	const gchar *old_path;
	gchar *new_path;
	CamelException ex;

	g_mutex_lock (priv->folder_lock);

	if (!priv->folder || !priv->loaded || !CAMEL_IS_FOLDER (priv->folder))
		if (!load_folder_no_lock (priv)) 
		{
			g_mutex_unlock (priv->folder_lock);
			return;
		}

	if (!priv->iter || !priv->iter_parented)
		return;

	/* Create new full name */
	cfolder = _tny_camel_folder_get_camel_folder (TNY_CAMEL_FOLDER (self));
	old_path = camel_folder_get_full_name (cfolder);
	parent_info = priv->iter->parent;
	new_path = g_strdup_printf ("%s/%s", parent_info->name, name);

	/* Check that the name really changes */
	if (!strcmp (old_path, new_path)) 
	{
		g_free (new_path);
		return;
	}

	/* Rename folder */
	camel_exception_init (&ex);
	camel_store_rename_folder (cfolder->parent_store, old_path, (const gchar *) new_path, &ex);
	g_free (new_path);

	if (camel_exception_is_set (&ex))
	{
		g_set_error (err, TNY_FOLDER_ERROR, 
			TNY_FOLDER_ERROR_SET_NAME,
			camel_exception_get_description (&ex));

		camel_exception_clear (&ex);

	} else if (priv->cached_name)
	{
		g_free (priv->cached_name);
		priv->cached_name = g_strdup (name);
	}

	g_mutex_unlock (priv->folder_lock);
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


static TnyMsgRemoveStrategy* 
tny_camel_folder_get_msg_remove_strategy (TnyFolder *self)
{
	return TNY_CAMEL_FOLDER_GET_CLASS (self)->get_msg_remove_strategy_func (self);
}

static TnyMsgRemoveStrategy* 
tny_camel_folder_get_msg_remove_strategy_default (TnyFolder *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	return TNY_MSG_REMOVE_STRATEGY (g_object_ref (G_OBJECT (priv->remove_strat)));
}

static void 
tny_camel_folder_set_msg_remove_strategy (TnyFolder *self, TnyMsgRemoveStrategy *st)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->set_msg_remove_strategy_func (self, st);
	return;
}

static void 
tny_camel_folder_set_msg_remove_strategy_default (TnyFolder *self, TnyMsgRemoveStrategy *st)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	if (priv->remove_strat)
		g_object_unref (G_OBJECT (priv->remove_strat));

	priv->remove_strat = TNY_MSG_REMOVE_STRATEGY (g_object_ref (G_OBJECT (st)));

	return;
}



static void 
tny_camel_folder_remove_folder (TnyFolderStore *self, TnyFolder *folder, GError **err)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->remove_folder_func (self, folder, err);
}

static void 
tny_camel_folder_remove_folder_default (TnyFolderStore *self, TnyFolder *folder, GError **err)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	CamelStore *store = (CamelStore*) _tny_camel_account_get_service 
		(TNY_CAMEL_ACCOUNT (priv->account));
	TnyCamelFolder *cfol = TNY_CAMEL_FOLDER (folder);
	TnyCamelFolderPriv *cpriv = TNY_CAMEL_FOLDER_GET_PRIVATE (cfol);
	gchar *cfolname; gchar *folname; gint parlen;
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;    

	if (!cpriv->folder_name || !priv->folder_name)
		return;

	g_mutex_lock (priv->folder_lock);
	g_mutex_lock (cpriv->folder_lock);

	cfolname = cpriv->folder_name;
	folname = priv->folder_name;
	parlen = strlen (folname);

	/* /INBOX/test      *
	 * /INBOX/test/test */

	if (!strncmp (folname, cfolname, parlen))
	{
		gchar *ccfoln = cfolname + parlen;
		if ((*ccfoln == '/') && (strrchr (ccfoln, '/') == ccfoln))
		{
			camel_store_delete_folder (store, cfolname, &ex);

			if (camel_exception_is_set (&ex))
			{
				g_set_error (err, TNY_FOLDER_STORE_ERROR, 
					TNY_FOLDER_STORE_ERROR_REMOVE_FOLDER,
					camel_exception_get_description (&ex));
				camel_exception_clear (&ex);
			} else {
				g_free (cpriv->folder_name); 
				cpriv->folder_name = NULL;
			}
		}
	}

	g_mutex_unlock (cpriv->folder_lock);
	g_mutex_unlock (priv->folder_lock);

	return;
}

static void
tny_camel_folder_set_folder_info (TnyFolderStore *self, TnyCamelFolder *folder, CamelFolderInfo *info)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	TnyCamelStoreAccountPriv *apriv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (priv->account);

	_tny_camel_folder_set_id (folder, info->full_name);
	_tny_camel_folder_set_folder_type (folder, info);
	_tny_camel_folder_set_unread_count (folder, info->unread);
	_tny_camel_folder_set_all_count (folder, info->total);
	_tny_camel_folder_set_name (folder, info->name);
	_tny_camel_folder_set_iter (folder, info);
	
	apriv->managed_folders = g_list_prepend (apriv->managed_folders, folder);
	_tny_camel_folder_set_account (folder, TNY_STORE_ACCOUNT (priv->account));
}

static TnyFolder*
tny_camel_folder_create_folder (TnyFolderStore *self, const gchar *name, GError **err)
{
	return TNY_CAMEL_FOLDER_GET_CLASS (self)->create_folder_func (self, name, err);
}


static TnyFolder*
tny_camel_folder_create_folder_default (TnyFolderStore *self, const gchar *name, GError **err)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	CamelStore *store; gchar *folname;
	TnyFolder *folder; CamelFolderInfo *info;

	if (!name || strlen (name) <= 0)
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_CREATE_FOLDER,
				_("Failed to create folder with no name"));

		return NULL;
	}

	if (!priv->folder_name)
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_CREATE_FOLDER,
				_("Failed to create folder %s"), name);

		return NULL;
	}

	store = (CamelStore*) _tny_camel_account_get_service (TNY_CAMEL_ACCOUNT (priv->account));

	g_assert (CAMEL_IS_STORE (store));

	g_mutex_lock (priv->folder_lock);
	folname = priv->folder_name;
	info = camel_store_create_folder (store, priv->folder_name, name, &ex);
	g_mutex_unlock (priv->folder_lock);

	if (camel_exception_is_set (&ex)) 
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_CREATE_FOLDER,
				camel_exception_get_description (&ex));
		camel_exception_clear (&ex);

		if (info)
			camel_store_free_folder_info (store, info);

		return NULL;
	}

	folder = tny_camel_folder_new ();
	tny_camel_folder_set_folder_info (self, TNY_CAMEL_FOLDER (folder), info);
	_tny_camel_folder_set_subscribed (TNY_CAMEL_FOLDER (folder), FALSE);

	return folder;
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
tny_camel_folder_get_folders (TnyFolderStore *self, TnyList *list, TnyFolderStoreQuery *query, GError **err)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->get_folders_func (self, list, query, err);
}

static void 
tny_camel_folder_get_folders_default (TnyFolderStore *self, TnyList *list, TnyFolderStoreQuery *query, GError **err)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	TnyCamelStoreAccountPriv *apriv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (priv->account);
	CamelFolderInfo *iter;

	g_assert (priv->folder_name != NULL);

	if (!priv->iter && priv->iter_parented)
	{
		CamelStore *store = (CamelStore*) _tny_camel_account_get_service (TNY_CAMEL_ACCOUNT (priv->account));
		CamelException ex = CAMEL_EXCEPTION_INITIALISER;

		priv->iter = camel_store_get_folder_info (store, priv->folder_name, 0, &ex);

		if (camel_exception_is_set (&ex))
		{
			g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_GET_FOLDERS,
				camel_exception_get_description (&ex));
			camel_exception_clear (&ex);
			return;
		}

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
		    
			tny_camel_folder_set_folder_info (self, folder, iter);

			tny_list_prepend (list, G_OBJECT (folder));
		}
		iter = iter->next;
	  }
	}

	return;
}


typedef struct 
{
	GError *err;
	TnyFolderStore *self;
	TnyList *list;
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

	if (info->err)
		g_error_free (info->err);

	g_slice_free (GetFoldersInfo, info);

	return;
}

static gboolean
tny_camel_folder_get_folders_async_callback (gpointer thr_user_data)
{
	GetFoldersInfo *info = thr_user_data;

	if (info->callback)
		info->callback (info->self, info->list, &info->err, info->user_data);

	return FALSE;
}

static gpointer 
tny_camel_folder_get_folders_async_thread (gpointer thr_user_data)
{
	GetFoldersInfo *info = (GetFoldersInfo*) thr_user_data;
	GError *err;

	tny_folder_store_get_folders (TNY_FOLDER_STORE (info->self),
		info->list, info->query, &err);

	if (err != NULL)
		info->err = g_error_copy ((const GError *) &err);
	else
		info->err = NULL;

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
tny_camel_folder_get_folders_async (TnyFolderStore *self, TnyList *list, TnyGetFoldersCallback callback, TnyFolderStoreQuery *query, gpointer user_data)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->get_folders_async_func (self, list, callback, query, user_data);
}

static void 
tny_camel_folder_get_folders_async_default (TnyFolderStore *self, TnyList *list, TnyGetFoldersCallback callback, TnyFolderStoreQuery *query, gpointer user_data)
{
	GetFoldersInfo *info = g_slice_new (GetFoldersInfo);
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


void
_tny_camel_folder_set_folder (TnyCamelFolder *self, CamelFolder *camel_folder)
{
	_tny_camel_folder_set_id (self, camel_folder_get_full_name (camel_folder));

	return;
}

/**
 * tny_camel_folder_get_folder:
 * @self: A #TnyCamelFolder object
 *
 * You must unreference the returned value
 *
 * Return value: The CamelFolder instance to play proxy for
 **/
CamelFolder*
tny_camel_folder_get_folder (TnyCamelFolder *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	CamelFolder *retval = NULL;

	/* g_mutex_lock (priv->folder_lock); */
	if (G_UNLIKELY (!priv->loaded))
		if (!load_folder (priv))
			return NULL;
	
	retval = priv->folder;
	if (retval)
		camel_object_ref (CAMEL_OBJECT (retval));
	/* g_mutex_unlock (priv->folder_lock); */

	return retval;
}

/**
 * tny_camel_folder_new_with_folder:
 * @camel_folder: CamelFolder instance to play proxy for 
 *
 * The #TnyCamelFolder implementation is actually a proxy for #CamelFolder
 *
 * Return value: A new #TnyFolder instance implemented for Camel
 **/
TnyFolder*
tny_camel_folder_new_with_folder (CamelFolder *camel_folder)
{
	TnyCamelFolder *self = g_object_new (TNY_TYPE_CAMEL_FOLDER, NULL);

	_tny_camel_folder_set_folder (self, camel_folder);

	return TNY_FOLDER (self);
}


/**
 * tny_camel_folder_new:
 * 
 * The #TnyCamelFolder implementation is actually a proxy for #CamelFolder.
 * You need to set the #CamelFolder after using this constructor using
 * tny_camel_folder_set_folder
 *
 * Return value: A new #TnyFolder instance implemented for Camel
 **/
TnyFolder*
tny_camel_folder_new (void)
{
	TnyCamelFolder *self = g_object_new (TNY_TYPE_CAMEL_FOLDER, NULL);

	return TNY_FOLDER (self);
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

	if (G_LIKELY (priv->remove_strat))
		g_object_unref (G_OBJECT (priv->remove_strat));
	priv->remove_strat = NULL;

	g_mutex_unlock (priv->folder_lock);

	g_mutex_free (priv->folder_lock);
	priv->folder_lock = NULL;

#ifdef HEALTHY_CHECK
	g_mutex_free (priv->poshdr_lock);
	priv->poshdr_lock = NULL;
#endif

	if (priv->folder_name)
		g_free (priv->folder_name);

	(*parent_class->finalize) (object);

	return;
}

static void
tny_folder_init (gpointer g, gpointer iface_data)
{
	TnyFolderIface *klass = (TnyFolderIface *)g;

	klass->get_msg_remove_strategy_func = tny_camel_folder_get_msg_remove_strategy;
	klass->set_msg_remove_strategy_func = tny_camel_folder_set_msg_remove_strategy;
	klass->get_headers_func = tny_camel_folder_get_headers;
	klass->get_msg_func = tny_camel_folder_get_msg;
	klass->get_id_func = tny_camel_folder_get_id;
	klass->set_name_func = tny_camel_folder_set_name;
	klass->get_name_func = tny_camel_folder_get_name;
	klass->get_folder_type_func = tny_camel_folder_get_folder_type;
	klass->get_unread_count_func = tny_camel_folder_get_unread_count;
	klass->get_all_count_func = tny_camel_folder_get_all_count;
	klass->get_account_func = tny_camel_folder_get_account;
	klass->is_subscribed_func = tny_camel_folder_is_subscribed;
	klass->refresh_async_func = tny_camel_folder_refresh_async;
	klass->refresh_func = tny_camel_folder_refresh;
	klass->remove_msg_func = tny_camel_folder_remove_msg;
	klass->expunge_func = tny_camel_folder_expunge;
	klass->add_msg_func = tny_camel_folder_add_msg;
	klass->transfer_msgs_func = tny_camel_folder_transfer_msgs;

	return;
}

static void
tny_folder_store_init (gpointer g, gpointer iface_data)
{
	TnyFolderStoreIface *klass = (TnyFolderStoreIface *)g;

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

	class->get_msg_remove_strategy_func = tny_camel_folder_get_msg_remove_strategy_default;
	class->set_msg_remove_strategy_func = tny_camel_folder_set_msg_remove_strategy_default;
	class->get_headers_func = tny_camel_folder_get_headers_default;
	class->get_msg_func = tny_camel_folder_get_msg_default;
	class->get_id_func = tny_camel_folder_get_id_default;
	class->set_name_func = tny_camel_folder_set_name_default;
	class->get_name_func = tny_camel_folder_get_name_default;
	class->get_folder_type_func = tny_camel_folder_get_folder_type_default;
	class->get_unread_count_func = tny_camel_folder_get_unread_count_default;
	class->get_all_count_func = tny_camel_folder_get_all_count_default;
	class->get_account_func = tny_camel_folder_get_account_default;
	class->is_subscribed_func = tny_camel_folder_is_subscribed_default;
	class->refresh_async_func = tny_camel_folder_refresh_async_default;
	class->refresh_func = tny_camel_folder_refresh_default;
	class->remove_msg_func = tny_camel_folder_remove_msg_default;
	class->add_msg_func = tny_camel_folder_add_msg_default;
	class->expunge_func = tny_camel_folder_expunge_default;
	class->transfer_msgs_func = tny_camel_folder_transfer_msgs_default;

	class->get_folders_async_func = tny_camel_folder_get_folders_async_default;
	class->get_folders_func = tny_camel_folder_get_folders_default;
	class->create_folder_func = tny_camel_folder_create_folder_default;
	class->remove_folder_func = tny_camel_folder_remove_folder_default;

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
#ifdef HEALTHY_CHECK
	priv->poshdr_lock = g_mutex_new ();
#endif
	priv->folder_lock = g_mutex_new ();
	priv->cached_name = NULL;
	priv->cached_folder_type = TNY_FOLDER_TYPE_UNKNOWN;

	priv->remove_strat = tny_camel_msg_remove_strategy_new ();

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

		static const GInterfaceInfo tny_folder_info = 
		{
		  (GInterfaceInitFunc) tny_folder_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		static const GInterfaceInfo tny_folder_store_info = 
		{
		  (GInterfaceInitFunc) tny_folder_store_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyCamelFolder",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_FOLDER_STORE, 
			&tny_folder_store_info);

		g_type_add_interface_static (type, TNY_TYPE_FOLDER, 
			&tny_folder_info);
	}

	return type;
}

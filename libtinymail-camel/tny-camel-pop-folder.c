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


#include <tny-folder.h>
#include <tny-folder-store.h>
#include <tny-camel-folder.h>
#include <tny-camel-pop-folder.h>

#include <camel/camel.h>
#include <camel/camel-session.h>
#include <camel/camel-store.h>

#include <tny-folder.h>
#include <tny-camel-pop-store-account.h>

#include "tny-camel-account-priv.h"
#include "tny-camel-store-account-priv.h"
#include "tny-camel-folder-priv.h"
#include "tny-camel-common-priv.h"

#include "tny-camel-pop-store-account-priv.h"

#include <tny-camel-shared.h>
#include <tny-account-store.h>
#include <tny-error.h>

static GObjectClass *parent_class = NULL;

#if 0
static void 
tny_camel_pop_folder_refresh_impl (TnyFolder *self, GError **err, CamelOperationStatusFunc status_func)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	TnyCamelPopStoreAccountPriv *poppriv = TNY_CAMEL_POP_STORE_ACCOUNT_GET_PRIVATE (priv->account);
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (priv->account);
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	CamelURL *url = NULL;
	GList *options = apriv->options;
	TnyCamelFolderPriv *priv_dst;
	TnyIterator *iter;
	GPtrArray *uids, *transferred_uids = NULL;
	guint list_length;
	CamelFolder *pop_folder, *cfol_dst;
	CamelStore *pop_store;
	const gchar *myerr = NULL;
	TnyFolder *to_folder = NULL;

	to_folder = self;

	url = camel_url_new ("pop://", &ex);

	if (camel_exception_is_set (&ex)) 
		goto errorhandler;

	camel_url_set_protocol (url, "pop"); 
	camel_url_set_user (url, apriv->user);
	camel_url_set_host (url, apriv->host);

	while (options)
	{
		gchar *ptr, *dup = g_strdup (options->data);
		gchar *option, *value;
		ptr = strchr (dup, '=');
		if (ptr) {
			ptr++;
			value = g_strdup (ptr); ptr--;
			*ptr = '\0'; option = dup;
		} else {
			option = dup;
			value = g_strdup ("1");
		}
		camel_url_set_param (url, option, value);
		g_free (value);
		g_free (dup);
		options = g_list_next (options);
	}

	if (G_LIKELY (apriv->url_string))
		g_free (apriv->url_string);

	apriv->url_string = camel_url_to_string (url, 0);
	camel_url_free (url); url = NULL;

	g_static_rec_mutex_lock (apriv->service_lock);
	apriv->service = camel_session_get_service
		((CamelSession*) apriv->session, apriv->url_string, 
		apriv->type, &ex);
	g_static_rec_mutex_unlock (apriv->service_lock);

	/* TODO: Clean these up when done */
	tny_session_camel_set_pass_func (apriv->session, priv->account, apriv->get_pass_func);
	tny_session_camel_set_forget_pass_func (apriv->session, priv->account, apriv->forget_pass_func);

	if (apriv->service == NULL || camel_exception_is_set (&ex))
		goto errorhandler;

	if (!camel_service_connect (apriv->service, &ex))
		goto errorhandler;

	if (camel_exception_is_set (&ex))
		goto errorhandler;

	pop_store = CAMEL_STORE (apriv->service);

	pop_folder = camel_store_get_folder (pop_store, "INBOX", 0, &ex);
	if (pop_folder == NULL || camel_exception_is_set (&ex))
		goto errorhandler;

	camel_folder_refresh_info (pop_folder, &ex);
	if (camel_exception_is_set (&ex))
		goto errorhandler;

	priv_dst = TNY_CAMEL_FOLDER_GET_PRIVATE (to_folder);

	cfol_dst = _tny_camel_folder_get_camel_folder (TNY_CAMEL_FOLDER (to_folder));

	uids = camel_folder_get_uids (pop_folder);

	camel_folder_transfer_messages_to (pop_folder, uids, cfol_dst, 
			&transferred_uids, poppriv->delete_originals, &ex);

	if (camel_exception_is_set (&ex)) {
		g_mutex_unlock (priv_dst->folder_lock);
		goto errorhandler;
	}

	if (poppriv->delete_originals)
		camel_folder_sync (pop_folder, TRUE, &ex);

	if (camel_exception_is_set (&ex)) {
		g_mutex_unlock (priv_dst->folder_lock);
		goto errorhandler;
	}

	/* Why don't these delete the arrays with TRUE? */

	if (transferred_uids) 
		g_ptr_array_free (transferred_uids, FALSE);
	g_ptr_array_free (uids, FALSE);

	return;

errorhandler:

	if (camel_exception_is_set (&ex))
	{
		if (!myerr) myerr = camel_exception_get_description (&ex);
		camel_exception_clear (&ex);
	} else
		if (!myerr) myerr = "Unknown error";

	g_set_error (err, TNY_FOLDER_ERROR, 
			TNY_FOLDER_ERROR_REFRESH, myerr);

	if (apriv->service) /* Must this one be unreffed? */
		apriv->service = NULL;

	if (pop_folder)
		camel_object_unref (CAMEL_OBJECT (pop_folder));

	if (url)
		camel_url_free (url);

	return;
}


static void 
tny_camel_pop_folder_refresh (TnyFolder *self, GError **err)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	TnyCamelPopStoreAccountPriv *poppriv = TNY_CAMEL_POP_STORE_ACCOUNT_GET_PRIVATE (priv->account);

	g_mutex_lock (priv->folder_lock);

	tny_camel_pop_folder_refresh_impl (self, err, NULL);

	g_mutex_unlock (priv->folder_lock);

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
} RefreshPopFolderInfo;


static void
tny_camel_pop_folder_refresh_async_destroyer (gpointer thr_user_data)
{
	RefreshPopFolderInfo *info = thr_user_data;
	TnyFolder *self = info->self;

	/* thread reference */
	g_object_unref (G_OBJECT (self));
	if (info->err)
		g_error_free (info->err);

	g_slice_free (RefreshPopFolderInfo, thr_user_data);

	return;
}


static gboolean
tny_camel_pop_folder_refresh_async_callback (gpointer thr_user_data)
{
	RefreshPopFolderInfo *info = thr_user_data;

	if (info->callback)
		info->callback (info->self, info->cancelled, &info->err, info->user_data);

	return FALSE;
}


typedef struct
{
	RefreshPopFolderInfo *minfo;
	gchar *what;
	gint pc, oftotal;
} PopProgressInfo;



static void
destroy_progress_idle (gpointer data)
{
	PopProgressInfo *info = data;

	/* gidle reference */
	g_object_unref (G_OBJECT (info->minfo->self));
	g_free (info->what);

	g_slice_free (RefreshPopFolderInfo, info->minfo);
	g_slice_free (PopProgressInfo, data);

	return;
}

static gboolean
progress_func (gpointer data)
{
	PopProgressInfo *info = data;
	RefreshPopFolderInfo *minfo = info->minfo;

	if (minfo && minfo->status_callback)
	{
		minfo->status_callback (minfo->self, (const gchar*)info->what, 
			info->pc, info->oftotal, minfo->user_data);
	}

	return FALSE;
} 


static void
tny_camel_pop_folder_refresh_async_status (struct _CamelOperation *op, const char *what, int pc, void *thr_user_data)
{
	RefreshPopFolderInfo *oinfo = thr_user_data;
	PopProgressInfo *info = g_slice_new (PopProgressInfo);

	/* Camel will shredder what and thr_user_data, so we need to copy it */

	info->what = g_strdup (what);
	info->minfo = g_slice_new (RefreshPopFolderInfo);
	info->minfo->callback = oinfo->callback;
	info->minfo->cancelled = oinfo->cancelled;
	info->minfo->self = oinfo->self;
	info->minfo->status_callback = oinfo->status_callback;
	info->minfo->user_data = oinfo->user_data;
	info->oftotal = 100;

	if (pc < 0)
		info->pc = 0;
	else 
		if (pc > info->oftotal)
			info->pc = info->oftotal;
		else
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
tny_camel_pop_folder_refresh_async_thread (gpointer thr_user_data)
{
	RefreshPopFolderInfo *info = thr_user_data;
	TnyFolder *self = info->self;
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (priv->account);
	gchar *str; CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	GError *err = NULL;

	g_mutex_lock (priv->folder_lock);

	if (!_tny_camel_folder_load_folder_no_lock (priv))
	{
		tny_camel_pop_folder_refresh_async_destroyer (info);
		g_mutex_unlock (priv->folder_lock);
		g_thread_exit (NULL);
		return NULL;
	}

	info->cancelled = FALSE;
	str = g_strdup_printf (_("Fetching summary information for new messages in POP account"));
	_tny_camel_account_start_camel_operation (TNY_CAMEL_ACCOUNT (priv->account), 
		tny_camel_pop_folder_refresh_async_status, info, str);
	g_free (str);

	tny_camel_pop_folder_refresh_impl (self, &err, 
		tny_camel_pop_folder_refresh_async_status);

	info->err = NULL;

	if (camel_exception_is_set (&ex))
	{
		g_set_error (&err, TNY_FOLDER_ERROR, 
			TNY_FOLDER_ERROR_REFRESH,
			camel_exception_get_description (&ex));
		if (err != NULL)
			info->err = g_error_copy ((const GError *) err);
	}

	priv->cached_length = camel_folder_get_message_count (priv->folder);

	if (G_LIKELY (priv->folder) && CAMEL_IS_FOLDER (priv->folder) && G_LIKELY (priv->has_summary_cap))
		priv->unread_length = (guint)camel_folder_get_unread_message_count (priv->folder);

	info->cancelled = camel_operation_cancel_check (apriv->cancel);
	_tny_camel_account_stop_camel_operation (TNY_CAMEL_ACCOUNT (priv->account));

	g_mutex_unlock (priv->folder_lock);

	if (info->callback)
	{
		if (info->depth > 0)
		{
			g_idle_add_full (G_PRIORITY_HIGH, 
				tny_camel_pop_folder_refresh_async_callback, 
				info, tny_camel_pop_folder_refresh_async_destroyer);
		} else {
			tny_camel_pop_folder_refresh_async_callback (info);
			tny_camel_pop_folder_refresh_async_destroyer (info);
		}
	} else /* Thread reference */
		g_object_unref (G_OBJECT (self));

	g_thread_exit (NULL);

	return NULL;
}

static void
tny_camel_pop_folder_refresh_async (TnyFolder *self, TnyRefreshFolderCallback callback, TnyRefreshFolderStatusCallback status_callback, gpointer user_data)
{
	if (TRUE)
	{
		/* Temporary solution until multithreaded authentication works */

		GError *err = NULL;

		tny_camel_pop_folder_refresh_impl (self, &err, NULL);

		if (callback)
			callback (self, FALSE, &err, user_data);

		if (err != NULL)
			g_error_free (err);

	} else 
	{

		RefreshPopFolderInfo *info = g_slice_new (RefreshPopFolderInfo);
		GThread *thread;

		info->err = NULL;
		info->self = self;
		info->callback = callback;
		info->status_callback = status_callback;
		info->user_data = user_data;
		info->depth = g_main_depth ();

		/* thread reference */
		g_object_ref (G_OBJECT (self));

		thread = g_thread_create (tny_camel_pop_folder_refresh_async_thread,
				info, FALSE, NULL);

	}

	return;
}
#endif

/**
 * tny_camel_pop_folder_new:
 * 
 *
 * Return value: A new POP #TnyFolder instance implemented for Camel
 **/
TnyFolder*
tny_camel_pop_folder_new (void)
{
	TnyCamelPOPFolder *self = g_object_new (TNY_TYPE_CAMEL_POP_FOLDER, NULL);

	return TNY_FOLDER (self);
}

static void
tny_camel_pop_folder_finalize (GObject *object)
{
    
	(*parent_class->finalize) (object);

	return;
}

static void
tny_camel_pop_folder_refresh_async (TnyFolder *self, TnyRefreshFolderCallback callback, TnyRefreshFolderStatusCallback status_callback, gpointer user_data)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (priv->account);
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;

	camel_service_disconnect (apriv->service, FALSE, &ex);
	camel_service_connect (apriv->service, &ex);

	TNY_CAMEL_FOLDER_CLASS (parent_class)->refresh_async_func (self, callback, status_callback, user_data);
}

static void 
tny_camel_pop_folder_refresh (TnyFolder *self, GError **err)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (priv->account);
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;

	camel_service_disconnect (apriv->service, FALSE, &ex);
	camel_service_connect (apriv->service, &ex);

	TNY_CAMEL_FOLDER_CLASS (parent_class)->refresh_func (self, err);
}

static void 
tny_camel_pop_folder_class_init (TnyCamelPOPFolderClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	TNY_CAMEL_FOLDER_CLASS (class)->refresh_async_func = tny_camel_pop_folder_refresh_async;
	TNY_CAMEL_FOLDER_CLASS (class)->refresh_func = tny_camel_pop_folder_refresh; 

	object_class->finalize = tny_camel_pop_folder_finalize;

	return;
}


static void
tny_camel_pop_folder_instance_init (GTypeInstance *instance, gpointer g_class)
{
	return;
}

GType 
tny_camel_pop_folder_get_type (void)
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
		  sizeof (TnyCamelPOPFolderClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_camel_pop_folder_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyCamelPOPFolder),
		  0,      /* n_preallocs */
		  tny_camel_pop_folder_instance_init    /* instance_init */
		};
	    
		type = g_type_register_static (TNY_TYPE_CAMEL_FOLDER,
			"TnyCamelPOPFolder",
			&info, 0);	    
	}

	return type;
}


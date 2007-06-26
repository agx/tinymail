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

#include <tny-status.h>
#include <tny-folder-store.h>
#include <tny-folder.h>
#include <tny-folder-stats.h>
#include <tny-camel-folder.h>
#include <tny-msg.h>
#include <tny-header.h>
#include <tny-camel-msg.h>
#include <tny-camel-header.h>
#include <tny-store-account.h>
#include <tny-camel-store-account.h>
#include <tny-list.h>
#include <tny-error.h>
#include <tny-folder-change.h>
#include <tny-folder-observer.h>
#include <tny-folder-store-change.h>
#include <tny-folder-store-observer.h>
#include <tny-simple-list.h>

#define TINYMAIL_ENABLE_PRIVATE_API
#include "tny-common-priv.h"
#undef TINYMAIL_ENABLE_PRIVATE_API

#include <camel/camel-folder.h>
#include <camel/camel.h>
#include <camel/camel-session.h>
#include <camel/camel-store.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <tny-camel-msg-remove-strategy.h>
#include <tny-camel-full-msg-receive-strategy.h>
#include <tny-camel-partial-msg-receive-strategy.h>
#include <tny-session-camel.h>

#include "tny-camel-account-priv.h"
#include "tny-camel-store-account-priv.h"
#include "tny-camel-folder-priv.h"
#include "tny-camel-header-priv.h"
#include "tny-camel-msg-priv.h"
#include "tny-camel-common-priv.h"
#include "tny-session-camel-priv.h"

#include <tny-camel-shared.h>

#include <camel/camel-folder-summary.h>

static GObjectClass *parent_class = NULL;

static void
notify_folder_store_observers_about (TnyFolderStore *self, TnyFolderStoreChange *change)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	TnyIterator *iter;
	TnyList *list;

	g_static_rec_mutex_lock (priv->obs_lock);

	if (!priv->sobservers) {
		g_static_rec_mutex_unlock (priv->obs_lock);
		return;
	}

	list = tny_list_copy (priv->sobservers);
	g_static_rec_mutex_unlock (priv->obs_lock);

	iter = tny_list_create_iterator (list);

	while (!tny_iterator_is_done (iter))
	{
		TnyFolderStoreObserver *observer = TNY_FOLDER_STORE_OBSERVER (tny_iterator_get_current (iter));

		tny_folder_store_observer_update (observer, change);
		g_object_unref (G_OBJECT (observer));
		tny_iterator_next (iter);
	}
	g_object_unref (G_OBJECT (iter));

	g_object_unref (list);

}

static void
notify_folder_observers_about (TnyFolder *self, TnyFolderChange *change)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	TnyIterator *iter;
	TnyList *list;

	g_static_rec_mutex_lock (priv->obs_lock);

	if (!priv->observers) {
		g_static_rec_mutex_unlock (priv->obs_lock);
		return;
	}

	list = tny_list_copy (priv->observers);
	g_static_rec_mutex_unlock (priv->obs_lock);

	iter = tny_list_create_iterator (list);

	while (!tny_iterator_is_done (iter))
	{
		TnyFolderObserver *observer = TNY_FOLDER_OBSERVER (tny_iterator_get_current (iter));
		tny_folder_observer_update (observer, change);
		g_object_unref (G_OBJECT (observer));
		tny_iterator_next (iter);
	}
	g_object_unref (G_OBJECT (iter));

	g_object_unref (list);

}

void 
_tny_camel_folder_check_unread_count (TnyCamelFolder *self)
{
	TnyFolderChange *change = tny_folder_change_new (TNY_FOLDER (self));
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	priv->cached_length = camel_folder_get_message_count (priv->folder);
	priv->unread_length = camel_folder_get_unread_message_count (priv->folder);
	tny_folder_change_set_new_unread_count (change, priv->unread_length);
	tny_folder_change_set_new_all_count (change, priv->cached_length);
	notify_folder_observers_about (TNY_FOLDER (self), change);
	g_object_unref (change);
}

static void 
folder_changed (CamelFolder *camel_folder, CamelFolderChangeInfo *info, gpointer user_data)
{
	TnyCamelFolderPriv *priv = (TnyCamelFolderPriv *) user_data;
	TnyFolder *self = priv->self;
	TnyFolderChange *change = NULL;
	CamelFolderSummary *summary;
	gboolean old = priv->dont_fkill, has_chg = FALSE;
	gint i = 0;

	g_static_rec_mutex_lock (priv->folder_lock);

	if (!priv->folder) 
	{
		g_static_rec_mutex_unlock (priv->folder_lock);
		return;
	}

	summary = priv->folder->summary;

	for (i = 0; i< info->uid_added->len; i++)
	{
		const char *uid = info->uid_added->pdata[i];

		CamelMessageInfo *minfo = camel_folder_summary_uid (summary, uid);
		if (info)
		{
			TnyHeader *hdr = _tny_camel_header_new ();
			guint32 flags = camel_message_info_flags (minfo);

			if (info->push_email_event) 
			{
				if ((flags & CAMEL_MESSAGE_DELETED) && (flags & CAMEL_MESSAGE_SEEN))
					priv->unread_length++;
				priv->cached_length++;
				has_chg = TRUE;
			}

			if (!change)
				change = tny_folder_change_new (TNY_FOLDER (self));

			_tny_camel_header_set_folder (TNY_CAMEL_HEADER (hdr), 
				TNY_CAMEL_FOLDER (self), priv);

			_tny_camel_header_set_as_memory (TNY_CAMEL_HEADER (hdr), minfo);
			tny_folder_change_add_added_header (change, hdr);
			g_object_unref (G_OBJECT (hdr));
		}
	}

	for (i = 0; i< info->uid_removed->len; i++)
	{
		const char *uid = info->uid_removed->pdata[i];

		CamelMessageInfo *minfo = camel_message_info_new_uid (NULL, uid);
		if (minfo)
		{
			TnyHeader *hdr = _tny_camel_header_new ();

			if (info->push_email_event) 
			{
				priv->cached_length--;
				priv->unread_sync++;
				has_chg = TRUE;
			}

			if (!change)
				change = tny_folder_change_new (self);

			_tny_camel_header_set_as_memory (TNY_CAMEL_HEADER (hdr), minfo);
			tny_folder_change_add_removed_header (change, hdr);
			g_object_unref (hdr);
		}
	}

	if (change)
	{
		if (priv->unread_sync > 10)
		{
			/* The unread-sync is to avoid the expensive counting of unread
			 * unread messages (yes I know it sucks, but get_unread_msg_cnt
			 * walks the entire summary to count the unread ones).
			 * TNY TODO: a better solution for this */
			priv->unread_length = camel_folder_get_unread_message_count (priv->folder);
			priv->unread_sync = 0;
			has_chg = TRUE;
			tny_folder_change_set_new_unread_count (change, priv->unread_length);
		}

		if (has_chg) 
			tny_folder_change_set_new_all_count (change, priv->cached_length);

		priv->dont_fkill = TRUE;
		notify_folder_observers_about (TNY_FOLDER (self), change);
		g_object_unref (G_OBJECT (change));
		priv->dont_fkill = old;
	}

	/* search for IN TNY */
	/*if (info->uid_removed && info->uid_removed->len > 0)
		camel_folder_summary_save (camel_folder->summary);*/

	g_static_rec_mutex_unlock (priv->folder_lock);
}


static void
unload_folder_no_lock (TnyCamelFolderPriv *priv, gboolean destroy)
{
	if (priv->dont_fkill)
		return;

	if (priv->folder && !CAMEL_IS_FOLDER (priv->folder))
	{
		if (CAMEL_IS_OBJECT (priv->folder))
		{
			g_critical ("Killing invalid CamelObject (should be a Camelfolder)\n");
			while (((CamelObject*)priv->folder)->ref_count >= 1)
				camel_object_unref (CAMEL_OBJECT (priv->folder));
		} else
			g_critical ("Corrupted CamelFolder instance at (I can't recover from this state, therefore I will leak)\n");
	}

	if (G_LIKELY (priv->folder) && CAMEL_IS_FOLDER (priv->folder))
	{
		if (priv->folder_changed_id != 0)
			camel_object_remove_event (priv->folder, priv->folder_changed_id);

		/* printf ("UNLOAD (%s): %d\n",
				priv->folder_name?priv->folder_name:"NUL",
				(((CamelObject*)priv->folder)->ref_count));  */

		camel_object_unref (CAMEL_OBJECT (priv->folder));
		priv->folder = NULL;
	}

	priv->folder = NULL;
	priv->loaded = FALSE;

	return;
}

static void 
unload_folder (TnyCamelFolderPriv *priv, gboolean destroy)
{
	g_static_rec_mutex_lock (priv->folder_lock);
	unload_folder_no_lock (priv, destroy);
	g_static_rec_mutex_unlock (priv->folder_lock);
}

static void
determine_push_email (TnyCamelFolderPriv *priv)
{
	if (!priv->folder || (((CamelObject *)priv->folder)->ref_count <= 0) || !CAMEL_IS_FOLDER (priv->folder))
		return;

	if (priv->observers && tny_list_get_length (priv->observers) > 0) {
		camel_folder_set_push_email (priv->folder, TRUE);
		priv->push = TRUE;
	} else {
		camel_folder_set_push_email (priv->folder, FALSE);
		priv->push = FALSE;
	}

	return;
}

static void 
do_try_on_success (CamelStore *store, TnyCamelFolderPriv *priv, CamelException *ex)
{
	if (priv->folder && !camel_exception_is_set (ex) && CAMEL_IS_FOLDER (priv->folder)) 
	{

		if (priv->folder->folder_flags & CAMEL_FOLDER_IS_READONLY)
			priv->caps &= ~TNY_FOLDER_CAPS_WRITABLE;
		else
			priv->caps |= TNY_FOLDER_CAPS_WRITABLE;

		if (priv->folder->folder_flags & CAMEL_FOLDER_HAS_PUSHEMAIL_CAPABILITY)
			priv->caps |= TNY_FOLDER_CAPS_PUSHEMAIL;
		else
			priv->caps &= ~TNY_FOLDER_CAPS_PUSHEMAIL;

		if (!priv->iter || !priv->iter->name || strcmp (priv->iter->full_name, priv->folder_name) != 0)
		{
			guint32 flags = CAMEL_STORE_FOLDER_INFO_FAST | CAMEL_STORE_FOLDER_INFO_NO_VIRTUAL;

			if (priv->iter && !priv->iter_parented)
				camel_folder_info_free  (priv->iter);

			priv->iter = camel_store_get_folder_info (store, priv->folder_name, flags, ex);
			priv->iter_parented = TRUE;
		}
	}
}

static gboolean
load_folder_no_lock (TnyCamelFolderPriv *priv)
{
	if (priv->folder && !CAMEL_IS_FOLDER (priv->folder))
		unload_folder_no_lock (priv, FALSE);

	if (!priv->folder && !priv->loaded && priv->folder_name)
	{
		guint newlen = 0;
		CamelException ex = CAMEL_EXCEPTION_INITIALISER;
		CamelStore *store = priv->store;

		priv->folder = camel_store_get_folder 
			(store, priv->folder_name, 0, &ex);

		do_try_on_success (store, priv, &ex);

		if (!priv->folder || camel_exception_is_set (&ex) || !CAMEL_IS_FOLDER (priv->folder))
		{
			/* TNY TODO: Leak it? (this is "gash" anyway) */
			priv->folder = camel_store_get_folder (store, priv->folder_name, 0, &ex);

			if (!priv->folder || camel_exception_is_set (&ex) || !CAMEL_IS_FOLDER (priv->folder))
			{
				if (priv->folder)
					while (((CamelObject*)priv->folder)->ref_count >= 1)
						camel_object_unref ((CamelObject *) (priv->folder));
				priv->folder = NULL;
				priv->loaded = FALSE;

				return FALSE;
			} else {
				CamelException ex = CAMEL_EXCEPTION_INITIALISER;
				do_try_on_success (store, priv, &ex);
			}
		}

		determine_push_email (priv);

		if (store->flags & CAMEL_STORE_SUBSCRIPTIONS)
			priv->subscribed = 
				camel_store_folder_subscribed (store,
					camel_folder_get_full_name (priv->folder));
		else 
			priv->subscribed = TRUE;

		newlen = camel_folder_get_message_count (priv->folder);

		priv->folder_changed_id = camel_object_hook_event (priv->folder, 
			"folder_changed", (CamelObjectEventHookFunc)folder_changed, 
			priv);

		priv->has_summary_cap = camel_folder_has_summary_capability (priv->folder);

		if (G_LIKELY (priv->folder) && G_LIKELY (priv->has_summary_cap))
			priv->unread_length = (guint)
				camel_folder_get_unread_message_count (priv->folder);

		priv->loaded = TRUE;
	}
	
	return TRUE;
}

gboolean 
_tny_camel_folder_load_folder_no_lock (TnyCamelFolderPriv *priv)
{
	return load_folder_no_lock (priv);
}


static gboolean
load_folder (TnyCamelFolderPriv *priv)
{
	gboolean retval;

	g_static_rec_mutex_lock (priv->folder_lock);
	retval = load_folder_no_lock (priv);
	g_static_rec_mutex_unlock (priv->folder_lock);

	return retval;
}

void 
_tny_camel_folder_uncache_attachments (TnyCamelFolder *self, const gchar *uid)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	g_static_rec_mutex_lock (priv->folder_lock);

	if (!priv->folder || !priv->loaded || !CAMEL_IS_FOLDER (priv->folder))
		if (!load_folder_no_lock (priv))
		{
			g_static_rec_mutex_unlock (priv->folder_lock);
			return;
		}

	camel_folder_delete_attachments (priv->folder, uid);

	g_static_rec_mutex_unlock (priv->folder_lock);
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
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	gboolean haderr = FALSE;

	g_assert (TNY_IS_CAMEL_MSG (msg));

	if (!_tny_session_check_operation (TNY_FOLDER_PRIV_GET_SESSION(priv), err, 
			TNY_FOLDER_ERROR, TNY_FOLDER_ERROR_ADD_MSG))
		return;

	g_static_rec_mutex_lock (priv->folder_lock);

	if (!priv->folder || !priv->loaded || !CAMEL_IS_FOLDER (priv->folder))
		if (!load_folder_no_lock (priv))
		{
			_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));
			g_static_rec_mutex_unlock (priv->folder_lock);
			return;
		}

	message = _tny_camel_msg_get_camel_mime_message (TNY_CAMEL_MSG (msg));
	if (message && CAMEL_IS_MIME_MESSAGE (message)) {
		camel_folder_append_message (priv->folder, message, NULL, NULL, &ex);
		priv->unread_length = camel_folder_get_unread_message_count (priv->folder);
		priv->cached_length = camel_folder_get_message_count (priv->folder);
	} else {
		g_set_error (err, TNY_FOLDER_ERROR, 
			TNY_FOLDER_ERROR_ADD_MSG,
			"Malformed message");
		haderr = TRUE;
	}

	g_static_rec_mutex_unlock (priv->folder_lock);

	if (camel_exception_is_set (&ex))
	{
		g_set_error (err, TNY_FOLDER_ERROR, 
			TNY_FOLDER_ERROR_ADD_MSG,
			camel_exception_get_description (&ex));
	} else if (!haderr)
	{
		TnyHeader *header = tny_msg_get_header (msg);

		if (header && TNY_IS_HEADER (header))
		{
			TnyFolderChange *change = tny_folder_change_new (self);
			tny_folder_change_add_added_header (change, header);
			tny_folder_change_set_new_all_count (change, priv->cached_length);
			tny_folder_change_set_new_unread_count (change, priv->unread_length);
			notify_folder_observers_about (self, change);
			g_object_unref (G_OBJECT (change));
		}

		if (header && G_IS_OBJECT (header))
			g_object_unref (G_OBJECT (header));
	}

	_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));

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

	if (!_tny_session_check_operation (TNY_FOLDER_PRIV_GET_SESSION(priv), err, 
			TNY_FOLDER_ERROR, TNY_FOLDER_ERROR_REMOVE_MSG))
		return;

	if (!priv->remove_strat) {
		_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));
		return;
	}

	g_static_rec_mutex_lock (priv->folder_lock);

	if (!priv->folder || !priv->loaded || !CAMEL_IS_FOLDER (priv->folder))
		if (!load_folder_no_lock (priv))
		{
			_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));
			g_static_rec_mutex_unlock (priv->folder_lock);
			return;
		}

	tny_msg_remove_strategy_perform_remove (priv->remove_strat, self, header, err);

	_tny_camel_folder_check_unread_count (TNY_CAMEL_FOLDER (self));

	g_static_rec_mutex_unlock (priv->folder_lock);

	_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));

	return;
}

static void 
tny_camel_folder_sync (TnyFolder *self, gboolean expunge, GError **err)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->sync_func (self, expunge, err);
	return;
}

static void 
tny_camel_folder_sync_default (TnyFolder *self, gboolean expunge, GError **err)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;

	if (!_tny_session_check_operation (TNY_FOLDER_PRIV_GET_SESSION(priv), err, 
			TNY_FOLDER_ERROR, TNY_FOLDER_ERROR_SYNC))
		return;

	g_static_rec_mutex_lock (priv->folder_lock);

	if (!priv->folder || !priv->loaded || !CAMEL_IS_FOLDER (priv->folder))
		if (!load_folder_no_lock (priv))
		{
			g_static_rec_mutex_unlock (priv->folder_lock);
			_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));
			return;
		}

	camel_folder_sync (priv->folder, expunge, &ex);

	_tny_camel_folder_check_unread_count (TNY_CAMEL_FOLDER (self));

	g_static_rec_mutex_unlock (priv->folder_lock);

	if (camel_exception_is_set (&ex))
	{
		g_set_error (err, TNY_FOLDER_ERROR, 
			TNY_FOLDER_ERROR_SYNC,
			camel_exception_get_description (&ex));
	}

	_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));

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

	g_static_rec_mutex_lock (priv->folder_lock);

	if (!priv->folder || !priv->loaded || !CAMEL_IS_FOLDER (priv->folder))
	{
		CamelStore *store;
		CamelFolder *cfolder;

		if (!load_folder_no_lock (priv)) 
		{
			g_static_rec_mutex_unlock (priv->folder_lock);
			return FALSE;
		}

		store = priv->store;
		cfolder = _tny_camel_folder_get_camel_folder (TNY_CAMEL_FOLDER (self));
		priv->subscribed = camel_store_folder_subscribed (store, 
					camel_folder_get_full_name (cfolder));
	}

	retval = priv->subscribed;
	g_static_rec_mutex_unlock (priv->folder_lock);

	return retval;
}

void
_tny_camel_folder_set_subscribed (TnyCamelFolder *self, gboolean subscribed)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	priv->subscribed = subscribed;
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

	g_static_rec_mutex_lock (priv->folder_lock);
	if (priv->unread_length == 0 && priv->folder)
		priv->unread_length = camel_folder_get_unread_message_count (priv->folder);
	g_static_rec_mutex_unlock (priv->folder_lock);

	return priv->unread_length;
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


void
_tny_camel_folder_set_local_size (TnyCamelFolder *self, guint len)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	priv->local_size = len;

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

	return priv->cached_length;
}


static TnyAccount*  
tny_camel_folder_get_account (TnyFolder *self)
{
	return TNY_CAMEL_FOLDER_GET_CLASS (self)->get_account_func (self);
}

static TnyAccount*  
tny_camel_folder_get_account_default (TnyFolder *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	return TNY_ACCOUNT (g_object_ref (priv->account));
}

void
_tny_camel_folder_set_account (TnyCamelFolder *self, TnyAccount *account)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	g_assert (TNY_IS_CAMEL_ACCOUNT (account));

	priv->account = account;
	priv->store = (CamelStore*) _tny_camel_account_get_service (TNY_CAMEL_ACCOUNT (priv->account));

	return;
}



typedef struct 
{
	TnyFolder *self;
	TnyRefreshFolderCallback callback;
	TnyStatusCallback status_callback;
	gpointer user_data;
	gboolean cancelled;
	/* This stops us from calling a status callback after the operation has 
	 * finished. */
	TnyIdleStopper* stopper;
	guint depth;
	GError *err;
	TnySessionCamel *session;
} RefreshFolderInfo;


/** This is the GDestroyNotify callback provided to g_idle_add_full()
 * for tny_camel_folder_refresh_async_callback().
 */
static void
tny_camel_folder_refresh_async_destroyer (gpointer thr_user_data)
{
	RefreshFolderInfo *info = thr_user_data;
	TnyFolder *self = info->self;
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	_tny_camel_folder_unreason (priv);

	/* thread reference */
	g_object_unref (G_OBJECT (self));
	if (info->err)
		g_error_free (info->err);

	_tny_session_stop_operation (info->session);

	tny_idle_stopper_destroy (info->stopper);
	info->stopper = NULL;
	
	g_slice_free (RefreshFolderInfo, thr_user_data);

	return;
}

static gboolean
tny_camel_folder_refresh_async_callback (gpointer thr_user_data)
{
	RefreshFolderInfo *info = thr_user_data;
	TnyFolder *self = info->self;
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	TnyFolderChange *change = tny_folder_change_new (self);

	if (info->callback)
		info->callback (info->self, info->cancelled, &info->err, info->user_data);

	/* Prevent status callbacks from being called after this
	 * (can happen because the 2 idle callbacks have different priorities)
	 * by causing tny_idle_stopper_is_stopped() to return TRUE. */
	tny_idle_stopper_stop (info->stopper);

	tny_folder_change_set_new_all_count (change, priv->cached_length);
	tny_folder_change_set_new_unread_count (change, priv->unread_length);
	notify_folder_observers_about (self, change);
	g_object_unref (change);

	return FALSE;
}


static void
tny_camel_folder_refresh_async_status (struct _CamelOperation *op, const char *what, int sofar, int oftotal, void *thr_user_data)
{
	RefreshFolderInfo *oinfo = thr_user_data;
	TnyProgressInfo *info = NULL;

	info = tny_progress_info_new (G_OBJECT (oinfo->self), oinfo->status_callback, 
		TNY_FOLDER_STATUS, TNY_FOLDER_STATUS_CODE_REFRESH, what, sofar, 
		oftotal, oinfo->stopper, oinfo->user_data);

	if (oinfo->depth > 0)
	{
		g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
			tny_progress_info_idle_func, info, 
			tny_progress_info_destroy);
	} else {
		tny_progress_info_idle_func (info);
		tny_progress_info_destroy (info);
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
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	GError *err = NULL;

	g_static_rec_mutex_lock (priv->folder_lock);

	if (!load_folder_no_lock (priv))
	{
		tny_camel_folder_refresh_async_destroyer (info);
		g_static_rec_mutex_unlock (priv->folder_lock);
		g_thread_exit (NULL);
		return NULL;
	}

	info->cancelled = FALSE;

	_tny_camel_account_start_camel_operation (TNY_CAMEL_ACCOUNT (priv->account), 
		tny_camel_folder_refresh_async_status, info, 
		"Fetching summary information for new messages in folder");

	priv->want_changes = FALSE;
	camel_folder_refresh_info (priv->folder, &ex);
	priv->want_changes = TRUE;

	info->cancelled = camel_operation_cancel_check (apriv->cancel);

	priv->cached_length = camel_folder_get_message_count (priv->folder);

	if (G_LIKELY (priv->folder) && CAMEL_IS_FOLDER (priv->folder) && G_LIKELY (priv->has_summary_cap))
		priv->unread_length = (guint)camel_folder_get_unread_message_count (priv->folder);

	_tny_camel_account_stop_camel_operation (TNY_CAMEL_ACCOUNT (priv->account));

	info->err = NULL;

	if (camel_exception_is_set (&ex))
	{
		g_set_error (&err, TNY_FOLDER_ERROR, 
			TNY_FOLDER_ERROR_REFRESH,
			camel_exception_get_description (&ex));
		if (err != NULL)
			info->err = g_error_copy ((const GError *) err);
	}


	g_static_rec_mutex_unlock (priv->folder_lock);

	if (info->callback)
	{
		if (info->depth > 0)
		{
			g_idle_add_full (G_PRIORITY_HIGH, 
				tny_camel_folder_refresh_async_callback, 
				info, tny_camel_folder_refresh_async_destroyer);
		} else {
			tny_camel_folder_refresh_async_callback (info);
			tny_camel_folder_refresh_async_destroyer (info);
		}
	} else { /* Thread reference */
		g_object_unref (G_OBJECT (self));
		_tny_camel_folder_unreason (priv);
	}
	g_thread_exit (NULL);

	return NULL;
}

static void
tny_camel_folder_refresh_async (TnyFolder *self, TnyRefreshFolderCallback callback, TnyStatusCallback status_callback, gpointer user_data)
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
tny_camel_folder_refresh_async_default (TnyFolder *self, TnyRefreshFolderCallback callback, TnyStatusCallback status_callback, gpointer user_data)
{
	RefreshFolderInfo *info;
	GThread *thread;
	GError *err = NULL;
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	if (!_tny_session_check_operation (TNY_FOLDER_PRIV_GET_SESSION(priv), &err, 
			TNY_FOLDER_ERROR, TNY_FOLDER_ERROR_REFRESH))
	{
		if (callback)
			callback (self, TRUE, &err, user_data);
		g_error_free (err);
		return;
	}

	/* Idle info for the status callback: */

	info = g_slice_new (RefreshFolderInfo);
	info->session = TNY_FOLDER_PRIV_GET_SESSION (priv);
	info->err = NULL;
	info->self = self;
	info->callback = callback;
	info->status_callback = status_callback;
	info->user_data = user_data;
	info->depth = g_main_depth ();
	
	/* Use a ref count because we do not know which of the 2 idle callbacks 
	 * will be the last, and we can only unref self in the last callback:
	 * This is destroyed in the idle GDestroyNotify callback.*/

	info->stopper = tny_idle_stopper_new();

	/* thread reference */
	g_object_ref (G_OBJECT (self));
	_tny_camel_folder_reason (priv);

	/* This will cause the idle status callback to be called,
	 * via _tny_camel_account_start_camel_operation,
	 * and also calls the idle main callback: */

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
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	guint oldlen, oldurlen;
	TnyFolderChange *change = NULL;

	if (!_tny_session_check_operation (TNY_FOLDER_PRIV_GET_SESSION(priv), err, 
			TNY_FOLDER_ERROR, TNY_FOLDER_ERROR_REFRESH))
		return;

	g_static_rec_mutex_lock (priv->folder_lock);

	if (!load_folder_no_lock (priv))
	{
		g_static_rec_mutex_unlock (priv->folder_lock);
		_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));
		return;
	}

	/*_tny_camel_account_start_camel_operation (TNY_CAMEL_ACCOUNT (priv->account), 
		NULL, NULL, NULL); */

	oldlen = priv->cached_length;
	oldurlen = priv->unread_length;

	priv->want_changes = FALSE;
	camel_folder_refresh_info (priv->folder, &ex);
	priv->want_changes = TRUE;

	/* _tny_camel_account_stop_camel_operation (TNY_CAMEL_ACCOUNT (priv->account)); */

	priv->cached_length = camel_folder_get_message_count (priv->folder);    
	if (G_LIKELY (priv->folder) && CAMEL_IS_FOLDER (priv->folder) && G_LIKELY (priv->has_summary_cap))
		priv->unread_length = (guint) camel_folder_get_unread_message_count (priv->folder);

	if (camel_exception_is_set (&ex))
	{
		g_set_error (err, TNY_FOLDER_ERROR, 
			TNY_FOLDER_ERROR_REFRESH,
			camel_exception_get_description (&ex));
	}

	g_static_rec_mutex_unlock (priv->folder_lock);

	_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));

	change = tny_folder_change_new (self);
	tny_folder_change_set_new_all_count (change, priv->cached_length);
	tny_folder_change_set_new_unread_count (change, priv->unread_length);
	notify_folder_observers_about (self, change);
	g_object_unref (change);

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
	CamelMessageInfo *mi = (CamelMessageInfo *) data;

	/* Unpack speedup trick */
	TnyFolder *self = ptr->self;
	TnyCamelFolderPriv *priv = ptr->priv;
	TnyList *headers = ptr->headers;

	/* TODO: Proxy instantiation (happens a lot, could use a pool) */

	header = _tny_camel_header_new ();

	_tny_camel_header_set_folder (TNY_CAMEL_HEADER (header), TNY_CAMEL_FOLDER (self), priv);
	_tny_camel_header_set_camel_message_info (TNY_CAMEL_HEADER (header), mi, FALSE);

	tny_list_prepend (headers, (GObject*)header);

	g_object_unref (G_OBJECT (header));

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
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	FldAndPriv *ptr = NULL;

	g_assert (TNY_IS_LIST (headers));

	if (!_tny_session_check_operation (TNY_FOLDER_PRIV_GET_SESSION(priv), err, 
			TNY_FOLDER_ERROR, TNY_FOLDER_ERROR_REFRESH))
		return;

	g_static_rec_mutex_lock (priv->folder_lock);

	if (!load_folder_no_lock (priv))
	{
		g_static_rec_mutex_unlock (priv->folder_lock);
		_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));
		return;
	}

	g_object_ref (G_OBJECT (headers));

	ptr = g_slice_new (FldAndPriv);
	ptr->self = self;
	ptr->priv = priv;
	ptr->headers = headers;

	if (refresh && priv->folder && CAMEL_IS_FOLDER (priv->folder))
	{
		priv->want_changes = FALSE;
		camel_folder_refresh_info (priv->folder, &ex);
		priv->want_changes = TRUE;

		if (camel_exception_is_set (&ex))
		{
			g_set_error (err, TNY_FOLDER_ERROR, 
				TNY_FOLDER_ERROR_REFRESH,
				camel_exception_get_description (&ex));
		}
	}

	if (priv->folder && CAMEL_IS_FOLDER (priv->folder))
		g_ptr_array_foreach (priv->folder->summary->messages, 
			add_message_with_uid, ptr);

	g_slice_free (FldAndPriv, ptr);

	g_object_unref (G_OBJECT (headers));
	g_static_rec_mutex_unlock (priv->folder_lock);

	_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));

	return;
}



typedef struct 
{
	TnyFolder *self;
	TnyMsg *msg;
	TnyHeader *header;
	GError *err;
	gpointer user_data;
	guint depth;
	TnyGetMsgCallback callback;
	TnyStatusCallback status_callback;
	TnySessionCamel *session;
	TnyIdleStopper *stopper;
	gboolean cancelled;
} GetMsgInfo;


static void
tny_camel_folder_get_msg_async_destroyer (gpointer thr_user_data)
{
	GetMsgInfo *info = (GetMsgInfo *) thr_user_data;
	TnyFolderChange *change;
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (info->self);

	if (info->msg) 
	{
		change = tny_folder_change_new (info->self);
		tny_folder_change_set_received_msg (change, info->msg);
		notify_folder_observers_about (info->self, change);
		g_object_unref (G_OBJECT (change));
		g_object_unref (G_OBJECT (info->msg));
	}

	/* thread reference */
	_tny_camel_folder_unreason (priv);
	g_object_unref (G_OBJECT (info->self));

	  if (info->err)
		g_error_free (info->err);

	_tny_session_stop_operation (info->session);

	tny_idle_stopper_destroy (info->stopper);
	info->stopper = NULL;

	g_slice_free (GetMsgInfo, info);
}


static gboolean
tny_camel_folder_get_msg_async_callback (gpointer thr_user_data)
{
	GetMsgInfo *info = (GetMsgInfo *) thr_user_data;

	if (info->callback)
		info->callback (info->self, info->cancelled, info->msg, &info->err, info->user_data);

	/* Prevent status callbacks from being called after this
	 * (can happen because the 2 idle callbacks have different priorities)
	 * by causing tny_idle_stopper_is_stopped() to return TRUE. */
	tny_idle_stopper_stop (info->stopper);

	return FALSE;
}


static void
tny_camel_folder_get_msg_async_status (struct _CamelOperation *op, const char *what, int sofar, int oftotal, void *thr_user_data)
{
	GetMsgInfo *oinfo = thr_user_data;
	TnyProgressInfo *info = NULL;

	info = tny_progress_info_new (G_OBJECT (oinfo->self), oinfo->status_callback, 
		TNY_FOLDER_STATUS, TNY_FOLDER_STATUS_CODE_GET_MSG, what, sofar, 
		oftotal, oinfo->stopper, oinfo->user_data);

	if (oinfo->depth > 0)
	{
		g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
			tny_progress_info_idle_func, info, 
			tny_progress_info_destroy);
	} else {
		tny_progress_info_idle_func (info);
		tny_progress_info_destroy (info);
	}

	return;
}

static gpointer 
tny_camel_folder_get_msg_async_thread (gpointer thr_user_data)
{
	GetMsgInfo *info = (GetMsgInfo *) thr_user_data;
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (info->self);
	GError *err = NULL;
	CamelOperation *cancel;

	/* This one doesn't use the _tny_camel_account_start_camel_operation 
	 * infrastructure because it doesn't need to cancel existing operations
	 * due to a patch to camel-lite allowing messages to be fetched while
	 * other operations are happening */

	cancel = camel_operation_new (tny_camel_folder_get_msg_async_status, info);

	/* To disable parallel getting of messages while summary is being retreived,
	 * restore this lock (A) */

	/* g_static_rec_mutex_lock (priv->folder_lock); */

	camel_operation_ref (cancel);
	camel_operation_register (cancel);
	camel_operation_start (cancel, (char *) "Getting message");

	info->msg = tny_msg_receive_strategy_perform_get_msg (priv->receive_strat, info->self, info->header, &err);

	info->cancelled = camel_operation_cancel_check (cancel);

	camel_operation_unregister (cancel);
	camel_operation_end (cancel);
	if (cancel)
		camel_operation_unref (cancel);

	/* To disable parallel getting of messages while summary is being retreived,
	 * restore this lock (B) */

	/* g_static_rec_mutex_unlock (priv->folder_lock);  */

	info->err = NULL;
	if (err != NULL)
	{
		info->err = g_error_copy ((const GError *) err);
		if (info->msg && G_IS_OBJECT (info->msg))
			g_object_unref (G_OBJECT (info->msg));
		info->msg = NULL;
	} else
		info->err = NULL;

	g_object_unref (G_OBJECT (info->header));

	if (info->callback)
	{
		if (info->depth > 0)
		{
			g_idle_add_full (G_PRIORITY_HIGH, 
				tny_camel_folder_get_msg_async_callback, 
				info, tny_camel_folder_get_msg_async_destroyer);
		} else {
			tny_camel_folder_get_msg_async_callback (info);
			tny_camel_folder_get_msg_async_destroyer (info);
		}
	} else {/* thread reference */
		_tny_camel_folder_unreason (priv);
		g_object_unref (G_OBJECT (info->self));
	}

	g_thread_exit (NULL);

	return NULL;

}
static void
tny_camel_folder_get_msg_async (TnyFolder *self, TnyHeader *header, TnyGetMsgCallback callback, TnyStatusCallback status_callback, gpointer user_data)
{
	return TNY_CAMEL_FOLDER_GET_CLASS (self)->get_msg_async_func (self, header, callback, status_callback, user_data);
}


static void
tny_camel_folder_get_msg_async_default (TnyFolder *self, TnyHeader *header, TnyGetMsgCallback callback, TnyStatusCallback status_callback, gpointer user_data)
{
	GetMsgInfo *info;
	GThread *thread;
	GError *err = NULL;
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	if (!_tny_session_check_operation (TNY_FOLDER_PRIV_GET_SESSION(priv), &err, 
			TNY_FOLDER_ERROR, TNY_FOLDER_ERROR_GET_MSG))
	{
		if (callback)
			callback (self, TRUE, NULL, &err, user_data);
		g_error_free (err);
		return;
	}

	info = g_slice_new (GetMsgInfo);
	info->cancelled = FALSE;
	info->session = TNY_FOLDER_PRIV_GET_SESSION (priv);
	info->self = self;
	info->header = header;
	info->callback = callback;
	info->status_callback = status_callback;
	info->user_data = user_data;
	info->depth = g_main_depth ();

	/* Use a ref count because we do not know which of the 2 idle callbacks 
	 * will be the last, and we can only unref self in the last callback:
	 * This is destroyed in the idle GDestroyNotify callback. */

	info->stopper = tny_idle_stopper_new();

	/* thread reference */
	_tny_camel_folder_reason (priv);
	g_object_ref (G_OBJECT (info->self));
	g_object_ref (G_OBJECT (info->header));

	thread = g_thread_create (tny_camel_folder_get_msg_async_thread,
			info, FALSE, NULL);

	return;
}



static TnyMsgReceiveStrategy* 
tny_camel_folder_get_msg_receive_strategy (TnyFolder *self)
{
	return TNY_CAMEL_FOLDER_GET_CLASS (self)->get_msg_receive_strategy_func (self);
}

static TnyMsgReceiveStrategy* 
tny_camel_folder_get_msg_receive_strategy_default (TnyFolder *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	return TNY_MSG_RECEIVE_STRATEGY (g_object_ref (G_OBJECT (priv->receive_strat)));
}

static void 
tny_camel_folder_set_msg_receive_strategy (TnyFolder *self, TnyMsgReceiveStrategy *st)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->set_msg_receive_strategy_func (self, st);
	return;
}

static void 
tny_camel_folder_set_msg_receive_strategy_default (TnyFolder *self, TnyMsgReceiveStrategy *st)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	if (priv->receive_strat)
		g_object_unref (G_OBJECT (priv->receive_strat));

	priv->receive_strat = TNY_MSG_RECEIVE_STRATEGY (g_object_ref (G_OBJECT (st)));

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
	TnyMsg *retval = NULL;
	TnyFolderChange *change;

	if (!_tny_session_check_operation (TNY_FOLDER_PRIV_GET_SESSION(priv), err, 
			TNY_FOLDER_ERROR, TNY_FOLDER_ERROR_GET_MSG))
		return NULL;

	g_assert (TNY_IS_HEADER (header));

	if (!priv->receive_strat) {
		_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));
		return NULL;
	}

	g_static_rec_mutex_lock (priv->folder_lock);

	if (!priv->folder || !priv->loaded || !CAMEL_IS_FOLDER (priv->folder))
		if (!load_folder_no_lock (priv))
		{
			g_static_rec_mutex_unlock (priv->folder_lock);
			_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));
			return NULL;
		}

	retval = tny_msg_receive_strategy_perform_get_msg (priv->receive_strat, self, header, err);

	if (retval)
	{
		change = tny_folder_change_new (self);
		tny_folder_change_set_received_msg (change, retval);
		notify_folder_observers_about (self, change);
		g_object_unref (G_OBJECT (change));
	}

	g_static_rec_mutex_unlock (priv->folder_lock);

	_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));

	return retval;
}




static TnyMsg*
tny_camel_folder_find_msg (TnyFolder *self, const gchar *url_string, GError **err)
{
	return TNY_CAMEL_FOLDER_GET_CLASS (self)->find_msg_func (self, url_string, err);
}

static TnyMsg*
tny_camel_folder_find_msg_default (TnyFolder *self, const gchar *url_string, GError **err)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	TnyMsg *retval = NULL;
	TnyHeader *hdr;
	CamelMessageInfo *info;
	const gchar *uid;

	if (!_tny_session_check_operation (TNY_FOLDER_PRIV_GET_SESSION(priv), err, 
			TNY_FOLDER_ERROR, TNY_FOLDER_ERROR_GET_MSG))
		return NULL;

	if (!priv->receive_strat) {
		_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));
		return NULL;
	}

	g_static_rec_mutex_lock (priv->folder_lock);

	if (!priv->folder || !priv->loaded || !CAMEL_IS_FOLDER (priv->folder))
		if (!load_folder_no_lock (priv))
		{
			g_static_rec_mutex_unlock (priv->folder_lock);
			_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));
			return NULL;
		}

	uid = strrchr (url_string, '/');

	/* Skip over the '/': */
	if (uid && strlen (uid))
		++uid;
	
	if (uid && uid[0] != '/' && strlen (uid) > 0)
	{
		info = camel_message_info_new_uid (NULL, uid);
		hdr = _tny_camel_header_new ();
		_tny_camel_header_set_as_memory (TNY_CAMEL_HEADER (hdr), info);
		retval = tny_msg_receive_strategy_perform_get_msg (priv->receive_strat, self, hdr, err);
		g_object_unref (G_OBJECT (hdr));
	} else {
		g_warning ("%s: malformed url string: %s", __FUNCTION__, url_string);
		g_set_error (err, TNY_FOLDER_ERROR, 
				TNY_FOLDER_ERROR_GET_MSG,
				"This url string is malformed.");
		retval = NULL;
	}

	g_static_rec_mutex_unlock (priv->folder_lock);

	_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));

	return retval;
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



static TnyFolder*
tny_camel_folder_copy (TnyFolder *self, TnyFolderStore *into, const gchar *new_name, gboolean del, GError **err)
{
	return TNY_CAMEL_FOLDER_GET_CLASS (self)->copy_func (self, into, new_name, del, err);
}

typedef struct {
	GList *rems;
	GList *adds;
	TnyFolder *created;
	gboolean observers_ready;
} CpyRecRet;

typedef struct {
	TnyFolderStore *str;
	TnyFolder *fol;
} CpyEvent;

static CpyEvent*
cpy_event_new (TnyFolderStore *str, TnyFolder *fol)
{
	CpyEvent *e = g_slice_new (CpyEvent);
	e->str = (TnyFolderStore *) g_object_ref (str);
	e->fol = (TnyFolder *) g_object_ref (fol);
	return e;
}

static void
cpy_event_free (CpyEvent *e)
{
	g_object_unref (e->str);
	g_object_unref (e->fol);
	g_slice_free (CpyEvent, e);
}

static CpyRecRet*
recurse_copy (TnyFolder *folder, TnyFolderStore *into, const gchar *new_name, gboolean del, GError **err, GList *rems, GList *adds)
{
	CpyRecRet *cpyr = g_slice_new0 (CpyRecRet);

	TnyFolderStore *a_store=NULL;
	TnyFolder *retval = NULL;
	TnyStoreAccount *acc_to;
	TnyCamelFolderPriv *fpriv = TNY_CAMEL_FOLDER_GET_PRIVATE (folder);
	TnyList *headers;

	GError *nerr = NULL;

	g_static_rec_mutex_lock (fpriv->folder_lock);

	load_folder_no_lock (fpriv);

	tny_debug ("tny_folder_copy: create %s\n", new_name);

	retval = tny_folder_store_create_folder (into, new_name, &nerr);
	if (nerr != NULL) {
		if (retval)
			g_object_unref (retval);
		goto exception;
	}

	/* tny_debug ("recurse_copy: adding to adds: %s\n", tny_folder_get_name (retval));
	 * adds = g_list_append (adds, cpy_event_new (TNY_FOLDER_STORE (into), retval)); */

	if (TNY_IS_FOLDER_STORE (folder))
	{
		TnyList *folders = tny_simple_list_new ();
		TnyIterator *iter;

		tny_folder_store_get_folders (TNY_FOLDER_STORE (folder), folders, NULL, &nerr);

		if (nerr != NULL)
		{
			g_object_unref (folders);
			g_object_unref (retval);
			goto exception;
		}

		iter = tny_list_create_iterator (folders);
		while (!tny_iterator_is_done (iter))
		{
			TnyFolder *cur = TNY_FOLDER (tny_iterator_get_current (iter));
			TnyFolder *mnew;

			CpyRecRet *rt = recurse_copy (cur, TNY_FOLDER_STORE (retval), 
				tny_folder_get_name (cur), del, &nerr, rems, adds);

			mnew = rt->created;
			rems = rt->rems;
			adds = rt->adds;

			g_slice_free (CpyRecRet, rt);

			if (nerr != NULL)
			{
				if (mnew) g_object_unref (mnew);
				g_object_unref (cur);
				g_object_unref (iter);
				g_object_unref (folders);
				g_object_unref (retval);
				goto exception;
			}

			g_object_unref (mnew);
			g_object_unref (cur);
			tny_iterator_next (iter);
		}
		g_object_unref (iter);
		g_object_unref (folders);
	}

	headers = tny_simple_list_new ();
	tny_folder_get_headers (folder, headers, TRUE, &nerr);
	if (nerr != NULL) {
		g_object_unref (headers);
		g_object_unref (retval);
		retval = NULL;
		goto exception;
	}

	tny_debug ("tny_folder_copy: transfer %s to %s\n", tny_folder_get_name (folder), tny_folder_get_name (retval));

	tny_folder_transfer_msgs (folder, headers, retval, del, &nerr);

	if (nerr != NULL) {
		g_object_unref (headers);
		g_object_unref (retval);
		retval = NULL;
		goto exception;
	}

	acc_to = TNY_STORE_ACCOUNT (tny_folder_get_account (retval));
	if (acc_to) {
		if (tny_folder_is_subscribed (folder))
			tny_store_account_subscribe (acc_to, retval);
		g_object_unref (acc_to);
	}

	if (del)
	{
		tny_debug ("tny_folder_copy: del orig %s\n", tny_folder_get_name (folder));

		a_store = tny_folder_get_folder_store (folder);
		if (a_store) {
			/* tny_debug ("recurse_copy: prepending to rems: %s\n", tny_folder_get_name (folder));
			rems = g_list_append (rems, cpy_event_new (a_store, folder)); */
			tny_folder_store_remove_folder (a_store, folder, &nerr);
			g_object_unref (a_store);
		} else {
			g_set_error (&nerr, TNY_FOLDER_ERROR, 
				TNY_FOLDER_ERROR_COPY,
				"The folder (%s) didn't have a parent, therefore "
				"failed to remove it while moving. This problem "
				"indicates a bug in the software.", 
				folder ? tny_folder_get_name (folder):"none");
		}
	}

exception:

	if (nerr != NULL)
		g_propagate_error (err, nerr);

	g_static_rec_mutex_unlock (fpriv->folder_lock);

	cpyr->created = retval;
	cpyr->adds = adds;
	cpyr->rems = rems;

	return cpyr;
}

typedef GList * (*lstmodfunc) (GList *list, gpointer data);

static GList*
recurse_evt (TnyFolder *folder, TnyFolderStore *into, GList *list, lstmodfunc func, gboolean rem)
{
	TnyList *folders = tny_simple_list_new ();
	TnyIterator *iter;
	TnyStoreAccount *acc;
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (folder);

	if (!priv->folder)
		load_folder_no_lock (priv);

	acc = TNY_STORE_ACCOUNT (tny_folder_get_account (folder));

	if (rem)
		tny_store_account_unsubscribe (acc, folder);
	else
	{
		if (tny_folder_is_subscribed (folder))
			tny_store_account_subscribe (acc, folder);
	}

	g_object_unref (acc);

	list = func (list, cpy_event_new (TNY_FOLDER_STORE (into), folder));


	tny_folder_store_get_folders (TNY_FOLDER_STORE (folder), folders, NULL, NULL);

	iter = tny_list_create_iterator (folders);
	while (!tny_iterator_is_done (iter))
	{
		TnyFolder *cur = TNY_FOLDER (tny_iterator_get_current (iter));

		recurse_evt (cur, TNY_FOLDER_STORE (folder), list, func, rem);

		g_object_unref (cur);
		tny_iterator_next (iter);
	}
	g_object_unref (iter);
	g_object_unref (folders);

	return list;
}


static void
notify_folder_store_observers_about_for_store_acc (TnyFolderStore *self, TnyFolderStoreChange *change)
{
	TnyCamelStoreAccountPriv *priv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (self);
	TnyIterator *iter;

	if (!priv->sobservers)
		return;

	iter = tny_list_create_iterator (priv->sobservers);
	while (!tny_iterator_is_done (iter))
	{
		TnyFolderStoreObserver *observer = TNY_FOLDER_STORE_OBSERVER (tny_iterator_get_current (iter));
		tny_folder_store_observer_update (observer, change);
		g_object_unref (G_OBJECT (observer));
		tny_iterator_next (iter);
	}
	g_object_unref (G_OBJECT (iter));
}

static void
notify_folder_observers_about_copy (GList *adds, GList *rems, gboolean del)
{
	rems = g_list_first (rems);

	while (rems)
	{
		CpyEvent *evt = rems->data;


		if (del) {
			TnyFolderStoreChange *change = tny_folder_store_change_new (evt->str);
			tny_folder_store_change_add_removed_folder (change, evt->fol);

			if (TNY_IS_CAMEL_STORE_ACCOUNT (evt->str))
				notify_folder_store_observers_about_for_store_acc (evt->str, change);
			else
				notify_folder_store_observers_about (evt->str, change);

			tny_debug ("tny_folder_copy: observers notify folder-del %s\n", 
				tny_folder_get_name (evt->fol));

			g_object_unref (G_OBJECT (change));
		}

		cpy_event_free (evt);
		rems = g_list_next (rems);
	}

	g_list_free (rems);

	while (adds)
	{
		TnyFolderStoreChange *change;
		CpyEvent *evt = adds->data;

		change = tny_folder_store_change_new (evt->str);
		tny_folder_store_change_add_created_folder (change, evt->fol);

		if (TNY_IS_CAMEL_STORE_ACCOUNT (evt->str))
			notify_folder_store_observers_about_for_store_acc (evt->str, change);
		else
			notify_folder_store_observers_about (evt->str, change);

		g_object_unref (G_OBJECT (change));

		tny_debug ("tny_folder_copy: observers notify folder-add %s\n",
			 tny_folder_get_name (evt->fol));

		cpy_event_free (evt);
		adds = g_list_next (adds);
	}

	g_list_free (adds);
}

static CpyRecRet*
tny_camel_folder_copy_shared (TnyFolder *self, TnyFolderStore *into, const gchar *new_name, gboolean del, GError **err, GList *rems, GList *adds)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	TnyFolder *retval = NULL;
	gboolean succeeded = FALSE, tried=FALSE;
	TnyAccount *a, *b = NULL;
	GError *nerr = NULL;
	GError *terr = NULL;
	CpyRecRet *retc;

	retc = g_slice_new0 (CpyRecRet);

	if (del && priv->reason_to_live != 0)
	{
		g_set_error (&nerr, TNY_FOLDER_ERROR, 
				TNY_FOLDER_ERROR_COPY,
				"You should not use this operation with del=TRUE "
				"while the folder is still in use. There are "
				"still %d users of this folder. This problem "
				"indicates a bug in the software.", 
				priv->reason_to_live);

		g_propagate_error (err, nerr);
		/* g_error_free (nerr); */
		return retc;
	}

	g_static_rec_mutex_lock (priv->folder_lock);

	retc->observers_ready = FALSE;

	if (TNY_IS_CAMEL_FOLDER (into) || TNY_IS_CAMEL_STORE_ACCOUNT (into))
	{
		a = tny_folder_get_account (self);

		if (TNY_IS_FOLDER (into))
			b = tny_folder_get_account (TNY_FOLDER (into));
		else /* it's a TnyCamelStoreAccount in this case */
			b = g_object_ref (into);

		if (a && b && (del && (a == b)))
		{
			TnyFolderStore *a_store;
			TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (a);
			CamelException ex = CAMEL_EXCEPTION_INITIALISER;
			gchar *from, *to;

			/* load_folder_no_lock (priv);
			   load_folder_no_lock (tpriv); */

			from = priv->folder_name;

			if (TNY_IS_CAMEL_FOLDER (into)) {
				TnyCamelFolderPriv *tpriv = TNY_CAMEL_FOLDER_GET_PRIVATE (into);
				to = g_strdup_printf ("%s/%s", tpriv->folder_name, new_name);
			} else
				to = g_strdup (new_name);

			tny_debug ("tny_folder_copy: rename %s to %s\n", from, to);

			a_store = tny_folder_get_folder_store (self);
			if (a_store) {
				rems = recurse_evt (self, a_store,
					rems, g_list_prepend, TRUE);
				g_object_unref (a_store);
			}

			/* This does a g_rename on the mmap()ed file! */
			camel_store_rename_folder (CAMEL_STORE (apriv->service), from, to, &ex);

			if (!camel_exception_is_set (&ex))
			{
				CamelFolderInfo *iter;

				gboolean was_new=FALSE;
				retval = tny_camel_store_account_factor_folder 
					(TNY_CAMEL_STORE_ACCOUNT (a), to, &was_new);

				succeeded = TRUE;

				if (was_new)
				{
					CamelStore *store = priv->store;
					CamelException ex = CAMEL_EXCEPTION_INITIALISER;

					iter = camel_store_get_folder_info (store, to, 0, &ex);
					if (camel_exception_is_set (&ex))
					{
						g_set_error (&terr, TNY_FOLDER_ERROR, 
							TNY_FOLDER_ERROR_COPY,
							camel_exception_get_description (&ex));
						camel_exception_clear (&ex);
						succeeded = FALSE; tried=TRUE;
					}
					if (succeeded) {
						TnyCamelFolderPriv *rpriv = TNY_CAMEL_FOLDER_GET_PRIVATE (retval);
						_tny_camel_folder_set_folder_info (TNY_FOLDER_STORE (a), 
							TNY_CAMEL_FOLDER (retval), iter);
						_tny_camel_folder_set_parent (TNY_CAMEL_FOLDER (retval), into);
						rpriv->folder = NULL; /* This might be a leak */
					} 
				}

				if (succeeded)
					adds = recurse_evt (retval, TNY_FOLDER_STORE (into), 
						adds, g_list_append, FALSE);


			} else {
				g_set_error (&terr, TNY_FOLDER_ERROR, 
					TNY_FOLDER_ERROR_COPY,
					camel_exception_get_description (&ex));
				tried=TRUE;
			}

			g_free (to);
		}

		if (a)
			g_object_unref (a);

		if (b)
			g_object_unref (b);
	}

	if (!succeeded)
	{
		CpyRecRet *cpyr;

		tny_debug ("tny_folder_copy: recurse_copy\n");

		if (nerr != NULL)
			g_error_free (nerr);
		nerr = NULL;

		cpyr = recurse_copy (self, into, new_name, del, &nerr, adds, rems);

		if (nerr != NULL) {
			if (!tried)
				g_propagate_error (err, nerr);
			else
				g_error_free (nerr);
		} else if (del) {
			TnyFolderStore *from = tny_folder_get_folder_store (self);
			tny_folder_store_remove_folder (from, self, &nerr);
			g_object_unref (from);
			if (nerr != NULL ) {
				if (!tried)
					g_propagate_error (err, nerr);
				else
					g_error_free (nerr);
			}
		}

		retval = cpyr->created;
		adds = cpyr->adds;
		rems = cpyr->rems;

		g_slice_free (CpyRecRet, cpyr);
	}

	if (tried && terr)
		g_propagate_error (err, terr);

	if (!tried && terr)
		g_error_free (terr);

	retc->created = retval;
	retc->adds = adds;
	retc->rems = rems;

	g_static_rec_mutex_unlock (priv->folder_lock);

	return retc;
}

static TnyFolder*
tny_camel_folder_copy_default (TnyFolder *self, TnyFolderStore *into, const gchar *new_name, gboolean del, GError **err)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	GList *rems=NULL, *adds=NULL;
	TnyFolder *retval = NULL;
	GError *nerr = NULL;
	CpyRecRet *cpyr;

	if (!_tny_session_check_operation (TNY_FOLDER_PRIV_GET_SESSION(priv), err, 
			TNY_FOLDER_ERROR, TNY_FOLDER_ERROR_COPY))
		return NULL;

	cpyr = tny_camel_folder_copy_shared (self, into, new_name, del, &nerr, rems, adds);

	retval = cpyr->created;
	rems = cpyr->rems;
	adds = cpyr->adds;

	g_slice_free (CpyRecRet, cpyr);

	if (nerr != NULL)
	{
		g_propagate_error (err, nerr);
		/* g_error_free (nerr); */
	} else
		notify_folder_observers_about_copy (adds, rems, del);

	_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));

	return retval;
}


typedef struct 
{
	TnyFolder *self;
	TnyFolderStore *into;
	gchar *new_name;
	gboolean delete_originals;
	GError *err;

	gpointer user_data;
	guint depth;
	TnyCopyFolderCallback callback;
	TnyStatusCallback status_callback;
	TnyFolder *new_folder;
	TnySessionCamel *session;
	TnyIdleStopper *stopper;
	gboolean cancelled;
	GList *rems, *adds;
} CopyFolderInfo;


static void
tny_camel_folder_copy_async_destroyer (gpointer thr_user_data)
{
	CopyFolderInfo *info = (CopyFolderInfo *) thr_user_data;

	if (info->err == NULL)
	{
		notify_folder_observers_about_copy (info->adds, info->rems, 
			info->delete_originals);
	}

	/* thread reference */
	g_object_unref (G_OBJECT (info->into));
	g_object_unref (G_OBJECT (info->self));
	/* _tny_camel_folder_unreason (priv); */

	if (info->err)
		g_error_free (info->err);

	_tny_session_stop_operation (info->session);

	tny_idle_stopper_destroy (info->stopper);
	info->stopper = NULL;
	g_free (info->new_name);

	g_slice_free (CopyFolderInfo, info);
}

static gboolean
tny_camel_folder_copy_async_callback (gpointer thr_user_data)
{
	CopyFolderInfo *info = (CopyFolderInfo *) thr_user_data;

	if (info->callback)
		info->callback (info->self, info->into, info->cancelled, 
			info->new_folder, &info->err, info->user_data);

	/* Prevent status callbacks from being called after this
	 * (can happen because the 2 idle callbacks have different priorities)
	 * by causing tny_idle_stopper_is_stopped() to return TRUE. */
	tny_idle_stopper_stop (info->stopper);

	return FALSE;
}

static void
tny_camel_folder_copy_async_status (struct _CamelOperation *op, const char *what, int sofar, int oftotal, void *thr_user_data)
{
	CopyFolderInfo *oinfo = thr_user_data;
	TnyProgressInfo *info = NULL;

	/* Send back progress data only for these internal operations */
	if ((g_ascii_strcasecmp(what, "Renaming folder")) &&
	    (g_ascii_strcasecmp(what, "Moving messages")) &&
	    (g_ascii_strcasecmp(what, "Copying messages"))) 
		return;
	
	info = tny_progress_info_new (G_OBJECT (oinfo->self), oinfo->status_callback, 
		TNY_FOLDER_STATUS, TNY_FOLDER_STATUS_CODE_COPY_FOLDER, what, sofar, 
		oftotal, oinfo->stopper, oinfo->user_data);

	if (oinfo->depth > 0)
	{
		g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
			tny_progress_info_idle_func, info, 
			tny_progress_info_destroy);
	} else {
		tny_progress_info_idle_func (info);
		tny_progress_info_destroy (info);
	}

	return;
}


static gpointer 
tny_camel_folder_copy_async_thread (gpointer thr_user_data)
{
	CopyFolderInfo *info = thr_user_data;
	TnyFolder *self = info->self;
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (priv->account);
	GError *nerr = NULL;
	CpyRecRet *cpyr;

	g_static_rec_mutex_lock (priv->folder_lock);

	info->cancelled = FALSE;

	_tny_camel_account_start_camel_operation (TNY_CAMEL_ACCOUNT (priv->account), 
						  tny_camel_folder_copy_async_status, 
						  info, 
						  "Copying folder");

	info->adds = NULL; info->rems = NULL;

	cpyr = tny_camel_folder_copy_shared (info->self, info->into, 
			info->new_name, info->delete_originals, &nerr, 
			info->rems, info->adds);

	info->new_folder = cpyr->created;
	info->rems = cpyr->rems;
	info->adds = cpyr->adds;

	g_slice_free (CpyRecRet, cpyr);

	info->cancelled = camel_operation_cancel_check (apriv->cancel);

	_tny_camel_account_stop_camel_operation (TNY_CAMEL_ACCOUNT (priv->account));

	info->err = NULL;
	if (nerr != NULL)
	{
		g_propagate_error (&info->err, nerr);
	}

	g_static_rec_mutex_unlock (priv->folder_lock);

	if (info->callback)
	{
		if (info->depth > 0)
		{
			g_idle_add_full (G_PRIORITY_HIGH, 
				tny_camel_folder_copy_async_callback, 
				info, tny_camel_folder_copy_async_destroyer);
		} else {
			tny_camel_folder_copy_async_callback (info);
			tny_camel_folder_copy_async_destroyer (info);
		}
	} else { /* Thread reference */
		g_object_unref (info->into);
		g_object_unref (info->self);
/* 		_tny_camel_folder_unreason (priv); */
	}
	g_thread_exit (NULL);

	return NULL;
}

static void
tny_camel_folder_copy_async (TnyFolder *self, TnyFolderStore *into, const gchar *new_name, gboolean del, TnyCopyFolderCallback callback, TnyStatusCallback status_callback, gpointer user_data)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->copy_async_func (self, into, new_name, del, callback, status_callback, user_data);
	return;
}

static void
tny_camel_folder_copy_async_default (TnyFolder *self, TnyFolderStore *into, const gchar *new_name, gboolean del, TnyCopyFolderCallback callback, TnyStatusCallback status_callback, gpointer user_data)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	CopyFolderInfo *info;
	GThread *thread;
	GError *err = NULL;

	if (!_tny_session_check_operation (TNY_FOLDER_PRIV_GET_SESSION(priv), &err, 
			TNY_FOLDER_ERROR, TNY_FOLDER_ERROR_COPY))
	{
		if (callback)
			callback (self, into, TRUE, NULL, &err, user_data);
		g_error_free (err);
		return;
	}

	info = g_slice_new (CopyFolderInfo);

	info->cancelled = FALSE;
	info->session = TNY_FOLDER_PRIV_GET_SESSION (priv);
	info->self = self;
	info->new_folder = NULL;
	info->into = into;
	info->callback = callback;
	info->status_callback = status_callback;
	info->user_data = user_data;
	info->depth = g_main_depth ();
	info->err = NULL;
	info->delete_originals = del;
	info->new_name = g_strdup (new_name);

	/* Use a ref count because we do not know which of the 2 idle callbacks 
	 * will be the last, and we can only unref self in the last callback:
	 * This is destroyed in the idle GDestroyNotify callback. */

	info->stopper = tny_idle_stopper_new();

	/* thread reference */
/* 	_tny_camel_folder_reason (priv); */
	g_object_ref (G_OBJECT (info->self));
	g_object_ref (G_OBJECT (info->into));

	thread = g_thread_create (tny_camel_folder_copy_async_thread,
			info, FALSE, NULL);

	return;
}


typedef struct 
{
	GError *err;
	TnyFolder *self;
	TnyTransferMsgsCallback callback;
	TnyStatusCallback status_callback;
	TnyIdleStopper *stopper;
	gpointer user_data;
	guint depth;
	TnyList *header_list;
	TnyFolder *folder_dst;
	gboolean delete_originals;
	TnySessionCamel *session;
	gint from_all; 
	gint to_all;
	gint from_unread;
	gint to_unread;
	gboolean cancelled;
} TransferMsgsInfo;


static void 
inform_observers_about_transfer (TnyFolder *from, TnyFolder *to, gboolean del_orig, TnyList *headers, gint from_all, gint to_all, gint from_unread, gint to_unread)
{
	TnyFolderChange *tochange = tny_folder_change_new (to);
	TnyFolderChange *fromchange = tny_folder_change_new (from);
	TnyIterator *iter;

	iter = tny_list_create_iterator (headers);
	while (!tny_iterator_is_done (iter)) 
	{
		TnyHeader *header;
		header = TNY_HEADER (tny_iterator_get_current (iter));
		if (del_orig)
			tny_folder_change_add_removed_header (fromchange, header);
		tny_folder_change_add_added_header (tochange, header);
		g_object_unref (G_OBJECT (header));
		tny_iterator_next (iter);
	}
	g_object_unref (G_OBJECT (iter));

	tny_folder_change_set_new_all_count (tochange, to_all);
	tny_folder_change_set_new_unread_count (tochange, to_unread);

	tny_folder_change_set_new_all_count (fromchange, from_all);
	tny_folder_change_set_new_unread_count (fromchange, from_unread);

	notify_folder_observers_about (to, tochange);
	notify_folder_observers_about (from, fromchange);

	g_object_unref (tochange);
	g_object_unref (fromchange);

	return;
}

static void
tny_camel_folder_transfer_msgs_async_destroyer (gpointer thr_user_data)
{
	TransferMsgsInfo *info = thr_user_data;
	TnyCamelFolderPriv *priv_src = TNY_CAMEL_FOLDER_GET_PRIVATE (info->self);
	TnyCamelFolderPriv *priv_dst = TNY_CAMEL_FOLDER_GET_PRIVATE (info->folder_dst);

	inform_observers_about_transfer (info->self, info->folder_dst, info->delete_originals,
		info->header_list, info->from_all, info->to_all, info->from_unread, info->to_unread);

	/* thread reference */
	_tny_camel_folder_unreason (priv_src);
	g_object_unref (G_OBJECT (info->self));
	g_object_unref (G_OBJECT (info->header_list));
	_tny_camel_folder_unreason (priv_dst);
	g_object_unref (G_OBJECT (info->folder_dst));

	if (info->err) 
		g_error_free (info->err);

	_tny_session_stop_operation (info->session);

	tny_idle_stopper_destroy (info->stopper);
	info->stopper = NULL;

	g_slice_free (TransferMsgsInfo, info);

	return;
}

static gboolean
tny_camel_folder_transfer_msgs_async_callback (gpointer thr_user_data)
{
	TransferMsgsInfo *info = thr_user_data;

	if (info->callback)
		info->callback (info->self, info->cancelled, &info->err, info->user_data);

	/* Prevent status callbacks from being called after this
	 * (can happen because the 2 idle callbacks have different priorities)
	 * by causing tny_idle_stopper_is_stopped() to return TRUE. */
	tny_idle_stopper_stop (info->stopper);

	return FALSE;
}


static void
transfer_msgs_thread_clean (TnyFolder *self, TnyList *headers, TnyFolder *folder_dst, gboolean delete_originals, GError **err)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	TnyFolder *folder_src = self;
	TnyCamelFolderPriv *priv_src, *priv_dst;
	TnyIterator *iter;
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	CamelFolder *cfol_src, *cfol_dst;
	guint list_length;
	GPtrArray *uids = NULL;
	GPtrArray *transferred_uids = NULL;

	g_assert (TNY_IS_LIST (headers));
	g_assert (TNY_IS_FOLDER (folder_src));
	g_assert (TNY_IS_FOLDER (folder_dst));

	if (!_tny_session_check_operation (TNY_FOLDER_PRIV_GET_SESSION(priv), err, 
			TNY_FOLDER_ERROR, TNY_FOLDER_ERROR_TRANSFER_MSGS))
		return;

	iter = tny_list_create_iterator (headers);
	list_length = tny_list_get_length (headers);
	uids = g_ptr_array_sized_new (list_length);

	if (list_length < 1) {
		_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));
		return;
	}

	/* Get privates */
	priv_src = TNY_CAMEL_FOLDER_GET_PRIVATE (folder_src);
	priv_dst = TNY_CAMEL_FOLDER_GET_PRIVATE (folder_dst);

	g_static_rec_mutex_lock (priv_src->folder_lock);
	g_static_rec_mutex_lock (priv_dst->folder_lock);

	if (!priv_src->folder || !priv_src->loaded || !CAMEL_IS_FOLDER (priv_src->folder))
		if (!load_folder_no_lock (priv_src)) {
			g_static_rec_mutex_unlock (priv_src->folder_lock);
			g_static_rec_mutex_unlock (priv_dst->folder_lock);
			return;
		}

	if (!priv_dst->folder || !priv_dst->loaded || !CAMEL_IS_FOLDER (priv_dst->folder))
		if (!load_folder_no_lock (priv_dst)) {
			g_static_rec_mutex_unlock (priv_src->folder_lock);
			g_static_rec_mutex_unlock (priv_dst->folder_lock);
			return;
		}

	/* Get camel folders */
	cfol_src = _tny_camel_folder_get_camel_folder (TNY_CAMEL_FOLDER (folder_src));
	cfol_dst = _tny_camel_folder_get_camel_folder (TNY_CAMEL_FOLDER (folder_dst));

	/* Create uids */
	while (!tny_iterator_is_done (iter)) 
	{
		TnyHeader *header;
		const gchar *uid;

		header = TNY_HEADER (tny_iterator_get_current (iter));
		uid = tny_header_get_uid (header);

		if (G_UNLIKELY (uid == NULL)) 
		{
			g_set_error (err, TNY_FOLDER_ERROR, 
				TNY_FOLDER_ERROR_TRANSFER_MSGS,
				"You can only pass summary items as headers. "
				"These are instances that you got with the "
				"tny_folder_get_headers API. You can't use "
				"the header instances that tny_msg_get_header "
				"will return you. This problem indicates a bug "
				"in the software.");

			g_object_unref (G_OBJECT (header));
			g_object_unref (G_OBJECT (iter));
			g_ptr_array_free (uids, TRUE);

			g_static_rec_mutex_unlock (priv_dst->folder_lock);
			g_static_rec_mutex_unlock (priv_src->folder_lock);

			return;
		} else
			g_ptr_array_add (uids, (gpointer) g_strdup (uid));

		g_object_unref (G_OBJECT (header));
		tny_iterator_next (iter);
	}

	g_object_unref (G_OBJECT (iter));

	camel_folder_transfer_messages_to (cfol_src, uids, cfol_dst, 
			&transferred_uids, delete_originals, &ex);

	if (camel_exception_is_set (&ex)) 
	{
		g_set_error (err, TNY_FOLDER_ERROR, 
			TNY_FOLDER_ERROR_TRANSFER_MSGS,
			camel_exception_get_description (&ex));
	} else 
	{
		if (delete_originals)
			camel_folder_sync (cfol_src, TRUE, &ex);
		if (camel_exception_is_set (&ex))
		{
			g_set_error (err, TNY_FOLDER_ERROR, 
				TNY_FOLDER_ERROR_TRANSFER_MSGS,
				camel_exception_get_description (&ex));
		}
	}

	if (transferred_uids)
		g_ptr_array_free (transferred_uids, TRUE);

	g_ptr_array_foreach (uids, (GFunc) g_free, NULL);
	g_ptr_array_free (uids, TRUE);

	g_static_rec_mutex_unlock (priv_dst->folder_lock);
	g_static_rec_mutex_unlock (priv_src->folder_lock);

	_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));

	return;
}

static void
tny_camel_folder_transfer_msgs_async_status (struct _CamelOperation *op, const char *what, int sofar, int oftotal, void *thr_user_data)
{
	TransferMsgsInfo *oinfo = thr_user_data;
	TnyProgressInfo *info = NULL;

	info = tny_progress_info_new (G_OBJECT (oinfo->self), oinfo->status_callback, 
		TNY_FOLDER_STATUS, TNY_FOLDER_STATUS_CODE_XFER_MSGS, what, sofar, 
		oftotal, oinfo->stopper, oinfo->user_data);

	if (oinfo->depth > 0)
	{
		g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
			tny_progress_info_idle_func, info, 
			tny_progress_info_destroy);
	} else {
		tny_progress_info_idle_func (info);
		tny_progress_info_destroy (info);
	}

	return;
}

static gpointer 
tny_camel_folder_transfer_msgs_async_thread (gpointer thr_user_data)
{
	TransferMsgsInfo *info = (TransferMsgsInfo*) thr_user_data;
	TnyCamelFolderPriv *priv_src = NULL, *priv_dst = NULL;
	TnyCamelAccountPriv *apriv = NULL;
	GError *err = NULL;

	priv_src = TNY_CAMEL_FOLDER_GET_PRIVATE (info->self);
	priv_dst = TNY_CAMEL_FOLDER_GET_PRIVATE (info->folder_dst);


	if (!priv_src->folder || !priv_src->loaded || !CAMEL_IS_FOLDER (priv_src->folder))
		if (!load_folder_no_lock (priv_src)) 
			return NULL;

	if (!priv_dst->folder || !priv_dst->loaded || !CAMEL_IS_FOLDER (priv_dst->folder))
		if (!load_folder_no_lock (priv_dst)) 
			return NULL;

	info->cancelled = FALSE;
	apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (priv_src->account);
	
	/* Start operation */
	_tny_camel_account_start_camel_operation (TNY_CAMEL_ACCOUNT (priv_src->account), 
		tny_camel_folder_transfer_msgs_async_status, info, 
		"Transfer messages between two folders");

	transfer_msgs_thread_clean (info->self, info->header_list, info->folder_dst, 
			info->delete_originals, &err);

	/* Check cancelation and stop operation */
	info->cancelled = camel_operation_cancel_check (apriv->cancel);
	_tny_camel_account_stop_camel_operation (TNY_CAMEL_ACCOUNT (priv_src->account));

	/* Get data */
	info->from_all = camel_folder_get_message_count (priv_src->folder);
	info->to_all = camel_folder_get_message_count (priv_dst->folder);
	info->from_unread = camel_folder_get_unread_message_count (priv_src->folder);
	info->to_unread = camel_folder_get_unread_message_count (priv_dst->folder);

	/* Check errors */
	if (err != NULL)
		info->err = g_error_copy ((const GError *) err);
	else
		info->err = NULL;

	/* Call callback function, if it exists */
	if (info->callback)
	{
		if (info->depth > 0)
		{
			g_idle_add_full (G_PRIORITY_HIGH, 
				tny_camel_folder_transfer_msgs_async_callback, 
				info, tny_camel_folder_transfer_msgs_async_destroyer);
		} else {
			tny_camel_folder_transfer_msgs_async_callback (info);
			tny_camel_folder_transfer_msgs_async_destroyer (info);
		}
	} else { 
		/* Thread reference */
		_tny_camel_folder_unreason (priv_src);
		g_object_unref (G_OBJECT (info->self));
		g_object_unref (G_OBJECT (info->header_list));
		_tny_camel_folder_unreason (priv_dst);
		g_object_unref (G_OBJECT (info->folder_dst));
	}

	g_thread_exit (NULL);

	return NULL;
}

static void
tny_camel_folder_transfer_msgs_async (TnyFolder *self, TnyList *header_list, TnyFolder *folder_dst, gboolean delete_originals, TnyTransferMsgsCallback callback, TnyStatusCallback status_callback, gpointer user_data)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->transfer_msgs_async_func (self, header_list, folder_dst, delete_originals, callback, status_callback, user_data);
	return;
}

static void
tny_camel_folder_transfer_msgs_async_default (TnyFolder *self, TnyList *header_list, TnyFolder *folder_dst, gboolean delete_originals, TnyTransferMsgsCallback callback, TnyStatusCallback status_callback, gpointer user_data)
{
	TransferMsgsInfo *info;
	GThread *thread;
	GError *err = NULL;
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	TnyCamelFolderPriv *priv_src = priv;
	TnyCamelFolderPriv *priv_dst = TNY_CAMEL_FOLDER_GET_PRIVATE (folder_dst);

	if (!_tny_session_check_operation (TNY_FOLDER_PRIV_GET_SESSION(priv), &err, 
			TNY_FOLDER_ERROR, TNY_FOLDER_ERROR_TRANSFER_MSGS))
	{
		if (callback)
			callback (self, TRUE, &err, user_data);
		g_error_free (err);
		return;
	}

	info = g_slice_new (TransferMsgsInfo);
	info->cancelled = FALSE;
	info->session = TNY_FOLDER_PRIV_GET_SESSION (priv);
	info->self = self;
	info->header_list = header_list; 
	info->folder_dst = folder_dst;
	info->callback = callback;
	info->status_callback = status_callback;
	info->user_data = user_data;
	info->delete_originals = delete_originals;
	info->depth = g_main_depth ();

	/* Use a ref count because we do not know which of the 2 idle callbacks 
	 * will be the last, and we can only unref self in the last callback:
	 * This is destroyed in the idle GDestroyNotify callback. */

	info->stopper = tny_idle_stopper_new();

	/* thread reference */
	g_object_ref (G_OBJECT (info->header_list));
	_tny_camel_folder_reason (priv_src);
	g_object_ref (G_OBJECT (info->self));
	_tny_camel_folder_reason (priv_dst);
	g_object_ref (G_OBJECT (info->folder_dst));

	thread = g_thread_create (tny_camel_folder_transfer_msgs_async_thread,
			info, FALSE, NULL);    

	return;
}


static void
tny_camel_folder_transfer_msgs (TnyFolder *self, TnyList *headers, TnyFolder *folder_dst, gboolean delete_originals, GError **err)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->transfer_msgs_func (self, headers, folder_dst, delete_originals, err);
	return;
}


static void
tny_camel_folder_transfer_msgs_default (TnyFolder *self, TnyList *headers, TnyFolder *folder_dst, gboolean delete_originals, GError **err)
{
	gint from, to, ufrom, uto;
	TnyCamelFolderPriv *priv_src, *priv_dst;

	g_assert (TNY_IS_LIST (headers));
	g_assert (TNY_IS_FOLDER (self));
	g_assert (TNY_IS_FOLDER (folder_dst));

	priv_src = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	priv_dst = TNY_CAMEL_FOLDER_GET_PRIVATE (folder_dst);

	if (!priv_src->folder || !priv_src->loaded || !CAMEL_IS_FOLDER (priv_src->folder))
		if (!load_folder (priv_src)) 
			return;

	if (!priv_dst->folder || !priv_dst->loaded || !CAMEL_IS_FOLDER (priv_dst->folder))
		if (!load_folder (priv_dst)) 
			return;

	transfer_msgs_thread_clean (self, headers, folder_dst, delete_originals, err);

	if (!err || *err == NULL)
	{
		from = camel_folder_get_message_count (priv_src->folder);
		to = camel_folder_get_message_count (priv_dst->folder);
		ufrom = camel_folder_get_unread_message_count (priv_src->folder);
		uto = camel_folder_get_unread_message_count (priv_dst->folder);

		inform_observers_about_transfer (self, folder_dst, delete_originals, 
				headers, from, to, ufrom, uto);
	}

	return;
}

void
_tny_camel_folder_set_id (TnyCamelFolder *self, const gchar *id)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	g_static_rec_mutex_lock (priv->folder_lock);

	/* unload_folder_no_lock (priv, TRUE); */

	if (G_UNLIKELY (priv->folder_name))
		g_free (priv->folder_name);

	priv->folder_name = g_strdup (id);

	g_static_rec_mutex_unlock (priv->folder_lock);

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
tny_camel_folder_uncache (TnyCamelFolder *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	if (G_LIKELY (priv->folder != NULL))
		unload_folder (priv, FALSE);

	return;
}

void 
_tny_camel_folder_unreason (TnyCamelFolderPriv *priv)
{
	g_mutex_lock (priv->reason_lock);
	priv->reason_to_live--;

	if (priv->reason_to_live == 0) 
	{
		/* The special case is for when the amount of items is ZERO
		 * while we are listening for Push E-mail events. That's a
		 * reason by itself not to destroy priv->folder */

		if (!(priv->push && priv->folder && priv->folder->summary && 
			priv->folder->summary->messages && 
			priv->folder->summary->messages->len == 0)) {

			/* For any other folder that has no more reason to live,
			 * we'll uncache (this means destroying the CamelFolder
			 * instance and freeing up memory */

			tny_camel_folder_uncache ((TnyCamelFolder *)priv->self);

		}
	}
	g_mutex_unlock (priv->reason_lock);
}

void 
_tny_camel_folder_reason (TnyCamelFolderPriv *priv)
{
	g_mutex_lock (priv->reason_lock);
	priv->reason_to_live++;
	g_mutex_unlock (priv->reason_lock);
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


void 
_tny_camel_folder_remove_folder_actual (TnyFolderStore *self, TnyFolder *folder, GError **err)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	TnyCamelStoreAccountPriv *apriv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (priv->account);
	CamelStore *store = priv->store;
	TnyCamelFolder *cfol = TNY_CAMEL_FOLDER (folder);
	TnyCamelFolderPriv *cpriv = TNY_CAMEL_FOLDER_GET_PRIVATE (cfol);
	gchar *cfolname; gchar *folname; gint parlen;
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	gboolean changed = FALSE;

	g_static_rec_mutex_lock (priv->folder_lock);
	g_static_rec_mutex_lock (cpriv->folder_lock);

	if (apriv->iter)
	{
		/* Known memleak
		camel_store_free_folder_info (apriv->iter_store, apriv->iter); */
		apriv->iter = NULL;
	}

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
			CamelException subex = CAMEL_EXCEPTION_INITIALISER;

			g_static_rec_mutex_lock (cpriv->obs_lock);
			if (cpriv->observers) {
				g_object_unref (G_OBJECT (cpriv->observers));
				cpriv->observers = NULL;
			}
			if (cpriv->sobservers) {
				g_object_unref (G_OBJECT (cpriv->sobservers));
				cpriv->sobservers = NULL;
			}
			g_static_rec_mutex_unlock (priv->obs_lock);

			if (camel_store_supports_subscriptions (store))
				camel_store_subscribe_folder (store, cfolname, &subex);

			camel_store_delete_folder (store, cfolname, &ex);

			if (camel_exception_is_set (&ex))
			{
				g_set_error (err, TNY_FOLDER_STORE_ERROR, 
					TNY_FOLDER_STORE_ERROR_REMOVE_FOLDER,
					camel_exception_get_description (&ex));
				camel_exception_clear (&ex);
			} else 
			{
				if (camel_store_supports_subscriptions (store))
					camel_store_unsubscribe_folder (store, cfolname, &subex);

				changed = TRUE;
				g_free (cpriv->folder_name); 
				cpriv->folder_name = NULL;
				apriv->managed_folders = 
					g_list_remove (apriv->managed_folders, cfol);
			}
		}
	}

	g_static_rec_mutex_unlock (cpriv->folder_lock);
	g_static_rec_mutex_unlock (priv->folder_lock);

	if (changed)
	{
		TnyFolderStoreChange *change;

		change = tny_folder_store_change_new (self);
		tny_folder_store_change_add_removed_folder (change, folder);
		notify_folder_store_observers_about (self, change);
		g_object_unref (G_OBJECT (change));
	}

	return;
}

static void
recurse_remove (TnyFolderStore *from, TnyFolder *folder, GError **err)
{
	TnyCamelFolderPriv *fpriv = TNY_CAMEL_FOLDER_GET_PRIVATE (from);
	GError *nerr = NULL;

	g_static_rec_mutex_lock (fpriv->folder_lock);

	if (TNY_IS_FOLDER_STORE (folder))
	{
		TnyList *folders = tny_simple_list_new ();
		TnyIterator *iter;

		tny_folder_store_get_folders (TNY_FOLDER_STORE (folder), 
				folders, NULL, &nerr);

		if (nerr != NULL)
		{
			g_object_unref (folders);
			goto exception;
		}

		iter = tny_list_create_iterator (folders);
		while (!tny_iterator_is_done (iter))
		{
			TnyFolder *cur = TNY_FOLDER (tny_iterator_get_current (iter));

			recurse_remove (TNY_FOLDER_STORE (folder), cur, &nerr);

			if (nerr != NULL)
			{
				g_object_unref (cur);
				g_object_unref (iter);
				g_object_unref (folders);
				goto exception;
			}

			g_object_unref (cur);
			tny_iterator_next (iter);
		}
		g_object_unref (iter);
		g_object_unref (folders);
	}

	tny_debug ("tny_folder_store_remove: actual removal of %s\n", 
			tny_folder_get_name (folder));

	_tny_camel_folder_remove_folder_actual (from, folder, &nerr);


exception:

	if (nerr != NULL)
		g_propagate_error (err, nerr);

	g_static_rec_mutex_unlock (fpriv->folder_lock);

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
	GError *nerr = NULL;

	if (!_tny_session_check_operation (TNY_FOLDER_PRIV_GET_SESSION(priv), err, 
			TNY_FOLDER_STORE_ERROR, TNY_FOLDER_STORE_ERROR_REMOVE_FOLDER))
		return;

	recurse_remove (self, folder, &nerr);

	if (nerr != NULL)
		g_propagate_error (err, nerr);

	_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));

	return;
}


void
_tny_camel_folder_set_folder_info (TnyFolderStore *self, TnyCamelFolder *folder, CamelFolderInfo *info)
{
	_tny_camel_folder_set_id (folder, info->full_name);
	_tny_camel_folder_set_folder_type (folder, info);
	_tny_camel_folder_set_unread_count (folder, info->unread);
	_tny_camel_folder_set_all_count (folder, info->total);
	_tny_camel_folder_set_local_size (folder, info->local_size);
	if (!info->name)
		g_warning ("Creating invalid folder\n");
	_tny_camel_folder_set_name (folder, info->name);
	_tny_camel_folder_set_iter (folder, info);

	if (TNY_IS_CAMEL_FOLDER (self)) {
		TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
		_tny_camel_folder_set_account (folder, priv->account);
	} else if (TNY_IS_CAMEL_STORE_ACCOUNT (self)){
		_tny_camel_folder_set_account (folder, TNY_ACCOUNT (self));
	}

	_tny_camel_folder_set_parent (folder, self);
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
	TnyFolderStoreChange *change;
	CamelException subex = CAMEL_EXCEPTION_INITIALISER;

	if (!_tny_session_check_operation (TNY_FOLDER_PRIV_GET_SESSION(priv), err, 
			TNY_FOLDER_STORE_ERROR, TNY_FOLDER_STORE_ERROR_CREATE_FOLDER))
		return NULL;

	if (!name || strlen (name) <= 0)
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_CREATE_FOLDER,
				"Failed to create folder with no name");
		_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));
		return NULL;
	}

	if (!priv->folder_name)
	{
		g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_CREATE_FOLDER,
				"Failed to create folder %s", name);
		_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));
		return NULL;
	}

	store = (CamelStore*) priv->store;

	g_assert (CAMEL_IS_STORE (store));

	g_static_rec_mutex_lock (priv->folder_lock);
	folname = priv->folder_name;
	info = camel_store_create_folder (store, priv->folder_name, name, &ex);
	g_static_rec_mutex_unlock (priv->folder_lock);

	if (!info || camel_exception_is_set (&ex)) 
	{
		if (camel_exception_is_set (&ex))
			g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_CREATE_FOLDER,
				camel_exception_get_description (&ex));
		else
			g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_CREATE_FOLDER,
				"Unknown error while trying to create folder");

		if (info)
			camel_store_free_folder_info (store, info);
		_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));
		return NULL;
	}

	g_assert (info != NULL);

	if (camel_store_supports_subscriptions (store))
		camel_store_subscribe_folder (store, info->full_name, &subex);

	folder = _tny_camel_folder_new ();
	_tny_camel_folder_set_folder_info (self, TNY_CAMEL_FOLDER (folder), info);

	change = tny_folder_store_change_new (self);
	tny_folder_store_change_add_created_folder (change, folder);
	notify_folder_store_observers_about (self, change);
	g_object_unref (G_OBJECT (change));

	_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));

	return folder;
}

/* Sets a TnyFolderStore as the parent of a TnyCamelFolder. Note that
 * this code could cause a cross-reference situation, if the parent
 * was used to create the child. */

void
_tny_camel_folder_set_parent (TnyCamelFolder *self, TnyFolderStore *parent)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	if (priv->parent)
		g_object_unref (G_OBJECT (priv->parent));
	priv->parent = g_object_ref (G_OBJECT (parent));
	return;
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
	CamelFolderInfo *iter;

	if (!_tny_session_check_operation (TNY_FOLDER_PRIV_GET_SESSION(priv), err, 
			TNY_FOLDER_STORE_ERROR, TNY_FOLDER_STORE_ERROR_GET_FOLDERS))
		return;


	if (!priv->iter && priv->iter_parented)
	{
		CamelStore *store = priv->store;
		CamelException ex = CAMEL_EXCEPTION_INITIALISER;

		g_return_if_fail (priv->folder_name != NULL);

		priv->iter = camel_store_get_folder_info (store, priv->folder_name, 0, &ex);

		if (camel_exception_is_set (&ex))
		{
			g_set_error (err, TNY_FOLDER_STORE_ERROR, 
				TNY_FOLDER_STORE_ERROR_GET_FOLDERS,
				camel_exception_get_description (&ex));
			camel_exception_clear (&ex);
			_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));

			if (priv->iter == NULL)
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
		/* Also take a look at camel-maildir-store.c:525 */
		if (!(iter->flags & CAMEL_FOLDER_VIRTUAL) && _tny_folder_store_query_passes (query, iter))
		{
			gboolean was_new = FALSE;

			TnyCamelFolder *folder = (TnyCamelFolder *) tny_camel_store_account_factor_folder (
				TNY_CAMEL_STORE_ACCOUNT (priv->account), 
				iter->full_name, &was_new);

			if (was_new)
				_tny_camel_folder_set_folder_info (self, folder, iter);

			tny_list_prepend (list, G_OBJECT (folder));
			g_object_unref (G_OBJECT (folder));
		}
		iter = iter->next;
	  }
	}

	_tny_session_stop_operation (TNY_FOLDER_PRIV_GET_SESSION (priv));

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
	guint depth; TnySessionCamel *session;
} GetFoldersInfo;


static void
tny_camel_folder_get_folders_async_destroyer (gpointer thr_user_data)
{
	GetFoldersInfo *info = thr_user_data;
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (info->self);

	/* thread reference */
	_tny_camel_folder_unreason (priv);
	g_object_unref (G_OBJECT (info->self));
	g_object_unref (G_OBJECT (info->list));

	if (info->err)
		g_error_free (info->err);

	_tny_session_stop_operation (info->session);

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
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (info->self);

	GError *err = NULL;

	tny_folder_store_get_folders (TNY_FOLDER_STORE (info->self),
		info->list, info->query, &err);

	if (err != NULL)
		info->err = g_error_copy ((const GError *) err);
	else
		info->err = NULL;

	if (info->query)
		g_object_unref (G_OBJECT (info->query));


	if (info->callback)
	{
		if (info->depth > 0)
		{
			g_idle_add_full (G_PRIORITY_HIGH, 
				tny_camel_folder_get_folders_async_callback, 
				info, tny_camel_folder_get_folders_async_destroyer);
		} else {
			tny_camel_folder_get_folders_async_callback (info);
			tny_camel_folder_get_folders_async_destroyer (info);
		}
	} else {
		/* thread reference */
		_tny_camel_folder_unreason (priv);
		g_object_unref (G_OBJECT (info->self));
		g_object_unref (G_OBJECT (info->list));
	}

	g_thread_exit (NULL);

	return NULL;
}

static void 
tny_camel_folder_get_folders_async (TnyFolderStore *self, TnyList *list, TnyGetFoldersCallback callback, TnyFolderStoreQuery *query, TnyStatusCallback status_callback, gpointer user_data)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->get_folders_async_func (self, list, callback, query, status_callback, user_data);
}

static void 
tny_camel_folder_get_folders_async_default (TnyFolderStore *self, TnyList *list, TnyGetFoldersCallback callback, TnyFolderStoreQuery *query, TnyStatusCallback status_callback, gpointer user_data)
{
	GetFoldersInfo *info;
	GThread *thread;
	GError *err = NULL;
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	if (!_tny_session_check_operation (TNY_FOLDER_PRIV_GET_SESSION(priv), &err, 
			TNY_FOLDER_ERROR, TNY_FOLDER_ERROR_REFRESH))
	{
		if (callback)
			callback (self, list, &err, user_data);
		g_error_free (err);
		return;
	}

	info = g_slice_new (GetFoldersInfo);
	info->session = TNY_FOLDER_PRIV_GET_SESSION (priv);
	info->self = self;
	info->list = list;
	info->callback = callback;
	info->user_data = user_data;
	info->query = query;
	info->depth = g_main_depth ();

	/* thread reference */
	_tny_camel_folder_reason (priv);
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

	if (G_UNLIKELY (!priv->loaded))
		if (!load_folder (priv))
			return NULL;
	
	retval = priv->folder;
	if (retval)
		camel_object_ref (CAMEL_OBJECT (retval));

	return retval;
}


static void 
tny_camel_folder_poke_status (TnyFolder *self)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->poke_status_func (self);
	return;
}

typedef struct
{
	TnyFolder *self;
	gint unread;
	gint total;
} PokeStatusInfo;

static gboolean
tny_camel_folder_poke_status_callback (gpointer data)
{
	PokeStatusInfo *info = (PokeStatusInfo *) data;
	TnyFolder *self = (TnyFolder *) info->self;
	TnyFolderChange *change = tny_folder_change_new (self);
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	priv->cached_length = (guint) info->total;
	tny_folder_change_set_new_all_count (change, priv->cached_length);
	priv->unread_length = (guint) info->unread;
	tny_folder_change_set_new_unread_count (change, priv->unread_length);
	notify_folder_observers_about (self, change);

	g_object_unref (change);

	return FALSE;
}

static void
tny_camel_folder_poke_status_destroyer (gpointer data)
{
	PokeStatusInfo *info = (PokeStatusInfo *) data;
	g_object_unref (info->self);
	g_slice_free (PokeStatusInfo, info);
}


static GStaticMutex poke_folders_lock = G_STATIC_MUTEX_INIT;
static GList *poke_folders = NULL;
static GThread *poke_folders_thread = NULL;

static gpointer
tny_camel_folder_poke_status_thread (gpointer data)
{

	while (poke_folders)
	{
		PokeStatusInfo *info = NULL;
		TnyFolder *folder = NULL;
		TnyCamelFolderPriv *priv = NULL;
		CamelStore *store = NULL;
		int newlen = -1, newurlen = -1, uidnext = -1;

		g_static_mutex_lock (&poke_folders_lock);

		folder = poke_folders->data;

		priv = TNY_CAMEL_FOLDER_GET_PRIVATE (folder);
		store = priv->store;

		camel_store_get_folder_status (store, priv->folder_name, 
			&newurlen, &newlen, &uidnext);

		if (newurlen == -1 || newlen == -1)
		{
			if (priv->iter) {
				info = g_slice_new (PokeStatusInfo);
				info->unread = priv->iter->unread;
				info->total = priv->iter->total;
			}
		} else {
			info = g_slice_new (PokeStatusInfo);
			info->unread = newurlen;
			info->total = newlen;
		}

		if (info && folder)
		{
			info->self = TNY_FOLDER (g_object_ref (folder));
			g_idle_add_full (G_PRIORITY_HIGH, 
				tny_camel_folder_poke_status_callback, 
				info, tny_camel_folder_poke_status_destroyer);
		}

		g_object_unref (folder);

		poke_folders = g_list_next (poke_folders);

		if (!poke_folders)
		{
			poke_folders_thread = NULL;
			break; /* Reason for A */
		} else {
			g_static_mutex_unlock (&poke_folders_lock);
			usleep (5000); /* Allow other folders to be added */
		}
	}

	g_static_mutex_unlock (&poke_folders_lock); /* A */

	g_thread_exit (NULL);
	return NULL;
}


static void 
tny_camel_folder_poke_status_default (TnyFolder *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	CamelStore *store = priv->store;
	PokeStatusInfo *info = NULL;

	if (priv->folder)
	{
		info = g_slice_new (PokeStatusInfo);
		g_static_rec_mutex_lock (priv->folder_lock);
		info->unread = camel_folder_get_unread_message_count (priv->folder);
		info->total = camel_folder_get_message_count (priv->folder);
		g_static_rec_mutex_unlock (priv->folder_lock);
	} else {
		g_static_rec_mutex_lock (priv->folder_lock);

		if (store && CAMEL_IS_DISCO_STORE (store)  && priv->folder_name 
			&& camel_disco_store_status (CAMEL_DISCO_STORE (store)) == CAMEL_DISCO_STORE_ONLINE)
		{
			g_static_mutex_lock (&poke_folders_lock);

			poke_folders = g_list_append (poke_folders, g_object_ref (self));
			if (!poke_folders_thread)
				poke_folders_thread = g_thread_create (tny_camel_folder_poke_status_thread, self, TRUE, NULL);

			g_static_mutex_unlock (&poke_folders_lock);
		} else {
			if (priv->iter) {
				info = g_slice_new (PokeStatusInfo);
				info->unread = priv->iter->unread;
				info->total = priv->iter->total;
			}
		}

		g_static_rec_mutex_unlock (priv->folder_lock);
	}


	if (info)
	{
		info->self = TNY_FOLDER (g_object_ref (self));

		if (g_main_depth () > 0)
		{
			g_idle_add_full (G_PRIORITY_HIGH, 
					tny_camel_folder_poke_status_callback, 
					info, tny_camel_folder_poke_status_destroyer);
		} else 
		{
			tny_camel_folder_poke_status_callback (info);
			tny_camel_folder_poke_status_destroyer (info);
		}
	}

	return;
}

static TnyFolderStore*  
tny_camel_folder_get_folder_store (TnyFolder *self)
{
	return TNY_CAMEL_FOLDER_GET_CLASS (self)->get_folder_store_func (self);
}

static TnyFolderStore*  
tny_camel_folder_get_folder_store_default (TnyFolder *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	if (priv->parent)
		g_object_ref (priv->parent);

	return priv->parent;
}


static void
tny_camel_folder_add_observer (TnyFolder *self, TnyFolderObserver *observer)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->add_observer_func (self, observer);
}

static void
tny_camel_folder_add_observer_default (TnyFolder *self, TnyFolderObserver *observer)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	g_assert (TNY_IS_FOLDER_OBSERVER (observer));

	g_static_rec_mutex_lock (priv->obs_lock);
	if (!priv->observers)
		priv->observers = tny_simple_list_new ();
	tny_list_prepend (priv->observers, G_OBJECT (observer));
	g_static_rec_mutex_unlock (priv->obs_lock);

	determine_push_email (priv);

	return;
}


static void
tny_camel_folder_remove_observer (TnyFolder *self, TnyFolderObserver *observer)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->remove_observer_func (self, observer);
}

static void
tny_camel_folder_remove_observer_default (TnyFolder *self, TnyFolderObserver *observer)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	g_assert (TNY_IS_FOLDER_OBSERVER (observer));

	g_static_rec_mutex_lock (priv->obs_lock);

	if (!priv->observers) {
		g_static_rec_mutex_unlock (priv->obs_lock);
		return;
	}

	tny_list_remove (priv->observers, G_OBJECT (observer));
	g_static_rec_mutex_unlock (priv->obs_lock);

	determine_push_email (priv);

	return;
}


static TnyFolderStats *
tny_camel_folder_get_stats (TnyFolder *self)
{
	return TNY_CAMEL_FOLDER_GET_CLASS (self)->get_stats_func (self);
}

static TnyFolderStats * 
tny_camel_folder_get_stats_default (TnyFolder *self)
{
	TnyFolderStats *retval = NULL;
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	if (!load_folder (priv))
		return NULL;

	retval = tny_folder_stats_new (self);

	priv->unread_length = camel_folder_get_unread_message_count (priv->folder);
	priv->cached_length = camel_folder_get_message_count (priv->folder);
	priv->local_size = camel_folder_get_local_size (priv->folder);
	tny_folder_stats_set_local_size (retval, priv->local_size);

	return retval;
}

static void
tny_camel_folder_store_add_observer (TnyFolderStore *self, TnyFolderStoreObserver *observer)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->add_store_observer_func (self, observer);
}

static void
tny_camel_folder_store_add_observer_default (TnyFolderStore *self, TnyFolderStoreObserver *observer)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	g_assert (TNY_IS_FOLDER_STORE_OBSERVER (observer));

	g_static_rec_mutex_lock (priv->obs_lock);
	if (!priv->sobservers)
		priv->sobservers = tny_simple_list_new ();
	tny_list_prepend (priv->sobservers, G_OBJECT (observer));
	g_static_rec_mutex_unlock (priv->obs_lock);

	return;
}


static void
tny_camel_folder_store_remove_observer (TnyFolderStore *self, TnyFolderStoreObserver *observer)
{
	TNY_CAMEL_FOLDER_GET_CLASS (self)->remove_store_observer_func (self, observer);
}

static void
tny_camel_folder_store_remove_observer_default (TnyFolderStore *self, TnyFolderStoreObserver *observer)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	g_assert (TNY_IS_FOLDER_STORE_OBSERVER (observer));

	g_static_rec_mutex_lock (priv->obs_lock);
	if (!priv->sobservers) {
		g_static_rec_mutex_unlock (priv->obs_lock);
		return;
	}
	tny_list_remove (priv->sobservers, G_OBJECT (observer));
	g_static_rec_mutex_unlock (priv->obs_lock);

	return;
}


static gchar* 
tny_camel_folder_get_url_string (TnyFolder *self)
{
	return TNY_CAMEL_FOLDER_GET_CLASS (self)->get_url_string_func (self);
}

static gchar* 
tny_camel_folder_get_url_string_default (TnyFolder *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	gchar *retval = NULL;

	/* iter->uri is a cache.
	 * Check for strlen(), because camel produces an empty (non-null) 
	 * uri for POP. */
	if (priv->iter && priv->iter->uri && (strlen (priv->iter->uri) > 0))
	{
		retval = g_strdup_printf ("%s", priv->iter->uri);
	} else if (priv->account)
	{
		TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (priv->account);
		if (apriv->service)
		{
			char *urls = camel_service_get_url (apriv->service);
			const char *foln = camel_folder_get_full_name (priv->folder);
			retval = g_strdup_printf ("%s/%s", urls, foln);
			g_free (urls);
		}
	}

	if (!retval) { /* Strange, a local one?*/
		g_warning ("tny_folder_get_url_string does not have an "
				"iter nor account. Using maildir as type.\n");
		retval = g_strdup_printf ("maildir://%s", priv->folder_name);
	}

	/* printf ("DEBUG: %s: retval='%s'\n", __FUNCTION__, retval); */
	return retval;
}

static TnyFolderCaps 
tny_camel_folder_get_caps (TnyFolder *self)
{
	return TNY_CAMEL_FOLDER_GET_CLASS (self)->get_caps_func (self);
}

static TnyFolderCaps 
tny_camel_folder_get_caps_default (TnyFolder *self)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	return priv->caps;
}

TnyFolder*
_tny_camel_folder_new_with_folder (CamelFolder *camel_folder)
{
	TnyCamelFolder *self = g_object_new (TNY_TYPE_CAMEL_FOLDER, NULL);

	_tny_camel_folder_set_folder (self, camel_folder);

	return TNY_FOLDER (self);
}



TnyFolder*
_tny_camel_folder_new (void)
{
	TnyCamelFolder *self = g_object_new (TNY_TYPE_CAMEL_FOLDER, NULL);

	return TNY_FOLDER (self);
}


static void
tny_camel_folder_finalize (GObject *object)
{
	TnyCamelFolder *self = (TnyCamelFolder*) object;
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);


	g_static_rec_mutex_lock (priv->obs_lock);
	if (priv->observers)
		g_object_unref (G_OBJECT (priv->observers));
	if (priv->sobservers)
		g_object_unref (G_OBJECT (priv->sobservers));
	g_static_rec_mutex_unlock (priv->obs_lock);

	g_static_rec_mutex_lock (priv->folder_lock);
	priv->dont_fkill = FALSE;

	if (priv->account && TNY_IS_CAMEL_STORE_ACCOUNT (priv->account))
	{
		TnyCamelStoreAccountPriv *apriv = TNY_CAMEL_STORE_ACCOUNT_GET_PRIVATE (priv->account);
		apriv->managed_folders = g_list_remove (apriv->managed_folders, self);
	}

	if (!priv->iter_parented && priv->iter)
	{
		CamelStore *store = priv->store;
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

	if (G_LIKELY (priv->receive_strat))
		g_object_unref (G_OBJECT (priv->receive_strat));
	priv->receive_strat = NULL;

	if (G_LIKELY (priv->parent))
		g_object_unref (G_OBJECT (priv->parent));
	priv->parent = NULL;

	g_static_rec_mutex_unlock (priv->folder_lock);

	g_static_rec_mutex_free (priv->folder_lock);
	priv->folder_lock = NULL;

	g_static_rec_mutex_free (priv->obs_lock);
	priv->obs_lock = NULL;

	g_mutex_free (priv->reason_lock);

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
	klass->get_msg_receive_strategy_func = tny_camel_folder_get_msg_receive_strategy;
	klass->set_msg_receive_strategy_func = tny_camel_folder_set_msg_receive_strategy;
	klass->get_headers_func = tny_camel_folder_get_headers;
	klass->get_msg_func = tny_camel_folder_get_msg;
	klass->find_msg_func = tny_camel_folder_find_msg;
	klass->get_msg_async_func = tny_camel_folder_get_msg_async;
	klass->get_id_func = tny_camel_folder_get_id;
	klass->get_name_func = tny_camel_folder_get_name;
	klass->get_folder_type_func = tny_camel_folder_get_folder_type;
	klass->get_unread_count_func = tny_camel_folder_get_unread_count;
	klass->get_all_count_func = tny_camel_folder_get_all_count;
	klass->get_account_func = tny_camel_folder_get_account;
	klass->is_subscribed_func = tny_camel_folder_is_subscribed;
	klass->refresh_async_func = tny_camel_folder_refresh_async;
	klass->refresh_func = tny_camel_folder_refresh;
	klass->remove_msg_func = tny_camel_folder_remove_msg;
	klass->sync_func = tny_camel_folder_sync;
	klass->add_msg_func = tny_camel_folder_add_msg;
	klass->transfer_msgs_func = tny_camel_folder_transfer_msgs;
	klass->transfer_msgs_async_func = tny_camel_folder_transfer_msgs_async;
	klass->copy_func = tny_camel_folder_copy;
	klass->copy_async_func = tny_camel_folder_copy_async;
	klass->poke_status_func = tny_camel_folder_poke_status;
	klass->add_observer_func = tny_camel_folder_add_observer;
	klass->remove_observer_func = tny_camel_folder_remove_observer;
	klass->get_folder_store_func = tny_camel_folder_get_folder_store;
	klass->get_stats_func = tny_camel_folder_get_stats;
	klass->get_url_string_func = tny_camel_folder_get_url_string;
	klass->get_caps_func = tny_camel_folder_get_caps;

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
	klass->add_observer_func = tny_camel_folder_store_add_observer;
	klass->remove_observer_func = tny_camel_folder_store_remove_observer;

	return;
}

static void 
tny_camel_folder_class_init (TnyCamelFolderClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;
	object_class->finalize = tny_camel_folder_finalize;

	class->get_msg_receive_strategy_func = tny_camel_folder_get_msg_receive_strategy_default;
	class->set_msg_receive_strategy_func = tny_camel_folder_set_msg_receive_strategy_default;
	class->get_msg_remove_strategy_func = tny_camel_folder_get_msg_remove_strategy_default;
	class->set_msg_remove_strategy_func = tny_camel_folder_set_msg_remove_strategy_default;
	class->get_headers_func = tny_camel_folder_get_headers_default;
	class->get_msg_func = tny_camel_folder_get_msg_default;
	class->find_msg_func = tny_camel_folder_find_msg_default;
	class->get_msg_async_func = tny_camel_folder_get_msg_async_default;
	class->get_id_func = tny_camel_folder_get_id_default;
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
	class->sync_func = tny_camel_folder_sync_default;
	class->transfer_msgs_func = tny_camel_folder_transfer_msgs_default;
	class->transfer_msgs_async_func = tny_camel_folder_transfer_msgs_async_default;
	class->copy_func = tny_camel_folder_copy_default;
	class->copy_async_func = tny_camel_folder_copy_async_default;
	class->poke_status_func = tny_camel_folder_poke_status_default;
	class->add_observer_func = tny_camel_folder_add_observer_default;
	class->remove_observer_func = tny_camel_folder_remove_observer_default;
	class->get_folder_store_func = tny_camel_folder_get_folder_store_default;
	class->get_stats_func = tny_camel_folder_get_stats_default;
	class->get_url_string_func = tny_camel_folder_get_url_string_default;
	class->get_caps_func = tny_camel_folder_get_caps_default;

	class->get_folders_async_func = tny_camel_folder_get_folders_async_default;
	class->get_folders_func = tny_camel_folder_get_folders_default;
	class->create_folder_func = tny_camel_folder_create_folder_default;
	class->remove_folder_func = tny_camel_folder_remove_folder_default;
	class->add_store_observer_func = tny_camel_folder_store_add_observer_default;
	class->remove_store_observer_func = tny_camel_folder_store_remove_observer_default;

	g_type_class_add_private (object_class, sizeof (TnyCamelFolderPriv));

	return;
}



/**
 * tny_camel_folder_set_strict_retrieval:
 * @self: a #TnyCamelFolder instance
 * @setting: whether or not to enforce strict retrieval
 * 
 * API WARNING: This API might change
 *
 * Sets whether or not the message retrieve strategies need to strictly enforce
 * the retrieval type. For example in case of a partial retrieval strategy,
 * enforce a removal of a full previously retrieved message and retrieve a
 * new message. In case of a full retrieval strategy and a partial cache, remove
 * the partial cache and retrieve the message again.
 *
 **/
void 
tny_camel_folder_set_strict_retrieval (TnyCamelFolder *self, gboolean setting)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);
	priv->strict_retrieval = setting;
}

static void
tny_camel_folder_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyCamelFolder *self = (TnyCamelFolder *)instance;
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (self);

	priv->strict_retrieval = FALSE;
	priv->self = (TnyFolder *) self;
	priv->want_changes = TRUE;
	priv->caps = 0;
	priv->local_size = 0;
	priv->unread_sync = 0;
	priv->dont_fkill = FALSE;
	priv->observers = NULL;
	priv->sobservers = NULL;
	priv->iter = NULL;
	priv->iter_parented = FALSE;
	priv->reason_to_live = 0;
	priv->loaded = FALSE;
	priv->folder_changed_id = 0;
	priv->folder = NULL;
	priv->cached_name = NULL;
	priv->cached_folder_type = TNY_FOLDER_TYPE_UNKNOWN;
	priv->remove_strat = tny_camel_msg_remove_strategy_new ();
	priv->receive_strat = tny_camel_full_msg_receive_strategy_new ();
	priv->reason_lock = g_mutex_new ();
	priv->folder_lock = g_new0 (GStaticRecMutex, 1);
	g_static_rec_mutex_init (priv->folder_lock);
	priv->obs_lock = g_new0 (GStaticRecMutex, 1);
	g_static_rec_mutex_init (priv->obs_lock);

	return;
}

/**
 * tny_camel_folder_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
GType 
tny_camel_folder_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY (!_camel_type_init_done))
	{
		if (!g_thread_supported ()) 
			g_thread_init (NULL);

		camel_type_init ();
		_camel_type_init_done = TRUE;
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

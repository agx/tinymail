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

#include <tny-camel-send-queue.h>
#include <tny-camel-shared.h>
#include <tny-camel-msg.h>
#include <tny-device.h>
#include <tny-simple-list.h>
#include <tny-folder.h>
#include <tny-error.h>

#include <tny-camel-queue-priv.h>
#include <tny-camel-folder.h>
#include <tny-transport-account.h>
#include <tny-store-account.h>
#include <tny-camel-store-account.h>
#include <tny-folder-observer.h>

static GObjectClass *parent_class = NULL;

#include <tny-status.h>
#define TINYMAIL_ENABLE_PRIVATE_API
#include "tny-common-priv.h"
#undef TINYMAIL_ENABLE_PRIVATE_API

#include "tny-camel-send-queue-priv.h"
#include "tny-camel-folder-priv.h"
#include "tny-camel-account-priv.h"
#include "tny-session-camel-priv.h"

#define TNY_CAMEL_SEND_QUEUE_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_CAMEL_SEND_QUEUE, TnyCamelSendQueuePriv))

typedef struct {
	TnySendQueue *self;
	TnyMsg *msg; 
	TnyHeader *header;
	GError *error;
	gint i, total;
} ErrorInfo;

typedef struct {
	TnySendQueue *self;
	TnyMsg *msg;
	TnyHeader *header;
	gint i, total;
	guint signal_id;
} ControlInfo;


static TnyFolder*
get_sentbox (TnySendQueue *self)
{
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);
	TnyFolder *retval = tny_send_queue_get_sentbox (self);

	if (retval) {
		if (!priv->observer_attached) {
			tny_folder_add_observer (retval, TNY_FOLDER_OBSERVER (self));
			priv->observer_attached = TRUE;
		}
	}

	return retval;
}

static gboolean 
emit_error_on_mainloop (gpointer data)
{
	ErrorInfo *info = data;
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (info->self);
	TnyCamelAccountPriv *apriv = NULL;

	if (priv->trans_account)
		apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (priv->trans_account);
	if (apriv)
		tny_lockable_lock (apriv->session->priv->ui_lock);
	g_signal_emit (info->self, tny_send_queue_signals [TNY_SEND_QUEUE_ERROR_HAPPENED], 
				0, info->header, info->msg, info->error);
	if (apriv)
		tny_lockable_unlock (apriv->session->priv->ui_lock);

	return FALSE;
}

static void
destroy_error_info (gpointer data)
{
	ErrorInfo *info = data;

	if (info->msg)
		g_object_unref (info->msg);
	if (info->self)
		g_object_unref (info->self);
	if (info->error)
		g_error_free (info->error);
	g_slice_free (ErrorInfo, info);

	return;
}

static void
emit_error (TnySendQueue *self, TnyHeader *header, TnyMsg *msg, GError *error, int i, int total)
{
	ErrorInfo *info = g_slice_new0 (ErrorInfo);

	if (error != NULL)
		info->error = g_error_copy ((const GError *) error);
	if (self)
		info->self = TNY_SEND_QUEUE (g_object_ref (self));
	if (msg)
		info->msg = TNY_MSG (g_object_ref (msg));
	if (header)
		info->header = TNY_HEADER (g_object_ref (header));
	info->i = i;
	info->total = total;
	g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
		emit_error_on_mainloop, info, destroy_error_info);

	return;
}

static gboolean 
emit_control_signals_on_mainloop (gpointer data)
{
	ControlInfo *info = data;
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (info->self);
	TnyCamelAccountPriv *apriv = NULL;

	if (priv && priv->trans_account)
		apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (priv->trans_account);
	if (apriv)
		tny_lockable_lock (apriv->session->priv->ui_lock);
	g_signal_emit (info->self, tny_send_queue_signals [info->signal_id], 
		0, info->header, info->msg, info->i, info->total);
	if (apriv)
		tny_lockable_unlock (apriv->session->priv->ui_lock);

	return FALSE;
}

static void
destroy_control_info (gpointer data)
{
	ControlInfo *info = data;

	if (info->msg)
		g_object_unref (info->msg);
	if (info->header)
		g_object_unref (info->header);
	if (info->self)
		g_object_unref (info->self);
	g_slice_free (ControlInfo, info);

	return;
}

static void
emit_control (TnySendQueue *self, TnyHeader *header, TnyMsg *msg, guint signal_id, int i, int total)
{
	ControlInfo *info = g_slice_new0 (ControlInfo);

	if (self)
		info->self = TNY_SEND_QUEUE (g_object_ref (self));
	if (msg)
		info->msg = TNY_MSG (g_object_ref (msg));
	if (header)
		info->header = TNY_HEADER (g_object_ref (header));

	info->signal_id = signal_id;
	info->i = i;
	info->total = total;

	g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
			 emit_control_signals_on_mainloop, info, destroy_control_info);
	
	return;
}

static void
on_msg_sent_get_msg (TnyFolder *folder, gboolean cancelled, TnyMsg *msg, GError *err, gpointer user_data)
{
	TnySendQueue *self = (TnySendQueue *) user_data;
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);

	if (!err && !cancelled) {
		TnyHeader *header = tny_msg_get_header (msg);

		g_signal_emit (self, tny_send_queue_signals [TNY_SEND_QUEUE_MSG_SENT], 
			0, header, msg, priv->cur_i, priv->total);

		g_object_unref (header);
	}

	g_object_unref (self);
}

static void 
on_status (GObject *self, TnyStatus *status, gpointer user_data) 
{
	return;
}

static void
tny_camel_send_queue_update (TnyFolderObserver *self, TnyFolderChange *change)
{
	TnyFolderChangeChanged changed;
	TnyFolder *folder = NULL;
	TnyFolder *sentbox = tny_send_queue_get_sentbox (TNY_SEND_QUEUE (self));

	changed = tny_folder_change_get_changed (change);
	folder = tny_folder_change_get_folder (change);

	if (folder == sentbox)
	{
		if (changed & TNY_FOLDER_CHANGE_CHANGED_ADDED_HEADERS)
		{
			TnyList *list = tny_simple_list_new ();
			TnyIterator *iter = NULL;
			tny_folder_change_get_added_headers (change, list);
			iter = tny_list_create_iterator (list);
			while (!tny_iterator_is_done (iter))
			{
				TnyHeader *cur = TNY_HEADER (tny_iterator_get_current (iter));
				tny_folder_get_msg_async (sentbox, cur, 
					on_msg_sent_get_msg, on_status, 
					g_object_ref (self));
				g_object_unref (cur);
				tny_iterator_next (iter);
			}
			g_object_unref (iter);
			g_object_unref (list);
		}
	}

	g_object_unref (folder);
	g_object_unref (sentbox);

	return;
}

static gpointer
thread_main (gpointer data)
{
	TnySendQueue *self = (TnySendQueue *) data;
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);
	TnyFolder *sentbox = NULL, *outbox = NULL;
	guint i = 0, length = 0;
	TnyList *list = NULL;
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (priv->trans_account);
	TnySessionCamelPriv *spriv = TNY_SESSION_CAMEL (apriv->session)->priv;
	TnyDevice *device = g_object_ref (spriv->device);

	g_object_ref (self); /* My own reference */
	priv->is_running = TRUE;
	list = tny_simple_list_new ();

	g_mutex_lock (priv->todo_lock);
	{
		GError *terror = NULL;
		sentbox = get_sentbox (self);
		outbox = tny_send_queue_get_outbox (self);

		tny_folder_get_headers (outbox, list, TRUE, &terror);

		if (terror != NULL)
		{
			emit_error (self, NULL, NULL, terror, i, priv->total);
			g_error_free (terror);
			g_object_unref (list);
			g_mutex_unlock (priv->todo_lock);
			goto errorhandler;
		}

		length = tny_list_get_length (list);
		priv->total = length;
	}
	g_mutex_unlock (priv->todo_lock);

	g_object_unref (list);

	priv->do_continue = TRUE;

	while (length > 0 && priv->do_continue && tny_device_is_online (device))
	{
		TnyHeader *header = NULL;
		TnyMsg *msg = NULL;

		g_mutex_lock (priv->sending_lock);

		g_mutex_lock (priv->todo_lock);
		{
			GError *ferror = NULL;
			TnyIterator *hdriter = NULL;
			TnyList *headers = tny_simple_list_new ();
			GList *to_remove = NULL;
			TnyIterator *giter = NULL;

			if (tny_device_is_online (device))
				tny_folder_get_headers (outbox, headers, TRUE, &ferror);
			else {
				priv->is_running = FALSE;
				g_mutex_unlock (priv->todo_lock);
				g_mutex_unlock (priv->sending_lock);
				goto errorhandler;
			}

			if (ferror != NULL)
			{
				priv->is_running = FALSE;
				emit_error (self, NULL, NULL, ferror, i, priv->total);
				g_error_free (ferror);
				g_object_unref (headers);
				g_mutex_unlock (priv->todo_lock);
				g_mutex_unlock (priv->sending_lock);
				goto errorhandler;
			}

			/* Some code to remove the suspended items */
			giter = tny_list_create_iterator (headers);
			while (!tny_iterator_is_done (giter))
			{
				TnyHeader *curhdr = TNY_HEADER (tny_iterator_get_current (giter));
				TnyHeaderFlags flags = tny_header_get_flags (curhdr);

				flags &= TNY_HEADER_FLAG_PRIORITY;
				if (flags & TNY_HEADER_FLAG_SUSPENDED_PRIORITY)
					to_remove = g_list_prepend (to_remove, curhdr);
				g_object_unref (curhdr);
				tny_iterator_next (giter);
			}
			g_object_unref (giter);

			while (to_remove) {
				tny_list_remove (headers, G_OBJECT (to_remove->data));
				to_remove = g_list_next (to_remove);
			}
			if (to_remove)
				g_list_free (to_remove);

			length = tny_list_get_length (headers);
			priv->total = length;

			if (length <= 0)
			{
				priv->is_running = FALSE;
				g_object_unref (headers);
				g_mutex_unlock (priv->todo_lock);
				g_mutex_unlock (priv->sending_lock);
				break;
			}

			hdriter = tny_list_create_iterator (headers);
			header = (TnyHeader *) tny_iterator_get_current (hdriter);

			g_object_unref (hdriter);
			g_object_unref (headers);
		}
		g_mutex_unlock (priv->todo_lock);

		if (header && TNY_IS_HEADER (header) && tny_device_is_online (device))
		{
			TnyList *hassent = tny_simple_list_new ();
			GError *err = NULL;

			tny_list_prepend (hassent, G_OBJECT (header));
			msg = tny_folder_get_msg (outbox, header, &err);

			/* Emits msg-sending signal to inform a new msg is being sent */
			emit_control (self, header, msg, TNY_SEND_QUEUE_MSG_SENDING, i, priv->total);

			if (err == NULL) 
			{
				tny_transport_account_send (priv->trans_account, msg, &err);

				if (err != NULL) {
					emit_error (self, header, msg, err, i, priv->total);
					priv->do_continue = FALSE;
				}
			} else {
				emit_error (self, header, msg, err, i, priv->total);
				priv->do_continue = FALSE;
			}

			g_mutex_lock (priv->todo_lock);
			{
				if (err == NULL)
				{
					GError *newerr = NULL;
					priv->cur_i = i;
					tny_folder_transfer_msgs (outbox, hassent, sentbox, TRUE, &newerr);
					if (newerr != NULL) 
					{
						emit_error (self, header, msg, newerr, i, priv->total);
						priv->do_continue = FALSE;
						g_error_free (newerr);
					}
					priv->total--;
				}
			}
			g_mutex_unlock (priv->todo_lock);

			/* Emits msg-sent signal to inform msg has been sent */
			/* This now happens in on_msg_sent_get_msg! */

			g_object_unref (hassent);

			if (err != NULL)
				g_error_free (err);

			i++;

			/* hassent is owner now */
			g_object_unref (header);
		} else 
		{
			priv->is_running = FALSE;
			/* Not good, or we just went offline, let's just kill this thread */ 
			length = 0;
			if (header && G_IS_OBJECT (header))
				g_object_unref (header);
			g_mutex_unlock (priv->sending_lock);
			break;
		}

		g_mutex_unlock (priv->sending_lock);

	}

	priv->is_running = FALSE;

errorhandler:

	priv->is_running = FALSE;

	g_object_unref (device);
	g_object_unref (sentbox);
	g_object_unref (outbox);
	g_object_unref (self); /* The one added here (My own reference) */
	g_object_unref (self); /* Thread reference (Before creation reference) */

	priv->thread = NULL;

	return NULL;
}

static void 
create_worker (TnySendQueue *self)
{
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);

	if (!priv->is_running)
	{
		g_object_ref (self); /* Before creation reference */
		priv->thread = g_thread_create (thread_main, self, FALSE, NULL);
	}

	return;
}


static void
tny_camel_send_queue_cancel (TnySendQueue *self, gboolean remove, GError **err)
{
	TNY_CAMEL_SEND_QUEUE_GET_CLASS (self)->cancel_func (self, remove, err);
}

static void
tny_camel_send_queue_cancel_default (TnySendQueue *self, gboolean remove, GError **err)
{
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);

	g_mutex_lock (priv->sending_lock);

	priv->is_running = FALSE;
	priv->do_continue = FALSE;

	if (remove)
	{
		TnyFolder *outbox;
		TnyList *headers = tny_simple_list_new ();
		TnyIterator *iter;

		outbox = tny_send_queue_get_outbox (self);

		tny_folder_get_headers (outbox, headers, TRUE, err);

		if (err != NULL && *err != NULL)
		{
			g_object_unref (headers);
			g_object_unref (outbox);
			g_mutex_unlock (priv->sending_lock);
			return;
		}

		iter = tny_list_create_iterator (headers);

		while (!tny_iterator_is_done (iter))
		{
			TnyHeader *header = TNY_HEADER (tny_iterator_get_current (iter));
			tny_folder_remove_msg (outbox, header, err);

			if (err != NULL && *err != NULL)
			{
				g_object_unref (header);
				g_object_unref (iter);
				g_object_unref (headers);
				g_object_unref (outbox);
				g_mutex_unlock (priv->sending_lock);
				return;
			}

			g_object_unref (header);
			tny_iterator_next (iter);
		}
		g_object_unref (iter);
		g_object_unref (headers);

		tny_folder_sync (outbox, TRUE, err);
		g_object_unref (outbox);
	}

	g_mutex_unlock (priv->sending_lock);

	return;
}


static void
tny_camel_send_queue_add (TnySendQueue *self, TnyMsg *msg, GError **err)
{
	TNY_CAMEL_SEND_QUEUE_GET_CLASS (self)->add_func (self, msg, err);
}

typedef struct {
	TnyMsg *msg;
	TnySendQueue *self;
	TnySendQueueAddCallback callback;
	TnyStatusCallback status_callback;
	TnyIdleStopper* stopper;
	gboolean cancelled;
	guint depth;
	TnySessionCamel *session;
	GCond* condition;
	gboolean had_callback;
	GMutex *mutex;
	gpointer user_data;
} OnAddedInfo;


static void
on_added (TnyFolder *folder, gboolean cancelled, GError *err, gpointer user_data)
{
	OnAddedInfo *info = (OnAddedInfo *) user_data;
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (info->self);

	/* Call user callback after msg has beed added to OUTBOX, waiting to be sent*/
	if (info->callback)
		info->callback (info->self, info->cancelled, info->msg, info->user_data, err);

	priv->total++;
	if (priv->total >= 1 && !priv->is_running)
		create_worker (info->self);
	if (info->self)
		g_object_unref (info->self);
	if (info->msg)
		g_object_unref (info->msg);

	g_slice_free (OnAddedInfo, info);

	return;
}

static void
tny_camel_send_queue_add_default (TnySendQueue *self, TnyMsg *msg, GError **err)
{
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);

	g_assert (TNY_IS_CAMEL_MSG (msg));

	g_mutex_lock (priv->todo_lock);
	{
		TnyFolder *outbox;
		TnyList *headers = tny_simple_list_new ();
		OnAddedInfo *info = NULL;

		outbox = tny_send_queue_get_outbox (self);

		if (!outbox || !TNY_IS_FOLDER (outbox))
		{
			g_set_error (err, TNY_SEND_QUEUE_ERROR, 
				TNY_SEND_QUEUE_ERROR_ADD,
				"Operating can't continue: send queue not ready "
				"because it does not have a valid outbox. "
				"This problem indicates a bug in the software.");
			g_object_unref (headers);
			g_mutex_unlock (priv->todo_lock);
			return;
		}

		tny_folder_get_headers (outbox, headers, TRUE, err);

		if (err!= NULL && *err != NULL)
		{
			g_object_unref (headers);
			g_object_unref (outbox);
			g_mutex_unlock (priv->todo_lock);
			return;
		}

		priv->total = tny_list_get_length (headers);
		g_object_unref (headers);

		info = g_slice_new0 (OnAddedInfo);
		info->msg = TNY_MSG (g_object_ref (msg));
		info->self = TNY_SEND_QUEUE (g_object_ref (self));
		info->callback = NULL;

		tny_folder_add_msg_async (outbox, msg, on_added, on_status, info);

		g_object_unref (outbox);
	}
	g_mutex_unlock (priv->todo_lock);

	return;
}


void
tny_camel_send_queue_add_async (TnyCamelSendQueue *self, TnyMsg *msg, TnySendQueueAddCallback callback, TnyStatusCallback status_callback, gpointer user_data)
{
	TNY_CAMEL_SEND_QUEUE_GET_CLASS (self)->add_async_func (self, msg, callback, status_callback, user_data);
}


static void
tny_camel_send_queue_add_async_default (TnyCamelSendQueue *self, TnyMsg *msg, TnySendQueueAddCallback callback, TnyStatusCallback status_callback, gpointer user_data)
{
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);
	GError *err = NULL;

	g_assert (TNY_IS_CAMEL_MSG (msg));

	g_mutex_lock (priv->todo_lock);
	{
		TnyFolder *outbox;
		TnyList *headers = tny_simple_list_new ();
		OnAddedInfo *info = NULL;

		outbox = tny_send_queue_get_outbox (TNY_SEND_QUEUE (self));

		if (!outbox || !TNY_IS_FOLDER (outbox))
		{
			g_set_error (&err, TNY_SEND_QUEUE_ERROR, 
				TNY_SEND_QUEUE_ERROR_ADD,
				"Operating can't continue: send queue not ready "
				"because it does not have a valid outbox. "
				"This problem indicates a bug in the software.");
			g_object_unref (headers);
			g_mutex_unlock (priv->todo_lock);
			return;
		}

		tny_folder_get_headers (outbox, headers, TRUE, &err);

		if (err!= NULL)
		{
			g_object_unref (headers);
			g_object_unref (outbox);
			g_mutex_unlock (priv->todo_lock);
			return;
		}

		priv->total = tny_list_get_length (headers);
		g_object_unref (headers);

		info = g_slice_new0 (OnAddedInfo);

		info->msg = TNY_MSG (g_object_ref (msg));
		info->self = TNY_SEND_QUEUE (g_object_ref (self));
		info->callback = callback;
		info->user_data = user_data;

		tny_folder_add_msg_async (outbox, msg, on_added, on_status, info);

		g_object_unref (outbox);
	}
	g_mutex_unlock (priv->todo_lock);

	return;
}


static TnyFolder*
create_maildir (TnySendQueue *self, const gchar *name)
{
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (TNY_CAMEL_ACCOUNT (priv->trans_account));
	CamelStore *store = (CamelStore*) apriv->service;
	CamelSession *session = (CamelSession*) apriv->session;
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	gchar *full_path;
	const gchar *aname;
	CamelStore *mdstore = NULL;

	aname = tny_account_get_name (TNY_ACCOUNT (priv->trans_account));
	if (aname == NULL)
		aname = tny_account_get_id (TNY_ACCOUNT (priv->trans_account));

	g_assert (aname);

	full_path = g_strdup_printf ("maildir://%s/%s/maildir", session->storage_path, aname);

	mdstore = camel_session_get_store(session, full_path, &ex);

	if (!camel_exception_is_set (&ex) && mdstore)
	{
		CamelFolder *cfolder = NULL;

		cfolder = camel_store_get_folder (mdstore, name, CAMEL_STORE_FOLDER_CREATE, &ex);
		if (!camel_exception_is_set (&ex) && cfolder)
		{
			CamelFolderInfo *iter;

			iter = camel_store_get_folder_info (mdstore, name, 
					CAMEL_STORE_FOLDER_INFO_FAST|CAMEL_STORE_FOLDER_INFO_NO_VIRTUAL,&ex);

			if (!camel_exception_is_set (&ex) && iter)
			{
				TnyCamelFolder *folder = TNY_CAMEL_FOLDER (_tny_camel_folder_new ());
				TnyCamelFolderPriv *fpriv = TNY_CAMEL_FOLDER_GET_PRIVATE (folder);
				TnyStoreAccount *saccount = tny_camel_store_account_new ();

				fpriv->dont_fkill = TRUE; /* Magic :) */
				_tny_camel_folder_set_id (folder, iter->full_name);
				_tny_camel_folder_set_folder_type (folder, iter);
				_tny_camel_folder_set_unread_count (folder, iter->unread);
				_tny_camel_folder_set_all_count (folder, iter->total);
				_tny_camel_folder_set_local_size (folder, iter->local_size);
				_tny_camel_folder_set_name (folder, iter->name);
				_tny_camel_folder_set_iter (folder, iter);

				tny_camel_account_set_session (TNY_CAMEL_ACCOUNT (saccount), 
					(TnySessionCamel *) session);
				_tny_camel_folder_set_account (folder, TNY_ACCOUNT (saccount));

				fpriv->store = mdstore;

				g_free (full_path);

				return TNY_FOLDER (folder);

			} else if (iter && CAMEL_IS_STORE (mdstore))
				camel_store_free_folder_info (mdstore, iter);

		} else 
		{
			g_critical (_("Can't create folder \"%s\" in %s"), name, full_path);
			if (cfolder && CAMEL_IS_OBJECT (cfolder))
				camel_object_unref (CAMEL_OBJECT (cfolder));
		}
	} else 
	{
		g_critical (_("Can't create store on %s"), full_path);
		if (store && CAMEL_IS_OBJECT (mdstore))
			camel_object_unref (CAMEL_OBJECT (mdstore));
	}

	g_free (full_path);

	return NULL;
}

static TnyFolder* 
tny_camel_send_queue_get_sentbox (TnySendQueue *self)
{
	return TNY_CAMEL_SEND_QUEUE_GET_CLASS (self)->get_sentbox_func (self);
}

static TnyFolder* 
tny_camel_send_queue_get_sentbox_default (TnySendQueue *self)
{
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);

	if (!priv->sentbox_cache)
		priv->sentbox_cache = create_maildir (self, "sentbox");

	return TNY_FOLDER (g_object_ref (G_OBJECT (priv->sentbox_cache)));
}


static TnyFolder* 
tny_camel_send_queue_get_outbox (TnySendQueue *self)
{
	return TNY_CAMEL_SEND_QUEUE_GET_CLASS (self)->get_outbox_func (self);
}

static TnyFolder* 
tny_camel_send_queue_get_outbox_default (TnySendQueue *self)
{
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);

	if (!priv->outbox_cache)
		priv->outbox_cache = create_maildir (self, "outbox");

	return TNY_FOLDER (g_object_ref (G_OBJECT (priv->outbox_cache)));
}

static void
tny_camel_send_queue_finalize (GObject *object)
{
	TnySendQueue *self = (TnySendQueue*) object;
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);
	TnyCamelAccountPriv *apriv = NULL;
	TnyFolder *sentbox = NULL;

	sentbox = tny_send_queue_get_sentbox (self);

	if (sentbox) {
		if (priv->observer_attached)
			tny_folder_remove_observer (sentbox, TNY_FOLDER_OBSERVER (self));
		g_object_unref (sentbox);
	}

	if (priv->trans_account)
	{
		apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (priv->trans_account);
		_tny_session_camel_unreg_queue (apriv->session, (TnyCamelSendQueue *) self);
	}

	g_mutex_lock (priv->todo_lock);

	if (priv->signal != -1)
		g_signal_handler_disconnect (G_OBJECT (priv->trans_account), 
			priv->signal);
	if (priv->sentbox_cache)
		g_object_unref (priv->sentbox_cache);
	if (priv->outbox_cache)
		g_object_unref (priv->outbox_cache);

	g_mutex_unlock (priv->todo_lock);

	g_mutex_free (priv->todo_lock);
	g_mutex_free (priv->sending_lock);
	g_object_unref (priv->trans_account);

	(*parent_class->finalize) (object);

	return;
}


/**
 * tny_camel_send_queue_new:
 * @trans_account: A #TnyCamelTransportAccount instance
 *
 * Create a new #TnySendQueue instance implemented for Camel
 *
 * Return value: A new #TnySendQueue instance implemented for Camel
 **/
TnySendQueue*
tny_camel_send_queue_new (TnyCamelTransportAccount *trans_account)
{
	TnyCamelSendQueue *self = g_object_new (TNY_TYPE_CAMEL_SEND_QUEUE, NULL);

	g_assert (TNY_IS_CAMEL_TRANSPORT_ACCOUNT (trans_account));
	tny_camel_send_queue_set_transport_account (self, trans_account);

	return TNY_SEND_QUEUE (self);
}

static void
on_setonline_happened (TnyCamelAccount *account, gboolean online, gpointer user_data)
{
	if (online)
		tny_camel_send_queue_flush (TNY_CAMEL_SEND_QUEUE (user_data));
	return;
}


/**
 * tny_camel_send_queue_set_transport_account:
 * @self: a valid #TnyCamelSendQueue instance
 * @trans_account: A #TnyCamelTransportAccount instance
 *
 * set the transport account for this send queue.
 * 
 **/
void
tny_camel_send_queue_set_transport_account (TnyCamelSendQueue *self,
					    TnyCamelTransportAccount *trans_account)
{
	TnyCamelSendQueuePriv *priv;
	TnyCamelAccountPriv *apriv = NULL;

	g_return_if_fail (TNY_IS_CAMEL_SEND_QUEUE(self));
	g_return_if_fail (TNY_IS_CAMEL_TRANSPORT_ACCOUNT(trans_account));

	priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);

	if (priv->trans_account) 
	{
		apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (priv->trans_account);
		_tny_session_camel_unreg_queue (apriv->session, self);

		g_object_unref (priv->trans_account);

		if (priv->signal != -1)
			g_signal_handler_disconnect (G_OBJECT (priv->trans_account), 
				priv->signal);
	}

	priv->signal = (gint) g_signal_connect (G_OBJECT (trans_account),
		"set-online-happened", G_CALLBACK (on_setonline_happened), self);
	priv->trans_account = TNY_TRANSPORT_ACCOUNT (g_object_ref(trans_account));
	apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (priv->trans_account);
	_tny_session_camel_reg_queue (apriv->session, self);
	tny_camel_send_queue_flush (TNY_CAMEL_SEND_QUEUE (self));

	return;

}

/**
 * tny_camel_send_queue_get_transport_account:
 * @self: a valid #TnyCamelSendQueue instance
 *
 * Get the transport account for this send queue. If not NULL, the returned value 
 * must be unreferences when no longer needed.
 *
 * Return value: A #TnyCamelTransportAccount instance or NULL
 **/
TnyCamelTransportAccount*
tny_camel_send_queue_get_transport_account (TnyCamelSendQueue *self)
{
	TnyCamelSendQueuePriv *priv;

	g_return_val_if_fail (TNY_IS_CAMEL_SEND_QUEUE(self), NULL);
	priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);
	if (!priv->trans_account)
		return NULL;
	g_object_ref (priv->trans_account);

	return TNY_CAMEL_TRANSPORT_ACCOUNT(priv->trans_account);
}

/**
 * tny_camel_send_queue_flush:
 * @self: a valid #TnyCamelSendQueue instance
 *
 * Flush the messages which are currently in this send queue
 **/
void
tny_camel_send_queue_flush (TnyCamelSendQueue *self)
{
	g_return_if_fail (TNY_IS_CAMEL_SEND_QUEUE(self));
	create_worker (TNY_SEND_QUEUE (self));
}



static void
tny_send_queue_init (gpointer g, gpointer iface_data)
{
	TnySendQueueIface *klass = (TnySendQueueIface *)g;

	klass->add_func = tny_camel_send_queue_add;
	klass->get_outbox_func = tny_camel_send_queue_get_outbox;
	klass->get_sentbox_func = tny_camel_send_queue_get_sentbox;
	klass->cancel_func = tny_camel_send_queue_cancel;

	return;
}

static void 
tny_camel_send_queue_class_init (TnyCamelSendQueueClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	class->add_func = tny_camel_send_queue_add_default;
	class->add_async_func = tny_camel_send_queue_add_async_default;
	class->get_outbox_func = tny_camel_send_queue_get_outbox_default;
	class->get_sentbox_func = tny_camel_send_queue_get_sentbox_default;
	class->cancel_func = tny_camel_send_queue_cancel_default;

	object_class->finalize = tny_camel_send_queue_finalize;

	g_type_class_add_private (object_class, sizeof (TnyCamelSendQueuePriv));

	return;
}


static void
tny_camel_send_queue_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyCamelSendQueue *self = (TnyCamelSendQueue*)instance;
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);

	priv->observer_attached = FALSE;
	priv->signal = -1;
	priv->sentbox_cache = NULL;
	priv->outbox_cache = NULL;
	priv->todo_lock = g_mutex_new ();
	priv->sending_lock = g_mutex_new ();
	priv->do_continue = FALSE;
	priv->is_running = FALSE;
	priv->thread = NULL;

	return;
}

static void
tny_folder_observer_init (TnyFolderObserverIface *klass)
{
	klass->update_func = tny_camel_send_queue_update;
}

/**
 * tny_camel_send_queue_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
GType 
tny_camel_send_queue_get_type (void)
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
		  sizeof (TnyCamelSendQueueClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_camel_send_queue_class_init, /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyCamelSendQueue),
		  0,      /* n_preallocs */
		  tny_camel_send_queue_instance_init,    /* instance_init */
		  NULL
		};

		static const GInterfaceInfo tny_folder_observer_info = {
			(GInterfaceInitFunc) tny_folder_observer_init,
			NULL,
			NULL
		};

		static const GInterfaceInfo tny_send_queue_info = 
		{
		  (GInterfaceInitFunc) tny_send_queue_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyCamelSendQueue",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_FOLDER_OBSERVER,
			&tny_folder_observer_info);

		g_type_add_interface_static (type, TNY_TYPE_SEND_QUEUE,
			&tny_send_queue_info);
	}

	return type;
}


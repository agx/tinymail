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
#include <tny-simple-list.h>
#include <tny-folder.h>

#include <tny-camel-folder.h>
#include <tny-transport-account.h>

static GObjectClass *parent_class = NULL;

#include "tny-camel-send-queue-priv.h"
#include "tny-camel-folder-priv.h"
#include "tny-camel-account-priv.h"

#define TNY_CAMEL_SEND_QUEUE_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_CAMEL_SEND_QUEUE, TnyCamelSendQueuePriv))



static gpointer
thread_main (gpointer data)
{
	TnySendQueue *self = (TnySendQueue *) data;
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);
	TnyFolder *sentbox, *outbox;
	guint i = 0, length = 0;
	TnyList *list;

	priv->creating_spin = FALSE;

	list = tny_simple_list_new ();

	g_mutex_lock (priv->todo_lock);
	{
		GError *terror = NULL;
		sentbox = tny_send_queue_get_sentbox (self);
		outbox = tny_send_queue_get_outbox (self);
		tny_folder_get_headers (outbox, list, TRUE, &terror);
		if (terror != NULL)
		{
			g_signal_emit (self, tny_send_queue_signals [TNY_SEND_QUEUE_ERROR_HAPPENED], 
				0, NULL, terror, i, priv->total);
			g_object_unref (G_OBJECT (list));
			goto errorhandler;
		}

		length = tny_list_get_length (list);
		priv->total = length;
	}
	g_mutex_unlock (priv->todo_lock);

	g_object_unref (G_OBJECT (list));

	while (length > 0)
	{
		TnyHeader *header;
		TnyMsg *msg;

		g_mutex_lock (priv->todo_lock);
		{
			GError *ferror = NULL;
			TnyIterator *hdriter;
			TnyList *headers = tny_simple_list_new ();

			tny_folder_get_headers (outbox, headers, TRUE, &ferror);

			if (ferror != NULL)
			{
				g_signal_emit (self, tny_send_queue_signals [TNY_SEND_QUEUE_ERROR_HAPPENED], 
					0, msg, ferror, i, priv->total);
				g_object_unref (G_OBJECT (headers));
				goto errorhandler;
			}

			length = tny_list_get_length (headers);

			priv->total = length;

			if (length <= 0)
			{
				g_object_unref (G_OBJECT (headers));
				g_mutex_unlock (priv->todo_lock);
				break;
			}
			hdriter = tny_list_create_iterator (headers);
			header = (TnyHeader *) tny_iterator_get_current (hdriter);

			g_object_unref (G_OBJECT (hdriter));
			g_object_unref (G_OBJECT (headers));
		}
		g_mutex_unlock (priv->todo_lock);


		if (header && TNY_IS_HEADER (header))
		{
			TnyList *hassent = tny_simple_list_new ();
			GError *err = NULL;

			tny_list_prepend (hassent, G_OBJECT (header));
			msg = tny_folder_get_msg (outbox, header, &err);
			g_object_unref (G_OBJECT (header));	

			if (err == NULL) 
			{
				tny_transport_account_send (priv->trans_account, msg, &err);
				if (err != NULL)
					g_signal_emit (self, tny_send_queue_signals [TNY_SEND_QUEUE_ERROR_HAPPENED], 
						0, msg, err, i, priv->total);
			} else
				g_signal_emit (self, tny_send_queue_signals [TNY_SEND_QUEUE_ERROR_HAPPENED], 
					0, msg, err, i, priv->total);

			g_mutex_lock (priv->todo_lock);
			{
				if (err == NULL)
				{
					GError *newerr = NULL;
					tny_folder_transfer_msgs (outbox, hassent, sentbox, TRUE, &newerr);
					if (newerr != NULL)
						g_signal_emit (self, tny_send_queue_signals [TNY_SEND_QUEUE_ERROR_HAPPENED], 
							0, msg, newerr, i, priv->total);

					priv->total--;
				}
			}
			g_mutex_unlock (priv->todo_lock);

			g_object_unref (G_OBJECT (hassent));

			if (err == NULL)
				g_signal_emit (self, tny_send_queue_signals [TNY_SEND_QUEUE_MSG_SENT], 
					0, msg, i, priv->total);

			i++;
		} else 
		{

			/* Not good, let's just kill this thread */ 

			length = 0;
			if (header && G_IS_OBJECT (header))
				g_object_unref (G_OBJECT (header));
		}
	}

errorhandler:

	g_object_unref (G_OBJECT (sentbox));
	g_object_unref (G_OBJECT (outbox));

	priv->thread = NULL;

	g_thread_exit (NULL);
	return NULL;
}

static void 
create_worker (TnySendQueue *self)
{
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);

	while (priv->creating_spin);

	priv->creating_spin = TRUE;
	priv->thread = g_thread_create (thread_main, self, TRUE, NULL);

	return;
}


static void
tny_camel_send_queue_add (TnySendQueue *self, TnyMsg *msg)
{
	TNY_CAMEL_SEND_QUEUE_GET_CLASS (self)->add_func (self, msg);
}

static void
tny_camel_send_queue_add_default (TnySendQueue *self, TnyMsg *msg)
{
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);

	g_assert (TNY_IS_CAMEL_MSG (msg));
	
	g_mutex_lock (priv->todo_lock);
	{
		TnyFolder *outbox;
		TnyList *headers = tny_simple_list_new ();

		outbox = tny_send_queue_get_outbox (self);

		/* TODO handle and report errors here */
		tny_folder_get_headers (outbox, headers, TRUE, NULL);
		priv->total = tny_list_get_length (headers);
		g_object_unref (G_OBJECT (headers));

		/* TODO error checking and reporting here */
		tny_folder_add_msg (outbox, msg, NULL);
		priv->total++;

		if (priv->total >= 1 && !priv->thread && !priv->creating_spin)
			create_worker (self);

		g_object_unref (G_OBJECT (outbox));
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

	/*	mdstore = _tny_camel_account_get_service (TNY_CAMEL_ACCOUNT (priv->trans_account)); */

	if (!camel_exception_is_set (&ex) && mdstore)
	{
		CamelFolder *cfolder = NULL;

		cfolder = camel_store_get_folder (mdstore, name, CAMEL_STORE_FOLDER_CREATE, &ex);
		if (!camel_exception_is_set (&ex) && cfolder)
		{
			CamelFolderInfo *iter;

			/* camel_object_unref (CAMEL_OBJECT (cfolder)); */

			iter = camel_store_get_folder_info (mdstore, name, 
					CAMEL_STORE_FOLDER_INFO_FAST|CAMEL_STORE_FOLDER_INFO_NO_VIRTUAL,&ex);

			if (!camel_exception_is_set (&ex) && iter)
			{
				TnyCamelFolder *folder = TNY_CAMEL_FOLDER (tny_camel_folder_new ());
				TnyCamelFolderPriv *fpriv = TNY_CAMEL_FOLDER_GET_PRIVATE (folder);

				_tny_camel_folder_set_id (folder, iter->full_name);
				_tny_camel_folder_set_folder_type (folder, iter);
				_tny_camel_folder_set_unread_count (folder, iter->unread);
				_tny_camel_folder_set_all_count (folder, iter->total);
				_tny_camel_folder_set_name (folder, iter->name);
				_tny_camel_folder_set_iter (folder, iter);
				_tny_camel_folder_set_account (folder, TNY_ACCOUNT (priv->trans_account));

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
	TnyCamelSendQueue *self = (TnyCamelSendQueue*) object;
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);

	g_mutex_lock (priv->todo_lock);
		
	if (priv->sentbox_cache)
		g_object_unref (G_OBJECT (priv->sentbox_cache));

	if (priv->outbox_cache)
		g_object_unref (G_OBJECT (priv->outbox_cache));

	g_mutex_unlock (priv->todo_lock);
	g_mutex_free (priv->todo_lock);

	g_object_unref (G_OBJECT (priv->trans_account));
	
	(*parent_class->finalize) (object);

	return;
}

/**
 * tny_camel_send_queue_new:
 * @trans_account: A #TnyCamelTransportAccount instance
 *
 *
 * Return value: A new #TnySendQueue instance implemented for Camel
 **/
TnySendQueue*
tny_camel_send_queue_new (TnyCamelTransportAccount *trans_account)
{
	TnyCamelSendQueue *self = g_object_new (TNY_TYPE_CAMEL_SEND_QUEUE, NULL);
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);

	g_assert (TNY_IS_CAMEL_TRANSPORT_ACCOUNT (trans_account));
	
	priv->trans_account = TNY_TRANSPORT_ACCOUNT (g_object_ref (G_OBJECT (trans_account)));
	
	g_mutex_lock (priv->todo_lock);
	{
		TnyFolder *outbox;
		TnyList *headers = tny_simple_list_new ();

		outbox = tny_send_queue_get_outbox (TNY_SEND_QUEUE (self));

		/* TODO handle and report errors here */
		tny_folder_get_headers (outbox, headers, TRUE, NULL);
		priv->total = tny_list_get_length (headers);
		g_object_unref (G_OBJECT (headers));

		if (priv->total >= 1)
			create_worker (TNY_SEND_QUEUE (self));

		g_object_unref (G_OBJECT (outbox));
	}
	g_mutex_unlock (priv->todo_lock);


	return TNY_SEND_QUEUE (self);
}


static void
tny_send_queue_init (gpointer g, gpointer iface_data)
{
	TnySendQueueIface *klass = (TnySendQueueIface *)g;

	klass->add_func = tny_camel_send_queue_add;
	klass->get_outbox_func = tny_camel_send_queue_get_outbox;
	klass->get_sentbox_func = tny_camel_send_queue_get_sentbox;

	return;
}

static void 
tny_camel_send_queue_class_init (TnyCamelSendQueueClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	class->add_func = tny_camel_send_queue_add_default;
	class->get_outbox_func = tny_camel_send_queue_get_outbox_default;
	class->get_sentbox_func = tny_camel_send_queue_get_sentbox_default;

	object_class->finalize = tny_camel_send_queue_finalize;

	g_type_class_add_private (object_class, sizeof (TnyCamelSendQueuePriv));

	return;
}


static void
tny_camel_send_queue_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyCamelSendQueue *self = (TnyCamelSendQueue*)instance;
	TnyCamelSendQueuePriv *priv = TNY_CAMEL_SEND_QUEUE_GET_PRIVATE (self);

	priv->creating_spin = FALSE;
	priv->sentbox_cache = NULL;
	priv->outbox_cache = NULL;
	priv->todo_lock = g_mutex_new ();

	return;
}

GType 
tny_camel_send_queue_get_type (void)
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

		static const GInterfaceInfo tny_send_queue_info = 
		{
		  (GInterfaceInitFunc) tny_send_queue_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyCamelSendQueue",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_SEND_QUEUE,
			&tny_send_queue_info);

	}

	return type;
}


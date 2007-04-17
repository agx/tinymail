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

#include <config.h>
#include <glib.h>
#include <glib/gi18n-lib.h>

#include <tny-merge-folder.h>
#include <tny-error.h>
#include <tny-simple-list.h>

static GObjectClass *parent_class = NULL;

typedef struct _TnyMergeFolderPriv TnyMergeFolderPriv;

struct _TnyMergeFolderPriv
{
	gchar *id, *name;
	TnyList *mothers;
};

#define TNY_MERGE_FOLDER_GET_PRIVATE(o) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_MERGE_FOLDER, TnyMergeFolderPriv))


static void
tny_merge_folder_remove_msg (TnyFolder *self, TnyHeader *header, GError **err)
{
	TnyFolder *fol = tny_header_get_folder (header);

	tny_folder_remove_msg (fol, header, err);
	g_object_unref (fol);

	return;
}

static void
tny_merge_folder_add_msg (TnyFolder *self, TnyMsg *msg, GError **err)
{
	g_warning ("tny_merge_folder_add_msg not implemented: "
		   "add it to the mother folder instead\n");

	g_set_error (err, TNY_FOLDER_ERROR, 
		TNY_FOLDER_ERROR_ADD_MSG,
		"tny_merge_folder_add_msg not implemented: "
		"add it to the mother folder instead");
}

static void
tny_merge_folder_sync (TnyFolder *self, gboolean expunge, GError **err)
{
	TnyMergeFolderPriv *priv = TNY_MERGE_FOLDER_GET_PRIVATE (self);
	TnyIterator *iter;

	iter = tny_list_create_iterator (priv->mothers);
	while (!tny_iterator_is_done (iter))
	{
		TnyFolder *cur = TNY_FOLDER (tny_iterator_get_current (iter));
		tny_folder_sync (cur, expunge, err);

		/* TODO: Handle err ! */

		g_object_unref (cur);
		tny_iterator_next (iter);
	}

	g_object_unref (iter);

	return;
}

static TnyMsgRemoveStrategy*
tny_merge_folder_get_msg_remove_strategy (TnyFolder *self)
{

	g_warning ("tny_merge_folder_get_msg_remove_strategy not implemented: "
		   "add it to the mother folder instead\n");

	return NULL;
}

static void
tny_merge_folder_set_msg_remove_strategy (TnyFolder *self, TnyMsgRemoveStrategy *st)
{

	g_warning ("tny_merge_folder_set_msg_remove_strategy not implemented: "
		   "add it to the mother folder instead\n");

	return;
}

static TnyMsgReceiveStrategy*
tny_merge_folder_get_msg_receive_strategy (TnyFolder *self)
{

	g_warning ("tny_merge_folder_get_msg_receive_strategy not implemented: "
		   "add it to the mother folder instead\n");

	return NULL;
}

static void
tny_merge_folder_set_msg_receive_strategy (TnyFolder *self, TnyMsgReceiveStrategy *st)
{

	g_warning ("tny_merge_folder_set_msg_receive_strategy not implemented: "
		   "add it to the mother folder instead\n");

	return;

}

static TnyMsg*
tny_merge_folder_get_msg (TnyFolder *self, TnyHeader *header, GError **err)
{
	TnyFolder *fol = tny_header_get_folder (header);
	TnyMsg *retval = tny_folder_get_msg (fol, header, err);
	g_object_unref (fol);

	return retval;
}

static TnyMsg*
tny_merge_folder_find_msg (TnyFolder *self, const gchar *url_string, GError **err)
{

	TnyMergeFolderPriv *priv = TNY_MERGE_FOLDER_GET_PRIVATE (self);
	TnyIterator *iter;
	TnyMsg *retval = NULL;

	iter = tny_list_create_iterator (priv->mothers);
	while (!tny_iterator_is_done (iter) && !retval)
	{
		TnyFolder *cur = TNY_FOLDER (tny_iterator_get_current (iter));
		retval = tny_folder_find_msg (cur, url_string, err);

		/* TODO: Handle err */

		g_object_unref (cur);
		tny_iterator_next (iter);
	}

	g_object_unref (iter);

	return retval;
}

static void
tny_merge_folder_get_msg_async (TnyFolder *self, TnyHeader *header, TnyGetMsgCallback callback, gpointer user_data)
{
	GError *err = NULL;
	TnyMsg *msg;

	/* TODO: do this is a thread */
	msg = tny_merge_folder_get_msg (self, header, &err);

	if (callback) /* TODO: this is not always on the gmainloop */
		callback (self, msg, &err, user_data);
}

static void
tny_merge_folder_get_headers (TnyFolder *self, TnyList *headers, gboolean refresh, GError **err)
{
	TnyMergeFolderPriv *priv = TNY_MERGE_FOLDER_GET_PRIVATE (self);
	TnyIterator *iter;

	iter = tny_list_create_iterator (priv->mothers);

	while (!tny_iterator_is_done (iter))
	{
		TnyFolder *cur = TNY_FOLDER (tny_iterator_get_current (iter));
		tny_folder_get_headers (cur, headers, refresh, err);

		/* TODO: Handle err */

		g_object_unref (cur);
		tny_iterator_next (iter);
	}

	g_object_unref (iter);
}

static const gchar*
tny_merge_folder_get_name (TnyFolder *self)
{
	TnyMergeFolderPriv *priv = TNY_MERGE_FOLDER_GET_PRIVATE (self);

	return priv->name;
}

static const gchar*
tny_merge_folder_get_id (TnyFolder *self)
{
	TnyMergeFolderPriv *priv = TNY_MERGE_FOLDER_GET_PRIVATE (self);

	if (!priv->id)
	{
		TnyIterator *iter;
		GString *ids = g_string_new ("");
		gboolean first = TRUE;

		iter = tny_list_create_iterator (priv->mothers);

		while (!tny_iterator_is_done (iter))
		{
			TnyFolder *cur = TNY_FOLDER (tny_iterator_get_current (iter));
			if (!first)
				g_string_append_c (ids, '&');
			g_string_append (ids, tny_folder_get_id (cur));
			g_object_unref (cur);
			first = FALSE;
			tny_iterator_next (iter);
		}

		priv->id = ids->str;
		g_string_free (ids, FALSE);
		g_object_unref (iter);
	}

	return priv->id;
}

static TnyAccount*
tny_merge_folder_get_account (TnyFolder *self)
{
	g_warning ("tny_merge_folder_get_account not implemented."
		   "Use the mother folders for this functionatily\n");

	return NULL;
}

static void
tny_merge_folder_set_name (TnyFolder *self, const gchar *name, GError **err)
{
	TnyMergeFolderPriv *priv = TNY_MERGE_FOLDER_GET_PRIVATE (self);

	if (priv->name)
		g_free (priv->name);
	priv->name = g_strdup (name);

	return;
}

static TnyFolderType
tny_merge_folder_get_folder_type (TnyFolder *self)
{
	return TNY_FOLDER_TYPE_MERGE;
}

static guint
tny_merge_folder_get_all_count (TnyFolder *self)
{
	TnyMergeFolderPriv *priv = TNY_MERGE_FOLDER_GET_PRIVATE (self);
	TnyIterator *iter;
	guint total = 0;

	iter = tny_list_create_iterator (priv->mothers);

	while (!tny_iterator_is_done (iter))
	{
		TnyFolder *cur = TNY_FOLDER (tny_iterator_get_current (iter));
		total += tny_folder_get_all_count (cur);
		g_object_unref (cur);
		tny_iterator_next (iter);
	}

	g_object_unref (iter);

	return total;
}

static guint
tny_merge_folder_get_unread_count (TnyFolder *self)
{
	TnyMergeFolderPriv *priv = TNY_MERGE_FOLDER_GET_PRIVATE (self);
	TnyIterator *iter;
	guint total = 0;

	iter = tny_list_create_iterator (priv->mothers);

	while (!tny_iterator_is_done (iter))
	{
		TnyFolder *cur = TNY_FOLDER (tny_iterator_get_current (iter));
		total += tny_folder_get_unread_count (cur);
		g_object_unref (cur);
		tny_iterator_next (iter);
	}

	g_object_unref (iter);

	return total;
}

static gboolean
tny_merge_folder_is_subscribed (TnyFolder *self)
{
	return TRUE;
}

static void
tny_merge_folder_refresh (TnyFolder *self, GError **err)
{
	TnyMergeFolderPriv *priv = TNY_MERGE_FOLDER_GET_PRIVATE (self);
	TnyIterator *iter;

	iter = tny_list_create_iterator (priv->mothers);
	while (!tny_iterator_is_done (iter))
	{
		TnyFolder *cur = TNY_FOLDER (tny_iterator_get_current (iter));
		tny_folder_refresh (cur, err);

		/* TODO: Handler err */

		g_object_unref (cur);
		tny_iterator_next (iter);
	}

	g_object_unref (iter);

	return;
}

static void
tny_merge_folder_refresh_async (TnyFolder *self, TnyRefreshFolderCallback callback, TnyRefreshFolderStatusCallback status_callback, gpointer user_data)
{
	GError *err = NULL;

	/* TODO: do this is a thread */
	tny_merge_folder_refresh (self, &err);

	if (callback) /* TODO: this is not always on the gmainloop */
		callback (self, FALSE, &err, user_data);
}

static void
tny_merge_folder_transfer_msgs (TnyFolder *self, TnyList *header_list, TnyFolder *folder_dst, gboolean delete_originals, GError **err)
{
	TnyIterator *iter = tny_list_create_iterator (header_list);

	while (!tny_iterator_is_done (iter))
	{
		TnyHeader *current = TNY_HEADER (tny_iterator_get_current (iter));
		TnyFolder *folder = tny_header_get_folder (current);

		TnyList *nlist = tny_simple_list_new ();
		tny_list_prepend (nlist, G_OBJECT (current));

		tny_folder_transfer_msgs (folder, nlist, folder_dst, delete_originals, err);
		/* TODO: handle err*/

		g_object_unref (nlist);
		g_object_unref (folder);
		g_object_unref (current);
		tny_iterator_next (iter);
	}

	g_object_unref (iter);

	return;
}

static void
tny_merge_folder_transfer_msgs_async (TnyFolder *self, TnyList *header_list, TnyFolder *folder_dst, gboolean delete_originals, TnyTransferMsgsCallback callback, gpointer user_data)
{
	GError *err = NULL;

	/* TODO: do this is a thread */
	tny_merge_folder_transfer_msgs (self, header_list, folder_dst, delete_originals, &err);

	if (callback) 	/* TODO call the callback in gmainloop */
		callback (self, &err, user_data);

}

static TnyFolder*
tny_merge_folder_copy (TnyFolder *self, TnyFolderStore *into, const gchar *new_name, gboolean del, GError **err)
{
	TnyMergeFolderPriv *priv = TNY_MERGE_FOLDER_GET_PRIVATE (self);
	TnyIterator *iter = tny_list_create_iterator (priv->mothers);
	TnyFolder *nfol = NULL;

	while (!tny_iterator_is_done (iter))
	{
		TnyFolder *folder = TNY_FOLDER (tny_iterator_get_current (iter));

		if (!nfol)
		{
			nfol = tny_folder_copy (folder, into, new_name, del, err);
			/* TODO: handle err */
		} else {
			TnyList *nlist = tny_simple_list_new ();

			tny_folder_get_headers (folder, nlist, FALSE, err);
			/* TODO: handle err */
			tny_folder_transfer_msgs (folder, nlist, nfol, del, err);
			/* TODO: handle err*/
			g_object_unref (nlist);
		}


		g_object_unref (folder);
		tny_iterator_next (iter);
	}

	g_object_unref (iter);

	return NULL;
}

static void
tny_merge_folder_poke_status (TnyFolder *self)
{
	TnyMergeFolderPriv *priv = TNY_MERGE_FOLDER_GET_PRIVATE (self);
	TnyIterator *iter;

	iter = tny_list_create_iterator (priv->mothers);
	while (!tny_iterator_is_done (iter))
	{
		TnyFolder *cur = TNY_FOLDER (tny_iterator_get_current (iter));
		tny_folder_poke_status (cur);
		g_object_unref (cur);
		tny_iterator_next (iter);
	}

	g_object_unref (iter);

	return;
}

static void
tny_merge_folder_add_observer (TnyFolder *self, TnyFolderObserver *observer)
{
	g_warning ("tny_merge_folder_add_observer not yet implemented!\n");
}

static void
tny_merge_folder_remove_observer (TnyFolder *self, TnyFolderObserver *observer)
{
	g_warning ("tny_merge_folder_remove_observer not yet implemented!\n");
}

static TnyFolderStore*
tny_merge_folder_get_folder_store (TnyFolder *self)
{
	g_warning ("tny_merge_folder_get_folder_store not reliable. "
		   "Please don't use this functionality\n");

	return TNY_FOLDER_STORE (g_object_ref (self));
}

static TnyFolderStats*
tny_merge_folder_get_stats (TnyFolder *self)
{
	TnyFolderStats *retval = tny_folder_stats_new (self);
	TnyMergeFolderPriv *priv = TNY_MERGE_FOLDER_GET_PRIVATE (self);
	TnyIterator *iter;
	gint total_size = 0;

	iter = tny_list_create_iterator (priv->mothers);
	while (!tny_iterator_is_done (iter))
	{
		TnyFolder *cur = TNY_FOLDER (tny_iterator_get_current (iter));
		TnyFolderStats *cstats = tny_folder_get_stats (cur);
		total_size += tny_folder_stats_get_local_size (cstats);
		g_object_unref (cstats);
		g_object_unref (cur);
		tny_iterator_next (iter);
	}

	g_object_unref (iter);

	/* TNY TODO: update unread, all_count and local_size here ! */

	tny_folder_stats_set_local_size (retval, total_size);

	return retval;
}

static gchar*
tny_merge_folder_get_url_string (TnyFolder *self)
{
	g_warning ("tny_merge_folder_get_url_string not reliable. "
		   "Please don't use this functionality\n");

	return "not://implemented";
}

static TnyFolderCaps
tny_merge_folder_get_caps (TnyFolder *self)
{
	/* All should be off: since @self isn't yet observing its mothers 
	  push e-mail ain't working yet, and self ain't writable either: the
	  app developer is expected to use the mother folders for writing 
	  operations! */

	return 0;
}

void 
tny_merge_folder_add_folder (TnyMergeFolder *self, TnyFolder *folder)
{
	TnyMergeFolderPriv *priv = TNY_MERGE_FOLDER_GET_PRIVATE (self);

	/* TODO: register @self as observer of @folder and proxy the
	 * events through to observers of @self. */

	tny_list_prepend (priv->mothers, G_OBJECT (folder));

	return;
}

TnyFolder*
tny_merge_folder_new (void)
{
	TnyMergeFolder *self = g_object_new (TNY_TYPE_MERGE_FOLDER, NULL);

	return TNY_FOLDER (self);

}

static void
tny_merge_folder_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyMergeFolder *self = (TnyMergeFolder *) instance;
	TnyMergeFolderPriv *priv = TNY_MERGE_FOLDER_GET_PRIVATE (self);

	priv->name = g_strdup ("Merged folder");
	priv->id = g_strdup  ("");
	priv->mothers = tny_simple_list_new ();

	return;
}

static void
tny_merge_folder_finalize (GObject *object)
{
	TnyMergeFolderPriv *priv = TNY_MERGE_FOLDER_GET_PRIVATE (object);

	g_object_unref (priv->mothers);

	if (priv->id)
		g_free (priv->id);

	if (priv->name)
		g_free (priv->name);

	parent_class->finalize (object);
}

static void
tny_folder_init (TnyFolderIface *klass)
{
	klass->remove_msg_func = tny_merge_folder_remove_msg;
	klass->add_msg_func = tny_merge_folder_add_msg;
	klass->sync_func = tny_merge_folder_sync;
	klass->get_msg_remove_strategy_func = tny_merge_folder_get_msg_remove_strategy;
	klass->set_msg_remove_strategy_func = tny_merge_folder_set_msg_remove_strategy;
	klass->get_msg_receive_strategy_func = tny_merge_folder_get_msg_receive_strategy;
	klass->set_msg_receive_strategy_func = tny_merge_folder_set_msg_receive_strategy;
	klass->get_msg_func = tny_merge_folder_get_msg;
	klass->find_msg_func = tny_merge_folder_find_msg;
	klass->get_msg_async_func = tny_merge_folder_get_msg_async;
	klass->get_headers_func = tny_merge_folder_get_headers;
	klass->get_name_func = tny_merge_folder_get_name;
	klass->get_id_func = tny_merge_folder_get_id;
	klass->get_account_func = tny_merge_folder_get_account;
	klass->set_name_func = tny_merge_folder_set_name;
	klass->get_folder_type_func = tny_merge_folder_get_folder_type;
	klass->get_all_count_func = tny_merge_folder_get_all_count;
	klass->get_unread_count_func = tny_merge_folder_get_unread_count;
	klass->is_subscribed_func = tny_merge_folder_is_subscribed;
	klass->refresh_async_func = tny_merge_folder_refresh_async;
	klass->refresh_func = tny_merge_folder_refresh;
	klass->transfer_msgs_func = tny_merge_folder_transfer_msgs;
	klass->transfer_msgs_async_func = tny_merge_folder_transfer_msgs_async;
	klass->copy_func = tny_merge_folder_copy;
	klass->poke_status_func = tny_merge_folder_poke_status;
	klass->add_observer_func = tny_merge_folder_add_observer;
	klass->remove_observer_func = tny_merge_folder_remove_observer;
	klass->get_folder_store_func = tny_merge_folder_get_folder_store;
	klass->get_stats_func = tny_merge_folder_get_stats;
	klass->get_url_string_func = tny_merge_folder_get_url_string;
	klass->get_caps_func = tny_merge_folder_get_caps;
}

static void
tny_merge_folder_class_init (TnyMergeFolderClass *klass)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;
	object_class->finalize = tny_merge_folder_finalize;

	g_type_class_add_private (object_class, sizeof (TnyMergeFolderPriv));

	return;
}


GType
tny_merge_folder_get_type (void)
{
	static GType type = 0;
	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
			sizeof (TnyMergeFolderClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			(GClassInitFunc) tny_merge_folder_class_init,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			sizeof (TnyMergeFolder),
			0,      /* n_preallocs */
			tny_merge_folder_instance_init,    /* instance_init */
			NULL
		};


		static const GInterfaceInfo tny_folder_info = 
		{
			(GInterfaceInitFunc) tny_folder_init, /* interface_init */
			NULL,         /* interface_finalize */
			NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyMergeFolder",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_FOLDER,
			&tny_folder_info);

	}


	return type;
}
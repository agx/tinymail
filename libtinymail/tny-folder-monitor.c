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

#include <string.h>
#include <config.h>
#include <glib.h>
#include <glib/gi18n-lib.h>

#include <tny-folder-monitor.h>
#include <tny-simple-list.h>
#include <tny-folder-change.h>

static GObjectClass *parent_class = NULL;

typedef struct _TnyFolderMonitorPriv TnyFolderMonitorPriv;

struct _TnyFolderMonitorPriv
{
	TnyList *lists;
	TnyFolder *folder;
	GMutex *lock;
};


#define TNY_FOLDER_MONITOR_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_FOLDER_MONITOR, TnyFolderMonitorPriv))

/**
 * tny_folder_monitor_add_list:
 * @self: a #TnyFolderChange instance
 * @list: a #TnyList instance
 *
 * Add @list to the registered lists that are interested in changes. @list will
 * remain referenced until it's unregisterd using tny_folder_monitor_remove_list
 * or until @self is finalized.
 *
 **/
void 
tny_folder_monitor_add_list (TnyFolderMonitor *self, TnyList *list)
{
	TNY_FOLDER_MONITOR_GET_CLASS (self)->add_list_func (self, list);
	return;
}

static void 
tny_folder_monitor_add_list_default (TnyFolderMonitor *self, TnyList *list)
{
	TnyFolderMonitorPriv *priv = TNY_FOLDER_MONITOR_GET_PRIVATE (self);

	g_assert (TNY_IS_LIST (list));

	g_mutex_lock (priv->lock);
	tny_list_prepend (priv->lists, G_OBJECT (list));
	g_mutex_unlock (priv->lock);

	return;
}

/**
 * tny_folder_monitor_remove_list:
 * @self: a #TnyFolderChange instance
 * @list: a #TnyList instance
 *
 * Remove @list from the lists that are interested in changes. This will remove
 * the reference that got added when you ussed the tny_folder_monitor_add_list.
 *
 **/
void 
tny_folder_monitor_remove_list (TnyFolderMonitor *self, TnyList *list)
{
	TNY_FOLDER_MONITOR_GET_CLASS (self)->remove_list_func (self, list);
	return;
}

static void 
tny_folder_monitor_remove_list_default (TnyFolderMonitor *self, TnyList *list)
{
	TnyFolderMonitorPriv *priv = TNY_FOLDER_MONITOR_GET_PRIVATE (self);

	g_assert (TNY_IS_LIST (list));

	g_mutex_lock (priv->lock);
	tny_list_remove (priv->lists, G_OBJECT (list));
	g_mutex_unlock (priv->lock);

	return;
}

/**
 * tny_folder_monitor_poke_status
 * @self: a #TnyFolderMonitor instance
 *
 * Trigger the poke method on the folder instance being monitored.
 * Also take a look at tny_folder_poke_status in #TnyFolder.
 *
 **/
void 
tny_folder_monitor_poke_status (TnyFolderMonitor *self)
{
	TNY_FOLDER_MONITOR_GET_CLASS (self)->poke_status_func (self);
	return;
}

static void 
tny_folder_monitor_poke_status_default (TnyFolderMonitor *self)
{
	TnyFolderMonitorPriv *priv = TNY_FOLDER_MONITOR_GET_PRIVATE (self);

	if (priv->folder && TNY_IS_FOLDER (priv->folder))
		tny_folder_poke_status (priv->folder);

	return;
}


static void
tny_folder_monitor_update (TnyFolderObserver *self, TnyFolderChange *change)
{
	TNY_FOLDER_MONITOR_GET_CLASS (self)->update_func (self, change);
	return;
}

static void
foreach_list_add_header (TnyFolderMonitorPriv *priv, TnyHeader *header)
{
	TnyIterator *iter;
	/* const gchar *uid = tny_header_get_uid (header); */

	iter = tny_list_create_iterator (priv->lists);
	while (!tny_iterator_is_done (iter))
	{
		/*gboolean found = FALSE;*/
		TnyList *list = TNY_LIST (tny_iterator_get_current (iter));

		/*TnyIterator *liter = tny_list_create_iterator (list);
		while (!tny_iterator_is_done (liter))
		{
			TnyHeader *hdr = (TnyHeader *) tny_iterator_get_current (liter);
			if (!strcmp (uid, tny_header_get_uid (hdr)))
				found = TRUE;
			g_object_unref (hdr);
			tny_iterator_next (liter);
			if (found)
				break;
		}
		g_object_unref (liter);

		if (!found)*/
			tny_list_prepend (list, G_OBJECT (header));

		g_object_unref (G_OBJECT (list));
		tny_iterator_next (iter);
	}
	g_object_unref (G_OBJECT (iter));
}

static gboolean
is_in_my_array (GPtrArray *array, const gchar *uid)
{
	gint i=0;
	for (i=0; i<array->len; i++)
		if (!strcmp (array->pdata[i], uid))
			return TRUE;
	return FALSE;
}

static void 
remove_headers_from_list (TnyList *list, GPtrArray *array)
{
	TnyIterator *iter;
	TnyHeader *header = NULL;
	GList *to_remove = NULL;

	iter = tny_list_create_iterator (list);
	while (G_LIKELY (!tny_iterator_is_done (iter)))
	{
		const gchar *id;
		header = TNY_HEADER (tny_iterator_get_current (iter));
		id = tny_header_get_uid (header);

		if (is_in_my_array (array, id)) {
			to_remove = g_list_prepend (to_remove, 
				g_object_ref (header));
		}

		g_object_unref (header);
		tny_iterator_next (iter);
	}
	g_object_unref (iter);

	while (to_remove)
	{
		GObject *mheader = to_remove->data;
		g_object_unref (mheader); /* from the loop */
		tny_list_remove (list, mheader);
		to_remove = g_list_next (to_remove);
	}
	g_list_free (to_remove);

	return;
}

static void
foreach_list_remove_headers (TnyFolderMonitorPriv *priv, GPtrArray *array)
{
	TnyIterator *iter;

	iter = tny_list_create_iterator (priv->lists);
	while (!tny_iterator_is_done (iter))
	{
		TnyList *list = TNY_LIST (tny_iterator_get_current (iter));
		remove_headers_from_list (list, array);
		g_object_unref (list);
		tny_iterator_next (iter);
	}
	g_object_unref (iter);
}

static int
cmpstringp(const void *p1, const void *p2)
{
	return strcmp(* (char * const *) p1, * (char * const *) p2);
}


static void
tny_folder_monitor_update_default (TnyFolderObserver *self, TnyFolderChange *change)
{
	TnyFolderMonitorPriv *priv = TNY_FOLDER_MONITOR_GET_PRIVATE (self);
	TnyIterator *iter;
	TnyList *list;
	TnyFolderChangeChanged changed;

	g_mutex_lock (priv->lock);

	changed = tny_folder_change_get_changed (change);

	if (changed & TNY_FOLDER_CHANGE_CHANGED_ADDED_HEADERS)
	{
		/* The added headers */
		list = tny_simple_list_new ();
		tny_folder_change_get_added_headers (change, list);
		iter = tny_list_create_iterator (list);
		while (!tny_iterator_is_done (iter))
		{
			TnyHeader *header = TNY_HEADER (tny_iterator_get_current (iter));
			foreach_list_add_header (priv, header);
			g_object_unref (header);
			tny_iterator_next (iter);
		}
		g_object_unref (iter);
		g_object_unref (list);
	}

	if (changed & TNY_FOLDER_CHANGE_CHANGED_EXPUNGED_HEADERS)
	{
		GPtrArray *array = NULL;
		/* The removed headers */
		list = tny_simple_list_new ();
		tny_folder_change_get_expunged_headers (change, list);
		array = g_ptr_array_sized_new (tny_list_get_length (list));

		iter = tny_list_create_iterator (list);
		while (!tny_iterator_is_done (iter))
		{
			const gchar *uid;
			TnyHeader *header = TNY_HEADER (tny_iterator_get_current (iter));
			uid = tny_header_get_uid (header);
			if (uid) {
				gchar *tuid = g_strdup (uid);
				g_ptr_array_add (array, tuid);
			}
			g_object_unref (header);
			tny_iterator_next (iter);
		}
		g_object_unref (iter);
		g_object_unref (list);

		g_ptr_array_sort (array, (GCompareFunc) cmpstringp);
		foreach_list_remove_headers (priv, array);
		g_ptr_array_foreach (array, (GFunc) g_free, NULL);
		g_ptr_array_free (array, TRUE);
	}


	g_mutex_unlock (priv->lock);

	return;
}

/**
 * tny_folder_monitor_stop:
 * @self: a #TnyFolderMonitor instance
 *
 * Stop monitoring the folder. At some point in time you must perform method 
 * after you used tny_folder_monitor_start (for example before unreferencing 
 * @self). 
 *
 * The reason for that is that unless you stop @self, it would still have a
 * reference being held by the #TnyFolder instance being monitored. By stopping
 * this monitor, you explicitly ask that folder instance to unregister @self as
 * an observer. And by that loosing that reference too.
 *
 **/
void 
tny_folder_monitor_stop (TnyFolderMonitor *self)
{
	TNY_FOLDER_MONITOR_GET_CLASS (self)->stop_func (self);
	return;
}

static void 
tny_folder_monitor_stop_default (TnyFolderMonitor *self)
{
	TnyFolderMonitorPriv *priv = TNY_FOLDER_MONITOR_GET_PRIVATE (self);
	tny_folder_remove_observer (priv->folder, TNY_FOLDER_OBSERVER (self));
	return;
}

/**
 * tny_folder_monitor_start:
 * @self: a #TnyFolderMonitor instance
 *
 * Start monitoring the folder. The starting of a monitor implies that @self
 * will become an observer of the folder instance. This adds a reference to @self
 * that at some point in time must be removed using tny_folder_monitor_stop.
 *
 * This means that if you start the monitor, you must also at some point stop it.
 * 
 * Keep this in mind: monitoring means that registered lists are kept alive. A
 * list that is alive means that it's consuming memory!
 **/
void 
tny_folder_monitor_start (TnyFolderMonitor *self)
{
	TNY_FOLDER_MONITOR_GET_CLASS (self)->start_func (self);
	return;
}

static void 
tny_folder_monitor_start_default (TnyFolderMonitor *self)
{
	TnyFolderMonitorPriv *priv = TNY_FOLDER_MONITOR_GET_PRIVATE (self);
	tny_folder_add_observer (priv->folder, TNY_FOLDER_OBSERVER (self));
	return;
}

/**
 * tny_folder_monitor_new:
 * @folder: a #TnyFolder instance
 *
 * Creates a folder monitor for @folder
 *
 * Return value: a new #TnyFolderMonitor instance
 **/
TnyFolderObserver*
tny_folder_monitor_new (TnyFolder *folder)
{
	TnyFolderMonitor *self = g_object_new (TNY_TYPE_FOLDER_MONITOR, NULL);
	TnyFolderMonitorPriv *priv = TNY_FOLDER_MONITOR_GET_PRIVATE (self);

	g_assert (TNY_IS_FOLDER (folder));

	priv->folder = folder; /* not referenced to avoid cross references */

	return TNY_FOLDER_OBSERVER (self);
}


static void
tny_folder_monitor_finalize (GObject *object)
{
	TnyFolderMonitorPriv *priv = TNY_FOLDER_MONITOR_GET_PRIVATE (object);

	g_mutex_lock (priv->lock);
	g_object_unref (G_OBJECT (priv->lists));
	g_mutex_unlock (priv->lock);

	g_mutex_free (priv->lock);

	parent_class->finalize (object);
}

static void
tny_folder_monitor_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyFolderMonitor *self = (TnyFolderMonitor *)instance;
	TnyFolderMonitorPriv *priv = TNY_FOLDER_MONITOR_GET_PRIVATE (self);

	priv->lock = g_mutex_new ();

	g_mutex_lock (priv->lock);

	priv->folder = NULL;
	priv->lists = tny_simple_list_new ();

	g_mutex_unlock (priv->lock);

	return;
}

static void
tny_folder_observer_init (TnyFolderObserverIface *klass)
{
	klass->update_func = tny_folder_monitor_update;
}

static void
tny_folder_monitor_class_init (TnyFolderMonitorClass *klass)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;

	klass->update_func = tny_folder_monitor_update_default;
	klass->poke_status_func = tny_folder_monitor_poke_status_default;
	klass->add_list_func = tny_folder_monitor_add_list_default;
	klass->remove_list_func = tny_folder_monitor_remove_list_default;
	klass->stop_func = tny_folder_monitor_stop_default;
	klass->start_func = tny_folder_monitor_start_default;

	object_class->finalize = tny_folder_monitor_finalize;
	g_type_class_add_private (object_class, sizeof (TnyFolderMonitorPriv));
}

GType
tny_folder_monitor_get_type (void)
{
	static GType type = 0;
	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
			sizeof (TnyFolderMonitorClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			(GClassInitFunc) tny_folder_monitor_class_init,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			sizeof (TnyFolderMonitor),
			0,      /* n_preallocs */
			tny_folder_monitor_instance_init,    /* instance_init */
			NULL
		};


		static const GInterfaceInfo tny_folder_observer_info = 
		{
			(GInterfaceInitFunc) tny_folder_observer_init, /* interface_init */
			NULL,         /* interface_finalize */
			NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyFolderMonitor",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_FOLDER_OBSERVER,
			&tny_folder_observer_info);

	}
	return type;
}

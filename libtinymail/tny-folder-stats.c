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

#include <tny-folder-stats.h>
#include <tny-simple-list.h>

static GObjectClass *parent_class = NULL;

typedef struct _TnyFolderStatsPriv TnyFolderStatsPriv;

struct _TnyFolderStatsPriv
{
	TnyFolder *folder;
	gsize local_size;
};

#define TNY_FOLDER_STATS_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_FOLDER_STATS, TnyFolderStatsPriv))



/**
 * tny_folder_stats_get_unread_count:
 * @self: a #TnyFolderStats instance
 *
 * Get the amount of unread messages in @self
 *
 * Return value: the amount of unread messages
 **/
guint 
tny_folder_stats_get_unread_count (TnyFolderStats *self)
{
	TnyFolderStatsPriv *priv = TNY_FOLDER_STATS_GET_PRIVATE (self);

	return tny_folder_get_unread_count (priv->folder);
}



/**
 * tny_folder_stats_get_all_count:
 * @self: a #TnyFolderStats instance
 *
 * Get the amount of messages in @self
 *
 * Return value: the amount of messages
 **/
guint 
tny_folder_stats_get_all_count (TnyFolderStats *self)
{
	TnyFolderStatsPriv *priv = TNY_FOLDER_STATS_GET_PRIVATE (self);

	return tny_folder_get_all_count (priv->folder);
}

/**
 * tny_folder_stats_set_local_size:
 * @self: a #TnyFolderStats instance
 * @local_size: the local size of the folder
 *
 * Set the local disk space that @self is consuming (should probably only be
 * used by the #TnyFolder implementation internally: it'll define the value
 * that tny_folder_stats_get_local_size will return).
 **/
void 
tny_folder_stats_set_local_size (TnyFolderStats *self, gsize local_size)
{
	TnyFolderStatsPriv *priv = TNY_FOLDER_STATS_GET_PRIVATE (self);
	priv->local_size = local_size;
	return;
}

/**
 * tny_folder_stats_get_local_size:
 * @self: a #TnyFolderStats instance
 *
 * Get the local disk space that @self is consuming
 *
 * Return value: The local size
 **/
gsize 
tny_folder_stats_get_local_size (TnyFolderStats *self)
{
	TnyFolderStatsPriv *priv = TNY_FOLDER_STATS_GET_PRIVATE (self);
	return priv->local_size;
}


/**
 * tny_folder_stats_new:
 * @folder: a #TnyFolder instance
 *
 * Creates a stats object for for @folder
 *
 * Return value: a new #TnyFolderStats instance
 **/
TnyFolderStats*
tny_folder_stats_new (TnyFolder *folder)
{
	TnyFolderStats *self = g_object_new (TNY_TYPE_FOLDER_STATS, NULL);
	TnyFolderStatsPriv *priv = TNY_FOLDER_STATS_GET_PRIVATE (self);

	priv->folder = TNY_FOLDER (g_object_ref (G_OBJECT (folder)));

	return self;
}

/**
 * tny_folder_stats_get_folder:
 * @self: a #TnyFolderStats instance
 *
 * Get the folder of @self. The return value of this method must be unreferenced 
 * after use
 *
 * Return value: the #TnyFolder instance related to this statsset
 **/
TnyFolder* 
tny_folder_stats_get_folder (TnyFolderStats *self)
{
	TnyFolderStatsPriv *priv = TNY_FOLDER_STATS_GET_PRIVATE (self);
	TnyFolder *retval = NULL;

	if (priv->folder)
		retval = TNY_FOLDER (g_object_ref (G_OBJECT (priv->folder)));

	return retval;
}


static void
tny_folder_stats_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyFolderStats *self = (TnyFolderStats *)instance;
	TnyFolderStatsPriv *priv = TNY_FOLDER_STATS_GET_PRIVATE (self);

	priv->folder = NULL;

	return;
}

static void
tny_folder_stats_finalize (GObject *object)
{
	TnyFolderStats *self = (TnyFolderStats *)object;	
	TnyFolderStatsPriv *priv = TNY_FOLDER_STATS_GET_PRIVATE (self);

	if (priv->folder)
		g_object_unref (G_OBJECT (priv->folder));
	priv->folder = NULL;

	(*parent_class->finalize) (object);

	return;
}


static void 
tny_folder_stats_class_init (TnyFolderStatsClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;
	object_class->finalize = tny_folder_stats_finalize;
	g_type_class_add_private (object_class, sizeof (TnyFolderStatsPriv));

	return;
}

GType 
tny_folder_stats_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyFolderStatsClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_folder_stats_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyFolderStats),
		  0,      /* n_preallocs */
		  tny_folder_stats_instance_init,   /* instance_init */
		  NULL
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyFolderStats",
			&info, 0);
	}

	return type;
}


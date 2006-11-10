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

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <string.h>

#include <tny-camel-msg-remove-strategy.h>

#include <tny-camel-folder.h>
#include "tny-camel-folder-priv.h"

static GObjectClass *parent_class = NULL;


static void
tny_camel_msg_remove_strategy_remove (TnyMsgRemoveStrategy *self, TnyFolder *folder, TnyHeader *header)
{
	TnyCamelFolderPriv *priv;
	const gchar *id;

	g_assert (TNY_IS_CAMEL_FOLDER (folder));
	g_assert (TNY_IS_HEADER (header));

	priv = TNY_CAMEL_FOLDER_GET_PRIVATE (folder);
	id = tny_header_get_uid (TNY_HEADER (header));
	camel_folder_delete_message (priv->folder, id);

	return;
}


/**
 * tny_camel_msg_remove_strategy_new:
 *
 *
 * Return value: a new #TnyMimePartSaveStrategy instance implemented for Gtk+
 **/
TnyMsgRemoveStrategy*
tny_camel_msg_remove_strategy_new (void)
{
	TnyCamelMsgRemoveStrategy *self = g_object_new (TNY_TYPE_CAMEL_MSG_REMOVE_STRATEGY, NULL);

	return TNY_MSG_REMOVE_STRATEGY (self);
}

static void
tny_camel_msg_remove_strategy_instance_init (GTypeInstance *instance, gpointer g_class)
{
	return;
}

static void
tny_camel_msg_remove_strategy_finalize (GObject *object)
{
	(*parent_class->finalize) (object);

	return;
}

static void
tny_camel_msg_remove_strategy_init (gpointer g, gpointer iface_data)
{
	TnyMsgRemoveStrategyIface *klass = (TnyMsgRemoveStrategyIface *)g;

	klass->remove_func = tny_camel_msg_remove_strategy_remove;

	return;
}

static void 
tny_camel_msg_remove_strategy_class_init (TnyCamelMsgRemoveStrategyClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_camel_msg_remove_strategy_finalize;

	return;
}

GType 
tny_camel_msg_remove_strategy_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyCamelMsgRemoveStrategyClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_camel_msg_remove_strategy_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyCamelMsgRemoveStrategy),
		  0,      /* n_preallocs */
		  tny_camel_msg_remove_strategy_instance_init,    /* instance_init */
		  NULL
		};

		static const GInterfaceInfo tny_camel_msg_remove_strategy_info = 
		{
		  (GInterfaceInitFunc) tny_camel_msg_remove_strategy_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyCamelMsgRemoveStrategy",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_MSG_REMOVE_STRATEGY, 
			&tny_camel_msg_remove_strategy_info);
	}

	return type;
}

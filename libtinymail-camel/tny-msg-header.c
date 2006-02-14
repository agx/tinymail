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


/* TnyMsgHeader is a proxy for real subject CamelMessageInfo */

#include <glib.h>

#include <tny-msg-header-iface.h>
#include <tny-msg-header.h>

static GObjectClass *parent_class = NULL;

#include "tny-msg-folder-priv.h"
#include "tny-msg-header-priv.h"

#include <camel/camel-folder.h>

#define TNY_MSG_HEADER_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_MSG_HEADER_TYPE, TnyMsgHeaderPriv))

/* TODO: Remove debugging purpose */
static gint allocations=0;

static void
tny_msg_header_set_camel_message_info_priv  (TnyMsgHeaderPriv *priv, CamelMessageInfo *camel_message_info)
{

	/* camel_message_info_ref (camel_message_info); */
	
	priv->message_info = camel_message_info;

	return;
}

static void
unload_msg_header (TnyMsgHeaderPriv *priv)
{
	if (priv->message_info) 
	{
		camel_message_info_free (priv->message_info);
		priv->message_info = NULL;

		/* TODO: Remove debugging purpose */
		allocations--;
		g_print ("Deallocation. Current = %d\n", allocations);
	}

	priv->message_info = NULL;

	return;
}

static void
load_msg_header (TnyMsgHeaderPriv *priv)
{
	if (!priv->message_info && priv->folder && priv->id)
	{
		CamelFolder *folder = _tny_msg_folder_get_camel_folder (priv->folder);
		CamelMessageInfo *msginfo = camel_folder_get_message_info (folder, priv->id);

		tny_msg_header_set_camel_message_info_priv (priv, msginfo);

		/* TODO: Remove debugging purpose */
		allocations++;
		g_print ("Allocation. Current = %d\n", allocations);
	}

	return;
}

void /* public version of this method */
_tny_msg_header_set_camel_message_info (TnyMsgHeader *self, CamelMessageInfo *camel_message_info)
{
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (self);

	tny_msg_header_set_camel_message_info_priv (priv, camel_message_info);

	return;
}

const TnyMsgFolderIface* 
tny_msg_header_get_folder (TnyMsgHeaderIface *self)
{
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (TNY_MSG_HEADER (self));
	
	return priv->folder;
}


void
tny_msg_header_set_folder (TnyMsgHeaderIface *self, const TnyMsgFolderIface* folder)
{
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (TNY_MSG_HEADER (self));

	/* if (priv->folder)
		g_object_unref (G_OBJECT (priv->folder));

	   g_object_ref (G_OBJECT (folder) ) */

	priv->folder = (TnyMsgFolderIface*)folder;

	return;
}

/* TODO: Add these
camel_message_info_mlist     
camel_message_info_flags     
camel_message_info_size      
camel_message_info_message_id
*/

static const gchar*
tny_msg_header_get_cc (TnyMsgHeaderIface *self)
{
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (TNY_MSG_HEADER (self));

	load_msg_header (priv);

	return camel_message_info_cc (priv->message_info);
}

static const time_t
tny_msg_header_get_date_received (TnyMsgHeaderIface *self)
{
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (TNY_MSG_HEADER (self));

	load_msg_header (priv);

	return camel_message_info_date_received (priv->message_info);
}

static const time_t
tny_msg_header_get_date_sent (TnyMsgHeaderIface *self)
{
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (TNY_MSG_HEADER (self));

	load_msg_header (priv);

	return camel_message_info_date_sent (priv->message_info);
}
	
static const gchar*
tny_msg_header_get_from (TnyMsgHeaderIface *self)
{
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (TNY_MSG_HEADER (self));

	load_msg_header (priv);

	return camel_message_info_from (priv->message_info);
}

static const gchar*
tny_msg_header_get_subject (TnyMsgHeaderIface *self)
{
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (TNY_MSG_HEADER (self));

	load_msg_header (priv);

	return camel_message_info_subject (priv->message_info);
}


static const gchar*
tny_msg_header_get_to (TnyMsgHeaderIface *self)
{
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (TNY_MSG_HEADER (self));

	load_msg_header (priv);

	return camel_message_info_to (priv->message_info);
}

static const gchar*
tny_msg_header_get_message_id (TnyMsgHeaderIface *self)
{
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (TNY_MSG_HEADER (self));

	load_msg_header (priv);

	return (const gchar*)camel_message_info_message_id (priv->message_info);
}


static const gchar*
tny_msg_header_get_id (TnyMsgHeaderIface *self)
{
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (TNY_MSG_HEADER (self));

	load_msg_header (priv);

	return camel_message_info_uid (priv->message_info);
}

static void
tny_msg_header_set_id (TnyMsgHeaderIface *self, const gchar *id)
{
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (TNY_MSG_HEADER (self));

	unload_msg_header (priv);

	/* Speedup trick, also check tny-msg-folder.c */

	/* if (priv->id)
		g_free (priv->id);

	priv->id = g_strdup (id); */

	/* Yes I know what I'm doing, also check tny-msg-folder.c */
	priv->id = (gchar*)id;

	return;
}


static const gboolean 
tny_msg_header_has_cache (TnyMsgHeaderIface *self)
{
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (TNY_MSG_HEADER (self));

	return (priv->message_info != NULL);
}

static void
tny_msg_header_uncache (TnyMsgHeaderIface *self)
{
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (TNY_MSG_HEADER (self));

	unload_msg_header (priv);

	return;
}

static void
tny_msg_header_finalize (GObject *object)
{
	TnyMsgHeader *self = (TnyMsgHeader*) object;
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (self);

	/* if (priv->folder)
		g_object_unref (G_OBJECT (priv->folder)); */

	unload_msg_header (priv);

	/* Indeed, check the speedup trick above */

	/* if (priv->id)
		g_free (priv->id); */

	(*parent_class->finalize) (object);

	return;
}

TnyMsgHeader*
tny_msg_header_new (void)
{
	TnyMsgHeader *self = g_object_new (TNY_MSG_HEADER_TYPE, NULL);
	
	return self;
}


static void
tny_msg_header_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyMsgHeaderIfaceClass *klass = (TnyMsgHeaderIfaceClass *)g_iface;

	klass->get_from_func = tny_msg_header_get_from;
	klass->get_message_id_func = tny_msg_header_get_message_id;
	klass->get_to_func = tny_msg_header_get_to;
	klass->get_subject_func = tny_msg_header_get_subject;
	klass->get_date_received_func = tny_msg_header_get_date_received;
	klass->get_date_sent_func = tny_msg_header_get_date_sent;
	klass->get_cc_func = tny_msg_header_get_cc;

	klass->get_id_func = tny_msg_header_get_id;	
	klass->set_id_func = tny_msg_header_set_id;
	
	klass->set_folder_func = tny_msg_header_set_folder;
	klass->get_folder_func = tny_msg_header_get_folder;

	klass->has_cache_func = tny_msg_header_has_cache;
	klass->uncache_func = tny_msg_header_uncache;

	return;
}


static void 
tny_msg_header_class_init (TnyMsgHeaderClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_msg_header_finalize;

	g_type_class_add_private (object_class, sizeof (TnyMsgHeaderPriv));

	return;
}

static void
tny_msg_header_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyMsgHeader *self = (TnyMsgHeader *)instance;
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (self);

	priv->folder = NULL;
	priv->message_info = NULL;

	return;
}

GType 
tny_msg_header_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgHeaderClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_msg_header_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMsgHeader),
		  0,      /* n_preallocs */
		  tny_msg_header_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_msg_header_iface_info = 
		{
		  (GInterfaceInitFunc) tny_msg_header_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyMsgHeader",
			&info, 0);

		g_type_add_interface_static (type, TNY_MSG_HEADER_IFACE_TYPE, 
			&tny_msg_header_iface_info);
	}

	return type;
}

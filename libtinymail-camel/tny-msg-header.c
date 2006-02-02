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

#include <glib.h>

#include <tny-msg-header-iface.h>
#include <tny-msg-header.h>
#include <tny-msg-folder-iface.h>

static GObjectClass *parent_class = NULL;

#include "tny-msg-header-priv.h"

#define TNY_MSG_HEADER_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_MSG_HEADER_TYPE, TnyMsgHeaderPriv))


void
_tny_msg_header_set_camel_message_info (TnyMsgHeader *self, CamelMessageInfo *camel_message_info)
{
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (self);

	if (priv->message_info)
		camel_object_unref (priv->message_info);

	camel_object_ref (camel_message_info);

	priv->message_info = camel_message_info;

	return;
}

const TnyMsgFolderIface* 
tny_msg_header_get_folder (TnyMsgHeaderIface *self)
{
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (TNY_MSG_HEADER (self));
	
	return priv->folder;
}


void
tny_msg_header_set_folder (TnyMsgHeaderIface *self, const TnyMsgFolderIface *folder)
{
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (TNY_MSG_HEADER (self));
	
	if (priv->folder)
		g_object_unref (G_OBJECT (priv->folder));

	g_object_ref (G_OBJECT (folder));

	priv->folder = (TnyMsgFolderIface*)folder;

	return;
}
	
static const gchar*
tny_msg_header_get_from (TnyMsgHeaderIface *self)
{
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (TNY_MSG_HEADER (self));
	
	return camel_message_info_from (priv->message_info);
}

static const gchar*
tny_msg_header_get_subject (TnyMsgHeaderIface *self)
{
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (TNY_MSG_HEADER (self));

	return camel_message_info_subject (priv->message_info);
}


static const gchar*
tny_msg_header_get_to (TnyMsgHeaderIface *self)
{
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (TNY_MSG_HEADER (self));

	return camel_message_info_to (priv->message_info);
}

static const gchar*
tny_msg_header_get_id (TnyMsgHeaderIface *self)
{
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (TNY_MSG_HEADER (self));

	return camel_message_info_uid (priv->message_info);
}

static void
tny_msg_header_set_id (TnyMsgHeaderIface *self, const gchar *id)
{
	return;
}

static void
tny_msg_header_set_from (TnyMsgHeaderIface *self, const gchar *from)
{
	return;
}

static void
tny_msg_header_set_to (TnyMsgHeaderIface *self, const gchar *to)
{
	return;
}

static void
tny_msg_header_set_subject (TnyMsgHeaderIface *self, const gchar *subject)
{
	return;
}

static void
tny_msg_header_finalize (GObject *object)
{
	TnyMsgHeader *self = (TnyMsgHeader*) object;
	TnyMsgHeaderPriv *priv = TNY_MSG_HEADER_GET_PRIVATE (self);

	if (priv->folder)
		g_object_unref (G_OBJECT (priv->folder));

	if (priv->message_info)
		camel_object_unref (priv->message_info);

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
	klass->get_id_func = tny_msg_header_get_id;
	klass->get_to_func = tny_msg_header_get_to;
	klass->get_subject_func = tny_msg_header_get_subject;

	klass->set_from_func = tny_msg_header_set_from;
	klass->set_id_func = tny_msg_header_set_id;
	klass->set_to_func = tny_msg_header_set_to;
	klass->set_subject_func = tny_msg_header_set_subject;
	
	klass->has_cache_func = NULL;
	klass->uncache_func = NULL;

	return;
}


static void 
tny_msg_header_class_init (TnyMsgHeaderIfaceClass *class)
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

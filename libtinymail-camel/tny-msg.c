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

#include <tny-msg.h>
#include <tny-msg-mime-part-iface.h>
#include <tny-stream-iface.h>
#include <tny-msg-header-iface.h>
#include <tny-msg-mime-part.h>
#include <tny-stream-camel.h>
#include <tny-msg-header.h>

#include <camel/camel-stream-buffer.h>

static GObjectClass *parent_class = NULL;

#include "tny-msg-priv.h"

#define TNY_MSG_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_MSG_TYPE, TnyMsgPriv))

typedef gboolean (*CamelPartFunc)(CamelMimeMessage *, CamelMimePart *, void *data);

static gboolean
message_foreach_part_rec (CamelMimeMessage *msg, CamelMimePart *part, CamelPartFunc callback, void *data)
{
        CamelDataWrapper *containee;
        int parts, i;
        int go = TRUE;

        if (callback (msg, part, data) == FALSE)
                return FALSE;

        containee = camel_medium_get_content_object (CAMEL_MEDIUM (part));

        if (containee == NULL)
                return go;

        /* using the object types is more accurate than using the mime/types */
        if (CAMEL_IS_MULTIPART (containee)) {
                parts = camel_multipart_get_number (CAMEL_MULTIPART (containee));
                for (i = 0; go && i < parts; i++) {
                        CamelMimePart *part = camel_multipart_get_part (CAMEL_MULTIPART (containee), i);

                        go = message_foreach_part_rec (msg, part, callback, data);
                }
        } else if (CAMEL_IS_MIME_MESSAGE (containee)) {
                go = message_foreach_part_rec (msg, (CamelMimePart *)containee, callback, data);
        }

        return go;
}


static gboolean
received_a_part (CamelMimeMessage *message, CamelMimePart *part, void *data)
{
	TnyMsgPriv *priv = data;

	TnyMsgMimePartIface *tpart = TNY_MSG_MIME_PART_IFACE 
			(tny_msg_mime_part_new (part));


	TnyMsgMimePart *p = tny_msg_mime_part_new (part);

	/* Uncertain (_new is a ref by itself, right?) */
	/* g_object_ref (G_OBJECT (tpart)); */

	priv->parts = g_list_append (priv->parts, tpart);

	return TRUE;
}


void
_tny_msg_set_camel_mime_message (TnyMsg *self, CamelMimeMessage *message)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (self);
	CamelDataWrapper *wrapper;
	CamelMimePart *part;

	message_foreach_part_rec (message, (CamelMimePart *)message, received_a_part, priv);

	return;
}

const TnyMsgFolderIface* 
tny_msg_get_folder (TnyMsgIface *self)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));
	
	return (const TnyMsgFolderIface*)priv->folder;
}


void
tny_msg_set_folder (TnyMsgIface *self, const TnyMsgFolderIface* folder)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));

	priv->folder = (TnyMsgFolderIface*)folder;

	return;
}

static const GList*
tny_msg_get_parts (TnyMsgIface *self)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));

	return priv->parts;
}

static const TnyMsgHeaderIface*
tny_msg_get_header (TnyMsgIface *self)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));

	return priv->header;
}


static gint
tny_msg_add_part (TnyMsgIface *self, TnyMsgMimePartIface *part)
{
	/* TODO */
	return -1;
}

static void 
tny_msg_del_part (TnyMsgIface *self, gint id)
{
	/* TODO */
	return;
}


static void
tny_msg_set_header (TnyMsgIface *self, TnyMsgHeaderIface *header)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));

	if (priv->header)
		g_object_unref (G_OBJECT (priv->header));

	g_object_ref (G_OBJECT (header));

	priv->header = header;

	return;
}

static void 
destroy_part (gpointer data, gpointer user_data)
{
	if (data)
		g_object_unref (G_OBJECT (data));

	return;
}

static void
tny_msg_finalize (GObject *object)
{
	TnyMsg *self = (TnyMsg*) object;
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));

	if (priv->header)
		g_object_unref (G_OBJECT (priv->header));

	priv->header = NULL;

	if (priv->parts) 
	{
		g_list_foreach (priv->parts, 
			destroy_part, NULL);

		g_list_free (priv->parts);
	}

	priv->parts = NULL;

	return;
}


TnyMsg*
tny_msg_new (void)
{
	TnyMsg *self = g_object_new (TNY_MSG_TYPE, NULL);
	
	return self;
}


static void
tny_msg_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyMsgIfaceClass *klass = (TnyMsgIfaceClass *)g_iface;

	klass->get_parts_func = tny_msg_get_parts;
	klass->get_header_func = tny_msg_get_header;
	klass->set_header_func = tny_msg_set_header;

	klass->add_part_func = tny_msg_add_part;
	klass->del_part_func = tny_msg_del_part;

	klass->set_folder_func = tny_msg_set_folder;
	klass->get_folder_func = tny_msg_get_folder;

	return;
}

static void 
tny_msg_class_init (TnyMsgClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_msg_finalize;

	g_type_class_add_private (object_class, sizeof (TnyMsgPriv));

	return;
}


static void
tny_msg_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyMsg *self = (TnyMsg *)instance;
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (self);

	priv->message = NULL;
	priv->parts = NULL;
	priv->header = NULL;

	return;
}

GType 
tny_msg_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_msg_class_init, /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMsg),
		  0,      /* n_preallocs */
		  tny_msg_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_msg_iface_info = 
		{
		  (GInterfaceInitFunc) tny_msg_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyMsg",
			&info, 0);

		g_type_add_interface_static (type, TNY_MSG_IFACE_TYPE, 
			&tny_msg_iface_info);
	}

	return type;
}


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

#include <tny-msg-attachment.h>

static GObjectClass *parent_class = NULL;

typedef struct _TnyMsgAttachmentPriv TnyMsgAttachmentPriv;

struct _TnyMsgAttachmentPriv
{
	gchar *mime_type;
	gchar *data;
	gint id;
};

#define TNY_MSG_ATTACHMENT_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_MSG_ATTACHMENT_TYPE, TnyMsgAttachmentPriv))

static const gint
tny_msg_attachment_get_id (TnyMsgAttachmentIface *self)
{
	TnyMsgAttachmentPriv *priv = TNY_MSG_ATTACHMENT_GET_PRIVATE (TNY_MSG_ATTACHMENT (self));

	return priv->id;
}

static const gchar*
tny_msg_attachment_get_mime_type (TnyMsgAttachmentIface *self)
{
	TnyMsgAttachmentPriv *priv = TNY_MSG_ATTACHMENT_GET_PRIVATE (TNY_MSG_ATTACHMENT (self));

	return priv->mime_type;
}

static const gchar*
tny_msg_attachment_get_data (TnyMsgAttachmentIface *self)
{
	TnyMsgAttachmentPriv *priv = TNY_MSG_ATTACHMENT_GET_PRIVATE (TNY_MSG_ATTACHMENT (self));

	return priv->data;
}

static void
tny_msg_attachment_set_id (TnyMsgAttachmentIface *self, gint id)
{
	TnyMsgAttachmentPriv *priv = TNY_MSG_ATTACHMENT_GET_PRIVATE (TNY_MSG_ATTACHMENT (self));

	priv->id = id;

	return;
}

static void
tny_msg_attachment_set_mime_type (TnyMsgAttachmentIface *self, gchar *mime_type)
{
	TnyMsgAttachmentPriv *priv = TNY_MSG_ATTACHMENT_GET_PRIVATE (TNY_MSG_ATTACHMENT (self));

	if (priv->mime_type)
		g_free (priv->mime_type);

	priv->mime_type = g_strdup (mime_type);

	return;
}

static void
tny_msg_attachment_set_data (TnyMsgAttachmentIface *self, gchar *data)
{
	TnyMsgAttachmentPriv *priv = TNY_MSG_ATTACHMENT_GET_PRIVATE (TNY_MSG_ATTACHMENT (self));

	if (priv->data)
		g_free (priv->data);

	priv->data = g_strdup (data);

	return;
}

static void
tny_msg_attachment_finalize (GObject *object)
{
	TnyMsgAttachment *self = (TnyMsgAttachment*) object;
	TnyMsgAttachmentPriv *priv = TNY_MSG_ATTACHMENT_GET_PRIVATE (self);

	if (priv->mime_type)
		g_free (priv->mime_type);

	if (priv->data)
		g_free (priv->data);

	(*parent_class->finalize) (object);

	return;
}

static void 
tny_msg_attachment_class_init (TnyMsgAttachmentClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_msg_attachment_finalize;

	g_type_class_add_private (object_class, sizeof (TnyMsgAttachmentPriv));

	return;
}

TnyMsgAttachment*
tny_msg_attachment_new (void)
{
	TnyMsgAttachment *self = g_object_new (TNY_MSG_ATTACHMENT_TYPE, NULL);
	
	return self;
}


static void
tny_msg_attachment_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyMsgAttachmentIfaceClass *klass = (TnyMsgAttachmentIfaceClass *)g_iface;

	klass->get_id_func = tny_msg_attachment_get_id;
	klass->get_mime_type_func = tny_msg_attachment_get_mime_type;
	klass->get_data_func = tny_msg_attachment_get_data;
	klass->set_id_func = tny_msg_attachment_set_id;
	klass->set_mime_type_func = tny_msg_attachment_set_mime_type;
	klass->set_data_func = tny_msg_attachment_set_data;

	return;
}

static void
tny_msg_attachment_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyMsgAttachment *self = (TnyMsgAttachment *)instance;

	TNY_MSG_ATTACHMENT_GET_PRIVATE (self)->mime_type = NULL;
	TNY_MSG_ATTACHMENT_GET_PRIVATE (self)->data = NULL;

	return;
}

GType 
tny_msg_attachment_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgAttachmentClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_msg_attachment_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMsgAttachment),
		  0,      /* n_preallocs */
		  tny_msg_attachment_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_msg_attachment_iface_info = 
		{
		  (GInterfaceInitFunc) tny_msg_attachment_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyMsgAttachment",
			&info, 0);

		g_type_add_interface_static (type, TNY_MSG_ATTACHMENT_IFACE_TYPE, 
			&tny_msg_attachment_iface_info);
	}

	return type;
}

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
#include <tny-msg-attachment-iface.h>
#include <tny-msg-body-iface.h>
#include <tny-msg-header-iface.h>
#include <tny-msg-attachment.h>
#include <tny-msg-body.h>
#include <tny-msg-header.h>

static GObjectClass *parent_class = NULL;

typedef struct _TnyMsgPriv TnyMsgPriv;

struct _TnyMsgPriv
{
	TnyMsgHeaderIface *header;
	TnyMsgBodyIface *body;
	GList *attachments;
};

#define TNY_MSG_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_MSG_TYPE, TnyMsgPriv))

static const GList*
tny_msg_get_attachments (TnyMsgIface *self)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));

	return priv->attachments;
}

static const TnyMsgBodyIface*
tny_msg_get_body (TnyMsgIface *self)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));

	return priv->body;
}

static const TnyMsgHeaderIface*
tny_msg_get_header (TnyMsgIface *self)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));

	return priv->header;
}

static gint
compare_attachments_by_id (gconstpointer a, gconstpointer b)
{
	TnyMsgAttachmentIface *attachmenta = (TnyMsgAttachmentIface*)a;
	TnyMsgAttachmentIface *attachmentb = (TnyMsgAttachmentIface*)b;

	gint aid = tny_msg_attachment_iface_get_id (attachmenta);
	gint bid = tny_msg_attachment_iface_get_id (attachmentb);

	return (aid - bid);
}

static gint
find_attachment_by_id (gconstpointer a, gconstpointer b)
{
	gint aid = (gint)a;
	TnyMsgAttachmentIface *attachment = (TnyMsgAttachmentIface*)b;
	gint bid = tny_msg_attachment_iface_get_id (attachment);

	return (aid - bid);
}

static gint
tny_msg_add_attachment (TnyMsgIface *self, TnyMsgAttachmentIface *attachment)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));

	gint id = g_list_length (priv->attachments);

	id++;

	tny_msg_attachment_iface_set_id (attachment, id);

	priv->attachments = g_list_insert_sorted (
		priv->attachments, 
		attachment, compare_attachments_by_id);

	return id;
}

static void 
tny_msg_del_attachment (TnyMsgIface *self, gint id)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));

	GList *found = g_list_find_custom (priv->attachments,
		(gconstpointer)id, find_attachment_by_id);

	if (found)
	{
		priv->attachments = g_list_remove_link
			(priv->attachments, found);

		g_object_unref (G_OBJECT (found->data));
	}
	
	return;
}

static void
tny_msg_set_body (TnyMsgIface *self, TnyMsgBodyIface *body)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));

	if (priv->body)
		g_object_unref (G_OBJECT (priv->body));

	g_object_ref (G_OBJECT (body));
	priv->body = body;

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
destroy_attachment (gpointer data, gpointer user_data)
{
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

	if (priv->body)
		g_object_unref (G_OBJECT (priv->body));


	if (priv->attachments) 
	{
		g_list_foreach (priv->attachments, 
			destroy_attachment, NULL);

		g_list_free (priv->attachments);
		priv->attachments = NULL;
	}

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

	klass->get_attachments_func = tny_msg_get_attachments;
	klass->get_body_func = tny_msg_get_body;		
	klass->get_header_func = tny_msg_get_header;
	klass->add_attachment_func = tny_msg_add_attachment;
	klass->del_attachment_func = tny_msg_del_attachment;
	klass->set_body_func = tny_msg_set_body;

	return;
}

static void 
tny_msg_class_init (TnyMsgIfaceClass *class)
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


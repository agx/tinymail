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
#include <tny-stream-iface.h>
#include <tny-msg-header-iface.h>
#include <tny-msg-attachment.h>
#include <tny-camel-stream.h>
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
	CamelTransferEncoding encoding = camel_mime_part_get_encoding (part);

	switch (encoding)
	{
		case CAMEL_TRANSFER_ENCODING_DEFAULT:
		case CAMEL_TRANSFER_ENCODING_7BIT:
		case CAMEL_TRANSFER_ENCODING_8BIT:
		case CAMEL_TRANSFER_ENCODING_QUOTEDPRINTABLE:
		{
			CamelDataWrapper *wrapper;
			CamelMedium *medium = CAMEL_MEDIUM (part);
			CamelStream *stream = camel_stream_mem_new ();
			wrapper = camel_medium_get_content_object (medium);
			camel_data_wrapper_write_to_stream (wrapper, stream);

			tny_camel_stream_print (stream);

			priv->body_stream = TNY_STREAM_IFACE 
				(tny_camel_stream_new (stream));

			/* Loose my own ref (tnycamelstream keeps one) */
			camel_object_unref (CAMEL_OBJECT (stream));
		} break;

		case CAMEL_TRANSFER_ENCODING_BASE64:
		case CAMEL_TRANSFER_ENCODING_BINARY:
		case CAMEL_TRANSFER_ENCODING_UUENCODE:
			/* Handle attachments */
			break;

		case CAMEL_TRANSFER_NUM_ENCODINGS:
		default:
			/* Huh? */
			break;
	}

	return TRUE;
}


void
_tny_msg_set_camel_mime_message (TnyMsg *self, CamelMimeMessage *message)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (self);
	CamelDataWrapper *wrapper;
	CamelMimePart *part;

	message_foreach_part_rec (message, (CamelMimePart *)message, received_a_part, priv);

	if (!priv->body_stream)
	{
		g_print ("Message has no body?!\n");
	}

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
tny_msg_get_attachments (TnyMsgIface *self)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));

	return priv->attachments;
}

static const TnyStreamIface*
tny_msg_get_body_stream (TnyMsgIface *self)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));

	return priv->body_stream;
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

	g_object_ref (G_OBJECT (attachment));

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

	if (priv->body_stream)
		g_object_unref (G_OBJECT (priv->body_stream));

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
	klass->get_body_stream_func = tny_msg_get_body_stream;		
	klass->get_header_func = tny_msg_get_header;
	klass->set_header_func = tny_msg_set_header;

	klass->add_attachment_func = tny_msg_add_attachment;
	klass->del_attachment_func = tny_msg_del_attachment;

	klass->set_folder_func = tny_msg_set_folder;
	klass->get_folder_func = tny_msg_get_folder;

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
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (self);

	priv->message = NULL;
	priv->body_stream = NULL;
	priv->attachments = NULL;
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


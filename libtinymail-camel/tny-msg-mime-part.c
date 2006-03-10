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

#include <string.h>

#include <tny-msg-mime-part-iface.h>
#include <tny-msg-mime-part.h>

#include <tny-camel-stream.h>

#include <camel/camel-stream-mem.h>
#include <camel/camel-data-wrapper.h>

#include <tny-camel-shared.h>

static GObjectClass *parent_class = NULL;

typedef struct _TnyMsgMimePartPriv TnyMsgMimePartPriv;

struct _TnyMsgMimePartPriv
{
	CamelMimePart *part;
	gchar *cached_content_type;
	guint index;
};

#define TNY_MSG_MIME_PART_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_MSG_MIME_PART, TnyMsgMimePartPriv))


static void
tny_msg_mime_part_write_to_stream (TnyMsgMimePartIface *self, TnyStreamIface *stream)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);
	TnyStreamIface *retval = NULL;
	CamelDataWrapper *wrapper;

	CamelMedium *medium = CAMEL_MEDIUM (priv->part);
	CamelStream *cstream = CAMEL_STREAM (tny_camel_stream_new (stream));
	
	wrapper = camel_medium_get_content_object (medium);
	camel_data_wrapper_write_to_stream (wrapper, cstream);

	camel_object_unref (CAMEL_OBJECT (cstream));

	return;
}

static TnyStreamIface* 
tny_msg_mime_part_get_stream (TnyMsgMimePartIface *self)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);
	TnyStreamIface *retval = NULL;
	CamelDataWrapper *wrapper;

	CamelMedium *medium = CAMEL_MEDIUM (priv->part);
	CamelStream *stream = camel_stream_mem_new ();
	
	wrapper = camel_medium_get_content_object (medium);
	camel_data_wrapper_write_to_stream (wrapper, stream);

	retval = TNY_STREAM_IFACE (tny_stream_camel_new (stream));

	/* Loose own ref (tnycamelstream keeps one) */
	camel_object_unref (CAMEL_OBJECT (stream));

	return retval;
}

/* TODO: static */
const gchar* 
tny_msg_mime_part_get_content_type (TnyMsgMimePartIface *self)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);

	if (!priv->cached_content_type)
	{
		CamelContentType *type = camel_mime_part_get_content_type (priv->part);
		priv->cached_content_type = g_strdup_printf ("%s/%s", type->type, type->subtype);
	}

	return priv->cached_content_type;
}

static gboolean 
tny_msg_mime_part_content_type_is (TnyMsgMimePartIface *self, const gchar *type)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);
	CamelContentType *ctype = camel_mime_part_get_content_type (priv->part);
	gchar *dup, *str1, *str2, *ptr;
	gboolean retval = FALSE;

	/* Whoooo, pointer hocus .. */

	dup = g_strdup (type);
	ptr = strchr (dup, '/');
	ptr++; str2 = g_strdup (ptr);
	ptr--; *ptr = '\0'; str1 = dup;

	/* pocus ! */

	retval = camel_content_type_is (ctype, (const char*)str1, (const char*)str2);

	g_free (dup);
	g_free (str2);

	return retval;
}


static void
tny_msg_mime_part_finalize (GObject *object)
{
	TnyMsgMimePart *self = (TnyMsgMimePart*) object;
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);

	if (priv->cached_content_type)
		g_free (priv->cached_content_type);

	if (priv->part)
		camel_object_unref (CAMEL_OBJECT (priv->part));

	(*parent_class->finalize) (object);

	return;
}

void
tny_msg_mime_part_set_part (TnyMsgMimePart *self, CamelMimePart *part)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);

	if (priv->cached_content_type)
		g_free (priv->cached_content_type);

	priv->cached_content_type = NULL;

	if (priv->part)
		camel_object_unref (CAMEL_OBJECT (priv->part));

	camel_object_ref (CAMEL_OBJECT (part));

	priv->part = part;

	return;
}

CamelMimePart*
tny_msg_mime_part_get_part (TnyMsgMimePart *self)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);

	return priv->part;
}


static void
tny_msg_mime_part_set_index (TnyMsgMimePartIface *self, guint index)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);

	priv->index = index;

	return;
}

static const guint
tny_msg_mime_part_get_index (TnyMsgMimePartIface *self)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);

	return priv->index;
}

static const gchar*
tny_msg_mime_part_get_filename (TnyMsgMimePartIface *self)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);

	return camel_mime_part_get_filename (priv->part);
}

static const gchar*
tny_msg_mime_part_get_content_id (TnyMsgMimePartIface *self)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);

	return camel_mime_part_get_content_id (priv->part);
}

static const gchar*
tny_msg_mime_part_get_description (TnyMsgMimePartIface *self)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);

	return camel_mime_part_get_description (priv->part);
}

static const gchar*
tny_msg_mime_part_get_content_location (TnyMsgMimePartIface *self)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);

	return camel_mime_part_get_content_location (priv->part);
}

/**
 * tny_msg_mime_part_new:
 * 
 * The #TnyMsgMimePart implementation is actually a proxy for #CamelMimePart.
 *
 * Return value: A new #TnyMsgMimePartIface instance implemented for Camel
 **/
TnyMsgMimePart*
tny_msg_mime_part_new (CamelMimePart *part)
{
	TnyMsgMimePart *self = g_object_new (TNY_TYPE_MSG_MIME_PART, NULL);
	
	tny_msg_mime_part_set_part (self, part);

	return self;
}



static void
tny_msg_mime_part_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyMsgMimePartIfaceClass *klass = (TnyMsgMimePartIfaceClass *)g_iface;

	klass->content_type_is_func = tny_msg_mime_part_content_type_is;
	klass->get_content_type_func = tny_msg_mime_part_get_content_type;
	klass->get_stream_func = tny_msg_mime_part_get_stream;
	klass->write_to_stream_func = tny_msg_mime_part_write_to_stream;

	klass->set_index_func = tny_msg_mime_part_set_index;
	klass->get_index_func = tny_msg_mime_part_get_index;

	klass->get_filename_func = tny_msg_mime_part_get_filename;
	klass->get_content_id_func = tny_msg_mime_part_get_content_id;
	klass->get_description_func = tny_msg_mime_part_get_description;
	klass->get_content_location_func = tny_msg_mime_part_get_content_location;

	return;
}


static void 
tny_msg_mime_part_class_init (TnyMsgMimePartClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_msg_mime_part_finalize;

	g_type_class_add_private (object_class, sizeof (TnyMsgMimePartPriv));

	return;
}

static void
tny_msg_mime_part_instance_init (GTypeInstance *instance, gpointer g_class)
{
	return;
}

GType 
tny_msg_mime_part_get_type (void)
{
	static GType type = 0;

	if (!camel_type_init_done)
	{
		camel_type_init ();
		camel_type_init_done = TRUE;
	}

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgMimePartClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_msg_mime_part_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMsgMimePart),
		  0,      /* n_preallocs */
		  tny_msg_mime_part_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_msg_mime_part_iface_info = 
		{
		  (GInterfaceInitFunc) tny_msg_mime_part_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyMsgMimePart",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_MSG_MIME_PART_IFACE, 
			&tny_msg_mime_part_iface_info);
	}

	return type;
}

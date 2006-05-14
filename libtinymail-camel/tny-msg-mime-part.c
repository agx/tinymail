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

#include "tny-msg-mime-part-priv.h"

/* Locking warning: tny-msg.c also locks priv->part_lock */

static gboolean 
tny_msg_mime_part_is_attachment (TnyMsgMimePartIface *self)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);
	CamelDataWrapper *dw = camel_medium_get_content_object((CamelMedium *)priv->part);

	/* From evolution/mail/em-format.c (func=em_format_is_attachment) */

	if (G_LIKELY (dw))
	{
		return !(camel_content_type_is (dw->mime_type, "multipart", "*")
                 || camel_content_type_is(dw->mime_type, "application", "x-pkcs7-mime")
                 || camel_content_type_is(dw->mime_type, "application", "pkcs7-mime")
                 || camel_content_type_is(dw->mime_type, "application", "x-inlinepgp-signed")
                 || camel_content_type_is(dw->mime_type, "application", "x-inlinepgp-encrypted")
                 || (camel_content_type_is (dw->mime_type, "text", "*")
                     && camel_mime_part_get_filename(priv->part) == NULL));
	}

	return FALSE;
}

static void
tny_msg_mime_part_write_to_stream (TnyMsgMimePartIface *self, TnyStreamIface *stream)
{
	/* TODO: Add a filter (decoder) here */

	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);
	CamelDataWrapper *wrapper;
	CamelMedium *medium;
	CamelStream *cstream = CAMEL_STREAM (tny_camel_stream_new (stream));

	g_mutex_lock (priv->part_lock);

	medium = CAMEL_MEDIUM (priv->part);
	camel_object_ref (CAMEL_OBJECT (medium));

	/* Once medium is referenced, we can continue without lock */
	g_mutex_unlock (priv->part_lock);

	wrapper = camel_medium_get_content_object (medium);

	if (G_UNLIKELY (!wrapper))
	{
		g_error ("Mime part does not yet have a source stream, use "
			"tny_msg_mime_part_construct_from_stream first");
		camel_object_unref (CAMEL_OBJECT (cstream));
		return;
	}

	camel_stream_reset (wrapper->stream);
	camel_stream_write_to_stream (wrapper->stream, cstream);

	/* This should work but doesn't . . .
	camel_data_wrapper_write_to_stream (wrapper, cstream); */

	camel_object_unref (CAMEL_OBJECT (cstream));

	/* We are done, so unreference the reference above */
	camel_object_unref (CAMEL_OBJECT (medium));

	return;
}

static gint
tny_msg_mime_part_construct_from_stream (TnyMsgMimePartIface *self, TnyStreamIface *stream, const gchar *type)
{

	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);
	CamelDataWrapper *wrapper;
	gint retval = -1;
	CamelMedium *medium;
	CamelStream *cstream = CAMEL_STREAM (tny_camel_stream_new (stream));
	
	g_mutex_lock (priv->part_lock);
	medium = CAMEL_MEDIUM (priv->part);
	camel_object_ref (CAMEL_OBJECT (medium));
	g_mutex_unlock (priv->part_lock);

	wrapper = camel_medium_get_content_object (medium);

	if (G_LIKELY (wrapper))
		camel_object_unref (CAMEL_OBJECT (wrapper));

	wrapper = camel_data_wrapper_new (); 
	camel_data_wrapper_set_mime_type (wrapper, type);
	retval = camel_data_wrapper_construct_from_stream (wrapper, cstream);

	camel_medium_set_content_object(medium, wrapper);

	camel_object_unref (CAMEL_OBJECT (cstream));

	camel_object_unref (CAMEL_OBJECT (medium));

	return retval;
}

static TnyStreamIface* 
tny_msg_mime_part_get_stream (TnyMsgMimePartIface *self)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);
	TnyStreamIface *retval = NULL;
	CamelDataWrapper *wrapper;
	CamelMedium *medium;
	CamelStream *stream = camel_stream_mem_new ();
	
	g_mutex_lock (priv->part_lock);
	medium =  CAMEL_MEDIUM (priv->part);
	camel_object_ref (CAMEL_OBJECT (medium));
	g_mutex_unlock (priv->part_lock);

	wrapper = camel_medium_get_content_object (medium);

	if (G_UNLIKELY (!wrapper))
	{
		wrapper = camel_data_wrapper_new (); 
		camel_medium_set_content_object (medium, wrapper);
	}

	retval = TNY_STREAM_IFACE (tny_stream_camel_new (stream));

	/* Parenting: Loose own ref (the tnystreamcamel wrapper keeps one) */
	camel_object_unref (CAMEL_OBJECT (stream));

	tny_stream_iface_reset (retval);

	camel_object_unref (CAMEL_OBJECT (medium));

	return retval;
}

/* TODO: static */
const gchar* 
tny_msg_mime_part_get_content_type (TnyMsgMimePartIface *self)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);

	if (G_LIKELY (!priv->cached_content_type))
	{
		CamelContentType *type;

		g_mutex_lock (priv->part_lock);
		type = camel_mime_part_get_content_type (priv->part);
		priv->cached_content_type = g_strdup_printf ("%s/%s", type->type, type->subtype);
		/* TODO: Q: camel_content_type_unref (type); */
		g_mutex_unlock (priv->part_lock);
	}

	return priv->cached_content_type;
}

static gboolean 
tny_msg_mime_part_content_type_is (TnyMsgMimePartIface *self, const gchar *type)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);
	CamelContentType *ctype;
	gchar *dup, *str1, *str2, *ptr;
	gboolean retval = FALSE;

	g_mutex_lock (priv->part_lock);
	ctype = camel_mime_part_get_content_type (priv->part);
	g_mutex_unlock (priv->part_lock);

	/* Whoooo, pointer hocus .. */

	dup = g_strdup (type);
	ptr = strchr (dup, '/');
	ptr++; str2 = g_strdup (ptr);
	ptr--; *ptr = '\0'; str1 = dup;

	/* pocus ! */

	retval = camel_content_type_is (ctype, (const char*)str1, 
			(const char*)str2);

	/* TODO: Q: camel_content_type_unref (ctype); */

	g_free (dup);
	g_free (str2);

	return retval;
}


void
tny_msg_mime_part_set_part (TnyMsgMimePart *self, CamelMimePart *part)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->part_lock);

	if (G_UNLIKELY (priv->cached_content_type))
		g_free (priv->cached_content_type);
	priv->cached_content_type = NULL;
	if (G_UNLIKELY (priv->part))
		camel_object_unref (CAMEL_OBJECT (priv->part));
	camel_object_ref (CAMEL_OBJECT (part));
	priv->part = part;

	g_mutex_unlock (priv->part_lock);

	return;
}

CamelMimePart*
tny_msg_mime_part_get_part (TnyMsgMimePart *self)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);
	CamelMimePart *retval;

	g_mutex_lock (priv->part_lock);
	retval = priv->part;
	g_mutex_unlock (priv->part_lock);

	return retval;
}


static const gchar*
tny_msg_mime_part_get_filename (TnyMsgMimePartIface *self)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);
	const gchar *retval;

	g_mutex_lock (priv->part_lock);
	retval = camel_mime_part_get_filename (priv->part);
	g_mutex_unlock (priv->part_lock);
	
	return retval;
}

static const gchar*
tny_msg_mime_part_get_content_id (TnyMsgMimePartIface *self)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);
	const gchar *retval;

	g_mutex_lock (priv->part_lock);
	retval = camel_mime_part_get_content_id (priv->part);
	g_mutex_unlock (priv->part_lock);

	return retval;
}

static const gchar*
tny_msg_mime_part_get_description (TnyMsgMimePartIface *self)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);
	const gchar *retval;

	g_mutex_lock (priv->part_lock);
	retval = camel_mime_part_get_description (priv->part);
	g_mutex_unlock (priv->part_lock);

	return retval;
}

static const gchar*
tny_msg_mime_part_get_content_location (TnyMsgMimePartIface *self)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);
	const gchar *retval;

	g_mutex_lock (priv->part_lock);
	retval = camel_mime_part_get_content_location (priv->part);
	g_mutex_unlock (priv->part_lock);

	return retval;
}


static void 
tny_msg_mime_part_set_content_location (TnyMsgMimePartIface *self, const gchar *content_location)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->part_lock);
	camel_mime_part_set_content_location (priv->part, content_location);
	g_mutex_unlock (priv->part_lock);

	return;
}

static void 
tny_msg_mime_part_set_description (TnyMsgMimePartIface *self, const gchar *description)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->part_lock);
	camel_mime_part_set_description (priv->part, description);
	g_mutex_unlock (priv->part_lock);

	return;
}

static void 
tny_msg_mime_part_set_content_id (TnyMsgMimePartIface *self, const gchar *content_id)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->part_lock);
	camel_mime_part_set_content_id (priv->part, content_id);
	g_mutex_unlock (priv->part_lock);

	return;
}

static void 
tny_msg_mime_part_set_filename (TnyMsgMimePartIface *self, const gchar *filename)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->part_lock);
	camel_mime_part_set_filename (priv->part, filename);
	g_mutex_unlock (priv->part_lock);

	return;
}

static void 
tny_msg_mime_part_set_content_type (TnyMsgMimePartIface *self, const gchar *content_type)
{
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->part_lock);

	camel_mime_part_set_content_type (priv->part, content_type);
	if (G_UNLIKELY (priv->cached_content_type))
		g_free (priv->cached_content_type);
	priv->cached_content_type = NULL;

	g_mutex_unlock (priv->part_lock);

	return;
}

static void
tny_msg_mime_part_finalize (GObject *object)
{
	TnyMsgMimePart *self = (TnyMsgMimePart*) object;
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->part_lock);
	if (priv->cached_content_type)
		g_free (priv->cached_content_type);
	priv->cached_content_type = NULL;
	g_mutex_unlock (priv->part_lock);

	if (G_LIKELY (priv->part))
	{
		g_mutex_lock (priv->part_lock);
		camel_object_unref (CAMEL_OBJECT (priv->part));
		g_mutex_unlock (priv->part_lock);
	}

	g_mutex_free (priv->part_lock);
	priv->part_lock = NULL;

	(*parent_class->finalize) (object);

	return;
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
	klass->construct_from_stream_func = tny_msg_mime_part_construct_from_stream;
	klass->get_filename_func = tny_msg_mime_part_get_filename;
	klass->get_content_id_func = tny_msg_mime_part_get_content_id;
	klass->get_description_func = tny_msg_mime_part_get_description;
	klass->get_content_location_func = tny_msg_mime_part_get_content_location;
	klass->set_content_location_func = tny_msg_mime_part_set_content_location;
	klass->set_description_func = tny_msg_mime_part_set_description;
	klass->set_content_id_func = tny_msg_mime_part_set_content_id;
	klass->set_filename_func = tny_msg_mime_part_set_filename;
	klass->set_content_type_func = tny_msg_mime_part_set_content_type;
	klass->is_attachment_func = tny_msg_mime_part_is_attachment;

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
	TnyMsgMimePart *self = (TnyMsgMimePart*)instance;
	TnyMsgMimePartPriv *priv = TNY_MSG_MIME_PART_GET_PRIVATE (self);

	priv->part_lock = g_mutex_new ();

	return;
}

GType 
tny_msg_mime_part_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY (!camel_type_init_done))
	{
		camel_type_init ();
		camel_type_init_done = TRUE;
	}

	if (G_UNLIKELY(type == 0))
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

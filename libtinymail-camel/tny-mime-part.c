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

#include <string.h>
#include <tny-mime-part-iface.h>
#include <tny-mime-part.h>
#include <tny-camel-stream.h>
#include <camel/camel-stream-mem.h>
#include <camel/camel-data-wrapper.h>
#include <tny-camel-shared.h>

static GObjectClass *parent_class = NULL;

#include "tny-mime-part-priv.h"

#include <camel/camel-url.h>
#include <camel/camel-stream.h>
#include <camel/camel-stream-mem.h>
#include <camel/camel-multipart.h>
#include <camel/camel-multipart-encrypted.h>
#include <camel/camel-multipart-signed.h>
#include <camel/camel-medium.h>
#include <camel/camel-mime-message.h>
#include <camel/camel-gpg-context.h>
#include <camel/camel-smime-context.h>
#include <camel/camel-string-utils.h>
#include <camel/camel-stream-filter.h>
#include <camel/camel-stream-null.h>
#include <camel/camel-mime-filter-charset.h>
#include <camel/camel-mime-filter-windows.h>


/* Locking warning: tny-msg.c also locks priv->part_lock */

static gboolean 
tny_mime_part_is_attachment (TnyMimePartIface *self)
{
	TnyMimePartPriv *priv = TNY_MIME_PART_GET_PRIVATE (self);
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
tny_mime_part_write_to_stream (TnyMimePartIface *self, TnyStreamIface *stream)
{
	TnyMimePartPriv *priv = TNY_MIME_PART_GET_PRIVATE (self);
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
		g_error (_("Mime part does not yet have a source stream, use "
			"tny_mime_part_construct_from_stream first"));
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

static void
camel_stream_format_text (CamelDataWrapper *dw, CamelStream *stream)
{
	/* Stolen from evolution, evil evil me!! moehahah */

        CamelStreamFilter *filter_stream;
        CamelMimeFilterCharset *filter;
        const char *charset = "UTF-8"; /* I default to UTF-8, like it or not */
        CamelMimeFilterWindows *windows = NULL;

	if (dw->mime_type && (charset = camel_content_type_param 
		(dw->mime_type, "charset")) && 
		g_ascii_strncasecmp(charset, "iso-8859-", 9) == 0) 
	{
                CamelStream *null;

                /* Since a few Windows mailers like to claim they sent
                 * out iso-8859-# encoded text when they really sent
                 * out windows-cp125#, do some simple sanity checking
                 * before we move on... */

                null = camel_stream_null_new();
                filter_stream = camel_stream_filter_new_with_stream(null);
                camel_object_unref(null);

                windows = (CamelMimeFilterWindows *)camel_mime_filter_windows_new(charset);
                camel_stream_filter_add (filter_stream, (CamelMimeFilter *)windows);

                camel_data_wrapper_decode_to_stream (dw, (CamelStream *)filter_stream);
                camel_stream_flush ((CamelStream *)filter_stream);
                camel_object_unref (filter_stream);

                charset = camel_mime_filter_windows_real_charset (windows);

        }

        filter_stream = camel_stream_filter_new_with_stream (stream);

        if ((filter = camel_mime_filter_charset_new_convert (charset, "UTF-8"))) 
	{
                camel_stream_filter_add (filter_stream, (CamelMimeFilter *) filter);
                camel_object_unref (filter);
        }

        camel_data_wrapper_decode_to_stream (dw, (CamelStream *)filter_stream);
        camel_stream_flush ((CamelStream *)filter_stream);
        camel_object_unref (filter_stream);

        if (windows)
                camel_object_unref(windows);

	return;
}


static void
tny_mime_part_decode_to_stream (TnyMimePartIface *self, TnyStreamIface *stream)
{

	TnyMimePartPriv *priv = TNY_MIME_PART_GET_PRIVATE (self);
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
		g_error (_("Mime part does not yet have a source stream, use "
			"tny_mime_part_construct_from_stream first"));
		camel_object_unref (CAMEL_OBJECT (cstream));
		return;
	}

	if (camel_content_type_is (wrapper->mime_type, "text", "*"))
		camel_stream_format_text (wrapper, cstream);
        else
		camel_data_wrapper_decode_to_stream (wrapper, cstream);

	camel_object_unref (CAMEL_OBJECT (cstream));

	/* We are done, so unreference the reference above */
	camel_object_unref (CAMEL_OBJECT (medium));

	return;
}

static gint
tny_mime_part_construct_from_stream (TnyMimePartIface *self, TnyStreamIface *stream, const gchar *type)
{

	TnyMimePartPriv *priv = TNY_MIME_PART_GET_PRIVATE (self);
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
tny_mime_part_get_stream (TnyMimePartIface *self)
{
	TnyMimePartPriv *priv = TNY_MIME_PART_GET_PRIVATE (self);
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

const gchar* 
tny_mime_part_get_content_type (TnyMimePartIface *self)
{
	TnyMimePartPriv *priv = TNY_MIME_PART_GET_PRIVATE (self);

	if (G_LIKELY (!priv->cached_content_type))
	{
		CamelContentType *type;

		g_mutex_lock (priv->part_lock);
		type = camel_mime_part_get_content_type (priv->part);
		priv->cached_content_type = g_strdup_printf ("%s/%s", type->type, type->subtype);
		/* Q: camel_content_type_unref (type); */
		g_mutex_unlock (priv->part_lock);
	}

	return priv->cached_content_type;
}

static gboolean 
tny_mime_part_content_type_is (TnyMimePartIface *self, const gchar *type)
{
	TnyMimePartPriv *priv = TNY_MIME_PART_GET_PRIVATE (self);
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

/**
 * tny_mime_part_set_part:
 * @self: The #TnyMimePart instance
 * @part: The #CamelMimePart instance
 *
 *
 **/
void
tny_mime_part_set_part (TnyMimePart *self, CamelMimePart *part)
{
	TnyMimePartPriv *priv = TNY_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->part_lock);

	if (G_UNLIKELY (priv->cached_content_type))
		g_free (priv->cached_content_type);
	priv->cached_content_type = NULL;

	/*if (priv->part)
		camel_object_unref (CAMEL_OBJECT (priv->part));
	camel_object_ref (CAMEL_OBJECT (part));*/

	priv->part = part;

	g_mutex_unlock (priv->part_lock);

	return;
}

/**
 * tny_mime_part_get_part:
 * @self: The #TnyMimePart instance
 *
 * Return value: The #CamelMimePart instance
 **/
CamelMimePart*
tny_mime_part_get_part (TnyMimePart *self)
{
	TnyMimePartPriv *priv = TNY_MIME_PART_GET_PRIVATE (self);
	CamelMimePart *retval;

	g_mutex_lock (priv->part_lock);
	retval = priv->part;
	g_mutex_unlock (priv->part_lock);

	return retval;
}


static const gchar*
tny_mime_part_get_filename (TnyMimePartIface *self)
{
	TnyMimePartPriv *priv = TNY_MIME_PART_GET_PRIVATE (self);
	const gchar *retval;

	g_mutex_lock (priv->part_lock);
	retval = camel_mime_part_get_filename (priv->part);
	g_mutex_unlock (priv->part_lock);
	
	return retval;
}

static const gchar*
tny_mime_part_get_content_id (TnyMimePartIface *self)
{
	TnyMimePartPriv *priv = TNY_MIME_PART_GET_PRIVATE (self);
	const gchar *retval;

	g_mutex_lock (priv->part_lock);
	retval = camel_mime_part_get_content_id (priv->part);
	g_mutex_unlock (priv->part_lock);

	return retval;
}

static const gchar*
tny_mime_part_get_description (TnyMimePartIface *self)
{
	TnyMimePartPriv *priv = TNY_MIME_PART_GET_PRIVATE (self);
	const gchar *retval;

	g_mutex_lock (priv->part_lock);
	retval = camel_mime_part_get_description (priv->part);
	g_mutex_unlock (priv->part_lock);

	return retval;
}

static const gchar*
tny_mime_part_get_content_location (TnyMimePartIface *self)
{
	TnyMimePartPriv *priv = TNY_MIME_PART_GET_PRIVATE (self);
	const gchar *retval;

	g_mutex_lock (priv->part_lock);
	retval = camel_mime_part_get_content_location (priv->part);
	g_mutex_unlock (priv->part_lock);

	return retval;
}


static void 
tny_mime_part_set_content_location (TnyMimePartIface *self, const gchar *content_location)
{
	TnyMimePartPriv *priv = TNY_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->part_lock);
	camel_mime_part_set_content_location (priv->part, content_location);
	g_mutex_unlock (priv->part_lock);

	return;
}

static void 
tny_mime_part_set_description (TnyMimePartIface *self, const gchar *description)
{
	TnyMimePartPriv *priv = TNY_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->part_lock);
	camel_mime_part_set_description (priv->part, description);
	g_mutex_unlock (priv->part_lock);

	return;
}

static void 
tny_mime_part_set_content_id (TnyMimePartIface *self, const gchar *content_id)
{
	TnyMimePartPriv *priv = TNY_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->part_lock);
	camel_mime_part_set_content_id (priv->part, content_id);
	g_mutex_unlock (priv->part_lock);

	return;
}

static void 
tny_mime_part_set_filename (TnyMimePartIface *self, const gchar *filename)
{
	TnyMimePartPriv *priv = TNY_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->part_lock);
	camel_mime_part_set_filename (priv->part, filename);
	g_mutex_unlock (priv->part_lock);

	return;
}

static void 
tny_mime_part_set_content_type (TnyMimePartIface *self, const gchar *content_type)
{
	TnyMimePartPriv *priv = TNY_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->part_lock);

	camel_mime_part_set_content_type (priv->part, content_type);
	if (G_UNLIKELY (priv->cached_content_type))
		g_free (priv->cached_content_type);
	priv->cached_content_type = NULL;

	g_mutex_unlock (priv->part_lock);

	return;
}

static void
tny_mime_part_finalize (GObject *object)
{
	TnyMimePart *self = (TnyMimePart*) object;
	TnyMimePartPriv *priv = TNY_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->part_lock);
	if (priv->cached_content_type)
		g_free (priv->cached_content_type);
	priv->cached_content_type = NULL;
	g_mutex_unlock (priv->part_lock);

	if (G_LIKELY (priv->part))
	{
		g_mutex_lock (priv->part_lock);

		/* http://bugzilla.gnome.org/show_bug.cgi?id=343683 */
		/* camel_object_unref (CAMEL_OBJECT (priv->part)); */
		g_mutex_unlock (priv->part_lock);
	}

	g_mutex_free (priv->part_lock);
	priv->part_lock = NULL;

	(*parent_class->finalize) (object);

	return;
}


/**
 * tny_mime_part_new:
 * 
 * The #TnyMimePart implementation is actually a proxy for #CamelMimePart.
 *
 * Return value: A new #TnyMimePartIface instance implemented for Camel
 **/
TnyMimePart*
tny_mime_part_new (CamelMimePart *part)
{
	TnyMimePart *self = g_object_new (TNY_TYPE_MIME_PART, NULL);
	
	tny_mime_part_set_part (self, part);

	return self;
}



static void
tny_mime_part_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyMimePartIfaceClass *klass = (TnyMimePartIfaceClass *)g_iface;

	klass->content_type_is_func = tny_mime_part_content_type_is;
	klass->get_content_type_func = tny_mime_part_get_content_type;
	klass->get_stream_func = tny_mime_part_get_stream;
	klass->write_to_stream_func = tny_mime_part_write_to_stream;
	klass->construct_from_stream_func = tny_mime_part_construct_from_stream;
	klass->get_filename_func = tny_mime_part_get_filename;
	klass->get_content_id_func = tny_mime_part_get_content_id;
	klass->get_description_func = tny_mime_part_get_description;
	klass->get_content_location_func = tny_mime_part_get_content_location;
	klass->set_content_location_func = tny_mime_part_set_content_location;
	klass->set_description_func = tny_mime_part_set_description;
	klass->set_content_id_func = tny_mime_part_set_content_id;
	klass->set_filename_func = tny_mime_part_set_filename;
	klass->set_content_type_func = tny_mime_part_set_content_type;
	klass->is_attachment_func = tny_mime_part_is_attachment;

	klass->decode_to_stream_func = tny_mime_part_decode_to_stream;

	return;
}


static void 
tny_mime_part_class_init (TnyMimePartClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_mime_part_finalize;

	g_type_class_add_private (object_class, sizeof (TnyMimePartPriv));

	return;
}

static void
tny_mime_part_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyMimePart *self = (TnyMimePart*)instance;
	TnyMimePartPriv *priv = TNY_MIME_PART_GET_PRIVATE (self);

	priv->part_lock = g_mutex_new ();

	return;
}

GType 
tny_mime_part_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY (!camel_type_init_done))
	{
		if (!g_thread_supported ()) 
			g_thread_init (NULL);

		camel_type_init ();
		camel_type_init_done = TRUE;
	}

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMimePartClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_mime_part_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMimePart),
		  0,      /* n_preallocs */
		  tny_mime_part_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_mime_part_iface_info = 
		{
		  (GInterfaceInitFunc) tny_mime_part_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyMimePart",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_MIME_PART_IFACE, 
			&tny_mime_part_iface_info);
	}

	return type;
}

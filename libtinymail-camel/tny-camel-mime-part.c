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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <config.h>

#include <glib/gi18n-lib.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <string.h>

#include <tny-mime-part.h>
#include <tny-camel-mime-part.h>
#include <tny-camel-stream.h>
#include <tny-stream-camel.h>
#include <camel/camel-stream-mem.h>
#include <camel/camel-data-wrapper.h>
#include <tny-camel-shared.h>
#include <tny-list.h>
#include <tny-camel-msg.h>

static GObjectClass *parent_class = NULL;

#include "tny-camel-mime-part-priv.h"
#include "tny-camel-msg-header-priv.h"
#include "tny-camel-msg-priv.h"

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

static void 
tny_camel_mime_part_set_header_pair (TnyMimePart *self, const gchar *name, const gchar *value)
{
	TNY_CAMEL_MIME_PART_GET_CLASS (self)->set_header_pair_func (self, name, value);
	return;
}

static void 
tny_camel_mime_part_set_header_pair_default (TnyMimePart *self, const gchar *name, const gchar *value)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);

	camel_medium_add_header (CAMEL_MEDIUM (priv->part), name, value);

	return;
}

static void 
tny_camel_mime_part_get_header_pairs (TnyMimePart *self, TnyList *list)
{
	TNY_CAMEL_MIME_PART_GET_CLASS (self)->get_header_pairs_func (self, list);
	return;
}

static void 
tny_camel_mime_part_get_header_pairs_default (TnyMimePart *self, TnyList *list)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);
	guint i = 0;
	GArray *headers = camel_medium_get_headers (CAMEL_MEDIUM (priv->part));
	
	for (i=0; i < headers->len; i++)
	{
		CamelMediumHeader *header = &g_array_index (headers, CamelMediumHeader, i);
		tny_list_append (list, G_OBJECT (tny_pair_new (header->name, header->value)));
	}

	camel_medium_free_headers (CAMEL_MEDIUM (priv->part), headers);

	return;
}

static void
tny_camel_mime_part_get_parts (TnyMimePart *self, TnyList *list)
{
	TNY_CAMEL_MIME_PART_GET_CLASS (self)->get_parts_func (self, list);
	return;
}

static void
tny_camel_mime_part_get_parts_default (TnyMimePart *self, TnyList *list)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);
	CamelDataWrapper *containee;
	gboolean is_related = FALSE;
	CamelContentType *content_type = NULL;

	g_assert (TNY_IS_LIST (list));

	g_mutex_lock (priv->part_lock);

	containee = camel_medium_get_content_object (CAMEL_MEDIUM (priv->part));
	
	if (G_UNLIKELY (containee == NULL)) {
		g_mutex_unlock (priv->part_lock);
		return;
	}

	content_type = camel_mime_part_get_content_type (priv->part);
	if (content_type != NULL) {
		if ((strcmp (content_type->type, "multipart")==0) &&
		    (strcmp (content_type->subtype, "related") == 0)) {
			is_related = TRUE;
		}
	}

	if (CAMEL_IS_MULTIPART (containee))
	{
		guint i, parts = camel_multipart_get_number (CAMEL_MULTIPART (containee));
		for (i = 0; i < parts; i++) 
		{
			CamelMimePart *tpart = camel_multipart_get_part (CAMEL_MULTIPART (containee), i);
			TnyMimePart *newpart=NULL;
			CamelContentType *type;

			if (!tpart || !CAMEL_IS_MIME_PART (tpart))
				continue;

			type = camel_mime_part_get_content_type (tpart);
			if (CAMEL_IS_MIME_MESSAGE (tpart))
			{
				TnyHeader *nheader = NULL;

				newpart = TNY_MIME_PART (tny_camel_msg_new ());
				_tny_camel_mime_part_set_part (TNY_CAMEL_MIME_PART (newpart), CAMEL_MIME_PART (tpart));

				nheader = _tny_camel_msg_header_new (CAMEL_MIME_MESSAGE (tpart), NULL, 
					camel_mime_message_get_date_received (CAMEL_MIME_MESSAGE (tpart), NULL));
				_tny_camel_msg_set_header (TNY_CAMEL_MSG (newpart), nheader);

				g_object_unref (nheader);
			}
			else if (camel_content_type_is (type, "message", "rfc822"))
			{
				CamelDataWrapper *c = camel_medium_get_content_object (CAMEL_MEDIUM (tpart));

				if (c && CAMEL_IS_MIME_PART (c) && CAMEL_IS_MIME_MESSAGE (c)) 
				{
					TnyHeader *nheader = NULL;

					newpart = TNY_MIME_PART (tny_camel_msg_new ());
					_tny_camel_mime_part_set_part (TNY_CAMEL_MIME_PART (newpart), CAMEL_MIME_PART (c));
					nheader = _tny_camel_msg_header_new (CAMEL_MIME_MESSAGE (c), NULL, 
						camel_mime_message_get_date_received (CAMEL_MIME_MESSAGE (c), NULL));
					_tny_camel_msg_set_header (TNY_CAMEL_MSG (newpart), nheader);

					g_object_unref (nheader);
				}

			} else {
				newpart = tny_camel_mime_part_new_with_part (tpart);
				if (is_related && (camel_mime_part_get_disposition (tpart) == NULL)) {
					camel_mime_part_set_disposition (tpart, "inline");
				}
			}

			tny_list_prepend (list, G_OBJECT (newpart));
			g_object_unref (G_OBJECT (newpart));
		}
	}

	g_mutex_unlock (priv->part_lock);

	return;
}



static gint
tny_camel_mime_part_add_part (TnyMimePart *self, TnyMimePart *part)
{
	return TNY_CAMEL_MIME_PART_GET_CLASS (self)->add_part_func (self, part);
}

static gint
tny_camel_mime_part_add_part_default (TnyMimePart *self, TnyMimePart *part)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);
	CamelMedium *medium;
	CamelDataWrapper *containee;
	CamelMultipart *body;
	gint curl = 0, retval = 0;
	CamelMimePart *cpart;

	/* Yes, indeed (I don't yet support non TnyCamelMimePart mime part 
	   instances, and I know I should. Feel free to implement the copying
	   if you really need it) */

	g_assert (TNY_IS_CAMEL_MIME_PART (part));

	g_mutex_lock (priv->part_lock);

	medium = CAMEL_MEDIUM (priv->part);
	containee = camel_medium_get_content_object (medium);

	/* Warp it into a multipart */
	if (G_UNLIKELY (!containee) || G_LIKELY (!CAMEL_IS_MULTIPART (containee)))
	{
		CamelContentType *type;
		gchar *applied_type = NULL;
		if (containee)
			camel_object_unref (CAMEL_OBJECT (containee));

		curl = 0;
		
		type = camel_mime_part_get_content_type (priv->part);
		if (!g_ascii_strcasecmp (type->type, "multipart")) {
			applied_type = g_strdup_printf ("%s/%s", type->type, type->subtype);
		} else {
			applied_type = g_strdup ("multipart/mixed");
		}

		body = camel_multipart_new ();
		camel_data_wrapper_set_mime_type (CAMEL_DATA_WRAPPER (body),
						applied_type);
		g_free (applied_type);
		camel_multipart_set_boundary (body, NULL);
		camel_medium_set_content_object (medium, CAMEL_DATA_WRAPPER (body));
	} else
		body = CAMEL_MULTIPART (containee);

	cpart = tny_camel_mime_part_get_part (TNY_CAMEL_MIME_PART (part));
	if (CAMEL_IS_MIME_MESSAGE (cpart)) {
		CamelMimePart *message_part = camel_mime_part_new ();
		const gchar *subject;
		gchar *description;

		subject = camel_mime_message_get_subject (CAMEL_MIME_MESSAGE (cpart));
		if (subject)
			description = g_strdup (subject);
		else
			description = _("Forwarded message");

		camel_mime_part_set_disposition (message_part, "inline");
		camel_medium_set_content_object (CAMEL_MEDIUM (message_part), 
						 CAMEL_DATA_WRAPPER (cpart));
		camel_mime_part_set_content_type (message_part, "message/rfc822");
		camel_multipart_add_part (body, message_part);
		camel_object_unref (CAMEL_OBJECT (message_part));
	} else {
		camel_multipart_add_part (body, cpart);
	}
	camel_object_unref (CAMEL_OBJECT (cpart));

	retval = camel_multipart_get_number (body);

	g_mutex_unlock (priv->part_lock);

	return retval;
}

/* TODO: camel_mime_message_set_date(msg, time(0), 930); */

static void 
tny_camel_mime_part_del_part (TnyMimePart *self,  TnyMimePart *part)
{
	TNY_CAMEL_MIME_PART_GET_CLASS (self)->del_part_func (self, part);
	return;
}

static void 
tny_camel_mime_part_del_part_default (TnyMimePart *self, TnyMimePart *part)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);
	CamelDataWrapper *containee;
	CamelMimePart *cpart;

	/* Yes, indeed (I don't yet support non TnyCamelMimePart mime part 
	   instances, and I know I should. Feel free to implement the copying
	   if you really need it) */

	g_assert (TNY_IS_CAMEL_MIME_PART (part));

	g_mutex_lock (priv->part_lock);

	containee = camel_medium_get_content_object (CAMEL_MEDIUM (priv->part));

	if (containee && CAMEL_IS_MULTIPART (containee))
	{
		cpart = tny_camel_mime_part_get_part (TNY_CAMEL_MIME_PART (part));
		camel_multipart_remove_part (CAMEL_MULTIPART (containee), cpart);
		camel_object_unref (CAMEL_OBJECT (cpart));
	}

	g_mutex_unlock (priv->part_lock);

	return;
}



static gboolean 
tny_camel_mime_part_is_attachment (TnyMimePart *self)
{
	return TNY_CAMEL_MIME_PART_GET_CLASS (self)->is_attachment_func (self);
}


static gboolean 
tny_camel_mime_part_is_attachment_default (TnyMimePart *self)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);
	CamelDataWrapper *dw = NULL;
	CamelMedium *medium = (CamelMedium *)priv->part;
	const gchar *contdisp = camel_medium_get_header (medium, "content-disposition");

	/* Content-Disposition is excellent for this, of course (but we might
	 * not actually have this header, as not all E-mail clients add it) */


	if (contdisp)
	{
		if (camel_strstrcase (contdisp, "inline"))
			return FALSE;
		if (camel_strstrcase (contdisp, "attachment"))
			return TRUE;
	}

	/* Check the old fashioned way */
	dw = camel_medium_get_content_object(medium);

	if (dw)
	{
		return !(/*camel_content_type_is (dw->mime_type, "multipart", "*")
			 ||*/ camel_content_type_is(dw->mime_type, "application", "x-pkcs7-mime")
			 || camel_content_type_is(dw->mime_type, "application", "pkcs7-mime")
			 || camel_content_type_is(dw->mime_type, "application", "x-inlinepgp-signed")
			 || camel_content_type_is(dw->mime_type, "application", "x-inlinepgp-encrypted")
			 || ( /* camel_content_type_is (dw->mime_type, "text", "*") text/x-patch 
			     && */ camel_mime_part_get_filename(priv->part) == NULL));
	}

	return FALSE;
}

static void
tny_camel_mime_part_write_to_stream (TnyMimePart *self, TnyStream *stream)
{
	TNY_CAMEL_MIME_PART_GET_CLASS (self)->write_to_stream_func (self, stream);
	return;
}

static void
tny_camel_mime_part_write_to_stream_default (TnyMimePart *self, TnyStream *stream)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);
	CamelDataWrapper *wrapper;
	CamelMedium *medium;
	CamelStream *cstream;

	g_assert (TNY_IS_STREAM (stream));

	cstream = tny_stream_camel_new (stream);

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
tny_camel_mime_part_decode_to_stream (TnyMimePart *self, TnyStream *stream)
{
	TNY_CAMEL_MIME_PART_GET_CLASS (self)->decode_to_stream_func (self, stream);
	return;
}

static void
tny_camel_mime_part_decode_to_stream_default (TnyMimePart *self, TnyStream *stream)
{

	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);
	CamelDataWrapper *wrapper;
	CamelMedium *medium;
	CamelStream *cstream;

	g_assert (TNY_IS_STREAM (stream));

	cstream = tny_stream_camel_new (stream);

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
	camel_object_unref (CAMEL_OBJECT (medium));

	return;
}

static gint
tny_camel_mime_part_construct_from_stream (TnyMimePart *self, TnyStream *stream, const gchar *type)
{
	return TNY_CAMEL_MIME_PART_GET_CLASS (self)->construct_from_stream_func (self, stream, type);
}

static gint
tny_camel_mime_part_construct_from_stream_default (TnyMimePart *self, TnyStream *stream, const gchar *type)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);
	CamelDataWrapper *wrapper;
	gint retval = -1;
	CamelMedium *medium;
	CamelStream *cstream;

	g_assert (TNY_IS_STREAM (stream));

	cstream = tny_stream_camel_new (stream);

	g_mutex_lock (priv->part_lock);
	medium = CAMEL_MEDIUM (priv->part);
	camel_object_ref (CAMEL_OBJECT (medium));
	g_mutex_unlock (priv->part_lock);

	wrapper = camel_medium_get_content_object (medium);

	if (G_LIKELY (wrapper))
		camel_object_unref (CAMEL_OBJECT (wrapper));

	if (!g_ascii_strcasecmp (type, "message/rfc822")) {
		wrapper = (CamelDataWrapper *) camel_mime_message_new ();
	} else {
		wrapper = camel_data_wrapper_new ();
	}
	retval = camel_data_wrapper_construct_from_stream (wrapper, cstream);
	camel_data_wrapper_set_mime_type (wrapper, type);

	camel_medium_set_content_object(medium, wrapper);

	camel_object_unref (CAMEL_OBJECT (cstream));
	camel_object_unref (CAMEL_OBJECT (medium));

	return retval;
}

static TnyStream* 
tny_camel_mime_part_get_stream (TnyMimePart *self)
{
	return TNY_CAMEL_MIME_PART_GET_CLASS (self)->get_stream_func (self);
}

static TnyStream* 
tny_camel_mime_part_get_stream_default (TnyMimePart *self)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);
	TnyStream *retval = NULL;
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

	if (wrapper->stream)
	{
		camel_stream_reset (wrapper->stream);
		camel_stream_write_to_stream (wrapper->stream, stream);
	}

	retval = TNY_STREAM (tny_camel_stream_new (stream));

	/* Parenting: Loose own ref (the tnystreamcamel wrapper keeps one) */
	camel_object_unref (CAMEL_OBJECT (stream));


	tny_stream_reset (retval);

	camel_object_unref (CAMEL_OBJECT (medium));

	return retval;
}

static const gchar* 
tny_camel_mime_part_get_content_type (TnyMimePart *self)
{
	return TNY_CAMEL_MIME_PART_GET_CLASS (self)->get_content_type_func (self);
}

static const gchar* 
tny_camel_mime_part_get_content_type_default (TnyMimePart *self)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);

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
tny_camel_mime_part_is_purged (TnyMimePart *self)
{
	return TNY_CAMEL_MIME_PART_GET_CLASS (self)->is_purged_func (self);
}

static gboolean
tny_camel_mime_part_is_purged_default (TnyMimePart *self)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);
	const gchar *disposition;

	disposition = camel_mime_part_get_disposition (priv->part);
	return (disposition != NULL) && (!strcmp (disposition, "purged"));
}

static gboolean 
tny_camel_mime_part_content_type_is (TnyMimePart *self, const gchar *type)
{
	return TNY_CAMEL_MIME_PART_GET_CLASS (self)->content_type_is_func (self, type);
}

static gboolean 
tny_camel_mime_part_content_type_is_default (TnyMimePart *self, const gchar *type)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);
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
_tny_camel_mime_part_set_part (TnyCamelMimePart *self, CamelMimePart *part)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->part_lock);

	if (G_UNLIKELY (priv->cached_content_type))
		g_free (priv->cached_content_type);
	priv->cached_content_type = NULL;

	if (priv->part)
		camel_object_unref (CAMEL_OBJECT (priv->part));

	camel_object_ref (CAMEL_OBJECT (part));
	priv->part = part;

	g_mutex_unlock (priv->part_lock);

	return;
}

/**
 * tny_camel_mime_part_get_part:
 * @self: The #TnyCamelMimePart instance
 * 
 * Get the #CamelMimePart instance that is being proxied by @self.
 *
 * Return value: The #CamelMimePart instance
 **/
CamelMimePart*
tny_camel_mime_part_get_part (TnyCamelMimePart *self)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);
	CamelMimePart *retval;

	g_mutex_lock (priv->part_lock);
	retval = priv->part;
	if (retval)
		camel_object_ref (CAMEL_OBJECT (retval));
	g_mutex_unlock (priv->part_lock);

	return retval;
}


static const gchar*
tny_camel_mime_part_get_filename (TnyMimePart *self)
{
	return TNY_CAMEL_MIME_PART_GET_CLASS (self)->get_filename_func (self);
}

static const gchar*
tny_camel_mime_part_get_filename_default (TnyMimePart *self)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);
	const gchar *retval;

	g_mutex_lock (priv->part_lock);
	retval = camel_mime_part_get_filename (priv->part);
	g_mutex_unlock (priv->part_lock);
	
	return retval;
}

static const gchar*
tny_camel_mime_part_get_content_id (TnyMimePart *self)
{
	return TNY_CAMEL_MIME_PART_GET_CLASS (self)->get_content_id_func (self);
}

static const gchar*
tny_camel_mime_part_get_content_id_default (TnyMimePart *self)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);
	const gchar *retval;

	g_mutex_lock (priv->part_lock);
	retval = camel_mime_part_get_content_id (priv->part);
	g_mutex_unlock (priv->part_lock);

	return retval;
}

static const gchar*
tny_camel_mime_part_get_description (TnyMimePart *self)
{
	return TNY_CAMEL_MIME_PART_GET_CLASS (self)->get_description_func (self);
}

static const gchar*
tny_camel_mime_part_get_description_default (TnyMimePart *self)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);
	const gchar *retval;

	g_mutex_lock (priv->part_lock);
	retval = camel_mime_part_get_description (priv->part);
	g_mutex_unlock (priv->part_lock);

	return retval;
}

static const gchar*
tny_camel_mime_part_get_content_location (TnyMimePart *self)
{
	return TNY_CAMEL_MIME_PART_GET_CLASS (self)->get_content_location_func (self);
}

static const gchar*
tny_camel_mime_part_get_content_location_default (TnyMimePart *self)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);
	const gchar *retval;

	g_mutex_lock (priv->part_lock);
	retval = camel_mime_part_get_content_location (priv->part);
	g_mutex_unlock (priv->part_lock);

	return retval;
}


static void 
tny_camel_mime_part_set_content_location (TnyMimePart *self, const gchar *content_location)
{
	TNY_CAMEL_MIME_PART_GET_CLASS (self)->set_content_location_func (self, content_location);
	return;
}

static void 
tny_camel_mime_part_set_content_location_default (TnyMimePart *self, const gchar *content_location)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->part_lock);
	camel_mime_part_set_content_location (priv->part, content_location);
	g_mutex_unlock (priv->part_lock);

	return;
}

static void 
tny_camel_mime_part_set_description (TnyMimePart *self, const gchar *description)
{
	TNY_CAMEL_MIME_PART_GET_CLASS (self)->set_description_func (self, description);
	return;
}

static void 
tny_camel_mime_part_set_description_default (TnyMimePart *self, const gchar *description)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->part_lock);
	camel_mime_part_set_description (priv->part, description);
	g_mutex_unlock (priv->part_lock);

	return;
}

static void 
tny_camel_mime_part_set_content_id (TnyMimePart *self, const gchar *content_id)
{
	TNY_CAMEL_MIME_PART_GET_CLASS (self)->set_content_id_func (self, content_id);
	return;
}

static void 
tny_camel_mime_part_set_content_id_default (TnyMimePart *self, const gchar *content_id)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->part_lock);
	camel_mime_part_set_content_id (priv->part, content_id);
	g_mutex_unlock (priv->part_lock);

	return;
}

static void 
tny_camel_mime_part_set_filename (TnyMimePart *self, const gchar *filename)
{
	TNY_CAMEL_MIME_PART_GET_CLASS (self)->set_filename_func (self, filename);
	return;
}

static void 
tny_camel_mime_part_set_filename_default (TnyMimePart *self, const gchar *filename)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->part_lock);
	if (filename) /* Q: Perhaps is this check not needed (and even invalid) */
		camel_mime_part_set_filename (priv->part, filename);
	g_mutex_unlock (priv->part_lock);

	return;
}


static void 
tny_camel_mime_part_set_content_type (TnyMimePart *self, const gchar *content_type)
{
	TNY_CAMEL_MIME_PART_GET_CLASS (self)->set_content_type_func (self, content_type);
	return;
}

static void
tny_camel_mime_part_set_purged_default (TnyMimePart *self)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->part_lock);
	camel_mime_part_set_disposition (priv->part, "purged");
	g_mutex_unlock (priv->part_lock);
}

static void
tny_camel_mime_part_set_purged (TnyMimePart *self)
{
	TNY_CAMEL_MIME_PART_GET_CLASS (self)->set_purged_func (self);
	return;
}

static void 
tny_camel_mime_part_set_content_type_default (TnyMimePart *self, const gchar *content_type)
{
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);

	g_assert (CAMEL_IS_MEDIUM (priv->part));

	g_mutex_lock (priv->part_lock);

	camel_mime_part_set_content_type (priv->part, content_type);

	if (G_UNLIKELY (priv->cached_content_type))
		g_free (priv->cached_content_type);
	priv->cached_content_type = NULL;

	g_mutex_unlock (priv->part_lock);

	return;
}

static void
tny_camel_mime_part_finalize (GObject *object)
{
	TnyCamelMimePart *self = (TnyCamelMimePart*) object;
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);
	
	g_mutex_lock (priv->part_lock);

	if (priv->cached_content_type)
		g_free (priv->cached_content_type);
	priv->cached_content_type = NULL;

	if (G_LIKELY (priv->part) && CAMEL_IS_OBJECT (priv->part))
		camel_object_unref (CAMEL_OBJECT (priv->part));

	g_mutex_unlock (priv->part_lock);

	g_mutex_free (priv->part_lock);
	priv->part_lock = NULL;

	(*parent_class->finalize) (object);

	return;
}


/**
 * tny_camel_mime_part_new:
 * 
 * Create a new MIME part instance
 * 
 * Return value: A new #TnyMimePart instance implemented for Camel
 **/
TnyMimePart*
tny_camel_mime_part_new (void)
{
	TnyCamelMimePart *self = g_object_new (TNY_TYPE_CAMEL_MIME_PART, NULL);
	CamelMimePart *cpart = camel_mime_part_new ();

	_tny_camel_mime_part_set_part (self, cpart);

	return TNY_MIME_PART (self);
}


/**
 * tny_camel_mime_part_new_with_part:
 * @part: a #CamelMimePart object
 * 
 * Create a new MIME part instance that is a proxy for a #CamelMimePart one
 *
 * Return value: A new #TnyMimePart instance implemented for Camel
 **/
TnyMimePart*
tny_camel_mime_part_new_with_part (CamelMimePart *part)
{
	TnyCamelMimePart *self = g_object_new (TNY_TYPE_CAMEL_MIME_PART, NULL);

	_tny_camel_mime_part_set_part (self, part);

	return TNY_MIME_PART (self);
}



static void
tny_mime_part_init (gpointer g, gpointer iface_data)
{
	TnyMimePartIface *klass = (TnyMimePartIface *)g;

	klass->content_type_is_func = tny_camel_mime_part_content_type_is;
	klass->get_content_type_func = tny_camel_mime_part_get_content_type;
	klass->get_stream_func = tny_camel_mime_part_get_stream;
	klass->write_to_stream_func = tny_camel_mime_part_write_to_stream;
	klass->construct_from_stream_func = tny_camel_mime_part_construct_from_stream;
	klass->get_filename_func = tny_camel_mime_part_get_filename;
	klass->get_content_id_func = tny_camel_mime_part_get_content_id;
	klass->get_description_func = tny_camel_mime_part_get_description;
	klass->get_content_location_func = tny_camel_mime_part_get_content_location;
	klass->is_purged_func = tny_camel_mime_part_is_purged;
	klass->set_content_location_func = tny_camel_mime_part_set_content_location;
	klass->set_description_func = tny_camel_mime_part_set_description;
	klass->set_purged_func = tny_camel_mime_part_set_purged;
	klass->set_content_id_func = tny_camel_mime_part_set_content_id;
	klass->set_filename_func = tny_camel_mime_part_set_filename;
	klass->set_content_type_func = tny_camel_mime_part_set_content_type;
	klass->is_attachment_func = tny_camel_mime_part_is_attachment;
	klass->decode_to_stream_func = tny_camel_mime_part_decode_to_stream;
	klass->get_parts_func = tny_camel_mime_part_get_parts;
	klass->add_part_func = tny_camel_mime_part_add_part;
	klass->del_part_func = tny_camel_mime_part_del_part;
	klass->get_header_pairs_func = tny_camel_mime_part_get_header_pairs;
	klass->set_header_pair_func = tny_camel_mime_part_set_header_pair;

	return;
}


static void 
tny_camel_mime_part_class_init (TnyCamelMimePartClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	class->content_type_is_func = tny_camel_mime_part_content_type_is_default;
	class->get_content_type_func = tny_camel_mime_part_get_content_type_default;
	class->get_stream_func = tny_camel_mime_part_get_stream_default;
	class->write_to_stream_func = tny_camel_mime_part_write_to_stream_default;
	class->construct_from_stream_func = tny_camel_mime_part_construct_from_stream_default;
	class->get_filename_func = tny_camel_mime_part_get_filename_default;
	class->get_content_id_func = tny_camel_mime_part_get_content_id_default;
	class->get_description_func = tny_camel_mime_part_get_description_default;
	class->get_content_location_func = tny_camel_mime_part_get_content_location_default;
	class->is_purged_func = tny_camel_mime_part_is_purged_default;
	class->set_purged_func = tny_camel_mime_part_set_purged_default;
	class->set_content_location_func = tny_camel_mime_part_set_content_location_default;
	class->set_description_func = tny_camel_mime_part_set_description_default;
	class->set_content_id_func = tny_camel_mime_part_set_content_id_default;
	class->set_filename_func = tny_camel_mime_part_set_filename_default;
	class->set_content_type_func = tny_camel_mime_part_set_content_type_default;
	class->is_attachment_func = tny_camel_mime_part_is_attachment_default;
	class->decode_to_stream_func = tny_camel_mime_part_decode_to_stream_default;
	class->get_parts_func = tny_camel_mime_part_get_parts_default;
	class->add_part_func = tny_camel_mime_part_add_part_default;
	class->del_part_func = tny_camel_mime_part_del_part_default;
	class->get_header_pairs_func = tny_camel_mime_part_get_header_pairs_default;
	class->set_header_pair_func = tny_camel_mime_part_set_header_pair_default;

	object_class->finalize = tny_camel_mime_part_finalize;

	g_type_class_add_private (object_class, sizeof (TnyCamelMimePartPriv));

	return;
}

static void
tny_camel_mime_part_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyCamelMimePart *self = (TnyCamelMimePart*)instance;
	TnyCamelMimePartPriv *priv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);

	priv->part_lock = g_mutex_new ();

	return;
}

/**
 * tny_camel_mime_part_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
GType 
tny_camel_mime_part_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY (!_camel_type_init_done))
	{
		if (!g_thread_supported ()) 
			g_thread_init (NULL);

		camel_type_init ();
		_camel_type_init_done = TRUE;
	}

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyCamelMimePartClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_camel_mime_part_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyCamelMimePart),
		  0,      /* n_preallocs */
		  tny_camel_mime_part_instance_init,    /* instance_init */
		  NULL
		};

		static const GInterfaceInfo tny_mime_part_info = 
		{
		  (GInterfaceInitFunc) tny_mime_part_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyCamelMimePart",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_MIME_PART, 
			&tny_mime_part_info);
	}

	return type;
}

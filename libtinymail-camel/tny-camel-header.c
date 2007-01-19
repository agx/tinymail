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

#include <glib.h>
#include <string.h>

#include <tny-header.h>
#include <tny-camel-header.h>
#include <tny-camel-folder.h>

#include "tny-camel-common-priv.h"
#include "tny-camel-folder-priv.h"
#include "tny-camel-header-priv.h"

#include <tny-camel-shared.h>

#include <camel/camel-folder.h>
#include <camel/camel.h>
#include <camel/camel-folder-summary.h>
#include <camel/camel-stream-null.h>

static GObjectClass *parent_class = NULL;

static const gchar *invalid = "Invalid";

static void 
destroy_write (TnyCamelHeader *self)
{
	/* Also check out tny-msg.c: tny_msg_finalize (read the stupid hack) */
	if (((WriteInfo*)self->info)->msg)
		camel_object_unref (CAMEL_OBJECT (((WriteInfo*)self->info)->msg));

	if (((WriteInfo*)self->info)->mime_from && ((WriteInfo*)self->info)->mime_from != invalid)
		g_free (((WriteInfo*)self->info)->mime_from);

	g_slice_free (WriteInfo, self->info);
	self->info = NULL;
	self->write = 3;
}

static void 
destroy_mem (TnyCamelHeader *self)
{
	if (((MemInfo*)self->info)->uid)
		g_free (((MemInfo*)self->info)->uid);
	if (((MemInfo*)self->info)->bcc)
		g_free (((MemInfo*)self->info)->bcc);
	if (((MemInfo*)self->info)->cc)
		g_free (((MemInfo*)self->info)->cc);
	if (((MemInfo*)self->info)->message_id)
		g_free (((MemInfo*)self->info)->message_id);
	if (((MemInfo*)self->info)->from)
		g_free (((MemInfo*)self->info)->from);
	if (((MemInfo*)self->info)->to)
		g_free (((MemInfo*)self->info)->to);
	if (((MemInfo*)self->info)->subject)
		g_free (((MemInfo*)self->info)->subject);
	if (((MemInfo*)self->info)->replyto)
		g_free (((MemInfo*)self->info)->replyto);

	g_slice_free (MemInfo, self->info);
	self->info = NULL;
	self->write = 3;
}

static void
prepare_for_mem (TnyCamelHeader *self, MemInfo *info)
{
	if (self->write != 2)
	{
		if (info == NULL)
		{
			self->info = g_slice_new0 (MemInfo);
			((MemInfo*)self->info)->uid = NULL;
			((MemInfo*)self->info)->bcc = NULL;
			((MemInfo*)self->info)->cc = NULL;
			((MemInfo*)self->info)->message_id = NULL;
			((MemInfo*)self->info)->from = NULL;
			((MemInfo*)self->info)->to = NULL;
			((MemInfo*)self->info)->subject = NULL;
			((MemInfo*)self->info)->replyto = NULL;
		} else 
			self->info = info;

		self->write = 2;
#ifdef HEALTHY_CHECK
		self->healthy = 1;
#endif
	}

	return;
}


static void
prepare_for_write (TnyCamelHeader *self)
{
	if (self->write != 1)
	{
		self->info = g_slice_new0 (WriteInfo);
		((WriteInfo*)self->info)->msg = camel_mime_message_new ();
		((WriteInfo*)self->info)->mime_from = NULL;
		self->write = 1;
#ifdef HEALTHY_CHECK
		self->healthy = 1;
#endif
	}

	return;
}

void 
_tny_camel_header_set_camel_message_info (TnyCamelHeader *self, CamelMessageInfo *camel_message_info, gboolean knowit)
{
	if (!knowit && G_UNLIKELY (self->info))
		g_warning ("Strange behaviour: Overwriting existing message info");

	if (self->write == 1)
		destroy_write (self);
	else if (self->write == 2)
		destroy_mem (self);

	self->info = camel_message_info;
	self->write = 0;
#ifdef HEALTHY_CHECK
	self->healthy = 1;
#endif

	return;
}

void 
_tny_camel_header_set_as_memory (TnyCamelHeader *self, MemInfo *info)
{
	if (G_UNLIKELY (self->info))
		g_warning ("Strange behaviour: Overwriting existing message info");

	if (self->write == 1)
		destroy_write (self);
	else if (self->write == 2)
		destroy_mem (self);

	self->write = 3;
	self->info = NULL;
	prepare_for_mem (self, info);

	return;
}


void 
_tny_camel_header_set_camel_mime_message (TnyCamelHeader *self, CamelMimeMessage *camel_mime_message)
{
	if (G_UNLIKELY (self->info))
		g_warning ("Strange behaviour: Overwriting existing message info");

	if (self->write == 1)
		destroy_write (self);
	else if (self->write == 2)
		destroy_mem (self);

	self->info = g_slice_new0 (WriteInfo);
	((WriteInfo*)self->info)->mime_from = NULL;
	self->write = 1;
#ifdef HEALTHY_CHECK
		self->healthy = 1;
#endif

	((WriteInfo*)self->info)->msg = camel_mime_message;
	camel_object_ref (CAMEL_OBJECT (camel_mime_message));

	return;
}
static const gchar*
tny_camel_header_get_replyto (TnyHeader *self)
{
	TnyCamelHeader *me = TNY_CAMEL_HEADER (self);
	const gchar *retval;

#ifdef HEALTHY_CHECK
	if (!me->healthy || G_UNLIKELY (!me->info))
		return invalid;
#else
	if (G_UNLIKELY (!me->info))
		return invalid;
#endif

	if (G_UNLIKELY (me->write == 1)) 
	{
		CamelInternetAddress *addr = (CamelInternetAddress*)
			camel_mime_message_get_reply_to (((WriteInfo*)me->info)->msg);
		if (addr)
			retval = camel_address_format (CAMEL_ADDRESS (addr));
		else
			retval = invalid;
	} else if (G_UNLIKELY (me->write == 2))
		return ((MemInfo *)me->info)->replyto;
	else
		retval = invalid;

	return retval;
}


static void
tny_camel_header_set_bcc (TnyHeader *self, const gchar *bcc)
{
	TnyCamelHeader *me;
	CamelInternetAddress *addr;

	me = TNY_CAMEL_HEADER (self);
	addr = camel_internet_address_new ();

	_foreach_email_add_to_inet_addr (bcc, addr);

	prepare_for_write (me);

	camel_mime_message_set_recipients (((WriteInfo*)me->info)->msg, 
		CAMEL_RECIPIENT_TYPE_BCC, addr);

	camel_object_unref (CAMEL_OBJECT (addr));

	return;
}

static void
tny_camel_header_set_cc (TnyHeader *self, const gchar *cc)
{
	TnyCamelHeader *me = TNY_CAMEL_HEADER (self);    
	CamelInternetAddress *addr = camel_internet_address_new ();

	_foreach_email_add_to_inet_addr (cc, addr);

	prepare_for_write (me);

	camel_mime_message_set_recipients (((WriteInfo*)me->info)->msg, 
		CAMEL_RECIPIENT_TYPE_CC, addr);
	camel_object_unref (CAMEL_OBJECT (addr));

	return;
}

static void
tny_camel_header_set_from (TnyHeader *self, const gchar *from)
{
	TnyCamelHeader *me = TNY_CAMEL_HEADER (self);    
	CamelInternetAddress *addr = camel_internet_address_new ();
	gchar *dup;

	dup = g_strdup (from);
	_string_to_camel_inet_addr (dup, addr);
	g_free (dup);

	prepare_for_write (me);

	camel_mime_message_set_from (((WriteInfo*)me->info)->msg, addr);
	camel_object_unref (CAMEL_OBJECT (addr));

	return;
}

static void
tny_camel_header_set_subject (TnyHeader *self, const gchar *subject)
{
	TnyCamelHeader *me = TNY_CAMEL_HEADER (self);    
	prepare_for_write (me);
	camel_mime_message_set_subject (((WriteInfo*)me->info)->msg, subject);

	return;
}

static void
tny_camel_header_set_to (TnyHeader *self, const gchar *to)
{
	TnyCamelHeader *me = TNY_CAMEL_HEADER (self);    
	CamelInternetAddress *addr = camel_internet_address_new ();
	gchar *dup;

	dup = g_strdup (to);
	_foreach_email_add_to_inet_addr (dup, addr);
	g_free (dup);

	prepare_for_write (me);

	camel_mime_message_set_recipients (((WriteInfo*)me->info)->msg, 
		CAMEL_RECIPIENT_TYPE_TO, addr);

	camel_object_unref (CAMEL_OBJECT (addr));

	return;
}


static void
tny_camel_header_set_replyto (TnyHeader *self, const gchar *replyto)
{
	TnyCamelHeader *me = TNY_CAMEL_HEADER (self);    
	CamelInternetAddress *addr = camel_internet_address_new ();
	gchar *dup;

	dup = g_strdup (replyto);
	_foreach_email_add_to_inet_addr (dup, addr);
	g_free (dup);

	prepare_for_write (me);

	camel_mime_message_set_reply_to (((WriteInfo*)me->info)->msg, addr);

	camel_object_unref (CAMEL_OBJECT (addr));

	return;
}


static const gchar*
tny_camel_header_get_cc (TnyHeader *self)
{
	TnyCamelHeader *me = TNY_CAMEL_HEADER (self);
	const gchar *retval;

#ifdef HEALTHY_CHECK
	if (!me->healthy)
		return invalid;
#endif

	if (G_UNLIKELY (!me->info))
		return invalid;

	if (G_UNLIKELY (me->write == 1))
		retval = camel_medium_get_header (CAMEL_MEDIUM (((WriteInfo*)me->info)->msg), "cc");
	else if (G_UNLIKELY (me->write == 2))
		retval = ((MemInfo *)me->info)->cc;
	else
		retval = camel_message_info_cc ((CamelMessageInfo*)me->info);

	return retval;
}

static const gchar*
tny_camel_header_get_bcc (TnyHeader *self)
{
	TnyCamelHeader *me = TNY_CAMEL_HEADER (self);
	const gchar *retval;

#ifdef HEALTHY_CHECK
	if (!me->healthy)
		return invalid;
#endif

	if (G_UNLIKELY (!me->info))
		return invalid;

	if (G_UNLIKELY (me->write == 1))
		retval = camel_medium_get_header (CAMEL_MEDIUM (((WriteInfo*)me->info)->msg), "bcc");
	else if (G_UNLIKELY (me->write == 2))
		return ((MemInfo *)me->info)->bcc;
	else
		retval = invalid;

	return retval;
}

static TnyHeaderFlags
tny_camel_header_get_flags (TnyHeader *self)
{
	TnyCamelHeader *me = TNY_CAMEL_HEADER (self);
	TnyHeaderFlags retval = 0;

#ifdef HEALTHY_CHECK
	if (!me->healthy)
		return 0;
#endif

	if (G_UNLIKELY (me->write == 1))
	{
		g_warning ("tny_camel_header_get_flags: This is a header for a new message!\n");
		return retval;
	} else if (G_UNLIKELY (me->write == 2))
		return ((MemInfo *)me->info)->flags;

	/* This is only legal because the flags between CamelLite and Tinymail are equalized */
	retval = camel_message_info_flags ((CamelMessageInfo*)me->info);

	return retval;
}

static void
tny_camel_header_set_flags (TnyHeader *self, TnyHeaderFlags mask)
{
	TnyCamelHeader *me = TNY_CAMEL_HEADER (self);

	if (me->write != 0)
	{
		g_warning ("tny_camel_header_get_flags: This is a header for a new message!\n");
		return;
	} 

	/* This is only legal because the flags between CamelLite and Tinymail are equalized */
	camel_message_info_set_flags ((CamelMessageInfo*)me->info, mask, ~0);

	return;
}

static void
tny_camel_header_unset_flags (TnyHeader *self, TnyHeaderFlags mask)
{
	TnyCamelHeader *me = TNY_CAMEL_HEADER (self);

#ifdef HEALTHY_CHECK
	if (!me->healthy)
		return;
#endif

	if (me->write != 0)
	{
		g_warning ("tny_camel_header_get_flags: This is a header for a new message!\n");
		return;
	}

	camel_message_info_set_flags ((CamelMessageInfo*)me->info, mask, 0);

	return;
}

static time_t
tny_camel_header_get_date_received (TnyHeader *self)
{
	TnyCamelHeader *me = TNY_CAMEL_HEADER (self);
	time_t retval = 0;

#ifdef HEALTHY_CHECK
	if (!me->healthy || G_UNLIKELY (!me->info))
		return retval;
#else
	if (G_UNLIKELY (!me->info))
		return retval;
#endif

	if (G_UNLIKELY (me->write == 1))
	{
		int tzone;

		retval = camel_mime_message_get_date (((WriteInfo*)me->info)->msg, &tzone);

		if (retval == CAMEL_MESSAGE_DATE_CURRENT)
			retval = camel_mime_message_get_date_received (((WriteInfo*)me->info)->msg, &tzone);
		if (retval == CAMEL_MESSAGE_DATE_CURRENT)
		{
			time (&retval);
			tzone = 0;
		}

		retval += (tzone / 100) * 60 * 60;
		retval += (tzone % 100) * 60;
	}
	else if (G_UNLIKELY (me->write == 2))
		retval = ((MemInfo *)me->info)->date_received;
	else
		retval = camel_message_info_date_received ((CamelMessageInfo*)me->info);

	return retval;
}

static time_t
tny_camel_header_get_date_sent (TnyHeader *self)
{
	TnyCamelHeader *me = TNY_CAMEL_HEADER (self);
	time_t retval = 0;

#ifdef HEALTHY_CHECK
	if (!me->healthy || G_UNLIKELY (!me->info))
		return retval;
#else
	if (G_UNLIKELY (!me->info))
		return retval;
#endif

	if (G_UNLIKELY (me->write == 1))
	{
		int tzone;
		retval = camel_mime_message_get_date (((WriteInfo*)me->info)->msg, &tzone);

		if (retval == CAMEL_MESSAGE_DATE_CURRENT)
			retval = camel_mime_message_get_date_received (((WriteInfo*)me->info)->msg, &tzone);
		if (retval == CAMEL_MESSAGE_DATE_CURRENT)
		{
			time (&retval);
			tzone = 0;
		}

		retval += (tzone / 100) * 60 * 60;
		retval += (tzone % 100) * 60;
	}
	else if (G_UNLIKELY (me->write == 2))
		retval = ((MemInfo *)me->info)->date_sent;
	else
		retval = camel_message_info_date_sent ((CamelMessageInfo*)me->info);

	return retval;
}
	
static const gchar*
tny_camel_header_get_from (TnyHeader *self)
{
	TnyCamelHeader *me = TNY_CAMEL_HEADER (self);
	const gchar *retval;

#ifdef HEALTHY_CHECK
	if (!me->healthy || G_UNLIKELY (!me->info))
		return invalid;
#else
	if (G_UNLIKELY (!me->info))
		return invalid;
#endif

	if (G_UNLIKELY (me->write == 1))
	{
		if (G_LIKELY (!((WriteInfo*)me->info)->mime_from))
		{
			CamelInternetAddress *addr = (CamelInternetAddress*)
				camel_mime_message_get_from (((WriteInfo*)me->info)->msg);
				if (addr)
					((WriteInfo*)me->info)->mime_from = camel_address_format (CAMEL_ADDRESS (addr));
				else ((WriteInfo*)me->info)->mime_from = (gchar*) invalid;
		}

		retval = (const gchar*)((WriteInfo*)me->info)->mime_from;
	}
	else if (G_UNLIKELY (me->write == 2))
		retval = ((MemInfo *)me->info)->from;
	else
		retval = camel_message_info_from ((CamelMimeMessage*)me->info);

	return retval;
}

static const gchar*
tny_camel_header_get_subject (TnyHeader *self)
{
	TnyCamelHeader *me = TNY_CAMEL_HEADER (self);
	const gchar *retval;

#ifdef HEALTHY_CHECK
	if (!me->healthy || G_UNLIKELY (!me->info))
		return invalid;
#else
	if (G_UNLIKELY (!me->info))
		return invalid;
#endif

	if (G_UNLIKELY (me->write == 1))
		retval = camel_mime_message_get_subject (((WriteInfo*)me->info)->msg);
	else if (G_UNLIKELY (me->write == 2))
		retval = ((MemInfo *)me->info)->subject;
	else
		retval = camel_message_info_subject ((CamelMessageInfo*)me->info);

	return retval;
}


static const gchar*
tny_camel_header_get_to (TnyHeader *self)
{
	TnyCamelHeader *me = TNY_CAMEL_HEADER (self);
	gchar *retval;

#ifdef HEALTHY_CHECK
	if (!me->healthy || G_UNLIKELY (!me->info))
		return invalid;
#else
	if (G_UNLIKELY (!me->info))
		return invalid;
#endif

	if (G_UNLIKELY (me->write == 1))
		retval = (gchar*) camel_medium_get_header (CAMEL_MEDIUM (((WriteInfo*)me->info)->msg), "to");
	else if (G_UNLIKELY (me->write == 2))
		retval = ((MemInfo *)me->info)->to;
	else
		retval = (gchar*) camel_message_info_to ((CamelMessageInfo*)me->info);

	return (const gchar*)retval;
}

static const gchar*
tny_camel_header_get_message_id (TnyHeader *self)
{
	TnyCamelHeader *me = TNY_CAMEL_HEADER (self);
	gchar *retval;

#ifdef HEALTHY_CHECK
	if (!me->healthy || G_UNLIKELY (!me->info))
		return invalid;
#else
	if (G_UNLIKELY (!me->info))
		return invalid;
#endif

	if (G_UNLIKELY (me->write == 1))
		retval = (gchar*) camel_mime_message_get_message_id (((WriteInfo*)me->info)->msg);
	else if (G_UNLIKELY (me->write == 2))
		retval = ((MemInfo *)me->info)->message_id;
	else
		retval = (gchar*) camel_message_info_message_id ((CamelMessageInfo*)me->info);

	return (const gchar*)retval;


}



static guint
tny_camel_header_get_message_size (TnyHeader *self)
{
	TnyCamelHeader *me = TNY_CAMEL_HEADER (self);
	guint retval;

#ifdef HEALTHY_CHECK
	if (!me->healthy || G_UNLIKELY (!me->info))
		return 0;
#else
	if (G_UNLIKELY (!me->info))
		return 0;
#endif

	if (G_UNLIKELY (me->write == 1))
	{
		CamelStreamNull *sn = (CamelStreamNull *)camel_stream_null_new();
		camel_data_wrapper_write_to_stream((CamelDataWrapper *)((WriteInfo*)me->info)->msg, (CamelStream *)sn);
		retval = (guint) sn->written;
		camel_object_unref((CamelObject *)sn);
	}
	else if (G_UNLIKELY (me->write == 2))
		retval = ((MemInfo *)me->info)->message_size;
	 else
		retval = (guint) camel_message_info_size ((CamelMessageInfo*)me->info);

	return retval;


}

static const gchar*
tny_camel_header_get_uid (TnyHeader *self)
{
	TnyCamelHeader *me = TNY_CAMEL_HEADER (self);
	const gchar *retval;

#ifdef HEALTHY_CHECK
	if (!me->healthy || G_UNLIKELY (!me->info) || G_UNLIKELY (me->write == 1))
		return invalid;
#else
	if (G_UNLIKELY (!me->info) || G_UNLIKELY (me->write == 1))
		return invalid;
	else if (G_UNLIKELY (me->write == 2))
		return ((MemInfo *)me->info)->uid;
#endif

	retval = camel_message_info_uid ((CamelMessageInfo*)me->info);

	return retval;
}

static void
tny_camel_header_finalize (GObject *object)
{
	TnyCamelHeader *self = (TnyCamelHeader*) object;

#ifdef HEALTHY_CHECK
	self->healthy = 0;
#endif

	if (G_UNLIKELY (self->write == 1))
		destroy_write (self);
	else if (G_UNLIKELY (self->write == 2))
		destroy_mem (self);

	if (self->folder)
	{
		TnyCamelFolderPriv *fpriv = TNY_CAMEL_FOLDER_GET_PRIVATE (self->folder);
		fpriv->headers_managed--;
		_tny_camel_folder_check_uncache (((TnyCamelFolder*)self->folder), fpriv);
	}

	/* Normally we do camel_folder_free_message_info here, but we already got
	rid of our initial reference at tny-folder.c:add_message_with_uid
	I know this is actually ugly */

	(*parent_class->finalize) (object);

	return;
}

/**
 * tny_camel_header_new:
 *
 *
 * Return value: A new #TnyHeader instance implemented for Camel
 **/
TnyHeader*
tny_camel_header_new (void)
{
	TnyCamelHeader *self = g_object_new (TNY_TYPE_CAMEL_HEADER, NULL);
	
	self->info = NULL;
	self->write = 3;

#ifdef HEALTHY_CHECK
	self->healthy = 0;
#endif

	return (TnyHeader*)self;
}

void
_tny_camel_header_set_folder (TnyCamelHeader *self, TnyCamelFolder *folder, TnyCamelFolderPriv *fpriv)
{
	TnyCamelHeader *me = TNY_CAMEL_HEADER (self);
	fpriv->headers_managed++;
	me->folder = (TnyCamelFolder*)folder;

	return;
}

static TnyFolder*
tny_camel_header_get_folder (TnyHeader *self)
{
	TnyCamelHeader *me = TNY_CAMEL_HEADER (self);

	if (me->folder)
		g_object_ref (G_OBJECT (me->folder));

	return (TnyFolder*)me->folder;
}

static void
tny_header_init (gpointer g, gpointer iface_data)
{
	TnyHeaderIface *klass = (TnyHeaderIface *)g;

	klass->get_from_func = tny_camel_header_get_from;
	klass->get_message_id_func = tny_camel_header_get_message_id;
	klass->get_message_size_func = tny_camel_header_get_message_size;
	klass->get_to_func = tny_camel_header_get_to;
	klass->get_subject_func = tny_camel_header_get_subject;
	klass->get_date_received_func = tny_camel_header_get_date_received;
	klass->get_date_sent_func = tny_camel_header_get_date_sent;
	klass->get_cc_func = tny_camel_header_get_cc;
	klass->get_bcc_func = tny_camel_header_get_bcc;
	klass->get_replyto_func = tny_camel_header_get_replyto;
	klass->get_uid_func = tny_camel_header_get_uid;
	klass->get_folder_func = tny_camel_header_get_folder;
	klass->set_bcc_func = tny_camel_header_set_bcc;
	klass->set_cc_func = tny_camel_header_set_cc;
	klass->set_to_func = tny_camel_header_set_to;
	klass->set_from_func = tny_camel_header_set_from;
	klass->set_subject_func = tny_camel_header_set_subject;
	klass->set_replyto_func = tny_camel_header_set_replyto;
	klass->set_flags_func = tny_camel_header_set_flags;
	klass->unset_flags_func = tny_camel_header_unset_flags;
	klass->get_flags_func = tny_camel_header_get_flags;

	return;
}


static void 
tny_camel_header_class_init (TnyCamelHeaderClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_camel_header_finalize;

	return;
}


GType 
tny_camel_header_get_type (void)
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
		  sizeof (TnyCamelHeaderClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_camel_header_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyCamelHeader),
		  0,      /* n_preallocs */
		  NULL,    /* instance_init */
		  NULL
		};

		static const GInterfaceInfo tny_header_info = 
		{
		  (GInterfaceInitFunc) tny_header_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyCamelHeader",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_HEADER, 
			&tny_header_info);
	}

	return type;
}

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
#include "tny-camel-msg-header-priv.h"

#include <tny-camel-shared.h>

#include <camel/camel-folder.h>
#include <camel/camel.h>
#include <camel/camel-folder-summary.h>
#include <camel/camel-stream-null.h>

static GObjectClass *parent_class = NULL;


static const gchar*
tny_camel_msg_header_get_replyto (TnyHeader *self)
{
	TnyCamelMsgHeader *me = TNY_CAMEL_MSG_HEADER (self);
	const gchar *retval = NULL;

	CamelInternetAddress *addr = (CamelInternetAddress*)
		camel_mime_message_get_reply_to (me->msg);
	if (addr)
		retval = camel_address_format (CAMEL_ADDRESS (addr));

	return retval;
}


static void
tny_camel_msg_header_set_bcc (TnyHeader *self, const gchar *bcc)
{
	TnyCamelMsgHeader *me;
	CamelInternetAddress *addr;

	me = TNY_CAMEL_MSG_HEADER (self);
	addr = camel_internet_address_new ();

	_foreach_email_add_to_inet_addr (bcc, addr);

	camel_mime_message_set_recipients (me->msg, 
		CAMEL_RECIPIENT_TYPE_BCC, addr);

	camel_object_unref (CAMEL_OBJECT (addr));

	return;
}

static void
tny_camel_msg_header_set_cc (TnyHeader *self, const gchar *cc)
{
	TnyCamelMsgHeader *me = TNY_CAMEL_MSG_HEADER (self);    
	CamelInternetAddress *addr = camel_internet_address_new ();

	_foreach_email_add_to_inet_addr (cc, addr);

	camel_mime_message_set_recipients (me->msg, 
		CAMEL_RECIPIENT_TYPE_CC, addr);

	camel_object_unref (CAMEL_OBJECT (addr));

	return;
}

static void
tny_camel_msg_header_set_from (TnyHeader *self, const gchar *from)
{
	TnyCamelMsgHeader *me = TNY_CAMEL_MSG_HEADER (self);    
	CamelInternetAddress *addr = camel_internet_address_new ();
	gchar *dup;

	dup = g_strdup (from);
	_string_to_camel_inet_addr (dup, addr);
	g_free (dup);

	camel_mime_message_set_from (me->msg, addr);
	camel_object_unref (CAMEL_OBJECT (addr));

	return;
}

static void
tny_camel_msg_header_set_subject (TnyHeader *self, const gchar *subject)
{
	TnyCamelMsgHeader *me = TNY_CAMEL_MSG_HEADER (self);    

	camel_mime_message_set_subject (me->msg, subject);

	return;
}

static void
tny_camel_msg_header_set_to (TnyHeader *self, const gchar *to)
{
	TnyCamelMsgHeader *me = TNY_CAMEL_MSG_HEADER (self);    
	CamelInternetAddress *addr = camel_internet_address_new ();
	gchar *dup;

	dup = g_strdup (to);
	_foreach_email_add_to_inet_addr (dup, addr);
	g_free (dup);

	camel_mime_message_set_recipients (me->msg, 
		CAMEL_RECIPIENT_TYPE_TO, addr);

	camel_object_unref (CAMEL_OBJECT (addr));

	return;
}


static void
tny_camel_msg_header_set_replyto (TnyHeader *self, const gchar *replyto)
{
	TnyCamelMsgHeader *me = TNY_CAMEL_MSG_HEADER (self);    
	CamelInternetAddress *addr = camel_internet_address_new ();
	gchar *dup;

	dup = g_strdup (replyto);
	_foreach_email_add_to_inet_addr (dup, addr);
	g_free (dup);

	camel_mime_message_set_reply_to (me->msg, addr);

	camel_object_unref (CAMEL_OBJECT (addr));

	return;
}


static const gchar*
tny_camel_msg_header_get_cc (TnyHeader *self)
{
	TnyCamelMsgHeader *me = TNY_CAMEL_MSG_HEADER (self);
	const gchar *retval = NULL;

	retval = camel_medium_get_header (CAMEL_MEDIUM (me->msg), "cc");

	return retval;
}

static const gchar*
tny_camel_msg_header_get_bcc (TnyHeader *self)
{
	TnyCamelMsgHeader *me = TNY_CAMEL_MSG_HEADER (self);
	const gchar *retval = NULL;

	retval = camel_medium_get_header (CAMEL_MEDIUM (me->msg), "bcc");

	return retval;
}

static TnyHeaderFlags
tny_camel_msg_header_get_flags (TnyHeader *self)
{
	g_warning ("tny_header_get_flags: This is a header instance for a new message. It has no flags.\n");
	return 0;
}

static void
tny_camel_msg_header_set_flags (TnyHeader *self, TnyHeaderFlags mask)
{
	g_warning ("tny_header_set_flags: This is a header instance for a new message. It has no flags.\n");
	return;
}

static void
tny_camel_msg_header_unset_flags (TnyHeader *self, TnyHeaderFlags mask)
{
	g_warning ("tny_header_unset_flags: This is a header instance for a new message. It has no flags.\n");
	return;
}

static time_t
tny_camel_msg_header_get_date_received (TnyHeader *self)
{
	TnyCamelMsgHeader *me = TNY_CAMEL_MSG_HEADER (self);
	time_t retval = 0;
	int tzone;

	retval = camel_mime_message_get_date (me->msg, &tzone);

	if (retval == CAMEL_MESSAGE_DATE_CURRENT)
		retval = camel_mime_message_get_date_received (me->msg, &tzone);
	if (retval == CAMEL_MESSAGE_DATE_CURRENT)
	{
		time (&retval);
		tzone = 0;
	}

	retval += (tzone / 100) * 60 * 60;
	retval += (tzone % 100) * 60;

	return retval;
}

static time_t
tny_camel_msg_header_get_date_sent (TnyHeader *self)
{
	TnyCamelMsgHeader *me = TNY_CAMEL_MSG_HEADER (self);
	time_t retval = 0;
	int tzone;

	retval = camel_mime_message_get_date (me->msg, &tzone);

	if (retval == CAMEL_MESSAGE_DATE_CURRENT)
		retval = camel_mime_message_get_date_received (me->msg, &tzone);
	if (retval == CAMEL_MESSAGE_DATE_CURRENT)
	{
		time (&retval);
		tzone = 0;
	}

	retval += (tzone / 100) * 60 * 60;
	retval += (tzone % 100) * 60;

	return retval;
}

static const gchar*
tny_camel_msg_header_get_from (TnyHeader *self)
{
	TnyCamelMsgHeader *me = TNY_CAMEL_MSG_HEADER (self);
	const gchar *retval = NULL;

	if (G_LIKELY (!me->mime_from))
	{
		CamelInternetAddress *addr = (CamelInternetAddress*)
			camel_mime_message_get_from (me->msg);
		if (addr)
			me->mime_from = camel_address_format (CAMEL_ADDRESS (addr));
		else me->mime_from = NULL;
	}

	retval = (const gchar*)me->mime_from;

	return retval;
}

static const gchar*
tny_camel_msg_header_get_subject (TnyHeader *self)
{
	TnyCamelMsgHeader *me = TNY_CAMEL_MSG_HEADER (self);
	const gchar *retval = NULL;

	retval = camel_mime_message_get_subject (me->msg);

	return retval;
}


static const gchar*
tny_camel_msg_header_get_to (TnyHeader *self)
{
	TnyCamelMsgHeader *me = TNY_CAMEL_MSG_HEADER (self);
	const gchar *retval = NULL;

	retval = camel_medium_get_header (CAMEL_MEDIUM (me->msg), "to");

	return retval;
}

static const gchar*
tny_camel_msg_header_get_message_id (TnyHeader *self)
{
	TnyCamelMsgHeader *me = TNY_CAMEL_MSG_HEADER (self);
	const gchar *retval = NULL;

	retval = camel_mime_message_get_message_id (me->msg);

	return retval;
}



static guint
tny_camel_msg_header_get_message_size (TnyHeader *self)
{
	TnyCamelMsgHeader *me = TNY_CAMEL_MSG_HEADER (self);
	guint retval;
	CamelStreamNull *sn = (CamelStreamNull *)camel_stream_null_new();

	camel_data_wrapper_write_to_stream((CamelDataWrapper *)me->msg, (CamelStream *)sn);

	retval = (guint) sn->written;
	camel_object_unref((CamelObject *)sn);

	return retval;
}

static const gchar*
tny_camel_msg_header_get_uid (TnyHeader *self)
{
	TnyCamelMsgHeader *me = TNY_CAMEL_MSG_HEADER (self);

	if (!me->old_uid)
	{
		g_warning ("tny_header_get_uid: This is a header instance for a new message. "
			"The uid of it is therefore not available. This indicates a problem "
			"in the software.");
	}

	return me->old_uid;
}

static void
tny_camel_msg_header_finalize (GObject *object)
{
	TnyCamelMsgHeader *me = (TnyCamelMsgHeader *) object;

	if (me->old_uid)
		g_free (me->old_uid);

	(*parent_class->finalize) (object);

	return;
}

static TnyFolder*
tny_camel_msg_header_get_folder (TnyHeader *self)
{
	TnyCamelMsgHeader *me = TNY_CAMEL_MSG_HEADER (self);

	if (me->folder)
		g_object_ref (G_OBJECT (me->folder));

	return (TnyFolder*) me->folder;
}

TnyHeader*
_tny_camel_msg_header_new (CamelMimeMessage *msg, TnyFolder *folder)
{
	TnyCamelMsgHeader *self = g_object_new (TNY_TYPE_CAMEL_MSG_HEADER, NULL);

	/*  For this implementation of TnyHeader: self dies when the TnyMsg dies who
		owns this msg. If this ever changes then we need to add a reference here, 
		and remove it in the finalize. Same for folder. */

	self->old_uid = NULL;
	self->msg = msg; 
	self->folder = folder;

	return (TnyHeader*) self;
}



static void
tny_header_init (gpointer g, gpointer iface_data)
{
	TnyHeaderIface *klass = (TnyHeaderIface *)g;

	klass->get_from_func = tny_camel_msg_header_get_from;
	klass->get_message_id_func = tny_camel_msg_header_get_message_id;
	klass->get_message_size_func = tny_camel_msg_header_get_message_size;
	klass->get_to_func = tny_camel_msg_header_get_to;
	klass->get_subject_func = tny_camel_msg_header_get_subject;
	klass->get_date_received_func = tny_camel_msg_header_get_date_received;
	klass->get_date_sent_func = tny_camel_msg_header_get_date_sent;
	klass->get_cc_func = tny_camel_msg_header_get_cc;
	klass->get_bcc_func = tny_camel_msg_header_get_bcc;
	klass->get_replyto_func = tny_camel_msg_header_get_replyto;
	klass->get_uid_func = tny_camel_msg_header_get_uid;
	klass->get_folder_func = tny_camel_msg_header_get_folder;
	klass->set_bcc_func = tny_camel_msg_header_set_bcc;
	klass->set_cc_func = tny_camel_msg_header_set_cc;
	klass->set_to_func = tny_camel_msg_header_set_to;
	klass->set_from_func = tny_camel_msg_header_set_from;
	klass->set_subject_func = tny_camel_msg_header_set_subject;
	klass->set_replyto_func = tny_camel_msg_header_set_replyto;
	klass->set_flags_func = tny_camel_msg_header_set_flags;
	klass->unset_flags_func = tny_camel_msg_header_unset_flags;
	klass->get_flags_func = tny_camel_msg_header_get_flags;

	return;
}


static void 
tny_camel_msg_header_class_init (TnyCamelMsgHeaderClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_camel_msg_header_finalize;

	return;
}


/**
 * tny_camel_msg_header_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
GType 
tny_camel_msg_header_get_type (void)
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
		  sizeof (TnyCamelMsgHeaderClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_camel_msg_header_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyCamelMsgHeader),
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
			"TnyCamelMsgHeader",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_HEADER, 
			&tny_header_info);
	}

	return type;
}

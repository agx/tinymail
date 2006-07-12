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

#include <tny-msg-header-iface.h>
#include <tny-msg-header.h>
#include <tny-msg-folder.h>

#include "tny-camel-common-priv.h"
#include "tny-msg-folder-priv.h"
#include "tny-msg-header-priv.h"

#include <tny-camel-shared.h>

#include <camel/camel-folder.h>
#include <camel/camel.h>
#include <camel/camel-folder-summary.h>

static GObjectClass *parent_class = NULL;

typedef struct _WriteInfo WriteInfo;
struct _WriteInfo
{
	CamelMimeMessage *msg;
	gchar *mime_from;
};

//#pragma pack(1) /* Size will be 21 in stead of 24 */
struct _TnyMsgHeader 
{
	GObject parent;
	void *info;

	TnyMsgFolderIface *folder;
	guchar write:1;
};

struct _TnyMsgHeaderClass 
{
	GObjectClass parent_class;
};

static const gchar *invalid = "Invalid";

static void 
destroy_write (TnyMsgHeader *self)
{
	if (((WriteInfo*)self->info)->msg)
		camel_object_unref (CAMEL_OBJECT (((WriteInfo*)self->info)->msg));

	if (((WriteInfo*)self->info)->mime_from)
		g_free (((WriteInfo*)self->info)->mime_from);

	g_free (self->info);
}

static void
prepare_for_write (TnyMsgHeader *self)
{
	if (!self->write)
	{
		self->info = g_new0 (WriteInfo, 1);
		((WriteInfo*)self->info)->msg = camel_mime_message_new ();
		((WriteInfo*)self->info)->mime_from = NULL;
		self->write = 1;
	}

	return;
}

void /* protected method */
_tny_msg_header_set_camel_message_info (TnyMsgHeader *self, CamelMessageInfo *camel_message_info)
{
	if (G_UNLIKELY (self->info))
		g_warning ("Strange behaviour: Overwriting existing message info");

	if (self->write)
		destroy_write (self);

	self->info = camel_message_info;
	self->write = 0;

	return;
}

CamelMimeMessage* /* protected method */
_tny_msg_header_get_camel_mime_message (TnyMsgHeader *self)
{

	if (G_UNLIKELY (self->write == 0))
	{
		g_warning ("Strange behaviour: the header was not written");
		return NULL;
	}

	return ((WriteInfo*)self->info)->msg;

}



static const gchar*
tny_msg_header_get_replyto (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	const gchar *retval = NULL;

	/* TODO get_replyto */

	return retval;
}



static void
tny_msg_header_set_bcc (TnyMsgHeaderIface *self, const gchar *bcc)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	CamelInternetAddress *addr = camel_internet_address_new ();

	_foreach_email_add_to_inet_addr (bcc, addr);

	prepare_for_write (me);

	camel_mime_message_set_recipients (((WriteInfo*)me->info)->msg, 
		CAMEL_RECIPIENT_TYPE_BCC, addr);

	camel_object_unref (CAMEL_OBJECT (addr));

	return;
}

static void
tny_msg_header_set_cc (TnyMsgHeaderIface *self, const gchar *cc)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	CamelInternetAddress *addr = camel_internet_address_new ();

	_foreach_email_add_to_inet_addr (cc, addr);

	prepare_for_write (me);

	camel_mime_message_set_recipients (((WriteInfo*)me->info)->msg, 
		CAMEL_RECIPIENT_TYPE_CC, addr);
	camel_object_unref (CAMEL_OBJECT (addr));

	return;
}

static void
tny_msg_header_set_from (TnyMsgHeaderIface *self, const gchar *from)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
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
tny_msg_header_set_subject (TnyMsgHeaderIface *self, const gchar *subject)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	prepare_for_write (me);
	camel_mime_message_set_subject (((WriteInfo*)me->info)->msg, subject);

	return;
}

static void
tny_msg_header_set_to (TnyMsgHeaderIface *self, const gchar *to)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	CamelInternetAddress *addr = camel_internet_address_new ();
	gchar *dup;

	dup = g_strdup (to);
	_foreach_email_add_to_inet_addr (dup, addr);
	g_free (dup);

	prepare_for_write (me);

	camel_mime_message_set_recipients ((CamelMimeMessage*)me->info, 
		CAMEL_RECIPIENT_TYPE_TO, addr);

	camel_object_unref (CAMEL_OBJECT (addr));

	return;
}


static void
tny_msg_header_set_replyto (TnyMsgHeaderIface *self, const gchar *to)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	/* TODO set replyto */

	return;
}


static const gchar*
tny_msg_header_get_cc (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	const gchar *retval;

	if (G_UNLIKELY (!me->info))
		return invalid;

	if (G_UNLIKELY (me->write))
		retval = camel_medium_get_header (CAMEL_MEDIUM (((WriteInfo*)me->info)->msg), "cc");
	else
		retval = camel_message_info_cc ((CamelMessageInfo*)me->info);

	return retval;
}

static const gchar*
tny_msg_header_get_bcc (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	const gchar *retval;

	if (G_UNLIKELY (!me->info))
		return invalid;

	if (G_UNLIKELY (me->write))
		retval = camel_medium_get_header (CAMEL_MEDIUM (((WriteInfo*)me->info)->msg), "bcc");
	else
		retval = invalid;

	return retval;
}

static TnyMsgHeaderFlags
tny_msg_header_get_flags (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	TnyMsgHeaderFlags retval;

	if (me->write)
	{
		g_warning ("tny_msg_header_get_flags: This is a header for a new message!\n");
		return retval;
	}

	retval = camel_message_info_flags ((CamelMessageInfo*)me->info);

	return retval;
}

static void
tny_msg_header_set_flags (TnyMsgHeaderIface *self, TnyMsgHeaderFlags mask)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	if (me->write)
	{
		g_warning ("tny_msg_header_get_flags: This is a header for a new message!\n");
		return;
	}

    	camel_message_info_set_flags ((CamelMessageInfo*)me->info, mask, ~0);

	return;
}

static void
tny_msg_header_unset_flags (TnyMsgHeaderIface *self, TnyMsgHeaderFlags mask)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	if (me->write)
	{
		g_warning ("tny_msg_header_get_flags: This is a header for a new message!\n");
		return;
	}

    	camel_message_info_set_flags ((CamelMessageInfo*)me->info, mask, 0);

	return;
}

static time_t
tny_msg_header_get_date_received (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	time_t retval;

	if (G_UNLIKELY (!me->info))
		return retval;

	if (G_UNLIKELY (me->write))
		retval = camel_mime_message_get_date_received (((WriteInfo*)me->info)->msg, NULL);
	else
		retval = camel_message_info_date_received ((CamelMessageInfo*)me->info);

	return retval;
}

static time_t
tny_msg_header_get_date_sent (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	time_t retval;

	if (G_UNLIKELY (!me->info) || G_UNLIKELY (me->write))
		return retval;

	retval = camel_message_info_date_received ((CamelMessageInfo*)me->info);

	return retval;
}
	
static const gchar*
tny_msg_header_get_from (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	
	const gchar *retval;

	if (G_UNLIKELY (!me->info))
		return invalid;

	if (G_UNLIKELY (me->write))
	{
		if (G_LIKELY (!((WriteInfo*)me->info)->mime_from))
		{
			CamelInternetAddress *addr = (CamelInternetAddress*)
				camel_mime_message_get_from (((WriteInfo*)me->info)->msg);
			((WriteInfo*)me->info)->mime_from = camel_address_format (CAMEL_ADDRESS (addr));
		}

		retval = (const gchar*)((WriteInfo*)me->info)->mime_from;
	} else
		retval = camel_message_info_from ((CamelMimeMessage*)me->info);

	return retval;
}

static const gchar*
tny_msg_header_get_subject (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	const gchar *retval;

	if (G_UNLIKELY (!me->info))
		return invalid;

	if (G_UNLIKELY (me->write))
		retval = camel_mime_message_get_subject (((WriteInfo*)me->info)->msg);
	else
		retval = camel_message_info_subject ((CamelMessageInfo*)me->info);

	return retval;
}


static const gchar*
tny_msg_header_get_to (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	gchar *retval;

	if (G_UNLIKELY (!me->info))
		return invalid;

	if (G_UNLIKELY (me->write))
		retval = (gchar*) camel_medium_get_header (CAMEL_MEDIUM (((WriteInfo*)me->info)->msg), "to");
	else
		retval = (gchar*) camel_message_info_to ((CamelMessageInfo*)me->info);

	return (const gchar*)retval;
}

static const gchar*
tny_msg_header_get_message_id (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	gchar *retval;

	if (G_UNLIKELY (!me->info))
		return invalid;

	if (G_UNLIKELY (me->write))
		retval = (gchar*) camel_mime_message_get_message_id (((WriteInfo*)me->info)->msg);
	else
		retval = (gchar*) camel_message_info_message_id ((CamelMessageInfo*)me->info);

	return (const gchar*)retval;


}


static const gchar*
tny_msg_header_get_uid (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	const gchar *retval;

	if (G_UNLIKELY (!me->info) || G_UNLIKELY (me->write))
		return invalid;

	retval = camel_message_info_uid ((CamelMessageInfo*)me->info);

	return retval;
}


static gboolean 
tny_msg_header_has_cache (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	return TRUE;
}

static void
tny_msg_header_uncache (TnyMsgHeaderIface *self)
{
	return;
}

static void
tny_msg_header_finalize (GObject *object)
{
	TnyMsgHeader *self = (TnyMsgHeader*) object;

	if (G_UNLIKELY (self->write))
	{
		destroy_write (self);
	}

	/* Normally we do camel_folder_free_message_info here, but we already got
	   rid of our initial reference at tny-msg-folder.c:add_message_with_uid
	   I know this is actually ugly */

	(*parent_class->finalize) (object);

	return;
}

/**
 * tny_msg_header_new:
 *
 *
 * Return value: A new #TnyMsgHeader instance implemented for Camel
 **/
TnyMsgHeader*
tny_msg_header_new (void)
{
	TnyMsgHeader *self = g_object_new (TNY_TYPE_MSG_HEADER, NULL);
	
	self->info = NULL;
	self->write = 0;

	return self;
}

static void
tny_msg_header_set_folder (TnyMsgHeaderIface *self, const TnyMsgFolderIface *folder)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	me->folder = (TnyMsgFolderIface*)folder;
	return;
}

static const TnyMsgFolderIface*
tny_msg_header_get_folder (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	return (const TnyMsgFolderIface*)me->folder;
}

static void
tny_msg_header_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyMsgHeaderIfaceClass *klass = (TnyMsgHeaderIfaceClass *)g_iface;

	klass->get_from_func = tny_msg_header_get_from;
	klass->get_message_id_func = tny_msg_header_get_message_id;
	klass->get_to_func = tny_msg_header_get_to;
	klass->get_subject_func = tny_msg_header_get_subject;
	klass->get_date_received_func = tny_msg_header_get_date_received;
	klass->get_date_sent_func = tny_msg_header_get_date_sent;
	klass->get_cc_func = tny_msg_header_get_cc;
	klass->get_bcc_func = tny_msg_header_get_bcc;
	klass->get_replyto_func = tny_msg_header_get_replyto;
	klass->get_uid_func = tny_msg_header_get_uid;
	klass->set_folder_func = tny_msg_header_set_folder;
	klass->get_folder_func = tny_msg_header_get_folder;
	klass->set_bcc_func = tny_msg_header_set_bcc;
	klass->set_cc_func = tny_msg_header_set_cc;
	klass->set_to_func = tny_msg_header_set_to;
	klass->set_from_func = tny_msg_header_set_from;
	klass->set_subject_func = tny_msg_header_set_subject;
	klass->set_replyto_func = tny_msg_header_set_replyto;
	klass->has_cache_func = tny_msg_header_has_cache;
	klass->uncache_func = tny_msg_header_uncache;
	klass->set_flags_func = tny_msg_header_set_flags;
	klass->unset_flags_func = tny_msg_header_unset_flags;
	klass->get_flags_func = tny_msg_header_get_flags;

	return;
}


static void 
tny_msg_header_class_init (TnyMsgHeaderClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_msg_header_finalize;

	return;
}


GType 
tny_msg_header_get_type (void)
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
		  sizeof (TnyMsgHeaderClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_msg_header_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMsgHeader),
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};

		static const GInterfaceInfo tny_msg_header_iface_info = 
		{
		  (GInterfaceInitFunc) tny_msg_header_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyMsgHeader",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_MSG_HEADER_IFACE, 
			&tny_msg_header_iface_info);
	}

	return type;
}

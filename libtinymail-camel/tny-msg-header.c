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

#include "tny-camel-common-priv.h"
#include "tny-msg-folder-priv.h"
#include "tny-msg-header-priv.h"

#include <tny-camel-shared.h>

#include <camel/camel-folder.h>
#include <camel/camel.h>
#include <camel/camel-folder-summary.h>

static GObjectClass *parent_class = NULL;

struct _TnyMsgHeader 
{
	GObject parent;

	gchar *uid;
	TnyMsgFolderIface *folder;
	gboolean use_summary;
	GMutex *hdr_lock;
	CamelMessageInfo *message_info;
	CamelMimeMessage *mime_message;
	gchar *mime_from;
	const gchar *invalid;
	gboolean uncachable;
};

struct _TnyMsgHeaderClass 
{
	GObjectClass parent_class;
};

gboolean 
_tny_msg_header_get_is_pop (TnyMsgHeader *self)
{
	/* All summary types aren't pop atm ;-) */
	return (!self->use_summary);
}

void
_tny_msg_header_set_not_uncachable (TnyMsgHeader *self)
{
	self->uncachable = FALSE;
	return;
}

static void
unload_msg_header (TnyMsgHeader *self)
{
	if (G_UNLIKELY (!self->uncachable))
		return;

	if (G_LIKELY (self->mime_from))
	{
		g_free (self->mime_from);
		self->mime_from = NULL;
	}
	if (G_LIKELY (self->use_summary))
	{
		if (G_LIKELY (self->message_info))
		{
			/* It looks like this conflicts with freeing the folder 
			camel_message_info_free (self->message_info); 
			self->message_info = NULL; */
		}
	} else {
		if (G_LIKELY (self->mime_message) && G_LIKELY (CAMEL_IS_OBJECT (self->mime_message)))
			camel_object_unref (CAMEL_OBJECT (self->mime_message));	
		self->mime_message = NULL;
	}

	return;
}

/**
 * tny_msg_header_set_use_summary:
 * @self: The #TnyMsgHeader instance
 * @val: Whether or not to use summary capabilities
 *
 **/
void 
tny_msg_header_set_use_summary (TnyMsgHeader *self, gboolean val)
{
	g_mutex_lock (self->hdr_lock);
	unload_msg_header (self);
	self->use_summary = val;
	g_mutex_unlock (self->hdr_lock);

	return;
}

static void
load_msg_header (TnyMsgHeader *self)
{

	if (G_UNLIKELY (!self->uncachable))
		return;

	if (G_LIKELY (self->use_summary))
	{
		if (G_LIKELY (!self->message_info) && G_LIKELY (self->folder) && G_LIKELY (self->uid))
		{
		  CamelFolder *folder = _tny_msg_folder_get_camel_folder (self->folder);
		  CamelMessageInfo *msginfo;

		  if (folder)
		  {
			msginfo = camel_folder_get_message_info (folder, self->uid);
			_tny_msg_header_set_camel_message_info (self, msginfo);
		  }
		}
	} else {
		if (G_LIKELY (!self->mime_message) && G_LIKELY (self->folder) && G_LIKELY (self->uid))
		{
		  CamelFolder *folder = _tny_msg_folder_get_camel_folder (self->folder);
		  CamelException ex = CAMEL_EXCEPTION_INITIALISER;

		  if (folder)
		  {
			self->mime_message = camel_folder_get_message 
				(folder, self->uid, &ex);
		  }
		}
	}

	return;
}

static void
prepare_for_write (TnyMsgHeader *self)
{
	unload_msg_header (self);

	self->use_summary = FALSE;

	if (G_LIKELY (!self->mime_message))
		self->mime_message = camel_mime_message_new ();

	_tny_msg_header_set_not_uncachable (self);

	return;
}

void /* protected method */
_tny_msg_header_set_camel_message_info (TnyMsgHeader *self, CamelMessageInfo *camel_message_info)
{

	if (G_UNLIKELY (self->message_info))
		g_warning ("Strange behaviour: Overwriting existing message info");

	self->message_info = camel_message_info;

	return;
}

CamelMimeMessage* /* protected method */
_tny_msg_header_get_camel_mime_message (TnyMsgHeader *self)
{
	CamelMimeMessage *retval;

	retval = self->mime_message;

	return retval;
}

void /* protected method */
_tny_msg_header_set_camel_mime_message (TnyMsgHeader *self, CamelMimeMessage *camel_mime_message)
{

	if (G_UNLIKELY (self->mime_message))
		g_warning (_("Strange behaviour: Overwriting existing MIME message"));

	self->mime_message = camel_mime_message;

	return;
}

const TnyMsgFolderIface* 
tny_msg_header_get_folder (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	const TnyMsgFolderIface *retval;

	g_mutex_lock (me->hdr_lock);
	retval = me->folder;
	g_mutex_unlock (me->hdr_lock);

	return retval;
}


void
tny_msg_header_set_folder (TnyMsgHeaderIface *self, const TnyMsgFolderIface* folder)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	if (G_UNLIKELY (me->folder))
		g_warning (_("Strange behaviour: Overwriting existing folder"));

	g_mutex_lock (me->hdr_lock);
	me->folder = (TnyMsgFolderIface*)folder;
	g_mutex_unlock (me->hdr_lock);

	return;
}


static const gchar*
tny_msg_header_get_replyto (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	const gchar *retval=NULL;

	g_mutex_lock (me->hdr_lock);
	load_msg_header (me);
	/* TODO */
	g_mutex_unlock (me->hdr_lock);

	return retval;
}



static void
tny_msg_header_set_bcc (TnyMsgHeaderIface *self, const gchar *bcc)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	CamelInternetAddress *addr = camel_internet_address_new ();

	g_mutex_lock (me->hdr_lock);

	_foreach_email_add_to_inet_addr (bcc, addr);

	prepare_for_write (me);

	camel_mime_message_set_recipients (me->mime_message, 
		CAMEL_RECIPIENT_TYPE_BCC, addr);

	camel_object_unref (CAMEL_OBJECT (addr));

	g_mutex_unlock (me->hdr_lock);

	return;
}

static void
tny_msg_header_set_cc (TnyMsgHeaderIface *self, const gchar *cc)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	CamelInternetAddress *addr = camel_internet_address_new ();

	g_mutex_lock (me->hdr_lock);

	_foreach_email_add_to_inet_addr (cc, addr);

	prepare_for_write (me);

	camel_mime_message_set_recipients (me->mime_message, 
		CAMEL_RECIPIENT_TYPE_CC, addr);

	camel_object_unref (CAMEL_OBJECT (addr));

	g_mutex_unlock (me->hdr_lock);

	return;
}

static void
tny_msg_header_set_from (TnyMsgHeaderIface *self, const gchar *from)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	CamelInternetAddress *addr = camel_internet_address_new ();
	gchar *dup;

	g_mutex_lock (me->hdr_lock);

	dup = g_strdup (from);
	_string_to_camel_inet_addr (dup, addr);
	g_free (dup);

	prepare_for_write (me);

	camel_mime_message_set_from (me->mime_message, addr);
	camel_object_unref (CAMEL_OBJECT (addr));

	g_mutex_unlock (me->hdr_lock);

	return;
}

static void
tny_msg_header_set_subject (TnyMsgHeaderIface *self, const gchar *subject)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	g_mutex_lock (me->hdr_lock);

	prepare_for_write (me);

	camel_mime_message_set_subject (me->mime_message, subject);

	g_mutex_unlock (me->hdr_lock);

	return;
}

static void
tny_msg_header_set_to (TnyMsgHeaderIface *self, const gchar *to)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	CamelInternetAddress *addr = camel_internet_address_new ();
	gchar *dup;

	g_mutex_lock (me->hdr_lock);

	dup = g_strdup (to);

	_foreach_email_add_to_inet_addr (dup, addr);

	g_free (dup);

	prepare_for_write (me);

	camel_mime_message_set_recipients (me->mime_message, 
		CAMEL_RECIPIENT_TYPE_TO, addr);

	camel_object_unref (CAMEL_OBJECT (addr));

	g_mutex_unlock (me->hdr_lock);

	return;
}


static void
tny_msg_header_set_replyto (TnyMsgHeaderIface *self, const gchar *to)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	g_mutex_lock (me->hdr_lock);
	prepare_for_write (me);
	/* TODO */
	g_mutex_unlock (me->hdr_lock);

	return;
}


static const gchar*
tny_msg_header_get_cc (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	const gchar *retval=NULL;

	g_mutex_lock (me->hdr_lock);

	load_msg_header (me);

	if (G_LIKELY (me->use_summary) && G_LIKELY (me->message_info))
		retval = camel_message_info_cc (me->message_info);
	else if (G_LIKELY (me->mime_message))
		retval = camel_medium_get_header (CAMEL_MEDIUM (me->mime_message), "cc");

	if (G_UNLIKELY (!retval))
		retval = me->invalid;

	g_mutex_unlock (me->hdr_lock);

	return retval;
}

static const gchar*
tny_msg_header_get_bcc (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	static const gchar *retval=NULL;

	g_mutex_lock (me->hdr_lock);

	load_msg_header (me);

	if (G_LIKELY (me->use_summary)) /* TODO */
		retval = me->invalid;
	else if (G_LIKELY (me->mime_message))
		retval = camel_medium_get_header (CAMEL_MEDIUM (me->mime_message), "bcc");

	if (G_UNLIKELY (!retval))
		retval = me->invalid;

	g_mutex_unlock (me->hdr_lock);

	return retval;
}

static const TnyMsgHeaderFlags
tny_msg_header_get_flags (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	TnyMsgHeaderFlags retval;

	g_mutex_lock (me->hdr_lock);

	load_msg_header (me);

	if (G_LIKELY (me->use_summary) && G_LIKELY (me->message_info))
		retval = camel_message_info_flags (me->message_info);

	g_mutex_unlock (me->hdr_lock);

	return (const TnyMsgHeaderFlags)retval;
}

static void
tny_msg_header_set_flags (TnyMsgHeaderIface *self, TnyMsgHeaderFlags mask)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	g_mutex_lock (me->hdr_lock);

	prepare_for_write (me);

	if (G_LIKELY (me->use_summary) && G_LIKELY (me->message_info))
    	camel_message_info_set_flags (me->message_info, mask, ~0);

	g_mutex_unlock (me->hdr_lock);
}

static void
tny_msg_header_unset_flags (TnyMsgHeaderIface *self, TnyMsgHeaderFlags mask)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	g_mutex_lock (me->hdr_lock);

	prepare_for_write (me);

	if (G_LIKELY (me->use_summary) && G_LIKELY (me->message_info))
    	camel_message_info_set_flags (me->message_info, mask, 0);

	g_mutex_unlock (me->hdr_lock);
}

static const time_t
tny_msg_header_get_date_received (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	time_t retval;

	g_mutex_lock (me->hdr_lock);

	load_msg_header (me);

	if (G_LIKELY (me->use_summary) && G_LIKELY (me->message_info))
		retval = camel_message_info_date_received (me->message_info);
	else if (G_LIKELY (me->mime_message))
		retval = camel_mime_message_get_date_received (me->mime_message, NULL);

	g_mutex_unlock (me->hdr_lock);

	return (const time_t)retval;
}

static const time_t
tny_msg_header_get_date_sent (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	time_t retval;

	g_mutex_lock (me->hdr_lock);

	load_msg_header (me);

	if (G_LIKELY (me->use_summary) && G_LIKELY (me->message_info))
		retval = camel_message_info_date_sent (me->message_info);
	else {
		/* TODO: write case */
	}

	g_mutex_unlock (me->hdr_lock);

	return (const time_t)retval;
}
	
static const gchar*
tny_msg_header_get_from (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	
	const gchar *retval=NULL;

	g_mutex_lock (me->hdr_lock);

	load_msg_header (me);

	if (G_LIKELY (me->use_summary) && G_LIKELY (me->message_info))
		retval = camel_message_info_from (me->message_info);
	else
	{
		if (G_LIKELY (!me->mime_from) && G_LIKELY (me->mime_message))
		{
			CamelInternetAddress *addr = (CamelInternetAddress*)
				camel_mime_message_get_from (me->mime_message);
			me->mime_from = camel_address_format (CAMEL_ADDRESS (addr));
			/* camel_object_unref (CAMEL_OBJECT (addr)); */
		}

		retval = (const gchar*)me->mime_from;
	}

	if (G_UNLIKELY (!retval))
		retval = me->invalid;

	g_mutex_unlock (me->hdr_lock);

	return retval;
}

static const gchar*
tny_msg_header_get_subject (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	const gchar *retval=NULL;

	g_mutex_lock (me->hdr_lock);

	load_msg_header (me);

	if (G_LIKELY (me->use_summary) && G_LIKELY (me->message_info))
		retval = camel_message_info_subject (me->message_info);
	else if (G_LIKELY (me->mime_message))
		retval = camel_mime_message_get_subject (me->mime_message);

	if (G_UNLIKELY (!retval))
		retval = me->invalid;

	g_mutex_unlock (me->hdr_lock);

	return retval;
}


static const gchar*
tny_msg_header_get_to (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);
	
	const gchar *retval=NULL;

	g_mutex_lock (me->hdr_lock);

	load_msg_header (me);

	if (G_LIKELY (me->use_summary) && G_LIKELY (me->message_info))
		retval = camel_message_info_to (me->message_info);
	else if (G_LIKELY (me->mime_message))
		retval = camel_medium_get_header (CAMEL_MEDIUM (me->mime_message), "to");

	if (G_UNLIKELY (!retval))
		retval = me->invalid;

	g_mutex_unlock (me->hdr_lock);

	return retval;
}

static const gchar*
tny_msg_header_get_message_id (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	const gchar *retval=NULL;

	g_mutex_lock (me->hdr_lock);

	load_msg_header (me);

	if (G_LIKELY (me->use_summary) && G_LIKELY (me->message_info))
		retval = (const gchar*)camel_message_info_message_id (me->message_info);
	else if (G_LIKELY (me->mime_message))
		retval = camel_mime_message_get_message_id (me->mime_message);

	if (G_UNLIKELY (!retval))
		retval = me->invalid;

	g_mutex_unlock (me->hdr_lock);

	return retval;

}


static const gchar*
tny_msg_header_get_uid (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	const gchar *retval;

	g_mutex_lock (me->hdr_lock);

	load_msg_header (me);

	if (G_LIKELY (me->use_summary) && G_LIKELY (me->message_info))
		retval = camel_message_info_uid (me->message_info);
	else /* Bleh solution ... */
		retval = me->uid;

	g_mutex_unlock (me->hdr_lock);

	return retval;
}

static void
tny_msg_header_set_uid (TnyMsgHeaderIface *self, const gchar *uid)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	g_mutex_lock (me->hdr_lock);

	unload_msg_header (me);

	/* Speedup trick, also check tny-msg-folder.c */
	/* if (priv->uid)
		g_free (priv->uid);
	priv->uid = g_strdup (uid); */

	/* Yes I know what I'm doing, also check tny-msg-folder.c */

	if (G_LIKELY (me->use_summary))
		me->uid = (gchar*)uid;
	else {
		/* NEED TO INVESTIGATE THIS
		For some reason the trick doesn't work if you don't 
		have support for summaries */
		if (me->uid)
			g_free (me->uid);
		me->uid = g_strdup (uid);
		/* TODO: And if I AM going to do it this way, I need to lock 
		 priv->uid */
	}

	g_mutex_unlock (me->hdr_lock);

	return;
}


static const gboolean 
tny_msg_header_has_cache (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	gboolean retval;

	g_mutex_lock (me->hdr_lock);

	if (G_LIKELY (me->use_summary))
		retval = (me->message_info != NULL);
	else
		retval = (me->mime_message != NULL);

	g_mutex_unlock (me->hdr_lock);

	return (const gboolean)retval;
}

static void
tny_msg_header_uncache (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	g_mutex_lock (me->hdr_lock);

	if (G_LIKELY (me->use_summary) && G_LIKELY (me->message_info))
		unload_msg_header (me);

	if (G_UNLIKELY (!me->use_summary) && G_LIKELY (me->mime_message))
		unload_msg_header (me);

	g_mutex_unlock (me->hdr_lock);

	return;
}

static void
tny_msg_header_finalize (GObject *object)
{
	TnyMsgHeader *self = (TnyMsgHeader*) object;

	g_mutex_lock (self->hdr_lock);

	/* Instance is going to dissapear, set uncachable as we 
	   reuse the functionality */
	self->uncachable = TRUE;

	if (G_LIKELY (self->use_summary) && G_LIKELY (self->message_info))
		unload_msg_header (self);

	if (G_UNLIKELY (!self->use_summary) && G_LIKELY (self->mime_message))
		unload_msg_header (self);

	/* Indeed, check the speedup trick above */
	if (G_LIKELY (self->uid) && G_UNLIKELY (!self->use_summary))
		g_free (self->uid); /* Also check above */

	g_mutex_unlock (self->hdr_lock);

	g_mutex_free (self->hdr_lock);

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
	static const gchar *inv = "Invalid";

	self->uncachable = TRUE;	
	self->invalid = inv;
	self->mime_message = NULL;
	self->message_info = NULL;
	self->mime_from = NULL;

	/* Second allocation :-( */
	self->hdr_lock = g_mutex_new ();

	return self;
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
	klass->set_uid_func = tny_msg_header_set_uid;
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

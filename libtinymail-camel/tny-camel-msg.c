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

#include <time.h>

#include <tny-list.h>
#include <tny-iterator.h>

#include <tny-camel-msg.h>
#include <tny-mime-part.h>
#include <tny-stream.h>
#include <tny-header.h>
#include <tny-camel-mime-part.h>
#include <tny-stream-camel.h>
#include <tny-camel-header.h>

#include <tny-camel-shared.h>

static GObjectClass *parent_class = NULL;

#include <camel/camel-types.h>

#include "tny-camel-msg-priv.h"
#include "tny-camel-mime-part-priv.h"
#include "tny-camel-header-priv.h"

#define TNY_CAMEL_MSG_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_CAMEL_MSG, TnyCamelMsgPriv))


CamelMimeMessage* 
_tny_camel_msg_get_camel_mime_message (TnyCamelMsg *self)
{
	TnyCamelMimePartPriv *ppriv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);

	return CAMEL_MIME_MESSAGE (ppriv->part);
}



static TnyFolder* 
tny_camel_msg_get_folder (TnyMsg *self)
{
	return TNY_CAMEL_MSG_GET_CLASS (self)->get_folder_func (self);
}

static TnyFolder* 
tny_camel_msg_get_folder_default (TnyMsg *self)
{
	TnyCamelMsgPriv *priv = TNY_CAMEL_MSG_GET_PRIVATE (self);
	TnyFolder *retval;

	g_mutex_lock (priv->folder_lock);
	retval = priv->folder;
	g_object_ref (G_OBJECT (retval));
	g_mutex_unlock (priv->folder_lock);

	return retval;
}


void
_tny_camel_msg_set_folder (TnyCamelMsg *self, TnyFolder* folder)
{
	TnyCamelMsgPriv *priv = TNY_CAMEL_MSG_GET_PRIVATE (self);

	g_assert (TNY_IS_CAMEL_FOLDER (folder));

	g_mutex_lock (priv->folder_lock);
	priv->folder = (TnyFolder*)folder;
	g_mutex_unlock (priv->folder_lock);

	return;
}



static TnyHeader*
tny_camel_msg_get_header (TnyMsg *self)
{
	return TNY_CAMEL_MSG_GET_CLASS (self)->get_header_func (self);
}

static TnyHeader*
tny_camel_msg_get_header_default (TnyMsg *self)
{
	TnyCamelMsgPriv *priv = TNY_CAMEL_MSG_GET_PRIVATE (self);
	TnyHeader *retval;

	g_mutex_lock (priv->header_lock);
	retval = priv->header;
	if (retval)
		g_object_ref (G_OBJECT (retval));
	else {
		CamelMimeMessage *msg;
		priv->header = tny_camel_header_new ();
		msg = _tny_camel_msg_get_camel_mime_message (TNY_CAMEL_MSG (self));
		_tny_camel_header_set_camel_mime_message 
			(TNY_CAMEL_HEADER (priv->header), msg);
		retval = priv->header;
	}
	g_mutex_unlock (priv->header_lock);

	return retval;
}



void
_tny_camel_msg_set_header (TnyCamelMsg *self, TnyCamelHeader *header)
{
	TnyCamelMsgPriv *priv = TNY_CAMEL_MSG_GET_PRIVATE (self);
	g_mutex_lock (priv->header_lock);

	if (priv->header)
		g_object_unref (G_OBJECT (priv->header));

	g_object_ref (G_OBJECT (header));
	priv->header = TNY_HEADER (header);
	g_mutex_unlock (priv->header_lock);
	return;
}

static void
tny_camel_msg_set_header (TnyMsg *self, TnyHeader *header)
{
	TNY_CAMEL_MSG_GET_CLASS (self)->set_header_func (self, header);
	return;
}

static void
tny_camel_msg_set_header_default (TnyMsg *self, TnyHeader *header)
{
	CamelMimeMessage *msg;
	TnyCamelMsgPriv *priv = TNY_CAMEL_MSG_GET_PRIVATE (self);
	CamelInternetAddress *from, *recipients, *replyto;
	const gchar *str;

	g_assert (TNY_IS_CAMEL_HEADER (header));

	g_mutex_lock (priv->header_lock);

	if (priv->header)
		g_object_unref (G_OBJECT (priv->header));
	g_object_ref (G_OBJECT (header));

	priv->header = header;

	if (((TnyCamelHeader *)header)->write)
	{
		TnyCamelMimePartPriv *ppriv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);
		g_mutex_lock (ppriv->part_lock);
		if (ppriv->part)
			camel_object_unref (CAMEL_OBJECT (ppriv->part));
		/* Add a reference? */
		ppriv->part = (CamelMimePart *) ((WriteInfo*)((TnyCamelHeader *)header)->info)->msg;
		g_mutex_unlock (ppriv->part_lock);
	} else {
		msg = _tny_camel_msg_get_camel_mime_message (TNY_CAMEL_MSG (self));
		_tny_camel_header_set_camel_mime_message 
			(TNY_CAMEL_HEADER (priv->header), msg);
	}

/*
	from = camel_internet_address_new ();
	str = tny_header_get_from (header);
	_foreach_email_add_to_inet_addr (str, from);
	camel_mime_message_set_from (msg, from);
	camel_object_unref (CAMEL_OBJECT (from));

	replyto = camel_internet_address_new ();
	str = tny_header_get_replyto (header);
	_foreach_email_add_to_inet_addr (str, replyto);
	camel_mime_message_set_from (msg, replyto);
	camel_object_unref (CAMEL_OBJECT (replyto));

	recipients = camel_internet_address_new ();
	str = tny_header_get_to (header);
	_foreach_email_add_to_inet_addr (str, recipients);
	camel_mime_message_set_recipients (msg, CAMEL_RECIPIENT_TYPE_TO, recipients);
	camel_object_unref (CAMEL_OBJECT (recipients));

	str = tny_header_get_cc (header);
	_foreach_email_add_to_inet_addr (str, recipients);
	camel_mime_message_set_recipients (msg, CAMEL_RECIPIENT_TYPE_CC, recipients);
	camel_object_unref (CAMEL_OBJECT (recipients));

	str = tny_header_get_bcc (header);
	_foreach_email_add_to_inet_addr (str, recipients);
	camel_mime_message_set_recipients (msg, CAMEL_RECIPIENT_TYPE_BCC, recipients);
	camel_object_unref (CAMEL_OBJECT (recipients));

	camel_mime_message_set_subject (msg ,tny_header_get_subject (header));
*/

	g_mutex_unlock (priv->header_lock);

	return;
}


static void
tny_camel_msg_finalize (GObject *object)
{
	TnyCamelMsg *self = (TnyCamelMsg*) object;
	TnyCamelMsgPriv *priv = TNY_CAMEL_MSG_GET_PRIVATE (self);
	TnyCamelMimePartPriv *ppriv = TNY_CAMEL_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->message_lock);
	g_mutex_lock (priv->header_lock);

	if (G_LIKELY (priv->header))
		g_object_unref (G_OBJECT (priv->header));
	priv->header = NULL;

	g_mutex_unlock (priv->header_lock);
	g_mutex_unlock (priv->message_lock);

	g_mutex_free (priv->message_lock);
	g_mutex_free (priv->header_lock);
	g_mutex_free (priv->parts_lock);
	g_mutex_free (priv->folder_lock);

	(*parent_class->finalize) (object);

	return;
}

/**
 * tny_camel_msg_new:
 * 
 * The #TnyCamelMsg implementation is actually a proxy for #CamelMimeMessage (and
 * a few other Camel types)
 *
 * Return value: A new #TnyMsg instance implemented for Camel
 **/
TnyMsg*
tny_camel_msg_new (void)
{
	TnyCamelMsg *self = g_object_new (TNY_TYPE_CAMEL_MSG, NULL);
	
	_tny_camel_mime_part_set_part (TNY_CAMEL_MIME_PART (self), 
		CAMEL_MIME_PART (camel_mime_message_new ()));

	return TNY_MSG (self);
}

/**
 * tny_camel_msg_new_with_header:
 * @header: a #TnyHeader object
 *
 * The #TnyCamelMsg implementation is actually a proxy for #CamelMimeMessage (and
 * a few other Camel types)
 *
 * Return value: A new #TnyMsg instance implemented for Camel
 **/
TnyMsg*
tny_camel_msg_new_with_header (TnyHeader *header)
{
	TnyCamelMsg *self = g_object_new (TNY_TYPE_CAMEL_MSG, NULL);

	_tny_camel_mime_part_set_part (TNY_CAMEL_MIME_PART (self), 
		CAMEL_MIME_PART (camel_mime_message_new ()));

	tny_camel_msg_set_header (TNY_MSG (self), header);

	return TNY_MSG (self);
}

/**
 * tny_camel_msg_new_with_part:
 * @part: a #CamelMimePart object
 *
 * The #TnyMsg implementation is actually a proxy for #CamelMimePart.
 *
 * Return value: A new #TnyMsg instance implemented for Camel
 **/
TnyMsg*
tny_camel_msg_new_with_part (CamelMimePart *part)
{
	TnyCamelMsg *self = g_object_new (TNY_TYPE_CAMEL_MSG, NULL);

	_tny_camel_mime_part_set_part (TNY_CAMEL_MIME_PART (self), 
		CAMEL_MIME_PART (camel_mime_message_new ()));

	return TNY_MSG (self);
}

/**
 * tny_camel_msg_new_with_part_and_header:
 * @part: a #CamelMimePart object
 * @header: a #TnyHeader object
 * 
 * The #TnyMsg implementation is actually a proxy for #CamelMimePart.
 *
 * Return value: A new #TnyMsg instance implemented for Camel
 **/
TnyMsg*
tny_camel_msg_new_with_part_and_header (CamelMimePart *part, TnyHeader *header)
{
	TnyCamelMsg *self = g_object_new (TNY_TYPE_CAMEL_MSG, NULL);

	_tny_camel_mime_part_set_part (TNY_CAMEL_MIME_PART (self), part);
	tny_camel_msg_set_header (TNY_MSG (self), header);

	return TNY_MSG (self);
}

static void
tny_msg_init (gpointer g, gpointer iface_data)
{
	TnyMsgIface *klass = (TnyMsgIface *)g;

	klass->get_header_func = tny_camel_msg_get_header;
	klass->set_header_func = tny_camel_msg_set_header;
	klass->get_folder_func = tny_camel_msg_get_folder;

	return;
}

static void 
tny_camel_msg_class_init (TnyCamelMsgClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	class->get_header_func = tny_camel_msg_get_header_default;
	class->set_header_func = tny_camel_msg_set_header_default;
	class->get_folder_func = tny_camel_msg_get_folder_default;

	object_class->finalize = tny_camel_msg_finalize;

	g_type_class_add_private (object_class, sizeof (TnyCamelMsgPriv));

	return;
}


static void
tny_camel_msg_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyCamelMsg *self = (TnyCamelMsg*)instance;
	TnyCamelMsgPriv *priv = TNY_CAMEL_MSG_GET_PRIVATE (self);

	priv->header = NULL;

	priv->message_lock = g_mutex_new ();
	priv->parts_lock = g_mutex_new ();
	priv->header_lock = g_mutex_new ();
	priv->folder_lock = g_mutex_new ();

	return;
}

GType 
tny_camel_msg_get_type (void)
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
		  sizeof (TnyCamelMsgClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_camel_msg_class_init, /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyCamelMsg),
		  0,      /* n_preallocs */
		  tny_camel_msg_instance_init,    /* instance_init */
		  NULL
		};

		static const GInterfaceInfo tny_msg_info = 
		{
		  (GInterfaceInitFunc) tny_msg_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (TNY_TYPE_CAMEL_MIME_PART,
			"TnyCamelMsg",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_MSG,
			&tny_msg_info);

	}

	return type;
}


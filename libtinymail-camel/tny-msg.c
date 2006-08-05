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
#include <tny-list-iface.h>
#include <tny-iterator-iface.h>

#include <tny-msg.h>
#include <tny-msg-mime-part-iface.h>
#include <tny-stream-iface.h>
#include <tny-msg-header-iface.h>
#include <tny-msg-mime-part.h>
#include <tny-stream-camel.h>
#include <tny-msg-header.h>

#include <tny-camel-shared.h>

static GObjectClass *parent_class = NULL;

#include <camel/camel-types.h>

#include "tny-msg-priv.h"
#include "tny-msg-mime-part-priv.h"
#include "tny-msg-header-priv.h"

#define TNY_MSG_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_MSG, TnyMsgPriv))

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

        if (G_UNLIKELY (containee == NULL))
                return go;

        if (G_LIKELY (CAMEL_IS_MULTIPART (containee)))
	{
                parts = camel_multipart_get_number (CAMEL_MULTIPART (containee));
                for (i = 0; go && i < parts; i++) 
		{
                        CamelMimePart *part = camel_multipart_get_part (CAMEL_MULTIPART (containee), i);
			if (part)
			{
				/* http://bugzilla.gnome.org/show_bug.cgi?id=343683 
				   and tny-msg-mime-part.c:515 ! */

				go = message_foreach_part_rec (msg, part, callback, data);
			} else go = FALSE;
                }

        } else if (G_LIKELY (CAMEL_IS_MIME_MESSAGE (containee)))
	{
                go = message_foreach_part_rec (msg, (CamelMimePart *)containee, callback, data);
        }

        return go;
}


static gboolean
received_a_part (CamelMimeMessage *message, CamelMimePart *part, void *data)
{
	TnyListIface *list = data;
	TnyMimePartIface *tpart;

	if (!part)
		return;

	/* http://bugzilla.gnome.org/show_bug.cgi?id=343683 
	   and tny-msg-mime-part.c:515 ! */

	tpart = TNY_MIME_PART_IFACE  (tny_msg_mime_part_new (part));

	tny_list_iface_prepend (list, (GObject*)tpart);
	g_object_unref (G_OBJECT (tpart));

	return TRUE;
}



CamelMimeMessage* 
_tny_msg_get_camel_mime_message (TnyMsg *self)
{
	TnyMimePartPriv *ppriv = TNY_MIME_PART_GET_PRIVATE (self);

	return CAMEL_MIME_MESSAGE (ppriv->part);
}


void
_tny_msg_set_camel_mime_message (TnyMsg *self, CamelMimeMessage *message)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (self);
	TnyMimePartPriv *ppriv = TNY_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->message_lock);

	if (ppriv->part)
		camel_object_unref (CAMEL_OBJECT (ppriv->part));

	/* camel_object_ref (CAMEL_OBJECT (message)); */
	ppriv->part = CAMEL_MIME_PART (message);
	
	/* Warning: large lock that locks code, not data */
	g_mutex_unlock (priv->message_lock);

	return;
}

TnyFolderIface* 
tny_msg_get_folder (TnyMsgIface *self)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));
	TnyFolderIface *retval;

	g_mutex_lock (priv->folder_lock);
	retval = priv->folder;
	g_mutex_unlock (priv->folder_lock);

	return retval;
}


void
_tny_msg_set_folder (TnyMsgIface *self, TnyFolderIface* folder)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));

	g_mutex_lock (priv->folder_lock);
	priv->folder = (TnyFolderIface*)folder;
	g_mutex_unlock (priv->folder_lock);

	return;
}

static void
tny_msg_get_parts (TnyMsgIface *self, TnyListIface *list)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));
	TnyMimePartPriv *ppriv = TNY_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->parts_lock);

	message_foreach_part_rec ((CamelMimeMessage*)ppriv->part, 
		(CamelMimePart *)ppriv->part, received_a_part, list);

	g_mutex_unlock (priv->parts_lock);

	return;
}

static TnyMsgHeaderIface*
tny_msg_get_header (TnyMsgIface *self)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));
	TnyMsgHeaderIface *retval;

	g_mutex_lock (priv->header_lock);
	retval = priv->header;
	g_mutex_unlock (priv->header_lock);

	return retval;
}


static gint
tny_msg_add_part (TnyMsgIface *self, TnyMimePartIface *part)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));
	TnyMimePartPriv *ppriv = TNY_MIME_PART_GET_PRIVATE (self);
	CamelMedium *medium;
	CamelDataWrapper *containee;
	TnyMimePartIface *tpart;
	gint curl = 0;

	g_mutex_lock (priv->message_lock);
	g_mutex_lock (ppriv->part_lock);

	medium = CAMEL_MEDIUM (ppriv->part);
	containee = camel_medium_get_content_object (medium);

	/* Warp it into a multipart */
	if (G_UNLIKELY (!containee) || G_LIKELY (!CAMEL_IS_MULTIPART (containee)))
	{
		/* TODO: restore original mime part? */
		if (G_LIKELY (containee))
			camel_object_unref (CAMEL_OBJECT (containee));

		curl = 0;

		containee = (CamelDataWrapper*)camel_multipart_new ();
		camel_multipart_set_boundary ((CamelMultipart*)containee, NULL);
		camel_medium_set_content_object (medium, containee);
	}

	/* TODO: coupling mistake. This makes it obligated to use a specific
	   implementation of MsgMimePartIface (the camel one). */

	g_mutex_lock (priv->parts_lock);

	camel_multipart_add_part ((CamelMultipart*)containee, 
		tny_msg_mime_part_get_part (TNY_MIME_PART (part)));

	g_mutex_unlock (priv->parts_lock);

	/* Warning: large lock that locks code, not data */
	g_mutex_unlock (ppriv->part_lock);
	g_mutex_unlock (priv->message_lock);

	return camel_multipart_get_number ((CamelMultipart*)containee);
}

/* TODO: camel_mime_message_set_date(msg, time(0), 930); */

static void 
tny_msg_del_part (TnyMsgIface *self, gint id)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));
	TnyMimePartPriv *ppriv = TNY_MIME_PART_GET_PRIVATE (self);
	gpointer remove;
	TnyIteratorIface *iterator;
	CamelDataWrapper *containee;

	g_mutex_lock (priv->message_lock);

	containee = camel_medium_get_content_object (CAMEL_MEDIUM (ppriv->part));
	camel_multipart_remove_part_at (CAMEL_MULTIPART (containee), id);

	if (G_LIKELY (remove))
		camel_object_unref (CAMEL_OBJECT (remove));

	/* Warning: large lock that locks code, not data */
	g_mutex_unlock (priv->message_lock);

	return;
}


static void
tny_msg_set_header (TnyMsgIface *self, TnyMsgHeaderIface *header)
{
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));

	g_mutex_lock (priv->header_lock);

	if (priv->header)
		g_object_unref (G_OBJECT (priv->header));
	g_object_ref (G_OBJECT (header));

	priv->header = header;

	g_mutex_unlock (priv->header_lock);

	return;
}


static void
tny_msg_finalize (GObject *object)
{
	TnyMsg *self = (TnyMsg*) object;
	TnyMsgPriv *priv = TNY_MSG_GET_PRIVATE (TNY_MSG (self));
	TnyMimePartPriv *ppriv = TNY_MIME_PART_GET_PRIVATE (self);

	g_mutex_lock (priv->header_lock);
	if (G_LIKELY (priv->header))
		g_object_unref (G_OBJECT (priv->header));
	priv->header = NULL;
	g_mutex_unlock (priv->header_lock);

	g_mutex_lock (priv->message_lock);
	if (ppriv->part)
	{
		/* http://bugzilla.gnome.org/show_bug.cgi?id=343683 */
		while (((CamelObject*)ppriv->part)->ref_count >= 1)
			camel_object_unref (CAMEL_OBJECT (ppriv->part));
	}
	ppriv->part = NULL;
	g_mutex_unlock (priv->message_lock);

	g_mutex_free (priv->message_lock);
	g_mutex_free (priv->header_lock);
	g_mutex_free (priv->parts_lock);
	g_mutex_free (priv->folder_lock);

	/* ppriv->part should also get destroyed (by gobject) */

	(*parent_class->finalize) (object);

	return;
}

/**
 * tny_msg_new:
 * 
 * The #TnyMsg implementation is actually a proxy for #CamelMimeMessage (and
 * a few other Camel types)
 *
 * Return value: A new #TnyMsgIface instance implemented for Camel
 **/
TnyMsg*
tny_msg_new (void)
{
	TnyMsg *self = g_object_new (TNY_TYPE_MSG, NULL);
	
	_tny_msg_set_camel_mime_message (self, camel_mime_message_new  ());

	return self;
}

/**
 * tny_msg_new_with_header:
 * @header: a #TnyMsgHeaderIface object
 *
 * The #TnyMsg implementation is actually a proxy for #CamelMimeMessage (and
 * a few other Camel types)
 *
 * Return value: A new #TnyMsgIface instance implemented for Camel
 **/
TnyMsg*
tny_msg_new_with_header (TnyMsgHeaderIface *header)
{
	TnyMsg *self = g_object_new (TNY_TYPE_MSG, NULL);

	tny_msg_set_header (TNY_MSG_IFACE (self), header);

	return self;
}


static void
tny_msg_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyMsgIfaceClass *klass = (TnyMsgIfaceClass *)g_iface;

	klass->get_parts_func = tny_msg_get_parts;
	klass->get_header_func = tny_msg_get_header;
	klass->set_header_func = tny_msg_set_header;
	klass->add_part_func = tny_msg_add_part;
	klass->del_part_func = tny_msg_del_part;
	klass->get_folder_func = tny_msg_get_folder;

	return;
}

static void 
tny_msg_class_init (TnyMsgClass *class)
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

	priv->header = NULL;

	priv->message_lock = g_mutex_new ();
	priv->parts_lock = g_mutex_new ();
	priv->header_lock = g_mutex_new ();
	priv->folder_lock = g_mutex_new ();

	return;
}

GType 
tny_msg_get_type (void)
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

		type = g_type_register_static (TNY_TYPE_MIME_PART,
			"TnyMsg",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_MSG_IFACE,
			&tny_msg_iface_info);

	}

	return type;
}


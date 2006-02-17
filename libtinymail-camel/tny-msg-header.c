/* libtinymail - The Tiny Mail base library
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


/* The story of TnyMsgHeader, By Philip Van Hoof *
 *
 * TnyMsgHeader is a proxy for real subject CamelMessageInfo. The type doesn't
 * use a priv pointer, but in stead is an opaque structure. This is done for 
 * decreasing the amount of allocations (a priv pointer is an unnecessary 
 * allocation). Allocations are slow.
 *
 * The tree model is responsible for calling the uncache method whenever the
 * corresponding row becomes invisble.
 *
 * All kinds of dirty non-gobject-standard tricks have been applied to make this
 * type as fast as possible. Including modifying the TNY_MSG_ACCOUNT() macro and
 * removing the gobject type-check.
 *
 * It's possible this type sooner or later gets reimplemented without using
 * gobject at all. As at this moment, the full size of the GObject type is added
 * for no real reason.
 */

#include <glib.h>

#include <tny-msg-header-iface.h>
#include <tny-msg-header.h>

#include "tny-msg-folder-priv.h"
#include "tny-msg-header-priv.h"

#include <camel/camel-folder.h>
#include <camel/camel.h>
#include <camel/camel-folder-summary.h>

static GObjectClass *parent_class = NULL;

struct _TnyMsgHeader 
{
	GObject parent;

	gchar *uid;
	TnyMsgFolderIface *folder;
	CamelMessageInfo *message_info;
};

struct _TnyMsgHeaderClass 
{
	GObjectClass parent_class;
};

static void
tny_msg_header_set_camel_message_info_priv  (TnyMsgHeader *self, CamelMessageInfo *camel_message_info)
{
	/* camel_message_info_ref (camel_message_info); */
	self->message_info = camel_message_info;

	return;
}

static void
unload_msg_header (TnyMsgHeader *self)
{
	if (self->message_info) 
		camel_message_info_free (self->message_info);

	self->message_info = NULL;

	return;
}

static void
load_msg_header (TnyMsgHeader *self)
{
	if (!self->message_info && self->folder && self->uid)
	{
		CamelFolder *folder = _tny_msg_folder_get_camel_folder (self->folder);
		CamelMessageInfo *msginfo = camel_folder_get_message_info (folder, self->uid);

		tny_msg_header_set_camel_message_info_priv (self, msginfo);
	}

	return;
}

void /* public version of this method */
_tny_msg_header_set_camel_message_info (TnyMsgHeader *self, CamelMessageInfo *camel_message_info)
{
	tny_msg_header_set_camel_message_info_priv (self, camel_message_info);

	return;
}

const TnyMsgFolderIface* 
tny_msg_header_get_folder (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	return me->folder;
}


void
tny_msg_header_set_folder (TnyMsgHeaderIface *self, const TnyMsgFolderIface* folder)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	me->folder = (TnyMsgFolderIface*)folder;
	return;
}

/* TODO: Add these
camel_message_info_mlist     
camel_message_info_flags     
camel_message_info_size      
camel_message_info_message_id
*/

static const gchar*
tny_msg_header_get_cc (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	load_msg_header (me);

	return camel_message_info_cc (me->message_info);
}

static const time_t
tny_msg_header_get_date_received (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	load_msg_header (me);

	return camel_message_info_date_received (me->message_info);
}

static const time_t
tny_msg_header_get_date_sent (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	load_msg_header (me);

	return camel_message_info_date_sent (me->message_info);
}
	
static const gchar*
tny_msg_header_get_from (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	load_msg_header (me);

	return camel_message_info_from (me->message_info);
}

static const gchar*
tny_msg_header_get_subject (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	load_msg_header (me);

	return camel_message_info_subject (me->message_info);
}


static const gchar*
tny_msg_header_get_to (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	load_msg_header (me);

	return camel_message_info_to (me->message_info);
}

static const gchar*
tny_msg_header_get_message_id (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	load_msg_header (me);

	return (const gchar*)camel_message_info_message_id (me->message_info);
}


static const gchar*
tny_msg_header_get_uid (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	load_msg_header (me);

	return camel_message_info_uid (me->message_info);
}

static void
tny_msg_header_set_uid (TnyMsgHeaderIface *self, const gchar *uid)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	unload_msg_header (me);

	/* Speedup trick, also check tny-msg-folder.c */
	/* if (priv->uid)
		g_free (priv->uid);
	priv->uid = g_strdup (uid); */

	/* Yes I know what I'm doing, also check tny-msg-folder.c */
	me->uid = (gchar*)uid;

	return;
}


static const gboolean 
tny_msg_header_has_cache (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	return (me->message_info != NULL);
}

static void
tny_msg_header_uncache (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *me = TNY_MSG_HEADER (self);

	if (me->message_info)
		unload_msg_header (me);

	return;
}

static void
tny_msg_header_finalize (GObject *object)
{
	TnyMsgHeader *self = (TnyMsgHeader*) object;

	if (self->message_info)
		unload_msg_header (self);

	/* Indeed, check the speedup trick above */
	/* if (self->uid)
		g_free (self->uid); */

	(*parent_class->finalize) (object);

	return;
}

TnyMsgHeader*
tny_msg_header_new (void)
{
	TnyMsgHeader *self = g_object_new (TNY_MSG_HEADER_TYPE, NULL);
	
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

	klass->get_uid_func = tny_msg_header_get_uid;	
	klass->set_uid_func = tny_msg_header_set_uid;
	
	klass->set_folder_func = tny_msg_header_set_folder;
	klass->get_folder_func = tny_msg_header_get_folder;

	klass->has_cache_func = tny_msg_header_has_cache;
	klass->uncache_func = tny_msg_header_uncache;

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

	if (type == 0) 
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

		g_type_add_interface_static (type, TNY_MSG_HEADER_IFACE_TYPE, 
			&tny_msg_header_iface_info);
	}

	return type;
}

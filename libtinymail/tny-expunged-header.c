/* libtinymailui-gtk - The Tiny Mail UI library for Gtk+
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

#include <glib.h>

#define TINYMAIL_ENABLE_PRIVATE_API
#include "tny-common-priv.h"
#undef TINYMAIL_ENABLE_PRIVATE_API

#include <tny-header.h>

static GObjectClass *parent_class = NULL;

static const gchar*
tny_expunged_header_get_uid (TnyHeader *self)
{
	return "...";
}

static const gchar*
tny_expunged_header_get_bcc (TnyHeader *self)
{
	return "Expunged";
}

static const gchar*
tny_expunged_header_get_cc (TnyHeader *self)
{
	return "Expunged";
}

static const gchar*
tny_expunged_header_get_subject (TnyHeader *self)
{
	return "Expunged";
}

static const gchar*
tny_expunged_header_get_to (TnyHeader *self)
{
	return "Expunged";
}

static const gchar*
tny_expunged_header_get_from (TnyHeader *self)
{
	return "Expunged";
}

static const gchar*
tny_expunged_header_get_replyto (TnyHeader *self)
{
	return "Expunged";
}

static const gchar*
tny_expunged_header_get_message_id (TnyHeader *self)
{
	return "Expunged";
}

static guint
tny_expunged_header_get_message_size (TnyHeader *self)
{
	return 0;
}

static time_t
tny_expunged_header_get_date_received (TnyHeader *self)
{
	return -1;
}

static time_t
tny_expunged_header_get_date_sent (TnyHeader *self)
{
	return -1;
}

static void
tny_expunged_header_set_bcc (TnyHeader *self, const gchar *bcc)
{
}

static void
tny_expunged_header_set_cc (TnyHeader *self, const gchar *cc)
{
}

static void
tny_expunged_header_set_from (TnyHeader *self, const gchar *from)
{
}

static void
tny_expunged_header_set_subject (TnyHeader *self, const gchar *subject)
{
}

static void
tny_expunged_header_set_to (TnyHeader *self, const gchar *to)
{
}

static void
tny_expunged_header_set_replyto (TnyHeader *self, const gchar *to)
{
}

static TnyFolder*
tny_expunged_header_get_folder (TnyHeader *self)
{
	return NULL;
}

static TnyHeaderFlags
tny_expunged_header_get_flags (TnyHeader *self)
{
	return TNY_HEADER_FLAG_EXPUNGED;
}

static void
tny_expunged_header_set_flag (TnyHeader *self, TnyHeaderFlags mask)
{
}

static void
tny_expunged_header_unset_flag (TnyHeader *self, TnyHeaderFlags mask)
{
}

static void
tny_expunged_header_finalize (GObject *object)
{
	parent_class->finalize (object);
}

static void
tny_expunged_header_instance_init (GTypeInstance *instance, gpointer g_class)
{
}

static void
tny_header_init (TnyHeaderIface *klass)
{
	klass->get_uid_func = tny_expunged_header_get_uid;
	klass->get_bcc_func = tny_expunged_header_get_bcc;
	klass->get_cc_func = tny_expunged_header_get_cc;
	klass->get_subject_func = tny_expunged_header_get_subject;
	klass->get_to_func = tny_expunged_header_get_to;
	klass->get_from_func = tny_expunged_header_get_from;
	klass->get_replyto_func = tny_expunged_header_get_replyto;
	klass->get_message_id_func = tny_expunged_header_get_message_id;
	klass->get_message_size_func = tny_expunged_header_get_message_size;
	klass->get_date_received_func = tny_expunged_header_get_date_received;
	klass->get_date_sent_func = tny_expunged_header_get_date_sent;
	klass->set_bcc_func = tny_expunged_header_set_bcc;
	klass->set_cc_func = tny_expunged_header_set_cc;
	klass->set_from_func = tny_expunged_header_set_from;
	klass->set_subject_func = tny_expunged_header_set_subject;
	klass->set_to_func = tny_expunged_header_set_to;
	klass->set_replyto_func = tny_expunged_header_set_replyto;
	klass->get_folder_func = tny_expunged_header_get_folder;
	klass->get_flags_func = tny_expunged_header_get_flags;
	klass->set_flag_func = tny_expunged_header_set_flag;
	klass->unset_flag_func = tny_expunged_header_unset_flag;
}

static void
tny_expunged_header_class_init (TnyExpungedHeaderClass *klass)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;
	object_class->finalize = tny_expunged_header_finalize;
}

TnyHeader*
tny_expunged_header_new (void)
{
	return (TnyHeader *) g_object_new (TNY_TYPE_EXPUNGED_HEADER, NULL);
}

GType
tny_expunged_header_get_type (void)
{
	static GType type = 0;
	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
			sizeof (TnyExpungedHeaderClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			(GClassInitFunc) tny_expunged_header_class_init,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			sizeof (TnyExpungedHeader),
			0,      /* n_preallocs */
			tny_expunged_header_instance_init,    /* instance_init */
			NULL
		};


		static const GInterfaceInfo tny_header_info = 
		{
			(GInterfaceInitFunc) tny_header_init, /* interface_init */
			NULL,         /* interface_finalize */
			NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyExpungedHeader",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_HEADER,
			&tny_header_info);

	}
	return type;
}

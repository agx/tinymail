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
#include <glib.h>
#include <glib/gi18n-lib.h>

#include <tny-camel-partial-msg-receive-strategy.h>

#include <tny-camel-folder.h>
#include <tny-error.h>
#include <tny-msg.h>
#include <tny-header.h>
#include <tny-camel-msg.h>
#include <tny-camel-header.h>

#include <camel/camel-folder.h>
#include <camel/camel.h>
#include <camel/camel-session.h>
#include <camel/camel-store.h>

#include "tny-camel-folder-priv.h"

static GObjectClass *parent_class = NULL;

static TnyMsg *
tny_camel_partial_msg_receive_strategy_perform_get_msg (TnyMsgReceiveStrategy *self, TnyFolder *folder, TnyHeader *header, GError **err)
{
	return TNY_CAMEL_PARTIAL_MSG_RECEIVE_STRATEGY_GET_CLASS (self)->perform_get_msg_func (self, folder, header, err);
}

static TnyMsg *
tny_camel_partial_msg_receive_strategy_perform_get_msg_default (TnyMsgReceiveStrategy *self, TnyFolder *folder, TnyHeader *header, GError **err)
{
	TnyCamelFolderPriv *priv = TNY_CAMEL_FOLDER_GET_PRIVATE (folder);
	TnyMsg *message = NULL;
	CamelMimeMessage *camel_message = NULL;
	const gchar *id;
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;

	g_assert (TNY_IS_HEADER (header));

	id = tny_header_get_uid (TNY_HEADER (header));

	message = NULL;
	camel_message = camel_folder_get_message (priv->folder, (const char *) id, CAMEL_FOLDER_RECEIVE_PARTIAL, -1, &ex);

	if (camel_exception_is_set (&ex))
	{
		g_set_error (err, TNY_FOLDER_ERROR, 
			TNY_FOLDER_ERROR_GET_MSG,
			camel_exception_get_description (&ex));
	} else 
	{
		if (camel_message && CAMEL_IS_OBJECT (camel_message))
		{
			TnyCamelHeader *nheader = TNY_CAMEL_HEADER (tny_camel_header_new ());

			message = tny_camel_msg_new ();
			_tny_camel_msg_set_folder (TNY_CAMEL_MSG (message), folder);
			_tny_camel_mime_part_set_part (TNY_CAMEL_MIME_PART (message), 
				CAMEL_MIME_PART (camel_message)); 
			_tny_camel_header_set_camel_mime_message (nheader, camel_message);
			_tny_camel_msg_set_header (TNY_CAMEL_MSG (message), nheader);
			g_object_unref (G_OBJECT (nheader));
		}
	}

	if (camel_message && CAMEL_IS_OBJECT (camel_message))
		camel_object_unref (CAMEL_OBJECT (camel_message));

	return message;
}

static void
tny_camel_partial_msg_receive_strategy_finalize (GObject *object)
{
	parent_class->finalize (object);
}

static void
tny_msg_receive_strategy_init (TnyMsgReceiveStrategyIface *klass)
{
	klass->perform_get_msg_func = tny_camel_partial_msg_receive_strategy_perform_get_msg;
}

static void
tny_camel_partial_msg_receive_strategy_class_init (TnyCamelPartialMsgReceiveStrategyClass *klass)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;

	klass->perform_get_msg_func = tny_camel_partial_msg_receive_strategy_perform_get_msg_default;

	object_class->finalize = tny_camel_partial_msg_receive_strategy_finalize;
}

static void 
tny_camel_partial_msg_receive_strategy_instance_init (GTypeInstance *instance, gpointer g_class)
{
	return;
}

/**
 * tny_camel_partial_msg_receive_strategy_new:
 * 
 * A message receiver that fetches partial messages (only the body)
 *
 * Return value: A new #TnyMsgReceiveStrategy instance implemented for Camel
 **/
TnyMsgReceiveStrategy* 
tny_camel_partial_msg_receive_strategy_new (void)
{
	TnyCamelPartialMsgReceiveStrategy *self = g_object_new (TNY_TYPE_CAMEL_PARTIAL_MSG_RECEIVE_STRATEGY, NULL);

	return TNY_MSG_RECEIVE_STRATEGY (self);
}


GType
tny_camel_partial_msg_receive_strategy_get_type (void)
{
	static GType type = 0;
	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
			sizeof (TnyCamelPartialMsgReceiveStrategyClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			(GClassInitFunc) tny_camel_partial_msg_receive_strategy_class_init,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			sizeof (TnyCamelPartialMsgReceiveStrategy),
			0,      /* n_preallocs */
			tny_camel_partial_msg_receive_strategy_instance_init,    /* instance_init */
			NULL
		};


		static const GInterfaceInfo tny_msg_receive_strategy_info = 
		{
			(GInterfaceInitFunc) tny_msg_receive_strategy_init, /* interface_init */
			NULL,         /* interface_finalize */
			NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyCamelPartialMsgReceiveStrategy",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_MSG_RECEIVE_STRATEGY,
			&tny_msg_receive_strategy_info);

	}
	return type;
}
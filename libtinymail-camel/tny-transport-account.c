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

#include <glib.h>

#include <string.h>

#include <tny-transport-account-iface.h>
#include <tny-transport-account.h>

#include <tny-msg-folder-iface.h>
#include <tny-msg-folder.h>

#include <camel/camel.h>
#include <camel/camel-session.h>
#include <camel/camel-store.h>


static GObjectClass *parent_class = NULL;

#include "tny-account-priv.h"
#include "tny-transport-account-priv.h"

#include <tny-camel-shared.h>

#define TNY_TRANSPORT_ACCOUNT_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_TRANSPORT_ACCOUNT, TnyTransportAccountPriv))


static void
tny_transport_account_send (TnyTransportAccountIface *self, TnyMsgIface *msg)
{
	TnyTransportAccountPriv *priv = TNY_TRANSPORT_ACCOUNT_GET_PRIVATE (self);
	TnyAccountPriv *apriv = TNY_ACCOUNT_GET_PRIVATE (self);

	/* TODO */

	return;
}


/**
 * tny_transport_account_new:
 * 
 *
 * Return value: A new #TnyTransportAccountIface instance implemented for Camel
 **/
TnyTransportAccount*
tny_transport_account_new (void)
{
	TnyTransportAccount *self = g_object_new (TNY_TYPE_TRANSPORT_ACCOUNT, NULL);

	return self;
}

static void
tny_transport_account_instance_init (GTypeInstance *instance, gpointer g_class)
{
	/*
	TnyTransportAccount *self = (TnyTransportAccount *)instance;
	TnyTransportAccountPriv *priv = TNY_TRANSPORT_ACCOUNT_GET_PRIVATE (self);
	*/

	return;
}


static void
tny_transport_account_finalize (GObject *object)
{
	/*
	TnyTransportAccount *self = (TnyTransportAccount *)object;	
	TnyTransportAccountPriv *priv = TNY_TRANSPORT_ACCOUNT_GET_PRIVATE (self);
	*/

	(*parent_class->finalize) (object);

	return;
}

static void
tny_transport_account_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyTransportAccountIfaceClass *klass = (TnyTransportAccountIfaceClass *)g_iface;

	klass->send_func = tny_transport_account_send;

	return;
}


static void 
tny_transport_account_class_init (TnyTransportAccountClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_transport_account_finalize;

	g_type_class_add_private (object_class, sizeof (TnyTransportAccountPriv));

	return;
}

GType 
tny_transport_account_get_type (void)
{
	static GType type = 0;

	if (!camel_type_init_done)
	{
		camel_type_init ();
		camel_type_init_done = TRUE;
	}

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyTransportAccountClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_transport_account_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyTransportAccount),
		  0,      /* n_preallocs */
		  tny_transport_account_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_transport_account_iface_info = 
		{
		  (GInterfaceInitFunc) tny_transport_account_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (TNY_TYPE_ACCOUNT,
			"TnyTransportAccount",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_TRANSPORT_ACCOUNT_IFACE, 
			&tny_transport_account_iface_info);
	}

	return type;
}


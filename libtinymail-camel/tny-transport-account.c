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

#include <string.h>

#include <tny-transport-account-iface.h>
#include <tny-transport-account.h>

#include <tny-folder-iface.h>
#include <tny-folder.h>

#include <camel/camel.h>
#include <camel/camel-session.h>
#include <camel/camel-store.h>


static GObjectClass *parent_class = NULL;

#include <tny-msg.h>
#include <tny-header.h>
#include <tny-transport-account.h>

#include "tny-camel-common-priv.h"
#include "tny-msg-priv.h"
#include "tny-header-priv.h"
#include "tny-account-priv.h"
#include "tny-transport-account-priv.h"

#include <tny-camel-shared.h>

#define TNY_TRANSPORT_ACCOUNT_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_TRANSPORT_ACCOUNT, TnyTransportAccountPriv))


static void 
tny_transport_account_reconnect (TnyAccount *self)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);

	if (G_LIKELY (priv->session) && G_UNLIKELY (priv->proto) && G_UNLIKELY (priv->host))
	{
		GString *urlstr = g_string_new ("");

		if (priv->url_string)
			g_free (priv->url_string);

		urlstr = g_string_append (urlstr, priv->proto);
		urlstr = g_string_append (urlstr, "://");

		if (priv->user)
		{
			urlstr = g_string_append (urlstr, priv->user);
			urlstr = g_string_append (urlstr, "@");
		}

		urlstr = g_string_append (urlstr, priv->host);

		priv->url_string = urlstr->str;

		g_string_free (urlstr, FALSE);
	}
}

static void
tny_transport_account_send (TnyTransportAccountIface *self, TnyMsgIface *msg)
{
	TnyAccountPriv *apriv = TNY_ACCOUNT_GET_PRIVATE (self);
	TnyHeaderIface *header = (TnyHeaderIface *)tny_msg_iface_get_header (msg);
	CamelMimeMessage *message = _tny_msg_get_camel_mime_message (TNY_MSG (msg));
	CamelException ex =  CAMEL_EXCEPTION_INITIALISER;
	CamelTransport *transport;

	transport = camel_session_get_transport (CAMEL_SESSION (apriv->session), 
			apriv->url_string, &ex);

	if (G_LIKELY (transport && message && header))
	{
		CamelInternetAddress 
			*from = camel_internet_address_new (),
			*recipients = camel_internet_address_new ();
		const gchar *str = NULL;

		str = tny_header_iface_get_from (header);
		_foreach_email_add_to_inet_addr (str, from);

		str = tny_header_iface_get_to (header);
		_foreach_email_add_to_inet_addr (str, recipients);

		apriv->connected = TRUE;

		camel_transport_send_to (transport, message, (CamelAddress*)from, 
			(CamelAddress*)recipients, &ex);

		apriv->connected = FALSE;

		camel_object_unref (CAMEL_OBJECT (from));
		camel_object_unref (CAMEL_OBJECT (recipients));
	}

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
	TnyTransportAccount *self = (TnyTransportAccount *)instance;
	TnyAccountPriv *apriv = TNY_ACCOUNT_GET_PRIVATE (self);
	
	apriv->connected = FALSE;
	apriv->type = CAMEL_PROVIDER_TRANSPORT;
	apriv->account_type = TNY_ACCOUNT_TYPE_TRANSPORT;

	return;
}


static void
tny_transport_account_finalize (GObject *object)
{
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

	((TnyAccountClass*)class)->reconnect_func = tny_transport_account_reconnect;

	g_type_class_add_private (object_class, sizeof (TnyTransportAccountPriv));

	return;
}

GType 
tny_transport_account_get_type (void)
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


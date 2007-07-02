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

#include <string.h>

#include <tny-transport-account.h>
#include <tny-camel-transport-account.h>

#include <tny-folder.h>
#include <tny-camel-folder.h>
#include <tny-error.h>

#include <camel/camel.h>
#include <camel/camel-session.h>
#include <camel/camel-store.h>


static GObjectClass *parent_class = NULL;

#include <tny-camel-msg.h>
#include <tny-camel-header.h>
#include <tny-camel-transport-account.h>
#include <tny-status.h>
#define TINYMAIL_ENABLE_PRIVATE_API
#include "tny-common-priv.h"
#undef TINYMAIL_ENABLE_PRIVATE_API

#include "tny-camel-common-priv.h"
#include "tny-camel-msg-priv.h"
#include "tny-camel-header-priv.h"
#include "tny-camel-account-priv.h"
#include "tny-camel-transport-account-priv.h"

#include <tny-camel-shared.h>

#define TNY_CAMEL_TRANSPORT_ACCOUNT_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_CAMEL_TRANSPORT_ACCOUNT, TnyCamelTransportAccountPriv))

static void 
tny_camel_transport_account_prepare (TnyCamelAccount *self, gboolean recon_if, gboolean reservice)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	_tny_camel_account_refresh (self, recon_if);

	g_static_rec_mutex_lock (apriv->service_lock);

	if (!apriv->session && reservice && apriv->url_string)
	{
		if (!apriv->service && reservice && apriv->url_string)
		{
			if (apriv->service && CAMEL_IS_SERVICE (apriv->service))
			{
				camel_object_unref (CAMEL_OBJECT (apriv->service));
				apriv->service = NULL;
			} 

			if (camel_exception_is_set (apriv->ex))
				camel_exception_clear (apriv->ex);

			apriv->service = camel_session_get_service
				((CamelSession*) apriv->session, apriv->url_string, 
				apriv->type, apriv->ex);

			if (apriv->service && !camel_exception_is_set (apriv->ex)) 
			{
				apriv->service->data = self;
				apriv->service->connecting = (con_op) NULL;
				apriv->service->disconnecting = (con_op) NULL;
				apriv->service->reconnecter = (con_op) NULL;
				apriv->service->reconnection = (con_op) NULL;

			} else if (camel_exception_is_set (apriv->ex) && apriv->service)
			{
				g_warning ("Must cleanup service pointer\n");
				apriv->service = NULL;
			}
		}
	} else {
		camel_exception_set (apriv->ex, CAMEL_EXCEPTION_SYSTEM,
			_("Session not yet set, use tny_camel_account_set_session"));
	}

	g_static_rec_mutex_unlock (apriv->service_lock);

	return;
}

static void 
tny_camel_transport_account_try_connect (TnyAccount *self, GError **err)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	if (!apriv->url_string || !apriv->service || !CAMEL_IS_SERVICE (apriv->service))
	{
		if (camel_exception_is_set (apriv->ex))
		{
			g_set_error (err, TNY_ACCOUNT_ERROR, 
				TNY_ACCOUNT_ERROR_TRY_CONNECT,
				camel_exception_get_description (apriv->ex));
			camel_exception_clear (apriv->ex);
		} else {
			g_set_error (err, TNY_ACCOUNT_ERROR, 
				TNY_ACCOUNT_ERROR_TRY_CONNECT,
				"Account not yet fully configured. "
				"This problem indicates a bug in the software.");
		}

		return;
	}

	if (apriv->pass_func_set && apriv->forget_pass_func_set)
	{
		if (camel_exception_is_set (apriv->ex))
			camel_exception_clear (apriv->ex);

	} else {
			g_set_error (err, TNY_ACCOUNT_ERROR, 
				TNY_ACCOUNT_ERROR_TRY_CONNECT,
				"Get and Forget password functions not yet set "
				"This problem indicates a bug in the software.");
	}

}


static void
tny_camel_transport_account_send (TnyTransportAccount *self, TnyMsg *msg, GError **err)
{
	TNY_CAMEL_TRANSPORT_ACCOUNT_GET_CLASS (self)->send_func (self, msg, err);
	return;
}


static void
tny_camel_transport_account_send_default (TnyTransportAccount *self, TnyMsg *msg, GError **err)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	CamelMimeMessage *message;
	CamelException ex =  CAMEL_EXCEPTION_INITIALISER;
	CamelTransport *transport;
	CamelAddress *from, *recipients;
	gboolean reperr = TRUE;

	g_assert (CAMEL_IS_SESSION (apriv->session));
	g_assert (TNY_IS_CAMEL_MSG (msg));

	if (apriv->service == NULL || !CAMEL_IS_SERVICE (apriv->service))
	{
		g_set_error (err, TNY_TRANSPORT_ACCOUNT_ERROR, 
				TNY_TRANSPORT_ACCOUNT_ERROR_SEND,
				"Account not ready for this operation (%s). "
				"This problem indicates a bug in the software.",
				camel_exception_get_description (apriv->ex));
		return;
	}

	transport = (CamelTransport *) apriv->service;
	g_assert (CAMEL_IS_TRANSPORT (transport));

	/* TODO: Why not simply use tny_account_try_connect here ? */

	g_static_rec_mutex_lock (apriv->service_lock);
	/* camel_service_connect can launch GUI things */

	if (!apriv->service || !camel_service_connect (apriv->service, &ex))
	{
		if (camel_exception_is_set (&ex))
		{
			g_set_error (err, TNY_TRANSPORT_ACCOUNT_ERROR, 
				TNY_TRANSPORT_ACCOUNT_ERROR_SEND,
				camel_exception_get_description (&ex));
			g_static_rec_mutex_unlock (apriv->service_lock);
			return;
		}
	} 

	g_static_rec_mutex_unlock (apriv->service_lock);

	message = _tny_camel_msg_get_camel_mime_message (TNY_CAMEL_MSG (msg));
	from = (CamelAddress *) camel_mime_message_get_from (message);
	recipients = (CamelAddress *) camel_mime_message_get_recipients (message, CAMEL_RECIPIENT_TYPE_TO);

	camel_transport_send_to (transport, message, from, 
			recipients, &ex);

	if (camel_exception_is_set (&ex))
	{
		g_set_error (err, TNY_TRANSPORT_ACCOUNT_ERROR, 
			TNY_TRANSPORT_ACCOUNT_ERROR_SEND,
			camel_exception_get_description (&ex));
		camel_exception_clear (&ex);
		reperr = FALSE;
	}

	camel_service_disconnect (apriv->service, TRUE, &ex);

	if (reperr && camel_exception_is_set (&ex))
	{
		g_set_error (err, TNY_TRANSPORT_ACCOUNT_ERROR, 
			TNY_TRANSPORT_ACCOUNT_ERROR_SEND,
			camel_exception_get_description (&ex));
		camel_exception_clear (&ex);
	}

	/*g_object_unref (G_OBJECT (header));*/

	return;
}


/**
 * tny_camel_transport_account_new:
 * 
 * Create a new #TnyTransportAccount instance implemented for Camel
 * 
 * Return value: A new #TnyTransportAccount instance implemented for Camel
 **/
TnyTransportAccount*
tny_camel_transport_account_new (void)
{
	TnyCamelTransportAccount *self = g_object_new (TNY_TYPE_CAMEL_TRANSPORT_ACCOUNT, NULL);

	return TNY_TRANSPORT_ACCOUNT (self);
}

static void
tny_camel_transport_account_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyCamelTransportAccount *self = (TnyCamelTransportAccount *)instance;
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	apriv->service = NULL;
	apriv->type = CAMEL_PROVIDER_TRANSPORT;
	apriv->account_type = TNY_ACCOUNT_TYPE_TRANSPORT;

	if (apriv->mech) /* Remove the PLAIN default */
		g_free (apriv->mech);
	apriv->mech = NULL;

	return;
}


static void
tny_camel_transport_account_finalize (GObject *object)
{
	(*parent_class->finalize) (object);

	return;
}

static void
tny_transport_account_init (gpointer g, gpointer iface_data)
{
	TnyTransportAccountIface *klass = (TnyTransportAccountIface *)g;

	klass->send_func = tny_camel_transport_account_send;

	return;
}


static void 
tny_camel_transport_account_class_init (TnyCamelTransportAccountClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	class->send_func = tny_camel_transport_account_send_default;

	object_class->finalize = tny_camel_transport_account_finalize;

	TNY_CAMEL_ACCOUNT_CLASS (class)->prepare_func = tny_camel_transport_account_prepare;
	TNY_CAMEL_ACCOUNT_CLASS (class)->try_connect_func = tny_camel_transport_account_try_connect;

	g_type_class_add_private (object_class, sizeof (TnyCamelTransportAccountPriv));

	return;
}

/**
 * tny_camel_transport_account_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
GType 
tny_camel_transport_account_get_type (void)
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
		  sizeof (TnyCamelTransportAccountClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_camel_transport_account_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyCamelTransportAccount),
		  0,      /* n_preallocs */
		  tny_camel_transport_account_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_transport_account_info = 
		{
		  (GInterfaceInitFunc) tny_transport_account_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (TNY_TYPE_CAMEL_ACCOUNT,
			"TnyCamelTransportAccount",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_TRANSPORT_ACCOUNT, 
			&tny_transport_account_info);
	}

	return type;
}


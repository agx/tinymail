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
#include <glib-object.h>
#include <glib/gi18n-lib.h>
#include <tny-maemo-device.h>
#include <coniciap.h>
#include <conicconnection.h>
#include <conicconnectionevent.h>


static GObjectClass *parent_class = NULL;

typedef struct {
	ConIcConnection *cnx;
	gboolean        is_online;
} TnyMaemoDevicePriv;

#define TNY_MAEMO_DEVICE_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_MAEMO_DEVICE, TnyMaemoDevicePriv))


static void 
tny_maemo_device_reset (TnyDevice *self)
{
	//TnyMaemoDevicePriv *priv = TNY_MAEMO_DEVICE_GET_PRIVATE (self);
	/* FIXME: hmm... what to do here? */
}


static void
on_connection_event (ConIcConnection *self, ConIcConnectionEvent *event, gpointer user_data)
{
	TnyMaemoDevice *device; 
	TnyMaemoDevicePriv *priv;
	
	g_return_if_fail (event);
	g_return_if_fail (user_data);

	device = TNY_MAEMO_DEVICE(user_data);
	priv   = TNY_MAEMO_DEVICE_GET_PRIVATE (device);

	
	switch (con_ic_connection_event_get_error(event)) {
	case CON_IC_CONNECTION_ERROR_NONE:
		break;
	case CON_IC_CONNECTION_ERROR_INVALID_IAP:
		g_warning ("conic: IAP is invalid");
		break;
	case CON_IC_CONNECTION_ERROR_CONNECTION_FAILED:
		g_warning ("conic: connection failed");
		break;
	case CON_IC_CONNECTION_ERROR_USER_CANCELED:
		g_warning ("conic: user cancelled");
		break;
	default:
		g_return_if_reached ();
	}

	switch (con_ic_connection_event_get_status(event)) {
		
	case CON_IC_STATUS_CONNECTED:
		priv->is_online = TRUE;
		g_signal_emit (device, tny_device_signals [TNY_DEVICE_CONNECTION_CHANGED],
			       0, TRUE);
		break;
	case CON_IC_STATUS_DISCONNECTED:		
		priv->is_online = FALSE;
		g_signal_emit (device, tny_device_signals [TNY_DEVICE_CONNECTION_CHANGED],
			       0, FALSE);
		break;
	case CON_IC_STATUS_DISCONNECTING:
		priv->is_online = FALSE;
		break;
	default:
		g_return_if_reached (); /* should not happen */
	}
}



static void 
tny_maemo_device_force_online (TnyDevice *self)
{
	TnyMaemoDevicePriv *priv;	
	
	g_return_if_fail (TNY_IS_DEVICE(self));
	priv   = TNY_MAEMO_DEVICE_GET_PRIVATE (self);

	if (!priv->is_online) 
		if (!con_ic_connection_connect (priv->cnx,CON_IC_CONNECT_FLAG_NONE))
			g_warning ("could not send connect dbus message");
}


static void
tny_maemo_device_force_offline (TnyDevice *self)
{
	TnyMaemoDevicePriv *priv;	

	g_return_if_fail (TNY_IS_DEVICE(self));
	priv   = TNY_MAEMO_DEVICE_GET_PRIVATE (self);

	if (priv->is_online)
		if (!con_ic_connection_disconnect (priv->cnx))
			g_warning ("could not send disconnect dbus message");
}


static gboolean
tny_maemo_device_is_online (TnyDevice *self)
{
	g_return_val_if_fail (TNY_IS_DEVICE(self), FALSE);
	
	/* FIXME: get the info directly from iap? */
	return TNY_MAEMO_DEVICE_GET_PRIVATE (self)->is_online;
}

static void
tny_maemo_device_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyMaemoDevice *self = (TnyMaemoDevice *)instance;
	TnyMaemoDevicePriv *priv = TNY_MAEMO_DEVICE_GET_PRIVATE (self);
	
	priv->is_online     = FALSE; 
}



/**
 * tny_maemo_device_new:
 *
 * Return value: A new #TnyDevice instance
 **/
TnyDevice*
tny_maemo_device_new (void)
{
	TnyMaemoDevice *self; 
	TnyMaemoDevicePriv *priv;

	self = g_object_new (TNY_TYPE_MAEMO_DEVICE, NULL);
	priv   = TNY_MAEMO_DEVICE_GET_PRIVATE (self);

	priv->cnx = con_ic_connection_new ();
	if (!priv->cnx) {
		g_warning ("con_ic_connection_new failed");
		g_object_unref (self);
		return NULL;
	}
	g_signal_connect (priv->cnx, "connection-event",
			  G_CALLBACK(on_connection_event), self);
	
	/*
	 * this will get us in connected state, but only if there is already a connection.
	 * thus, this will setup our state correctly when we receive the signals
	 */
	if (!con_ic_connection_connect (priv->cnx,CON_IC_CONNECT_FLAG_AUTOMATICALLY_TRIGGERED))
		g_warning ("could not send auto-connect dbus message");
	
	return TNY_DEVICE (self);
}


static void
tny_maemo_device_finalize (GObject *obj)
{
	TnyMaemoDevicePriv *priv;
	priv   = TNY_MAEMO_DEVICE_GET_PRIVATE (obj);

	con_ic_connection_disconnect (priv->cnx);
	g_object_unref (priv->cnx);
	priv->cnx = NULL;
	
	(*parent_class->finalize) (obj);
}


static void
tny_device_init (gpointer g, gpointer iface_data)
{
	TnyDeviceIface *klass = (TnyDeviceIface *)g;

	klass->is_online_func     = tny_maemo_device_is_online;
	klass->reset_func         = tny_maemo_device_reset;
	klass->force_offline_func = tny_maemo_device_force_offline;
	klass->force_online_func  = tny_maemo_device_force_online;
}



static void 
tny_maemo_device_class_init (TnyMaemoDeviceClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_maemo_device_finalize;

	g_type_class_add_private (object_class, sizeof (TnyMaemoDevicePriv));
}

GType 
tny_maemo_device_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMaemoDeviceClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_maemo_device_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMaemoDevice),
		  0,      /* n_preallocs */
		  tny_maemo_device_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_device_info = 
		{
		  (GInterfaceInitFunc) tny_device_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyMaemoDevice",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_DEVICE, 
			&tny_device_info);

	}
	return type;
}

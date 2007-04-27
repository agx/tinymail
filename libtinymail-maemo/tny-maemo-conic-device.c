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
#include <tny-maemo-conic-device.h>

static gboolean tny_maemo_conic_device_is_online (TnyDevice *self);

static GObjectClass *parent_class = NULL;

typedef struct {
	ConIcConnection *cnx;
	gboolean        is_online;
	gchar     *iap;
} TnyMaemoConicDevicePriv;

#define TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_MAEMO_CONIC_DEVICE, TnyMaemoConicDevicePriv))


static void 
tny_maemo_conic_device_reset (TnyDevice *device)
{
	g_message (__FUNCTION__);
}


static void
on_connection_event (ConIcConnection *cnx, ConIcConnectionEvent *event, gpointer user_data)
{
	TnyMaemoConicDevice *device; 
	TnyMaemoConicDevicePriv *priv;
	gboolean is_online = FALSE;

	g_return_if_fail (CON_IC_IS_CONNECTION(cnx));
	g_return_if_fail (user_data);

	device = TNY_MAEMO_CONIC_DEVICE(user_data);
	priv   = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (device);

#ifdef MAEMO_CONIC_DUMMY
	g_message ("%s: HACK: outsmarting libconic by emitting signal regardless of the reported connection status.", __FUNCTION__);
	g_signal_emit (device, tny_device_signals [TNY_DEVICE_CONNECTION_CHANGED],
		       0, TRUE);
	return;
#endif /*MAEMO_CONIC_DUMMY*/

	
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
		priv->iap = g_strdup (con_ic_event_get_iap_id ((ConIcEvent*)(event)));
		is_online = TRUE;
		break;
	case CON_IC_STATUS_DISCONNECTED:
		priv->iap = NULL;
		is_online = FALSE;
		break;
	case CON_IC_STATUS_DISCONNECTING:
		is_online = FALSE;
		break;
	default:
		g_return_if_reached (); 
	}

	priv->is_online = is_online;
	g_signal_emit (device, tny_device_signals [TNY_DEVICE_CONNECTION_CHANGED],
		       0, is_online);
}



/**
 * tny_maemo_conic_device_connect:
 * @self: a #TnyDevice object
 * @iap_id: the id of the Internet Access Point (IAP), or NULL for 'any;
 * 
 * try to connect to a specific IAP, or to any if @iap_id == NULL
 * this calls con_ic_connection_connect(_by_id)
 * 
 * Returns TRUE if sending the command worked, FALSE otherwise
 **/
gboolean
tny_maemo_conic_device_connect (TnyMaemoConicDevice *self, const gchar* iap_id)
{
	TnyMaemoConicDevicePriv *priv;

	g_return_if_fail (TNY_IS_DEVICE(self));
	priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);
	
	g_return_val_if_fail (priv->cnx, FALSE);

	if (iap_id) {
		if (!con_ic_connection_connect_by_id (priv->cnx, iap_id, CON_IC_CONNECT_FLAG_NONE)) {
			g_warning ("could not send connect_by_id dbus message");
			return FALSE;
		}
	} else
		if (!con_ic_connection_connect (priv->cnx, CON_IC_CONNECT_FLAG_NONE)) {
			g_warning ("could not send connect dbus message");
			return FALSE;
		}
	
	return TRUE;
}


/**
 * tny_maemo_conic_device_disconnect:
 * @self: a #TnyDevice object
 * @iap_id: the id of the Internet Access Point (IAP), or NULL for 'any';
 * 
 * try to disconnect from a specific IAP, or to any if @iap_id == NULL
 * this calls con_ic_connection_disconnect(_by_id)
 * 
 * Returns TRUE if sending the command worked, FALSE otherwise
 **/
gboolean
tny_maemo_conic_device_disconnect (TnyMaemoConicDevice *self, const gchar* iap_id)
{
	TnyMaemoConicDevicePriv *priv;	
	
	g_return_if_fail (TNY_IS_MAEMO_CONIC_DEVICE(self));
	g_return_val_if_fail (priv->cnx, FALSE);

	priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);


	if (iap_id) {
		if (!con_ic_connection_disconnect_by_id (priv->cnx, iap_id)) {
			g_warning ("could not send disconnect_by_id dbus message");
			return FALSE;
		}
	} else
		if (!con_ic_connection_disconnect (priv->cnx)) {
			g_warning ("could not send disconnect dbus message");
			return FALSE;
		}
	return TRUE;
}



/**
 * tny_maemo_conic_device_get_current_iap_id:
 * @self: a #TnyDevice object
 * 
 * retrieve the iap-id of the connection that is currently active; a precondition is
 * that we _are_ connected.
 * 
 * Returns: the iap-id for the current connection, or NULL in case of error
 **/
const gchar*
tny_maemo_conic_device_get_current_iap_id (TnyMaemoConicDevice *self)
{
	TnyMaemoConicDevicePriv *priv;
	
	g_return_val_if_fail (TNY_IS_MAEMO_CONIC_DEVICE(self), NULL);
	g_return_val_if_fail (tny_maemo_conic_device_is_online(TNY_DEVICE(self)), NULL);

	priv   = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);
	
	return priv->iap;
}


/**
 * tny_maemo_conic_device_get_iap:
 * @self: a #TnyDevice object
 * @iap_id: the id of the IAP to get
 * 
 * get the IAP object (#ConIcIap) for the given iap-id. The returned GObject must be
 * freed with g_object_unref after use. Refer to the ConIc documentation for details about
 * the #ConICIap.
 
 * 
 * Returns: ConIcIap object or NULL in case of error
 **/
ConIcIap*
tny_maemo_conic_device_get_iap (TnyMaemoConicDevice *self, const gchar *iap_id)
{
	TnyMaemoConicDevicePriv *priv;

	g_return_val_if_fail (TNY_IS_MAEMO_CONIC_DEVICE(self), NULL);
	g_return_val_if_fail (iap_id, NULL);
	g_return_val_if_fail (priv->cnx, NULL);

	priv   = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);

	/* Note that it is very unusual to return a reference from a get_() function, 
	 * but we must do so because that mistake has already been made in 
	 * con_ic_connection_get_iap().
	 * If we just unref immediately then libconic might destroy the object.
	 */
	return con_ic_connection_get_iap (priv->cnx, iap_id);
}


/**
 * tny_maemo_conic_device_get_iap_list:
 * @self: a #TnyDevice object
 * 
 * get a list of all IAP objects (#ConIcIap) that are available; it should be freed
 * with #tny_maemo_conic_device_free_iap_list. This function uses
 * con_ic_connection_get_all_iaps
 *  
 * Returns: the list or NULL in case of error
 **/
GSList*
tny_maemo_conic_device_get_iap_list (TnyMaemoConicDevice *self)
{
	TnyMaemoConicDevicePriv *priv;
	
	g_return_val_if_fail (TNY_IS_MAEMO_CONIC_DEVICE(self), NULL);
	g_return_val_if_fail (priv->cnx, NULL);

	priv   = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);

	return con_ic_connection_get_all_iaps (priv->cnx);
}


/**
 * tny_maemo_conic_device_free_iap_list:
 * @self: a #TnyDevice object
 *  
 * free a  list of IAP objects retrieved from tny_maemo_conic_device_get_iap_list
 *  
 **/
void
tny_maemo_conic_device_free_iap_list (TnyMaemoConicDevice *self, GSList* cnx_list)
{
	GSList *cur = cnx_list;
	while (cur) {
		g_object_unref (G_OBJECT(cur->data));
		cur = g_slist_next (cur);
	}
	g_slist_free (cnx_list);
}



static void 
tny_maemo_conic_device_force_online (TnyDevice *self)
{
	TnyMaemoConicDevicePriv *priv;	

#ifdef MAEMO_CONIC_DUMMY
	return;
#endif /*MAEMO_CONIC_DUMMY*/
	
	g_return_if_fail (TNY_IS_DEVICE(self));
	g_return_if_fail (priv->cnx);

	priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);

	if (!con_ic_connection_connect (priv->cnx, CON_IC_CONNECT_FLAG_NONE))
			g_warning ("could not send connect dbus message");
}


static void
tny_maemo_conic_device_force_offline (TnyDevice *self)
{
	TnyMaemoConicDevicePriv *priv;	

#ifdef MAEMO_CONIC_DUMMY
	return;
#endif /*MAEMO_CONIC_DUMMY*/

	g_return_if_fail (TNY_IS_DEVICE(self));
	g_return_if_fail (priv->cnx);

	priv   = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);

	if (!con_ic_connection_disconnect (priv->cnx))
		g_warning ("could not send disconnect dbus message");
}


static gboolean
tny_maemo_conic_device_is_online (TnyDevice *self)
{
#ifdef MAEMO_CONIC_DUMMY
	return TRUE;
#endif /*MAEMO_CONIC_DUMMY*/
	
	g_return_val_if_fail (TNY_IS_DEVICE(self), FALSE);	

	return TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self)->is_online;
}


static void
tny_maemo_conic_device_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyMaemoConicDevice *self = (TnyMaemoConicDevice *)instance;

	TnyMaemoConicDevicePriv *priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);
	priv->iap = NULL;
	priv->cnx = con_ic_connection_new ();
	if (!priv->cnx) {
		g_warning ("con_ic_connection_new failed. The TnyMaemoConicDevice will be useless.");
	}
	g_signal_connect (priv->cnx, "connection-event",
			  G_CALLBACK(on_connection_event), self);

	/*
	 * this will get us in connected state only if there is already a connection.
	 * thus, this will setup our state correctly when we receive the signals
	 */
	if (!con_ic_connection_connect (priv->cnx, CON_IC_CONNECT_FLAG_AUTOMATICALLY_TRIGGERED))
		g_warning ("could not send connect dbus message");
	
	
	priv->is_online     = FALSE; 
}



/**
 * tny_maemo_conic_device_new:
 *
 * Return value: A new #TnyDevice instance
 **/
TnyDevice*
tny_maemo_conic_device_new (void)
{
	TnyMaemoConicDevice *self = g_object_new (TNY_TYPE_MAEMO_CONIC_DEVICE, NULL);
	return TNY_DEVICE (self);
}

static void
tny_maemo_conic_device_finalize (GObject *obj)
{
	TnyMaemoConicDevicePriv *priv;
	priv   = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (obj);
	if (priv->cnx && CON_IC_IS_CONNECTION(priv->cnx)) {
		if (!con_ic_connection_disconnect (priv->cnx))
			g_warning ("failed to send disconnect dbus message");
		g_object_unref (priv->cnx);
		priv->cnx = NULL;
	}

	if (priv->iap) {
		g_free (priv->iap);
		priv->iap = NULL;
	}

	(*parent_class->finalize) (obj);
}


static void
tny_device_init (gpointer g, gpointer iface_data)
{
	TnyDeviceIface *klass = (TnyDeviceIface *)g;

	klass->is_online_func     = tny_maemo_conic_device_is_online;
	klass->reset_func         = tny_maemo_conic_device_reset;
	klass->force_offline_func = tny_maemo_conic_device_force_offline;
	klass->force_online_func  = tny_maemo_conic_device_force_online;
}



static void 
tny_maemo_conic_device_class_init (TnyMaemoConicDeviceClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_maemo_conic_device_finalize;

	g_type_class_add_private (object_class, sizeof (TnyMaemoConicDevicePriv));
}

GType 
tny_maemo_conic_device_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMaemoConicDeviceClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_maemo_conic_device_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMaemoConicDevice),
		  0,      /* n_preallocs */
		  tny_maemo_conic_device_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_device_info = 
		{
		  (GInterfaceInitFunc) tny_device_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyMaemoConicDevice",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_DEVICE, 
			&tny_device_info);

	}
	return type;
}

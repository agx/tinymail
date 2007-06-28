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
#include <conicevent.h>
#include <coniciap.h>
#include <conicconnection.h>
#include <conicconnectionevent.h>
#include <string.h> /* For strcmp() */
#include <stdio.h> /* for printf */

#ifdef MAEMO_CONIC_DUMMY
/* #include "coniciap-private.h"
 * This is not installed, so we predeclare 
 * the struct instead.
 * Of course, this is a hack and could break if the private API 
 * changes.
 * It would be better for libconic to support scratchbox.
 */
struct _ConIcIap 
{
	GObject parent_instance;
	gboolean dispose_has_run;
	gchar *id;
	gchar *name;
	gchar *bearer;
};

#define MAEMO_CONIC_DUMMY_IAP_ID_FILENAME "maemo_conic_dummy_id"
static gboolean on_dummy_connection_check (gpointer user_data);
#endif /* MAEMO_CONIC_DUMMY */

static gboolean tny_maemo_conic_device_is_online (TnyDevice *self);

static GObjectClass *parent_class = NULL;

typedef struct {
	ConIcConnection *cnx;
	gboolean        is_online;
	gchar     *iap;
	gboolean 	forced; /* Whether the is_online value is forced rather than real. */
	
	/* When TRUE, we are waiting for the success or failure signal. */
	gboolean attempting_connection;
	
#ifdef MAEMO_CONIC_DUMMY	
	gint dummy_env_check_timeout;
#endif /* MAEMO_CONIC_DUMMY */
} TnyMaemoConicDevicePriv;

#define TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_MAEMO_CONIC_DEVICE, TnyMaemoConicDevicePriv))


static void 
tny_maemo_conic_device_reset (TnyDevice *device)
{
	TnyMaemoConicDevice *self;
	TnyMaemoConicDevicePriv *priv;
	g_return_if_fail (TNY_IS_DEVICE(device));
	self = TNY_MAEMO_CONIC_DEVICE (device);
	priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);

	const gboolean status_before = tny_maemo_conic_device_is_online (device);

	priv->forced = FALSE;

	if (status_before != tny_maemo_conic_device_is_online (device))
		g_signal_emit (device, tny_device_signals [TNY_DEVICE_CONNECTION_CHANGED],
		       0, !status_before);
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

	g_message (__FUNCTION__);

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
		
		/* Set this to FALSE to stop blocking 
		 * tny_maemo_conic_device_connect(): */
		if (priv->attempting_connection)
			priv->attempting_connection = FALSE;
			
		g_message ("new status: CONNECTED (%s)", priv->iap);
		break;
	case CON_IC_STATUS_DISCONNECTED:
		priv->iap = NULL;
		is_online = FALSE;
		
		/* Set this to FALSE to stop blocking 
		 * tny_maemo_conic_device_connect(): */
		if (priv->attempting_connection)
			priv->attempting_connection = FALSE;
			
		g_message ("new status: DISCONNECTED");
		break;
	case CON_IC_STATUS_DISCONNECTING:
		is_online = FALSE;
		g_message ("new status: DISCONNECTING");
		break;
	default:
		g_return_if_reached (); 
	}

	priv->is_online = is_online;
	priv->forced = FALSE; /* is_online is now accurate. */
	g_message ("emitting signal CONNECTION_CHANGED: %s", is_online ? "online" : "offline");
	g_signal_emit (device, tny_device_signals [TNY_DEVICE_CONNECTION_CHANGED],
		       0, is_online);
}



/**
 * tny_maemo_conic_device_connect:
 * @self: a #TnyDevice object
 * @iap_id: the id of the Internet Access Point (IAP), or NULL for 'any;
 * 
 * Try to connect to a specific IAP, or to any if @iap_id == NULL
 * this calls con_ic_connection_connect(_by_id).
 * This may show a dialog to allow the user to select a connection, or 
 * may otherwise take a significant amount of time. This function blocks until 
 * the connection has either succeeded or failed.
 * 
 * Returns TRUE if a connection was made, FALSE otherwise.
 **/
gboolean
tny_maemo_conic_device_connect (TnyMaemoConicDevice *self, const gchar* iap_id)
{
	TnyMaemoConicDevicePriv *priv;

	g_return_val_if_fail (TNY_IS_DEVICE(self), FALSE);
	priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);
	
	g_return_val_if_fail (priv->cnx, FALSE);

	g_message (__FUNCTION__);
	g_message ("connecting to %s", iap_id ? iap_id : "<any>");
	
	if (iap_id) {
		priv->attempting_connection = TRUE;
		
		if (!con_ic_connection_connect_by_id (priv->cnx, iap_id, CON_IC_CONNECT_FLAG_NONE)) {
			g_warning ("could not send connect_by_id dbus message");
			return FALSE;
		}
	} else {
		priv->attempting_connection = TRUE;
		
		if (!con_ic_connection_connect (priv->cnx, CON_IC_CONNECT_FLAG_NONE)) {
			g_warning ("could not send connect dbus message");
			return FALSE;
		}
	}
	
	/* Wait for the CON_IC_STATUS_CONNECTED (succeeded) or 
	 * CON_IC_STATUS_DISCONNECTED event: */
	 
	/* When the signal has been handled, 
	 * attempting_connection will be reset to FALSE. */
	while (priv->attempting_connection) {
		/* Iterate the main loop so that the signal can be called. */
		if (g_main_context_pending (NULL)) {
			g_main_context_iteration (NULL, FALSE);
		}
	}
	
	return priv->is_online;
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
/* don't try to disconnect if we're in dummy mode, as we're not "really"
 * connected in that case either
 */
#ifndef MAEMO_CONIC_DUMMY 
	TnyMaemoConicDevicePriv *priv;	
	
	g_return_val_if_fail (TNY_IS_MAEMO_CONIC_DEVICE(self), FALSE);
	g_return_val_if_fail (priv->cnx, FALSE);

	priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);
	
	g_message (__FUNCTION__);
	g_message ("disconnecting from %s", iap_id ? iap_id : "<any>");

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
#endif /* MAEMO_CONIC_DUMMY*/
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
	g_return_val_if_fail (TNY_IS_MAEMO_CONIC_DEVICE(self), NULL);
	g_return_val_if_fail (tny_maemo_conic_device_is_online(TNY_DEVICE(self)), NULL);

	TnyMaemoConicDevicePriv *priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);
	
	#ifdef MAEMO_CONIC_DUMMY
	if (!(priv->iap)) {
		on_dummy_connection_check (self);
	}
	#endif
	
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
	g_return_val_if_fail (TNY_IS_MAEMO_CONIC_DEVICE(self), NULL);
	g_return_val_if_fail (iap_id, NULL);

	#ifdef MAEMO_CONIC_DUMMY 
	/* Note that we have re-declared the private struct so that we 
	 * can do this, which is very bad and fragile: */
 	ConIcIap *iap = g_object_new (CON_IC_TYPE_IAP, NULL);
 	iap->id = g_strdup(iap_id);
 	iap->name = g_strdup_printf("%s name", iap->id);
 	return iap;
	#else
	TnyMaemoConicDevicePriv *priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);
	g_return_val_if_fail (priv->cnx, NULL);

	/* Note that it is very unusual to return a reference from a get_() function, 
	 * but we must do so because that mistake has already been made in 
	 * con_ic_connection_get_iap().
	 * If we just unref immediately then libconic might destroy the object.
	 */
	return con_ic_connection_get_iap (priv->cnx, iap_id);
	#endif
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
	g_return_val_if_fail (TNY_IS_MAEMO_CONIC_DEVICE(self), NULL);

#ifdef MAEMO_CONIC_DUMMY
	/* libconic does not return a list of connections when running 
	 * in scratchbox, though it might do this in future when 
	 * "ethernet support" is implemented.
	 * So we return a fake list so we can exercise funcationality 
	 * that uses connections: */
	 GSList* result = NULL;
	 
	 int i = 0;
	 for (i = 0; i < 10; ++i) {
	 	/* con_ic_iap_new (id) would checkc for a gconf dir and 
	 	 * fails, though _new() functions should just call g_object_new().
	 	 *
	 	gchar *id = g_strdup_printf("debug id%d", i);
	 	ConIcIap *iap = con_ic_iap_new (id);
	 	g_free (id);
	 	*/
	 	
	 	/* Note that we have re-declared the private struct so that we 
	 	 * can do this, which is very bad and fragile: */
	 	ConIcIap *iap = g_object_new (CON_IC_TYPE_IAP, NULL);
	 	iap->id = g_strdup_printf("debug id%d", i);
	 	iap->name = g_strdup_printf("%s name", iap->id);

	 	result = g_slist_append (result, iap);	
	 }
	 
	 return result;
#else
	TnyMaemoConicDevicePriv *priv 
		= TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);
	g_return_val_if_fail (priv->cnx, NULL);

	return con_ic_connection_get_all_iaps (priv->cnx);
#endif
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
tny_maemo_conic_device_force_online (TnyDevice *device)
{
	TnyMaemoConicDevice *self;
	TnyMaemoConicDevicePriv *priv;
	g_return_if_fail (TNY_IS_DEVICE(device));
	self = TNY_MAEMO_CONIC_DEVICE (device);
	priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);

#ifdef MAEMO_CONIC_DUMMY
	return;
#endif /*MAEMO_CONIC_DUMMY*/	
	const gboolean already_online = tny_maemo_conic_device_is_online (device);

	g_message (__FUNCTION__);
	g_message ("force online, current status is: %s", already_online ? "online" : "offline");
	
	priv->forced = TRUE;

	/* Signal if it changed: */
	if (!already_online)
		g_signal_emit (device, tny_device_signals [TNY_DEVICE_CONNECTION_CHANGED],
		       0, TRUE);
}


static void
tny_maemo_conic_device_force_offline (TnyDevice *device)
{
	TnyMaemoConicDevice *self;
	TnyMaemoConicDevicePriv *priv;
	g_return_if_fail (TNY_IS_DEVICE(device));
	self = TNY_MAEMO_CONIC_DEVICE (device);
	priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);

#ifdef MAEMO_CONIC_DUMMY
	return;
#endif /*MAEMO_CONIC_DUMMY*/



	const gboolean already_offline = !tny_maemo_conic_device_is_online (device);

	priv->forced = TRUE;

	/* Signal if it changed: */
	if (!already_offline)
		g_signal_emit (device, tny_device_signals [TNY_DEVICE_CONNECTION_CHANGED],
		       0, FALSE);
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

#ifdef MAEMO_CONIC_DUMMY
static gboolean on_dummy_connection_check (gpointer user_data)
{
	TnyMaemoConicDevice *self = TNY_MAEMO_CONIC_DEVICE (user_data);
	TnyMaemoConicDevicePriv *priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);
		
	/* Check whether the enviroment variable has changed, 
	 * so we can fake a connection change: */
	gchar *filename = g_build_filename (
		g_get_home_dir (), 
		MAEMO_CONIC_DUMMY_IAP_ID_FILENAME,
		NULL);
		
	gchar *contents = 0;
	GError* error = 0;
	gboolean test = g_file_get_contents (filename, &contents, NULL, &error);
	if(error) {
		/* printf("%s: error from g_file_get_contents(): %s\n", __FUNCTION__, error->message); */
		g_error_free (error);
		error = NULL;
	}
	
	if (!test || !contents) {
		/* Default to the first debug connection: */
		contents = g_strdup ("debug id0");
	}
	
	if (contents)
		g_strstrip(contents);

	if (!(priv->iap) || (strcmp (contents, priv->iap) != 0)) {
		priv->iap = g_strdup (contents);
		
		printf ("DEBUG: TnyMaemoConicDevice: %s:\n  Dummy connection changing to %s\n", __FUNCTION__, priv->iap);
		g_signal_emit (self, tny_device_signals [TNY_DEVICE_CONNECTION_CHANGED],
		       0, TRUE);
	}
	
	g_free (contents);
	g_free (filename);
	
	return TRUE;
}
#endif /* MAEMO_CONIC_DUMMY */



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

	/* This might be necessary to make the connection object 
	 * actually emit the signal, though the documentation says 
	 * that they should be sent even when this is not set, 
	 * when we explicitly try to connect. 
	 * The signal still does not seem to be emitted.
	 */
	g_object_set (priv->cnx, "automatic-connection-events", TRUE, NULL);

	g_signal_connect (priv->cnx, "connection-event",
			  G_CALLBACK(on_connection_event), self);

	/*
	 * this will get us in connected state only if there is already a connection.
	 * thus, this will setup our state correctly when we receive the signals
	 */
	if (!con_ic_connection_connect (priv->cnx, CON_IC_CONNECT_FLAG_AUTOMATICALLY_TRIGGERED))
		g_warning ("could not send connect dbus message");
	
	
	priv->is_online     = FALSE; 
	priv->forced = FALSE;
	
	#ifdef MAEMO_CONIC_DUMMY
	/* Allow debuggers to fake a connection change by setting an environment 
	 * variable, which we check ever 1 second.
	 * This should match one of the fake iap IDs that we created in 
	 * tny_maemo_conic_device_get_iap_list().
	 */
	priv->dummy_env_check_timeout = 
		g_timeout_add (1000, on_dummy_connection_check, self);
	#endif /* MAEMO_CONIC_DUMMY */
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
	printf ("DEBUG: %s\n", __FUNCTION__);

	TnyMaemoConicDevicePriv *priv;
	priv   = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (obj);
	if (priv->cnx && CON_IC_IS_CONNECTION(priv->cnx)) {
		if (!tny_maemo_conic_device_disconnect (TNY_MAEMO_CONIC_DEVICE(obj),priv->iap))
			g_warning ("failed to disconnect dbus message");
		g_object_unref (priv->cnx);
		priv->cnx = NULL;
	}

	#ifdef MAEMO_CONIC_DUMMY
	if (priv->dummy_env_check_timeout) {
		g_source_remove (priv->dummy_env_check_timeout);
		priv->dummy_env_check_timeout = 0;
	}
	#endif /* MAEMO_CONIC_DUMMY */
	
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

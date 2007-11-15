/* libtinymail-camel - The Tiny Mail base library for Maemo
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

#include <config.h>
#include <glib.h>
#include <glib-object.h>
#include <tny-maemo-conic-device.h>
#include <conicevent.h>
#include <coniciap.h>
#include <conicconnection.h>
#include <conicconnectionevent.h>
#include <string.h>
#include <tny-error.h>
#include <gdk/gdk.h> /* For GDK_THREAD_ENTER/LEAVE */

static void stop_loop (TnyMaemoConicDevice *self);

static gboolean tny_maemo_conic_device_is_online (TnyDevice *self);

static GObjectClass *parent_class = NULL;

typedef struct {
	TnyMaemoConicDevice *self;
	gchar* iap_id;
	gpointer user_data;
	TnyMaemoConicDeviceConnectCallback callback;
} ConnectInfo;

typedef struct {
	ConIcConnection *cnx;
	gboolean is_online;
	gchar *iap;
	gboolean forced; /* Whether the is_online value is forced rather than real. */
	ConnectInfo *connect_slot;
	/* When non-NULL, we are waiting for the success or failure signal. */
	GMainLoop *loop;
	gint signal1;
} TnyMaemoConicDevicePriv;


#define TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_MAEMO_CONIC_DEVICE, TnyMaemoConicDevicePriv))

typedef struct {
	GObject *self;
	gboolean status;
} EmitStatusInfo;

static gboolean
dnsmasq_has_resolv (void)
{
	/* This is because silly Conic does not have a blocking API that tells
	 * us immediately when the device is online. */

	if (!g_file_test ("/var/run/resolv.conf", G_FILE_TEST_EXISTS))
		if (!g_file_test ("/tmp/resolv.conf.wlan0", G_FILE_TEST_EXISTS))
			if (!g_file_test ("/tmp/resolv.conf.ppp0", G_FILE_TEST_EXISTS))
				return FALSE;

	return TRUE;
}

static gboolean
conic_emit_status_idle (gpointer user_data)
{
	EmitStatusInfo *info = (EmitStatusInfo *) user_data;

	/* We lock the gdk thread because tinymail wants implementations to do
	 * this before emitting signals from within a g_idle_add_full callback.
	 * See http://www.tinymail.org/trac/tinymail/wiki/HowTnyLockable */

	gdk_threads_enter ();
	g_signal_emit (info->self, tny_device_signals [TNY_DEVICE_CONNECTION_CHANGED],
		0, info->status);
	gdk_threads_leave ();

	return FALSE;
}

static void
conic_emit_status_destroy (gpointer user_data)
{
	EmitStatusInfo *info = (EmitStatusInfo *) user_data;
	g_object_unref (info->self);
	g_slice_free (EmitStatusInfo, info);
	return;
}

static void 
conic_emit_status (TnyDevice *self, gboolean status)
{
	/* Emit it in an idle handler: */
	EmitStatusInfo *info = g_slice_new (EmitStatusInfo);
	guint time = 1000;

	info->self = g_object_ref (self);
	info->status = status;

	if (!dnsmasq_has_resolv())
		time = 5000;

	g_timeout_add_full (G_PRIORITY_DEFAULT, time, conic_emit_status_idle,
		info, conic_emit_status_destroy);

	return;
}

static void 
tny_maemo_conic_device_reset (TnyDevice *device)
{
	TnyMaemoConicDevice *self;
	TnyMaemoConicDevicePriv *priv;
	gboolean status_before = FALSE;

	g_return_if_fail (TNY_IS_DEVICE(device));
	self = TNY_MAEMO_CONIC_DEVICE (device);
	priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);

	status_before = tny_maemo_conic_device_is_online (device);
	priv->forced = FALSE;

	if (status_before != tny_maemo_conic_device_is_online (device))
		conic_emit_status (device, !status_before);

	return;
}

static void
handle_connect (TnyMaemoConicDevice *self, int con_err, int con_state)
{
	TnyMaemoConicDevicePriv *priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);

	if (priv->connect_slot) 
	{
		GError *err = NULL;
		gboolean canceled = FALSE;
		ConnectInfo *info = priv->connect_slot;

		/* Mark it as handled (TODO, this needs a small lock) */
		priv->connect_slot = NULL;

		switch (con_err) {
			case CON_IC_CONNECTION_ERROR_NONE:
				break;
			case CON_IC_CONNECTION_ERROR_INVALID_IAP:
				g_set_error (&err, TNY_ACCOUNT_ERROR, TNY_ERROR_UNSPEC,
					"IAP is invalid");
				break;
			case CON_IC_CONNECTION_ERROR_CONNECTION_FAILED:
				g_set_error (&err, TNY_ACCOUNT_ERROR, TNY_ERROR_UNSPEC,
					"Connection failed");
				break;
			case CON_IC_CONNECTION_ERROR_USER_CANCELED:
			default:
				canceled = TRUE;
				break;
		}

		if (info->callback) {
			/* We lock the gdk thread because tinymail wants implementations to do
			 * this before invoking callbacks from within a g_idle_add_full callback.
			 * See http://www.tinymail.org/trac/tinymail/wiki/HowTnyLockable */

			gdk_threads_enter ();
			info->callback (info->self, info->iap_id, canceled, err, info->user_data);
			gdk_threads_leave ();
		}

		if (err)
			g_error_free (err);

		g_object_unref (info->self);
		g_free (info->self);
		g_slice_free (ConnectInfo, info);
	}

	return;
}

typedef struct {
	TnyMaemoConicDevice *self;
	int con_err;
	int con_state;
} HandleConnInfo;

static gboolean
handle_con_idle (gpointer data)
{
	HandleConnInfo *info = (HandleConnInfo *) data;
	handle_connect (info->self, info->con_err, info->con_state);
	return FALSE;
}

static void 
handle_con_idle_destroy (gpointer data) 
{ 
	HandleConnInfo *info = (HandleConnInfo *) data;
	g_object_unref (info->self);
	g_slice_free (HandleConnInfo, data); 
}


static void
on_connection_event (ConIcConnection *cnx, ConIcConnectionEvent *event, gpointer user_data)
{
	TnyMaemoConicDevice *device = TNY_MAEMO_CONIC_DEVICE (user_data);
	TnyMaemoConicDevicePriv *priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (device);
	gboolean is_online = FALSE;
	gboolean emit = FALSE;
	HandleConnInfo *iinfo;
	int con_err, con_state;

	/* Don't emit nor make any changes in case of forced state */

	if (priv->forced)
		return;

	con_err = con_ic_connection_event_get_error (event);
	con_state = con_ic_connection_event_get_status (event);

	switch (con_err) {
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

	switch (con_state) {
		case CON_IC_STATUS_CONNECTED:
			if (priv->iap)
				g_free (priv->iap);
			priv->iap = g_strdup (con_ic_event_get_iap_id ((ConIcEvent*)(event)));
			if (!priv->is_online)
				emit = TRUE;
			is_online = TRUE;
			/* Stop blocking tny_maemo_conic_device_connect(), if we are: */
			stop_loop (device);
			break;

		case CON_IC_STATUS_DISCONNECTED:
			priv->iap = NULL;
			if (priv->is_online)
				emit = TRUE;
			is_online = FALSE;
			/* Stop blocking tny_maemo_conic_device_connect(), if we are: */
			stop_loop (device);
			break;

		case CON_IC_STATUS_DISCONNECTING:
			break;
		default:
			g_return_if_reached (); 
	}

	priv->is_online = is_online;

	if (priv->connect_slot && 
		(con_err != CON_IC_CONNECTION_ERROR_NONE || 
		 con_state == CON_IC_STATUS_CONNECTED)) 
	{

		/* If there's an error or if we just connected, we call the
		 * callback for tny_maemo_conic_device_connect, if any */

		iinfo = g_slice_new (HandleConnInfo);
		iinfo->self = (TnyMaemoConicDevice *) g_object_ref (device);
		iinfo->con_err = con_err;
		iinfo->con_state = con_state;

		g_idle_add_full (G_PRIORITY_HIGH, handle_con_idle, iinfo, 
			handle_con_idle_destroy);
	}

	if (emit)
		conic_emit_status (TNY_DEVICE (device), is_online);

	return;
}


/**
 * tny_maemo_conic_device_connect:
 * @self: a #TnyDevice object
 * @iap_id: the id of the Internet Access Point (IAP), or NULL for 'any;
 * @user_requested: whether or not the connection was automatically requested or by an user action
 * @callback: a #TnyMaemoConicDeviceConnectCallback
 * @user_data: user data for @callback
 * 
 * Try to connect to a specific IAP, or to any if @iap_id == NULL
 * this calls con_ic_connection_connect(_by_id).
 * This may show a dialog to allow the user to select a connection, or 
 * may otherwise take a significant amount of time. 
 **/
void 
tny_maemo_conic_device_connect_async (TnyMaemoConicDevice *self, 
				      const gchar* iap_id, 
				      gboolean user_requested,
				      TnyMaemoConicDeviceConnectCallback callback, 
				      gpointer user_data)
{
	TnyMaemoConicDevicePriv *priv = NULL;
	gboolean request_failed = FALSE;
	ConnectInfo *info;
	GError *err = NULL;
	ConIcConnectFlags flags;

	priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);

	info = g_slice_new (ConnectInfo);
	info->self = (TnyMaemoConicDevice *) g_object_ref (info);
	info->callback = callback;
	info->user_data = user_data;
	info->iap_id = g_strdup (iap_id);

	priv->connect_slot = info;

	/* Set the flags */
	if (user_requested)
		flags = CON_IC_CONNECT_FLAG_NONE;
	else
		flags = CON_IC_CONNECT_FLAG_AUTOMATICALLY_TRIGGERED;

	if (iap_id) {
		if (!con_ic_connection_connect_by_id (priv->cnx, iap_id, flags)) {
			g_set_error (&err, TNY_ACCOUNT_ERROR, TNY_ERROR_UNSPEC,
				"Could not send connect_by_id dbus message");
			request_failed = TRUE;
		}
	} else {
		if (!con_ic_connection_connect (priv->cnx, flags)) {
			g_set_error (&err, TNY_ACCOUNT_ERROR, TNY_ERROR_UNSPEC,
				"Could not send connect dbus message");
			request_failed = TRUE;
		}
	}

	if (request_failed) {
		priv->connect_slot = NULL;
		if (info->callback)
			info->callback (info->self, iap_id, FALSE, err, info->user_data);
		g_free (info->iap_id);
		g_object_unref (info->self);
		g_slice_free (ConnectInfo, info);
	}
 
	return;
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
	TnyMaemoConicDevicePriv *priv = NULL;

	g_return_val_if_fail (TNY_IS_MAEMO_CONIC_DEVICE(self), FALSE);

	priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);
	g_return_val_if_fail (priv->cnx, FALSE);

	if (iap_id) {
		if (!con_ic_connection_disconnect_by_id (priv->cnx, iap_id)) {
			g_warning ("Could not send disconnect_by_id dbus message");
			return FALSE;
		}
	} else {
		/* don't try to disconnect if iap_id==NULL, or conic will crash... */
		g_warning ("Could not send disconnect dbus message");
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
	TnyMaemoConicDevicePriv *priv = NULL;

	g_return_val_if_fail (TNY_IS_MAEMO_CONIC_DEVICE(self), NULL);
	g_return_val_if_fail (tny_maemo_conic_device_is_online(TNY_DEVICE(self)), NULL);

	priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);

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
	TnyMaemoConicDevicePriv *priv = NULL;
	g_return_val_if_fail (TNY_IS_MAEMO_CONIC_DEVICE(self), NULL);
	g_return_val_if_fail (iap_id, NULL);
	priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);
	g_return_val_if_fail (priv->cnx, NULL);

	/* Note that it is very unusual to return a reference from a get_() 
	 * function, but we must do so because that mistake has already been 
	 * made in con_ic_connection_get_iap (). If we just unref immediately 
	 * then libconic might destroy the object. */

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
	TnyMaemoConicDevicePriv *priv = NULL;

	priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);
	g_return_val_if_fail (priv->cnx, NULL);

	return con_ic_connection_get_all_iaps (priv->cnx);
}


/**
 * tny_maemo_conic_device_free_iap_list:
 * @self: a #TnyDevice object
 * @cnx_list: a list of IAP objects
 *
 * free a  list of IAP objects retrieved from tny_maemo_conic_device_get_iap_list
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
	return;
}


static void 
tny_maemo_conic_device_force_online (TnyDevice *device)
{
	TnyMaemoConicDevice *self;
	TnyMaemoConicDevicePriv *priv;
	gboolean already_online = FALSE;
	g_return_if_fail (TNY_IS_DEVICE(device));
	self = TNY_MAEMO_CONIC_DEVICE (device);
	priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);

	already_online = tny_maemo_conic_device_is_online (device);

	priv->forced = TRUE;
	priv->is_online = TRUE;

	/* Signal if it changed: */
	if (!already_online)
		g_signal_emit (device, tny_device_signals [TNY_DEVICE_CONNECTION_CHANGED], 0, TRUE);
}


static void
tny_maemo_conic_device_force_offline (TnyDevice *device)
{
	TnyMaemoConicDevice *self;
	TnyMaemoConicDevicePriv *priv;
	gboolean already_offline = FALSE;

	g_return_if_fail (TNY_IS_DEVICE(device));
	self = TNY_MAEMO_CONIC_DEVICE (device);
	priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);

	already_offline = !tny_maemo_conic_device_is_online (device);
	priv->forced = TRUE;
	priv->is_online = FALSE;

	/* Signal if it changed: */
	if (!already_offline)
		conic_emit_status (device, FALSE);

	return;
}

static gboolean
tny_maemo_conic_device_is_online (TnyDevice *self)
{
	g_return_val_if_fail (TNY_IS_DEVICE(self), FALSE);

	return TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self)->is_online;
}


static void
tny_maemo_conic_device_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyMaemoConicDevice *self = (TnyMaemoConicDevice *)instance;
	TnyMaemoConicDevicePriv *priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);

	/* We should not have a real is_online, based on what libconic has told us: */

	priv->forced = FALSE;
	priv->iap = NULL;
	priv->is_online = dnsmasq_has_resolv ();

	priv->cnx = con_ic_connection_new ();

	if (!priv->cnx)
		g_warning ("con_ic_connection_new failed. The TnyMaemoConicDevice will be useless.");

	/* This might be necessary to make the connection object actually emit 
	 * the signal, though the documentation says that they should be sent 
	 * even when this is not set, when we explicitly try to connect. The 
	 * signal still does not seem to be emitted. */
	g_object_set (priv->cnx, "automatic-connection-events", TRUE, NULL);

	priv->signal1 = (gint) g_signal_connect (priv->cnx, "connection-event",
			  G_CALLBACK(on_connection_event), self);

	/* This will get us in connected state only if there is already a connection.
	 * thus, this will setup our state correctly when we receive the signals. */
	if (!con_ic_connection_connect (priv->cnx, CON_IC_CONNECT_FLAG_AUTOMATICALLY_TRIGGERED))
		g_warning ("could not send connect dbus message");

	return;
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
	TnyMaemoConicDevicePriv *priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (obj);

	g_signal_handler_disconnect (obj, priv->signal1);

	if (priv->cnx && CON_IC_IS_CONNECTION(priv->cnx)) {
		tny_maemo_conic_device_disconnect (TNY_MAEMO_CONIC_DEVICE(obj),priv->iap);
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

static void 
stop_loop (TnyMaemoConicDevice *self)
{
	TnyMaemoConicDevicePriv *priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);
	if (priv->loop)
		g_main_loop_quit (priv->loop);
	return;
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
tny_maemo_conic_device_connect (TnyMaemoConicDevice *self, 
				const gchar* iap_id,
				gboolean user_requested)
{
	TnyMaemoConicDevicePriv *priv = NULL;
	gboolean request_failed = FALSE;
	ConIcConnectFlags flags;

	g_return_val_if_fail (TNY_IS_DEVICE(self), FALSE);
	priv = TNY_MAEMO_CONIC_DEVICE_GET_PRIVATE (self);

	g_return_val_if_fail (priv->cnx, FALSE);
	priv->loop = g_main_loop_new(NULL, FALSE /* not running immediately. */);

	/* Set the flags */
	if (user_requested)
		flags = CON_IC_CONNECT_FLAG_NONE;
	else
		flags = CON_IC_CONNECT_FLAG_AUTOMATICALLY_TRIGGERED;

	if (iap_id) {
		if (!con_ic_connection_connect_by_id (priv->cnx, iap_id, flags)) {
			g_warning ("could not send connect_by_id dbus message");
			request_failed = TRUE;
		}
	} else {
		if (!con_ic_connection_connect (priv->cnx, flags)) {
			g_warning ("could not send connect dbus message");
			request_failed = TRUE;
		}
	}

	if (request_failed) {
		g_object_unref (priv->loop);
		priv->loop = NULL;
	}
	
	/* Wait for the CON_IC_STATUS_CONNECTED (succeeded) or 
	 * CON_IC_STATUS_DISCONNECTED event: */
	 
	/* This is based on code found in gtk_dialog_run(): */
	GDK_THREADS_LEAVE();
	/* Run until g_main_loop_quit() is called by our signal handler. */
	if (priv->loop)
		g_main_loop_run (priv->loop);
	GDK_THREADS_ENTER();

	g_main_loop_unref (priv->loop);
	priv->loop = NULL;

	return priv->is_online;
}

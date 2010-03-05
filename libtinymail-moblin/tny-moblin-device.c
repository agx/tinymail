/* libtinymail-moblin - The Tinymail base library for Moblin
 * Copyright (C) 2010 Sergio Villar Senin <svillar@igalia.com>
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
#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>
#include "tny-moblin-device.h"

static GObjectClass *parent_class = NULL;

static gboolean tny_moblin_device_is_online (TnyDevice *self);
static gboolean tny_moblin_device_is_forced (TnyDevice *self);

typedef struct {
	gboolean fset;
	gboolean forced;
	guint emit_status_id;
} TnyMoblinDevicePriv;

#define TNY_MOBLIN_DEVICE_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_MOBLIN_DEVICE, TnyMoblinDevicePriv))

typedef struct {
	TnyMoblinDevice *self;
	gboolean status;
} EmitStatusInfo;

#define CONNMAN_SERVICE           "org.moblin.connman"
#define CONNMAN_MANAGER_PATH      "/"
#define CONNMAN_MANAGER_INTERFACE   CONNMAN_SERVICE ".Manager"

static gboolean
emit_status_idle (gpointer user_data)
{
	EmitStatusInfo *info = (EmitStatusInfo *) user_data;

	gdk_threads_enter ();
	g_signal_emit (info->self, tny_device_signals [TNY_DEVICE_CONNECTION_CHANGED],
		       0, info->status);
	gdk_threads_leave ();

	g_debug ("Emitting status, connection is %s", (info->status) ? "online" : "offline");

	return FALSE;
}

static void
emit_status_destroy (gpointer user_data)
{
	EmitStatusInfo *info;

	info = (EmitStatusInfo *) user_data;

	if (G_IS_OBJECT(info->self))
		g_object_unref (info->self);

	g_slice_free (EmitStatusInfo, info);
}

static void
emit_status (TnyMoblinDevice *self, gboolean status)
{
	EmitStatusInfo *info;
	TnyMoblinDevicePriv *priv;

	priv = TNY_MOBLIN_DEVICE_GET_PRIVATE (self);

	/* Emit it in an idle handler: */
	info = g_slice_new (EmitStatusInfo);
	info->self = g_object_ref (self);
	info->status = status;

	if (priv->emit_status_id) {
		g_debug ("%s: removing the previous emission", __FUNCTION__);
		g_source_remove (priv->emit_status_id);
		priv->emit_status_id = 0;
	}

	g_debug ("%s: emitting status (%p, %s) in %d ms",
		 __FUNCTION__, info, status ? "true" : "false", time);

	priv->emit_status_id = g_timeout_add_full (G_PRIORITY_DEFAULT, 1000,
						   emit_status_idle,
						   info, emit_status_destroy);
}


static void
tny_moblin_device_reset (TnyDevice *self)
{
	TnyMoblinDevicePriv *priv = TNY_MOBLIN_DEVICE_GET_PRIVATE (self);
	const gboolean status_before = tny_moblin_device_is_online (self);
	gboolean is_online;

	priv->fset = FALSE;
	priv->forced = FALSE;

	/* Signal if it changed: */
	is_online = tny_moblin_device_is_online (self);
	if (status_before != is_online)
		emit_status ((TnyMoblinDevice *) self, is_online);
}

static void
tny_moblin_device_force_online (TnyDevice *self)
{

	TnyMoblinDevicePriv *priv = TNY_MOBLIN_DEVICE_GET_PRIVATE (self);

	const gboolean already_online = tny_moblin_device_is_online (self);

	priv->fset = TRUE;
	priv->forced = TRUE;

	/* Signal if it changed: */
	if (!already_online)
		emit_status ((TnyMoblinDevice *) self, priv->forced);
}


static void
tny_moblin_device_force_offline (TnyDevice *self)
{
	TnyMoblinDevicePriv *priv = TNY_MOBLIN_DEVICE_GET_PRIVATE (self);

	const gboolean already_offline = !tny_moblin_device_is_online (self);

	priv->fset = TRUE;
	priv->forced = FALSE;

	/* Signal if it changed: */
	if (!already_offline)
		emit_status ((TnyMoblinDevice *) self, priv->forced);
}

static DBusMessage *
get_properties (DBusConnection *connection)
{
	DBusMessage *message, *reply;
	DBusError error;

	message = dbus_message_new_method_call (CONNMAN_SERVICE,
						CONNMAN_MANAGER_PATH,
						CONNMAN_MANAGER_INTERFACE,
						"GetProperties");
	if (message == NULL)
		return NULL;

	dbus_error_init(&error);

	reply = dbus_connection_send_with_reply_and_block(connection,
							  message, -1, &error);
	if (reply == NULL) {
		if (dbus_error_is_set(&error) == TRUE) {
			g_warning (error.message);
			dbus_error_free(&error);
		} else {
			g_warning ("%s: Failed to get properties\n", __FUNCTION__);
		}
		dbus_message_unref(message);
		return NULL;
	}
	dbus_message_unref(message);

	return reply;
}

static const char *
extract_state(DBusMessage *message)
{
	DBusMessageIter array, dict;

	dbus_message_iter_init(message, &array);
	dbus_message_iter_recurse(&array, &dict);

	while (dbus_message_iter_get_arg_type(&dict) == DBUS_TYPE_DICT_ENTRY) {
		DBusMessageIter entry, value;
		const char *key;

		dbus_message_iter_recurse(&dict, &entry);
		dbus_message_iter_get_basic(&entry, &key);

		dbus_message_iter_next(&entry);

		dbus_message_iter_recurse(&entry, &value);

		if (strcmp(key, "State") == 0) {
			const char *val;
			dbus_message_iter_get_basic(&value, &val);
			return val;
		}

		dbus_message_iter_next(&dict);
	}

	return NULL;
}

static gboolean
tny_moblin_device_is_online (TnyDevice *self)
{
	TnyMoblinDevicePriv *priv = TNY_MOBLIN_DEVICE_GET_PRIVATE (self);
	DBusMessage *message;
	DBusConnection *connection;
	const char *state;

	if (priv->fset)
		return priv->forced;

	connection = dbus_bus_get(DBUS_BUS_SYSTEM, NULL);
	if (!connection)
		return FALSE;

	message = get_properties(connection);
	state = extract_state(message);

	dbus_message_unref(message);

	if (state == NULL)
		return FALSE;

	g_debug ("TnyMoblinDevice is %s\n", state);

	return (!strncmp (state, "online", 6)) ? TRUE : FALSE;
}

static gboolean
tny_moblin_device_is_forced (TnyDevice *self)
{
	TnyMoblinDevicePriv *priv = TNY_MOBLIN_DEVICE_GET_PRIVATE (self);

	return priv->fset;
}

static void
tny_moblin_device_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyMoblinDevicePriv *priv = TNY_MOBLIN_DEVICE_GET_PRIVATE (instance);

	priv->forced = FALSE;
	priv->fset = FALSE;
	priv->emit_status_id = 0;
}

static void
_on_moblin_device_connection_changed (DBusGProxy *proxy,
				      DBusGProxyCall *call,
				      gpointer user_data)
{
	GError *error = NULL;
	gchar *state = NULL;

	dbus_g_proxy_end_call (proxy,
			       call,
			       &error,
                               G_TYPE_STRING,
			       &state,
			       G_TYPE_INVALID);

	if (error != NULL) {
		g_warning ("Error in method call: %s\n", error->message);
		g_clear_error (&error);
	} else {
		g_debug ("Network state changed, it is now %s\n", state);
		emit_status ((TnyMoblinDevice *) user_data,
			     (!strncmp (state, "online", 6)) ? TRUE : FALSE);
	}
}

/**
 * tny_moblin_device_new:
 *
 * Return value: A new #TnyDevice implemented for Moblin
 **/
TnyDevice*
tny_moblin_device_new (void)
{
	TnyMoblinDevice *self = g_object_new (TNY_TYPE_MOBLIN_DEVICE, NULL);
	DBusGConnection *connection;
	DBusGProxy *proxy;

	/* Listen to connection changes */
	connection = dbus_g_bus_get (DBUS_BUS_SYSTEM,
				     NULL);
	proxy = dbus_g_proxy_new_for_name (connection,
					   CONNMAN_SERVICE,
					   CONNMAN_MANAGER_PATH,
					   CONNMAN_MANAGER_INTERFACE);

	dbus_g_proxy_begin_call (proxy,
				 "GetState",
				 _on_moblin_device_connection_changed,
				 g_object_ref (self),
				 g_object_unref,
				 G_TYPE_INVALID);

	return TNY_DEVICE (self);
}


static void
tny_moblin_device_finalize (GObject *object)
{
	TnyMoblinDevicePriv *priv;

	g_debug ("%s: shutting the device down...", __FUNCTION__);

	priv = TNY_MOBLIN_DEVICE_GET_PRIVATE (object);

	if (priv->emit_status_id) {
		g_source_remove (priv->emit_status_id);
		priv->emit_status_id = 0;
	}

	(*parent_class->finalize) (object);
}


static void
tny_device_init (gpointer g, gpointer iface_data)
{
	TnyDeviceIface *klass = (TnyDeviceIface *)g;

	klass->is_online = tny_moblin_device_is_online;
	klass->is_forced = tny_moblin_device_is_forced;
	klass->reset= tny_moblin_device_reset;
	klass->force_offline= tny_moblin_device_force_offline;
	klass->force_online= tny_moblin_device_force_online;

	return;
}



static void 
tny_moblin_device_class_init (TnyMoblinDeviceClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_moblin_device_finalize;

	g_type_class_add_private (object_class, sizeof (TnyMoblinDevicePriv));

	return;
}

static gpointer 
tny_moblin_device_register_type (gpointer notused)
{
	GType type = 0;

	static const GTypeInfo info = 
		{
			sizeof (TnyMoblinDeviceClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			(GClassInitFunc) tny_moblin_device_class_init,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			sizeof (TnyMoblinDevice),
			0,      /* n_preallocs */
			tny_moblin_device_instance_init    /* instance_init */
		};
	
	static const GInterfaceInfo tny_device_info = 
		{
			(GInterfaceInitFunc) tny_device_init, /* interface_init */
			NULL,         /* interface_finalize */
			NULL          /* interface_data */
		};
	
	type = g_type_register_static (G_TYPE_OBJECT,
				       "TnyMoblinDevice",
				       &info, 0);
	
	g_type_add_interface_static (type, TNY_TYPE_DEVICE, 
				     &tny_device_info);
	
	return GSIZE_TO_POINTER (type);
}

GType 
tny_moblin_device_get_type (void)
{
	static GOnce once = G_ONCE_INIT;
	g_once (&once, tny_moblin_device_register_type, NULL);
	return GPOINTER_TO_SIZE (once.retval);
}

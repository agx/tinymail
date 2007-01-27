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

#include <glib/gi18n-lib.h>
#include <glib.h>
#include <string.h>

#include <tny-camel-account.h>
#include <tny-session-camel.h>
#include <tny-account-store.h>
#include <tny-folder.h>
#include <tny-camel-folder.h>

#include <camel/camel.h>
#include <camel/camel-session.h>
#include <camel/camel-store.h>
#include <camel/camel-offline-folder.h>
#include <camel/camel-offline-store.h>
#include <camel/camel-disco-folder.h>
#include <camel/camel-disco-store.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <tny-camel-shared.h>

#include "tny-camel-account-priv.h"
#include "tny-session-camel-priv.h"

static GObjectClass *parent_class = NULL;

static TnyAccountType
tny_camel_account_get_account_type (TnyAccount *self)
{
	return TNY_CAMEL_ACCOUNT_GET_CLASS (self)->get_account_type_func (self);
}

static TnyAccountType
tny_camel_account_get_account_type_default (TnyAccount *self)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	return priv->account_type;
}

/**
 * tny_camel_account_add_option:
 * @self: a #TnyCamelAccount object
 * @option: a "key=value" Camel option
 *
 * Add a Camel option to this #TnyCamelAccount instance. For example
 * "use_ssl=always" is the typical option added. Other possibilities for
 * the "use_ssl" option are "never" and "when-possible".
 *
 * The "always" means that normal SSL is used whereas "when-possible" means 
 * that TLS is tried (the STARTTLS capability of IMAP servers).
 *
 **/
void 
tny_camel_account_add_option (TnyCamelAccount *self, const gchar *option)
{
	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->add_option_func (self, option);
}

void 
tny_camel_account_add_option_default (TnyCamelAccount *self, const gchar *option)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	priv->options = g_list_prepend (priv->options, g_strdup (option));

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->reconnect_func (TNY_CAMEL_ACCOUNT (self));

	return;
}


static void
tny_camel_account_set_url_string (TnyAccount *self, const gchar *url_string)
{
	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->set_url_string_func (self, url_string);
}

static void
tny_camel_account_set_url_string_default (TnyAccount *self, const gchar *url_string)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	if (priv->url_string)
		g_free (priv->url_string);

	priv->url_string = g_strdup (url_string);

	if (G_UNLIKELY (!TNY_CAMEL_ACCOUNT_GET_CLASS (self)->reconnect_func))
		g_error ("This TnyAccount instance isn't a fully implemented type\n");

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->reconnect_func ((TnyCamelAccount*)self);

	return;
}

static const gchar*
tny_camel_account_get_url_string (TnyAccount *self)
{
	return TNY_CAMEL_ACCOUNT_GET_CLASS (self)->get_url_string_func (self);
}

static const gchar*
tny_camel_account_get_url_string_default (TnyAccount *self)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	return (const gchar*)priv->url_string;
}


static void
tny_camel_account_set_name (TnyAccount *self, const gchar *name)
{
	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->set_name_func (self, name);
}

static void
tny_camel_account_set_name_default (TnyAccount *self, const gchar *name)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	if (priv->name)
		g_free (priv->name);

	priv->name = g_strdup (name);

	return;
}

static const gchar*
tny_camel_account_get_name (TnyAccount *self)
{
	return TNY_CAMEL_ACCOUNT_GET_CLASS (self)->get_name_func (self);
}

static const gchar*
tny_camel_account_get_name_default (TnyAccount *self)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	return (const gchar*)priv->name;
}

static void 
tny_camel_account_stop_camel_operation_priv (TnyCamelAccountPriv *priv)
{
	if (priv->cancel)
	{
		camel_operation_unregister (priv->cancel);
		camel_operation_end (priv->cancel);
		if (priv->cancel)
			camel_operation_unref (priv->cancel);
	}

	priv->cancel = NULL;
	priv->inuse_spin = FALSE;

	return;
}

static gpointer
camel_cancel_hack_thread (gpointer data)
{
	camel_operation_cancel (NULL);

	g_thread_exit (NULL);
	return NULL;
}

void 
_tny_camel_account_start_camel_operation (TnyCamelAccount *self, CamelOperationStatusFunc func, gpointer user_data, const gchar *what)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	GThread *thread;

	g_mutex_lock (priv->cancel_lock);

	/* I know this isn't polite. But it works ;-) */
	/* camel_operation_cancel (NULL); */
	thread = g_thread_create (camel_cancel_hack_thread, NULL, TRUE, NULL);
	g_thread_join (thread);

	if (priv->cancel)
	{
		while (!camel_operation_cancel_check (priv->cancel)) 
		{ 
			g_warning (_("Cancellation failed, retrying\n"));
			thread = g_thread_create (camel_cancel_hack_thread, NULL, TRUE, NULL);
			g_thread_join (thread);
		}
		tny_camel_account_stop_camel_operation_priv (priv);
	}

	camel_operation_uncancel (NULL);

	while (priv->inuse_spin); 

	priv->inuse_spin = TRUE;


	priv->cancel = camel_operation_new (func, user_data);
	
	camel_operation_ref (priv->cancel);
	camel_operation_register (priv->cancel);
	camel_operation_start (priv->cancel, (char*)what);

	g_mutex_unlock (priv->cancel_lock);

	return;
}

void 
_tny_camel_account_stop_camel_operation (TnyCamelAccount *self)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	if (priv->cancel)
	{
		g_mutex_lock (priv->cancel_lock);
		tny_camel_account_stop_camel_operation_priv (priv);
		g_mutex_unlock (priv->cancel_lock);
	}

	return;
}


static gboolean 
tny_camel_account_is_connected (TnyAccount *self)
{
	return TNY_CAMEL_ACCOUNT_GET_CLASS (self)->is_connected_func (self);
}

static gboolean 
tny_camel_account_is_connected_default (TnyAccount *self)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	return priv->connected;
}

/**
 * tny_camel_account_set_session:
 * @self: a #TnyCamelAccount object
 * @session: a #TnySessionCamel object
 *
 * Set the #TnySessionCamel session this account will use
 *
 **/
void 
tny_camel_account_set_session (TnyCamelAccount *self, TnySessionCamel *session)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	g_static_rec_mutex_lock (priv->service_lock);
	priv->session = session;
	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->reconnect_func (self);
	_tny_session_camel_add_account (session, self);
	g_static_rec_mutex_unlock (priv->service_lock);

	return;
}

static void
tny_camel_account_set_id (TnyAccount *self, const gchar *id)
{
	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->set_id_func (self, id);
}

static void
tny_camel_account_set_id_default (TnyAccount *self, const gchar *id)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	if (G_UNLIKELY (priv->id))
		g_free (priv->id);

	priv->id = g_strdup (id);

	return;
}

static void
tny_camel_account_set_mech (TnyAccount *self, const gchar *mech)
{
	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->set_mech_func (self, mech);
}

static void
tny_camel_account_set_mech_default (TnyAccount *self, const gchar *mech)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	g_static_rec_mutex_lock (priv->service_lock);

	if (G_UNLIKELY (priv->mech))
		g_free (priv->mech);

	priv->mech = g_strdup (mech);

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->reconnect_func (TNY_CAMEL_ACCOUNT (self));
	
	g_static_rec_mutex_unlock (priv->service_lock);

	return;
}

static void
tny_camel_account_set_proto (TnyAccount *self, const gchar *proto)
{
	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->set_proto_func (self, proto);
}

static void
tny_camel_account_set_proto_default (TnyAccount *self, const gchar *proto)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	g_static_rec_mutex_lock (priv->service_lock);

	if (G_UNLIKELY (priv->proto))
		g_free (priv->proto);

	priv->proto = g_strdup (proto);

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->reconnect_func (TNY_CAMEL_ACCOUNT (self));
	
	g_static_rec_mutex_unlock (priv->service_lock);

	return;
}

static void
tny_camel_account_set_user (TnyAccount *self, const gchar *user)
{
	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->set_user_func (self, user);
}

static void
tny_camel_account_set_user_default (TnyAccount *self, const gchar *user)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	g_static_rec_mutex_lock (priv->service_lock);

	if (G_UNLIKELY (priv->user))
		g_free (priv->user);

	priv->user = g_strdup (user);

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->reconnect_func (TNY_CAMEL_ACCOUNT (self));

	g_static_rec_mutex_unlock (priv->service_lock);

	return;
}

static void
tny_camel_account_set_hostname (TnyAccount *self, const gchar *host)
{
	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->set_hostname_func (self, host);
}

static void
tny_camel_account_set_hostname_default (TnyAccount *self, const gchar *host)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	
	g_static_rec_mutex_lock (priv->service_lock);

	if (G_UNLIKELY (priv->host))
		g_free (priv->host);

	priv->host = g_strdup (host);

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->reconnect_func (TNY_CAMEL_ACCOUNT (self));

	g_static_rec_mutex_unlock (priv->service_lock);

	return;
}

static void
tny_camel_account_set_pass_func (TnyAccount *self, TnyGetPassFunc get_pass_func)
{
	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->set_pass_func_func (self, get_pass_func);
}

static void
tny_camel_account_set_pass_func_default (TnyAccount *self, TnyGetPassFunc get_pass_func)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	g_static_rec_mutex_lock (priv->service_lock);

	tny_session_camel_set_pass_func (priv->session, self, get_pass_func);
	priv->get_pass_func = get_pass_func;
	priv->pass_func_set = TRUE;

	if (G_UNLIKELY (!TNY_CAMEL_ACCOUNT_GET_CLASS (self)->reconnect_func))
		g_error ("This TnyAccount instance isn't a fully implemented type\n");

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->reconnect_func ((TnyCamelAccount*)self);

	g_static_rec_mutex_unlock (priv->service_lock);

	return;
}

static void
tny_camel_account_set_forget_pass_func (TnyAccount *self, TnyForgetPassFunc get_forget_pass_func)
{
	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->set_forget_pass_func_func (self, get_forget_pass_func);
}

static void
tny_camel_account_set_forget_pass_func_default (TnyAccount *self, TnyForgetPassFunc get_forget_pass_func)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	g_static_rec_mutex_lock (priv->service_lock);

	tny_session_camel_set_forget_pass_func (priv->session, self, get_forget_pass_func);
	priv->forget_pass_func = get_forget_pass_func;
	priv->forget_pass_func_set = TRUE;

	g_static_rec_mutex_unlock (priv->service_lock);

	return;
}

static const gchar*
tny_camel_account_get_id (TnyAccount *self)
{
	return TNY_CAMEL_ACCOUNT_GET_CLASS (self)->get_id_func (self);
}

static const gchar*
tny_camel_account_get_id_default (TnyAccount *self)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);	
	const gchar *retval;

	retval = (const gchar*)priv->id;

	return retval;
}

static const gchar*
tny_camel_account_get_mech (TnyAccount *self)
{
	return TNY_CAMEL_ACCOUNT_GET_CLASS (self)->get_mech_func (self);
}

static const gchar*
tny_camel_account_get_mech_default (TnyAccount *self)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	const gchar *retval;

	retval = (const gchar*)priv->mech;

	return retval;
}

static const gchar*
tny_camel_account_get_proto (TnyAccount *self)
{
	return TNY_CAMEL_ACCOUNT_GET_CLASS (self)->get_proto_func (self);
}

static const gchar*
tny_camel_account_get_proto_default (TnyAccount *self)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	const gchar *retval;

	retval = (const gchar*)priv->proto;

	return retval;
}

static const gchar*
tny_camel_account_get_user (TnyAccount *self)
{
	return TNY_CAMEL_ACCOUNT_GET_CLASS (self)->get_user_func (self);
}

static const gchar*
tny_camel_account_get_user_default (TnyAccount *self)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	const gchar *retval;

	retval = (const gchar*)priv->user;

	return retval;
}

static const gchar*
tny_camel_account_get_hostname (TnyAccount *self)
{
	return TNY_CAMEL_ACCOUNT_GET_CLASS (self)->get_hostname_func (self);
}

static const gchar*
tny_camel_account_get_hostname_default (TnyAccount *self)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);	
	const gchar *retval;

	retval = (const gchar*)priv->host;

	return retval;
}

static TnyGetPassFunc
tny_camel_account_get_pass_func (TnyAccount *self)
{
	return TNY_CAMEL_ACCOUNT_GET_CLASS (self)->get_pass_func_func (self);
}

static TnyGetPassFunc
tny_camel_account_get_pass_func_default (TnyAccount *self)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	TnyGetPassFunc retval;

	retval = priv->get_pass_func;

	return retval;
}

static TnyForgetPassFunc
tny_camel_account_get_forget_pass_func (TnyAccount *self)
{
	return TNY_CAMEL_ACCOUNT_GET_CLASS (self)->get_forget_pass_func_func (self);
}

static TnyForgetPassFunc
tny_camel_account_get_forget_pass_func_default (TnyAccount *self)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	TnyForgetPassFunc retval;

	retval = priv->forget_pass_func;

	return retval;
}

const CamelService*
_tny_camel_account_get_service (TnyCamelAccount *self)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	const CamelService *retval;

	retval = (const CamelService *)priv->service;

	return retval;
}

const gchar* 
_tny_camel_account_get_url_string (TnyCamelAccount *self)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	const gchar *retval;

	retval = (const gchar*)priv->url_string;

	return retval;
}


static void
tny_camel_account_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyCamelAccount *self = (TnyCamelAccount *)instance;
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	priv->cache_location = NULL;
	priv->service = NULL;
	priv->session = NULL;
	priv->url_string = NULL;

	priv->ex = camel_exception_new ();
	camel_exception_init (priv->ex);

	priv->cancel_lock = g_mutex_new ();
	priv->inuse_spin = FALSE;

	priv->options = NULL;
	priv->id = NULL;
	priv->user = NULL;
	priv->host = NULL;
	priv->proto = NULL;
	priv->mech = NULL;
	priv->forget_pass_func_set = FALSE;
	priv->pass_func_set = FALSE;
	priv->cancel = NULL;

	priv->service_lock = g_new (GStaticRecMutex, 1);
	g_static_rec_mutex_init (priv->service_lock);

	return;
}

/**
 * tny_camel_account_set_online_status:
 * @self: a #TnyCamelAccount object
 * @offline: whether or not the account is offline
 *
 * Set the connectivity status of an account
 *
 **/
void 
tny_camel_account_set_online_status (TnyCamelAccount *self, gboolean offline)
{
	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->set_online_status_func (self, offline);
}

void 
tny_camel_account_set_online_status_default (TnyCamelAccount *self, gboolean offline)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	/* TODO locking & error handling */

	if (!priv->service)
		return;

	/* _tny_camel_account_start_camel_operation (self, NULL, NULL, NULL); */

	/* g_static_rec_mutex_lock (priv->service_lock); */

	if (offline)
		camel_service_cancel_connect (priv->service);
	
	if (CAMEL_IS_DISCO_STORE (priv->service)) {
		if (!offline) {
			camel_disco_store_set_status (CAMEL_DISCO_STORE (priv->service),
										  CAMEL_DISCO_STORE_ONLINE, priv->ex);
			goto done;
		} else if (camel_disco_store_can_work_offline (CAMEL_DISCO_STORE (priv->service))) {
			
			camel_disco_store_set_status (CAMEL_DISCO_STORE (priv->service),
										  CAMEL_DISCO_STORE_OFFLINE,
										  priv->ex);
			goto done;
		}
	} else if (CAMEL_IS_OFFLINE_STORE (priv->service)) {
		
		if (!offline) {
			
			camel_offline_store_set_network_state (CAMEL_OFFLINE_STORE (priv->service),
												   CAMEL_OFFLINE_STORE_NETWORK_AVAIL,
												   priv->ex);
			goto done;
		} else {
			camel_offline_store_set_network_state (CAMEL_OFFLINE_STORE (priv->service),
												   CAMEL_OFFLINE_STORE_NETWORK_UNAVAIL,
												   priv->ex);
			goto done;
		}
	}
	
	if (offline)
		camel_service_disconnect (CAMEL_SERVICE (priv->service),
								  TRUE, priv->ex);

done:
		/* g_static_rec_mutex_unlock (priv->service_lock); */
		/* _tny_camel_account_stop_camel_operation (self); */

	return;
}


static void
tny_camel_account_finalize (GObject *object)
{
	TnyCamelAccount *self = (TnyCamelAccount *)object;
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	_tny_session_camel_forget_account (priv->session, (TnyCamelAccount*) object);    
	_tny_camel_account_start_camel_operation (self, NULL, NULL, NULL);
	_tny_camel_account_stop_camel_operation (self);

	g_mutex_lock (priv->cancel_lock);
	if (G_UNLIKELY (priv->cancel))
		camel_operation_unref (priv->cancel);
	g_mutex_unlock (priv->cancel_lock);

	g_mutex_free (priv->cancel_lock);
	priv->inuse_spin = FALSE;

	if (priv->options)
	{
		g_list_foreach (priv->options, (GFunc)g_free, NULL);
		g_list_free (priv->options);
	}

	g_static_rec_mutex_lock (priv->service_lock);

	if (G_LIKELY (priv->cache_location))
		g_free (priv->cache_location);

	if (G_LIKELY (priv->id))
		g_free (priv->id);

	if (G_LIKELY (priv->name))
		g_free (priv->name);

	if (G_LIKELY (priv->user))
		g_free (priv->user);

	if (G_LIKELY (priv->host))
		g_free (priv->host);

	if (G_LIKELY (priv->mech))
		g_free (priv->mech);

	if (G_LIKELY (priv->proto))
		g_free (priv->proto);

	if (G_LIKELY (priv->url_string))
		g_free (priv->url_string);

	g_static_rec_mutex_unlock (priv->service_lock);

	camel_exception_free (priv->ex);

	g_static_rec_mutex_free (priv->service_lock);

	(*parent_class->finalize) (object);

	return;
}


static void
tny_account_init (gpointer g, gpointer iface_data)
{
	TnyAccountIface *klass = (TnyAccountIface *)g;

	klass->get_hostname_func = tny_camel_account_get_hostname;
	klass->set_hostname_func = tny_camel_account_set_hostname;
	klass->get_proto_func = tny_camel_account_get_proto;
	klass->set_proto_func = tny_camel_account_set_proto;
	klass->get_mech_func = tny_camel_account_get_mech;
	klass->set_mech_func = tny_camel_account_set_mech;
	klass->get_user_func = tny_camel_account_get_user;
	klass->set_user_func = tny_camel_account_set_user;
	klass->get_pass_func_func = tny_camel_account_get_pass_func;
	klass->set_pass_func_func = tny_camel_account_set_pass_func;
	klass->get_forget_pass_func_func = tny_camel_account_get_forget_pass_func;
	klass->set_forget_pass_func_func = tny_camel_account_set_forget_pass_func;
	klass->set_id_func = tny_camel_account_set_id;
	klass->get_id_func = tny_camel_account_get_id;
	klass->is_connected_func = tny_camel_account_is_connected;
	klass->set_url_string_func = tny_camel_account_set_url_string;
	klass->get_url_string_func = tny_camel_account_get_url_string;
	klass->get_name_func = tny_camel_account_get_name;
	klass->set_name_func = tny_camel_account_set_name;
	klass->get_account_type_func = tny_camel_account_get_account_type;

	return;
}

static void 
tny_camel_account_class_init (TnyCamelAccountClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;


	class->get_hostname_func = tny_camel_account_get_hostname_default;
	class->set_hostname_func = tny_camel_account_set_hostname_default;
	class->get_proto_func = tny_camel_account_get_proto_default;
	class->set_proto_func = tny_camel_account_set_proto_default;
	class->get_mech_func = tny_camel_account_get_mech_default;
	class->set_mech_func = tny_camel_account_set_mech_default;
	class->get_user_func = tny_camel_account_get_user_default;
	class->set_user_func = tny_camel_account_set_user_default;
	class->get_pass_func_func = tny_camel_account_get_pass_func_default;
	class->set_pass_func_func = tny_camel_account_set_pass_func_default;
	class->get_forget_pass_func_func = tny_camel_account_get_forget_pass_func_default;
	class->set_forget_pass_func_func = tny_camel_account_set_forget_pass_func_default;
	class->set_id_func = tny_camel_account_set_id_default;
	class->get_id_func = tny_camel_account_get_id_default;
	class->is_connected_func = tny_camel_account_is_connected_default;
	class->set_url_string_func = tny_camel_account_set_url_string_default;
	class->get_url_string_func = tny_camel_account_get_url_string_default;
	class->get_name_func = tny_camel_account_get_name_default;
	class->set_name_func = tny_camel_account_set_name_default;
	class->get_account_type_func = tny_camel_account_get_account_type_default;

	class->add_option_func = tny_camel_account_add_option_default;
	class->set_online_status_func = tny_camel_account_set_online_status_default;

	object_class->finalize = tny_camel_account_finalize;

	g_type_class_add_private (object_class, sizeof (TnyCamelAccountPriv));

	return;
}

GType 
tny_camel_account_get_type (void)
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
		  sizeof (TnyCamelAccountClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_camel_account_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyCamelAccount),
		  0,      /* n_preallocs */
		  tny_camel_account_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_account_info = 
		{
		  (GInterfaceInitFunc) tny_account_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyCamelAccount",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_ACCOUNT, 
			&tny_account_info);

	}

	return type;
}

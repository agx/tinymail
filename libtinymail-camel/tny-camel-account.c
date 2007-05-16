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
#include <tny-error.h>
#include <tny-simple-list.h>
#include <tny-pair.h>

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

#define TINYMAIL_ENABLE_PRIVATE_API
#include "tny-common-priv.h"
#undef TINYMAIL_ENABLE_PRIVATE_API

static GObjectClass *parent_class = NULL;

static gboolean 
tny_camel_account_matches_url_string (TnyAccount *self, const gchar *url_string)
{
	return TNY_CAMEL_ACCOUNT_GET_CLASS (self)->matches_url_string_func (self, url_string);
}

static gboolean 
tny_camel_account_matches_url_string_default (TnyAccount *self, const gchar *url_string)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	CamelURL *in = NULL;
	CamelURL *org = NULL;
	gboolean retval = TRUE;

	if (url_string)
		camel_url_new (url_string, &ex);
	else
		return FALSE;

	if (camel_exception_is_set (&ex) || !in)
		return FALSE;

	if (priv->url_string)
		org = camel_url_new (priv->url_string, &ex);
	else {
		gchar *proto;
		if (priv->proto == NULL)
			return FALSE;
		proto = g_strdup_printf ("%s://", priv->proto); 
		org = camel_url_new (proto, &ex);
		g_free (proto);
		if (camel_exception_is_set (&ex) || !org)
			return FALSE;
		camel_url_set_protocol (org, priv->proto); 
		if (priv->user)
			camel_url_set_user (org, priv->user);
		camel_url_set_host (org, priv->host);
		if (priv->port != -1)
			camel_url_set_port (org, (int)priv->port);
	}

	if (camel_exception_is_set (&ex) || !org)
	{
		if (in)
			camel_url_free (in);
		if (org)
			camel_url_free (org);
		return FALSE;
	}

	if (in && org && in->protocol && (org->protocol && strcmp (org->protocol, in->protocol) != 0))
		retval = FALSE;

	if (in && org && in->user && (org->user && strcmp (org->user, in->user) != 0))
		retval = FALSE;

	if (in && org && in->host && (org->host && strcmp (org->host, in->host) != 0))
		retval = FALSE;

	if (in && org && in->port != 0 && (org->port != in->port))
		retval = FALSE;

	if (org)
		camel_url_free (org);

	if (in)
		camel_url_free (in);

	return retval;
}

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

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (self);

	return;
}

void 
_tny_camel_account_try_connect (TnyCamelAccount *self, GError **err)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (TNY_CAMEL_ACCOUNT (self));

	if (camel_exception_is_set (priv->ex))
	{
		g_set_error (err, TNY_ACCOUNT_ERROR, 
			TNY_ACCOUNT_ERROR_TRY_CONNECT,
			camel_exception_get_description (priv->ex));
	}

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

	priv->custom_url_string = TRUE;
	priv->url_string = g_strdup (url_string);

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (TNY_CAMEL_ACCOUNT (self));

	return;
}

static gchar*
tny_camel_account_get_url_string (TnyAccount *self)
{
	return TNY_CAMEL_ACCOUNT_GET_CLASS (self)->get_url_string_func (self);
}

static gchar*
tny_camel_account_get_url_string_default (TnyAccount *self)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	return priv->url_string?g_strdup (priv->url_string):NULL;
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

/* TODO: Documentation.
 */
void 
_tny_camel_account_start_camel_operation_n (TnyCamelAccount *self, CamelOperationStatusFunc func, gpointer user_data, const gchar *what, gboolean cancel)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	GThread *thread;
	GError *err = NULL;

	g_mutex_lock (priv->cancel_lock);

	if (cancel)
	{
		/* I know this isn't polite. But it works ;-) */
		/* camel_operation_cancel (NULL); */
		thread = g_thread_create (camel_cancel_hack_thread, NULL, TRUE, &err);
		if (err == NULL)
			g_thread_join (thread);
		else
			g_error_free (err);
	

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
	}

	while (priv->inuse_spin); 

	priv->inuse_spin = TRUE;

	priv->cancel = camel_operation_new (func, user_data);

	camel_operation_ref (priv->cancel);
	camel_operation_register (priv->cancel);
	camel_operation_start (priv->cancel, (char*)what);

	g_mutex_unlock (priv->cancel_lock);

	return;
}

/* TODO: Documentation. 
 * Why would we use this instead of just calling the callback directly in an 
 * idle handler? 
 * How does this allow the operation to be cancelled? Can the user cancel it, 
 * or does cancel happen only if there is an internal error?
 * murrayc.
 */
void 
_tny_camel_account_start_camel_operation (TnyCamelAccount *self, CamelOperationStatusFunc func, gpointer user_data, const gchar *what)
{
	_tny_camel_account_start_camel_operation_n (self, func, user_data, what, TRUE);
}

/* TODO: Documentation.
 */
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
	_tny_session_camel_add_account (session, self);

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (self);

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
tny_camel_account_set_secure_auth_mech (TnyAccount *self, const gchar *mech)
{
	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->set_secure_auth_mech_func (self, mech);
}

static void
tny_camel_account_set_secure_auth_mech_default (TnyAccount *self, const gchar *mech)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	g_static_rec_mutex_lock (priv->service_lock);

	priv->custom_url_string = FALSE;

	if (G_UNLIKELY (priv->mech))
		g_free (priv->mech);

	priv->mech = g_strdup (mech);

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (TNY_CAMEL_ACCOUNT (self));

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

	priv->custom_url_string = FALSE;

	if (G_UNLIKELY (priv->proto))
		g_free (priv->proto);

	priv->proto = g_strdup (proto);

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (TNY_CAMEL_ACCOUNT (self));

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

	priv->custom_url_string = FALSE;

	if (G_UNLIKELY (priv->user))
		g_free (priv->user);

	priv->user = g_strdup (user);

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (TNY_CAMEL_ACCOUNT (self));

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

	priv->custom_url_string = FALSE;

	if (G_UNLIKELY (priv->host))
		g_free (priv->host);

	priv->host = g_strdup (host);

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (TNY_CAMEL_ACCOUNT (self));

	g_static_rec_mutex_unlock (priv->service_lock);

	return;
}



static void
tny_camel_account_set_port (TnyAccount *self, guint port)
{
	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->set_port_func (self, port);
}

static void
tny_camel_account_set_port_default (TnyAccount *self, guint port)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	
	g_static_rec_mutex_lock (priv->service_lock);

	priv->custom_url_string = FALSE;

	priv->port = (gint) port;

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (TNY_CAMEL_ACCOUNT (self));

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

	priv->get_pass_func = get_pass_func;
	priv->pass_func_set = TRUE;

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (TNY_CAMEL_ACCOUNT (self));

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

	priv->forget_pass_func = get_forget_pass_func;
	priv->forget_pass_func_set = TRUE;

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (TNY_CAMEL_ACCOUNT (self));

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
tny_camel_account_get_secure_auth_mech (TnyAccount *self)
{
	return TNY_CAMEL_ACCOUNT_GET_CLASS (self)->get_secure_auth_mech_func (self);
}

static const gchar*
tny_camel_account_get_secure_auth_mech_default (TnyAccount *self)
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


static void 
tny_camel_account_cancel (TnyAccount *self)
{
	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->cancel_func (self);
}

static void 
tny_camel_account_cancel_default (TnyAccount *self)
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

	g_mutex_unlock (priv->cancel_lock);

	return;
}


static guint 
tny_camel_account_get_port (TnyAccount *self)
{
	return TNY_CAMEL_ACCOUNT_GET_CLASS (self)->get_port_func (self);
}

static guint 
tny_camel_account_get_port_default (TnyAccount *self)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	return (guint)priv->port;
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

	priv->get_pass_func = NULL;
	priv->forget_pass_func = NULL;
	priv->port = -1;
	priv->cache_location = NULL;
	priv->service = NULL;
	priv->session = NULL;
	priv->url_string = NULL;

	priv->ex = camel_exception_new ();
	camel_exception_init (priv->ex);

	priv->custom_url_string = FALSE;
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
 * tny_camel_account_set_online:
 * @self: a #TnyCamelAccount object
 * @online: whether or not the account is online
 * @err: a #GError instance or NULL
 *
 * Set the connectivity status of an account
 *
 **/
void 
tny_camel_account_set_online (TnyCamelAccount *self, gboolean online, GError **err)
{
	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->set_online_func (self, online, err);
}

void 
tny_camel_account_set_online_default (TnyCamelAccount *self, gboolean online, GError **err)
{
	/* Note that the human-readable GError:message strings here are only for debugging.
	 * They should never be shown to the user. The application should make its own 
	 * decisions about how to show these errors in the UI, and should make sure that 
	 * they are translated in the user's locale.
	 */

	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;

	if (!priv->service || !CAMEL_IS_SERVICE (priv->service))
	{
		if (camel_exception_is_set (priv->ex))
		{
			g_set_error (err, TNY_ACCOUNT_ERROR, 
				TNY_ACCOUNT_ERROR_TRY_CONNECT,
				camel_exception_get_description (priv->ex));
		} else {
			g_set_error (err, TNY_ACCOUNT_ERROR, 
				TNY_ACCOUNT_ERROR_TRY_CONNECT,
				_("Account not yet fully configured"));
		}

		return;
	}

	/* if (offline)
		camel_service_cancel_connect (priv->service); */

	if (CAMEL_IS_DISCO_STORE (priv->service)) {
		if (online) {
			camel_disco_store_set_status (CAMEL_DISCO_STORE (priv->service),
										  CAMEL_DISCO_STORE_ONLINE, &ex);
			if (!camel_exception_is_set (&ex))
				camel_service_connect (CAMEL_SERVICE (priv->service), &ex);

			goto done;
		} else if (camel_disco_store_can_work_offline (CAMEL_DISCO_STORE (priv->service))) {
			
			camel_disco_store_set_status (CAMEL_DISCO_STORE (priv->service),
										  CAMEL_DISCO_STORE_OFFLINE,
										  &ex);
			goto done;
		}
	} else if (CAMEL_IS_OFFLINE_STORE (priv->service)) {
		
		if (online) {
			
			camel_offline_store_set_network_state (CAMEL_OFFLINE_STORE (priv->service),
												   CAMEL_OFFLINE_STORE_NETWORK_AVAIL,
												   &ex);
			goto done;
		} else {
			camel_offline_store_set_network_state (CAMEL_OFFLINE_STORE (priv->service),
												   CAMEL_OFFLINE_STORE_NETWORK_UNAVAIL,
												   &ex);
			goto done;
		}
	}

	if (!online)
		camel_service_disconnect (CAMEL_SERVICE (priv->service),
								  TRUE, &ex);

done:

	if (camel_exception_is_set (&ex))
	{
		g_set_error (err, TNY_ACCOUNT_ERROR, 
			TNY_ACCOUNT_ERROR_TRY_CONNECT,
			camel_exception_get_description (&ex));
	}

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

	if (priv->service && CAMEL_IS_OBJECT (priv->service))
		camel_object_unref (CAMEL_OBJECT (priv->service));

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

	klass->get_port_func = tny_camel_account_get_port;
	klass->set_port_func = tny_camel_account_set_port;
	klass->get_hostname_func = tny_camel_account_get_hostname;
	klass->set_hostname_func = tny_camel_account_set_hostname;
	klass->get_proto_func = tny_camel_account_get_proto;
	klass->set_proto_func = tny_camel_account_set_proto;
	klass->get_secure_auth_mech_func = tny_camel_account_get_secure_auth_mech;
	klass->set_secure_auth_mech_func = tny_camel_account_set_secure_auth_mech;
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
	klass->cancel_func = tny_camel_account_cancel;
	klass->matches_url_string_func = tny_camel_account_matches_url_string;

	return;
}

static void 
tny_camel_account_class_init (TnyCamelAccountClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	class->get_port_func = tny_camel_account_get_port_default;
	class->set_port_func = tny_camel_account_set_port_default;
	class->get_hostname_func = tny_camel_account_get_hostname_default;
	class->set_hostname_func = tny_camel_account_set_hostname_default;
	class->get_proto_func = tny_camel_account_get_proto_default;
	class->set_proto_func = tny_camel_account_set_proto_default;
	class->get_secure_auth_mech_func = tny_camel_account_get_secure_auth_mech_default;
	class->set_secure_auth_mech_func = tny_camel_account_set_secure_auth_mech_default;
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
	class->cancel_func = tny_camel_account_cancel_default;
	class->matches_url_string_func = tny_camel_account_matches_url_string_default;

	class->add_option_func = tny_camel_account_add_option_default;
	class->set_online_func = tny_camel_account_set_online_default;

	object_class->finalize = tny_camel_account_finalize;

	g_type_class_add_private (object_class, sizeof (TnyCamelAccountPriv));

	return;
}

/**
 * tny_camel_account_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
GType 
tny_camel_account_get_type (void)
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

typedef struct 
{
	TnyCamelAccount *self;
	TnyCamelGetSupportedSecureAuthCallback callback;
	TnyStatusCallback status_callback;
	gpointer user_data;
	gboolean cancelled;
	TnyList *result;
	/* This stops us from calling a status callback after the operation has 
	 * finished. */
	TnyIdleStopper* stopper;
	GError *err;
	TnySessionCamel *session;
} GetSupportedAuthInfo;


static void
tny_camel_account_get_supported_secure_authentication_async_status (struct _CamelOperation *op, const char *what, int sofar, int oftotal, void *thr_user_data)
{
	printf ("DEBUG: %s\n", __FUNCTION__);
	
	/* Use the generic tny_progress* idle callback system 
	 * to call our status callback,
	 * using the IdleStopper to stop the status callback 
	 * from being called after the main callback: */
	GetSupportedAuthInfo *info = thr_user_data;
	TnyProgressInfo *progress_info = tny_progress_info_new (
		G_OBJECT (info->self), 
		info->status_callback, 
		TNY_GET_SUPPORTED_SECURE_AUTH_STATUS, 
		TNY_GET_SUPPORTED_SECURE_AUTH_STATUS_GET_SECURE_AUTH, 
		what, sofar, 
		oftotal, info->stopper, info->user_data);

	g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
		tny_progress_info_idle_func, progress_info, 
		tny_progress_info_destroy);
}
		
/** This is the GDestroyNotify callback provided to g_idle_add_full()
 * for tny_camel_account_get_supported_secure_authentication_async_callback().
 */
static void
tny_camel_account_get_supported_secure_authentication_async_destroyer (gpointer thr_user_data)
{
	printf ("DEBUG: %s)\n", __FUNCTION__);
		
	GetSupportedAuthInfo *info = thr_user_data;
	TnyCamelAccount *self = info->self;
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	/* thread reference */
	g_object_unref (G_OBJECT (self));
	if (info->err) {
		g_error_free (info->err);
		info->err = NULL;	
	}

	_tny_session_stop_operation (info->session);

	tny_idle_stopper_destroy (info->stopper);
	info->stopper = NULL;
	
	g_slice_free (GetSupportedAuthInfo, thr_user_data);
}

static gboolean
tny_camel_account_get_supported_secure_authentication_async_callback (gpointer thr_user_data)
{
	printf ("DEBUG: %s\n", __FUNCTION__);
		
	GetSupportedAuthInfo *info = thr_user_data;
	TnyCamelAccount *self = info->self;
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	if (info->callback) {
		printf ("DEBUG: %s: Calling callback.\n", __FUNCTION__);
		info->callback (info->self, info->cancelled, info->result, &info->err, info->user_data);
	}

	/* Prevent status callbacks from being called after this
	 * (can happen because the 2 idle callbacks have different priorities)
	 * by causing tny_idle_stopper_is_stopped() to return TRUE. */
	tny_idle_stopper_stop (info->stopper);

	return FALSE;
}


/* Starts the operation in the thread: */
static gpointer 
tny_camel_account_get_supported_secure_authentication_async_thread (
	gpointer thr_user_data)
{
	printf ("DEBUG: %s\n", __FUNCTION__);
		
	GetSupportedAuthInfo *info = thr_user_data;
	TnyCamelAccount *self = info->self;
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	GError *err = NULL;

	g_static_rec_mutex_lock (priv->service_lock);

	info->cancelled = FALSE;

	/* Make sure that the status callback is called (in an idle handler) 
	 * while this thread is running: */
	_tny_camel_account_start_camel_operation (TNY_CAMEL_ACCOUNT (self), 
		tny_camel_account_get_supported_secure_authentication_async_status, info, 
		"Querying supported secure authentication methods from server.");

	/* Do the actual work:
	 * This is happening in a thread, 
	 * and the status callback is being called regularly while this is 
	 * happening. */	
	GList *authtypes = camel_service_query_auth_types (priv->service, &ex);
	
	/* The result will be a TnyList of TnyPairs: */
	TnyList *result = tny_simple_list_new ();
	GList *iter = authtypes;
	while (iter) {
		CamelServiceAuthType *item = (CamelServiceAuthType *)iter->data;
		if (item) {
			/* Get the name of the auth method:
			 * Note that, at least for IMAP, authproto=NULL when 
			 * name=Password. */
			printf ("DEBUG: %s: authproto =%s, name=%s\n", __FUNCTION__, item->authproto, item->name);
			
			/* We don't use the value part of the TnyPair. */
			TnyPair *pair = tny_pair_new (item->authproto, NULL);
			tny_list_append (result, G_OBJECT (pair));
			g_object_unref (pair);
		}
		
		iter = g_list_next (iter);	
	}
	g_list_free (authtypes);
	authtypes = NULL;
	
	
	/* The work has finished, so clean up and provide the result via the 
	 * main callback: */
	info->result = result;
	
	info->cancelled = camel_operation_cancel_check (priv->cancel);
	
	_tny_camel_account_stop_camel_operation (TNY_CAMEL_ACCOUNT (self));

	/* Create the GError if necessary,
	 * from the CamelException: */
	info->err = NULL;
	if (camel_exception_is_set (&ex))
	{
		g_set_error (&err, TNY_FOLDER_ERROR, 
			TNY_FOLDER_ERROR_REFRESH,
			camel_exception_get_description (&ex));
		if (err != NULL)
			info->err = g_error_copy ((const GError *) err);
	}

	g_static_rec_mutex_unlock (priv->service_lock);

	/* Call the callback, with the result, in an idle thread,
	 * and stop this thread: */
	if (info->callback) {
		g_idle_add_full (G_PRIORITY_HIGH, 
				tny_camel_account_get_supported_secure_authentication_async_callback, 
				info, tny_camel_account_get_supported_secure_authentication_async_destroyer);
	} else {
		/* Thread reference */
		g_object_unref (G_OBJECT (self));
	}
	g_thread_exit (NULL);

	return NULL;
}


/*TODO: This should be a vfunc so that it is generally available 
 * (and implementable) for other imlpementations. But this should not need a 
 * TnyAccount instance, so we need to find some other object to put the vfunc. 
 */
 
/* tny_camel_account_get_supported_secure_authentication:
 * @self: a #TnyCamelAccount object.
 * @callback: A function to be called when the operation is complete.
 * @status_callback: A function to be called one or more times while the 
 * operation is in progress.
 * @user_data: Data to be passed to the callback and status callback.
 * 
 * Query the server for the list of supported secure authentication mechanisms.
 * The #TnyCamelAccount must have a valid hostname and the port number 
 * must be set if appropriate.
 * The returned strings may be used as parameters to 
 * tny_account_set_secure_auth_mech().
 */
void tny_camel_account_get_supported_secure_authentication (
  TnyCamelAccount *self,
  TnyCamelGetSupportedSecureAuthCallback callback,
  TnyStatusCallback status_callback,
  gpointer user_data)
{ 
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	g_return_if_fail (callback);
	g_return_if_fail (priv->session);
	
	
	/* Store all the interesting info in a struct 
	 * launch a thread and keep that struct-instance around.
	 * - While the thread is running, we regularly call the status callback in 
	 * an idle handler.
	 * - When the thread is finished, the main callback will 
	 * then be called in an idle handler.
	 */

	/* Check that the session is ready, and stop with a GError if it is not: */
	GError *err = NULL;
	if (!_tny_session_check_operation (priv->session, &err, 
			TNY_ACCOUNT_ERROR, TNY_ACCOUNT_ERROR_GET_SUPPORTED_AUTH))
	{
		if (callback)
			callback (self, TRUE /* cancelled */, NULL, &err, user_data);
		g_error_free (err);
		return;
	}


	/* Idle info for the status callback: */
	GetSupportedAuthInfo *info = g_slice_new (GetSupportedAuthInfo);
	info->session = priv->session;
	info->err = NULL;
	info->self = self;
	info->callback = callback;
	info->result = NULL;
	info->status_callback = status_callback;
	info->user_data = user_data;
	
	/* Use a ref count because we do not know which of the 2 idle callbacks 
	 * will be the last, and we can only unref self in the last callback:
	 * This is destroyed in the idle GDestroyNotify callback.
	 * A shared copy is taken and released by the _tny_progress* callback,
	 * so that it can prevent the stats callback from being called after 
	 * the main callback. */
	info->stopper = tny_idle_stopper_new();

	/* thread reference */
	g_object_ref (G_OBJECT (self));

	/* This will cause the idle status callback to be called,
	 * via _tny_camel_account_start_camel_operation,
	 * and also calls the idle main callback: */
	printf ("DEBUG: %s: before calling g_thread_create()\n", __FUNCTION__);
	GThread *thread = g_thread_create (
		tny_camel_account_get_supported_secure_authentication_async_thread,
		info, FALSE, NULL);
}


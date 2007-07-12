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
#include <tny-status.h>

#include "tny-session-camel-priv.h"
#include "tny-camel-common-priv.h"

#define TINYMAIL_ENABLE_PRIVATE_API
#include "tny-common-priv.h"
#undef TINYMAIL_ENABLE_PRIVATE_API

#include "tny-camel-account-priv.h"

#include <tny-camel-store-account.h>
#include "tny-camel-store-account-priv.h"

static GObjectClass *parent_class = NULL;

guint tny_camel_account_signals [TNY_CAMEL_ACCOUNT_LAST_SIGNAL];

void
_tny_camel_account_refresh (TnyCamelAccount *self, gboolean recon_if)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	g_static_rec_mutex_lock (apriv->service_lock);

	if (!apriv->custom_url_string)
	{
		CamelURL *url = NULL;
		GList *options = apriv->options;
		gchar *proto;
		gboolean urlneedfree = FALSE;

		if (apriv->proto == NULL)
			goto fail;

		proto = g_strdup_printf ("%s://", apriv->proto); 

		if (camel_exception_is_set (apriv->ex))
			camel_exception_clear (apriv->ex);

		if (!apriv->service) {
			url = camel_url_new (proto, apriv->ex);
			urlneedfree = TRUE;
		} else
			url = apriv->service->url;

		g_free (proto);

		if (!url)
			goto fail;

		camel_url_set_protocol (url, apriv->proto); 
		 if (apriv->user)
			camel_url_set_user (url, apriv->user);
		camel_url_set_host (url, apriv->host);
		if (apriv->port != -1)
			camel_url_set_port (url, (int)apriv->port);
		if (apriv->mech)
			camel_url_set_authmech (url, apriv->mech);

		while (options)
		{
			gchar *ptr, *dup = g_strdup (options->data);
			gchar *option, *value;
			ptr = strchr (dup, '=');
			if (ptr) {
				ptr++;
				value = g_strdup (ptr); ptr--;
				*ptr = '\0'; option = dup;
			} else {
				option = dup;
				value = g_strdup ("1");
			}
			camel_url_set_param (url, option, value);
			g_free (value);
			g_free (dup);
			options = g_list_next (options);
		}
		if (G_LIKELY (apriv->url_string))
			g_free (apriv->url_string);
		apriv->url_string = camel_url_to_string (url, 0);

		if (urlneedfree)
			camel_url_free (url);
	}

	if (recon_if && (apriv->status != TNY_CONNECTION_STATUS_DISCONNECTED))
	{
		CamelException ex = CAMEL_EXCEPTION_INITIALISER;

		if (!apriv->service)
			goto fail;

		apriv->service->reconnecting = TRUE;

		if (apriv->service->reconnecter)
			apriv->service->reconnecter (apriv->service, FALSE, apriv->service->data);

		camel_service_disconnect (apriv->service, FALSE, &ex);
		if (camel_exception_is_set (&ex))
			camel_exception_clear (&ex);

		camel_service_connect (apriv->service, &ex);
		if (apriv->service->reconnection)
		{
			if (!camel_exception_is_set (&ex))
				apriv->service->reconnection (apriv->service, TRUE, apriv->service->data);
			else
				apriv->service->reconnection (apriv->service, FALSE, apriv->service->data);
		}

		apriv->service->reconnecting = FALSE;
		
	}

fail:
	g_static_rec_mutex_unlock (apriv->service_lock);

	return;
}

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
		in = camel_url_new (url_string, &ex);
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

	/* For local maildir accounts, compare their paths, before the folder part. */
	if (in->path && org->path && (strcmp (in->protocol, "maildir") == 0)) {
		gchar *in_path = NULL;
		gchar *in_pos_hash = NULL;
		gchar *org_path = NULL;
		gchar *org_pos_hash = NULL;

		/* The folders have a # before them: */
		/* Copy the paths and set null-termination at the #: */
		in_path = g_strdup (in->path);
		in_pos_hash = strchr (in_path, '#');
		if (in_pos_hash)
			*in_pos_hash = '\0';

		org_path = g_strdup (org->path);
		org_pos_hash = strchr (org_path, '#');
		if (org_pos_hash)
			*org_pos_hash = '\0';

		if (strcmp (in_path, org_path) != 0)
			retval = FALSE;

		g_free (in_path);
		g_free (org_path);
	}

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
 * Add a Camel option to this #TnyCamelAccount instance. 

 * An often used option is the use_ssl one. For example "use_ssl=wrapped" or 
 * "use_ssl=tls" are the typical options added. Other possibilities for the 
 * "use_ssl" option are "never" and "when-possible":
 *
 * use_ssl=wrapped will wrap the connection on default port 993 with IMAP and
 * defualt port 995 with POP3 with SSL or also called "imaps" or "pops".
 *
 * use_ssl=tls will connect using default port 143 for IMAP and 110 for POP and 
 * requires support for STARTTLS, which is often a command for letting the 
 * connection be or become encrypted in IMAP and POP3 servers.
 *
 * use_ssl=when-possible will try to do a STARTTLS, but will fallback to a non
 * encrypted session if it fails (not recommended, as your users will want SSL
 * if they require this security for their accounts).
 *
 * use_ssl=never will forcefully make sure that no SSL is to be used.
 * 
 * One option for some accounts (like the IMAP accounts) is idle_delay. The
 * parameter is the amount of seconds that you want to wait for the IDLE state
 * to be stopped. Stopping the IDLE state will make the server flush all the 
 * pending events for the IDLE state. This improve responsibility of the Push 
 * E-mail and expunge events, although it will cause a little bit more continuous
 * bandwidth consumption (each delayth second). For example idle_delay=20. The
 * defualt value is 20.
 *
 * Another option is getsrv_delay, also for IMAP accounts, which allows you to 
 * specify the delay before the connection that gets created for receiving 
 * uncached messages gets shut down. If this service is not yet shut down, then
 * it'll be reused. Else a new one will be created that will be kept around for
 * delay seconds (in the hopes that new requests will happen). Keeping a socket
 * open for a long period of time might not be ideal for some situations, but
 * closing it very quickly will let almost each request create a new connection
 * with the IMAP server. Which is why you can play with the value yourself. For
 * example getsrv_delay=100. The default value is 100.
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

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (self, TRUE, FALSE);

	return;
}

TnyError
_tny_camel_account_get_tny_error_code_for_camel_exception_id (CamelException* ex)
{
	/* printf ("DEBUG: %s: ex->id=%d, ex->desc=%s\n", __FUNCTION__, ex->id, ex->desc); */
	if (!ex) {
		g_warning ("%s: The exception was NULL.\n", __FUNCTION__);
		return TNY_ACCOUNT_ERROR_TRY_CONNECT;
	}


	/* Try to provide a precise error code, 
	 * as much as possible: 
	 */
	switch (ex->id) {
	case CAMEL_EXCEPTION_SYSTEM_HOST_LOOKUP_FAILED:
		return TNY_ACCOUNT_ERROR_TRY_CONNECT_HOST_LOOKUP_FAILED;
	case CAMEL_EXCEPTION_SERVICE_UNAVAILABLE:
		return TNY_ACCOUNT_ERROR_TRY_CONNECT_SERVICE_UNAVAILABLE;
	case CAMEL_EXCEPTION_SERVICE_CANT_AUTHENTICATE:
		return TNY_ACCOUNT_ERROR_TRY_CONNECT_AUTHENTICATION_NOT_SUPPORTED;
	case CAMEL_EXCEPTION_SERVICE_CERTIFICATE:
		return TNY_ACCOUNT_ERROR_TRY_CONNECT_CERTIFICATE;
	case CAMEL_EXCEPTION_USER_CANCEL:
		/* TODO: This really shouldn't be shown to the user: */
		return TNY_ACCOUNT_ERROR_TRY_CONNECT_USER_CANCEL;
	case CAMEL_EXCEPTION_SYSTEM:
	default:
		/* A generic exception. 
		 * We should always try to provide a precise error code rather than this,
		 * so we can show a more helpful (translated) error message to the user.
		 */
		return TNY_ACCOUNT_ERROR_TRY_CONNECT;
	}
}

void 
_tny_camel_account_try_connect (TnyCamelAccount *self, gboolean for_online, GError **err)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (TNY_CAMEL_ACCOUNT (self), for_online, TRUE);
	if (camel_exception_is_set (priv->ex))
	{
		g_set_error (err, TNY_ACCOUNT_ERROR, 
			_tny_camel_account_get_tny_error_code_for_camel_exception_id (priv->ex),
			camel_exception_get_description (priv->ex));
		/* printf ("DEBUG: %s: camel exception: message=%s\n", __FUNCTION__, (*err)->message); */
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

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (TNY_CAMEL_ACCOUNT (self), TRUE, TRUE);

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
			/*while (!camel_operation_cancel_check (priv->cancel)) 
			{ 
				g_warning (_("Cancellation failed, retrying\n"));
				thread = g_thread_create (camel_cancel_hack_thread, NULL, TRUE, NULL);
				g_thread_join (thread);
			}*/
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

static void 
tny_camel_account_start_operation (TnyAccount *self, TnyStatusDomain domain, TnyStatusCode code, TnyStatusCallback status_callback, gpointer status_user_data)
{
	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->start_operation_func (self, domain, code, status_callback, status_user_data);
}

static void
tny_camel_account_stop_operation (TnyAccount *self, gboolean *canceled)
{
	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->stop_operation_func (self, canceled);
}




static void
refresh_status (struct _CamelOperation *op, const char *what, int sofar, int oftotal, void *user_data)
{
	RefreshStatusInfo *oinfo = user_data;
	TnyProgressInfo *info = NULL;

	info = tny_progress_info_new (G_OBJECT (oinfo->self), oinfo->status_callback, 
		oinfo->domain, oinfo->code, what, sofar, 
		oftotal, oinfo->stopper, oinfo->user_data);

	if (oinfo->depth > 0)
	{
		g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
			tny_progress_info_idle_func, info, 
			tny_progress_info_destroy);
	} else {
		tny_progress_info_idle_func (info);
		tny_progress_info_destroy (info);
	}

	return;
}

static void 
tny_camel_account_start_operation_default (TnyAccount *self, TnyStatusDomain domain, TnyStatusCode code, TnyStatusCallback status_callback, gpointer status_user_data)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	if (!priv->csyncop)
	{
		RefreshStatusInfo *info = g_slice_new (RefreshStatusInfo);

		info->self = TNY_ACCOUNT (g_object_ref (self));
		info->domain = domain;
		info->code = code;
		info->status_callback = status_callback;
		info->depth = g_main_depth ();
		info->user_data = status_user_data;
		info->stopper = tny_idle_stopper_new();

		priv->csyncop = info;

		_tny_camel_account_start_camel_operation_n (TNY_CAMEL_ACCOUNT (self), 
				refresh_status, info, "Starting operation", FALSE);

	} else
		g_critical ("Another synchronous operation is already in "
				"progress. This indicates an error in the "
				"software.");

	return;
}

static void
tny_camel_account_stop_operation_default (TnyAccount *self, gboolean *canceled)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);

	if (priv->csyncop)
	{
		RefreshStatusInfo *info = priv->csyncop;

		tny_idle_stopper_stop (info->stopper);
		tny_idle_stopper_destroy (info->stopper);
		info->stopper = NULL;
		g_object_unref (info->self);
		g_slice_free (RefreshStatusInfo, info);
		priv->csyncop = NULL;

		if (canceled)
			*canceled = camel_operation_cancel_check (priv->cancel);

		_tny_camel_account_stop_camel_operation (TNY_CAMEL_ACCOUNT (self));
	} else
		g_critical ("No synchronous operation was in "
				"progress while trying to stop one "
				"This indicates an error in the software.");

}


static TnyConnectionStatus 
tny_camel_account_get_connection_status (TnyAccount *self)
{
	return TNY_CAMEL_ACCOUNT_GET_CLASS (self)->get_connection_status_func (self);
}

static TnyConnectionStatus 
tny_camel_account_get_connection_status_default (TnyAccount *self)
{
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	return priv->status;
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

	camel_object_ref (session);
	priv->session = session;

	_tny_session_camel_add_account_1 (session, self);

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (self, FALSE, FALSE);

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

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (TNY_CAMEL_ACCOUNT (self), TRUE, FALSE);

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

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (TNY_CAMEL_ACCOUNT (self), TRUE, FALSE);

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

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (TNY_CAMEL_ACCOUNT (self), 
		!priv->in_auth, FALSE);

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

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (TNY_CAMEL_ACCOUNT (self), 
		TRUE, FALSE);

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

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (TNY_CAMEL_ACCOUNT (self), 
		TRUE, FALSE);

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
	gboolean reconf_if = FALSE;

	g_static_rec_mutex_lock (priv->service_lock);

	priv->get_pass_func = get_pass_func;
	priv->pass_func_set = TRUE;

	/*if (TNY_IS_CAMEL_STORE_ACCOUNT (self))
		reconf_if = TRUE;*/

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (TNY_CAMEL_ACCOUNT (self), 
		reconf_if, TRUE);

	if (priv->session)
		_tny_session_camel_add_account_2 (priv->session, TNY_CAMEL_ACCOUNT (self));

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
	gboolean reconf_if = FALSE;

	g_static_rec_mutex_lock (priv->service_lock);

	priv->forget_pass_func = get_forget_pass_func;
	priv->forget_pass_func_set = TRUE;

	/*if (TNY_IS_CAMEL_STORE_ACCOUNT (self))
		reconf_if = TRUE;*/

	TNY_CAMEL_ACCOUNT_GET_CLASS (self)->prepare_func (TNY_CAMEL_ACCOUNT (self), 
		reconf_if, FALSE);

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

	priv->is_connecting = FALSE;
	priv->in_auth = FALSE;
	priv->csyncop = NULL;
	priv->get_pass_func = NULL;
	priv->forget_pass_func = NULL;
	priv->port = -1;
	priv->cache_location = NULL;
	priv->service = NULL;
	priv->session = NULL;
	priv->url_string = NULL;
	priv->status = TNY_CONNECTION_STATUS_INIT;

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

	priv->account_lock = g_new (GStaticRecMutex, 1);
	g_static_rec_mutex_init (priv->account_lock);

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
_tny_camel_account_set_online (TnyCamelAccount *self, gboolean online, GError **err)
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
				_tny_camel_account_get_tny_error_code_for_camel_exception_id (priv->ex),
				camel_exception_get_description (priv->ex));
		} else {
			g_set_error (err, TNY_ACCOUNT_ERROR, 
				TNY_ACCOUNT_ERROR_TRY_CONNECT,
				"Account not yet fully configured. "
				"This problem indicates a bug in the software.");
		}

		return;
	}

	if (CAMEL_IS_DISCO_STORE (priv->service)) {
		if (online) {
			camel_disco_store_set_status (CAMEL_DISCO_STORE (priv->service),
					CAMEL_DISCO_STORE_ONLINE, &ex);

			if (!camel_exception_is_set (&ex))
				camel_service_connect (CAMEL_SERVICE (priv->service), &ex);

			if (TNY_IS_CAMEL_STORE_ACCOUNT (self)) 
			{
				if (!camel_exception_is_set (&ex))
					priv->status = TNY_CONNECTION_STATUS_CONNECTED;
				else
					priv->status = TNY_CONNECTION_STATUS_CONNECTED_BROKEN;

				_tny_camel_store_account_emit_conchg_signal (TNY_CAMEL_STORE_ACCOUNT (self));
			}

			if (!camel_exception_is_set (&ex))
				g_signal_emit (self, 
					tny_camel_account_signals [TNY_CAMEL_ACCOUNT_SET_ONLINE_HAPPENED], 0, TRUE);

			goto done;

		} else if (camel_disco_store_can_work_offline (CAMEL_DISCO_STORE (priv->service))) {
			
			camel_disco_store_set_status (CAMEL_DISCO_STORE (priv->service),
					CAMEL_DISCO_STORE_OFFLINE, &ex);

			if (TNY_IS_CAMEL_STORE_ACCOUNT (self)) 
			{
				if (!camel_exception_is_set (&ex))
					priv->status = TNY_CONNECTION_STATUS_DISCONNECTED;
				else
					priv->status = TNY_CONNECTION_STATUS_DISCONNECTED_BROKEN;

				_tny_camel_store_account_emit_conchg_signal (TNY_CAMEL_STORE_ACCOUNT (self));
			}

			goto done;
		}
	} else if (CAMEL_IS_OFFLINE_STORE (priv->service)) {
		
		if (online) {
			
			camel_offline_store_set_network_state (CAMEL_OFFLINE_STORE (priv->service),
					CAMEL_OFFLINE_STORE_NETWORK_AVAIL, &ex);

			if (TNY_IS_CAMEL_STORE_ACCOUNT (self)) 
			{
				if (!camel_exception_is_set (&ex))
					priv->status = TNY_CONNECTION_STATUS_CONNECTED;
				else
					priv->status = TNY_CONNECTION_STATUS_CONNECTED_BROKEN;

				_tny_camel_store_account_emit_conchg_signal (TNY_CAMEL_STORE_ACCOUNT (self));
			}

			if (!camel_exception_is_set (&ex))
				g_signal_emit (self, 
					tny_camel_account_signals [TNY_CAMEL_ACCOUNT_SET_ONLINE_HAPPENED], 0, TRUE);

			goto done;
		} else {
			camel_offline_store_set_network_state (CAMEL_OFFLINE_STORE (priv->service),
					CAMEL_OFFLINE_STORE_NETWORK_UNAVAIL, &ex);

			if (TNY_IS_CAMEL_STORE_ACCOUNT (self)) 
			{
				if (!camel_exception_is_set (&ex))
					priv->status = TNY_CONNECTION_STATUS_DISCONNECTED;
				else
					priv->status = TNY_CONNECTION_STATUS_DISCONNECTED_BROKEN;

				_tny_camel_store_account_emit_conchg_signal (TNY_CAMEL_STORE_ACCOUNT (self));
			}

			goto done;
		}
	}

	if (!online) {
		camel_service_disconnect (CAMEL_SERVICE (priv->service),
			  TRUE, &ex);

		if (TNY_IS_CAMEL_STORE_ACCOUNT (self)) 
		{
			if (!camel_exception_is_set (&ex))
				priv->status = TNY_CONNECTION_STATUS_DISCONNECTED;
			else
				priv->status = TNY_CONNECTION_STATUS_DISCONNECTED_BROKEN;

			_tny_camel_store_account_emit_conchg_signal (TNY_CAMEL_STORE_ACCOUNT (self));
		}

		if (!camel_exception_is_set (&ex))
			g_signal_emit (self, 
				tny_camel_account_signals [TNY_CAMEL_ACCOUNT_SET_ONLINE_HAPPENED], 0, FALSE);

	}

done:

	if (camel_exception_is_set (&ex))
	{
		g_set_error (err, TNY_ACCOUNT_ERROR, 
			_tny_camel_account_get_tny_error_code_for_camel_exception_id (&ex),
			camel_exception_get_description (&ex));
	}

	return;
}


typedef struct {
	TnyCamelAccount *self;
	gboolean online;
	GError **err;
	GMainLoop *loop;
} SetOnlineInfo;

static gpointer
set_online_thread (gpointer data)
{
	SetOnlineInfo *info = (SetOnlineInfo *) data;

	_tny_camel_account_set_online (info->self, info->online, info->err);

	if (g_main_loop_is_running (info->loop))
		g_main_loop_quit (info->loop);

	g_thread_exit (NULL);
	return NULL;
}

void 
tny_camel_account_set_online_default (TnyCamelAccount *self, gboolean online, GError **err)
{
	if (g_main_depth () != 0)
	{
		/* We are being called from the mainnloop */
		SetOnlineInfo *info = g_slice_new (SetOnlineInfo);
		GThread *thread = NULL;

		info->self = TNY_CAMEL_ACCOUNT (g_object_ref (self));
		info->online = online;
		info->err = err;
		info->loop = g_main_loop_new (NULL, FALSE);

		thread = g_thread_create (set_online_thread, info, TRUE, NULL);

		g_main_loop_run (info->loop);

		g_main_loop_unref (info->loop);
		g_object_unref (info->self);
		g_slice_free (SetOnlineInfo, info);

	} else
		_tny_camel_account_set_online (self, online, err);
}


static void
tny_camel_account_finalize (GObject *object)
{
	TnyCamelAccount *self = (TnyCamelAccount *)object;
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;


	if (priv->service && CAMEL_IS_SERVICE (priv->service))
	{
		priv->service->connecting = NULL;
		priv->service->disconnecting = NULL;
		priv->service->reconnecter = NULL;
		priv->service->reconnection = NULL;
		camel_service_disconnect (CAMEL_SERVICE (priv->service), FALSE, &ex);
	}

	if (priv->session) {
		_tny_session_camel_forget_account (priv->session, (TnyCamelAccount*) object);    
		camel_object_unref (priv->session);
	}
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
	g_static_rec_mutex_free (priv->account_lock);

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
	klass->get_connection_status_func = tny_camel_account_get_connection_status;
	klass->set_url_string_func = tny_camel_account_set_url_string;
	klass->get_url_string_func = tny_camel_account_get_url_string;
	klass->get_name_func = tny_camel_account_get_name;
	klass->set_name_func = tny_camel_account_set_name;
	klass->get_account_type_func = tny_camel_account_get_account_type;
	klass->cancel_func = tny_camel_account_cancel;
	klass->matches_url_string_func = tny_camel_account_matches_url_string;
	klass->start_operation_func = tny_camel_account_start_operation;
	klass->stop_operation_func =  tny_camel_account_stop_operation;

	return;
}

static void 
tny_camel_account_class_init (TnyCamelAccountClass *class)
{
	GObjectClass *object_class;
	static gboolean initialized = FALSE;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;


	if (!initialized) {
		/* create interface signals here. */

/**
 * TnyCamelAccount::set-online-happened
 * @self: the object on which the signal is emitted
 * @online: whether it was online
 * @user_data: user data set when the signal handler was connected.
 *
 * Emitted when tny_camel_account_set_online happened
 **/
		tny_camel_account_signals[TNY_CAMEL_ACCOUNT_SET_ONLINE_HAPPENED] =
		   g_signal_new ("set_online_happened",
			TNY_TYPE_CAMEL_ACCOUNT,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnyCamelAccountClass, set_online_happened_func),
			NULL, NULL,
			g_cclosure_marshal_VOID__BOOLEAN, 
			G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
	}

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
	class->get_connection_status_func = tny_camel_account_get_connection_status_default;
	class->set_url_string_func = tny_camel_account_set_url_string_default;
	class->get_url_string_func = tny_camel_account_get_url_string_default;
	class->get_name_func = tny_camel_account_get_name_default;
	class->set_name_func = tny_camel_account_set_name_default;
	class->get_account_type_func = tny_camel_account_get_account_type_default;
	class->cancel_func = tny_camel_account_cancel_default;
	class->matches_url_string_func = tny_camel_account_matches_url_string_default;
	class->start_operation_func = tny_camel_account_start_operation_default;
	class->stop_operation_func =  tny_camel_account_stop_operation_default;

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

/* Starts the operation in the thread: */
static gpointer 
tny_camel_account_get_supported_secure_authentication_async_thread (
	gpointer thr_user_data)
{
	GetSupportedAuthInfo *info = thr_user_data;
	TnyCamelAccount *self = info->self;
	TnyCamelAccountPriv *priv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (self);
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	GError *err = NULL;
	TnyStatus* status;

	g_static_rec_mutex_lock (priv->service_lock);

	status =  tny_status_new_literal(TNY_GET_SUPPORTED_SECURE_AUTH_STATUS, 
		TNY_GET_SUPPORTED_SECURE_AUTH_STATUS_GET_SECURE_AUTH, 0, 1,
		"Get secure authentication methods");

	info->status_callback(G_OBJECT(self), 
		status, info->user_data);
  
	/* Do the actual work:
	 * This is happening in a thread, 
	 * and the status callback is being called regularly while this is 
	 * happening. */	
	GList *authtypes = camel_service_query_auth_types (priv->service, &ex);

	tny_status_set_fraction(status, 1);
	info->status_callback(G_OBJECT(self), 
		status, info->user_data);

	/* The result will be a TnyList of TnyPairs: */
	TnyList *result = tny_simple_list_new ();
	GList *iter = authtypes;
	while (iter) {
		CamelServiceAuthType *item = (CamelServiceAuthType *)iter->data;
		if (item) {
			/* Get the name of the auth method:
			 * Note that, at least for IMAP, authproto=NULL when 
			 * name=Password. */

			/* We don't use the value part of the TnyPair. */
			TnyPair *pair = tny_pair_new (item->name, NULL);
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
		info->callback (info->self, info->cancelled, info->result, &info->err, info->user_data);
	} else {
		/* Thread reference */
		g_object_unref (G_OBJECT (self));
	}
	tny_status_free(status);
	g_thread_exit (NULL);

	return NULL;
}


/*TODO: This should be a vfunc so that it is generally available 
 * (and implementable) for other imlpementations. But this should not need a 
 * TnyAccount instance, so we need to find some other object to put the vfunc. 
 */
 
/** 
 * TnyCamelGetSupportedSecureAuthCallback:
 * @self: The TnyCamelAccount on which tny_camel_account_get_supported_secure_authentication() was called.
 * @cancelled: Whether the operation was cancelled.
 * @auth_types: A TnyList of TnyPair objects. Each TnyPair in the list has a supported secure authentication method name as its name. This list must be freed with g_object_unref().
 * @err: A GError if an error occurred, or NULL. This must be freed with g_error_free().
 * @user_data: The user data that was provided to tny_camel_account_get_supported_secure_authentication().
 * 
 * The callback for tny_camel_account_get_supported_secure_authentication().
 **/

/**
 * tny_camel_account_get_supported_secure_authentication:
 * @self: a #TnyCamelAccount object.
 * @callback: A function to be called when the operation is complete.
 * @status_callback: A function to be called one or more times while the operation is in progress.
 * @user_data: Data to be passed to the callback and status callback.
 * 
 * Query the server for the list of supported secure authentication mechanisms.
 * The #TnyCamelAccount must have a valid hostname and the port number 
 * must be set if appropriate.
 * The returned strings may be used as parameters to 
 * tny_account_set_secure_auth_mech().
 **/
void 
tny_camel_account_get_supported_secure_authentication (TnyCamelAccount *self, TnyCamelGetSupportedSecureAuthCallback callback, TnyStatusCallback status_callback, gpointer user_data)
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
	if (!_tny_session_check_operation (priv->session, TNY_ACCOUNT (self), &err, 
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

	g_thread_create (tny_camel_account_get_supported_secure_authentication_async_thread,
		info, FALSE, NULL);
}


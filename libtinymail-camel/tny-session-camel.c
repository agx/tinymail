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

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib/gstdio.h>

#include <camel/camel.h>
#include <camel/camel-filter-driver.h>

#include <camel/camel-store.h>
#include <camel/camel.h>
#include <camel/camel-session.h>

#include <tny-session-camel.h>
#include <tny-account.h>

#include <tny-device.h>
#include <tny-account-store.h>
#include <tny-camel-store-account.h>
#include <tny-camel-transport-account.h>

#include <tny-noop-lockable.h>

#include "tny-session-camel-priv.h"
#include "tny-camel-store-account-priv.h"
#include "tny-camel-transport-account-priv.h"
#include "tny-camel-account-priv.h"

#include <tny-camel-shared.h>

gboolean camel_type_init_done = FALSE;

static CamelSessionClass *ms_parent_class;

static void tny_session_camel_forget_password (CamelSession *, CamelService *, const char *, const char *, CamelException *);

static char *
tny_session_camel_get_password (CamelSession *session, CamelService *service, const char *domain,
		const char *prompt, const char *item, guint32 flags, CamelException *ex)
{
	TnySessionCamel *self = (TnySessionCamel *) session;
	TnySessionCamelPriv *priv = self->priv;

	TnyGetPassFunc func;
	TnyAccount *account;
	gboolean freeprmpt = FALSE, cancel = FALSE;
	gchar *retval = NULL, *prmpt = (gchar*)prompt;

	account = service->data;
	if (account)
	{

		func = tny_account_get_pass_func (account);

		if (!func)
			return g_strdup ("");

		if (prmpt == NULL)
		{
			freeprmpt = TRUE;
			prmpt = g_strdup_printf (_("Enter password for %s"), 
				tny_account_get_name (account));
		}
		
		/* TODO: fix this in camel-lite ! */

		if (!g_ascii_strncasecmp (tny_account_get_proto (account), "pop", 3))
		{
			if (flags & CAMEL_SESSION_PASSWORD_REPROMPT)
				tny_session_camel_forget_password (session, service, domain, item, ex);
		}

		tny_lockable_lock (self->priv->ui_lock);
		retval = func (account, prmpt, &cancel);
		tny_lockable_unlock (self->priv->ui_lock);

		if (freeprmpt)
			g_free (prmpt);
	}

	if (cancel || retval == NULL) {
		camel_exception_set (ex, CAMEL_EXCEPTION_USER_CANCEL,
			_("You cancelled when you had to enter a password"));
		retval = NULL;
	}

emptypass:

	return retval;
}

/**
 * tny_session_camel_set_ui_locker:
 * @self: a #TnySessionCamel instance
 * @ui_lock: a #TnyLockable instance 
 *
 * Set the user interface toolkit locker. The lock and unlock methods of this
 * locker should be implemented with the lock and unlock functionality of your
 * user interface toolkit.
 *
 * Good examples are gdk_threads_enter () and gdk_threads_leave () in gtk+.
 **/
void 
tny_session_camel_set_ui_locker (TnySessionCamel *self, TnyLockable *ui_lock)
{
	TnySessionCamelPriv *priv = self->priv;
	if (priv->ui_lock)
		g_object_unref (G_OBJECT (priv->ui_lock));
	priv->ui_lock = TNY_LOCKABLE (g_object_ref (ui_lock));
	return;
}



static void
tny_session_camel_forget_password (CamelSession *session, CamelService *service, const char *domain, const char *item, CamelException *ex)
{
	TnySessionCamel *self = (TnySessionCamel *)session;
	TnySessionCamelPriv *priv = self->priv;

	TnyForgetPassFunc func;
	TnyAccount *account;

	account = service->data;

	if (account)
	{
		func = tny_account_get_forget_pass_func (account);

		if (!func)
			return;

		tny_lockable_lock (self->priv->ui_lock);
		func (account);
		tny_lockable_unlock (self->priv->ui_lock);
	}

	return;
}


/* tny_session_camel_alert_user will for example be called when SSL is on and 
   camel_session_get_service is issued (for example TnyCamelTransportAccount and
   TnyCamelStore account issue this function). Its implementation is often done
   with GUI components (it should therefore not be called from a thread). This
   is a known issue (and the person who fixed this, please remove this warning) */

static gboolean
tny_session_camel_alert_user (CamelSession *session, CamelSessionAlertType type, const char *prompt, gboolean cancel)
{
	TnySessionCamel *self = (TnySessionCamel *)session;
	TnySessionCamelPriv *priv = self->priv;
	GThread *thread; gboolean inf;
	gboolean retval = FALSE;

	if (priv->account_store)
	{
		TnyAccountStore *account_store = (TnyAccountStore*) priv->account_store;
		TnyAlertType tnytype;

		switch (type)
		{
			case CAMEL_SESSION_ALERT_INFO:
				tnytype = TNY_ALERT_TYPE_INFO;
			break;
			case CAMEL_SESSION_ALERT_WARNING:
				tnytype = TNY_ALERT_TYPE_WARNING;
			break;
			case CAMEL_SESSION_ALERT_ERROR:
			default:
				tnytype = TNY_ALERT_TYPE_ERROR;
			break;
		}

		tny_lockable_lock (self->priv->ui_lock);
		retval = tny_account_store_alert (
			(TnyAccountStore*) self->priv->account_store, 
			tnytype, (const gchar *) prompt);
		tny_lockable_unlock (self->priv->ui_lock);
	}

	return retval;
}


CamelFolder *
mail_tool_uri_to_folder (CamelSession *session, const char *uri, guint32 flags, CamelException *ex)
{
	CamelURL *url;
	CamelStore *store = NULL;
	CamelFolder *folder = NULL;
	int offset = 0;
	char *curi = NULL;

	g_return_val_if_fail (uri != NULL, NULL);
	
	url = camel_url_new (uri + offset, ex);

	if (G_UNLIKELY (!url))
	{
		g_free(curi);
		return NULL;
	}

	store = (CamelStore *)camel_session_get_service(session, uri+offset, CAMEL_PROVIDER_STORE, ex);
	if (G_LIKELY (store))
	{
		const char *name;

		if (url->fragment) 
		{
			name = url->fragment;
		} else {
			if (url->path && *url->path)
				name = url->path + 1;
			else
				name = "";
		}

		if (offset) 
		{
			if (offset == 7)
				folder = (CamelFolder*)camel_store_get_trash (store, ex);
			else if (offset == 6)
				folder = (CamelFolder*)camel_store_get_junk (store, ex);
			else
				g_assert (FALSE);
		} else
			folder = (CamelFolder*)camel_store_get_folder (store, name, flags, ex);
		camel_object_unref (store);
	}

	camel_url_free (url);
	g_free(curi);

	return folder;
}


static CamelFolder *
get_folder (CamelFilterDriver *d, const char *uri, void *data, CamelException *ex)
{
	CamelSession *session = data;
	return mail_tool_uri_to_folder(session, uri, 0, ex);
}

static CamelFilterDriver *
tny_session_camel_get_filter_driver (CamelSession *session, const char *type, CamelException *ex)
{
	CamelFilterDriver *driver = camel_filter_driver_new (session);
	camel_filter_driver_set_folder_func (driver, get_folder, session);
	return driver; 
}


static void 
my_receive_func(CamelSession *session, struct _CamelSessionThreadMsg *m)
{
	return;
}

static void
my_free_func (CamelSession *session, struct _CamelSessionThreadMsg *m)
{
	return;
}


static void 
my_cancel_func (struct _CamelOperation *op, const char *what, int sofar, int oftotal, void *data)
{
	return;
}

static void *
tny_session_camel_ms_thread_msg_new (CamelSession *session, CamelSessionThreadOps *ops, unsigned int size)
{
	CamelSessionThreadMsg *msg = ms_parent_class->thread_msg_new(session, ops, size);

	msg->ops = g_new0 (CamelSessionThreadOps,1);
	msg->ops->free = my_free_func;
	msg->ops->receive = my_receive_func;
	msg->data = NULL;
	msg->op = camel_operation_new (my_cancel_func, NULL);

	return msg;

}

static void
tny_session_camel_ms_thread_msg_free (CamelSession *session, CamelSessionThreadMsg *m)
{
	ms_parent_class->thread_msg_free(session, m);
	return;
}

static void
tny_session_camel_ms_thread_status (CamelSession *session, CamelSessionThreadMsg *msg, const char *text, int pc)
{
	return;
}

/**
 * tny_session_camel_set_async_connecting:
 * @self: a #TnySessionCamel object
 * @enable: Whether or not to asynchronously connect
 *
 * Set connection strategy
 **/
void 
tny_session_camel_set_async_connecting (TnySessionCamel *self, gboolean enable)
{
	g_mutex_lock (self->priv->conlock);
	self->priv->async_connect = enable;
	g_mutex_unlock (self->priv->conlock);
}


static void
tny_session_camel_init (TnySessionCamel *instance)
{
	TnySessionCamelPriv *priv;
	instance->priv = g_slice_new (TnySessionCamelPriv);
	priv = instance->priv;

	priv->is_inuse = FALSE;
	priv->conlock = g_mutex_new ();
	priv->conthread = NULL;
	priv->current_accounts = NULL;
	priv->prev_constat = FALSE;
	priv->device = NULL;
	priv->camel_dir = NULL;
	priv->ui_lock = tny_noop_lockable_new ();
	priv->camel_dir = NULL;
	priv->in_auth_function = FALSE;
	priv->is_connecting = FALSE;
	priv->async_connect = TRUE;

	return;
}

void 
_tny_session_camel_add_account (TnySessionCamel *self, TnyCamelAccount *account)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (account);
	TnySessionCamelPriv *priv = self->priv;

	if (apriv->cache_location)
		g_free (apriv->cache_location);
	apriv->cache_location = g_strdup (priv->camel_dir);

	priv->current_accounts = g_list_prepend (priv->current_accounts, account);
}

void 
_tny_session_camel_forget_account (TnySessionCamel *self, TnyCamelAccount *account)
{
	TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (account);
	TnySessionCamelPriv *priv = self->priv;

	if (apriv->cache_location)
		g_free (apriv->cache_location);
	apriv->cache_location = NULL;

	priv->current_accounts = g_list_remove (priv->current_accounts, account);

	return;
}

typedef struct
{
	TnyDevice *device;
	gboolean online;
	gpointer user_data;
} BackgroundConnectInfo;

static void
foreach_account_set_connectivity (gpointer data, gpointer udata)
{
	BackgroundConnectInfo *info = udata;
	CamelSession *session = info->user_data;

	if (data && TNY_IS_CAMEL_ACCOUNT (data))
	{
		GError *err = NULL;

		_tny_camel_account_try_connect (TNY_CAMEL_ACCOUNT (data), &err);

		if (err == NULL)
			tny_camel_account_set_online (TNY_CAMEL_ACCOUNT (data), info->online, &err);

		if (err != NULL) 
		{
			tny_session_camel_alert_user (session, CAMEL_SESSION_ALERT_ERROR, err->message, FALSE);
			g_error_free (err);
		}
	}
}


static void
background_connect_destroy (gpointer data)
{
	BackgroundConnectInfo *info = data;

	g_object_unref (G_OBJECT (info->device));
	g_slice_free (BackgroundConnectInfo, data);

	return;
}

static gboolean
background_connect_idle (gpointer data)
{
	BackgroundConnectInfo *info = data;
	TnySessionCamel *self = info->user_data;
	TnySessionCamelPriv *priv = self->priv;

	if (priv->account_store)
		g_signal_emit (priv->account_store, 
			tny_account_store_signals [TNY_ACCOUNT_STORE_ACCOUNTS_RELOADED], 0);

	return FALSE;
}

static gpointer 
background_connect_thread (gpointer data)
{
	BackgroundConnectInfo *info = data;
	TnySessionCamel *self = info->user_data;
	TnySessionCamelPriv *priv = self->priv;

	g_mutex_lock (priv->conlock);

	priv->is_connecting = TRUE;

	if (priv->current_accounts &&
		priv->prev_constat != info->online && priv->account_store)

	{
		g_list_foreach (priv->current_accounts, 
			foreach_account_set_connectivity, info);
	}

	priv->is_connecting = FALSE;

	g_idle_add_full (G_PRIORITY_HIGH, 
		background_connect_idle, 
		info, background_connect_destroy);

	priv->prev_constat = info->online;

	g_mutex_unlock (priv->conlock);

	return NULL;
}

static void
connection_changed (TnyDevice *device, gboolean online, gpointer user_data)
{
	TnySessionCamel *self = user_data;
	TnySessionCamelPriv *priv = self->priv;
	BackgroundConnectInfo *info;
	gboolean inf;

	inf = priv->in_auth_function;
	if (inf) return;

	info = g_slice_new (BackgroundConnectInfo);

	info->device = g_object_ref (G_OBJECT (device));
	info->online = online;
	info->user_data = user_data;

	camel_session_set_online ((CamelSession *) self, online); 

	if (priv->async_connect)
		priv->conthread = g_thread_create (background_connect_thread, info, TRUE, NULL);
	else
		background_connect_thread (info);

	return;
}

/**
 * tny_session_camel_join_connecting
 * @self: a #TnySessionCamel object
 * 
 * Join the connection thread
 **/ 
void 
tny_session_camel_join_connecting (TnySessionCamel *self)
{
	TnySessionCamelPriv *priv = self->priv;
	
	if (priv->conthread)
		g_thread_join (priv->conthread);
}


/**
 * tny_session_camel_set_device:
 * @self: a #TnySessionCamel object
 * @device: a #TnyDevice instance
 *
 *
 **/
void 
tny_session_camel_set_device (TnySessionCamel *self, TnyDevice *device)
{
	TnySessionCamelPriv *priv = self->priv;

	if (priv->device && g_signal_handler_is_connected (G_OBJECT (priv->device), 
		priv->connchanged_signal))
	{
		g_signal_handler_disconnect (G_OBJECT (device), 
			priv->connchanged_signal);
	}

	priv->device = device;
	priv->connchanged_signal = g_signal_connect (
			G_OBJECT (device), "connection_changed",
			G_CALLBACK (connection_changed), self);

	return;
}

/**
 * tny_session_camel_set_account_store:
 * @self: a #TnySessionCamel object
 * @account_store: A #TnyAccountStore account store instance
 *
 *
 **/
void 
tny_session_camel_set_account_store (TnySessionCamel *self, TnyAccountStore *account_store)
{
	CamelSession *session = (CamelSession*) self;
	TnyDevice *device = (TnyDevice*)tny_account_store_get_device (account_store);
	gchar *base_directory = NULL;
	gchar *camel_dir = NULL;
	gboolean online;
	TnySessionCamelPriv *priv = self->priv;

	priv->account_store = (gpointer)account_store;    
	base_directory = g_strdup (tny_account_store_get_cache_dir (account_store));

	if (camel_init (base_directory, TRUE) != 0)
	{
		g_error (_("Critical ERROR: Cannot init %s as camel directory\n"), base_directory);
		g_object_unref (G_OBJECT (device));
		exit (1);
	}

	camel_dir = g_build_filename (base_directory, "mail", NULL);
	camel_provider_init();
	camel_session_construct (session, camel_dir);

	online = tny_device_is_online (device);
	camel_session_set_online ((CamelSession *) session, online); 
	priv->camel_dir = camel_dir;
	g_free (base_directory);
	tny_session_camel_set_device (self, device);
	g_object_unref (G_OBJECT (device));

	return;
}




/**
 * tny_session_camel_new:
 * @account_store: A TnyAccountStore instance
 *
 * Return value: The #TnySessionCamel singleton instance
 **/
TnySessionCamel*
tny_session_camel_new (TnyAccountStore *account_store)
{
	TnySessionCamel *retval = TNY_SESSION_CAMEL 
			(camel_object_new (TNY_TYPE_SESSION_CAMEL));

	tny_session_camel_set_account_store (retval, account_store);

	return retval;
}


static void
tny_session_camel_finalise (CamelObject *object)
{
	TnySessionCamel *self = (TnySessionCamel*)object;
	TnySessionCamelPriv *priv = self->priv;

	if (priv->device && g_signal_handler_is_connected (G_OBJECT (priv->device), priv->connchanged_signal))
	{
		g_signal_handler_disconnect (G_OBJECT (priv->device), 
			priv->connchanged_signal);
	}

	if (priv->ui_lock)
		g_object_unref (G_OBJECT (priv->ui_lock));

	if (priv->camel_dir)
		g_free (priv->camel_dir);

	g_mutex_free (priv->conlock);

	g_slice_free (TnySessionCamelPriv, self->priv);

	/* CamelObject types don't need parent finalization (build-in camel) */

	return;
}

static void
tny_session_camel_class_init (TnySessionCamelClass *tny_session_camel_class)
{
	CamelSessionClass *camel_session_class = CAMEL_SESSION_CLASS (tny_session_camel_class);

	camel_session_class->get_password = tny_session_camel_get_password;
	camel_session_class->forget_password = tny_session_camel_forget_password;
	camel_session_class->alert_user = tny_session_camel_alert_user;
	camel_session_class->get_filter_driver = tny_session_camel_get_filter_driver;
	camel_session_class->thread_msg_new = tny_session_camel_ms_thread_msg_new;
	camel_session_class->thread_msg_free = tny_session_camel_ms_thread_msg_free;
	camel_session_class->thread_status = tny_session_camel_ms_thread_status;

	return;
}

CamelType
tny_session_camel_get_type (void)
{
	static CamelType tny_session_camel_type = CAMEL_INVALID_TYPE;

	if (G_UNLIKELY (!camel_type_init_done))
	{
		if (!g_thread_supported ()) 
			g_thread_init (NULL);
		camel_type_init ();
		camel_type_init_done = TRUE;
	}

	if (G_UNLIKELY (tny_session_camel_type == CAMEL_INVALID_TYPE)) 
	{
		ms_parent_class = (CamelSessionClass *)camel_session_get_type();
		tny_session_camel_type = camel_type_register (
			camel_session_get_type (),
			"TnySessionCamel",
			sizeof (TnySessionCamel),
			sizeof (TnySessionCamelClass),
			(CamelObjectClassInitFunc) tny_session_camel_class_init,
			NULL,
			(CamelObjectInitFunc) tny_session_camel_init,
			(CamelObjectFinalizeFunc) tny_session_camel_finalise);
	}

	return tny_session_camel_type;
}

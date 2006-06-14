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

#include <tny-device-iface.h>
#include <tny-account-store-iface.h>
#include <tny-store-account.h>
#include <tny-transport-account.h>

#include "tny-store-account-priv.h"
#include "tny-transport-account-priv.h"

#include <tny-camel-shared.h>

gboolean camel_type_init_done = FALSE;

static CamelSessionClass *ms_parent_class;
static GList *password_funcs = NULL;
static GList *forget_password_funcs = NULL;

typedef struct
{
	CamelService *service;
	TnyGetPassFunc func;
	TnyAccountIface *account;

} PrivPassFunc;

typedef struct
{
	CamelService *service;
	TnyForgetPassFunc func;
	TnyAccountIface *account;

} PrivForgetPassFunc;

static void
tny_session_camel_forget_password (CamelSession *session, CamelService *service, const char *domain, const char *item, CamelException *ex);

/**
 * tny_session_camel_set_forget_pass_func:
 * @self: a #TnySessionCamel object
 * @account: the #TnyAccountIface account instance
 * @get_forget_pass_func: The function that will forget the password for the account
 *
 * For each account you can set a password handler. This makes it possible to
 * use your own password-store implementation for storing passwords. You set
 * the handler that tells that store to forget the password, using this method.
 *
 * It's recommended to also memset (buffer, 0, len) the memory where the
 * password was stored. You can do it in the forget_pass_func handler.
 *
 **/
void
tny_session_camel_set_forget_pass_func (TnySessionCamel *self, TnyAccountIface *account, TnyForgetPassFunc get_forget_pass_func)
{
	GList *copy = forget_password_funcs, *mark_del = NULL;
	PrivForgetPassFunc *pf;
	CamelSession *me = CAMEL_SESSION (self);
	gboolean found = FALSE;
	CamelService *service = (CamelService*)_tny_account_get_service (TNY_ACCOUNT (account));

	while (G_LIKELY (copy))
	{
		pf = copy->data;

		if (G_UNLIKELY (pf->service == NULL) || G_UNLIKELY (pf->account == NULL))
		{
			mark_del = g_list_prepend (mark_del, copy);
			copy = g_list_next (copy);
			continue;
		}

		if (G_UNLIKELY (pf->service == service))
		{
			found = TRUE;
			break;
		}

		copy = g_list_next (copy);
	}

	if (G_UNLIKELY (!found))
		pf = g_new0 (PrivForgetPassFunc, 1);

	pf->account = account;
	pf->func = get_forget_pass_func;
	pf->service = service;

	if (G_UNLIKELY (!found))
		forget_password_funcs = g_list_prepend (forget_password_funcs, pf);

	if (G_UNLIKELY (mark_del)) 
	{
		while (G_LIKELY (mark_del))
		{
			forget_password_funcs = g_list_remove (forget_password_funcs, mark_del->data);
			mark_del = g_list_next (mark_del);
		}

		g_list_free (mark_del);
	}

	return;
}

/**
 * tny_session_camel_set_pass_func:
 * @self: a #TnySessionCamel object
 * @account: the #TnyAccountIface account instance
 * @get_pass_func: The function that will return the password for the account
 *
 * For each account you can set a password handler. This makes it possible to
 * use your own password-store implementation for storing passwords. You set
 * that password handler method using this method.
 *
 **/
void
tny_session_camel_set_pass_func (TnySessionCamel *self, TnyAccountIface *account, TnyGetPassFunc get_pass_func)
{
	GList *copy = password_funcs, *mark_del = NULL;
	PrivPassFunc *pf;
	CamelSession *me = CAMEL_SESSION (self);
	gboolean found = FALSE;
	CamelService *service = (CamelService*)_tny_account_get_service (TNY_ACCOUNT (account));

	while (G_LIKELY (copy))
	{
		pf = copy->data;

		if (G_UNLIKELY (pf->service == NULL) || G_UNLIKELY (pf->account == NULL))
		{
			mark_del = g_list_prepend (mark_del, copy);
			copy = g_list_next (copy);
			continue;
		}

		if (G_UNLIKELY (pf->service == service))
		{
			found = TRUE;
			break;
		}

		copy = g_list_next (copy);
	}

	if (G_UNLIKELY (!found))
		pf = g_new0 (PrivPassFunc, 1);

	pf->account = account;
	pf->func = get_pass_func;
	pf->service = service;

	if (G_UNLIKELY (!found))
		password_funcs = g_list_prepend (password_funcs, pf);

	if (G_UNLIKELY (mark_del))
	{
		while (G_LIKELY (mark_del))
		{
			password_funcs = g_list_remove (password_funcs, mark_del->data);
			mark_del = g_list_next (mark_del);
		}

		g_list_free (mark_del);
	}

	return;
}

static char *
tny_session_camel_get_password (CamelSession *session, CamelService *service, const char *domain,
	      const char *prompt, const char *item, guint32 flags, CamelException *ex)
{
	GList *copy = password_funcs;
	TnyGetPassFunc func;
	TnyAccountIface *account;
	gboolean found = FALSE, freeprmpt = FALSE, cancel = FALSE;
	gchar *retval = NULL, *prmpt = (gchar*)prompt;

	while (G_LIKELY (copy))
	{
		PrivPassFunc *pf = copy->data;

		if (G_UNLIKELY (pf->service == service) && pf->account)
		{
			found = TRUE;
			func = pf->func;
			account = pf->account;
			break;
		}

		copy = g_list_next (copy);
	}

	if (G_LIKELY (found))
	{
		if (prmpt == NULL)
		{
			freeprmpt = TRUE;
			prmpt = g_strdup_printf (_("Enter password for %s"), 
				tny_account_iface_get_name (account));
		}
		
		if (!g_ascii_strncasecmp (tny_account_iface_get_proto (account), "pop", 3))
		{
			if (flags & CAMEL_SESSION_PASSWORD_REPROMPT)
				tny_session_camel_forget_password (session, service, domain, item, ex);
		}

		retval = func (account, prompt, &cancel);

		if (freeprmpt)
			g_free (prmpt);
	}

	if (cancel)
		camel_exception_set (ex, CAMEL_EXCEPTION_USER_CANCEL,
			_("You cancelled when you had to enter a password"));

	return retval;
}

static void
tny_session_camel_forget_password (CamelSession *session, CamelService *service, const char *domain, const char *item, CamelException *ex)
{
	GList *copy = forget_password_funcs;
	TnyForgetPassFunc func;
	TnyAccountIface *account;
	gboolean found = FALSE;

	while (G_LIKELY (copy))
	{
		PrivForgetPassFunc *pf = copy->data;

		if (G_UNLIKELY (pf->service == service) && pf->account)
		{
			found = TRUE;
			func = pf->func;
			account = pf->account;
			break;
		}
		copy = g_list_next (copy);
	}

	if (G_LIKELY (found))
		func (account);

	return;
}

static gboolean
tny_session_camel_alert_user (CamelSession *session, CamelSessionAlertType type, const char *prompt, gboolean cancel)
{
	TnySessionCamel *self = (TnySessionCamel *)session;

	if (self->account_store)
	{
		TnyAccountStoreIface *account_store = (TnyAccountStoreIface*)self->account_store;
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

		return tny_account_store_iface_alert (account_store, tnytype, prompt);
	}
	
	return FALSE;
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
my_cancel_func (struct _CamelOperation *op, const char *what, int pc, void *data)
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


static void
tny_session_camel_init (TnySessionCamel *instance)
{
	/* Avoid the first question in connection_changed */
	instance->prev_constat = FALSE;
	instance->device = NULL;
}

static void
connection_changed (TnyDeviceIface *device, gboolean online, gpointer user_data)
{
	TnySessionCamel *self = user_data;
	
	camel_session_set_online ((CamelSession *) self, online); 

	if (!self->first_switch && self->prev_constat != online && self->account_store)
	{
		GList *copy;

		copy = (GList*) tny_account_store_iface_get_store_accounts (self->account_store);;
	
		while (G_LIKELY (copy))
		{
			TnyStoreAccountIface *account = copy->data;

			_tny_account_set_online_status (account, !online);

			copy = g_list_next (copy);
		}

	}

	if (self->account_store && !self->first_switch)
		g_signal_emit (self->account_store, 
			tny_account_store_iface_signals [TNY_ACCOUNT_STORE_IFACE_ACCOUNTS_RELOADED], 0);

	self->first_switch = FALSE;

	self->prev_constat = online;

	return;
}

/**
 * tny_session_camel_set_device:
 * @self: a #TnySessionCamel object
 * @device: a #TnyDeviceIface instance
 *
 *
 **/
void 
tny_session_camel_set_device (TnySessionCamel *self, TnyDeviceIface *device)
{

	if (self->device && g_signal_handler_is_connected (G_OBJECT (self->device), self->connchanged_signal))
	{
		g_signal_handler_disconnect (G_OBJECT (device), 
			self->connchanged_signal);
	}

	self->device = device;
	self->connchanged_signal = 
		g_signal_connect (G_OBJECT (device), "connection_changed",
			G_CALLBACK (connection_changed), self);	

	return;
}

/**
 * tny_session_camel_set_account_store:
 * @self: a #TnySessionCamel object
 * @account_store: A #TnyAccountStoreIface account store instance
 *
 *
 **/
void 
tny_session_camel_set_account_store (TnySessionCamel *self, TnyAccountStoreIface *account_store)
{
	CamelSession *session = CAMEL_SESSION (self);
	TnyDeviceIface *device = (TnyDeviceIface*)tny_account_store_iface_get_device (account_store);

	self->account_store = (gpointer)account_store;

	gchar *base_directory = g_strdup (tny_account_store_iface_get_cache_dir (account_store));
	gchar *camel_dir = NULL;

	if (G_LIKELY (camel_init (base_directory, TRUE) != 0))
	{
		g_error (_("Critical ERROR: Cannot init %s as camel directory\n"), base_directory);
		exit (1);
	}

	camel_dir = g_build_filename (base_directory, "mail", NULL);
	camel_provider_init();
	camel_session_construct (session, camel_dir);

	/* Avoid the first question in connection_changed */
	self->first_switch = tny_device_iface_is_online (device);

	camel_session_set_online ((CamelSession *) session, 
		self->first_switch); 

	g_free (camel_dir);
	g_free (base_directory);

	tny_session_camel_set_device (self, device);

	return;
}




/**
 * tny_session_camel_new:
 * @account_store: A TnyAccountStoreIface instance
 *
 * Return value: The #TnySessionCamel singleton instance
 **/
TnySessionCamel*
tny_session_camel_new (TnyAccountStoreIface *account_store)
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

	if (self->device && g_signal_handler_is_connected (G_OBJECT (self->device), self->connchanged_signal))
	{
		g_signal_handler_disconnect (G_OBJECT (self->device), 
			self->connchanged_signal);
	}

	/* CamelObject types don't need parent finalization (build-in camel)
	(*((CamelObjectClass*)ms_parent_class)->finalise) (object); */

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

	tny_session_camel_class->set_pass_func_func = tny_session_camel_set_pass_func;
	tny_session_camel_class->set_forget_pass_func_func = tny_session_camel_set_forget_pass_func;
	tny_session_camel_class->set_account_store_func = tny_session_camel_set_account_store;

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

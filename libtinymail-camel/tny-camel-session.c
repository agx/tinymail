/* libtinymail - The Tiny Mail base library
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

#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib/gstdio.h>

#include <camel/camel.h>
#include <camel/camel-filter-driver.h>
#include <camel/camel-i18n.h>
#include <tny-camel-session.h>

static CamelSessionClass *ms_parent_class;
static GList *password_funcs = NULL;
static GList *forget_password_funcs = NULL;

typedef struct
{
	CamelSession *session;
	GetPassFunc func;
	TnyMsgAccountIface *account;

} PrivPassFunc;

typedef struct
{
	CamelSession *session;
	ForgetPassFunc func;
	TnyMsgAccountIface *account;

} PrivForgetPassFunc;


void
tny_camel_session_set_forget_pass_func (TnyCamelSession *self, TnyMsgAccountIface *account, ForgetPassFunc get_forget_pass_func)
{
	GList *copy = forget_password_funcs, *mark_del = NULL;
	PrivForgetPassFunc *pf;
	CamelSession *me = CAMEL_SESSION (self);
	gboolean found = FALSE;

	while (copy)
	{
		pf = copy->data;

		if (pf->session == NULL || pf->account == NULL)
		{
			mark_del = g_list_append (mark_del, copy);
			continue;
		}

		if (pf->session == me)
		{
			found = TRUE;
			break;
		}

		copy = g_list_next (copy);
	}

	if (!found)
		pf = g_new0 (PrivForgetPassFunc, 1);

	pf->account = account;
	pf->func = get_forget_pass_func;
	pf->session = me;

	if (!found)
		forget_password_funcs = g_list_append (forget_password_funcs, pf);

	if (mark_del) 
		while (mark_del)
		{
			forget_password_funcs = g_list_remove (forget_password_funcs, mark_del->data);
			mark_del = g_list_next (mark_del);
		}

	g_list_free (mark_del);

	self->get_forget_pass_func = get_forget_pass_func;
}

void
tny_camel_session_set_pass_func (TnyCamelSession *self, TnyMsgAccountIface *account, GetPassFunc get_pass_func)
{
	GList *copy = password_funcs, *mark_del = NULL;
	PrivPassFunc *pf;
	CamelSession *me = CAMEL_SESSION (self);
	gboolean found = FALSE;

	while (copy)
	{
		pf = copy->data;

		if (pf->session == NULL || pf->account == NULL)
		{
			mark_del = g_list_append (mark_del, copy);
			continue;
		}

		if (pf->session == me)
		{
			found = TRUE;
			break;
		}

		copy = g_list_next (copy);
	}

	if (!found)
		pf = g_new0 (PrivPassFunc, 1);

	pf->account = account;
	pf->func = get_pass_func;
	pf->session = me;

	if (!found)
		password_funcs = g_list_append (password_funcs, pf);


	if (mark_del) 
		while (mark_del)
		{
			password_funcs = g_list_remove (password_funcs, mark_del->data);
			mark_del = g_list_next (mark_del);
		}

	g_list_free (mark_del);

	self->get_pass_func = get_pass_func;
}

GetPassFunc 
tny_camel_session_get_pass_func (TnyCamelSession *self)
{
	return self->get_pass_func;
}

ForgetPassFunc 
tny_camel_session_get_forget_pass_func (TnyCamelSession *self)
{
	return self->get_forget_pass_func;
}

static char *
tny_camel_session_get_password (CamelSession *session, CamelService *service, const char *domain,
	      const char *prompt, const char *item, guint32 flags, CamelException *ex)
{
	GList *copy = password_funcs;
	GetPassFunc func;
	TnyMsgAccountIface *account;
	gboolean found = FALSE;

	while (copy)
	{
		PrivPassFunc *pf = copy->data;

		if (pf->session == session)
		{
			found = TRUE;
			func = pf->func;
			account = pf->account;
			break;
		}

		copy = g_list_next (copy);
	}

	if (found)
		return func (account);

	return NULL;
}

static void
tny_camel_session_forget_password (CamelSession *session, CamelService *service, const char *domain, const char *item, CamelException *ex)
{
	GList *copy = forget_password_funcs;
	ForgetPassFunc func;
	TnyMsgAccountIface *account;
	gboolean found = FALSE;

	while (copy)
	{
		PrivForgetPassFunc *pf = copy->data;

		if (pf->session == session)
		{
			found = TRUE;
			func = pf->func;
			account = pf->account;
			break;
		}

		copy = g_list_next (copy);
	}

	if (found)
		func (account);

	return;
}

static gboolean
tny_camel_session_alert_user (CamelSession *session, CamelSessionAlertType type, const char *prompt, gboolean cancel)
{
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
	if (!url) 
	{
		g_free(curi);
		return NULL;
	}

	store = (CamelStore *)camel_session_get_service(session, uri+offset, CAMEL_PROVIDER_STORE, ex);
	if (store) 
	{
		const char *name;

		if (url->fragment) 
		{
			name = url->fragment;
		} else 
		{
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
tny_camel_session_get_filter_driver (CamelSession *session, const char *type, CamelException *ex)
{
	CamelFilterDriver *driver = camel_filter_driver_new (session);
	camel_filter_driver_set_folder_func (driver, get_folder, session);

	return driver; 
}


static void 
my_receive_func(CamelSession *session, struct _CamelSessionThreadMsg *m)
{
	g_print ("Receive func\n");
}

static void
my_free_func (CamelSession *session, struct _CamelSessionThreadMsg *m)
{
	g_print ("Free func\n");
}


static void 
my_cancel_func (struct _CamelOperation *op, const char *what, int pc, void *data)
{
	g_print ("Cancel func\n");
}

static void *
tny_camel_session_ms_thread_msg_new (CamelSession *session, CamelSessionThreadOps *ops, unsigned int size)
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
tny_camel_session_ms_thread_msg_free (CamelSession *session, CamelSessionThreadMsg *m)
{
	ms_parent_class->thread_msg_free(session, m);
}

static void
tny_camel_session_ms_thread_status (CamelSession *session, CamelSessionThreadMsg *msg, const char *text, int pc)
{
	return;
}


static void
tny_camel_session_init (TnyCamelSession *instance)
{
	CamelSession *session = CAMEL_SESSION (instance);

	gchar *base_directory = g_build_filename (g_get_home_dir (), ".tinymail", NULL);
	gchar *camel_dir = NULL;

	if (camel_init (base_directory, TRUE) != 0)
	{
		g_print ("Critical ERROR: Cannot init %d as camel directory\n", base_directory);
		exit (1);
	}

	camel_dir = g_build_filename (g_get_home_dir (), ".tinymail", "mail", NULL);
	camel_provider_init();
	camel_session_construct (session, camel_dir);
	camel_session_set_online ((CamelSession *) session, TRUE);
	
	g_free (camel_dir);
	g_free (base_directory);

	return;
}


TnyCamelSession*
tny_camel_session_new (void)
{
	TnyCamelSession *retval = TNY_CAMEL_SESSION 
		(camel_object_new (TNY_CAMEL_SESSION_TYPE));
	
	return retval;
}


static void
tny_camel_session_finalise (TnyCamelSession *session)
{
	g_print ("Critical TODO item at %s\n", __FUNCTION__);
}

static void
tny_camel_session_class_init (TnyCamelSessionClass *tny_camel_session_class)
{
	CamelSessionClass *camel_session_class = CAMEL_SESSION_CLASS (tny_camel_session_class);
	
	camel_session_class->get_password = tny_camel_session_get_password;
	camel_session_class->forget_password = tny_camel_session_forget_password;
	camel_session_class->alert_user = tny_camel_session_alert_user;
	camel_session_class->get_filter_driver = tny_camel_session_get_filter_driver;

	camel_session_class->thread_msg_new = tny_camel_session_ms_thread_msg_new;
	camel_session_class->thread_msg_free = tny_camel_session_ms_thread_msg_free;
	camel_session_class->thread_status = tny_camel_session_ms_thread_status;

	tny_camel_session_class->get_pass_func_func = tny_camel_session_get_pass_func;
	tny_camel_session_class->set_pass_func_func = tny_camel_session_set_pass_func;
}

CamelType
tny_camel_session_get_type (void)
{
	static CamelType tny_camel_session_type = CAMEL_INVALID_TYPE;
	
	if (tny_camel_session_type == CAMEL_INVALID_TYPE) {
		ms_parent_class = (CamelSessionClass *)camel_session_get_type();
		tny_camel_session_type = camel_type_register (
			camel_session_get_type (),
			"TnyCamelSession",
			sizeof (TnyCamelSession),
			sizeof (TnyCamelSessionClass),
			(CamelObjectClassInitFunc) tny_camel_session_class_init,
			NULL,
			(CamelObjectInitFunc) tny_camel_session_init,
			(CamelObjectFinalizeFunc) tny_camel_session_finalise);
	}
	
	return tny_camel_session_type;
}

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

#define TNY_CAMEL_SESSION_C
#include <tny-camel-session.h>
#undef TNY_CAMEL_SESSION_C

#include "/home/pvanhoof/personal.h"

CamelSession *session;
static CamelSessionClass *ms_parent_class;

static char *
tny_camel_session_get_password (CamelSession *session, CamelService *service, const char *domain,
	      const char *prompt, const char *item, guint32 flags, CamelException *ex)
{
	return g_strdup(PERSONAL_PASSWORD);
}

static void
tny_camel_session_forget_password (CamelSession *session, CamelService *service, const char *domain, const char *item, CamelException *ex)
{
	return;
}

static gboolean
tny_camel_session_alert_user (CamelSession *session, CamelSessionAlertType type, const char *prompt, gboolean cancel)
{
	return FALSE;
}


CamelFolder *
mail_tool_uri_to_folder (const char *uri, guint32 flags, CamelException *ex)
{
	CamelURL *url;
	CamelStore *store = NULL;
	CamelFolder *folder = NULL;
	int offset = 0;
	char *curi = NULL;

	g_return_val_if_fail (uri != NULL, NULL);

	if (!strncmp (uri, "vtrash:", 7))
		offset = 7;
	else if (!strncmp (uri, "vjunk:", 6))
		offset = 6;
	
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
	return mail_tool_uri_to_folder(uri, 0, ex);
}

static CamelFilterDriver *
tny_camel_session_get_filter_driver (CamelSession *session, const char *type, CamelException *ex)
{
	CamelFilterDriver *driver = camel_filter_driver_new (session);
	camel_filter_driver_set_folder_func (driver, get_folder, NULL);

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
tny_camel_session_init (TnyCamelSession *session)
{
	g_print ("Critical TODO item at %s\n", __FUNCTION__);
}


void
tny_camel_session_prepare (void)
{

	const gchar *base_directory = "/home/pvanhoof/Temp/tinymail";
	gchar *camel_dir;

	if (camel_init (base_directory, TRUE) != 0)
		exit (0);

	camel_provider_init();

	session = CAMEL_SESSION (camel_object_new (TNY_CAMEL_SESSION_TYPE));
	
	camel_dir = g_strdup_printf ("%s/mail", base_directory);
	camel_session_construct (session, camel_dir);

	camel_session_set_online ((CamelSession *) session, TRUE);
	
	g_free (camel_dir);
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

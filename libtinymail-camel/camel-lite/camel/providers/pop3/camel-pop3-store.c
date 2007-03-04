/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* camel-pop3-store.c : class for a pop3 store */

/* 
 * Authors:
 *   Dan Winship <danw@ximian.com>
 *   Michael Zucchi <notzed@ximian.com>
 *
 * Copyright (C) 2000-2002 Ximian, Inc. (www.ximian.com)
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of version 2 of the GNU Lesser General Public 
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <sys/types.h>

#include "camel-file-utils.h"
#include "camel-operation.h"

#include "camel-pop3-store.h"
#include "camel-pop3-folder.h"
#include "camel-stream-buffer.h"
#include "camel-session.h"
#include "camel-exception.h"
#include "camel-url.h"
#include "libedataserver/md5-utils.h"
#include "camel-pop3-engine.h"
#include "camel-sasl.h"
#include "camel-data-cache.h"
#include "camel-tcp-stream.h"
#include "camel-tcp-stream-raw.h"
#ifdef HAVE_SSL
#include "camel-tcp-stream-ssl.h"
#endif
#include "camel-i18n.h"
#include "camel-net-utils.h"
#include "camel-disco-diary.h"

/* Specified in RFC 1939 */
#define POP3_PORT "110"
#define POP3S_PORT "995"

static CamelStoreClass *parent_class = NULL;

static void finalize (CamelObject *object);

static gboolean pop3_connect (CamelService *service, CamelException *ex);
static gboolean pop3_disconnect (CamelService *service, gboolean clean, CamelException *ex);
static GList *query_auth_types (CamelService *service, CamelException *ex);

static CamelFolder *get_folder (CamelStore *store, const char *folder_name, 
				guint32 flags, CamelException *ex);

static CamelFolder *get_trash  (CamelStore *store, CamelException *ex);

static gboolean
pop3_can_work_offline (CamelDiscoStore *disco_store)
{
	return TRUE;
}

static gboolean
pop3_connect_offline (CamelService *service, CamelException *ex)
{
	CamelPOP3Store *store = CAMEL_POP3_STORE (service);
	store->connected = !camel_exception_is_set (ex);
	return store->connected;

}

static gboolean
pop3_connect_online (CamelService *service, CamelException *ex)
{
	return pop3_connect (service, ex);
}


static gboolean
pop3_disconnect_offline (CamelService *service, gboolean clean, CamelException *ex)
{
	return TRUE;
}

static gboolean
pop3_disconnect_online (CamelService *service, gboolean clean, CamelException *ex)
{

	CamelPOP3Store *store = CAMEL_POP3_STORE (service);
	
	if (clean) {
		CamelPOP3Command *pc;
		
		pc = camel_pop3_engine_command_new(store->engine, 0, NULL, NULL, "QUIT\r\n");
		while (camel_pop3_engine_iterate(store->engine, NULL) > 0)
			;
		camel_pop3_engine_command_free(store->engine, pc);
	}

	camel_object_unref((CamelObject *)store->engine);
	store->engine = NULL;

	return TRUE;
}

static CamelFolder *
pop3_get_folder_online (CamelStore *store, const char *folder_name, guint32 flags, CamelException *ex)
{
	return get_folder (store, folder_name, flags, ex);
}



static int
isdir (char *name)
{
	struct stat st;
	if (stat (name, &st))
		return 0;
	return S_ISDIR (st.st_mode);
}

static char *ignored_names[] = { ".", "..", NULL };

int
ignorent (char *name)
{
	char **p;
	for (p = ignored_names; *p; p++)
		if (strcmp (name, *p) == 0)
			return 1;
	return 0;
}


void
my_du (char *name, int *my_size)
{
	DIR *dir;
	struct dirent *ent;

	chdir (name);
	dir = opendir (name);

	if (!dir)
		return;

	while ((ent = readdir (dir)))
	{
		if (!ignorent (ent->d_name))
		{
			char *p = g_strdup_printf ("%s/%s", name, ent->d_name);
			if (isdir (p))
				my_du (p, my_size);
			else 
			{
				struct stat st;
				if (stat (p, &st) == 0)
					*my_size += st.st_size;
			}
			g_free (p);
		}
	}

	closedir (dir);
}

static CamelFolderInfo *
pop3_build_folder_info(CamelPOP3Store *store, const char *folder_name)
{
	CamelURL *url;
	const char *name;
	CamelFolderInfo *fi;
	guint msize;
	gchar *folder_dir = store->storage_path;
	gchar *spath;
	FILE *f;

	fi = camel_folder_info_new ();

	fi->full_name = g_strdup(folder_name);
	fi->unread = -1;
	fi->total = -1;

	my_du (folder_dir, &msize);

	spath = g_strdup_printf ("%s/summary.mmap", folder_dir);
	f = fopen (spath, "r");
	g_free (spath);
	if (f) {
		gint tsize = ((sizeof (guint32) * 5) + sizeof (time_t));
		char *buffer = malloc (tsize), *ptr;
		guint32 version, a;
		a = fread (buffer, 1, tsize, f);
		if (a == tsize) 
		{
			ptr = buffer;
			version = g_ntohl(get_unaligned_u32(ptr));
			ptr += 16;
			fi->total = g_ntohl(get_unaligned_u32(ptr));
			ptr += 4;
			if (version < 0x100 && version >= 13)
				fi->unread = g_ntohl(get_unaligned_u32(ptr));
		}
		g_free (buffer);
		fclose (f);
	} 

	fi->local_size = msize;

	fi->uri = g_strdup ("");
	name = strrchr (fi->full_name, '/');
	if (name == NULL)
		name = fi->full_name;
	else
		name++;
	if (!g_ascii_strcasecmp (fi->full_name, "INBOX"))
		fi->name = g_strdup (_("Inbox"));
	else
		fi->name = g_strdup (name);

	return fi;
}

static CamelFolder *
pop3_get_folder_offline (CamelStore *store, const char *folder_name,
		    guint32 flags, CamelException *ex)
{
	return get_folder (store, folder_name, flags, ex);
}


static CamelFolderInfo *
pop3_get_folder_info_offline (CamelStore *store, const char *top, guint32 flags, CamelException *ex)
{
	return pop3_build_folder_info (CAMEL_POP3_STORE (store), "INBOX");
}

static CamelFolderInfo *
pop3_get_folder_info_online (CamelStore *store, const char *top, guint32 flags, CamelException *ex)
{
	CamelFolderInfo *info =  pop3_get_folder_info_offline (store, top, flags, ex);

	/* TODO: get read and unread count into info->unread & info->total */

	return info;
}


enum {
	MODE_CLEAR,
	MODE_SSL,
	MODE_TLS,
};

#ifdef HAVE_SSL
#define SSL_PORT_FLAGS (CAMEL_TCP_STREAM_SSL_ENABLE_SSL2 | CAMEL_TCP_STREAM_SSL_ENABLE_SSL3)
#define STARTTLS_FLAGS (CAMEL_TCP_STREAM_SSL_ENABLE_TLS)
#endif

static gboolean
connect_to_server (CamelService *service, struct addrinfo *ai, int ssl_mode, CamelException *ex)
{
	CamelPOP3Store *store = CAMEL_POP3_STORE (service);
	CamelStream *tcp_stream;
	CamelPOP3Command *pc;
	guint32 flags = 0;
	int clean_quit = TRUE;
	int ret;
	gchar *delete_days;

	store->connected = FALSE;

	if (ssl_mode != MODE_CLEAR) {
#ifdef HAVE_SSL
		if (ssl_mode == MODE_TLS) {
			tcp_stream = camel_tcp_stream_ssl_new_raw (service->session, service->url->host, STARTTLS_FLAGS);
		} else {
			tcp_stream = camel_tcp_stream_ssl_new (service->session, service->url->host, SSL_PORT_FLAGS);
		}
#else
		camel_exception_setv (ex, CAMEL_EXCEPTION_SERVICE_UNAVAILABLE,
				      _("Could not connect to %s: %s"),
				      service->url->host, _("SSL unavailable"));
		
		return FALSE;
#endif /* HAVE_SSL */
	} else {
		tcp_stream = camel_tcp_stream_raw_new ();
	}
	
	if ((ret = camel_tcp_stream_connect ((CamelTcpStream *) tcp_stream, ai)) == -1) {
		if (errno == EINTR)
			camel_exception_set (ex, CAMEL_EXCEPTION_USER_CANCEL,
					     _("Connection canceled"));
		else
			camel_exception_setv (ex, CAMEL_EXCEPTION_SERVICE_UNAVAILABLE,
					      _("Could not connect to %s: %s"),
					      service->url->host,
					      g_strerror (errno));
		
		camel_object_unref (tcp_stream);
		
		return FALSE;
	}
	
	/* parent class connect initialization */
	/*if (CAMEL_SERVICE_CLASS (parent_class)->connect (service, ex) == FALSE) {
		camel_object_unref (tcp_stream);
		return FALSE;
	}*/
	
	if (camel_url_get_param (service->url, "disable_extensions"))
		flags |= CAMEL_POP3_ENGINE_DISABLE_EXTENSIONS;
	
	if ((delete_days = (gchar *) camel_url_get_param(service->url,"delete_after"))) 
		store->delete_after =  atoi(delete_days);
	
	if (!(store->engine = camel_pop3_engine_new (tcp_stream, flags))) {
		camel_exception_setv (ex, CAMEL_EXCEPTION_SYSTEM,
				      _("Failed to read a valid greeting from POP server %s"),
				      service->url->host);
		camel_object_unref (tcp_stream);
		return FALSE;
	}

	store->engine->store = store;
	store->engine->partial_happening = FALSE;

	if (ssl_mode != MODE_TLS) {
		camel_object_unref (tcp_stream);
		store->connected = TRUE;
		return TRUE;
	}
	
#ifdef HAVE_SSL
	if (!(store->engine->capa & CAMEL_POP3_CAP_STLS)) {
		camel_exception_setv (ex, CAMEL_EXCEPTION_SYSTEM,
				      _("Failed to connect to POP server %s in secure mode: %s"),
				      service->url->host, _("STLS not supported by server"));
		goto stls_exception;
	}
	
	/* as soon as we send a STLS command, all hope is lost of a clean QUIT if problems arise */
	clean_quit = FALSE;
	
	pc = camel_pop3_engine_command_new (store->engine, 0, NULL, NULL, "STLS\r\n");
	while (camel_pop3_engine_iterate (store->engine, NULL) > 0)
		;
	
	ret = pc->state == CAMEL_POP3_COMMAND_OK;
	camel_pop3_engine_command_free (store->engine, pc);
	
	if (ret == FALSE) {
		camel_exception_setv (ex, CAMEL_EXCEPTION_SYSTEM,
				      _("Failed to connect to POP server %s in secure mode: %s"),
				      service->url->host, store->engine->line);
		goto stls_exception;
	}
	
	/* Okay, now toggle SSL/TLS mode */
	ret = camel_tcp_stream_ssl_enable_ssl (CAMEL_TCP_STREAM_SSL (tcp_stream));
	
	if (ret == -1) {
		camel_exception_setv (ex, CAMEL_EXCEPTION_SYSTEM,
				      _("Failed to connect to POP server %s in secure mode: %s"),
				      service->url->host, _("TLS negotiations failed"));
		goto stls_exception;
	}
#else
	camel_exception_setv (ex, CAMEL_EXCEPTION_SYSTEM,
			      _("Failed to connect to POP server %s in secure mode: %s"),
			      service->url->host, _("TLS is not available in this build"));
	goto stls_exception;
#endif /* HAVE_SSL */
	
	camel_object_unref (tcp_stream);
	
	/* rfc2595, section 4 states that after a successful STLS
           command, the client MUST discard prior CAPA responses */
	camel_pop3_engine_reget_capabilities (store->engine);

	store->connected = TRUE;

	return TRUE;
	
 stls_exception:
	if (clean_quit) {
		/* try to disconnect cleanly */
		pc = camel_pop3_engine_command_new (store->engine, 0, NULL, NULL, "QUIT\r\n");
		while (camel_pop3_engine_iterate (store->engine, NULL) > 0)
			;
		camel_pop3_engine_command_free (store->engine, pc);
	}
	
	camel_object_unref (CAMEL_OBJECT (store->engine));
	camel_object_unref (CAMEL_OBJECT (tcp_stream));
	store->engine = NULL;
	
	return FALSE;
}

static struct {
	char *value;
	char *serv;
	char *port;
	int mode;
} ssl_options[] = {
	{ "",              "pop3s", POP3S_PORT, MODE_SSL   },  /* really old (1.x) */
	{ "always",        "pop3s", POP3S_PORT, MODE_SSL   },
	{ "when-possible", "pop3",  POP3_PORT,  MODE_TLS   },
	{ "never",         "pop3",  POP3_PORT,  MODE_CLEAR },
	{ NULL,            "pop3",  POP3_PORT,  MODE_CLEAR },
};

static gboolean
connect_to_server_wrapper (CamelService *service, CamelException *ex)
{
	struct addrinfo hints, *ai;
	const char *ssl_mode;
	int mode, ret, i;
	char *serv;
	const char *port;

	if ((ssl_mode = camel_url_get_param (service->url, "use_ssl"))) {
		for (i = 0; ssl_options[i].value; i++)
			if (!strcmp (ssl_options[i].value, ssl_mode))
				break;
		mode = ssl_options[i].mode;
		serv = ssl_options[i].serv;
		port = ssl_options[i].port;
	} else {
		mode = MODE_CLEAR;
		serv = "pop3";
		port = POP3S_PORT;
	}
	
	if (service->url->port) {
		serv = g_alloca (16);
		sprintf (serv, "%d", service->url->port);
		port = NULL;
	}
	
	memset (&hints, 0, sizeof (hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = PF_UNSPEC;
	ai = camel_getaddrinfo(service->url->host, serv, &hints, ex);
	if (ai == NULL && port != NULL && camel_exception_get_id(ex) != CAMEL_EXCEPTION_USER_CANCEL) {
		camel_exception_clear (ex);
		ai = camel_getaddrinfo(service->url->host, port, &hints, ex);
	}
	
	if (ai == NULL)
		return FALSE;
	
	ret = connect_to_server (service, ai, mode, ex);
	
	camel_freeaddrinfo (ai);
	
	return ret;
}

extern CamelServiceAuthType camel_pop3_password_authtype;
extern CamelServiceAuthType camel_pop3_apop_authtype;

static GList *
query_auth_types (CamelService *service, CamelException *ex)
{
	CamelPOP3Store *store = CAMEL_POP3_STORE (service);
	GList *types = NULL;

        types = CAMEL_SERVICE_CLASS (parent_class)->query_auth_types (service, ex);
	if (camel_exception_is_set (ex))
		return NULL;

	if (connect_to_server_wrapper (service, NULL)) {
		types = g_list_concat(types, g_list_copy(store->engine->auth));
		pop3_disconnect (service, TRUE, NULL);
	} else {
		camel_exception_setv (ex, CAMEL_EXCEPTION_SERVICE_UNAVAILABLE,
				      _("Could not connect to POP server %s"),
				      service->url->host);
	}

	return types;
}


static int
try_sasl(CamelPOP3Store *store, const char *mech, CamelException *ex)
{
	CamelPOP3Stream *stream = store->engine->stream;
	unsigned char *line, *resp;
	CamelSasl *sasl;
	unsigned int len;
	int ret;

	sasl = camel_sasl_new("pop3", mech, (CamelService *)store);
	if (sasl == NULL) {
		camel_exception_setv (ex, CAMEL_EXCEPTION_SERVICE_URL_INVALID,
				      _("Unable to connect to POP server %s: "
					"No support for requested authentication mechanism."),
				      CAMEL_SERVICE (store)->url->host);
		return -1;
	}

	if (camel_stream_printf((CamelStream *)stream, "AUTH %s\r\n", mech) == -1)
		goto ioerror;

	while (1) {
		if (camel_pop3_stream_line(stream, &line, &len) == -1)
			goto ioerror;
		if (strncmp((char *) line, "+OK", 3) == 0)
			break;
		if (strncmp((char *) line, "-ERR", 4) == 0) {
			camel_exception_setv (ex, CAMEL_EXCEPTION_SERVICE_CANT_AUTHENTICATE,
					      _("SASL `%s' Login failed for POP server %s: %s"),
					      mech, CAMEL_SERVICE (store)->url->host, line);
			goto done;
		}
		/* If we dont get continuation, or the sasl object's run out of work, or we dont get a challenge,
		   its a protocol error, so fail, and try reset the server */
		if (strncmp((char *) line, "+ ", 2) != 0
		    || camel_sasl_authenticated(sasl)
		    || (resp = (unsigned char *) camel_sasl_challenge_base64(sasl, (const char *) line+2, ex)) == NULL) {
			camel_stream_printf((CamelStream *)stream, "*\r\n");
			camel_pop3_stream_line(stream, &line, &len);
			camel_exception_setv (ex, CAMEL_EXCEPTION_SERVICE_CANT_AUTHENTICATE,
					      _("Cannot login to POP server %s: SASL Protocol error"),
					      CAMEL_SERVICE (store)->url->host);
			goto done;
		}

		ret = camel_stream_printf((CamelStream *)stream, "%s\r\n", resp);
		g_free(resp);
		if (ret == -1)
			goto ioerror;

	}
	camel_object_unref((CamelObject *)sasl);
	return 0;
	
 ioerror:
	if (errno == EINTR) {
		camel_exception_set (ex, CAMEL_EXCEPTION_USER_CANCEL, _("Canceled"));
	} else {
		camel_exception_setv (ex, CAMEL_EXCEPTION_SYSTEM,
				      _("Failed to authenticate on POP server %s: %s"),
				      CAMEL_SERVICE (store)->url->host, g_strerror (errno));
	}
 done:
	camel_object_unref((CamelObject *)sasl);
	return -1;
}

static int
pop3_try_authenticate (CamelService *service, gboolean reprompt, const char *errmsg, CamelException *ex)
{
	CamelPOP3Store *store = (CamelPOP3Store *)service;
	CamelPOP3Command *pcu = NULL, *pcp = NULL;
	int status;
	
	/* override, testing only */
	/*printf("Forcing authmech to 'login'\n");
	service->url->authmech = g_strdup("LOGIN");*/
	
	if (!service->url->passwd) {
		char *prompt;
		guint32 flags = CAMEL_SESSION_PASSWORD_SECRET;
		
		if (reprompt)
			flags |= CAMEL_SESSION_PASSWORD_REPROMPT;
		
		prompt = g_strdup_printf (_("%sPlease enter the POP password for %s on host %s"),
					  errmsg ? errmsg : "",
					  service->url->user,
					  service->url->host);
		service->url->passwd = camel_session_get_password (camel_service_get_session (service), service, NULL,
								   prompt, "password", flags, ex);
		g_free (prompt);
		if (!service->url->passwd)
			return FALSE;
	}

	if (!service->url->authmech) {
		/* pop engine will take care of pipelining ability */
		pcu = camel_pop3_engine_command_new(store->engine, 0, NULL, NULL, "USER %s\r\n", service->url->user);
		pcp = camel_pop3_engine_command_new(store->engine, 0, NULL, NULL, "PASS %s\r\n", service->url->passwd);
	} else if (strcmp(service->url->authmech, "+APOP") == 0 && store->engine->apop) {
		char *secret, md5asc[33], *d;
		unsigned char md5sum[16], *s;
		
		secret = g_alloca(strlen(store->engine->apop)+strlen(service->url->passwd)+1);
		sprintf(secret, "%s%s",  store->engine->apop, service->url->passwd);
		md5_get_digest(secret, strlen (secret), md5sum);

		for (s = md5sum, d = md5asc; d < md5asc + 32; s++, d += 2)
			sprintf (d, "%.2x", *s);
		
		pcp = camel_pop3_engine_command_new(store->engine, 0, NULL, NULL, "APOP %s %s\r\n",
						    service->url->user, md5asc);
	} else {
		CamelServiceAuthType *auth;
		GList *l;

		l = store->engine->auth;
		while (l) {
			auth = l->data;
			if (strcmp(auth->authproto, service->url->authmech) == 0)
				return try_sasl(store, service->url->authmech, ex) == -1;
			l = l->next;
		}
		
		camel_exception_setv (ex, CAMEL_EXCEPTION_SERVICE_URL_INVALID,
				      _("Unable to connect to POP server %s: "
					"No support for requested authentication mechanism."),
				      CAMEL_SERVICE (store)->url->host);
		return FALSE;
	}
	
	while ((status = camel_pop3_engine_iterate(store->engine, pcp)) > 0)
		;
	
	if (status == -1) {
		if (errno == EINTR) {
			camel_exception_set (ex, CAMEL_EXCEPTION_USER_CANCEL, _("Canceled"));
		} else {
			camel_exception_setv (ex, CAMEL_EXCEPTION_SYSTEM,
					      _("Unable to connect to POP server %s.\n"
						"Error sending password: %s"),
					      CAMEL_SERVICE (store)->url->host,
					      errno ? g_strerror (errno) : _("Unknown error"));
		}
	} else if (pcu && pcu->state != CAMEL_POP3_COMMAND_OK) {
		camel_exception_setv (ex, CAMEL_EXCEPTION_SERVICE_CANT_AUTHENTICATE,
				      _("Unable to connect to POP server %s.\n"
					"Error sending username: %s"),
				      CAMEL_SERVICE (store)->url->host,
				      store->engine->line ? (char *)store->engine->line : _("Unknown error"));
	} else if (pcp->state != CAMEL_POP3_COMMAND_OK)
		camel_exception_setv (ex, CAMEL_EXCEPTION_SERVICE_CANT_AUTHENTICATE,
				      _("Unable to connect to POP server %s.\n"
					"Error sending password: %s"),
				      CAMEL_SERVICE (store)->url->host,
				      store->engine->line ? (char *)store->engine->line : _("Unknown error"));
	
	camel_pop3_engine_command_free(store->engine, pcp);
	
	if (pcu)
		camel_pop3_engine_command_free(store->engine, pcu);
	
	return status;
}

static gboolean
pop3_connect (CamelService *service, CamelException *ex)
{
	CamelPOP3Store *store = (CamelPOP3Store *)service;
	gboolean reprompt = FALSE;
	CamelSession *session;
	char *errbuf = NULL;
	int status;
	
	session = camel_service_get_session (service);

	if (!connect_to_server_wrapper (service, ex))
		return FALSE;
	
	while (1) {
		status = pop3_try_authenticate (service, reprompt, errbuf, ex);
		g_free (errbuf);
		errbuf = NULL;
		
		/* we only re-prompt if we failed to authenticate, any other error and we just abort */
		if (status == 0 && camel_exception_get_id (ex) == CAMEL_EXCEPTION_SERVICE_CANT_AUTHENTICATE) {
			errbuf = g_strdup_printf ("%s\n\n", camel_exception_get_description (ex));
			g_free (service->url->passwd);
			service->url->passwd = NULL;
			reprompt = TRUE;
			camel_exception_clear (ex);
			sleep (5); /* For example Cyrus-POPd dislikes hammering */
		} else
			break;
	}
	
	g_free (errbuf);
	
	if (status == -1 || camel_exception_is_set(ex)) {
		camel_service_disconnect(service, TRUE, ex);
		return FALSE;
	}
	
	/* Now that we are in the TRANSACTION state, try regetting the capabilities */
	store->engine->state = CAMEL_POP3_ENGINE_TRANSACTION;
	camel_pop3_engine_reget_capabilities (store->engine);
	
	return TRUE;
}

static gboolean
pop3_disconnect (CamelService *service, gboolean clean, CamelException *ex)
{
	CamelPOP3Store *store = CAMEL_POP3_STORE (service);
	
	if (clean) {
		CamelPOP3Command *pc;
		
		pc = camel_pop3_engine_command_new(store->engine, 0, NULL, NULL, "QUIT\r\n");
		while (camel_pop3_engine_iterate(store->engine, NULL) > 0)
			;
		camel_pop3_engine_command_free(store->engine, pc);
	}
	
	camel_object_unref((CamelObject *)store->engine);
	store->engine = NULL;

	return TRUE;
}

static CamelFolder *
get_folder (CamelStore *store, const char *folder_name, guint32 flags, CamelException *ex)
{
	if (g_ascii_strcasecmp (folder_name, "inbox") != 0) {
		camel_exception_setv (ex, CAMEL_EXCEPTION_FOLDER_INVALID,
				      _("No such folder `%s'."), folder_name);
		return NULL;
	}
	return camel_pop3_folder_new (store, ex);
}

static CamelFolder *
get_trash (CamelStore *store, CamelException *ex)
{
	/* no-op */
	return NULL;
}


static void
finalize (CamelObject *object)
{
	CamelPOP3Store *pop3_store = CAMEL_POP3_STORE (object);

	if (pop3_store->engine)
		camel_object_unref((CamelObject *)pop3_store->engine);
	pop3_store->engine = NULL;
	if (pop3_store->cache)
		camel_object_unref((CamelObject *)pop3_store->cache);
	pop3_store->cache = NULL;
	if (pop3_store->storage_path)
		g_free (pop3_store->storage_path);
	pop3_store->storage_path = NULL;
}



static void
pop3_construct (CamelService *service, CamelSession *session,
	   CamelProvider *provider, CamelURL *url,
	   CamelException *ex)
{
	CamelPOP3Store *pop3_store = CAMEL_POP3_STORE (service);
	CamelStore *store = CAMEL_STORE (service);
	CamelDiscoStore *disco_store = CAMEL_DISCO_STORE (service);
	char *tmp, *path;
	CamelURL *summary_url;

	CAMEL_SERVICE_CLASS (parent_class)->construct (service, session, provider, url, ex);
	if (camel_exception_is_set (ex))
		return;

	pop3_store->storage_path = camel_session_get_storage_path (session, service, ex);
	if (!pop3_store->storage_path)
		return;

	pop3_store->cache = camel_data_cache_new(pop3_store->storage_path, 0, ex);
	if (pop3_store->cache) {
		/* Default cache expiry - 1 week or not visited in a day */
		camel_data_cache_set_expire_age (pop3_store->cache, 60*60*24*7);
		camel_data_cache_set_expire_access (pop3_store->cache, 60*60*24);
	}

	pop3_store->base_url = camel_url_to_string (service->url, (CAMEL_URL_HIDE_PASSWORD |
								   CAMEL_URL_HIDE_PARAMS |
								   CAMEL_URL_HIDE_AUTH));

	/* setup journal*/
	path = g_strdup_printf ("%s/journal", pop3_store->storage_path);
	disco_store->diary = camel_disco_diary_new (disco_store, path, ex);
	g_free (path);

}

static void
camel_pop3_store_class_init (CamelPOP3StoreClass *camel_pop3_store_class)
{
	CamelServiceClass *camel_service_class =
		CAMEL_SERVICE_CLASS (camel_pop3_store_class);
	CamelStoreClass *camel_store_class =
		CAMEL_STORE_CLASS (camel_pop3_store_class);
	CamelDiscoStoreClass *camel_disco_store_class =
		CAMEL_DISCO_STORE_CLASS (camel_pop3_store_class);

	parent_class = CAMEL_STORE_CLASS (camel_type_get_global_classfuncs (camel_disco_store_get_type ()));

	/* virtual method overload */
	camel_service_class->construct = pop3_construct;
	camel_service_class->query_auth_types = query_auth_types;

	/* camel_service_class->connect = pop3_connect; */
	/* camel_service_class->disconnect = pop3_disconnect; */

	camel_store_class->get_folder = get_folder;
	camel_store_class->get_trash = get_trash;

	camel_disco_store_class->can_work_offline = pop3_can_work_offline;
	camel_disco_store_class->connect_online = pop3_connect_online;
	camel_disco_store_class->connect_offline = pop3_connect_offline;
	camel_disco_store_class->disconnect_online = pop3_disconnect_online;
	camel_disco_store_class->disconnect_offline = pop3_disconnect_offline;
	camel_disco_store_class->get_folder_online = pop3_get_folder_online;
	camel_disco_store_class->get_folder_offline = pop3_get_folder_offline;
	camel_disco_store_class->get_folder_resyncing = pop3_get_folder_online;
	camel_disco_store_class->get_folder_info_online = pop3_get_folder_info_online;
	camel_disco_store_class->get_folder_info_offline = pop3_get_folder_info_offline;
	camel_disco_store_class->get_folder_info_resyncing = pop3_get_folder_info_online;
}



static void
camel_pop3_store_init (gpointer object, gpointer klass)
{

	return;
}

CamelType
camel_pop3_store_get_type (void)
{
	static CamelType camel_pop3_store_type = CAMEL_INVALID_TYPE;

	if (!camel_pop3_store_type) {
		camel_pop3_store_type = camel_type_register (CAMEL_DISCO_STORE_TYPE,
							     "CamelPOP3Store",
							     sizeof (CamelPOP3Store),
							     sizeof (CamelPOP3StoreClass),
							     (CamelObjectClassInitFunc) camel_pop3_store_class_init,
							     NULL,
							     (CamelObjectInitFunc) camel_pop3_store_init,
							     finalize);
	}

	return camel_pop3_store_type;
}

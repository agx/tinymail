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
#include <glib.h>

#include <string.h>

#include <tny-account-iface.h>
#include <tny-account.h>
#include <tny-msg-folder-iface.h>
#include <tny-msg-folder.h>

#include <camel/camel.h>
#include <camel/camel-session.h>
#include <camel/camel-store.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <tny-session-camel.h>

static GObjectClass *parent_class = NULL;

#include "tny-account-priv.h"

#define TNY_ACCOUNT_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_ACCOUNT_TYPE, TnyAccountPriv))


static void 
fill_folders_recursive (TnyAccountIface *self, TnyMsgFolderIface *parent, CamelFolderInfo *iter)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);

	while (iter)
	{
		TnyMsgFolderIface *iface = TNY_MSG_FOLDER_IFACE (
			tny_msg_folder_new ());

		tny_msg_folder_iface_set_id (iface, iter->full_name);
		tny_msg_folder_iface_set_account (iface, self);

		if (parent)
			tny_msg_folder_iface_add_folder (parent, iface);
		else 
		{
			/* Uncertain (the _new is already a ref, right?) */
			/* g_object_ref (G_OBJECT (iface)) */

			priv->folders = g_list_append (priv->folders, iface);
		}

		tny_msg_folder_iface_uncache (iface);

		fill_folders_recursive (self, iface, iter->child);

		

		/* Tell the observers that they should reload */
		g_signal_emit (iface, tny_msg_folder_iface_signals [FOLDERS_RELOADED], 0);

		iter = iter->next;
	}
}

const GList*
tny_account_get_folders (TnyAccountIface *self)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);

	CamelStore *store = camel_session_get_store (CAMEL_SESSION (priv->session), 
			priv->url_string, priv->ex);
	CamelFolderInfo *info = camel_store_get_folder_info 
		(store, "", CAMEL_STORE_FOLDER_INFO_SUBSCRIBED |
			CAMEL_STORE_FOLDER_INFO_RECURSIVE, priv->ex);

	fill_folders_recursive (self, NULL, info);
	
	camel_store_free_folder_info (store, info);

	return priv->folders;
}

static void 
reconnect (TnyAccountPriv *priv)
{
	if (priv->proto && priv->user && priv->host)
	{
		CamelURL *url = NULL;
		gchar *proto = g_strdup_printf ("%s://", priv->proto); 

		url = camel_url_new (proto, priv->ex);
		g_free (proto);
	
		camel_url_set_protocol (url, priv->proto); 

		camel_url_set_user (url, priv->user);
		camel_url_set_host (url, priv->host);
	
		if (priv->url_string)
			g_free (priv->url_string);

		priv->url_string = camel_url_to_string (url, 0);

		if (priv->service)
			camel_object_unref (CAMEL_OBJECT (priv->service));

		priv->service = camel_session_get_service 
			(CAMEL_SESSION (priv->session), priv->url_string, 
			CAMEL_PROVIDER_STORE, priv->ex);

		if (priv->service == NULL) {
			g_error ("couldn't get service %s: %s\n", priv->url_string,
				   camel_exception_get_description (priv->ex));
			camel_exception_clear (priv->ex);
			return;
		}

		camel_url_free (url);
	}

	if (priv->service && priv->pass_func_set && priv->proto && priv->user && priv->host)
		camel_service_connect (priv->service, priv->ex);
		

	return;
}


void
tny_account_set_id (TnyAccountIface *self, const gchar *id)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);
	
	if (priv->id)
		g_free (priv->id);

	priv->id = g_strdup (id);

	return;
}

void
tny_account_set_proto (TnyAccountIface *self, const gchar *proto)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);
	
	if (priv->proto)
		g_free (priv->proto);

	priv->proto = g_strdup (proto);

	reconnect (priv);

	return;
}

void
tny_account_set_user (TnyAccountIface *self, const gchar *user)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);
	
	if (priv->user)
		g_free (priv->user);

	priv->user = g_strdup (user);

	reconnect (priv);

	return;
}

void
tny_account_set_hostname (TnyAccountIface *self, const gchar *host)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);
	
	if (priv->host)
		g_free (priv->host);

	priv->host = g_strdup (host);

	reconnect (priv);

	return;
}

void
tny_account_set_pass_func (TnyAccountIface *self, GetPassFunc get_pass_func)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);

	tny_session_camel_set_pass_func (priv->session, self, get_pass_func);
	priv->get_pass_func = get_pass_func;
	priv->pass_func_set = TRUE;

	reconnect (priv);

	return;
}

void
tny_account_set_forget_pass_func (TnyAccountIface *self, ForgetPassFunc get_forget_pass_func)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);

	tny_session_camel_set_forget_pass_func (priv->session, self, get_forget_pass_func);
	priv->forget_pass_func = get_forget_pass_func;
	priv->forget_pass_func_set = TRUE;

	return;
}

const gchar*
tny_account_get_id (TnyAccountIface *self)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);
	
	return (const gchar*)priv->id;
}

const gchar*
tny_account_get_proto (TnyAccountIface *self)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);
	
	return (const gchar*)priv->proto;
}

const gchar*
tny_account_get_user (TnyAccountIface *self)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);
	
	return (const gchar*)priv->user;
}

const gchar*
tny_account_get_hostname (TnyAccountIface *self)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);
	
	return (const gchar*)priv->host;
}

GetPassFunc
tny_account_get_pass_func (TnyAccountIface *self)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);

	return priv->get_pass_func;
}

ForgetPassFunc
tny_account_get_forget_pass_func (TnyAccountIface *self)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);

	return priv->forget_pass_func;
}

const CamelService*
_tny_account_get_service (TnyAccount *self)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);

	return (const CamelService *)priv->service;
}

const gchar*
_tny_account_get_url_string (TnyAccount *self)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);

	return (const gchar*)priv->url_string;
}


/**
 * tny_account_new:
 * 
 *
 * Return value: A new #TnyAccountIface instance implemented for Camel
 **/
TnyAccount*
tny_account_new (void)
{
	TnyAccount *self = g_object_new (TNY_ACCOUNT_TYPE, NULL);

	return self;
}

static void
tny_account_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyAccount *self = (TnyAccount *)instance;
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);

	priv->ex = camel_exception_new ();
	camel_exception_init (priv->ex);

	priv->id = NULL;
	priv->user = NULL;
	priv->host = NULL;
	priv->proto = NULL;
	priv->forget_pass_func_set = FALSE;
	priv->pass_func_set = FALSE;

	priv->session = tny_session_camel_get_instance ();

	return;
}

static void
destroy_folder (gpointer data, gpointer user_data)
{
	g_object_unref (G_OBJECT (data));
}


static void
tny_account_finalize (GObject *object)
{
	TnyAccount *self = (TnyAccount *)object;	
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);

	if (priv->folders)
	{
		g_list_foreach (priv->folders, destroy_folder, NULL);

		/* Tell the observers */
		/* g_signal_emit (TNY_ACCOUNT_IFACE (self), tny_msg_folder_iface_signals [FOLDERS_RELOADED], 0); */
	}

	priv->folders = NULL;

	if (priv->id)
		g_free (priv->id);

	if (priv->user)
		g_free (priv->user);

	if (priv->host)
		g_free (priv->host);

	if (priv->proto)
		g_free (priv->proto);


	camel_exception_free (priv->ex);

	(*parent_class->finalize) (object);

	return;
}

static void
tny_account_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyAccountIfaceClass *klass = (TnyAccountIfaceClass *)g_iface;

	klass->get_folders_func = tny_account_get_folders;

	klass->get_hostname_func = tny_account_get_hostname;
	klass->set_hostname_func = tny_account_set_hostname;

	klass->get_proto_func = tny_account_get_proto;
	klass->set_proto_func = tny_account_set_proto;

	klass->get_user_func = tny_account_get_user;
	klass->set_user_func = tny_account_set_user;

	klass->get_pass_func_func = tny_account_get_pass_func;
	klass->set_pass_func_func = tny_account_set_pass_func;

	klass->get_forget_pass_func_func = tny_account_get_forget_pass_func;
	klass->set_forget_pass_func_func = tny_account_set_forget_pass_func;

	klass->set_id_func = tny_account_set_id;
	klass->get_id_func = tny_account_get_id;

	return;
}

static void 
tny_account_class_init (TnyAccountClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_account_finalize;

	g_type_class_add_private (object_class, sizeof (TnyAccountPriv));

	return;
}

GType 
tny_account_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyAccountClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_account_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyAccount),
		  0,      /* n_preallocs */
		  tny_account_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_account_iface_info = 
		{
		  (GInterfaceInitFunc) tny_account_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyAccount",
			&info, 0);

		g_type_add_interface_static (type, TNY_ACCOUNT_IFACE_TYPE, 
			&tny_account_iface_info);
	}

	return type;
}


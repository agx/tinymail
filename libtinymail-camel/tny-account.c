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

#include <tny-account.h>

#include <tny-session-camel.h>
#include <tny-account-store-iface.h>
#include <tny-msg-folder-iface.h>
#include <tny-msg-folder.h>

#include <camel/camel.h>
#include <camel/camel-session.h>
#include <camel/camel-store.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

static GObjectClass *parent_class = NULL;

#include "tny-account-priv.h"

#include <tny-camel-shared.h>


static void 
tny_account_set_account_store (TnyAccountIface *self, const TnyAccountStoreIface *store)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);

	/* No need to reference (would also be cross reference): If store dies,
	   self should also die. */
	
	priv->store = store;

	g_static_rec_mutex_lock (priv->service_lock);

	priv->session = tny_session_camel_get_instance 
		(TNY_ACCOUNT_STORE_IFACE (priv->store));

	TNY_ACCOUNT_GET_CLASS (self)->reconnect_func (TNY_ACCOUNT (self));

	g_static_rec_mutex_unlock (priv->service_lock);

	return;
}

static const TnyAccountStoreIface*
tny_account_get_account_store (TnyAccountIface *self)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);

	return priv->store;
}


void
tny_account_set_id (TnyAccountIface *self, const gchar *id)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);

	g_static_rec_mutex_lock (priv->service_lock);

	if (G_UNLIKELY (priv->id))
		g_free (priv->id);

	priv->id = g_strdup (id);

	g_static_rec_mutex_unlock (priv->service_lock);

	return;
}

void
tny_account_set_proto (TnyAccountIface *self, const gchar *proto)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);
	
	g_static_rec_mutex_lock (priv->service_lock);

	if (G_UNLIKELY (priv->proto))
		g_free (priv->proto);

	priv->proto = g_strdup (proto);

	TNY_ACCOUNT_GET_CLASS (self)->reconnect_func (TNY_ACCOUNT (self));
	
	g_static_rec_mutex_unlock (priv->service_lock);

	return;
}

void
tny_account_set_user (TnyAccountIface *self, const gchar *user)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);
	
	g_static_rec_mutex_lock (priv->service_lock);

	if (G_UNLIKELY (priv->user))
		g_free (priv->user);

	priv->user = g_strdup (user);

	TNY_ACCOUNT_GET_CLASS (self)->reconnect_func (TNY_ACCOUNT (self));

	g_static_rec_mutex_unlock (priv->service_lock);

	return;
}

void
tny_account_set_hostname (TnyAccountIface *self, const gchar *host)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);
	
	g_static_rec_mutex_lock (priv->service_lock);

	if (G_UNLIKELY (priv->host))
		g_free (priv->host);

	priv->host = g_strdup (host);

	TNY_ACCOUNT_GET_CLASS (self)->reconnect_func (TNY_ACCOUNT (self));

	g_static_rec_mutex_unlock (priv->service_lock);

	return;
}

void
tny_account_set_pass_func (TnyAccountIface *self, TnyGetPassFunc get_pass_func)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);

	g_static_rec_mutex_lock (priv->service_lock);

	tny_session_camel_set_pass_func (priv->session, self, get_pass_func);
	priv->get_pass_func = get_pass_func;
	priv->pass_func_set = TRUE;

	if (G_UNLIKELY (!TNY_ACCOUNT_GET_CLASS (self)->reconnect_func))
		g_error ("This TnyAccountIface instance isn't a fully implemented type\n");

	TNY_ACCOUNT_GET_CLASS (self)->reconnect_func (TNY_ACCOUNT (self));

	g_static_rec_mutex_unlock (priv->service_lock);

	return;
}

void
tny_account_set_forget_pass_func (TnyAccountIface *self, TnyForgetPassFunc get_forget_pass_func)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);

	g_static_rec_mutex_lock (priv->service_lock);

	tny_session_camel_set_forget_pass_func (priv->session, self, get_forget_pass_func);
	priv->forget_pass_func = get_forget_pass_func;
	priv->forget_pass_func_set = TRUE;

	g_static_rec_mutex_unlock (priv->service_lock);

	return;
}

const gchar*
tny_account_get_id (TnyAccountIface *self)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);	
	const gchar *retval;

	g_static_rec_mutex_lock (priv->service_lock);
	retval = (const gchar*)priv->id;
	g_static_rec_mutex_unlock (priv->service_lock);

	return retval;
}

const gchar*
tny_account_get_proto (TnyAccountIface *self)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);
	const gchar *retval;
	
	g_static_rec_mutex_lock (priv->service_lock);
	retval = (const gchar*)priv->proto;
	g_static_rec_mutex_unlock (priv->service_lock);

	return retval;
}

const gchar*
tny_account_get_user (TnyAccountIface *self)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);
	const gchar *retval;

	g_static_rec_mutex_lock (priv->service_lock);
	retval = (const gchar*)priv->user;
	g_static_rec_mutex_unlock (priv->service_lock);

	return retval;
}

const gchar*
tny_account_get_hostname (TnyAccountIface *self)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);	
	const gchar *retval;

	g_static_rec_mutex_lock (priv->service_lock);
	retval = (const gchar*)priv->host;
	g_static_rec_mutex_unlock (priv->service_lock);

	return retval;
}

TnyGetPassFunc
tny_account_get_pass_func (TnyAccountIface *self)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);
	TnyGetPassFunc retval;

	g_static_rec_mutex_lock (priv->service_lock);
	retval = priv->get_pass_func;
	g_static_rec_mutex_unlock (priv->service_lock);

	return retval;
}

TnyForgetPassFunc
tny_account_get_forget_pass_func (TnyAccountIface *self)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);
	TnyForgetPassFunc retval;

	g_static_rec_mutex_lock (priv->service_lock);
	retval = priv->forget_pass_func;
	g_static_rec_mutex_unlock (priv->service_lock);

	return retval;
}

const CamelService* /* protected */
_tny_account_get_service (TnyAccount *self)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);
	const CamelService *retval;

	g_static_rec_mutex_lock (priv->service_lock);
	retval = (const CamelService *)priv->service;
	g_static_rec_mutex_unlock (priv->service_lock);

	return retval;
}

const gchar* /* protected */
_tny_account_get_url_string (TnyAccount *self)
{
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);
	const gchar *retval;

	g_static_rec_mutex_lock (priv->service_lock);
	retval = (const gchar*)priv->url_string;
	g_static_rec_mutex_unlock (priv->service_lock);

	return retval;
}


static void
tny_account_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyAccount *self = (TnyAccount *)instance;
	TnyAccountPriv *priv = TNY_ACCOUNT_GET_PRIVATE (self);

	priv->ex = camel_exception_new ();
	camel_exception_init (priv->ex);

	priv->store = NULL;
	priv->id = NULL;
	priv->user = NULL;
	priv->host = NULL;
	priv->proto = NULL;
	priv->forget_pass_func_set = FALSE;
	priv->pass_func_set = FALSE;

	priv->service_lock = g_new (GStaticRecMutex, 1);
	g_static_rec_mutex_init (priv->service_lock);

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

	g_static_rec_mutex_lock (priv->service_lock);
	if (G_LIKELY (priv->id))
		g_free (priv->id);

	if (G_LIKELY (priv->user))
		g_free (priv->user);

	if (G_LIKELY (priv->host))
		g_free (priv->host);

	if (G_LIKELY (priv->proto))
		g_free (priv->proto);

	g_static_rec_mutex_unlock (priv->service_lock);

	camel_exception_free (priv->ex);

	g_static_rec_mutex_free (priv->service_lock);

	(*parent_class->finalize) (object);

	return;
}


static void
tny_account_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyAccountIfaceClass *klass = (TnyAccountIfaceClass *)g_iface;

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
	klass->set_account_store_func = tny_account_set_account_store;
	klass->get_account_store_func = tny_account_get_account_store;

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

	if (G_UNLIKELY (!camel_type_init_done))
	{
		camel_type_init ();
		camel_type_init_done = TRUE;
	}

	if (G_UNLIKELY(type == 0))
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

		g_type_add_interface_static (type, TNY_TYPE_ACCOUNT_IFACE, 
			&tny_account_iface_info);

	}

	return type;
}

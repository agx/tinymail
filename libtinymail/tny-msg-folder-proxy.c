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

#include <tny-msg-folder-proxy.h>
#include <tny-msg-folder.h>
#include <tny-msg-folder-iface.h>


static const GList*
tny_msg_folder_proxy_get_headers (TnyMsgFolderIface *self)
{
	TnyMsgFolder *real = TNY_MSG_FOLDER_PROXY (self)->real;

	if (real == NULL)
	{
		real = tny_msg_folder_new ();
		TNY_MSG_FOLDER_PROXY (self)->real = real;
	}

	return tny_msg_folder_iface_get_headers (TNY_MSG_FOLDER_IFACE(real));
}

static const GList*
tny_msg_folder_proxy_get_messages (TnyMsgFolderIface *self)
{
	TnyMsgFolder *real = TNY_MSG_FOLDER_PROXY (self)->real;

	if (real == NULL)
	{
		real = tny_msg_folder_new ();
		TNY_MSG_FOLDER_PROXY (self)->real = real;
	}

	return tny_msg_folder_iface_get_messages (TNY_MSG_FOLDER_IFACE(real));
}

static const gchar*
tny_msg_folder_proxy_get_name (TnyMsgFolderIface *self)
{
	TnyMsgFolder *real = TNY_MSG_FOLDER_PROXY (self)->real;

	if (real == NULL)
	{
		real = tny_msg_folder_new ();
		TNY_MSG_FOLDER_PROXY (self)->real = real;
	}

	return tny_msg_folder_iface_get_name (TNY_MSG_FOLDER_IFACE(real));
}


static void
tny_msg_folder_proxy_set_name (TnyMsgFolderIface *self, const gchar *name)
{

	TnyMsgFolder *real = TNY_MSG_FOLDER_PROXY (self)->real;

	if (real == NULL)
	{
		real = tny_msg_folder_new ();
		TNY_MSG_FOLDER_PROXY (self)->real = real;
	}

	tny_msg_folder_iface_set_name (TNY_MSG_FOLDER_IFACE (real), name);

	return;
}

static const gint
tny_msg_folder_proxy_get_id (TnyMsgFolderIface *self)
{
	TnyMsgFolder *real = TNY_MSG_FOLDER_PROXY (self)->real;

	if (real == NULL)
	{
		real = tny_msg_folder_new ();
		TNY_MSG_FOLDER_PROXY (self)->real = real;
	}

	return tny_msg_folder_iface_get_id (TNY_MSG_FOLDER_IFACE(real));
}

static void
tny_msg_folder_proxy_set_id (TnyMsgFolderIface *self, gint id)
{
	TnyMsgFolder *real = TNY_MSG_FOLDER_PROXY (self)->real;

	if (real == NULL)
	{
		real = tny_msg_folder_new ();
		TNY_MSG_FOLDER_PROXY (self)->real = real;
	}

	tny_msg_folder_iface_set_id (TNY_MSG_FOLDER_IFACE (real), id);

	return;
}

static const gboolean 
tny_msg_folder_proxy_has_cache (TnyMsgFolderIface *self)
{
	return (TNY_MSG_FOLDER_PROXY (self)->real != NULL);
}

static void
tny_msg_folder_proxy_uncache (TnyMsgFolderIface *self)
{

	if (tny_msg_folder_proxy_has_cache (self))
	{
		TnyMsgFolder *real = TNY_MSG_FOLDER_PROXY (self)->real;
		tny_msg_folder_iface_destroy (TNY_MSG_FOLDER_IFACE (real));
		TNY_MSG_FOLDER_PROXY (self)->real = NULL;
	}
}

static void
tny_msg_folder_proxy_destroy (TnyMsgFolderIface *self)
{

	if (TNY_MSG_FOLDER_PROXY (self))
	{
		tny_msg_folder_proxy_uncache (self);

		g_object_unref (G_OBJECT (self));
	}

	self = NULL;

	return;
}

TnyMsgFolderProxy*
tny_msg_folder_proxy_new (void)
{
	TnyMsgFolderProxy *self = g_object_new (TNY_MSG_FOLDER_PROXY_TYPE, NULL);
	
	return self;
}

static void
tny_msg_folder_proxy_interface_init (gpointer g_iface, gpointer iface_data)
{
	TnyMsgFolderIfaceClass *klass = (TnyMsgFolderIfaceClass *)g_iface;

	klass->get_headers_func = tny_msg_folder_proxy_get_headers;
	klass->get_messages_func = tny_msg_folder_proxy_get_messages;

	klass->set_id_func = tny_msg_folder_proxy_set_id;	
	klass->get_id_func = tny_msg_folder_proxy_get_id;

	klass->set_name_func = tny_msg_folder_proxy_set_name;
	klass->get_name_func = tny_msg_folder_proxy_get_name;
	
	klass->has_cache_func = tny_msg_folder_proxy_has_cache;
	klass->uncache_func = tny_msg_folder_proxy_uncache;

	klass->destroy_func = tny_msg_folder_proxy_destroy;
}

static void
tny_msg_folder_proxy_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyMsgFolderProxy *self = (TnyMsgFolderProxy *)instance;

	self->real = NULL;

	return;
}

GType 
tny_msg_folder_proxy_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgFolderProxyClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMsgFolderProxy),
		  0,      /* n_preallocs */
		  tny_msg_folder_proxy_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_msg_folder_proxy_info = 
		{
		  (GInterfaceInitFunc) tny_msg_folder_proxy_interface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyMsgFolderProxy",
			&info, 0);

		g_type_add_interface_static (type, TNY_MSG_FOLDER_IFACE_TYPE, 
			&tny_msg_folder_proxy_info);
	}

	return type;
}

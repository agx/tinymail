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

#include <tny-msg-header-proxy.h>
#include <tny-msg-header.h>
#include <tny-msg-header-iface.h>

static GObjectClass *parent_class = NULL;

static TnyMsgHeader*
tny_msg_header_proxy_get_real (TnyMsgHeaderProxy *self)
{
	/* TODO: Implement getting the real TnyMsgHeader instance */

	return tny_msg_header_new ();
}

static const gchar*
tny_msg_header_proxy_get_from (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *real = TNY_MSG_HEADER_PROXY (self)->real;

	if (real == NULL)
	{
		real = tny_msg_header_proxy_get_real (TNY_MSG_HEADER_PROXY (self));
		TNY_MSG_HEADER_PROXY (self)->real = real;
	}

	return tny_msg_header_iface_get_from (TNY_MSG_HEADER_IFACE(real));
}

static const gchar*
tny_msg_header_proxy_get_subject (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *real = TNY_MSG_HEADER_PROXY (self)->real;

	if (real == NULL)
	{
		real = tny_msg_header_proxy_get_real (TNY_MSG_HEADER_PROXY (self));
		TNY_MSG_HEADER_PROXY (self)->real = real;
	}

	return tny_msg_header_iface_get_subject (TNY_MSG_HEADER_IFACE(real));
}

static const gchar*
tny_msg_header_proxy_get_to (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *real = TNY_MSG_HEADER_PROXY (self)->real;

	if (real == NULL)
	{
		real = tny_msg_header_proxy_get_real (TNY_MSG_HEADER_PROXY (self));
		TNY_MSG_HEADER_PROXY (self)->real = real;
	}

	return tny_msg_header_iface_get_to (TNY_MSG_HEADER_IFACE(real));
}

static const gchar*
tny_msg_header_proxy_get_id (TnyMsgHeaderIface *self)
{
	TnyMsgHeader *real = TNY_MSG_HEADER_PROXY (self)->real;

	if (real == NULL)
	{
		real = tny_msg_header_proxy_get_real (TNY_MSG_HEADER_PROXY (self));
		TNY_MSG_HEADER_PROXY (self)->real = real;
	}

	return tny_msg_header_iface_get_id (TNY_MSG_HEADER_IFACE(real));
}


static void
tny_msg_header_proxy_set_from (TnyMsgHeaderIface *self, const gchar *from)
{

	TnyMsgHeader *real = TNY_MSG_HEADER_PROXY (self)->real;

	if (real == NULL)
	{
		real = tny_msg_header_proxy_get_real (TNY_MSG_HEADER_PROXY (self));
		TNY_MSG_HEADER_PROXY (self)->real = real;
	}

	tny_msg_header_iface_set_from (TNY_MSG_HEADER_IFACE (real), from);

	return;
}

static void
tny_msg_header_proxy_set_to (TnyMsgHeaderIface *self, const gchar *to)
{
	TnyMsgHeader *real = TNY_MSG_HEADER_PROXY (self)->real;

	if (real == NULL)
	{
		real = tny_msg_header_proxy_get_real (TNY_MSG_HEADER_PROXY (self));
		TNY_MSG_HEADER_PROXY (self)->real = real;
	}

	tny_msg_header_iface_set_to (TNY_MSG_HEADER_IFACE (real), to);

	return;
}

static void
tny_msg_header_proxy_set_subject (TnyMsgHeaderIface *self, const gchar *subject)
{
	TnyMsgHeader *real = TNY_MSG_HEADER_PROXY (self)->real;

	if (real == NULL)
	{
		real = tny_msg_header_proxy_get_real (TNY_MSG_HEADER_PROXY (self));
		TNY_MSG_HEADER_PROXY (self)->real = real;
	}

	tny_msg_header_iface_set_subject (TNY_MSG_HEADER_IFACE (real), subject);

	return;
}

static void
tny_msg_header_proxy_set_id (TnyMsgHeaderIface *self, const gchar *id)
{
	TnyMsgHeader *real = TNY_MSG_HEADER_PROXY (self)->real;

	if (real == NULL)
	{
		real = tny_msg_header_proxy_get_real (TNY_MSG_HEADER_PROXY (self));
		TNY_MSG_HEADER_PROXY (self)->real = real;
	}

	tny_msg_header_iface_set_id (TNY_MSG_HEADER_IFACE (real), id);

	return;
}

static const gboolean 
tny_msg_header_proxy_has_cache (TnyMsgHeaderIface *self)
{
	return (TNY_MSG_HEADER_PROXY (self)->real != NULL);
}

static void
tny_msg_header_proxy_uncache (TnyMsgHeaderIface *self)
{

	if (tny_msg_header_proxy_has_cache (self))
	{
		TnyMsgHeader *real = TNY_MSG_HEADER_PROXY (self)->real;
		g_object_unref (G_OBJECT (real));
		TNY_MSG_HEADER_PROXY (self)->real = NULL;
	}

	return;
}

TnyMsgHeaderProxy*
tny_msg_header_proxy_new (void)
{
	TnyMsgHeaderProxy *self = g_object_new (TNY_MSG_HEADER_PROXY_TYPE, NULL);
	
	return self;
}

static void
tny_msg_header_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyMsgHeaderIfaceClass *klass = (TnyMsgHeaderIfaceClass *)g_iface;

	klass->get_from_func = tny_msg_header_proxy_get_from;
	klass->get_id_func = tny_msg_header_proxy_get_id;
	klass->get_to_func = tny_msg_header_proxy_get_to;
	klass->get_subject_func = tny_msg_header_proxy_get_subject;

	klass->set_from_func = tny_msg_header_proxy_set_from;
	klass->set_id_func = tny_msg_header_proxy_set_id;
	klass->set_to_func = tny_msg_header_proxy_set_to;
	klass->set_subject_func = tny_msg_header_proxy_set_subject;
	
	klass->has_cache_func = tny_msg_header_proxy_has_cache;
	klass->uncache_func = tny_msg_header_proxy_uncache;

	return;
}


static void
tny_msg_header_proxy_finalize (GObject *object)
{
	TnyMsgHeaderProxy *self = (TnyMsgHeaderProxy *)object;

	tny_msg_header_proxy_uncache (TNY_MSG_HEADER_IFACE (self));

	(*parent_class->finalize) (object);

	return;
}

static void
tny_msg_header_proxy_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyMsgHeaderProxy *self = (TnyMsgHeaderProxy *)instance;

	self->real = NULL;

	return;
}

static void 
tny_msg_header_proxy_class_init (TnyMsgHeaderProxyClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_msg_header_proxy_finalize;

	/* g_type_class_add_private (object_class, sizeof (MsgHeaderProxyPriv)); */

	return;
}

GType 
tny_msg_header_proxy_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgHeaderProxyClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_msg_header_proxy_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMsgHeaderProxy),
		  0,      /* n_preallocs */
		  tny_msg_header_proxy_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_msg_header_iface_info = 
		{
		  (GInterfaceInitFunc) tny_msg_header_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyMsgHeaderProxy",
			&info, 0);

		g_type_add_interface_static (type, TNY_MSG_HEADER_IFACE_TYPE, 
			&tny_msg_header_iface_info);
	}

	return type;
}

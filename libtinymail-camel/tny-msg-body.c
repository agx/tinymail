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

#include <tny-msg-body.h>

static GObjectClass *parent_class = NULL;

typedef struct _TnyMsgBodyPriv TnyMsgBodyPriv;

struct _TnyMsgBodyPriv
{
	gchar *data;
};

#define TNY_MSG_BODY_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_MSG_BODY_TYPE, TnyMsgBodyPriv))

static const gchar*
tny_msg_body_get_data (TnyMsgBodyIface *self)
{
	TnyMsgBodyPriv *priv = TNY_MSG_BODY_GET_PRIVATE (self);

	return priv->data;
}

static void
tny_msg_body_set_data (TnyMsgBodyIface *self, gchar *data)
{
	TnyMsgBodyPriv *priv = TNY_MSG_BODY_GET_PRIVATE (self);

	/* No allocation in this implementation, the TnyMsg has the allocation */

	priv->data = data;

	return;
}

static void
tny_msg_body_finalize (GObject *object)
{
	TnyMsgBody *self = (TnyMsgBody*) object;
	TnyMsgBodyPriv *priv = TNY_MSG_BODY_GET_PRIVATE (self);

	(*parent_class->finalize) (object);

	return;
}


TnyMsgBody*
tny_msg_body_new (void)
{
	TnyMsgBody *self = g_object_new (TNY_MSG_BODY_TYPE, NULL);
	
	return self;
}


static void
tny_msg_body_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyMsgBodyIfaceClass *klass = (TnyMsgBodyIfaceClass *)g_iface;

	klass->get_data_func = tny_msg_body_get_data;
	klass->set_data_func = tny_msg_body_set_data;

	return;
}

static void 
tny_msg_body_class_init (TnyMsgBodyIfaceClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_msg_body_finalize;

	g_type_class_add_private (object_class, sizeof (TnyMsgBodyPriv));

	return;
}

static void
tny_msg_body_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyMsgBody *self = (TnyMsgBody *)instance;

	TNY_MSG_BODY_GET_PRIVATE (self)->data = NULL;

	return;
}

GType 
tny_msg_body_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgBodyClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_msg_body_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMsgBody),
		  0,      /* n_preallocs */
		  tny_msg_body_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_msg_body_iface_info = 
		{
		  (GInterfaceInitFunc) tny_msg_body_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyMsgBody",
			&info, 0);

		g_type_add_interface_static (type, TNY_MSG_BODY_IFACE_TYPE, 
			&tny_msg_body_iface_info);
	}

	return type;
}

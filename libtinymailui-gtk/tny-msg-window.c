/* libtinymailui-gtk - The Tiny Mail UI library for Gtk+
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

#include <tny-msg-window.h>
#include <tny-msg-view-iface.h>

static GObjectClass *parent_class = NULL;

typedef struct _TnyMsgWindowPriv TnyMsgWindowPriv;

struct _TnyMsgWindowPriv
{
	TnyMsgViewIface *msg_view;
};

#define TNY_MSG_WINDOW_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_MSG_WINDOW_TYPE, TnyMsgWindowPriv))


static void 
tny_msg_window_set_msg (TnyMsgWindowIface *self, TnyMsgIface *msg)
{
	TnyMsgWindowPriv *priv = TNY_MSG_WINDOW_GET_PRIVATE (self);
	TnyMsgHeaderIface *header = TNY_MSG_HEADER_IFACE 
		(tny_msg_iface_get_header (msg));

	tny_msg_view_iface_set_msg (priv->msg_view, msg);

	gtk_window_set_title (GTK_WINDOW (self), tny_msg_header_iface_get_subject (header));

	return;
}

/**
 * tny_msg_window_new:
 *
 *
 * Return value: a new #TnyMsgWindowIface instance implemented for Gtk+
 **/
TnyMsgWindow*
tny_msg_window_new (void)
{
	TnyMsgWindow *self = g_object_new (TNY_MSG_WINDOW_TYPE, NULL);

	return self;
}

static void
tny_msg_window_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyMsgWindow *self = (TnyMsgWindow *)instance;
	TnyMsgWindowPriv *priv = TNY_MSG_WINDOW_GET_PRIVATE (self);

	gtk_window_set_default_size (GTK_WINDOW (self), 640, 480);

	priv->msg_view = TNY_MSG_VIEW_IFACE (tny_msg_view_new ());

	gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (priv->msg_view));

	gtk_widget_show (GTK_WIDGET (priv->msg_view));

	return;
}

static void
tny_msg_window_finalize (GObject *object)
{
	TnyMsgWindow *self = (TnyMsgWindow *)object;	
	TnyMsgWindowPriv *priv = TNY_MSG_WINDOW_GET_PRIVATE (self);

	/* gtk unrefs priv->msg_view for me */

	(*parent_class->finalize) (object);

	return;
}

static void
tny_msg_window_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyMsgWindowIfaceClass *klass = (TnyMsgWindowIfaceClass *)g_iface;

	klass->set_msg_func = tny_msg_window_set_msg;

	return;
}

static void 
tny_msg_window_class_init (TnyMsgWindowClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_msg_window_finalize;

	g_type_class_add_private (object_class, sizeof (TnyMsgWindowPriv));

	return;
}

GType 
tny_msg_window_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgWindowClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_msg_window_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMsgWindow),
		  0,      /* n_preallocs */
		  tny_msg_window_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_msg_window_iface_info = 
		{
		  (GInterfaceInitFunc) tny_msg_window_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (GTK_TYPE_WINDOW,
			"TnyMsgWindow",
			&info, 0);

		g_type_add_interface_static (type, TNY_MSG_WINDOW_IFACE_TYPE, 
			&tny_msg_window_iface_info);

	}

	return type;
}

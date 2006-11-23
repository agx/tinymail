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

#include <config.h>

#include <tny-gtk-msg-window.h>
#include <tny-msg-view.h>

static GObjectClass *parent_class = NULL;

typedef struct _TnyGtkMsgWindowPriv TnyGtkMsgWindowPriv;

struct _TnyGtkMsgWindowPriv
{
	TnyMsgView *msg_view;
};

#define TNY_GTK_MSG_WINDOW_GET_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_GTK_MSG_WINDOW, TnyGtkMsgWindowPriv))


static void
tny_gtk_msg_window_set_unavailable (TnyMsgView *self)
{
	TNY_GTK_MSG_WINDOW_GET_CLASS (self)->set_unavailable_func (self);
	return;
}

static void
tny_gtk_msg_window_set_unavailable_default (TnyMsgView *self)
{
	TnyGtkMsgWindowPriv *priv = TNY_GTK_MSG_WINDOW_GET_PRIVATE (self);
	tny_msg_view_set_unavailable (priv->msg_view);
	return;
}

static TnyMsg* 
tny_gtk_msg_window_get_msg (TnyMsgView *self)
{
	return TNY_GTK_MSG_WINDOW_GET_CLASS (self)->get_msg_func (self);
}

static TnyMsg* 
tny_gtk_msg_window_get_msg_default (TnyMsgView *self)
{
	TnyGtkMsgWindowPriv *priv = TNY_GTK_MSG_WINDOW_GET_PRIVATE (self);
	return tny_msg_view_get_msg (priv->msg_view);
}

static void 
tny_gtk_msg_window_set_msg (TnyMsgView *self, TnyMsg *msg)
{
	TNY_GTK_MSG_WINDOW_GET_CLASS (self)->set_msg_func (self, msg);
	return;
}

static void 
tny_gtk_msg_window_set_msg_default (TnyMsgView *self, TnyMsg *msg)
{
	TnyGtkMsgWindowPriv *priv = TNY_GTK_MSG_WINDOW_GET_PRIVATE (self);
	TnyHeader *header;

	if (msg)
	{
		g_assert (TNY_IS_MSG (msg));

		header = TNY_HEADER (tny_msg_get_header (msg));
		gtk_window_set_title (GTK_WINDOW (self), 
			tny_header_get_subject (header));
		g_object_unref (G_OBJECT (header));
	}
	tny_msg_view_set_msg (priv->msg_view, msg);

	return;
}




static void 
tny_gtk_msg_window_clear (TnyMsgView *self)
{
	return TNY_GTK_MSG_WINDOW_GET_CLASS (self)->clear_func (self);
}

static void 
tny_gtk_msg_window_clear_default (TnyMsgView *self)
{
	TnyGtkMsgWindowPriv *priv = TNY_GTK_MSG_WINDOW_GET_PRIVATE (self);

	tny_msg_view_clear (priv->msg_view);

	return;
}

static TnyMsgView* 
tny_gtk_msg_window_create_new_mytype (TnyMsgView *self)
{
	return TNY_GTK_MSG_WINDOW_GET_CLASS (self)->create_new_mytype_func (self);
}

static TnyMsgView* 
tny_gtk_msg_window_create_new_mytype_default (TnyMsgView *self)
{
	TnyGtkMsgWindowPriv *priv = TNY_GTK_MSG_WINDOW_GET_PRIVATE (self);

	/* This should not be a window, but the decorated one, as it has to be 
	   embeddable within a window. So we even decorate this one as usual. */

	return tny_msg_view_create_new_mytype (priv->msg_view);
}

static TnyMimePartView* 
tny_gtk_msg_window_create_mime_part_view_for (TnyMsgView *self, TnyMimePart *part)
{
	return TNY_GTK_MSG_WINDOW_GET_CLASS (self)->create_mime_part_view_for_func (self, part);
}

static TnyMimePartView* 
tny_gtk_msg_window_create_mime_part_view_for_default (TnyMsgView *self, TnyMimePart *part)
{
	TnyGtkMsgWindowPriv *priv = TNY_GTK_MSG_WINDOW_GET_PRIVATE (self);

	return tny_msg_view_create_mime_part_view_for (priv->msg_view, part);
}



static void 
tny_gtk_msg_window_mp_clear (TnyMimePartView *self)
{
	tny_msg_view_clear (TNY_MSG_VIEW (self));

	return;
}


static void 
tny_gtk_msg_window_mp_set_part (TnyMimePartView *self, TnyMimePart *part)
{
	TNY_GTK_MSG_WINDOW_GET_CLASS (self)->set_part_func (self, part);
	return;
}


static void 
tny_gtk_msg_window_mp_set_part_default (TnyMimePartView *self, TnyMimePart *part)
{
	TnyGtkMsgWindowPriv *priv = TNY_GTK_MSG_WINDOW_GET_PRIVATE (self);

	tny_mime_part_view_set_part (TNY_MIME_PART_VIEW (priv->msg_view), part);

	return;
}


static TnyMimePart* 
tny_gtk_msg_window_mp_get_part (TnyMimePartView *self)
{
	return TNY_GTK_MSG_WINDOW_GET_CLASS (self)->get_part_func (self);
}


static TnyMimePart* 
tny_gtk_msg_window_mp_get_part_default (TnyMimePartView *self)
{
	TnyGtkMsgWindowPriv *priv = TNY_GTK_MSG_WINDOW_GET_PRIVATE (self);

	return tny_mime_part_view_get_part (TNY_MIME_PART_VIEW (priv->msg_view));
}


/**
 * tny_gtk_msg_window_new:
 *
 *
 * Return value: a new #TnyMsgWindow instance implemented for Gtk+
 **/
TnyMsgWindow*
tny_gtk_msg_window_new (TnyMsgView *msgview)
{
	TnyGtkMsgWindow *self = g_object_new (TNY_TYPE_GTK_MSG_WINDOW, NULL);
	TnyGtkMsgWindowPriv *priv = TNY_GTK_MSG_WINDOW_GET_PRIVATE (self);

	if (G_UNLIKELY (priv->msg_view))
		gtk_container_remove (GTK_CONTAINER (self), GTK_WIDGET (priv->msg_view));

	priv->msg_view = msgview;

	/* This adds a reference to msgview (it's a gtkwidget) */
	gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (priv->msg_view));
	gtk_widget_show (GTK_WIDGET (priv->msg_view));

	return TNY_MSG_WINDOW (self);
}

static void
tny_gtk_msg_window_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyGtkMsgWindow *self = (TnyGtkMsgWindow *)instance;

	gtk_window_set_default_size (GTK_WINDOW (self), 640, 480);

	return;
}

static void
tny_gtk_msg_window_finalize (GObject *object)
{
	(*parent_class->finalize) (object);

	return;
}

static void
tny_msg_window_init (gpointer g, gpointer iface_data)
{
	return;
}


static void
tny_msg_view_init (gpointer g, gpointer iface_data)
{
	TnyMsgViewIface *klass = (TnyMsgViewIface *)g;

	klass->get_msg_func = tny_gtk_msg_window_get_msg;
	klass->set_msg_func = tny_gtk_msg_window_set_msg;
	klass->set_unavailable_func = tny_gtk_msg_window_set_unavailable;
	klass->clear_func = tny_gtk_msg_window_clear;

	return;
}

static void
tny_mime_part_view_init (gpointer g, gpointer iface_data)
{
	TnyMimePartViewIface *klass = (TnyMimePartViewIface *)g;

	klass->get_part_func = tny_gtk_msg_window_mp_get_part;
	klass->set_part_func = tny_gtk_msg_window_mp_set_part;
	klass->clear_func = tny_gtk_msg_window_mp_clear;

	return;
}

static void 
tny_gtk_msg_window_class_init (TnyGtkMsgWindowClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	class->get_msg_func = tny_gtk_msg_window_get_msg_default;
	class->set_msg_func = tny_gtk_msg_window_set_msg_default;
	class->set_unavailable_func = tny_gtk_msg_window_set_unavailable_default;
	class->clear_func = tny_gtk_msg_window_clear_default;
	class->create_new_mytype_func = tny_gtk_msg_window_create_new_mytype_default;
	class->create_mime_part_view_for_func = tny_gtk_msg_window_create_mime_part_view_for_default;

	class->get_part_func = tny_gtk_msg_window_mp_get_part_default;
	class->set_part_func = tny_gtk_msg_window_mp_set_part_default;

	object_class->finalize = tny_gtk_msg_window_finalize;

	g_type_class_add_private (object_class, sizeof (TnyGtkMsgWindowPriv));

	return;
}

GType 
tny_gtk_msg_window_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyGtkMsgWindowClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_gtk_msg_window_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyGtkMsgWindow),
		  0,      /* n_preallocs */
		  tny_gtk_msg_window_instance_init,    /* instance_init */
		  NULL
		};

		static const GInterfaceInfo tny_msg_window_info = 
		{
		  (GInterfaceInitFunc) tny_msg_window_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		static const GInterfaceInfo tny_msg_view_info = 
		{
		  (GInterfaceInitFunc) tny_msg_view_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		static const GInterfaceInfo tny_mime_part_view_info = 
		{
		  (GInterfaceInitFunc) tny_mime_part_view_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (GTK_TYPE_WINDOW,
			"TnyGtkMsgWindow",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_MIME_PART_VIEW, 
			&tny_mime_part_view_info);

		g_type_add_interface_static (type, TNY_TYPE_MSG_VIEW, 
			&tny_msg_view_info);

		g_type_add_interface_static (type, TNY_TYPE_MSG_WINDOW, 
			&tny_msg_window_info);

	}

	return type;
}

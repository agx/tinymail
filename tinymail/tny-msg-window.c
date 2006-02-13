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

#include <string.h>
#include <gtk/gtk.h>
#include <tny-msg-window.h>
#include <tny-text-buffer-stream.h>

static GObjectClass *parent_class = NULL;

typedef struct _TnyMsgWindowPriv TnyMsgWindowPriv;

struct _TnyMsgWindowPriv
{
	TnyMsgIface *msg;
	GtkTextView *textview;
};

#define TNY_MSG_WINDOW_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_MSG_WINDOW_TYPE, TnyMsgWindowPriv))


static void
reload_msg (TnyMsgWindowIface *self)
{
	TnyMsgWindowPriv *priv = TNY_MSG_WINDOW_GET_PRIVATE (self);

	GtkTextBuffer *buffer = gtk_text_view_get_buffer (priv->textview);
	TnyStreamIface *dest = TNY_STREAM_IFACE (tny_text_buffer_stream_new (buffer));
	TnyMsgHeaderIface *header = TNY_MSG_HEADER_IFACE (tny_msg_iface_get_header (priv->msg));
	GList *parts = (GList*)tny_msg_iface_get_parts (priv->msg);

	while (parts)
	{
		TnyMsgMimePartIface *part = parts->data;

		if (tny_msg_mime_part_iface_content_type_is (part, "text/*"))
		{
			TnyStreamIface *source = tny_msg_mime_part_iface_get_stream (part);
			
			tny_stream_iface_reset (source);
			tny_stream_iface_reset (dest);

			tny_stream_iface_write_to_stream (source, dest);

			break;
		}

		parts = g_list_next (parts);
	}

	gtk_window_set_title (GTK_WINDOW (self), tny_msg_header_iface_get_subject (header));

	return;
}

static void 
tny_msg_window_set_msg (TnyMsgWindowIface *self, TnyMsgIface *msg)
{
	TnyMsgWindowPriv *priv = TNY_MSG_WINDOW_GET_PRIVATE (self);

	if (priv->msg)
		g_object_unref (G_OBJECT (priv->msg));

	g_object_ref (G_OBJECT (msg));

	priv->msg = msg;

	reload_msg (self);

	return;
}


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
	
	priv->textview = GTK_TEXT_VIEW (gtk_text_view_new ());
	gtk_container_set_border_width (GTK_CONTAINER (self), 8);
	gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (priv->textview));

	gtk_widget_show (GTK_WIDGET (priv->textview));

	return;
}

static void
tny_msg_window_finalize (GObject *object)
{
	TnyMsgWindow *self = (TnyMsgWindow *)object;	
	TnyMsgWindowPriv *priv = TNY_MSG_WINDOW_GET_PRIVATE (self);

	if (priv->msg)
		g_object_unref (G_OBJECT (priv->msg));

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

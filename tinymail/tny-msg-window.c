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

#include <gtk/gtk.h>
#include <tny-msg-window.h>

static GObjectClass *parent_class = NULL;


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
	GtkWindow *window = GTK_WINDOW (self);

	gtk_window_set_title (window, "Message");
	gtk_container_set_border_width (GTK_CONTAINER (window), 8);

	return;
}

static void
tny_msg_window_finalize (GObject *object)
{
	TnyMsgWindow *self = (TnyMsgWindow *)object;	
	
	(*parent_class->finalize) (object);

	return;
}


static void 
tny_msg_window_class_init (TnyMsgWindowClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_msg_window_finalize;

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

		type = g_type_register_static (GTK_TYPE_WINDOW,
			"TnyMsgWindow",
			&info, 0);

	}

	return type;
}

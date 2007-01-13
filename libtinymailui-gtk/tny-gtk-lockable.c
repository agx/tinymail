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
#include <glib.h>
#include <glib/gi18n-lib.h>

#include <tny-gtk-lockable.h>

static GObjectClass *parent_class = NULL;

static void
tny_gtk_lockable_lock (TnyLockable *self)
{
	gdk_threads_enter ();
	gdk_flush ();
}

static void
tny_gtk_lockable_unlock (TnyLockable *self)
{
	gdk_flush ();
	gdk_threads_leave ();
}

static void
tny_gtk_lockable_finalize (GObject *object)
{
	parent_class->finalize (object);
}

static void
tny_lockable_init (TnyLockableIface *klass)
{
	klass->lock_func = tny_gtk_lockable_lock;
	klass->unlock_func = tny_gtk_lockable_unlock;
}

static void
tny_gtk_lockable_class_init (TnyGtkLockableClass *klass)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;
	object_class->finalize = tny_gtk_lockable_finalize;
}

static void
tny_gtk_lockable_instance_init (GTypeInstance *instance, gpointer g_class)
{
	return;
}

/**
 * tny_gtk_lockable_new:
 *
 * Create a #TnyLockable instance that uses gdk_threads_enter and gdk_threads_leave
 * as lock and unlock implementations.
 *
 * Return value: a new #TnyLockable instance implemented for Gtk+
 **/
TnyLockable*
tny_gtk_lockable_new (void)
{
	TnyGtkLockable *self = g_object_new (TNY_TYPE_GTK_LOCKABLE, NULL);
	return TNY_LOCKABLE (self);
}


GType
tny_gtk_lockable_get_type (void)
{
	static GType type = 0;
	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
			sizeof (TnyGtkLockableClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			(GClassInitFunc) tny_gtk_lockable_class_init,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			sizeof (TnyGtkLockable),
			0,      /* n_preallocs */
			tny_gtk_lockable_instance_init,    /* instance_init */
			NULL
		};


		static const GInterfaceInfo tny_lockable_info = 
		{
			(GInterfaceInitFunc) tny_lockable_init, /* interface_init */
			NULL,         /* interface_finalize */
			NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyGtkLockable",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_LOCKABLE,
			&tny_lockable_info);

	}
	return type;
}

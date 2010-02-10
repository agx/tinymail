/* libtinymail-moblin - The Tinymail base library for Gnome
 * Copyright (C) 2010 Sergio Villar Senin <svillar@igalia.com>
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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include <config.h>

#include <tny-moblin-platform-factory.h>
#include <tny-moblin-account-store.h>
#include <tny-moblin-device.h>
#include <tny-camel-mime-part.h>
#include <tny-camel-msg.h>

#ifdef GTKHTML
#include <tny-gtk-html-msg-view.h>
#endif

#include <tny-gtk-msg-view.h>
#include <tny-gtk-password-dialog.h>

static GObjectClass *parent_class = NULL;

static void
tny_moblin_platform_factory_instance_init (GTypeInstance *instance, gpointer g_class)
{
	/* Empty */
}


static TnyMsg*
tny_moblin_platform_factory_new_msg (TnyPlatformFactory *self)
{
	return tny_camel_msg_new ();
}


static TnyMimePart*
tny_moblin_platform_factory_new_mime_part (TnyPlatformFactory *self)
{
	return tny_camel_mime_part_new ();
}


static TnyAccountStore*
tny_moblin_platform_factory_new_account_store (TnyPlatformFactory *self)
{
	/* return tny_moblin_account_store_new (); */
	return NULL;
}

static TnyDevice*
tny_moblin_platform_factory_new_device (TnyPlatformFactory *self)
{
	return tny_moblin_device_new ();
}


static TnyPasswordGetter*
tny_moblin_platform_factory_new_password_getter (TnyPlatformFactory *self)
{
	return tny_gtk_password_dialog_new ();
}

static TnyMsgView*
tny_moblin_platform_factory_new_msg_view (TnyPlatformFactory *self)
{
	TnyMsgView *retval = NULL;

#ifdef GTKHTML
	retval = tny_gtk_html_msg_view_new ();
#endif

	if (!retval)
		retval = tny_gtk_msg_view_new ();

	return retval;
}

/**
 * tny_moblin_platform_factory_get_instance:
 *
 * Get the platform factory singleton instance implemented for Moblin desktops
 *
 * Return value: The #TnyPlatformFactory singleton instance implemented for Moblin desktops
 **/
TnyPlatformFactory*
tny_moblin_platform_factory_get_instance (void)
{
	TnyMoblinPlatformFactory *self = g_object_new (TNY_TYPE_MOBLIN_PLATFORM_FACTORY, NULL);

	return TNY_PLATFORM_FACTORY (self);
}


static void
tny_moblin_platform_factory_finalize (GObject *object)
{
	(*parent_class->finalize) (object);
}


static void
tny_platform_factory_init (gpointer g, gpointer iface_data)
{
	TnyPlatformFactoryIface *klass = (TnyPlatformFactoryIface *)g;

	klass->new_account_store= tny_moblin_platform_factory_new_account_store;
	klass->new_device= tny_moblin_platform_factory_new_device;
	klass->new_msg_view= tny_moblin_platform_factory_new_msg_view;
	klass->new_msg= tny_moblin_platform_factory_new_msg;
	klass->new_mime_part= tny_moblin_platform_factory_new_mime_part;
	klass->new_password_getter= tny_moblin_platform_factory_new_password_getter;
}


static TnyMoblinPlatformFactory *the_singleton = NULL;


static GObject*
tny_moblin_platform_factory_constructor (GType type, guint n_construct_params,
					 GObjectConstructParam *construct_params)
{
	GObject *object;

	/* TODO: potential problem: singleton without lock */

	if (G_UNLIKELY (!the_singleton)) {
		object = G_OBJECT_CLASS (parent_class)->constructor (type,
				n_construct_params, construct_params);

		the_singleton = TNY_MOBLIN_PLATFORM_FACTORY (object);
	}
	else {
		object = G_OBJECT (the_singleton);
		g_object_freeze_notify (G_OBJECT(the_singleton));
	}

	return object;
}

static void
tny_moblin_platform_factory_class_init (TnyMoblinPlatformFactoryClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_moblin_platform_factory_finalize;
	object_class->constructor = tny_moblin_platform_factory_constructor;
}

static gpointer
tny_moblin_platform_factory_register_type (gpointer notused)
{
	GType type = 0;

	static const GTypeInfo info =
		{
			sizeof (TnyMoblinPlatformFactoryClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			(GClassInitFunc) tny_moblin_platform_factory_class_init,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			sizeof (TnyMoblinPlatformFactory),
			0,      /* n_preallocs */
			tny_moblin_platform_factory_instance_init    /* instance_init */
		};

	static const GInterfaceInfo tny_platform_factory_info =
		{
			(GInterfaceInitFunc) tny_platform_factory_init, /* interface_init */
			NULL,         /* interface_finalize */
			NULL          /* interface_data */
		};

	type = g_type_register_static (G_TYPE_OBJECT,
				       "TnyMoblinPlatformFactory",
				       &info, 0);

	g_type_add_interface_static (type, TNY_TYPE_PLATFORM_FACTORY,
				     &tny_platform_factory_info);


	return GUINT_TO_POINTER (type);
}

GType
tny_moblin_platform_factory_get_type (void)
{
	static GOnce once = G_ONCE_INIT;
	g_once (&once, tny_moblin_platform_factory_register_type, NULL);
	return GPOINTER_TO_UINT (once.retval);
}

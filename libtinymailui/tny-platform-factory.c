/* libtinymailui - The Tiny Mail user interface library
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

/* The reason why this type is defined in libtinymailui rather than libtinymail
   is because the factory also returns types defined in libtinymailui */
   
#include <config.h>

#include <tny-platform-factory.h>


/**
 * tny_platform_factory_new_msg:
 * @self: a TnyPlatformFactory object
 *
 * Create a new #TnyMsg instance. The returned instance must be 
 * unreferenced after use.
 *
 * Implementors: when implementing a platform-specific library, return a 
 * new #TnyMsg instance.
 *
 * Return value: a #TnyMsg instance
 **/
TnyMsg* 
tny_platform_factory_new_msg (TnyPlatformFactory *self)
{
#ifdef DEBUG
	if (!TNY_PLATFORM_FACTORY_GET_IFACE (self)->new_msg_func)
		g_critical ("You must implement tny_platform_factory_new_msg\n");
#endif

	return TNY_PLATFORM_FACTORY_GET_IFACE (self)->new_msg_func (self);

}


/**
 * tny_platform_factory_new_mime_part:
 * @self: a TnyPlatformFactory object
 *
 * Create a new #TnyMimePart instance. The returned instance must be 
 * unreferenced after use.
 *
 * Implementors: when implementing a platform-specific library, return a 
 * new #TnyMimePart instance.
 *
 * Return value: a #TnyMimePart instance
 **/
TnyMimePart* 
tny_platform_factory_new_mime_part (TnyPlatformFactory *self)
{
#ifdef DEBUG
	if (!TNY_PLATFORM_FACTORY_GET_IFACE (self)->new_mime_part_func)
		g_critical ("You must implement tny_platform_factory_new_mime_part\n");
#endif

	return TNY_PLATFORM_FACTORY_GET_IFACE (self)->new_mime_part_func (self);
}


/**
 * tny_platform_factory_new_account_store:
 * @self: a TnyPlatformFactory object
 *
 * Create a new #TnyAccountStore instance. The returned instance must be 
 * unreferenced after use.
 *
 * Implementors: when implementing a platform-specific library, return a 
 * new #TnyAccountStore instance. It's allowed to reuse one instance, just
 * make sure that you add a reference then.
 *
 * Return value: a #TnyAccountStore instance
 **/
TnyAccountStore*
tny_platform_factory_new_account_store (TnyPlatformFactory *self)
{
#ifdef DEBUG
	if (!TNY_PLATFORM_FACTORY_GET_IFACE (self)->new_account_store_func)
		g_critical ("You must implement tny_platform_factory_new_account_store\n");
#endif

	return TNY_PLATFORM_FACTORY_GET_IFACE (self)->new_account_store_func (self);
}

/**
 * tny_platform_factory_new_device:
 * @self: a TnyPlatformFactory object
 *
 * Create a new #TnyDevice instance. The returned instance must be 
 * unreferenced after use.
 *
 * Implementors: when implementing a platform-specific library, return a 
 * new #TnyDevice instance. It's allowed to reuse one instance, just
 * make sure that you add a reference then.
 *
 * Return value: a #TnyDevice instance
 *
 **/
TnyDevice*
tny_platform_factory_new_device (TnyPlatformFactory *self)
{
#ifdef DEBUG
	if (!TNY_PLATFORM_FACTORY_GET_IFACE (self)->new_device_func)
		g_critical ("You must implement tny_platform_factory_new_device\n");
#endif

	return TNY_PLATFORM_FACTORY_GET_IFACE (self)->new_device_func (self);
}

/**
 * tny_platform_factory_new_msg_view:
 * @self: a TnyPlatformFactory object
 *
 * Create a new #TnyMsgView instance. The returned instance must be 
 * unreferenced after use.
 *
 * Implementors: when implementing a platform-specific library, return a 
 * new #TnyMsgView instance. It's allowed to reuse one instance, just
 * make sure that you add a reference then.
 *
 * Return value: a #TnyMsgView instance
 **/
TnyMsgView*
tny_platform_factory_new_msg_view (TnyPlatformFactory *self)
{
#ifdef DEBUG
	if (!TNY_PLATFORM_FACTORY_GET_IFACE (self)->new_msg_view_func)
		g_warning ("You must implement tny_platform_factory_new_msg_view\n");
#endif

	return TNY_PLATFORM_FACTORY_GET_IFACE (self)->new_msg_view_func (self);
}


static void
tny_platform_factory_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) 
		initialized = TRUE;
}

GType
tny_platform_factory_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyPlatformFactoryIface),
		  tny_platform_factory_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyPlatformFactory", &info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}

	return type;
}

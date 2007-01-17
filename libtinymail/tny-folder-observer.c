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

#include <config.h>

#include <tny-folder-observer.h>



/**
 * tny_folder_observer_update:
 * @self: A #TnyFolderObserver instance
 *
 * Observer's update method
 *
 **/
void 
tny_folder_observer_update (TnyFolderObserver *self, TnyFolderChange *change)
{
#ifdef DEBUG
	if (!TNY_FOLDER_OBSERVER_GET_IFACE (self)->update_func)
		g_critical ("You must implement tny_folder_observer_update\n");
#endif

	TNY_FOLDER_OBSERVER_GET_IFACE (self)->update_func (self, change);
	return;
}


static void
tny_folder_observer_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_folder_observer_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyFolderObserverIface),
		  tny_folder_observer_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL,   /* instance_init */
		  NULL
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyFolderObserver", &info, 0);
	}

	return type;
}


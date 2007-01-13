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

#include <tny-lockable.h>

/**
 * tny_lockable_lock:
 * @self: a #TnyLockable object
 * 
 * Lock @self
 **/
void 
tny_lockable_lock (TnyLockable *self)
{
#ifdef DEBUG
	if (!TNY_LOCKABLE_GET_IFACE (self)->lock_func)
		g_critical ("You must implement tny_lockable_lock\n");
#endif

	TNY_LOCKABLE_GET_IFACE (self)->lock_func (self);
	return;
}

/**
 * tny_lockable_unlock:
 * @self: a #TnyLockable object
 * 
 * Unlock @self
 **/
void 
tny_lockable_unlock (TnyLockable *self)
{
#ifdef DEBUG
	if (!TNY_LOCKABLE_GET_IFACE (self)->unlock_func)
		g_critical ("You must implement tny_lockable_unlock\n");
#endif

	TNY_LOCKABLE_GET_IFACE (self)->unlock_func (self);
	return;
}



static void
tny_lockable_base_init (gpointer g_class)
{
	return;
}

GType
tny_lockable_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyLockableIface),
		  tny_lockable_base_init,   /* base_init */
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
			"TnyLockable", &info, 0);
	}

	return type;
}

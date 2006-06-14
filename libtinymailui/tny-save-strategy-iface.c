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

#include <tny-save-strategy-iface.h>

/**
 * tny_save_strategy_iface_save:
 * @self: A TnySaveStrategyIface instance
 *
 * Performs the saving
 * 
 **/
void
tny_save_strategy_iface_save (TnySaveStrategyIface *self, TnyMsgMimePartIface *part)
{
#ifdef DEBUG
	if (!TNY_SAVE_STRATEGY_IFACE_GET_CLASS (self)->save_func)
		g_critical ("You must implement tny_save_strategy_iface_save\n");
#endif

	TNY_SAVE_STRATEGY_IFACE_GET_CLASS (self)->save_func (self, part);
	return;
}

static void
tny_save_strategy_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) 
		initialized = TRUE;
}

GType
tny_save_strategy_iface_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnySaveStrategyIfaceClass),
		  tny_save_strategy_iface_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnySaveStrategyIface", &info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}

	return type;
}

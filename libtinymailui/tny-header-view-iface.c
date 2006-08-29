/* libtinymailui - The Tiny Mail UI library
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

#include <tny-header-view-iface.h>


/**
 * tny_header_view_iface_clear:
 * @self: A #TnyHeaderViewIface instance
 *
 * Clear the view @self (show nothing)
 *
 * Implementors:  this method should clear @self (display nothing)
 * 
 **/
void
tny_header_view_iface_clear (TnyHeaderViewIface *self)
{
#ifdef DEBUG
	if (!TNY_HEADER_VIEW_IFACE_GET_CLASS (self)->clear_func)
		g_critical ("You must implement tny_header_view_iface_clear\n");
#endif

	TNY_HEADER_VIEW_IFACE_GET_CLASS (self)->clear_func (self);
	return;    
}


/**
 * tny_header_view_iface_set_header:
 * @self: A #TnyHeaderViewIface instance
 * @header: A #TnyHeaderIface instace
 *
 * Set header of the view @self (show nothing)
 * 
 * Implementors: this method should cause the view @self to show the header
 * @header to the user. Often this means showing the from, to, subject, date
 * and cc labels.
 *
 * #TnyHeaderViewIface is often used in a composition with the #TnyMsgViewIface
 * type (the #TnyMsgViewIface implementation contains a #TnyHeaderViewIface).
 *
 **/
void
tny_header_view_iface_set_header (TnyHeaderViewIface *self, TnyHeaderIface *header)
{
#ifdef DEBUG
	if (!TNY_HEADER_VIEW_IFACE_GET_CLASS (self)->set_header_func)
		g_critical ("You must implement tny_header_view_iface_set_header\n");
#endif

	TNY_HEADER_VIEW_IFACE_GET_CLASS (self)->set_header_func (self, header);
	return;
}

static void
tny_header_view_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_header_view_iface_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyHeaderViewIfaceClass),
		  tny_header_view_iface_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyHeaderViewIface", &info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}

	return type;
}



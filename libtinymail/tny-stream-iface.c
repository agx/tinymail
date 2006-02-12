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

#include <tny-stream-iface.h>

ssize_t
tny_stream_iface_write_to_stream (TnyStreamIface *self, TnyStreamIface *output)
{
	return TNY_STREAM_IFACE_GET_CLASS (self)->write_to_stream_func (self, output);
}

ssize_t
tny_stream_iface_read  (TnyStreamIface *self, char *buffer, size_t n)
{
	return TNY_STREAM_IFACE_GET_CLASS (self)->read_func (self, buffer, n);
}

ssize_t
tny_stream_iface_write (TnyStreamIface *self, const char *buffer, size_t n)
{
	return TNY_STREAM_IFACE_GET_CLASS (self)->write_func (self, buffer, n);
}

gint
tny_stream_iface_flush (TnyStreamIface *self)
{
	return TNY_STREAM_IFACE_GET_CLASS (self)->flush_func (self);
}

gint
tny_stream_iface_close (TnyStreamIface *self)
{
	return TNY_STREAM_IFACE_GET_CLASS (self)->close_func (self);
}

gboolean
tny_stream_iface_eos   (TnyStreamIface *self)
{
	return TNY_STREAM_IFACE_GET_CLASS (self)->eos_func (self);
}

gint
tny_stream_iface_reset (TnyStreamIface *self)
{
	return TNY_STREAM_IFACE_GET_CLASS (self)->reset_func (self);
}

static void
tny_stream_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_stream_iface_get_type (void)
{
	static GType type = 0;
	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyStreamIfaceClass),
		  tny_stream_iface_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyStreamIface", &info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}

	return type;
}



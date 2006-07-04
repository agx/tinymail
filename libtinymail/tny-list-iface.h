#ifndef TNY_LIST_IFACE_H
#define TNY_LIST_IFACE_H

/* Urgent TODO: add a unit test for this type */

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

#include <glib.h>
#include <glib-object.h>

#include <tny-shared.h>
#include <tny-iterator-iface.h>

G_BEGIN_DECLS

#define TNY_TYPE_LIST_IFACE             (tny_list_iface_get_type ())
#define TNY_LIST_IFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_LIST_IFACE, TnyListIface))
#define TNY_LIST_IFACE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_LIST_IFACE, TnyListIfaceClass))
#define TNY_IS_LIST_IFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_LIST_IFACE))
#define TNY_IS_LIST_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_LIST_IFACE))
#define TNY_LIST_IFACE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_LIST_IFACE, TnyListIfaceClass))

struct _TnyListIfaceClass
{
	GTypeInterface parent;

	guint (*length_func) (TnyListIface *self);
	void (*prepend_func) (TnyListIface *self, gpointer item);
	void (*append_func) (TnyListIface *self, gpointer item);
	void (*remove_func) (TnyListIface *self, gpointer item);
	void (*foreach_func) (TnyListIface *self, GFunc func, gpointer user_data);
	TnyListIface* (*copy_func) (TnyListIface *self);
	TnyIteratorIface* (*create_iterator_func) (TnyListIface *self);
};

GType tny_list_iface_get_type (void);

guint tny_list_iface_length (TnyListIface *self);
void tny_list_iface_prepend (TnyListIface *self, gpointer item);
void tny_list_iface_append (TnyListIface *self, gpointer item);
void tny_list_iface_remove (TnyListIface *self, gpointer item);
void tny_list_iface_foreach (TnyListIface *self, GFunc func, gpointer user_data);
TnyIteratorIface* tny_list_iface_create_iterator (TnyListIface *self);
TnyListIface *tny_list_iface_copy (TnyListIface *self);

G_END_DECLS

#endif

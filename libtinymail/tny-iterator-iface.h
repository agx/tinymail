#ifndef TNY_ITERATOR_IFACE_H
#define TNY_ITERATOR_IFACE_H

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

G_BEGIN_DECLS

#define TNY_TYPE_ITERATOR_IFACE             (tny_iterator_iface_get_type ())
#define TNY_ITERATOR_IFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_ITERATOR_IFACE, TnyIteratorIface))
#define TNY_ITERATOR_IFACE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_ITERATOR_IFACE, TnyIteratorIfaceClass))
#define TNY_IS_ITERATOR_IFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_ITERATOR_IFACE))
#define TNY_IS_ITERATOR_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_ITERATOR_IFACE))
#define TNY_ITERATOR_IFACE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_ITERATOR_IFACE, TnyIteratorIfaceClass))

struct _TnyIteratorIfaceClass
{
	GTypeInterface parent;

	void (*next_func) (TnyIteratorIface *self);
	void (*prev_func) (TnyIteratorIface *self);
	void (*first_func) (TnyIteratorIface *self);
	void (*nth_func) (TnyIteratorIface *self, guint nth);
	GObject* (*current_func) (TnyIteratorIface *self);

	gboolean (*is_done) (TnyIteratorIface *self);
	TnyListIface* (*get_list_func) (TnyIteratorIface *self);
};

GType tny_iterator_iface_get_type (void);

void tny_iterator_iface_next              (TnyIteratorIface *self);
void tny_iterator_iface_prev              (TnyIteratorIface *self);
void tny_iterator_iface_first             (TnyIteratorIface *self);
void tny_iterator_iface_nth               (TnyIteratorIface *self, guint nth);
GObject* tny_iterator_iface_current       (TnyIteratorIface *self);
gboolean tny_iterator_iface_is_done       (TnyIteratorIface *self);
TnyListIface* tny_iterator_iface_get_list (TnyIteratorIface *self);

G_END_DECLS

#endif

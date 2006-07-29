#ifndef TNY_LIST_H
#define TNY_LIST_H

/* libtinymailui-gtk - The Tiny Mail library
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
#include <tny-shared.h>
#include <tny-list-iface.h>
#include <tny-iterator-iface.h>

G_BEGIN_DECLS

#define TNY_TYPE_LIST             (tny_list_get_type ())
#define TNY_LIST(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_LIST, TnyList))
#define TNY_LIST_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_LIST, TnyListClass))
#define TNY_IS_LIST(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_LIST))
#define TNY_IS_LIST_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_LIST))
#define TNY_LIST_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_LIST, TnyListClass))

struct _TnyList 
{
	GObject parent;
};

struct _TnyListClass 
{
	GObjectClass parent;
};

GType tny_list_get_type (void);
TnyListIface* tny_list_new (void);

G_END_DECLS

#endif

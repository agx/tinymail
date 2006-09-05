#ifndef TNY_save_STRATEGY_H
#define TNY_save_STRATEGY_H

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

#define TNY_TYPE_SAVE_STRATEGY             (tny_save_strategy_get_type ())
#define TNY_SAVE_STRATEGY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_SAVE_STRATEGY, TnySaveStrategy))
#define TNY_IS_SAVE_STRATEGY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_SAVE_STRATEGY))
#define TNY_SAVE_STRATEGY_GET_IFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_SAVE_STRATEGY, TnySaveStrategyIface))

typedef struct _TnySaveStrategy TnySaveStrategy;
typedef struct _TnySaveStrategyIface TnySaveStrategyIface;

struct _TnySaveStrategyIface
{
	GTypeInterface parent;

	void (*save_func) (TnySaveStrategy *self, TnyMimePart *part);
};

GType tny_save_strategy_get_type (void);
void tny_save_strategy_save (TnySaveStrategy *self, TnyMimePart *part);

G_END_DECLS

#endif

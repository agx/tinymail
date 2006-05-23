#ifndef TNY_SAVE_STRATEGY_H
#define TNY_SAVE_STRATEGY_H

/* libtinymailui-gtk - The Tiny Mail UI library for Gtk+
 * Copyright (C) 2006-2007 Philip Van Hoof <pvanhoof@gnome.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with self program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <gtk/gtk.h>
#include <glib-object.h>
#include <tny-shared.h>

#include <tny-save-strategy-iface.h>

G_BEGIN_DECLS

#define TNY_TYPE_SAVE_STRATEGY             (tny_save_strategy_get_type ())
#define TNY_SAVE_STRATEGY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_SAVE_STRATEGY, TnySaveStrategy))
#define TNY_SAVE_STRATEGY_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_SAVE_STRATEGY, TnySaveStrategyClass))
#define TNY_IS_SAVE_STRATEGY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_SAVE_STRATEGY))
#define TNY_IS_SAVE_STRATEGY_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_SAVE_STRATEGY))
#define TNY_SAVE_STRATEGY_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_SAVE_STRATEGY, TnySaveStrategyClass))

typedef struct _TnySaveStrategy TnySaveStrategy;
typedef struct _TnySaveStrategyClass TnySaveStrategyClass;

struct _TnySaveStrategy
{
	GObject parent;

};

struct _TnySaveStrategyClass
{
	GObjectClass parent_class;
};

GType               tny_save_strategy_get_type       (void);
TnySaveStrategy*    tny_save_strategy_new            (void);

G_END_DECLS

#endif

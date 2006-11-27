#ifndef TNY_CAMEL_MSG_REMOVE_STRATEGY_H
#define TNY_CAMEL_MSG_REMOVE_STRATEGY_H

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

#include <glib-object.h>

#include <tny-msg-remove-strategy.h>

G_BEGIN_DECLS

#define TNY_TYPE_CAMEL_MSG_REMOVE_STRATEGY             (tny_camel_msg_remove_strategy_get_type ())
#define TNY_CAMEL_MSG_REMOVE_STRATEGY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_CAMEL_MSG_REMOVE_STRATEGY, TnyCamelMsgRemoveStrategy))
#define TNY_CAMEL_MSG_REMOVE_STRATEGY_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_CAMEL_MSG_REMOVE_STRATEGY, TnyCamelMsgRemoveStrategyClass))
#define TNY_IS_CAMEL_MSG_REMOVE_STRATEGY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_CAMEL_MSG_REMOVE_STRATEGY))
#define TNY_IS_CAMEL_MSG_REMOVE_STRATEGY_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_CAMEL_MSG_REMOVE_STRATEGY))
#define TNY_CAMEL_MSG_REMOVE_STRATEGY_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_CAMEL_MSG_REMOVE_STRATEGY, TnyCamelMsgRemoveStrategyClass))

typedef struct _TnyCamelMsgRemoveStrategy TnyCamelMsgRemoveStrategy;
typedef struct _TnyCamelMsgRemoveStrategyClass TnyCamelMsgRemoveStrategyClass;

struct _TnyCamelMsgRemoveStrategy
{
	GObject parent;

};

struct _TnyCamelMsgRemoveStrategyClass
{
	GObjectClass parent_class;

	/* virtual methods */
	void (*perform_remove_func) (TnyMsgRemoveStrategy *self, TnyFolder *folder, TnyHeader *header, GError **err);
};

GType tny_camel_msg_remove_strategy_get_type (void);
TnyMsgRemoveStrategy* tny_camel_msg_remove_strategy_new (void);

G_END_DECLS

#endif

#ifndef TNY_CONNECTION_STRATEGY_H
#define TNY_CONNECTION_STRATEGY_H

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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <glib.h>
#include <glib-object.h>

#include <tny-shared.h>

G_BEGIN_DECLS

#define TNY_TYPE_CONNECTION_STRATEGY             (tny_connection_strategy_get_type ())
#define TNY_CONNECTION_STRATEGY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_CONNECTION_STRATEGY, TnyConnectionStrategy))
#define TNY_IS_CONNECTION_STRATEGY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_CONNECTION_STRATEGY))
#define TNY_CONNECTION_STRATEGY_GET_IFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_CONNECTION_STRATEGY, TnyConnectionStrategyIface))

#ifndef TNY_SHARED_H
typedef struct _TnyConnectionStrategy TnyConnectionStrategy;
typedef struct _TnyConnectionStrategyIface TnyConnectionStrategyIface;
#endif

struct _TnyConnectionStrategyIface
{
	GTypeInterface parent;

	void (*on_connect_func) (TnyConnectionStrategy *self, TnyAccount *account);
	void (*on_connection_broken_func) (TnyConnectionStrategy *self, TnyAccount *account);
	void (*on_disconnect_func) (TnyConnectionStrategy *self, TnyAccount *account);
	void (*set_current_func) (TnyConnectionStrategy *self, TnyAccount *account, TnyFolder *folder);

};

GType tny_connection_strategy_get_type (void);

void tny_connection_strategy_on_connect (TnyConnectionStrategy *self, TnyAccount *account);
void tny_connection_strategy_on_disconnect (TnyConnectionStrategy *self, TnyAccount *account);
void tny_connection_strategy_on_connection_broken (TnyConnectionStrategy *self, TnyAccount *account);
void tny_connection_strategy_set_current (TnyConnectionStrategy *self, TnyAccount *account, TnyFolder *folder);

G_END_DECLS

#endif

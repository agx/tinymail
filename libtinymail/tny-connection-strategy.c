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

#include <config.h>
#include <tny-account.h>
#include <tny-connection-strategy.h>
#include <tny-list.h>

/**
 * tny_connection_strategy_on_connect:
 * @self: A #TnyConnectionStrategy instance
 * @account: a #TnyAccount instance
 *
 * Happens when @account got connected.
 **/
void 
tny_connection_strategy_on_connect (TnyConnectionStrategy *self, TnyAccount *account)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_CONNECTION_STRATEGY (self));
	g_assert (TNY_IS_ACCOUNT (account));

	g_assert (TNY_CONNECTION_STRATEGY_GET_IFACE (self)->on_connect_func != NULL);
#endif

	TNY_CONNECTION_STRATEGY_GET_IFACE (self)->on_connect_func (self, account);

	return;
}

/**
 * tny_connection_strategy_on_disconnect:
 * @self: A #TnyConnectionStrategy instance
 * @account: a #TnyAccount instance
 *
 * Happens when @account got disconnected.
 **/
void 
tny_connection_strategy_on_disconnect (TnyConnectionStrategy *self, TnyAccount *account)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_CONNECTION_STRATEGY (self));
	g_assert (TNY_IS_ACCOUNT (account));

	g_assert (TNY_CONNECTION_STRATEGY_GET_IFACE (self)->on_disconnect_func != NULL);
#endif

	TNY_CONNECTION_STRATEGY_GET_IFACE (self)->on_disconnect_func (self, account);

	return;
}

/**
 * tny_connection_strategy_on_connection_broken:
 * @self: A #TnyConnectionStrategy instance
 * @account: a #TnyAccount instance
 *
 * Happens when @account got connected.
 **/
void 
tny_connection_strategy_on_connection_broken (TnyConnectionStrategy *self, TnyAccount *account)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_CONNECTION_STRATEGY (self));
	g_assert (TNY_IS_ACCOUNT (account));

	g_assert (TNY_CONNECTION_STRATEGY_GET_IFACE (self)->on_connection_broken_func != NULL);
#endif

	TNY_CONNECTION_STRATEGY_GET_IFACE (self)->on_connection_broken_func (self, account);

	return;
}


static void
tny_connection_strategy_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_connection_strategy_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyConnectionStrategyIface),
		  tny_connection_strategy_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL,    /* instance_init */
		  NULL
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyConnectionStrategy", &info, 0);
	}

	return type;
}


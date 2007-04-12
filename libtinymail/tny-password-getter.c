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

#include <config.h>

#ifdef DBC
#include <string.h>
#endif

#include <tny-password-getter.h>

/**
 * tny_password_getter_get_password:
 * @self: a #TnyPasswordGetter object
 * @account: a #TnyAccount object
 * @prompt: the password question
 * @cancel: whether or not the user cancelled
 * 
 * Get the password of @self
 *
 * Return value: the password
 **/
const gchar * 
tny_password_getter_get_password (TnyPasswordGetter *self, TnyAccount *account, const gchar *prompt, gboolean *cancel)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_PASSWORD_GETTER (self));
	g_assert (TNY_IS_ACCOUNT (account));
	g_assert (TNY_PASSWORD_GETTER_GET_IFACE (self)->get_password_func != NULL);
#endif

	return TNY_PASSWORD_GETTER_GET_IFACE (self)->get_password_func (self, account, prompt, cancel);
}

/**
 * tny_password_getter_forget_password:
 * @self: a #TnyPasswordGetter object
 * @account: a #TnyAccount object
 * 
 * Set the password question of @self
 **/
void 
tny_password_getter_forget_password (TnyPasswordGetter *self, TnyAccount *account)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_PASSWORD_GETTER (self));
	g_assert (TNY_IS_ACCOUNT (account));
	g_assert (TNY_PASSWORD_GETTER_GET_IFACE (self)->forget_password_func != NULL);
#endif

	TNY_PASSWORD_GETTER_GET_IFACE (self)->forget_password_func (self, account);

	return;
}



static void
tny_password_getter_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_password_getter_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0)) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyPasswordGetterIface),
		  tny_password_getter_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL,   /* instance_init */
		  NULL
		};
		type = g_type_register_static (G_TYPE_INTERFACE,
			"TnyPasswordGetter", &info, 0);
	}

	return type;
}

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

#include <tny-account-iface.h>


/**
 * tny_account_iface_set_account_store:
 * @self: a #TnyAccountIface object
 * @store: a #TnyAccountStoreIface object
 *
 * Set the parent account store for the account.
 * 
 **/
void
tny_account_iface_set_account_store (TnyAccountIface *self, const TnyAccountStoreIface *store)
{
	TNY_ACCOUNT_IFACE_GET_CLASS (self)->set_account_store_func (self, store);
	return;
}

/**
 * tny_account_iface_get_account_store:
 * @self: a #TnyAccountIface object
 * 
 * Get the parent store for the account.
 * 
 * Return value: the store or NULL if none
 **/
const TnyAccountStoreIface*
tny_account_iface_get_account_store (TnyAccountIface *self)
{
	return TNY_ACCOUNT_IFACE_GET_CLASS (self)->get_account_store_func (self);
}

/**
 * tny_account_iface_get_id:
 * @self: a #TnyAccountIface object
 * 
 * Get an unique id for the account.
 * 
 * Return value: Unique id
 **/
const gchar*
tny_account_iface_get_id (TnyAccountIface *self)
{
	return TNY_ACCOUNT_IFACE_GET_CLASS (self)->get_id_func (self);
}

/**
 * tny_account_iface_set_id:
 * @self: a #TnyAccountIface object
 * 
 * Set the accounts unique id.
 * 
 **/
void 
tny_account_iface_set_id (TnyAccountIface *self, const gchar *id)
{
	TNY_ACCOUNT_IFACE_GET_CLASS (self)->set_id_func (self, id);
	return;
}

/**
 * tny_account_iface_set_forget_pass_func:
 * @self: a #TnyAccountIface object
 * 
 * Set the password that will be called when the password is no longer needed.
 * the function must free the password that was allocated by the function that
 * will return the password (get_pass_func).
 *
 * It's recommended to also memset (str, 0, strlen (str)) the memory.
 * 
 **/
void
tny_account_iface_set_forget_pass_func (TnyAccountIface *self, ForgetPassFunc get_forget_pass_func)
{
	TNY_ACCOUNT_IFACE_GET_CLASS (self)->set_forget_pass_func_func (self, get_forget_pass_func);
	return;
}

/**
 * tny_account_iface_get_forget_pass_func:
 * @self: a #TnyAccountIface object
 * 
 *
 * 
 * Return value: A pointer to the forget-password function
 **/
ForgetPassFunc
tny_account_iface_get_forget_pass_func (TnyAccountIface *self)
{
	return TNY_ACCOUNT_IFACE_GET_CLASS (self)->get_forget_pass_func_func (self);
}


/**
 * tny_account_iface_set_proto:
 * @self: a #TnyAccountIface object
 * @proto: the protocol (ex. "imap")
 * 
 * Set the protocol of an account
 * 
 **/
void
tny_account_iface_set_proto (TnyAccountIface *self, const gchar *proto)
{
	TNY_ACCOUNT_IFACE_GET_CLASS (self)->set_proto_func (self, proto);
	return;
}

/**
 * tny_account_iface_set_user:
 * @self: a #TnyAccountIface object
 * @user: the username
 * 
 * Set the user or login of an account
 * 
 **/
void
tny_account_iface_set_user (TnyAccountIface *self, const gchar *user)
{
	TNY_ACCOUNT_IFACE_GET_CLASS (self)->set_user_func (self, user);
	return;
}

/**
 * tny_account_iface_set_hostname:
 * @self: a #TnyAccountIface object
 * @host: the hostname
 * 
 * Set the hostname of an account
 * 
 **/
void
tny_account_iface_set_hostname (TnyAccountIface *self, const gchar *host)
{
	TNY_ACCOUNT_IFACE_GET_CLASS (self)->set_hostname_func (self, host);
	return;
}

/**
 * tny_account_iface_set_pass_func:
 * @self: a #TnyAccountIface object
 * @get_pass_func: a pointer to the function
 * 
 * Set the function that will be called when the password is needed.
 * The function must resturn a newly allocated string with the password.
 * 
 **/
void
tny_account_iface_set_pass_func (TnyAccountIface *self, GetPassFunc get_pass_func)
{
	TNY_ACCOUNT_IFACE_GET_CLASS (self)->set_pass_func_func (self, get_pass_func);
	return;
}

/**
 * tny_account_iface_get_proto:
 * @self: a #TnyAccountIface object
 * 
 * Get the protocol of an account
 * 
 * Return value: the protocol as a read-only string
 **/
const gchar*
tny_account_iface_get_proto (TnyAccountIface *self)
{
	return TNY_ACCOUNT_IFACE_GET_CLASS (self)->get_proto_func (self);
}

/**
 * tny_account_iface_get_user:
 * @self: a #TnyAccountIface object
 * 
 * Get the user or login of an account
 * 
 * Return value: the user as a read-only string
 **/
const gchar*
tny_account_iface_get_user (TnyAccountIface *self)
{
	return TNY_ACCOUNT_IFACE_GET_CLASS (self)->get_user_func (self);
}

/**
 * tny_account_iface_get_hostname:
 * @self: a #TnyAccountIface object
 * 
 * Get the hostname of an account
 * 
 * Return value: the hostname as a read-only string
 **/
const gchar*
tny_account_iface_get_hostname (TnyAccountIface *self)
{
	return TNY_ACCOUNT_IFACE_GET_CLASS (self)->get_hostname_func (self);
}

/**
 * tny_account_iface_get_pass_func:
 * @self: a #TnyAccountIface object
 * 
 *
 * 
 * Return value: A pointer to the get-password function
 **/
GetPassFunc
tny_account_iface_get_pass_func (TnyAccountIface *self)
{
	return TNY_ACCOUNT_IFACE_GET_CLASS (self)->get_pass_func_func (self);
}


static void
tny_account_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_account_iface_get_type (void)
{
	static GType type = 0;
	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyAccountIfaceClass),
		  tny_account_iface_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyAccountIface", &info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}

	return type;
}



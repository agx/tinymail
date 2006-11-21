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

#include <tny-account.h>


/**
 * tny_account_get_account_type:
 * @self: a #TnyAccount object
 *
 * Get the account type of @self. There are two account types: a store and 
 * transport account type.
 *
 * A store account typically contains folders and messages. Examples are NNTP,
 * IMAP and POP accounts.
 *
 * A transport account has a send method for sending #TnyMsg instances
 * using the transport implemented by the account (for example SMTP).
 *
 * Return value: The account type
 **/
TnyAccountType
tny_account_get_account_type (TnyAccount *self)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_GET_IFACE (self)->get_account_type_func)
		g_critical ("You must implement tny_account_get_account_type\n");
#endif

	return TNY_ACCOUNT_GET_IFACE (self)->get_account_type_func (self);
}


/**
 * tny_account_is_connected:
 * @self: a #TnyAccount object
 *
 * Get the connection status of @self
 *
 * Return value: whether or not the account is connected
 **/
gboolean 
tny_account_is_connected (TnyAccount *self)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_GET_IFACE (self)->is_connected_func)
		g_critical ("You must implement tny_account_is_connected\n");
#endif

	return TNY_ACCOUNT_GET_IFACE (self)->is_connected_func (self);
}


/**
 * tny_account_get_id:
 * @self: a #TnyAccount object
 * 
 * Get the unique id of @self
 *
 * The only certainty you have is that the id is unique in the #TnyAccountStore.
 * The format of the id isn't specified. The implementor of the #TnyAccountStore
 * must set this id using tny_account_set_id.
 * 
 * Return value: Unique id
 **/
const gchar*
tny_account_get_id (TnyAccount *self)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_GET_IFACE (self)->get_id_func)
		g_critical ("You must implement tny_account_get_id\n");
#endif

	return TNY_ACCOUNT_GET_IFACE (self)->get_id_func (self);
}

/**
 * tny_account_set_name:
 * @self: a #TnyAccount object
 * @name: the name
 *
 * Set the account's human readable name
 * 
 **/
void 
tny_account_set_name (TnyAccount *self, const gchar *name)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_GET_IFACE (self)->set_name_func)
		g_critical ("You must implement tny_account_set_name\n");
#endif

	TNY_ACCOUNT_GET_IFACE (self)->set_name_func (self, name);
	return;
}

/**
 * tny_account_set_id:
 * @self: a #TnyAccount object
 * @id: the id
 *
 * Set the unique id of the account. You need to set this property before you 
 * can start using the account. The id must be unique in a #TnyAccountStore and
 * is typically set in the implementation of a #TnyAccountStore.
 * 
 **/
void 
tny_account_set_id (TnyAccount *self, const gchar *id)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_GET_IFACE (self)->set_id_func)
		g_critical ("You must implement tny_account_set_id\n");
#endif

	TNY_ACCOUNT_GET_IFACE (self)->set_id_func (self, id);
	return;
}

/**
 * tny_account_set_forget_pass_func:
 * @self: a #TnyAccount object
 * @forget_pass_func: a pointer to the function
 *
 * Set the function that will be called in case the password was wrong and 
 * therefore can, for example, be forgotten by a password store.
 *
 * You need to set this property before you can start using the account. This
 * property is typically set in the implementation of a #TnyAccountStore.
 * 
 * Also see #TnyForgetPassFunc for more information about the function itself.
 *
 **/
void
tny_account_set_forget_pass_func (TnyAccount *self, TnyForgetPassFunc forget_pass_func)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_GET_IFACE (self)->set_forget_pass_func_func)
		g_critical ("You must implement tny_account_set_forget_pass_func\n");
#endif

	TNY_ACCOUNT_GET_IFACE (self)->set_forget_pass_func_func (self, forget_pass_func);
	return;
}

/**
 * tny_account_get_forget_pass_func:
 * @self: a #TnyAccount object
 * 
 * Return value: A pointer to the forget-password function
 *
 **/
TnyForgetPassFunc
tny_account_get_forget_pass_func (TnyAccount *self)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_GET_IFACE (self)->get_forget_pass_func_func)
		g_critical ("You must implement tny_account_get_forget_pass_func\n");
#endif

	return TNY_ACCOUNT_GET_IFACE (self)->get_forget_pass_func_func (self);
}

/**
 * tny_account_set_url_string:
 * @self: a #TnyAccount object
 * @url_string: the url string (ex. mbox://path)
 *  
 * Set the url string of @self. You don't need to use this for imap and pop
 * where you can use the simplified API (set_proto, set_hostname, etc). This
 * property is typically set in the implementation of a #TnyAccountStore.
 * 
 **/
void
tny_account_set_url_string (TnyAccount *self, const gchar *url_string)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_GET_IFACE (self)->set_url_string_func)
		g_critical ("You must implement tny_account_set_url_string\n");
#endif

	TNY_ACCOUNT_GET_IFACE (self)->set_url_string_func (self, url_string);
	return;
}

/**
 * tny_account_set_proto:
 * @self: a #TnyAccount object
 * @proto: the protocol (ex. "imap")
 * 
 * Set the protocol of @self. You need to set this property before you can start
 * using the account. This property is typically set in the implementation
 * of a #TnyAccountStore.
 * 
 **/
void
tny_account_set_proto (TnyAccount *self, const gchar *proto)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_GET_IFACE (self)->set_proto_func)
		g_critical ("You must implement tny_account_set_proto\n");
#endif

	TNY_ACCOUNT_GET_IFACE (self)->set_proto_func (self, proto);
	return;
}

/**
 * tny_account_set_user:
 * @self: a #TnyAccount object
 * @user: the username
 * 
 * Set the user or login of @self. You need to set this property before you
 * can start using the account. This property is typically set in the
 * implementation of a #TnyAccountStore.
 *
 **/
void
tny_account_set_user (TnyAccount *self, const gchar *user)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_GET_IFACE (self)->set_user_func)
		g_critical ("You must implement tny_account_set_user\n");
#endif

	TNY_ACCOUNT_GET_IFACE (self)->set_user_func (self, user);
	return;
}

/**
 * tny_account_set_hostname:
 * @self: a #TnyAccount object
 * @host: the hostname
 * 
 * Set the hostname of @self. You need to set this property before you can start
 * using the account. This property is typically set in the implementation of a
 * #TnyAccountStore.
 *
 **/
void
tny_account_set_hostname (TnyAccount *self, const gchar *host)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_GET_IFACE (self)->set_hostname_func)
		g_critical ("You must implement tny_account_set_hostname\n");
#endif

	TNY_ACCOUNT_GET_IFACE (self)->set_hostname_func (self, host);
	return;
}


/**
 * tny_account_set_pass_func:
 * @self: a #TnyAccount object
 * @get_pass_func: a pointer to the function
 * 
 * Set the function that will be called when the password is needed. The
 * function should return the password for a specific account. The password 
 * itself is usually stored in a secured password store.
 *
 * You need to set this property before you can start using the account. This
 * property is typically set in the implementation of a #TnyAccountStore.
 * 
 * Also see #TnyGetPassFunc for more information about the function itself. 
 *
 **/
void
tny_account_set_pass_func (TnyAccount *self, TnyGetPassFunc get_pass_func)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_GET_IFACE (self)->set_pass_func_func)
		g_critical ("You must implement tny_account_set_pass_func\n");
#endif

	TNY_ACCOUNT_GET_IFACE (self)->set_pass_func_func (self, get_pass_func);
	return;
}

/**
 * tny_account_get_proto:
 * @self: a #TnyAccount object
 * 
 * Get the protocol of @self. The returned value should not be freed.
 * 
 * Return value: the protocol as a read-only string
 *
 **/
const gchar*
tny_account_get_proto (TnyAccount *self)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_GET_IFACE (self)->get_proto_func)
		g_critical ("You must implement tny_account_get_proto\n");
#endif

	return TNY_ACCOUNT_GET_IFACE (self)->get_proto_func (self);
}



/**
 * tny_account_get_url_string:
 * @self: a #TnyAccount object
 * 
 * Get the url string of @self. The returned value should not be freed.
 * 
 * Return value: the url string as a read-only string
 *
 **/
const gchar*
tny_account_get_url_string (TnyAccount *self)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_GET_IFACE (self)->get_url_string_func)
		g_critical ("You must implement tny_account_get_url_string\n");
#endif

	return TNY_ACCOUNT_GET_IFACE (self)->get_url_string_func (self);
}

/**
 * tny_account_get_user:
 * @self: a #TnyAccount object
 * 
 * Get the user or login of @self. The returned value should not be freed.
 * 
 * Return value: the user as a read-only string
 *
 **/
const gchar*
tny_account_get_user (TnyAccount *self)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_GET_IFACE (self)->get_user_func)
		g_critical ("You must implement tny_account_get_user\n");
#endif

	return TNY_ACCOUNT_GET_IFACE (self)->get_user_func (self);
}

/**
 * tny_account_get_name:
 * @self: a #TnyAccount object
 * 
 * Get the human readable name of @self. The returned value should not 
 * be freed.
 * 
 * Return value: the human readable name as a read-only string
 *
 **/
const gchar*
tny_account_get_name (TnyAccount *self)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_GET_IFACE (self)->get_name_func)
		g_critical ("You must implement tny_account_get_name\n");
#endif

	return TNY_ACCOUNT_GET_IFACE (self)->get_name_func (self);
}

/**
 * tny_account_get_hostname:
 * @self: a #TnyAccount object
 * 
 * Get the hostname of @self. The returned value should not be freed.
 * 
 * Return value: the hostname as a read-only string
 *
 **/
const gchar*
tny_account_get_hostname (TnyAccount *self)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_GET_IFACE (self)->get_hostname_func)
		g_critical ("You must implement tny_account_get_hostname\n");
#endif

	return TNY_ACCOUNT_GET_IFACE (self)->get_hostname_func (self);
}

/**
 * tny_account_get_pass_func:
 * @self: a #TnyAccount object
 * 
 * Return value: A pointer to the get-password function
 *
 **/
TnyGetPassFunc
tny_account_get_pass_func (TnyAccount *self)
{
#ifdef DEBUG
	if (!TNY_ACCOUNT_GET_IFACE (self)->get_pass_func_func)
		g_critical ("You must implement tny_account_get_pass_func\n");
#endif

	return TNY_ACCOUNT_GET_IFACE (self)->get_pass_func_func (self);
}


static void
tny_account_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_account_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyAccountIface),
		  tny_account_base_init,   /* base_init */
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
			"TnyAccount", &info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}

	return type;
}


GType
tny_account_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TNY_ACCOUNT_TYPE_STORE, "TNY_ACCOUNT_TYPE_STORE", "store" },
      { TNY_ACCOUNT_TYPE_TRANSPORT, "TNY_ACCOUNT_TYPE_TRANSPORT", "transport" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyAccountType", values);
  }
  return etype;
}


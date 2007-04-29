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

#include <tny-account.h>

/**
 * tny_account_matches_url_string:
 * @self: a #TnyAccount object
 * @url_string: the url-string of the account to find
 *
 * Find out whether the account matches a certain url_string.
 *
 * Implementors: Be forgiving about things like passwords in the url_string:
 * while matching the folder, password and message-id pieces are insignificant.
 *
 * An example url_string can be imap://user:password@server/INBOX/005. Only 
 * "imap://user@server" is significant when searching. Also take a look at 
 * RFC 1808 for more information on url_string formatting.
 *
 * This method must be usable with and will be used for 
 * tny_account_store_find_account.
 *
 * Return value: whether or not @self matches with @url_string.
 **/
gboolean 
tny_account_matches_url_string (TnyAccount *self, const gchar *url_string)
{
	gboolean retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (url_string);
	g_assert (strlen (url_string) > 0);
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->matches_url_string_func != NULL);
#endif

	retval = TNY_ACCOUNT_GET_IFACE (self)->matches_url_string_func (self, url_string);

#ifdef DBC /* ensure*/
#endif

	return retval;
}

/**
 * tny_account_cancel:
 * @self: a #TnyAccount object
 *
 * Forcefully cancels the current operation that is happening on @self.
 * 
 **/
void 
tny_account_cancel (TnyAccount *self)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->cancel_func != NULL);
#endif

	TNY_ACCOUNT_GET_IFACE (self)->cancel_func (self);

#ifdef BDC /* ensure */
#endif

	return;
}

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
	TnyAccountType retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->get_account_type_func != NULL);
#endif

	retval = TNY_ACCOUNT_GET_IFACE (self)->get_account_type_func (self);

#ifdef DBC /* ensure */
#endif

	return retval;
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
	gboolean retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->is_connected_func != NULL);
#endif

	retval = TNY_ACCOUNT_GET_IFACE (self)->is_connected_func (self);

#ifdef DBC /* ensure */
#endif

	return retval;
}


/**
 * tny_account_get_id:
 * @self: a #TnyAccount object
 * 
 * Get the unique id of @self
 *
 * A certainty you have about this property is that the id is unique in the 
 * #TnyAccountStore. It doesn't have to be unique in the entire application.
 *
 * The format of the id isn't specified. The implementor of the #TnyAccountStore
 * must set this id using tny_account_set_id.
 * 
 * Return value: Unique id
 **/
const gchar*
tny_account_get_id (TnyAccount *self)
{
	const gchar *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->get_id_func != NULL);
#endif

	retval = TNY_ACCOUNT_GET_IFACE (self)->get_id_func (self);

#ifdef DBC /* ensure */
	g_assert (retval);
	g_assert (strlen (retval) > 0);
#endif

	return retval;
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
#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (name);
	g_assert (strlen (name) > 0);
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->set_name_func != NULL);
#endif

	TNY_ACCOUNT_GET_IFACE (self)->set_name_func (self, name);

#ifdef DBC /* require */
	g_assert (!strcmp (tny_account_get_name (self), name));
#endif

	return;
}

/**
 * tny_account_set_mech:
 * @self: a #TnyAccount object
 * @mech: the authentication mechanism
 * 
 * Set the account's authentication mechanism. For example in case of plain
 * you use "PLAIN" here. For anonymous you use "ANONYMOUS". This last one 
 * will for example result in a AUTHENTICATE ANONYMOUS request, as specified
 * in RFC 2245, on for example IMAP servers.
 * 
 **/
void
tny_account_set_mech (TnyAccount *self, const gchar *mech)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (mech);
	g_assert (strlen (mech) > 0);
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->set_mech_func != NULL);
#endif

	TNY_ACCOUNT_GET_IFACE (self)->set_mech_func (self, mech);

#ifdef DBC /* require */
	g_assert (!strcmp (tny_account_get_mech (self), mech));
#endif

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
#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (id);
	g_assert (strlen (id) > 0);
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->set_id_func != NULL);
#endif

	TNY_ACCOUNT_GET_IFACE (self)->set_id_func (self, id);

#ifdef DBC /* require */
	g_assert (!strcmp (tny_account_get_id (self), id));
#endif

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
 * Example:
 * <informalexample><programlisting>
 * static GHashTable *passwords;
 * static void
 * per_account_forget_pass_func (TnyAccount *account)
 * {
 *    TnyPlatformFactory *platfact = tny_my_platform_factory_get_instance ();
 *    TnyPasswordGetter *pwdgetter;
 *    pwdgetter = tny_platform_factory_new_password_getter (platfact);
 *    tny_password_getter_forget_password (pwdgetter, tny_account_get_id (account));
 *    g_object_unref (G_OBJECT (pwdgetter));
 *    return;
 * }
 * static void
 * tny_my_account_store_get_accounts (TnyAccountStore *self, TnyList *list, TnyGetAccountsRequestType types)
 * {
 *     TnyAccount *account = ...
 *     ...
 *     tny_account_set_forget_pass_func (account, per_account_forget_pass_func);
 *     tny_account_set_pass_func (account, per_account_get_pass_func);
 *     tny_list_prepend (list, (GObject*)account);
 *     g_object_unref (G_OBJECT (account));
 *     ...
 * }
 * </programlisting></informalexample>
 **/
void
tny_account_set_forget_pass_func (TnyAccount *self, TnyForgetPassFunc forget_pass_func)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->set_forget_pass_func_func != NULL);
#endif

	TNY_ACCOUNT_GET_IFACE (self)->set_forget_pass_func_func (self, forget_pass_func);

#ifdef DBC /* ensure */
	g_assert (tny_account_get_forget_pass_func (self) == forget_pass_func);
#endif

	return;
}

/**
 * tny_account_get_forget_pass_func:
 * @self: a #TnyAccount object
 * 
 * Get a pointer to the forget-password function
 *
 * Return value: A pointer to the forget-password function
 **/
TnyForgetPassFunc
tny_account_get_forget_pass_func (TnyAccount *self)
{
	TnyForgetPassFunc retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->get_forget_pass_func_func != NULL);
#endif

	retval = TNY_ACCOUNT_GET_IFACE (self)->get_forget_pass_func_func (self);

#ifdef DBC /* ensure */
#endif

	return retval;
}

/**
 * tny_account_set_url_string:
 * @self: a #TnyAccount object
 * @url_string: the url string (ex. mbox://path)
 *  
 * Set the url string of @self (RFC 1808). You don't need to use this for imap
 * and pop where you can use the simplified API (set_proto, set_hostname, etc).
 * This property is typically set in the implementation of a #TnyAccountStore.
 * 
 * For example the url_string for an SMTP account that uses SSL with authentication
 * type PLAIN: smtp://user;auth=PLAIN@smtp.server.com/;use_ssl=always
 * 
 * Don't forget to set the name, type and proto setting of the account too.
 **/
void
tny_account_set_url_string (TnyAccount *self, const gchar *url_string)
{
#ifdef DBC /* require */
	gchar *ptr1, *ptr2;
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (url_string);
	g_assert (strlen (url_string) > 0);
	g_assert (strstr (url_string, "://") != NULL);
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->set_url_string_func != NULL);
#endif

	TNY_ACCOUNT_GET_IFACE (self)->set_url_string_func (self, url_string);

#ifdef DBC /* ensure */

	/* TNY TODO: It's possible that tny_account_get_url_string strips the
	 * password. It would be interesting to have a contract check that 
	 * deals with this. */

	/* TNY TODO: check this DBC implementation for correctness: */
	ptr1 = tny_account_get_url_string (self);
	ptr2 = strchr (ptr1, '@');
	ptr1 = strchr (url_string, '@');
	g_assert (!strcmp (ptr1, ptr2));
#endif

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
#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (proto);
	g_assert (strlen (proto) > 0);
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->set_proto_func != NULL);
#endif

	TNY_ACCOUNT_GET_IFACE (self)->set_proto_func (self, proto);

#ifdef DBC /* ensure */
	g_assert (!strcmp (tny_account_get_proto (self), proto));
#endif

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
#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (user);
	g_assert (strlen (user) > 0);
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->set_user_func != NULL);
#endif

	TNY_ACCOUNT_GET_IFACE (self)->set_user_func (self, user);

#ifdef DBC /* ensure */
	g_assert (!strcmp (tny_account_get_user (self), user));
#endif

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
#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (host);
	g_assert (strlen (host) > 0);
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->set_hostname_func != NULL);
#endif

	TNY_ACCOUNT_GET_IFACE (self)->set_hostname_func (self, host);

#ifdef DBC /* ensure */
	g_assert (!strcmp (tny_account_get_hostname (self), host));
#endif

	return;
}



/**
 * tny_account_set_port:
 * @self: a #TnyAccount object
 * @port: the port to connect to on the hostname
 * 
 * Set the port of @self. If you don't set this property, the default port for
 * the protocol will be used (for example 143 and 993 for IMAP and 110 for POP3).
 *
 **/
void
tny_account_set_port (TnyAccount *self, guint port)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (port <= 65536);
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->set_port_func != NULL);
#endif

	TNY_ACCOUNT_GET_IFACE (self)->set_port_func (self, port);

#ifdef DBC /* ensure */
	g_assert (tny_account_get_port (self) == port);
#endif

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
 * Example:
 * <informalexample><programlisting>
 * static GHashTable *passwords;
 * static gchar* 
 * per_account_get_pass_func (TnyAccount *account, const gchar *prompt, gboolean *cancel)
 * {
 *    TnyPlatformFactory *platfact = tny_my_platform_factory_get_instance ();
 *    TnyPasswordGetter *pwdgetter;
 *    gchar *retval;
 *    pwdgetter = tny_platform_factory_new_password_getter (platfact);
 *    retval = (gchar*) tny_password_getter_get_password (pwdgetter, 
 *       tny_account_get_id (account), prompt, cancel);
 *    g_object_unref (G_OBJECT (pwdgetter));
 *    return retval;
 * }
 * static void
 * tny_my_account_store_get_accounts (TnyAccountStore *self, TnyList *list, TnyGetAccountsRequestType types)
 * {
 *     TnyAccount *account = ...
 *     ...
 *     tny_account_set_forget_pass_func (account, per_account_forget_pass_func);
 *     tny_account_set_pass_func (account, per_account_get_pass_func);
 *     tny_list_prepend (list, (GObject*)account);
 *     g_object_unref (G_OBJECT (account));
 *     ...
 * }
 * </programlisting></informalexample>
 **/
void
tny_account_set_pass_func (TnyAccount *self, TnyGetPassFunc get_pass_func)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->set_pass_func_func != NULL);
#endif

	TNY_ACCOUNT_GET_IFACE (self)->set_pass_func_func (self, get_pass_func);

#ifdef DBC /* ensure */
	g_assert (tny_account_get_pass_func (self) == get_pass_func);
#endif

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
	const gchar *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->get_proto_func != NULL);
#endif

	retval = TNY_ACCOUNT_GET_IFACE (self)->get_proto_func (self);

#ifdef DBC /* ensure */
#endif

	return retval;
}



/**
 * tny_account_get_url_string:
 * @self: a #TnyAccount object
 * 
 * Get the url string of @self or NULL if it's impossible to determine the url
 * string of @self. If not NULL, the returned value must be freed.
 * 
 * The url string is specified in RFC 1808 and looks for example like this:
 * imap://user@hostname. Note that it doesn't necessarily contain the password 
 * of the IMAP account.
 *
 * Return value: the url string or NULL.
 **/
gchar*
tny_account_get_url_string (TnyAccount *self)
{
	gchar *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->get_url_string_func != NULL);
#endif

	retval = TNY_ACCOUNT_GET_IFACE (self)->get_url_string_func (self);

#ifdef DBC /* ensure */
	if (retval)
		g_assert (strstr (retval, "://") != NULL);
#endif

	return retval;

}

/**
 * tny_account_get_user:
 * @self: a #TnyAccount object
 * 
 * Get the user or login of @self. The returned value should not be freed. The
 * returned value van be NULL in case of no user.
 * 
 * Return value: the user as a read-only string
 *
 **/
const gchar*
tny_account_get_user (TnyAccount *self)
{
	const gchar *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->get_user_func != NULL);
#endif

	retval = TNY_ACCOUNT_GET_IFACE (self)->get_user_func (self);

#ifdef DBC /* ensure */
#endif

	return retval;
}

/**
 * tny_account_get_name:
 * @self: a #TnyAccount object
 * 
 * Get the human readable name of @self. The returned value should not 
 * be freed. The returned value van be NULL in case of no human reabable name.
 * 
 * Return value: the human readable name as a read-only string
 *
 **/
const gchar*
tny_account_get_name (TnyAccount *self)
{
	const gchar *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->get_name_func != NULL);
#endif

	retval = TNY_ACCOUNT_GET_IFACE (self)->get_name_func (self);

#ifdef DBC /* ensure */
#endif

	return retval;
}


/**
 * tny_account_get_mech:
 * @self: a #TnyAccount object
 * 
 * Get the authentication mechanism for this account. Default is "PLAIN". The
 * returned value can be NULL, in which case a undefined default is used.
 * 
 * Return value: the authentication mechanism as a read-only string
 *
 **/
const gchar*
tny_account_get_mech (TnyAccount *self)
{
	const gchar *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->get_mech_func != NULL);
#endif

	retval = TNY_ACCOUNT_GET_IFACE (self)->get_mech_func (self);

#ifdef DBC /* ensure */
#endif

	return retval;
}


/**
 * tny_account_get_hostname:
 * @self: a #TnyAccount object
 * 
 * Get the hostname of @self. The returned value should not be freed. The 
 * returned value can be NULL, in which case no hostname is set (for example 
 * for a local account).
 * 
 * Return value: the hostname as a read-only string
 *
 **/
const gchar*
tny_account_get_hostname (TnyAccount *self)
{
	const gchar *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->get_hostname_func != NULL);
#endif

	retval = TNY_ACCOUNT_GET_IFACE (self)->get_hostname_func (self);

#ifdef DBC /* ensure */
#endif

	return retval;
}


/**
 * tny_account_get_port:
 * @self: a #TnyAccount object
 * 
 * Get the port of @self. 
 * 
 * Return value: the port
 **/
guint
tny_account_get_port (TnyAccount *self)
{
	guint retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->get_port_func != NULL);
#endif

	retval = TNY_ACCOUNT_GET_IFACE (self)->get_port_func (self);

#ifdef DBC /* ensure */
	g_assert (retval <= 65536);
#endif

	return retval;
}
/**
 * tny_account_get_pass_func:
 * @self: a #TnyAccount object
 * 
 * Get a pointer to the get-password function
 *
 * Return value: A pointer to the get-password function
 **/
TnyGetPassFunc
tny_account_get_pass_func (TnyAccount *self)
{
	TnyGetPassFunc retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->get_port_func != NULL);
#endif

	retval = TNY_ACCOUNT_GET_IFACE (self)->get_pass_func_func (self);

#ifdef DBC /* ensure */
#endif

	return retval;
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


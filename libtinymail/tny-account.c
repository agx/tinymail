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

#include <tny-status.h>


#ifdef DBC
#include <string.h>
#endif

#include <tny-account.h>

guint tny_account_signals [TNY_ACCOUNT_LAST_SIGNAL];


/**
 * tny_account_start_operation:
 * @self: a #TnyAccount object
 * @mdomain: the domain of the #TnyStatus instances that will happen in @status_callback
 * @mcode: the code of the #TnyStatus instances that will happen in @status_callback
 * @status_callback: status callback handler
 * @status_user_data: the user-data to give to the @status_callback
 *
 * Start an operation. This only works with methods that don't end with _async.
 **/
void 
tny_account_start_operation (TnyAccount *self, TnyStatusDomain domain, TnyStatusCode code, TnyStatusCallback status_callback, gpointer status_user_data)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->start_operation_func != NULL);
#endif

	TNY_ACCOUNT_GET_IFACE (self)->start_operation_func (self, domain, code, status_callback, status_user_data);

#ifdef DBC /* ensure*/
#endif

	return;
}

/**
 * tny_account_stop_operation:
 * @self: a #TnyAccount object
 * @canceled: NULL or byref whether the operation got canceled
 *
 * Stop the current operation. This only works with methods that don't end 
 * with _async.
 **/
void 
tny_account_stop_operation (TnyAccount *self, gboolean *canceled)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->stop_operation_func != NULL);
#endif

	TNY_ACCOUNT_GET_IFACE (self)->stop_operation_func (self, canceled);

#ifdef DBC /* ensure*/
#endif

	return;
}

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
 * RFC 1808 and RFC 4467 for more information on url_string formatting.
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
 * Try to cancel the current operation that is happening. This API, though,
 * guarantees nothing about any cancelations.
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
 * tny_account_get_connection_status:
 * @self: a #TnyAccount object
 *
 * Get the connection status of @self
 *
 * Return value: the status of the connection
 **/
TnyConnectionStatus 
tny_account_get_connection_status (TnyAccount *self)
{
	TnyConnectionStatus retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->get_connection_status_func != NULL);
#endif

	retval = TNY_ACCOUNT_GET_IFACE (self)->get_connection_status_func (self);

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

 /* TODO: It would be nice to have a tny_account_set_secure_auth_mech_from_enum() 
  * convenience function, with common clearly-defined and documented choices. */
 /* TODO: Document how to discover the possible values, by getting the 
  * capabilities from the server. */
 /* TODO: Document _all_ of these possible values in terms of what kind of 
  * authentication behaviour they specify, and which are even possible for 
  * IMAP, POP, or SMTP. */
  
/**
 * tny_account_set_secure_auth_mech:
 * @self: a #TnyAccount object
 * @mech: the authentication mechanism
 * 
 * Set the account's secure authentication mechanism. The possible values depend on 
 * the capabilities of the server, but here are some possible values:
 * - "ANONYMOUS": Results in an AUTHENTICATE ANONYMOUS request, as specified
 * in <ulink url="http://www.faqs.org/rfcs/rfc2245.html">RFC 2245</ulink>.
 * - "CRAM-MD5": Challenge-Response Authentication Mechanism, as specified in 
 * <ulink url="http://www.faqs.org/rfcs/rfc2195.html">RFC 2195</ulink>.
 * - "DIGEST-MD5": Digest Authentication, as specified in 
 * <ulink url="http://www.faqs.org/rfcs/rfc2831.html">RFC 2831</ulink>.
 * - GSSAPI: Generic Security Service Application Program Interface, as 
 * specified in <ulink url="http://www.faqs.org/rfcs/rfc2222.html">RFC 2222</ulink> 
 * and <ulink url="http://www.faqs.org/rfcs/rfc2078.html.html">RFC 2078</ulink>.
 * - Kerberos 4: as specified in 
 * <ulink url="http://www.faqs.org/rfcs/rfc2222.htmll">RFC 2222</ulink>.
 * - "NTLM / SPA": Secure Password Authentication, as used by Outlook Express.
 * - "Login"
 * - "PLAIN"
 * - "POP before SMTP".
 * 
 * Other relevant standards:
 * - <ulink url="http://www.faqs.org/rfcs/rfc1731.html">RFC 1731 - IMAP4 Authentication Mechanisms</ulink> 
 * 
 **/
void
tny_account_set_secure_auth_mech (TnyAccount *self, const gchar *mech)
{
#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (mech);
	g_assert (strlen (mech) > 0);
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->set_secure_auth_mech_func != NULL);
#endif

	TNY_ACCOUNT_GET_IFACE (self)->set_secure_auth_mech_func (self, mech);

#ifdef DBC /* require */
	g_assert (!strcmp (tny_account_get_secure_auth_mech (self), mech));
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
 * type PLAIN: smtp://user;auth=PLAIN@smtp.server.com/;use_ssl=wrapped
 * 
 * Don't forget to set the name, type and proto setting of the account too.
 **/
void
tny_account_set_url_string (TnyAccount *self, const gchar *url_string)
{
#ifdef DBC /* require */
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
 * property is typically set in the implementation of a #TnyAccountStore. Set
 * this property as the last of all properties that you will set to an account
 * in the #TnyAccountStore.
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
 * tny_account_get_secure_auth_mech:
 * @self: a #TnyAccount object
 * 
 * Get the secure authentication mechanism for this account. Default is "PLAIN". The
 * returned value can be NULL, in which case a undefined default is used.
 * 
 * Return value: the authentication mechanism as a read-only string
 *
 **/
const gchar*
tny_account_get_secure_auth_mech (TnyAccount *self)
{
	const gchar *retval;

#ifdef DBC /* require */
	g_assert (TNY_IS_ACCOUNT (self));
	g_assert (TNY_ACCOUNT_GET_IFACE (self)->get_secure_auth_mech_func != NULL);
#endif

	retval = TNY_ACCOUNT_GET_IFACE (self)->get_secure_auth_mech_func (self);

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

/**
 * TnyAccount::connection-status-changed
 * @self: the object on which the signal is emitted
 * @status: the #TnyConnectionStatus
 * @user_data: user data set when the signal handler was connected.
 *
 * Emitted when the connection status of an account changes.
 **/
		tny_account_signals[TNY_ACCOUNT_CONNECTION_STATUS_CHANGED] =
		   g_signal_new ("connection_status_changed",
			TNY_TYPE_ACCOUNT,
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (TnyAccountIface, connection_status_changed),
			NULL, NULL,
			g_cclosure_marshal_VOID__INT, 
			G_TYPE_NONE, 1, G_TYPE_INT);

		initialized = TRUE;
	}
}

/**
 * tny_account_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
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

/**
 * tny_account_type_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
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



/**
 * tny_account_signal_type_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
GType
tny_account_signal_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TNY_ACCOUNT_CONNECTION_STATUS_CHANGED, "TNY_ACCOUNT_CONNECTION_STATUS_CHANGED", "connection_status" },
      { TNY_ACCOUNT_LAST_SIGNAL, "TNY_ACCOUNT_LAST_SIGNAL", "last_signal" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyAccountSignal", values);
  }
  return etype;
}

/**
 * tny_connection_status_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
GType
tny_connection_status_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TNY_CONNECTION_STATUS_DISCONNECTED, "TNY_CONNECTION_STATUS_DISCONNECTED", "disconnected" },
      { TNY_CONNECTION_STATUS_DISCONNECTED_BROKEN, "TNY_CONNECTION_STATUS_DISCONNECTED_BROKEN", "disconnected-broken" },
      { TNY_CONNECTION_STATUS_CONNECTED_BROKEN, "TNY_CONNECTION_STATUS_CONNECTED_BROKEN", "connected-broken" },
      { TNY_CONNECTION_STATUS_CONNECTED, "TNY_CONNECTION_STATUS_CONNECTED", "connected" },
      { TNY_CONNECTION_STATUS_RECONNECTING, "TNY_CONNECTION_STATUS_RECONNECTING", "reconnecting" },
      { TNY_CONNECTION_STATUS_INIT, "TNY_CONNECTION_STATUS_INIT", "init" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyConnectionStatus", values);
  }
  return etype;
}



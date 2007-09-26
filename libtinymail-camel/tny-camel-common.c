/* libtinymail-camel - The Tiny Mail base library for Camel
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
#include <glib/gi18n-lib.h>
#include <string.h>

#include <tny-camel-account.h>
#include <tny-session-camel.h>

#include "tny-session-camel-priv.h"
#include "tny-camel-common-priv.h"
#include "tny-camel-account-priv.h"

#include <tny-folder-store-query.h>

static void remove_quotes (gchar *buffer);
static gchar **split_recipients (gchar *buffer);


/* TODOL Rename to tny_camel_session_check_operation. */
/** _tny_session_check_operation:
 * @session: A camel session.
 * @err: A pointer to a GError*, which will be set if the session is not ready. 
 * This should be freed with g_error_free().
 * @domain The error domain for the GError, if necessary.
 * @code The error code for the GError if necessary.
 * @result: TRUE if the session is ready to be used.
 *
 * Check that the session is ready to be used, and create a GError with the specified 
 * domain and code if the session is not ready.
 **/
gboolean 
_tny_session_check_operation (TnySessionCamel *session, TnyAccount *account, GError **err, GQuark domain, gint code)
{
	TnySessionCamel *in = (TnySessionCamel *) session;
	gboolean is_connecting = FALSE;

	if (in == NULL || !CAMEL_IS_SESSION (in))
	{
		g_set_error (err, domain, code,
			"Operating can't continue: account not ready. "
			"This problem indicates a bug in the software.");
		return FALSE;
	}

	if (account && TNY_IS_CAMEL_ACCOUNT (account))
	{
		TnyCamelAccountPriv *apriv = TNY_CAMEL_ACCOUNT_GET_PRIVATE (account);
		is_connecting = apriv->is_connecting;
	} else 
		is_connecting = in->priv->is_connecting;

	if (is_connecting)
	{
		g_set_error (err, domain, code,
			"Operating can't continue: connecting in progress. "
			"This problem indicates a bug in the software.");
		return FALSE;
	}

	in->priv->is_inuse = TRUE; /* Not yet used */

	return TRUE;
}

void 
_tny_session_stop_operation (TnySessionCamel *session)
{
	TnySessionCamel *in = (TnySessionCamel *) session;
	in->priv->is_inuse = FALSE;
}


gboolean 
_tny_folder_store_query_passes (TnyFolderStoreQuery *query, CamelFolderInfo *finfo)
{
	gboolean retval = FALSE;

	if (query && (tny_list_get_length (tny_folder_store_query_get_items (query)) > 0))
	{
		/* TNY TODO: Make this cope with AND constructs */
		TnyList *items = tny_folder_store_query_get_items (query);
		TnyIterator *iterator;
		iterator = tny_list_create_iterator (items);

		while (!tny_iterator_is_done (iterator))
		{
			TnyFolderStoreQueryItem *item = (TnyFolderStoreQueryItem*) tny_iterator_get_current (iterator);
			TnyFolderStoreQueryOption options = tny_folder_store_query_item_get_options (item);

			if ((options & TNY_FOLDER_STORE_QUERY_OPTION_SUBSCRIBED) &&
				finfo->flags & CAMEL_FOLDER_SUBSCRIBED)
					retval = TRUE;

			if ((options & TNY_FOLDER_STORE_QUERY_OPTION_UNSUBSCRIBED) &&
				!(finfo->flags & CAMEL_FOLDER_SUBSCRIBED))
					retval = TRUE;

			if (options & TNY_FOLDER_STORE_QUERY_OPTION_PATTERN_IS_REGEX)
			{
				regex_t *regex = (regex_t *) tny_folder_store_query_item_get_regex (item);

				if (regex && (options & TNY_FOLDER_STORE_QUERY_OPTION_MATCH_ON_NAME)) {
					if (regexec (regex, finfo->name, 0, NULL, 0) == 0)
						retval = TRUE;
				}

				if (regex && (options & TNY_FOLDER_STORE_QUERY_OPTION_MATCH_ON_ID)) {
					if (regexec (regex, finfo->full_name, 0, NULL, 0) == 0)
						retval = TRUE;
				}

			} else {
				const gchar *pattern = tny_folder_store_query_item_get_pattern (item);

				if (pattern && (options & TNY_FOLDER_STORE_QUERY_OPTION_MATCH_ON_NAME)) 
				{
					if (options & TNY_FOLDER_STORE_QUERY_OPTION_PATTERN_IS_CASE_INSENSITIVE)
					{
						if (g_strcasecmp (finfo->name, pattern) == 0)
							retval = TRUE;
					} else {
						if (strcmp (finfo->name, pattern) == 0)
							retval = TRUE;
					}
				}

				if (pattern && (options & TNY_FOLDER_STORE_QUERY_OPTION_MATCH_ON_ID)) 
				{
					if (options & TNY_FOLDER_STORE_QUERY_OPTION_PATTERN_IS_CASE_INSENSITIVE)
					{
						if (g_strcasecmp (finfo->full_name, pattern) == 0)
							retval = TRUE;
					} else {
						if (strcmp (finfo->full_name, pattern) == 0)
							retval = TRUE;
					}
				}
			}

			g_object_unref (item);
			tny_iterator_next (iterator);
		}
		 
		g_object_unref (iterator);
		g_object_unref (items);
	} else
		retval = TRUE;

	return retval;
}

static void
remove_quotes (gchar *buffer)
{
	gchar *tmp = buffer;
	gboolean first_is_quote = FALSE;

	if (buffer == NULL)
		return;

	/* First we remove the first quote */
	first_is_quote = (buffer[0] == '\"');
	while (*tmp != '\0') {
		if ((tmp[1] == '\"') && (tmp[2] == '\0'))
			tmp[1] = '\0';
		if (first_is_quote)
			tmp[0] = tmp[1];
		tmp++;
	}

	if ((tmp > buffer) && (*(tmp-1) == '\"'))
		*(tmp-1) = '\0';

}

static gchar **
split_recipients (gchar *buffer)
{
	gchar *tmp, *start;
	gboolean is_quoted = FALSE;
	GPtrArray *array = g_ptr_array_new ();

	start = tmp = buffer;

	if (buffer == NULL) {
		g_ptr_array_add (array, NULL);
		return (gchar **) g_ptr_array_free (array, FALSE);
	}

	while (*tmp != '\0') {
		if (*tmp == '\"')
			is_quoted = !is_quoted;
		if (*tmp == '\\')
			tmp++;
		if ((!is_quoted) && ((*tmp == ',') || (*tmp == ';'))) {
			gchar *part;
			part = g_strndup (start, tmp - start);
			g_ptr_array_add (array, part);
			start = tmp+1;
		}
		
		tmp++;
	}

	if (start != tmp)
		g_ptr_array_add (array, g_strdup (start));
	
	g_ptr_array_add (array, NULL);
	return (gchar **) g_ptr_array_free (array, FALSE);
}


void
_string_to_camel_inet_addr (gchar *tok, CamelInternetAddress *target)
{
	char *stfnd = NULL;
	
	stfnd = strchr (tok, '<');
	
	if (G_LIKELY (stfnd))
	{
		char *name = (char*)tok, *lname = NULL;
		char *email = stfnd+1, *gtfnd = NULL;

		lname = stfnd-1;

		gtfnd = strchr (stfnd, '>');
	
		if (G_UNLIKELY (!gtfnd))
		{
			g_warning (_("Invalid e-mail address in field"));
			return;
		}
	
		*stfnd = '\0';
		*gtfnd = '\0';
	
		if (G_LIKELY (*name == ' '))
			name++;
	
		if (G_LIKELY (*lname == ' '))
			*lname-- = '\0';
		remove_quotes (name);
		camel_internet_address_add (target, name, email);
	} else {
		
		char *name = (char*)tok;
		char *lname = name;

		lname += (strlen (name)-1);

		if (G_LIKELY (*name == ' '))
			name++;
	
		if (G_LIKELY (*lname == ' '))
			*lname-- = '\0';
		camel_internet_address_add (target, NULL, name);
	}
}


void
_foreach_email_add_to_inet_addr (const gchar *emails, CamelInternetAddress *target)
{
	char *dup = g_strdup (emails);
	gchar **parts, **current;

	if (!emails)
		return;

	parts = split_recipients (dup);
	current = parts;

	while (G_LIKELY (*current != NULL))
	{
		
		_string_to_camel_inet_addr (*current, target);

		current++;
	}

	g_strfreev (parts);
	g_free (dup);

	return;
}

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

#include "tny-camel-common-priv.h"
#include <tny-folder-store-query.h>

gboolean 
_tny_folder_store_query_passes (TnyFolderStoreQuery *query, CamelFolderInfo *finfo)
{
    	gboolean retval = FALSE;
    
	if (query)
	{
		TnyListIface *items = tny_folder_store_query_get_items (query);
		TnyIteratorIface *iterator;
		iterator = tny_list_iface_create_iterator (items);
		 
		 
		while (!tny_iterator_iface_is_done (iterator))
		{
			TnyFolderStoreQueryItem *item = (TnyFolderStoreQueryItem*) tny_iterator_iface_current (iterator);
			TnyFolderStoreQueryOption options = tny_folder_store_query_item_get_options (item);
			regex_t *regex = tny_folder_store_query_item_get_regex (item);

			if ((options & TNY_FOLDER_STORE_QUERY_OPTION_SUBSCRIBED) &&
			    finfo->flags & CAMEL_FOLDER_SUBSCRIBED)
				retval = TRUE;

			if ((options & TNY_FOLDER_STORE_QUERY_OPTION_UNSUBSCRIBED) &&
			    !(finfo->flags & CAMEL_FOLDER_SUBSCRIBED))
				retval = TRUE;

			if (regex && options & TNY_FOLDER_STORE_QUERY_OPTION_MATCH_ON_NAME)
			    if (regexec (regex, finfo->name, 0, NULL, 0) == 0)
				retval = TRUE;

			if (regex && options & TNY_FOLDER_STORE_QUERY_OPTION_MATCH_ON_ID)
			    if (regexec (regex, finfo->full_name, 0, NULL, 0) == 0)
				retval = TRUE;

			g_object_unref (G_OBJECT (item));
			tny_iterator_iface_next (iterator);
		}
		 
		g_object_unref (G_OBJECT (iterator));    
		g_object_unref (G_OBJECT (items));
	} else
		retval = TRUE;
    
 	return retval;
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
	char *tok, *save;

	tok = strtok_r (dup, ",;", &save);

	while (G_LIKELY (tok != NULL))
	{
		
		_string_to_camel_inet_addr (tok, target);

		tok = strtok_r (NULL, ",;", &save);
	}

	g_free (dup);

	return;
}

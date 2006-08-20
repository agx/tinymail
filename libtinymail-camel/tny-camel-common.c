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
			*name++;
	
		if (G_LIKELY (*lname == ' '))
			*lname-- = '\0';
		camel_internet_address_add (target, name, email);
	} else {
		
		char *name = (char*)tok;
		char *lname = name;

		lname += (strlen (name)-1);

		if (G_LIKELY (*name == ' '))
			*name++;
	
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

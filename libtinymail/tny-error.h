#ifndef TNY_ERROR_H
#define TNY_ERROR_H

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

#include <glib.h>
#include <glib-object.h>
#include <tny-shared.h>

#define TNY_TYPE_ERROR_DOMAIN (tny_error_domain_get_type())

enum _TnyErrorDomain
{
	TNY_FOLDER_ERROR,
	TNY_FOLDER_STORE_ERROR,
	TNY_TRANSPORT_ERROR
};

#define TNY_TYPE_ERROR (tny_error_get_type())

enum _TnyError 
{
	TNY_FOLDER_ERROR_EXPUNGE,
	TNY_FOLDER_ERROR_REMOVE_MSG,
	TNY_FOLDER_ERROR_ADD_MSG,
	TNY_FOLDER_ERROR_REFRESH,
	TNY_FOLDER_ERROR_GET_MSG,
	TNY_FOLDER_ERROR_TRANSFER_MSGS,
	TNY_FOLDER_ERROR_SET_NAME,

	TNY_FOLDER_STORE_ERROR_REMOVE_FOLDER,
	TNY_FOLDER_STORE_ERROR_GET_FOLDERS,
	TNY_FOLDER_STORE_ERROR_CREATE_FOLDER,

	TNY_TRANSPORT_ACCOUNT_ERROR_SEND
};

GType tny_error_domain_get_type (void);
GType tny_error_get_type (void);


#endif

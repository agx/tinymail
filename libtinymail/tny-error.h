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
	TNY_FOLDER_ERROR = 1,
	TNY_FOLDER_STORE_ERROR = 2,
	TNY_TRANSPORT_ACCOUNT_ERROR = 3,
	TNY_ACCOUNT_ERROR = 4
};

#define TNY_TYPE_ERROR (tny_error_get_type())

enum _TnyError 
{
	TNY_ERROR_UNSPEC = 1,

	TNY_FOLDER_ERROR_SYNC = 2,
	TNY_FOLDER_ERROR_REMOVE_MSG = 3,
	TNY_FOLDER_ERROR_ADD_MSG = 4,
	TNY_FOLDER_ERROR_REFRESH = 5,
	TNY_FOLDER_ERROR_GET_MSG = 6,
	TNY_FOLDER_ERROR_TRANSFER_MSGS = 7,
	TNY_FOLDER_ERROR_SET_NAME = 8,
	TNY_FOLDER_ERROR_COPY = 9,

	TNY_FOLDER_STORE_ERROR_REMOVE_FOLDER = 10,
	TNY_FOLDER_STORE_ERROR_GET_FOLDERS = 11,
	TNY_FOLDER_STORE_ERROR_CREATE_FOLDER = 12,

	TNY_TRANSPORT_ACCOUNT_ERROR_SEND = 13,

	TNY_ACCOUNT_ERROR_TRY_CONNECT = 14
};

GType tny_error_domain_get_type (void);
GType tny_error_get_type (void);


#endif

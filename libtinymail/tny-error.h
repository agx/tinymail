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
	TNY_ACCOUNT_ERROR = 4,
	TNY_ACCOUNT_STORE_ERROR = 5
};

#define TNY_TYPE_ERROR (tny_error_get_type())

/* TODO: These should probably be split into separate enums, one for each 
 * error domain. Error codes from one error domain should not be used with 
 * a separate error domain. The hard-coding of the numbers should then not 
 * be necessary or useful.
 */
 
/**
 * TnyError:
 *
 * An GError error code.
 * 
 * TODO: Ideally, we would have more error codes, one for each specific error, 
 * such as "hostname not accepted", so that the application can identify the 
 * exact error, and respond accordingly. However, these error codes are not 
 * yet present because the implementations do not yet provide such 
 * fine-grained information.
 * TODO: Document the other error codes.
 * 
 * TNY_ACCOUNT_ERROR_TRY_CONNECT: This can possibly indicate such errors as 
 * "hostname not resolved", "no network", or "password not accepted", 
 * though some implementations might return TNY_ACCOUNT_STORE_ERROR_UNKNOWN_ALERT 
 * (in the TNY_ACCOUNT_STORE_ERROR error domain) in these cases
 * Currently, the application must know whether it is using our camel 
 * implementation, in order to know whether it can expect to receive a 
 * TNY_ACCOUNT_ERROR_TRY_CONNECT (in the TNY_ACCOUNT_ERROR error domain) rather 
 * than a TNY_ACCOUNT_ERROR_UNKNOWN_ALERT (in the TNY_ACCOUNT_STORE_ERROR 
 * error domain) in these situations. If using our camel implementation then 
 * even this level of exact detection of the particular error is not currently 
 * possible, so a generic error dialog should probably be shown, though the 
 * application could choose to suggest possible causes for the error.
 * 
 * TNY_ACCOUNT_STORE_ERROR_UNKNOWN_ALERT: This indicates that an unknown error 
 * has occurred. Depending on the implementation being used, and the server 
 * being used, this could have multiple possible causes. More exact error 
 * information is only possible with other servers, and/or othre implementations.
 */
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

	TNY_ACCOUNT_ERROR_TRY_CONNECT = 14,
	TNY_ACCOUNT_ERROR_GET_SUPPORTED_AUTH = 16,

	TNY_ACCOUNT_STORE_ERROR_UNKNOWN_ALERT = 15
};

GType tny_error_domain_get_type (void);
GType tny_error_get_type (void);


#endif

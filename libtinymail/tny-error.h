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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <glib.h>
#include <glib-object.h>
#include <tny-shared.h>

#define TNY_TYPE_ERROR_DOMAIN (tny_error_domain_get_type())


enum _TnyErrorDomain
{
	TNY_SYSTEM_ERROR = 1,
	TNY_IO_ERROR = 2,
	TNY_SERVICE_ERROR = 3,
	TNY_MIME_ERROR = 4,

/*

	TNY_CONNECTION_ERROR = 2,
	TNY_PROTOCOL_ERROR = 3,
	TNY
	TNY_FOLDER_STORE_ERROR = 2,
	TNY_TRANSPORT_ACCOUNT_ERROR = 3,
	TNY_ACCOUNT_ERROR = 4,
	TNY_ACCOUNT_STORE_ERROR = 5,
	TNY_SEND_QUEUE_ERROR = 6
*/

};

typedef enum _TnyErrorDomain TnyErrorDomain;

#define TNY_TYPE_ERROR (tny_error_get_type())

 
/**
 * TnyError:
 *
 * A GError error code.
 */
enum _TnyError 
{
	TNY_SYSTEM_ERROR_UNKNOWN,
	TNY_SYSTEM_ERROR_MEMORY,
	TNY_SYSTEM_ERROR_CANCEL,

	TNY_IO_ERROR_WRITE,
	TNY_IO_ERROR_READ,

	TNY_SERVICE_ERROR_UNKNOWN,
	TNY_SERVICE_ERROR_AUTHENTICATE,
	TNY_SERVICE_ERROR_CONNECT,
	TNY_SERVICE_ERROR_UNAVAILABLE,
	TNY_SERVICE_ERROR_LOST_CONNECTION,
	TNY_SERVICE_ERROR_CERTIFICATE,
	TNY_SERVICE_ERROR_FOLDER_CREATE,
	TNY_SERVICE_ERROR_FOLDER_REMOVE,
	TNY_SERVICE_ERROR_FOLDER_RENAME,
	TNY_SERVICE_ERROR_FOLDER_IS_UNKNOWN,
	TNY_SERVICE_ERROR_PROTOCOL,
	TNY_SERVICE_ERROR_UNSUPPORTED,
	TNY_SERVICE_ERROR_NO_SUCH_MESSAGE,
	TNY_SERVICE_ERROR_MESSAGE_NOT_AVAILABLE,
	TNY_SERVICE_ERROR_STATE,

	TNY_SERVICE_ERROR_ADD_MSG,
	TNY_SERVICE_ERROR_REMOVE_MSG,
	TNY_SERVICE_ERROR_GET_MSG,
	TNY_SERVICE_ERROR_SYNC,
	TNY_SERVICE_ERROR_REFRESH,
	TNY_SERVICE_ERROR_COPY,
	TNY_SERVICE_ERROR_TRANSFER,
	TNY_SERVICE_ERROR_GET_FOLDERS,
	TNY_SERVICE_ERROR_SEND,

	TNY_MIME_ERROR_STATE,
	TNY_MIME_ERROR_MALFORMED,
/*

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
	TNY_TRANSPORT_ACCOUNT_ERROR_SEND_HOST_LOOKUP_FAILED = 23,
	TNY_TRANSPORT_ACCOUNT_ERROR_SEND_SERVICE_UNAVAILABLE = 24,
	TNY_TRANSPORT_ACCOUNT_ERROR_SEND_AUTHENTICATION_NOT_SUPPORTED = 25,
	TNY_TRANSPORT_ACCOUNT_ERROR_SEND_USER_CANCEL = 26,

	TNY_ACCOUNT_ERROR_TRY_CONNECT = 14,
	TNY_ACCOUNT_ERROR_TRY_CONNECT_HOST_LOOKUP_FAILED = 19,
	TNY_ACCOUNT_ERROR_TRY_CONNECT_SERVICE_UNAVAILABLE = 20,
	TNY_ACCOUNT_ERROR_TRY_CONNECT_AUTHENTICATION_NOT_SUPPORTED = 21,
	TNY_ACCOUNT_ERROR_TRY_CONNECT_CERTIFICATE = 22,
	TNY_ACCOUNT_ERROR_TRY_CONNECT_USER_CANCEL = 27,

	TNY_ACCOUNT_STORE_ERROR_UNKNOWN_ALERT = 15,

	TNY_ACCOUNT_ERROR_GET_SUPPORTED_AUTH = 16,

	TNY_SEND_QUEUE_ERROR_ADD = 17,

	TNY_ACCOUNT_STORE_ERROR_CANCEL_ALERT = 18,

	TNY_FOLDER_ERROR_REMOVE_MSGS = 19
*/
};

typedef GError TError;

typedef enum _TnyError TnyError;

const gchar* tny_error_get_message (GError *err);
gint tny_error_get_code (GError *err);

GType tny_error_domain_get_type (void);
GType tny_error_get_type (void);


#endif

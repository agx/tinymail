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
	TNY_NO_ERROR,

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
};

typedef GError TError;

typedef enum _TnyError TnyError;

const gchar* tny_error_get_message (GError *err);
gint tny_error_get_code (GError *err);

GType tny_error_domain_get_type (void);
GType tny_error_get_type (void);


#endif

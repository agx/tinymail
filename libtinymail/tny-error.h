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
#include <tny-shared.h>

enum _TnyErrorDomain
{
	TNY_TINYMAIL_ERROR,
	TNY_CAMEL_LITE_ERROR
};

enum _TnyError 
{
	TNY_FOLDER_EXPUNGE_ERROR,
	TNY_FOLDER_REMOVE_MSG_ERROR,
	TNY_FOLDER_ADD_MSG_ERROR,
	TNY_FOLDER_REFRESH_ERROR,
	TNY_FOLDER_GET_MSG_ERROR,
	TNY_FOLDER_TRANSFER_MSGS_ERROR,
	TNY_FOLDER_SET_NAME_ERROR,

	TNY_FOLDER_STORE_REMOVE_FOLDER_ERROR,
	TNY_FOLDER_STORE_GET_FOLDERS_ERROR,
	TNY_FOLDER_STORE_CREATE_FOLDER_ERROR,

	TNY_TRANSPORT_ACCOUNT_SEND_ERROR
};


#endif

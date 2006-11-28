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

#include <tny-error.h>

GType
tny_error_domain_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TNY_FOLDER_ERROR, "TNY_FOLDER_ERROR", "folder_error" },
      { TNY_FOLDER_STORE_ERROR, "TNY_FOLDER_STORE_ERROR", "folder_store_error" },
      { TNY_TRANSPORT_ERROR, "TNY_TRANSPORT_ERROR", "transport_error" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyErrorDomain", values);
  }
  return etype;
}



GType
tny_error_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TNY_FOLDER_ERROR_EXPUNGE, "TNY_FOLDER_ERROR_EXPUNGE", "folder_error_expunge" },
      { TNY_FOLDER_ERROR_REMOVE_MSG, "TNY_FOLDER_ERROR_REMOVE_MSG", "folder_error_remove_msg" },
      { TNY_FOLDER_ERROR_ADD_MSG, "TNY_FOLDER_ERROR_ADD_MSG", "folder_error_add_msg" },
      { TNY_FOLDER_ERROR_REFRESH, "TNY_FOLDER_ERROR_REFRESH", "folder_error_refresh" },
      { TNY_FOLDER_ERROR_GET_MSG, "TNY_FOLDER_ERROR_GET_MSG", "folder_error_get_msg" },
      { TNY_FOLDER_ERROR_TRANSFER_MSGS, "TNY_FOLDER_ERROR_TRANSFER_MSGS", "folder_error_transfer_msgs" },
      { TNY_FOLDER_ERROR_SET_NAME, "TNY_FOLDER_ERROR_SET_NAME", "folder_error_set_name" },

      { TNY_FOLDER_STORE_ERROR_REMOVE_FOLDER, "TNY_FOLDER_STORE_ERROR_REMOVE_FOLDER", "folder_store_error_remove_folder" },
      { TNY_FOLDER_STORE_ERROR_GET_FOLDERS, "TNY_FOLDER_STORE_ERROR_GET_FOLDERS", "folder_store_error_get_folders" },
      { TNY_FOLDER_STORE_ERROR_CREATE_FOLDER, "TNY_FOLDER_STORE_ERROR_CREATE_FOLDER", "folder_store_error_create_folder" },

      { TNY_TRANSPORT_ACCOUNT_ERROR_SEND, "TNY_TRANSPORT_ACCOUNT_ERROR_SEND", "transport_error_account_send" },

      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyError", values);
  }
}

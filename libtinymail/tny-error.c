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


/* TODO: These get_type() functions should be generated, as in GTK+, gnome-vfs, etc. */

/**
 * tny_error_domain_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
GType
tny_error_domain_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TNY_FOLDER_ERROR, "TNY_FOLDER_ERROR", "folder_error" },
      { TNY_FOLDER_STORE_ERROR, "TNY_FOLDER_STORE_ERROR", "folder_store_error" },
      { TNY_TRANSPORT_ACCOUNT_ERROR, "TNY_TRANSPORT_ACCOUNT_ERROR", "transport_account_error" },
      { TNY_ACCOUNT_ERROR, "TNY_ACCOUNT_ERROR", "account_error" },
      { TNY_ACCOUNT_STORE_ERROR, "TNY_ACCOUNT_STORE_ERROR", "account_store_error" },
      { TNY_SEND_QUEUE_ERROR, "TNY_SEND_QUEUE_ERROR", "send_queue_error" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyErrorDomain", values);
  }
  return etype;
}


/**
 * tny_error_get_type:
 *
 * GType system helper function
 *
 * Return value: a GType
 **/
GType
tny_error_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {




      { TNY_ERROR_UNSPEC, "TNY_ERROR_UNSPEC", "error_unspec" },
      { TNY_FOLDER_ERROR_SYNC, "TNY_FOLDER_ERROR_SYNC", "folder_error_sync" },
      { TNY_FOLDER_ERROR_REMOVE_MSG, "TNY_FOLDER_ERROR_REMOVE_MSG", "folder_error_remove_msg" },
      { TNY_FOLDER_ERROR_ADD_MSG, "TNY_FOLDER_ERROR_ADD_MSG", "folder_error_add_msg" },
      { TNY_FOLDER_ERROR_REFRESH, "TNY_FOLDER_ERROR_REFRESH", "folder_error_refresh" },
      { TNY_FOLDER_ERROR_GET_MSG, "TNY_FOLDER_ERROR_GET_MSG", "folder_error_get_msg" },
      { TNY_FOLDER_ERROR_TRANSFER_MSGS, "TNY_FOLDER_ERROR_TRANSFER_MSGS", "folder_error_transfer_msgs" },
      { TNY_FOLDER_ERROR_SET_NAME, "TNY_FOLDER_ERROR_SET_NAME", "folder_error_set_name" },
      { TNY_FOLDER_ERROR_COPY, "TNY_FOLDER_ERROR_COPY", "folder_error_copy" },

      { TNY_FOLDER_STORE_ERROR_REMOVE_FOLDER, "TNY_FOLDER_STORE_ERROR_REMOVE_FOLDER", "folder_store_error_remove_folder" },
      { TNY_FOLDER_STORE_ERROR_GET_FOLDERS, "TNY_FOLDER_STORE_ERROR_GET_FOLDERS", "folder_store_error_get_folders" },
      { TNY_FOLDER_STORE_ERROR_CREATE_FOLDER, "TNY_FOLDER_STORE_ERROR_CREATE_FOLDER", "folder_store_error_create_folder" },

      { TNY_TRANSPORT_ACCOUNT_ERROR_SEND, "TNY_TRANSPORT_ACCOUNT_ERROR_SEND", "transport_account_error_send" },
      { TNY_TRANSPORT_ACCOUNT_ERROR_SEND_HOST_LOOKUP_FAILED, "TNY_TRANSPORT_ACCOUNT_ERROR_SEND_HOST_LOOKUP_FAILED", "transport_account_error_send_host_lookup_failed" },
      { TNY_TRANSPORT_ACCOUNT_ERROR_SEND_SERVICE_UNAVAILABLE, "TNY_TRANSPORT_ACCOUNT_ERROR_SEND_SERVICE_UNAVAILABLE", "transport_account_error_send_service_unavailable" },
      { TNY_TRANSPORT_ACCOUNT_ERROR_SEND_AUTHENTICATION_NOT_SUPPORTED, "TNY_TRANSPORT_ACCOUNT_ERROR_SEND_AUTHENTICATION_NOT_SUPPORTED", "transport_account_error_send_authentication_not_supported" },
      { TNY_TRANSPORT_ACCOUNT_ERROR_SEND_USER_CANCEL, "TNY_ACCOUNT_ERROR_TRY_CONNECT_SEND_USER_CANCEL", "transport_account_error_send_user_cancel" },
     
      { TNY_ACCOUNT_ERROR_TRY_CONNECT, "TNY_ACCOUNT_ERROR_TRY_CONNECT", "account_error_try_connect" },
      { TNY_ACCOUNT_ERROR_TRY_CONNECT_HOST_LOOKUP_FAILED, "TNY_ACCOUNT_ERROR_TRY_CONNECT_HOST_LOOKUP_FAILED", "account_error_try_connect_host_lookup_failed" },
      { TNY_ACCOUNT_ERROR_TRY_CONNECT_SERVICE_UNAVAILABLE, "TNY_ACCOUNT_ERROR_TRY_CONNECT_SERVICE_UNAVAILABLE", "account_error_try_connect_service_unavailable" },
      { TNY_ACCOUNT_ERROR_TRY_CONNECT_AUTHENTICATION_NOT_SUPPORTED, "TNY_ACCOUNT_ERROR_TRY_CONNECT_AUTHENTICATION_NOT_SUPPORTED", "account_error_try_connect_authentication_not_supported" },
      { TNY_ACCOUNT_ERROR_TRY_CONNECT_CERTIFICATE, "TNY_ACCOUNT_ERROR_TRY_CONNECT_CERTIFICATE", "account_error_try_connect_certificate" },
      { TNY_ACCOUNT_ERROR_TRY_CONNECT_USER_CANCEL, "TNY_ACCOUNT_ERROR_TRY_CONNECT_USER_CANCEL", "account_error_try_connect_user_cancel" },
    
      { TNY_ACCOUNT_STORE_ERROR_UNKNOWN_ALERT, "TNY_ACCOUNT_STORE_ERROR_UNKNOWN_ALERT", "account_store_error_unknown_alert" },

      { TNY_SEND_QUEUE_ERROR_ADD, "TNY_SEND_QUEUE_ERROR_ADD", "send_queue_error_add"},

      { TNY_ACCOUNT_STORE_ERROR_CANCEL_ALERT, "TNY_ACCOUNT_STORE_ERROR_CANCEL_ALERT", "account_store_cancel_alert"},
      { TNY_ACCOUNT_ERROR_GET_SUPPORTED_AUTH, "TNY_ACCOUNT_ERROR_GET_SUPPORTED_AUTH", "account_get_supported_auth"},

      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyError", values);
  }
  return etype;
}

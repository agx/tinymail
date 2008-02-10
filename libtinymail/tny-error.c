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

#include <tny-error.h>


/**
 * tny_error_get_message:
 * @err: a #GError
 *
 * Get the error message
 *
 * returns: a error message
 **/
const gchar* 
tny_error_get_message (GError *err)
{
	return err->message;
}

/**
 * tny_error_get_code:
 * @err: a #GError
 *
 * Get the error's code
 *
 * returns: a error code
 **/
gint
tny_error_get_code (GError *err)
{
	return err->code;
}

/**
 * tny_error_domain_get_type:
 *
 * GType system helper function
 *
 * returns: a #GType
 **/
GType
tny_error_domain_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
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
 * returns: a #GType
 **/
GType
tny_error_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {


      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyError", values);
  }
  return etype;
}

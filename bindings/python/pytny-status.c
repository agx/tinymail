/* pytinymail - tinymail python bindings
 * Copyright (C) 2006-2007 
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

#include <tny-status.h>

#ifndef TNY_TYPE_STATUS
GType
pytny_status_get_type(void)
{
  static GType our_type = 0;
  
  if (our_type == 0)
    our_type = g_boxed_type_register_static ("TnyStatus",
					     (GBoxedCopyFunc)tny_status_copy,
					     (GBoxedFreeFunc)tny_status_free);
  return our_type;
}
#endif

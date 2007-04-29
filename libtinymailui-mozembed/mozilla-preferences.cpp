/* libtinymailui-mozembed - The Tiny Mail UI library for Gtk+
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

#include <config.h>

#include "mozilla-preferences.h"

#include <stdlib.h>
#include <nsIServiceManager.h>
#include <nsComponentManagerUtils.h>
#include <pref/nsIPref.h>
#include <nsCOMPtr.h>


extern "C" gboolean
_mozilla_preference_set (const char *preference_name, const char *new_value)
{
    g_return_val_if_fail (preference_name != NULL, FALSE);
    g_return_val_if_fail (new_value != NULL, FALSE);
    
    nsCOMPtr<nsIPref> pref = do_CreateInstance (NS_PREF_CONTRACTID);
    
    if (pref)
    {
	nsresult rv = pref->SetCharPref (preference_name, new_value);
	
	return NS_SUCCEEDED (rv) ? TRUE : FALSE;
    }
    
    return FALSE;
}

extern "C" gboolean
_mozilla_preference_set_boolean (const char *preference_name, gboolean new_boolean_value)
{
    g_return_val_if_fail (preference_name != NULL, FALSE);
    
    nsCOMPtr<nsIPref> pref = do_CreateInstance (NS_PREF_CONTRACTID);
    
    if (pref)
    {
	nsresult rv = pref->SetBoolPref (preference_name,
					 new_boolean_value ? PR_TRUE : PR_FALSE);
	
	return NS_SUCCEEDED (rv) ? TRUE : FALSE;
    }
    
    return FALSE;
}

extern "C" gboolean
_mozilla_preference_set_int (const char *preference_name, gint new_int_value)
{
    g_return_val_if_fail (preference_name != NULL, FALSE);
    
    nsCOMPtr<nsIPref> pref = do_CreateInstance (NS_PREF_CONTRACTID);
    
    if (pref)
    {
	nsresult rv = pref->SetIntPref (preference_name, new_int_value);
	
	return NS_SUCCEEDED (rv) ? TRUE : FALSE;
    }
    
    return FALSE;
}


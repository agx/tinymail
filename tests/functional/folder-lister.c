/* tinymail - Tiny Mail
 * Copyright (C) 2006-2007 Philip Van Hoof <pvanhoof@gnome.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with self program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
#include <tny-list-iface.h>
#include <tny-iterator-iface.h>
#include <tny-list.h>
#include <tny-account-store-iface.h>
#include <tny-store-account-iface.h>
#include <tny-folder-iface.h>

#include <account-store.h>

static gint recursion_level=0;

static void 
recursive_walk_subfolders (TnyFolderIface *parent)
{
    TnyListIface *folders = tny_folder_iface_get_folders (parent);
    if (folders)
    {
	    TnyIteratorIface *iterator = tny_list_iface_create_iterator (folders);
	    
	    while (!tny_iterator_iface_is_done (iterator))
	    {
		TnyFolderIface *folder = TNY_FOLDER_IFACE (tny_iterator_iface_current (iterator));
		
		if (!folder)
			continue;
		
		gint i=0;
		
		for (i=0; i<recursion_level; i++)
			g_print ("\t");
		
		g_print ("\t%s\n", tny_folder_iface_get_name (folder));
		
		recursion_level++;
		recursive_walk_subfolders (folder);
		recursion_level--;
		
		g_object_unref (G_OBJECT (folder));
		tny_iterator_iface_next (iterator);
	    }
	    
	    g_object_unref (G_OBJECT (iterator));
    }
    
    return;
}

static void 
mem_test_print_folders (void)
{
	TnyAccountStoreIface *account_store = TNY_ACCOUNT_STORE_IFACE 
		(tny_memtest_account_store_new ());
	TnyListIface *accounts = tny_list_new (), *folders;
	TnyStoreAccountIface *account;
	TnyIteratorIface *aiter, *fiter;
    
	tny_account_store_iface_get_accounts (account_store, accounts, 
			TNY_ACCOUNT_STORE_IFACE_STORE_ACCOUNTS);

	aiter = tny_list_iface_create_iterator (accounts);
	tny_iterator_iface_first (aiter);
	account = TNY_STORE_ACCOUNT_IFACE (tny_iterator_iface_current (aiter));
    
	folders = tny_store_account_iface_get_folders (account, 
			TNY_STORE_ACCOUNT_FOLDER_TYPE_ALL);
	
	if (folders)
	{
		fiter = tny_list_iface_create_iterator (folders);
		
		while (!tny_iterator_iface_is_done (fiter))
		{
		    TnyFolderIface *folder = TNY_FOLDER_IFACE (tny_iterator_iface_current (fiter));
		    
		    g_print ("Root folder: %s\n", tny_folder_iface_get_name (folder));
		    
		    recursive_walk_subfolders (folder);
		    
		    g_object_unref (G_OBJECT (folder));
		    tny_iterator_iface_next (fiter);
		}
		g_object_unref (G_OBJECT (fiter));
	}
    
	g_object_unref (G_OBJECT (account));
	g_object_unref (G_OBJECT (aiter));
    	g_object_unref (G_OBJECT (accounts));
    
	return;
}

int 
main (int argc, char **argv)
{
    g_type_init ();
    mem_test_print_folders ();
}


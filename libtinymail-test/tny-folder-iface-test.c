/* tinymail - Tiny Mail gunit test
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

#include <string.h>

#include <tny-folder-iface-test.h>
#include <tny-folder-iface.h>
#include <tny-folder.h>

#include <tny-list-iface.h>
#include <tny-iterator-iface.h>
#include <tny-list.h>
#include <tny-account-store-iface.h>
#include <tny-store-account-iface.h>
#include <tny-folder-iface.h>
#include <tny-header.h>

#include <account-store.h>

static TnyFolderIface *iface = NULL;
static TnyAccountStoreIface *account_store;
static TnyListIface *accounts, *root_folders;
static TnyStoreAccountIface *account;
static TnyIteratorIface *aiter;
static gchar *str;

typedef void (*performer) (TnyFolderIface *folder);

static void
do_test_folder (TnyFolderIface *folder)
{
    iface = folder;
}


static void 
recursive_walk_subfolders (TnyFolderIface *parent, const gchar *folname, performer func)
{
    TnyListIface *folders = tny_folder_iface_get_folders (parent);
    if (folders)
    {
	    TnyIteratorIface *iterator = tny_list_iface_create_iterator (folders);
	    
	    while (!tny_iterator_iface_is_done (iterator))
	    {
		TnyFolderIface *folder = TNY_FOLDER_IFACE (tny_iterator_iface_current (iterator));
								
		if (!strcmp (tny_folder_iface_get_id (folder), folname))
			func (folder);
		
		recursive_walk_subfolders (folder, folname, func);
		
		g_object_unref (G_OBJECT (folder));
		tny_iterator_iface_next (iterator);
	    }
	    
	    g_object_unref (G_OBJECT (iterator));
    }
    
    return;
}

static void 
mem_test_folder (TnyListIface *root_folders, const gchar *folname, performer func)
{
    	TnyIteratorIface *fiter;

	fiter = tny_list_iface_create_iterator (root_folders);
	tny_iterator_iface_first (fiter);
	    
	while (!tny_iterator_iface_is_done (fiter))
	{
	    TnyFolderIface *folder = TNY_FOLDER_IFACE (tny_iterator_iface_current (fiter));
		    		    
	    if (!strcmp (tny_folder_iface_get_id (folder), folname))
		func (folder);

	    recursive_walk_subfolders (folder, folname, func);
		    
	    g_object_unref (G_OBJECT (folder));
	    tny_iterator_iface_next (fiter);
	}
	g_object_unref (G_OBJECT (fiter));
    
	return;
}

static void
tny_folder_iface_test_setup (void)
{
	accounts = tny_list_new ();
	account_store = TNY_ACCOUNT_STORE_IFACE (tny_account_store_new (TRUE, NULL));
	tny_account_store_iface_get_accounts (account_store, accounts, 
			TNY_ACCOUNT_STORE_IFACE_STORE_ACCOUNTS);
	aiter = tny_list_iface_create_iterator (accounts);
	tny_iterator_iface_first (aiter);
	account = TNY_STORE_ACCOUNT_IFACE (tny_iterator_iface_current (aiter));
	
    	root_folders = tny_store_account_iface_get_folders (account, 
				TNY_STORE_ACCOUNT_FOLDER_TYPE_SUBSCRIBED);
    
	if (root_folders)
	    	mem_test_folder (root_folders, "INBOX/tny-folder-iface-test", do_test_folder);
    
 	if (iface)
	    	g_object_ref (G_OBJECT (iface));
    
	return;
}

static void 
tny_folder_iface_test_teardown (void)
{
    	if (iface)
	    	g_object_unref (G_OBJECT (iface));

    	g_object_unref (G_OBJECT (account));
	g_object_unref (G_OBJECT (aiter));
	g_object_unref (G_OBJECT (accounts));

	return;
}

	/* TODO: 
	test signal folder inserted and folders_reloaded (hard to test)
	test methods get_folders, get_message, get_headers, remove_message, expunge (hard to test)
	test properties name, id, account, folder_type
	test properties all_count and unread_count
	test methods set_subscribed with get_subscribed
	test async method refresh_async (hard to test)
	test method refresh */


static void
tny_folder_iface_test_get_headers_sync (void)
{
    	TnyListIface *headers;
	gint length = 0, all_count;

    	if (iface == NULL)
	{
		GUNIT_WARNING ("Test cannot continue (are you online?)");
	    	return;
	}
    
    	headers = tny_list_new ();
	tny_folder_iface_refresh (iface);
	all_count = tny_folder_iface_get_all_count (iface);
    
	tny_folder_iface_get_headers (iface, headers, FALSE);
	length = tny_list_iface_length (headers);
        
	str = g_strdup_printf ("I received %d headers, the folder tells me it has %d messages\n", length, all_count);
	gunit_fail_unless (length == all_count, str);
	g_free (str);
    
	g_object_unref (G_OBJECT (headers));
	tny_folder_iface_uncache (iface);
}


GUnitTestSuite*
create_tny_folder_iface_suite (void)
{
	GUnitTestSuite *suite = NULL;

	/* Create test suite */
	suite = gunit_test_suite_new ("TnyFolderIface");

	/* Add test case objects to test suite */
	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_folder_iface_test_get_headers_sync",
                                      tny_folder_iface_test_setup,
                                      tny_folder_iface_test_get_headers_sync,
				      tny_folder_iface_test_teardown));

	return suite;
}

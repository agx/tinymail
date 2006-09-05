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
#include <tny-camel-folder.h>
#include <tny-folder-store-iface.h>
#include <tny-list-iface.h>
#include <tny-iterator-iface.h>
#include <tny-simple-list.h>
#include <tny-account-store-iface.h>
#include <tny-store-account-iface.h>
#include <tny-folder-iface.h>
#include <tny-camel-header.h>

#include <account-store.h>

static TnyFolderIface *iface = NULL;
static TnyAccountStoreIface *account_store;
static TnyListIface *accounts;
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
recurse_folders (TnyFolderStoreIface *store, TnyFolderStoreQuery *query, const gchar *folname, performer func)
{
	TnyIteratorIface *iter;
	TnyListIface *folders = tny_simple_list_new ();

	tny_folder_store_iface_get_folders (store, folders, query);
	iter = tny_list_iface_create_iterator (folders);

	while (!tny_iterator_iface_is_done (iter))
	{
		TnyFolderStoreIface *folder = (TnyFolderStoreIface*) tny_iterator_iface_current (iter);

		if (!strcmp (tny_folder_iface_get_id (TNY_FOLDER_IFACE (folder)), folname))
			func (TNY_FOLDER_IFACE (folder));
	    
		recurse_folders (folder, query, folname, func);
	    
 		g_object_unref (G_OBJECT (folder));

		tny_iterator_iface_next (iter);	    
	}

	 g_object_unref (G_OBJECT (iter));
	 g_object_unref (G_OBJECT (folders));
}

static void
tny_folder_iface_test_setup (void)
{
	accounts = tny_simple_list_new ();
	account_store = tny_test_account_store_new (TRUE, NULL);
	tny_account_store_iface_get_accounts (account_store, accounts, 
			TNY_ACCOUNT_STORE_IFACE_STORE_ACCOUNTS);
	aiter = tny_list_iface_create_iterator (accounts);
	tny_iterator_iface_first (aiter);
	account = TNY_STORE_ACCOUNT_IFACE (tny_iterator_iface_current (aiter));
	
	recurse_folders (TNY_FOLDER_STORE_IFACE (account), NULL, "INBOX/tny-folder-iface-test", do_test_folder);

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
	test methods get_folders, get_message,
	test properties name, account, folder_type
	test properties unread_count
	test methods set_subscribed with get_subscribed
	test async method refresh_async (hard to test)
	*/


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
    
    	headers = tny_simple_list_new ();
	tny_folder_iface_refresh (iface);
	all_count = tny_folder_iface_get_all_count (iface);
    
	tny_folder_iface_get_headers (iface, headers, FALSE);
	length = tny_list_iface_length (headers);
        
	str = g_strdup_printf ("I received %d headers, the folder tells me it has %d messages\n", length, all_count);
	gunit_fail_unless (length == all_count, str);
	g_free (str);
    
	g_object_unref (G_OBJECT (headers));
}


static void
tny_folder_iface_test_remove_message (void)
{
    	TnyListIface *headers;
	gint orig_length = 0, test_len = 0, new_len = 0, headers_len = 0;
	TnyIteratorIface *iter;
    	TnyHeaderIface *header;
    
    	if (iface == NULL)
	{
		GUNIT_WARNING ("Test cannot continue (are you online?)");
	    	return;
	}
    
    	headers = tny_simple_list_new ();
	tny_folder_iface_refresh (iface);
	
	tny_folder_iface_get_headers (iface, headers, FALSE);
	orig_length = tny_list_iface_length (headers);
        test_len = tny_folder_iface_get_all_count (iface);
    
	str = g_strdup_printf ("I received %d headers, the folder tells me it has %d messages\n", orig_length, test_len);
	gunit_fail_unless (orig_length == test_len, str);
	g_free (str);
    
    	iter = tny_list_iface_create_iterator (headers);
    	tny_iterator_iface_first (iter);
    	header = (TnyHeaderIface*)tny_iterator_iface_current (iter);

    	/* Flag as removed */
    	tny_folder_iface_remove_message (iface, header);
    	tny_folder_iface_refresh (iface);

    	g_object_unref (G_OBJECT (headers));

    
    	new_len = tny_folder_iface_get_all_count (iface);
    	str = g_strdup_printf ("After removal but not yet expunge, the new length is %d, whereas it should be %d\n", new_len, orig_length);
	gunit_fail_unless (new_len == orig_length, str);
	g_free (str);

    	headers = tny_simple_list_new ();
    	tny_folder_iface_get_headers (iface, headers, FALSE);
	headers_len = tny_list_iface_length (headers);
	g_object_unref (G_OBJECT (headers));

       	str = g_strdup_printf ("After removal but not yet expunge, the header count is %d, whereas it should be %d\n", headers_len, orig_length);
	gunit_fail_unless (new_len == orig_length, str);
	g_free (str);

    	/* Expunge ...*/
    	tny_folder_iface_expunge (iface);    
	tny_folder_iface_refresh (iface);
    
        new_len = tny_folder_iface_get_all_count (iface);
    	str = g_strdup_printf ("After removal, the new length is %d, whereas it should be %d\n", new_len, orig_length-1);
	gunit_fail_unless (new_len == orig_length-1, str);
	g_free (str);

    	headers = tny_simple_list_new ();
    	tny_folder_iface_get_headers (iface, headers, FALSE);
	headers_len = tny_list_iface_length (headers);
	g_object_unref (G_OBJECT (headers));

       	str = g_strdup_printf ("After removal, the header count is %d, whereas it should be %d\n", headers_len, orig_length-1);
	gunit_fail_unless (new_len == orig_length-1, str);
	g_free (str);

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

    
    	/* Add test case objects to test suite */
	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_folder_iface_test_remove_message",
                                      tny_folder_iface_test_setup,
                                      tny_folder_iface_test_remove_message,
				      tny_folder_iface_test_teardown));

	return suite;
}

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

#include <tny-folder-test.h>
#include <tny-folder.h>
#include <tny-camel-folder.h>
#include <tny-folder-store.h>
#include <tny-list.h>
#include <tny-iterator.h>
#include <tny-simple-list.h>
#include <tny-account-store.h>
#include <tny-store-account.h>
#include <tny-folder.h>
#include <tny-camel-header.h>

#include <account-store.h>

static TnyFolder *iface = NULL;
static TnyAccountStore *account_store;
static TnyList *accounts;
static TnyStoreAccount *account;
static TnyIterator *aiter;
static gchar *str;

typedef void (*performer) (TnyFolder *folder);

static void
do_test_folder (TnyFolder *folder)
{
    iface = folder;
}


static void
recurse_folders (TnyFolderStore *store, TnyFolderStoreQuery *query, const gchar *folname, performer func)
{
	TnyIterator *iter;
	TnyList *folders = tny_simple_list_new ();

	tny_folder_store_get_folders (store, folders, query, NULL);
	iter = tny_list_create_iterator (folders);

	while (!tny_iterator_is_done (iter))
	{
		TnyFolderStore *folder = (TnyFolderStore*) tny_iterator_get_current (iter);

		if (!strcmp (tny_folder_get_id (TNY_FOLDER (folder)), folname))
			func (TNY_FOLDER (folder));
	    
		recurse_folders (folder, query, folname, func);
	    
 		g_object_unref (G_OBJECT (folder));

		tny_iterator_next (iter);	    
	}

	 g_object_unref (G_OBJECT (iter));
	 g_object_unref (G_OBJECT (folders));
}

static void
tny_folder_test_setup (void)
{
	accounts = tny_simple_list_new ();
	account_store = tny_test_account_store_new (TRUE, NULL);
	tny_account_store_get_accounts (account_store, accounts, 
			TNY_ACCOUNT_STORE_STORE_ACCOUNTS);
	aiter = tny_list_create_iterator (accounts);
	tny_iterator_first (aiter);
	account = TNY_STORE_ACCOUNT (tny_iterator_get_current (aiter));
	
	recurse_folders (TNY_FOLDER_STORE (account), NULL, "INBOX/tny-folder-iface-test", do_test_folder);

 	if (iface)
	    	g_object_ref (G_OBJECT (iface));
    
	return;
}

static void 
tny_folder_test_teardown (void)
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
tny_folder_test_get_headers_sync (void)
{
    	TnyList *headers;
	gint length = 0, all_count;

    	if (iface == NULL)
	{
		GUNIT_WARNING ("Test cannot continue (are you online?)");
	    	return;
	}
    
    	headers = tny_simple_list_new ();
	tny_folder_refresh (iface, NULL);
	all_count = tny_folder_get_all_count (iface);
    
	tny_folder_get_headers (iface, headers, FALSE, NULL);
	length = tny_list_get_length (headers);
        
	str = g_strdup_printf ("I received %d headers, the folder tells me it has %d messages\n", length, all_count);
	gunit_fail_unless (length == all_count, str);
	g_free (str);
    
	g_object_unref (G_OBJECT (headers));
}


static void
tny_folder_test_remove_message (void)
{
	TnyList *headers;
	gint orig_length = 0, test_len = 0, new_len = 0, headers_len = 0;
	TnyIterator *iter;
	TnyHeader *header;
	
	if (iface == NULL)
	{
		GUNIT_WARNING ("Test cannot continue (are you online?)");
		return;
	}
	
	headers = tny_simple_list_new ();
	tny_folder_refresh (iface, NULL);
	
	tny_folder_get_headers (iface, headers, FALSE, NULL);
	orig_length = tny_list_get_length (headers);
	test_len = tny_folder_get_all_count (iface);
	
	str = g_strdup_printf ("I received %d headers, the folder tells me it has %d messages\n", orig_length, test_len);
	gunit_fail_unless (orig_length == test_len, str);
	g_free (str);
	
	iter = tny_list_create_iterator (headers);
	tny_iterator_first (iter);
	header = (TnyHeader*)tny_iterator_get_current (iter);
	
	/* Flag as removed */
	tny_folder_remove_msg (iface, header, NULL);
	tny_folder_refresh (iface, NULL);
	
	g_object_unref (G_OBJECT (headers));
	
	
	new_len = tny_folder_get_all_count (iface);
	str = g_strdup_printf ("After removal but not yet expunge, the new length is %d, whereas it should be %d\n", new_len, orig_length);
	gunit_fail_unless (new_len == orig_length, str);
	g_free (str);
	
	headers = tny_simple_list_new ();
	tny_folder_get_headers (iface, headers, FALSE, NULL);
	headers_len = tny_list_get_length (headers);
	g_object_unref (G_OBJECT (headers));
	
	str = g_strdup_printf ("After removal but not yet expunge, the header count is %d, whereas it should be %d\n", headers_len, orig_length);
	gunit_fail_unless (new_len == orig_length, str);
	g_free (str);
	
	/* Expunge ...*/
	tny_folder_expunge (iface, NULL);    
	tny_folder_refresh (iface, NULL);
	
	new_len = tny_folder_get_all_count (iface);
	str = g_strdup_printf ("After removal, the new length is %d, whereas it should be %d\n", new_len, orig_length-1);
	gunit_fail_unless (new_len == orig_length-1, str);
	g_free (str);
	
	headers = tny_simple_list_new ();
	tny_folder_get_headers (iface, headers, FALSE, NULL);
	headers_len = tny_list_get_length (headers);
	g_object_unref (G_OBJECT (headers));
	
	str = g_strdup_printf ("After removal, the header count is %d, whereas it should be %d\n", headers_len, orig_length-1);
	gunit_fail_unless (new_len == orig_length-1, str);
	g_free (str);
	
}

GUnitTestSuite*
create_tny_folder_suite (void)
{
	GUnitTestSuite *suite = NULL;

	/* Create test suite */
	suite = gunit_test_suite_new ("TnyFolder");

	/* Add test case objects to test suite */
	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_folder_test_get_headers_sync",
                                      tny_folder_test_setup,
                                      tny_folder_test_get_headers_sync,
				      tny_folder_test_teardown));

    
    	/* Add test case objects to test suite */
	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_folder_test_remove_message",
                                      tny_folder_test_setup,
                                      tny_folder_test_remove_message,
				      tny_folder_test_teardown));

	return suite;
}

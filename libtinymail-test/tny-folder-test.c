/* tinymail - Tiny Mail unit test
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

#include "check_libtinymail.h"

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

static TnyFolder *iface = NULL, *folder2;
static TnyStoreAccount *account;
static gchar *str;
static gboolean callback_completed;
static GError *err;

typedef void (*performer) (TnyFolder *folder);

static void
do_test_folder (TnyFolder *folder)
{
	iface = folder;
}

static void
second_folder (TnyFolder *folder)
{
	folder2 = folder;
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
		{
			func (TNY_FOLDER (folder));
			g_object_ref (G_OBJECT (folder));
		}
	    
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
	TnyIterator *aiter;
	TnyList *accounts;

	TnyAccountStore *account_store = tny_test_account_store_new (TRUE, NULL);
	accounts = tny_simple_list_new ();
	tny_account_store_get_accounts (account_store, accounts, 
					TNY_ACCOUNT_STORE_STORE_ACCOUNTS);
	g_object_unref (G_OBJECT (account_store));

	aiter = tny_list_create_iterator (accounts);
	tny_iterator_first (aiter);
	account = TNY_STORE_ACCOUNT (tny_iterator_get_current (aiter));
	g_object_unref (G_OBJECT (aiter));
	g_object_unref (G_OBJECT (accounts));
	
	recurse_folders (TNY_FOLDER_STORE (account), NULL, "INBOX/tny-folder-iface-test", do_test_folder);

	return;
}

static void 
tny_folder_test_teardown (void)
{
    	if (iface)
	    	g_object_unref (G_OBJECT (iface));

    	g_object_unref (G_OBJECT (account));

	return;
}

/* TODO: 
   test signal folder inserted and folders_reloaded (hard to test)
   test transfer_msgs, transfer_msgs_async, copy
   test msg_remove_strategy
*/


static gboolean
timeout (gpointer data)
{
	g_print ("Async callback timed out\n");
	gtk_main_quit ();
	return FALSE;
}

START_TEST (tny_folder_test_get_headers_sync)
{
    	TnyList *headers;
	gint length = 0, all_count;

    	if (iface == NULL)
	{
		g_warning ("Test cannot continue (are you online?)");
	    	return;
	}
    
    	headers = tny_simple_list_new ();
	tny_folder_refresh (iface, NULL);
	all_count = tny_folder_get_all_count (iface);
    
	tny_folder_get_headers (iface, headers, FALSE, NULL);
	length = tny_list_get_length (headers);
        
	str = g_strdup_printf ("I received %d headers, the folder tells me it has %d messages\n", length, all_count);
	fail_unless (length == all_count, str);
	g_free (str);
    
	g_object_unref (G_OBJECT (headers));
}
END_TEST


static void
message_received (TnyFolder *folder, TnyMsg *msg, GError **err, gpointer user_data)
{
	fail_unless (*err == NULL, "Error receiving message async");

	callback_completed = TRUE;
	gtk_main_quit ();
}

START_TEST (tny_folder_test_msg)
{
	TnyList *headers;
	gint orig_length = 0, test_len = 0, new_len = 0, headers_len = 0;
	TnyIterator *iter;
	TnyHeader *header;
	TnyMsg *msg;
	TnyHeaderFlags hflags;
	
	if (iface == NULL)
	{
		g_warning ("Test cannot continue (are you online?)");
		return;
	}
	
	headers = tny_simple_list_new ();
	tny_folder_refresh (iface, NULL);
	
	tny_folder_get_headers (iface, headers, FALSE, NULL);
	orig_length = tny_list_get_length (headers);
	test_len = tny_folder_get_all_count (iface);
	
	str = g_strdup_printf ("I received %d headers, the folder tells me it has %d messages\n", orig_length, test_len);
	fail_unless (orig_length == test_len, str);
	g_free (str);
	
	iter = tny_list_create_iterator (headers);
	tny_iterator_first (iter);
	header = (TnyHeader*)tny_iterator_get_current (iter);
	g_object_unref (G_OBJECT (iter));

	/* Test get_msg */
	err = NULL;
	msg = tny_folder_get_msg (iface, header, &err);
	fail_unless (err == NULL, "Error fetching message");

	/* Test get_msg_async */
	callback_completed = FALSE;
	err = NULL;
	tny_folder_get_msg_async (iface, header, message_received, NULL);
	g_timeout_add (1000*6, timeout, NULL);
	gtk_main ();
	fail_unless (callback_completed, "Message was never received");

	/* Flag as removed */
	tny_folder_remove_msg (iface, header, NULL);
	tny_folder_refresh (iface, NULL);

	g_object_unref (G_OBJECT (header));

	new_len = tny_folder_get_all_count (iface);
	str = g_strdup_printf ("After removal but not yet expunge, the new length is %d, whereas it should be %d\n", new_len, orig_length);
	fail_unless (new_len == orig_length, str);
	g_free (str);
	/* Why is get_all_count wrong if headers are unrefed before? */
	g_object_unref (G_OBJECT (headers));
	
	headers = tny_simple_list_new ();
	tny_folder_get_headers (iface, headers, FALSE, NULL);
	headers_len = tny_list_get_length (headers);
	g_object_unref (G_OBJECT (headers));
	
	str = g_strdup_printf ("After removal but not yet expunge, the header count is %d, whereas it should be %d\n", headers_len, orig_length);
	fail_unless (new_len == orig_length, str);
	g_free (str);
	
	/* Expunge ...*/
	tny_folder_expunge (iface, NULL);    
	tny_folder_refresh (iface, NULL);
	
	new_len = tny_folder_get_all_count (iface);
	str = g_strdup_printf ("After removal, the new length is %d, whereas it should be %d\n", new_len, orig_length-1);
	fail_unless (new_len == orig_length-1, str);
	g_free (str);
	
	headers = tny_simple_list_new ();
	tny_folder_get_headers (iface, headers, FALSE, NULL);
	headers_len = tny_list_get_length (headers);
	g_object_unref (G_OBJECT (headers));
	
	str = g_strdup_printf ("After removal, the header count is %d, whereas it should be %d\n", headers_len, orig_length-1);
	fail_unless (headers_len == orig_length-1, str);
	g_free (str);

	/* Test add_msg (crashes for some reason) */
/* 	err = NULL; */
/* 	tny_folder_add_msg (iface, msg, &err); */
/* 	fail_unless (err == NULL, "Error adding message to folder"); */
/* 	tny_folder_refresh (iface, NULL); */
/* 	new_len = tny_folder_get_all_count (iface); */
/* 	fail_unless (new_len == orig_length, "After readd of removed message, message count should be the original"); */

	g_object_unref (G_OBJECT (msg));
}
END_TEST

START_TEST (tny_folder_test_properties)
{
	gint count;

	if (iface == NULL)
	{
		g_warning ("Test cannot continue (are you online?)");
		return;
	}

	fail_unless (strcmp (tny_folder_get_id (iface), "INBOX/tny-folder-iface-test") == 0, "Folder had wrong ID property");
	err = NULL;
	tny_folder_refresh (iface, &err); 
	fail_unless (err == NULL, "Error refreshing folder");
	fail_unless (tny_folder_get_all_count (iface) > 0, "Message count too small");
	fail_unless (tny_folder_get_unread_count (iface) == 1, "Unread count is wrong");
	TnyStoreAccount *acnt = (TnyStoreAccount *) tny_folder_get_account (iface);
	fail_unless (acnt == TNY_ACCOUNT (account), "Property account has wrong value");
	g_object_unref (G_OBJECT (acnt));
	fail_unless (tny_folder_get_folder_type (iface) == TNY_FOLDER_TYPE_NORMAL, "Folder type should be NORMAL");
	recurse_folders (TNY_FOLDER_STORE (account), NULL, "INBOX", second_folder);
	fail_unless (tny_folder_get_folder_type (folder2) == TNY_FOLDER_TYPE_INBOX, "Folder type should be INBOX");
	g_object_unref (G_OBJECT (folder2));
}
END_TEST

START_TEST (tny_folder_test_name)
{
	const gchar *name = "tny-folder-iface-test";
	const gchar *temp_name = "tny-folder-iface-test_temp";

	if (iface == NULL)
	{
		g_warning ("Test cannot continue (are you online?)");
		return;
	}

	fail_unless (strcmp (tny_folder_get_name (iface), name) == 0, "Folder had wrong name property");
	err = NULL;
	tny_folder_set_name (iface, temp_name, &err);
	fail_unless (err == NULL, "An error occured while renaming folder");
	fail_unless (strcmp (tny_folder_get_name (iface), temp_name) == 0, "Folder had wrong name property");
	err = NULL;
	tny_folder_set_name (iface, name, &err);
	fail_unless (err == NULL, "An error occured while renaming folder");
	fail_unless (strcmp (tny_folder_get_name (iface), name) == 0, "Folder had wrong name property");
}
END_TEST

START_TEST (tny_folder_test_subscribed)
{
	TnyFolder *folder;

	fail_unless (tny_folder_is_subscribed (iface), "Subscription property should be set");
	recurse_folders (TNY_FOLDER_STORE (account), NULL, "INBOX/unsubscribed_folder", second_folder);
	fail_unless (!tny_folder_is_subscribed (folder2), "Subscription property should be unset");
	g_object_unref (G_OBJECT (folder2));
}
END_TEST

START_TEST (tny_folder_test_refresh)
{
	if (iface == NULL)
	{
		g_warning ("Test cannot continue (are you online?)");
		return;
	}

	err = NULL;
	tny_folder_refresh (iface, &err);
	fail_unless (tny_folder_get_unread_count (iface) == 1, "Message count not updated");
}
END_TEST

static void
refresh_progress (TnyFolder *folder, const gchar *what, gint status, gint oftotal, gpointer user_data)
{
	g_print (".");
}

static void
folder_refreshed (TnyFolder *folder, gboolean cancelled, GError **err, gpointer user_data)
{
	g_print ("done\n");
	fail_unless (!cancelled, "Async refresh cancelled");
	callback_completed = TRUE;
	gtk_main_quit ();
}

START_TEST (tny_folder_test_refresh_async)
{
	if (iface == NULL)
	{
		g_warning ("Test cannot continue (are you online?)");
		return;
	}

	err = NULL;
	g_print ("Refreshing folder..");
	callback_completed = FALSE;
	tny_folder_refresh_async (iface, folder_refreshed, refresh_progress, &err);
	g_timeout_add (1000*6, timeout, NULL);
	gtk_main ();
	fail_unless (callback_completed, "Refresh callback was never called");
	fail_unless (tny_folder_get_unread_count (iface) == 1, "Message count not updated");
}
END_TEST

Suite *
create_tny_folder_suite (void)
{
	TCase *tc = NULL;
	Suite *s = suite_create ("Folder");

	tc = tcase_create ("Get Headers Sync");
	tcase_set_timeout (tc, 5);
	tcase_add_checked_fixture (tc, tny_folder_test_setup, tny_folder_test_teardown);
	tcase_add_test (tc, tny_folder_test_get_headers_sync);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Get/Get Async/Remove/Add Message");
	tcase_set_timeout (tc, 5);
	tcase_add_checked_fixture (tc, tny_folder_test_setup, tny_folder_test_teardown);
	tcase_add_test (tc, tny_folder_test_msg);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Properties");
	tcase_add_checked_fixture (tc, tny_folder_test_setup, tny_folder_test_teardown);
	tcase_add_test (tc, tny_folder_test_properties);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Name");
	tcase_set_timeout (tc, 15);
	tcase_add_checked_fixture (tc, tny_folder_test_setup, tny_folder_test_teardown);
	tcase_add_test (tc, tny_folder_test_name);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Subscribed");
	tcase_add_checked_fixture (tc, tny_folder_test_setup, tny_folder_test_teardown);
	tcase_add_test (tc, tny_folder_test_subscribed);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Refresh");
	tcase_set_timeout (tc, 10);
	tcase_add_checked_fixture (tc, tny_folder_test_setup, tny_folder_test_teardown);
	tcase_add_test (tc, tny_folder_test_refresh);
	tcase_add_test (tc, tny_folder_test_refresh_async);
	suite_add_tcase (s, tc);

	return s;
}

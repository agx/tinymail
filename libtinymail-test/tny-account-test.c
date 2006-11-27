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

#include <tny-account-test.h>
#include <tny-account.h>
#include <tny-folder-store.h>

#include <tny-camel-store-account.h>
#include <tny-list.h>
#include <tny-iterator.h>
#include <tny-simple-list.h>
#include <tny-account-store.h>
#include <tny-store-account.h>
#include <tny-folder.h>
#include <tny-camel-header.h>

#include <account-store.h>

static TnyAccount *iface = NULL;
static TnyAccountStore *account_store;
static TnyList *accounts;
static TnyIterator *aiter;
static gboolean online_tests=FALSE;
static gchar *str;

static void
tny_account_test_setup (void)
{
    iface = NULL;
    
    if (online_tests)
    {
	accounts = tny_simple_list_new ();
	account_store = tny_test_account_store_new (TRUE, NULL);
	tny_account_store_get_accounts (account_store, accounts, 
			TNY_ACCOUNT_STORE_STORE_ACCOUNTS);
	aiter = tny_list_create_iterator (accounts);
	tny_iterator_first (aiter);
    
	iface = TNY_ACCOUNT (tny_iterator_get_current (aiter));
    
    	if (!iface)
		online_tests = FALSE;
	
    }
    
    if (!iface)
	    iface = TNY_ACCOUNT (tny_camel_store_account_new ());
    
   return;
}

static void 
tny_account_test_teardown (void)
{
    	g_object_unref (G_OBJECT (iface));
	g_object_unref (G_OBJECT (aiter));
	g_object_unref (G_OBJECT (accounts));
    
	return;
}

static void
tny_store_account_test_get_folders (void)
{
    	TnyList *folders = NULL;
    
      	if (!online_tests)
	    	return;
    
    	folders = tny_simple_list_new ();
    
    	tny_folder_store_get_folders (TNY_FOLDER_STORE (iface),
			folders, NULL, NULL);
        
    	gunit_fail_unless (tny_list_get_length (folders) == 1, 
		"Account should have at least an inbox folder\n");
    
    	g_object_unref (G_OBJECT (folders));
    
    	return;
}

static void
tny_account_test_get_account_type (void)
{
	gunit_fail_unless (tny_account_get_account_type (iface) == TNY_ACCOUNT_TYPE_STORE, 
		"Account type should be store\n");
}

static void
tny_account_test_set_hostname (void)
{
	const gchar *str_in = "imap.imapserver.com", *str_out;

	tny_account_set_hostname (iface, str_in);
	str_out = tny_account_get_hostname (iface);

	str = g_strdup_printf ("Unable to set hostname to %s, it became %s\n", str_in, str_out);
	gunit_fail_unless (!strcmp (str_in, str_out), str);
	g_free (str);
}

static void
tny_account_test_set_user (void)
{
	const gchar *str_in = "myusername", *str_out;

	tny_account_set_user (iface, str_in);
	str_out = tny_account_get_user (iface);

	str = g_strdup_printf ("Unable to set user to %s, it became %s\n", str_in, str_out);
	gunit_fail_unless (!strcmp (str_in, str_out), str);
	g_free (str);
}

static void
tny_account_test_set_id (void)
{
	const gchar *str_in = "THE_ID", *str_out;

	tny_account_set_id (iface, str_in);
	str_out = tny_account_get_id (iface);

	str = g_strdup_printf ("Unable to set id to %s, it became %s\n", str_in, str_out);
	gunit_fail_unless (!strcmp (str_in, str_out), str);
	g_free (str);
}

static void
tny_account_test_set_name (void)
{
	const gchar *str_in = "The name of the account", *str_out;

	tny_account_set_name (iface, str_in);
	str_out = tny_account_get_name (iface);

	str = g_strdup_printf ("Unable to set name to %s, it became %s\n", str_in, str_out);
	gunit_fail_unless (!strcmp (str_in, str_out), str);
	g_free (str);
}

static void
tny_account_test_set_proto (void)
{
	const gchar *str_in = "imap", *str_out;

	tny_account_set_proto (iface, str_in);
	str_out = tny_account_get_proto (iface);

	str = g_strdup_printf ("Unable to set proto to %s, it became %s\n", str_in, str_out);
	gunit_fail_unless (!strcmp (str_in, str_out), str);
	g_free (str);
}

GUnitTestSuite*
create_tny_account_suite (void)
{
	GUnitTestSuite *suite = NULL;

	/* Create test suite */
	suite = gunit_test_suite_new ("TnyAccount");

	/* Add test case objects to test suite */

    	online_tests = FALSE;
    
	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_account_test_get_account_type",
                                      tny_account_test_setup,
                                      tny_account_test_get_account_type,
				      tny_account_test_teardown));

	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_account_test_set_hostname",
                                      tny_account_test_setup,
                                      tny_account_test_set_hostname,
				      tny_account_test_teardown));

	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_account_test_set_user",
                                      tny_account_test_setup,
                                      tny_account_test_set_user,
				      tny_account_test_teardown));


	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_account_test_set_id",
                                      tny_account_test_setup,
                                      tny_account_test_set_id,
				      tny_account_test_teardown));

	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_account_test_set_proto",
                                      tny_account_test_setup,
                                      tny_account_test_set_proto,
				      tny_account_test_teardown));

	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_account_test_set_name",
                                      tny_account_test_setup,
                                      tny_account_test_set_name,
				      tny_account_test_teardown));

    	online_tests = TRUE;
	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_store_account_test_get_folders",
                                      tny_account_test_setup,
                                      tny_store_account_test_get_folders,
				      tny_account_test_teardown));

	return suite;
}

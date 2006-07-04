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

#include <tny-account-iface-test.h>
#include <tny-account-iface.h>

#include <tny-store-account.h>

static TnyAccountIface *iface = NULL;
static gchar *str;

static void
tny_account_iface_test_setup (void)
{

	iface = TNY_ACCOUNT_IFACE (tny_store_account_new ());

	return;
}

static void 
tny_account_iface_test_teardown (void)
{
	g_object_unref (G_OBJECT (iface));

	return;
}

static void
tny_account_iface_test_something (void)
{
	/* TODO: Implement some useful tests */

	str = g_strdup_printf ("Reason\n");
	gunit_fail_unless (0 == 0, str);
	g_free (str);
}


GUnitTestSuite*
create_tny_account_iface_suite (void)
{
	GUnitTestSuite *suite = NULL;

	/* Create test suite */
	suite = gunit_test_suite_new ("TnyAccountIface");

	/* Add test case objects to test suite */
	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_account_iface_test_something",
                                      tny_account_iface_test_setup,
                                      tny_account_iface_test_something,
				      tny_account_iface_test_teardown));

	return suite;
}

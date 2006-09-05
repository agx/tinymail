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

#include <tny-iterator-test.h>
#include <tny-iterator.h>

static gchar *str;

static void
tny_iterator_test_setup (void)
{

	/*iface = TNY_ITERATOR (tny_iterator_new ()); */

	return;
}

static void 
tny_iterator_test_teardown (void)
{
	/* g_object_unref (G_OBJECT (iface)); */

	return;
}

static void
tny_iterator_test_something (void)
{
	/* TODO: Integrate these tests with the tny-list-test.c */
	
	str = g_strdup_printf ("Reason\n");
	gunit_fail_unless (0 == 0, str);
	g_free (str);
}


GUnitTestSuite*
create_tny_iterator_suite (void)
{
	GUnitTestSuite *suite = NULL;

	/* Create test suite */
	suite = gunit_test_suite_new ("TnyIterator");

	/* Add test case objects to test suite */
	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_iterator_test_something",
                                      tny_iterator_test_setup,
                                      tny_iterator_test_something,
				      tny_iterator_test_teardown));

	return suite;
}

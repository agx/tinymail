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

#include <tny-list-iface-test.h>
#include <tny-list.h>

static TnyListIface *iface = NULL, *source = NULL;
static gchar *str;

static void
tny_list_iface_test_setup (void)
{

	iface = TNY_LIST_IFACE (tny_list_new ());

	return;
}

static void 
tny_list_iface_test_teardown (void)
{
	/* g_object_unref (G_OBJECT (iface)); */

	return;
}

static void
tny_list_iface_test_list (void)
{
	TnyIteratorIface *iterator;
	gchar *item;
/* TODO: redo this test using a GOBject 
	tny_list_iface_append (iface, "2");
	tny_list_iface_append (iface, "3");
	tny_list_iface_append (iface, "4");
	tny_list_iface_prepend (iface, "1");

	str = g_strdup_printf ("Length should be 4 but is %d\n", tny_list_iface_length (iface));
	gunit_fail_unless (tny_list_iface_length (iface) == 4, str);
	g_free (str);

	iterator = tny_list_iface_create_iterator (iface);

	item = tny_iterator_iface_nth (iterator, 2);

	str = g_strdup_printf ("Item should be \"3\" but is %s\n", item);
	gunit_fail_unless (!strcmp (item, "3"), str);
	g_free (str);

	item = tny_iterator_iface_next (iterator);

	str = g_strdup_printf ("Item should be \"4\" but is %s\n", item);
	gunit_fail_unless (!strcmp (item, "4"), str);
	g_free (str);

	g_object_unref (G_OBJECT (iterator));
*/

}


GUnitTestSuite*
create_tny_list_iface_suite (void)
{
	GUnitTestSuite *suite = NULL;

	/* Create test suite */
	suite = gunit_test_suite_new ("TnyListIface");

	/* Add test case objects to test suite */
	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_list_iface_test_list",
                                      tny_list_iface_test_setup,
                                      tny_list_iface_test_list,
				      tny_list_iface_test_teardown));

	return suite;
}

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

#include <tny-msg-iface-test.h>
#include <tny-msg-iface.h>
#include <tny-msg.h>

#include <camel/camel-folder.h>
#include <camel/camel.h>
#include <camel/camel-folder-summary.h>

static TnyMsgIface *iface = NULL;
static gchar *str;

static void
tny_msg_iface_test_setup (void)
{
	/* Don't ask me why, I think this is a Camel bug */
	CamelInternetAddress *addr = camel_internet_address_new ();
	camel_object_unref (CAMEL_OBJECT (addr));

	iface = TNY_MSG_IFACE (tny_msg_new ());

	return;
}

static void 
tny_msg_iface_test_teardown (void)
{
	g_object_unref (G_OBJECT (iface));

	return;
}

static void
tny_msg_iface_test_something (void)
{
	/* TODO: 
	test methods get_parts, del_part and add_part (important tests)
	test properties header, folder */

	
	str = g_strdup_printf ("Reason\n");
	gunit_fail_unless (0 == 0, str);
	g_free (str);
}


GUnitTestSuite*
create_tny_msg_iface_suite (void)
{
	GUnitTestSuite *suite = NULL;

	/* Create test suite */
	suite = gunit_test_suite_new ("TnyMsgIface");

	/* Add test case objects to test suite */
	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_msg_iface_test_something",
                                      tny_msg_iface_test_setup,
                                      tny_msg_iface_test_something,
				      tny_msg_iface_test_teardown));

	return suite;
}

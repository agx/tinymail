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

#include <tny-msg-header-iface-test.h>

/* We are going to test the camel implementation */
#include <tny-msg-header.h>

static TnyMsgHeaderIface *iface = NULL;

void
tny_msg_header_iface_test_setup (void)
{
	g_print ("setup\n");
	iface = TNY_MSG_HEADER_IFACE (tny_msg_header_new ());

	return;
}

void 
tny_msg_header_iface_test_teardown (void)
{
	g_object_unref (G_OBJECT (iface));

	return;
}

void
tny_msg_header_iface_test_set_from (void)
{
	const gchar *str_in = "1st test string", *str_out;
	
	tny_msg_header_iface_set_from (iface, str_in);
	str_out = tny_msg_header_iface_get_from (iface);

	gunit_fail_unless(!strcmp (str_in, str_out), "Unable to set from!");

	return;
}

void
tny_msg_header_iface_test_set_to (void)
{
	const gchar *str_in = "1st test string", *str_out;
	
	tny_msg_header_iface_set_to (iface, str_in);
	str_out = tny_msg_header_iface_get_to (iface);

	gunit_fail_unless(!strcmp (str_in, str_out), "Unable to set to!");

	return;
}

void
tny_msg_header_iface_test_set_cc (void)
{
	const gchar *str_in = "1st test string", *str_out;
	
	tny_msg_header_iface_set_cc (iface, str_in);
	str_out = tny_msg_header_iface_get_cc (iface);

	g_print ("CC test: %s,%s\n", str_in, str_out);

	gunit_fail_unless(!strcmp (str_in, str_out), "Unable to set cc!");

	return;
}

void
tny_msg_header_iface_test_set_bcc (void)
{
	const gchar *str_in = "1st test string", *str_out;
	
	tny_msg_header_iface_set_bcc (iface, str_in);
	str_out = tny_msg_header_iface_get_bcc (iface);

	gunit_fail_unless(!strcmp (str_in, str_out), "Unable to set bcc!");

	return;
}

GUnitTestSuite*
create_tny_msg_header_iface_suite (void)
{
	GUnitTestSuite *suite = NULL;

	/* Create test suite */
	suite = gunit_test_suite_new ("TnyMsgheaderIface");

	g_print ("New suite\n");

	/* Add test case objects to test suite */
	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_msg_header_iface_test_set_bcc",
                                      tny_msg_header_iface_test_setup,
                                      tny_msg_header_iface_test_set_bcc,
				      tny_msg_header_iface_test_teardown));

	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_msg_header_iface_test_set_cc",
                                      tny_msg_header_iface_test_setup,
                                      tny_msg_header_iface_test_set_cc,
				      tny_msg_header_iface_test_teardown));

	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_msg_header_iface_test_set_to",
                                      tny_msg_header_iface_test_setup,
                                      tny_msg_header_iface_test_set_to,
				      tny_msg_header_iface_test_teardown));


	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_msg_header_iface_test_set_from",
                                      tny_msg_header_iface_test_setup,
                                      tny_msg_header_iface_test_set_from,
				      tny_msg_header_iface_test_teardown));

	return suite;
}

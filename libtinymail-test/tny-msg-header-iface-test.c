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

#include <camel/camel-folder.h>
#include <camel/camel.h>
#include <camel/camel-folder-summary.h>

#include <tny-msg-header-iface-test.h>

/* We are going to test the camel implementation */
#include <tny-msg-header.h>

static TnyMsgHeaderIface *iface = NULL;

static void
tny_msg_header_iface_test_setup (void)
{
	/* Don't ask me why, I think this is a Camel bug */
	CamelInternetAddress *addr = camel_internet_address_new ();
	camel_object_unref (CAMEL_OBJECT (addr));

	iface = TNY_MSG_HEADER_IFACE (tny_msg_header_new ());

	return;
}

static void 
tny_msg_header_iface_test_teardown (void)
{
	g_object_unref (G_OBJECT (iface));

	return;
}

static void
tny_msg_header_iface_test_set_from (void)
{
	const gchar *str_in = "Me myself and I <me.myself@and.i.com>", *str_out;
	
	tny_msg_header_iface_set_from (iface, str_in);
	str_out = tny_msg_header_iface_get_from (iface);

	gunit_fail_unless(!strcmp (str_in, str_out), "Unable to set from!\n");

	return;
}

static void
tny_msg_header_iface_test_set_to (void)
{
	gchar *str_in = g_strdup ("Myself <this@is.me>, You Do Die Daa <you.doe.die@daa.com>; patrick@test.com");
	const gchar *str_out;
	int i=0;
	
	tny_msg_header_iface_set_to (iface, (const gchar*)str_in);
	str_out = tny_msg_header_iface_get_to (iface);

	/* The implementation will always return a comma separated list
	 * but should also accept ; separated lists. Even mixed (both
	 * characters have the same meaning). */

	for (i=0; i < strlen(str_in); i++)
		if (str_in[i] == ';')
			str_in[i] = ',';

	gunit_fail_unless(!strcmp (str_in, str_out), "Unable to set to!\n");

	g_free (str_in);

	return;
}

static void
tny_msg_header_iface_test_set_cc (void)
{
	const gchar *str_in = "First user <first@user.be>, Second user <second@user.com>", *str_out;

	tny_msg_header_iface_set_cc (iface, str_in);
	str_out = tny_msg_header_iface_get_cc (iface);

	gunit_fail_unless(!strcmp (str_in, str_out), "Unable to set cc!\n");

	return;
}

static void
tny_msg_header_iface_test_set_bcc (void)
{
	const gchar *str_in = "The Invisible man <the.invisible@man.com>, mark@here.there.com", *str_out;

	tny_msg_header_iface_set_bcc (iface, str_in);
	str_out = tny_msg_header_iface_get_bcc (iface);

	gunit_fail_unless(!strcmp (str_in, str_out), "Unable to set bcc!\n");

	return;
}

static void
tny_msg_header_iface_test_set_subject (void)
{
	const gchar *str_in = "I'm the nice subject", *str_out;
	
	tny_msg_header_iface_set_subject (iface, str_in);
	str_out = tny_msg_header_iface_get_subject (iface);

	gunit_fail_unless(!strcmp (str_in, str_out), "Unable to set subject!\n");
}

static void
tny_msg_header_iface_test_set_replyto (void)
{

	/* GUNIT_WARNING ("TODO"); */

	return;
}

GUnitTestSuite*
create_tny_msg_header_iface_suite (void)
{
	GUnitTestSuite *suite = NULL;

	/* Create test suite */
	suite = gunit_test_suite_new ("TnyMsgHeaderIface");


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


	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_msg_header_iface_test_set_replyto",
                                      tny_msg_header_iface_test_setup,
                                      tny_msg_header_iface_test_set_replyto,
				      tny_msg_header_iface_test_teardown));

	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_msg_header_iface_test_set_subject",
                                      tny_msg_header_iface_test_setup,
                                      tny_msg_header_iface_test_set_subject,
				      tny_msg_header_iface_test_teardown));

	return suite;
}

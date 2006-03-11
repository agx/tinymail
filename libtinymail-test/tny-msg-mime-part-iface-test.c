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

#include <tny-msg-mime-part-iface-test.h>

/* We are going to test the camel implementation */
#include <tny-msg-mime-part.h>
#include <tny-stream-iface.h>
#include <tny-test-stream.h>

#include <camel/camel-folder.h>
#include <camel/camel.h>
#include <camel/camel-folder-summary.h>

static TnyMsgMimePartIface *iface = NULL;

static void
tny_msg_mime_part_iface_test_setup (void)
{
	/* Don't ask me why, I think this is a Camel bug */
	CamelInternetAddress *addr = camel_internet_address_new ();
	camel_object_unref (CAMEL_OBJECT (addr));

	CamelMimePart *cpart = camel_mime_part_new ();
	iface = TNY_MSG_MIME_PART_IFACE (tny_msg_mime_part_new (cpart));

	return;
}

static void 
tny_msg_mime_part_iface_test_teardown (void)
{
	g_object_unref (G_OBJECT (iface));

	return;
}

static void
tny_msg_mime_part_iface_test_set_content_location (void)
{
	const gchar *str_in = "test content location", *str_out;
	
	tny_msg_mime_part_iface_set_content_location (iface, str_in);
	str_out = tny_msg_mime_part_iface_get_content_location (iface);

	gunit_fail_unless(!strcmp (str_in, str_out), "Unable to set content location!\n");

	return;
}


static void
tny_msg_mime_part_iface_test_set_description (void)
{
	const gchar *str_in = "test description", *str_out;
	
	tny_msg_mime_part_iface_set_description (iface, str_in);
	str_out = tny_msg_mime_part_iface_get_description (iface);

	gunit_fail_unless(!strcmp (str_in, str_out), "Unable to set description!\n");

	return;
}

static void
tny_msg_mime_part_iface_test_set_content_id (void)
{
	const gchar *str_in = "test content id", *str_out;
	
	tny_msg_mime_part_iface_set_content_id (iface, str_in);
	str_out = tny_msg_mime_part_iface_get_content_id (iface);

	gunit_fail_unless(!strcmp (str_in, str_out), "Unable to set content id!\n");

	return;
}
    
static void
tny_msg_mime_part_iface_test_set_filename (void)
{
	const gchar *str_in = "test_filename.txt", *str_out;
	
	tny_msg_mime_part_iface_set_filename (iface, str_in);
	str_out = tny_msg_mime_part_iface_get_filename (iface);

	gunit_fail_unless(!strcmp (str_in, str_out), "Unable to set filename!\n");

	return;
}
       
static void
tny_msg_mime_part_iface_test_set_content_type (void)
{
	const gchar *str_in = "text/html", *str_out;
	
	tny_msg_mime_part_iface_set_content_type (iface, str_in);
	str_out = tny_msg_mime_part_iface_get_content_type (iface);

	gunit_fail_unless(!strcmp (str_in, str_out), "Unable to content type!\n");

	return;
}

static void
tny_msg_mime_part_iface_test_stream (void)
{
	TnyStreamIface *from = TNY_STREAM_IFACE (tny_test_stream_new ());
	TnyStreamIface *to = TNY_STREAM_IFACE (tny_test_stream_new ());
	gint n;
	TnyTestStream *myto = TNY_TEST_STREAM (to);

	tny_msg_mime_part_iface_construct_from_stream (iface, from);

g_print ("write to ->\n");
	tny_msg_mime_part_iface_write_to_stream (iface, to);
g_print ("write to ->\n");

	while (!tny_stream_iface_eos (to) || n > 1)
	{
		gchar buf[2];
		tny_stream_iface_read (to, buf, sizeof (buf));

		g_warning ("%s\n", buf);

		n++;
	}

	g_warning ("---> %s\n", myto->wrote);

}

GUnitTestSuite*
create_tny_msg_mime_part_iface_suite (void)
{
	GUnitTestSuite *suite = NULL;

	/* Create test suite */
	suite = gunit_test_suite_new ("TnyMsgMimePartIface");

	/* Add test case objects to test suite */
	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_msg_mime_part_iface_test_set_content_location",
                                      tny_msg_mime_part_iface_test_setup,
                                      tny_msg_mime_part_iface_test_set_content_location,
				      tny_msg_mime_part_iface_test_teardown));

	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_msg_mime_part_iface_test_set_description",
                                      tny_msg_mime_part_iface_test_setup,
                                      tny_msg_mime_part_iface_test_set_description,
				      tny_msg_mime_part_iface_test_teardown));

	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_msg_mime_part_iface_test_set_content_id",
                                      tny_msg_mime_part_iface_test_setup,
                                      tny_msg_mime_part_iface_test_set_content_id,
				      tny_msg_mime_part_iface_test_teardown));
	
	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_msg_mime_part_iface_test_set_content_type",
                                      tny_msg_mime_part_iface_test_setup,
                                      tny_msg_mime_part_iface_test_set_content_type,
				      tny_msg_mime_part_iface_test_teardown));

	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_msg_mime_part_iface_test_set_filename",
                                      tny_msg_mime_part_iface_test_setup,
                                      tny_msg_mime_part_iface_test_set_filename,
				      tny_msg_mime_part_iface_test_teardown));
g_print ("add\n");
      tny_msg_mime_part_iface_test_setup();
                                      tny_msg_mime_part_iface_test_stream();
				      tny_msg_mime_part_iface_test_teardown();

g_print ("done\n");

	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_msg_mime_part_iface_test_stream",
                                      tny_msg_mime_part_iface_test_setup,
                                      tny_msg_mime_part_iface_test_stream,
				      tny_msg_mime_part_iface_test_teardown));
	return suite;
}

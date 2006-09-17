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

#include <tny-mime-part-test.h>

/* We are going to test the camel implementation */
#include <tny-camel-mime-part.h>
#include <tny-stream.h>
#include <tny-test-stream.h>
#include <tny-stream-camel.h>
#include <tny-camel-stream.h>

#include <camel/camel-folder.h>
#include <camel/camel.h>
#include <camel/camel-folder-summary.h>


static TnyMimePart *iface = NULL;
static gchar *str;

static void
tny_mime_part_test_setup (void)
{
	/* Don't ask me why, I think this is a Camel bug */
	CamelInternetAddress *addr = camel_internet_address_new ();
	camel_object_unref (CAMEL_OBJECT (addr));

	CamelMimePart *cpart = camel_mime_part_new ();
	iface = TNY_MIME_PART (tny_camel_mime_part_new (cpart));

	return;
}

static void 
tny_mime_part_test_teardown (void)
{
	g_object_unref (G_OBJECT (iface));

	return;
}

static void
tny_mime_part_test_set_content_location (void)
{
	const gchar *str_in = "testcontentlocation", *str_out;
	
	tny_mime_part_set_content_location (iface, str_in);
	str_out = tny_mime_part_get_content_location (iface);

	str = g_strdup_printf ("Unable to set content location! (%s) vs. (%s)\n",
		str_in, str_out);

	gunit_fail_unless(!strcmp (str_in, str_out), str);

	g_free (str);

	return;
}


static void
tny_mime_part_test_set_description (void)
{
	const gchar *str_in = "test description", *str_out;
	
	tny_mime_part_set_description (iface, str_in);
	str_out = tny_mime_part_get_description (iface);

	gunit_fail_unless(!strcmp (str_in, str_out), "Unable to set description!\n");

	return;
}

static void
tny_mime_part_test_set_content_id (void)
{
	const gchar *str_in = "testcontentid", *str_out;
	
	tny_mime_part_set_content_id (iface, str_in);
	str_out = tny_mime_part_get_content_id (iface);

	gunit_fail_unless(!strcmp (str_in, str_out), "Unable to set content id!\n");

	return;
}
    
static void
tny_mime_part_test_set_filename (void)
{
	const gchar *str_in = "test_filename.txt", *str_out;
	
	tny_mime_part_set_filename (iface, str_in);
	str_out = tny_mime_part_get_filename (iface);

	gunit_fail_unless(!strcmp (str_in, str_out), "Unable to set filename!\n");

	return;
}
       
static void
tny_mime_part_test_set_content_type (void)
{
	const gchar *str_in = "text/html", *str_out;
	
	tny_mime_part_set_content_type (iface, str_in);
	str_out = tny_mime_part_get_content_type (iface);

	gunit_fail_unless(!strcmp (str_in, str_out), "Unable to content type!\n");

	return;
}

static void
tny_mime_part_test_stream (void)
{
	CamelStream *real_to = camel_stream_mem_new ();
	TnyStream *to = TNY_STREAM (tny_camel_stream_new (real_to));

/* TODO (this one crashes)

	tny_mime_part_construct_from_stream (iface, from, "text/plain");
	tny_mime_part_write_to_stream (iface, to);

	while (!tny_stream_is_eos (to) && n < 1)
	{
		gchar buf[2];
		tny_stream_read (to, buf, sizeof (buf));

		buf[2] = '\0';

		gunit_fail_unless(!strcmp (buf, "42"), "Unable to stream!\n");

		n++;
	}
*/
	g_object_unref (G_OBJECT (to));
	camel_object_unref (CAMEL_OBJECT (real_to));

	return;
}

GUnitTestSuite*
create_tny_mime_part_suite (void)
{
	GUnitTestSuite *suite = NULL;

	/* Create test suite */
	suite = gunit_test_suite_new ("TnyMimePart");

	/* Add test case objects to test suite */
	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_mime_part_test_set_content_location",
                                      tny_mime_part_test_setup,
                                      tny_mime_part_test_set_content_location,
				      tny_mime_part_test_teardown));

	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_mime_part_test_set_description",
                                      tny_mime_part_test_setup,
                                      tny_mime_part_test_set_description,
				      tny_mime_part_test_teardown));

	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_mime_part_test_set_content_id",
                                      tny_mime_part_test_setup,
                                      tny_mime_part_test_set_content_id,
				      tny_mime_part_test_teardown));
	
	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_mime_part_test_set_content_type",
                                      tny_mime_part_test_setup,
                                      tny_mime_part_test_set_content_type,
				      tny_mime_part_test_teardown));

	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_mime_part_test_set_filename",
                                      tny_mime_part_test_setup,
                                      tny_mime_part_test_set_filename,
				      tny_mime_part_test_teardown));

	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_mime_part_test_stream",
                                      tny_mime_part_test_setup,
                                      tny_mime_part_test_stream,
				      tny_mime_part_test_teardown));
	return suite;
}

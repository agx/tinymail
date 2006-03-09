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

#include <tny-stream-iface-test.h>

/* We are going to test the camel implementation */
#include <tny-stream-camel.h>
#include <camel/camel-stream-mem.h>
#include <camel/camel-data-wrapper.h>
#include <camel/camel-folder.h>
#include <camel/camel.h>
#include <camel/camel-folder-summary.h>

static TnyStreamIface *iface = NULL, *source = NULL;

static void
tny_stream_iface_test_setup (void)
{
	/* Don't ask me why, I think this is a Camel bug */
	CamelInternetAddress *addr = camel_internet_address_new ();
	CamelStream *stream = camel_stream_mem_new ();

	camel_object_unref (CAMEL_OBJECT (addr));

	/* This is the stream that is being tested */

	iface = TNY_STREAM_IFACE (tny_stream_camel_new (stream));

	/* This is a test stream that streams 21 times the answer to
	   all questions (21 times the bytes '4' and '2'). */

	source = TNY_STREAM_IFACE (tny_test_stream_new ());

	return;
}

static void 
tny_stream_iface_test_teardown (void)
{
	g_object_unref (G_OBJECT (iface));
	g_object_unref (G_OBJECT (source));

	return;
}

static void
tny_stream_iface_test_stream (void)
{
	gchar *buffer = (gchar*) malloc (sizeof (gchar) * 42);
	gint n=0;
	/* 21 times the answer to all questions */
	const gchar *ret = "424242424242424242424242424242424242424242";

	tny_stream_iface_reset (source);
	tny_stream_iface_reset (iface);

	tny_stream_iface_write_to_stream (source, iface);

	/* Reset the stream being tested and read the amount of bytes that
	   keep 21 times the answer to all questions from the stream */

	tny_stream_iface_reset (iface);
	tny_stream_iface_read (iface, buffer, strlen (ret));

	/* Check whether the stream in the beginning now contains 21 times
	   the answer to all questions */

	gunit_fail_unless(!strcmp (buffer, ret), 
		"At least one of the 42 first bytes changed!\n");

	/* Check whether the stream contains nothing but the answer to all
	   questions. */

	tny_stream_iface_reset (iface);
	tny_stream_iface_reset (source);

	while (!tny_stream_iface_eos (iface))
	{
		gchar buf[2];
		tny_stream_iface_read (iface, buf, sizeof (buf));

		gunit_fail_unless(!strcmp (buffer, "42"), "Bytes changed!\n");
		n++;
	}

	gunit_fail_unless(n != 21, 
		"Size in bytes isn't correct or reset didn't succeed!\n");

	g_free (buffer);
}


GUnitTestSuite*
create_tny_stream_iface_suite (void)
{
	GUnitTestSuite *suite = NULL;

	/* Create test suite */
	suite = gunit_test_suite_new ("TnyStreamIface");

	/* Add test case objects to test suite */
	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_stream_iface_test_stream",
                                      tny_stream_iface_test_setup,
                                      tny_stream_iface_test_stream,
				      tny_stream_iface_test_teardown));

	return suite;
}

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
#include <tny-camel-msg.h>
#include <tny-camel-header.h>
#include <tny-list-iface.h>
#include <tny-iterator-iface.h>
#include <tny-list.h>

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

	iface = tny_camel_msg_new ();

	return;
}

static void 
tny_msg_iface_test_teardown (void)
{
	g_object_unref (G_OBJECT (iface));

	return;
}
static void
tny_msg_iface_test_set_header (void)
{
	TnyHeaderIface *header = TNY_HEADER_IFACE (tny_camel_header_new ()), *gheader;
	tny_msg_iface_set_header (iface, header);
	gheader = tny_msg_iface_get_header (iface);
	    
	str = g_strdup_printf ("Get-header should return what setheader sets\n");
	gunit_fail_unless (header == gheader, str);
	g_free (str);
    
    	return;
}

static void
tny_msg_iface_test_add_part_del_part (void)
{
    
    	gint length = 0;
    	TnyListIface *parts = TNY_LIST_IFACE (tny_list_new());
	CamelMimePart *cpart = camel_mime_part_new ();
	TnyMimePartIface *part = TNY_MIME_PART_IFACE (tny_camel_mime_part_new (cpart));
    
	tny_msg_iface_add_part (iface, part);

    	tny_msg_iface_get_parts (iface, parts);
	length = tny_list_iface_length (parts);
    	g_object_unref (G_OBJECT (parts));

    	/* TnyMsg contains one part by itself */
	str = g_strdup_printf ("Length must be exactly 2, received %d parts\n", length);
	gunit_fail_unless (length == 2, str);
	g_free (str);

    	parts = TNY_LIST_IFACE (tny_list_new ());
    	tny_msg_iface_del_part (iface, part);
       	tny_msg_iface_get_parts (iface, parts);
	length = tny_list_iface_length (parts);
    	g_object_unref (G_OBJECT (parts));
    
	str = g_strdup_printf ("Length must be exactly 1, received %d parts\n", length);
	gunit_fail_unless (length == 1, str);
	g_free (str);
    
    	return;
}


GUnitTestSuite*
create_tny_msg_iface_suite (void)
{
	GUnitTestSuite *suite = NULL;

	/* Create test suite */
	suite = gunit_test_suite_new ("TnyMsgIface");

	/* Add test case objects to test suite */
	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_msg_iface_test_add_part_del_part",
                                      tny_msg_iface_test_setup,
                                      tny_msg_iface_test_add_part_del_part,
				      tny_msg_iface_test_teardown));

    
    	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_msg_iface_test_set_header",
                                      tny_msg_iface_test_setup,
                                      tny_msg_iface_test_set_header,
				      tny_msg_iface_test_teardown));

	return suite;
}

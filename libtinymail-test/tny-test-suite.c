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


/* Usage:
 * export LD_LIBRARY_PATH=./libtinymail-test/.libs/
 * gunit-test-runner-tool -s tinymail-test-suite -v gnome
 */

#include "tny-test-suite.h"

#include "tny-msg-header-iface-test.h"
#include "tny-stream-iface-test.h"
#include "tny-msg-mime-part-iface-test.h"
#include "tny-list-iface-test.h"
#include "tny-account-iface-test.h"
#include "tny-device-iface-test.h"
#include "tny-iterator-iface-test.h"
#include "tny-msg-folder-iface-test.h"
#include "tny-msg-iface-test.h"
#include "tny-platform-factory-iface-test.h"
#include "tny-account-store-iface-test.h"

GList*
gunit_get_test_suites()
{
	GList *suites = NULL;

	suites = g_list_append (suites, create_tny_msg_header_iface_suite ());
	suites = g_list_append (suites, create_tny_stream_iface_suite ());
	suites = g_list_append (suites, create_tny_msg_mime_part_iface_suite ());
	suites = g_list_append (suites, create_tny_list_iface_suite ());
	suites = g_list_append (suites, create_tny_account_iface_suite ());
	suites = g_list_append (suites, create_tny_device_iface_suite ());
	suites = g_list_append (suites, create_tny_iterator_iface_suite ());
	suites = g_list_append (suites, create_tny_msg_folder_iface_suite ());
	suites = g_list_append (suites, create_tny_msg_iface_suite ());
	suites = g_list_append (suites, create_tny_platform_factory_iface_suite ());
	suites = g_list_append (suites, create_tny_account_store_iface_suite ());

	return suites;
}

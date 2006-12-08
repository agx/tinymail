/* tinymail - Tiny Mail unit test
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

#include "tny-header-test.h"
#include "tny-stream-test.h"
#include "tny-mime-part-test.h"
#include "tny-list-test.h"
#include "tny-account-test.h"
#include "tny-device-test.h"
#include "tny-folder-test.h"
#include "tny-msg-test.h"
#include "tny-platform-factory-test.h"
#include "tny-account-store-test.h"
#include "tny-folder-store-query-test.h"

GList*
gunit_get_test_suites()
{
	GList *suites = NULL;

    	suites = g_list_append (suites, create_tny_msg_suite ());
    	suites = g_list_append (suites, create_tny_folder_store_query_suite ());
	suites = g_list_append (suites, create_tny_header_suite ());
	suites = g_list_append (suites, create_tny_stream_suite ());
	suites = g_list_append (suites, create_tny_mime_part_suite ());
	suites = g_list_append (suites, create_tny_list_suite ());
	suites = g_list_append (suites, create_tny_account_suite ());
	suites = g_list_append (suites, create_tny_device_suite ());
	suites = g_list_append (suites, create_tny_folder_suite ());
	suites = g_list_append (suites, create_tny_platform_factory_suite ());
	suites = g_list_append (suites, create_tny_account_store_suite ());
    
	return suites;
}

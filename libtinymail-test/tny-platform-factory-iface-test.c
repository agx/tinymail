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

#include <tny-platform-factory-iface-test.h>
#include <tny-platform-factory-iface.h>

#include <tny-platform-factory.h>

static TnyPlatformFactoryIface *iface = NULL;
static gchar *str;

static void
tny_platform_factory_iface_test_setup (void)
{
	iface = TNY_PLATFORM_FACTORY_IFACE (tny_platform_factory_get_instance ());

	return;
}

static void 
tny_platform_factory_iface_test_teardown (void)
{
	/* It's a singleton */
	/* g_object_unref (G_OBJECT (iface)); */

	return;
}

/* TODO: Define and implement this unit test */



static void
tny_platform_factory_iface_test_new_device (void)
{
	GObject *obj = (GObject*)tny_platform_factory_iface_new_device (iface);

	str = g_strdup_printf ("Returned instance doesn't implement TnyDeviceIface\n");
	gunit_fail_unless (TNY_IS_DEVICE_IFACE (obj), str);
	g_free (str);

	g_object_unref (G_OBJECT (obj));
}

static void
tny_platform_factory_iface_test_new_account_store (void)
{
	GObject *obj = (GObject*)tny_platform_factory_iface_new_account_store (iface);

	str = g_strdup_printf ("Returned instance doesn't implement TnyAccountStoreIface\n");
	gunit_fail_unless (TNY_IS_ACCOUNT_STORE_IFACE (obj), str);
	g_free (str);

	g_object_unref (G_OBJECT (obj));
}

static void
tny_platform_factory_iface_test_new_msg_view (void)
{
	GObject *obj = (GObject*)tny_platform_factory_iface_new_msg_view (iface);

	str = g_strdup_printf ("Returned instance doesn't implement TnyMsgViewIface\n");
	gunit_fail_unless (TNY_IS_MSG_VIEW_IFACE (obj), str);
	g_free (str);

	g_object_unref (G_OBJECT (obj));
}

GUnitTestSuite*
create_tny_platform_factory_iface_suite (void)
{
	GUnitTestSuite *suite = NULL;

	/* Create test suite */
	suite = gunit_test_suite_new ("TnyPlatformFactoryIface");

	/* Add test case objects to test suite */

	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_platform_factory_iface_test_new_device",
                                      tny_platform_factory_iface_test_setup,
                                      tny_platform_factory_iface_test_new_device,
				      tny_platform_factory_iface_test_teardown));


	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_platform_factory_iface_test_new_account_store",
                                      tny_platform_factory_iface_test_setup,
                                      tny_platform_factory_iface_test_new_account_store,
				      tny_platform_factory_iface_test_teardown));

	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_platform_factory_iface_test_new_msg_view",
                                      tny_platform_factory_iface_test_setup,
                                      tny_platform_factory_iface_test_new_msg_view,
				      tny_platform_factory_iface_test_teardown));

	return suite;
}

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

#include <tny-device-iface-test.h>
#include <tny-device-iface.h>
#include <device.h>

static TnyDeviceIface *iface = NULL;
static gchar *str;

static void
tny_device_iface_test_setup (void)
{

	iface = TNY_DEVICE_IFACE (tny_device_new ());

	return;
}

static void 
tny_device_iface_test_teardown (void)
{
	g_object_unref (G_OBJECT (iface));

	return;
}

/* TODO:  test signal connection_changed (hard to test) */

static void
tny_device_iface_test_is_online (void)
{
	tny_device_iface_force_online (iface);
		
	str = g_strdup_printf ("Device should be online after force_online\n");
	gunit_fail_unless (tny_device_iface_is_online(iface) == TRUE, str);
	g_free (str);

	tny_device_iface_force_offline (iface);
		
	str = g_strdup_printf ("Device should be offline after force_online\n");
	gunit_fail_unless (tny_device_iface_is_online(iface) == FALSE, str);
	g_free (str);
}

GUnitTestSuite*
create_tny_device_iface_suite (void)
{
	GUnitTestSuite *suite = NULL;

	/* Create test suite */
	suite = gunit_test_suite_new ("TnyDeviceIface");

	/* Add test case objects to test suite */
	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_device_iface_test_is_online",
                                      tny_device_iface_test_setup,
                                      tny_device_iface_test_is_online,
				      tny_device_iface_test_teardown));

	return suite;
}

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

#include "check_libtinymail.h"

#include <tny-device.h>
#include <device.h>

static TnyDevice *iface = NULL;
static gchar *str;

static void
tny_device_test_setup (void)
{
	iface = tny_test_device_new ();

	return;
}

static void 
tny_device_test_teardown (void)
{
	g_object_unref (G_OBJECT (iface));

	return;
}

/* TODO:  test signal connection_changed (hard to test) */

START_TEST (tny_device_test_is_online)
{
	tny_device_force_online (iface);
		
	str = g_strdup_printf ("Device should be online after force_online\n");
	fail_unless (tny_device_is_online(iface) == TRUE, str);
	g_free (str);

	tny_device_force_offline (iface);
		
	str = g_strdup_printf ("Device should be offline after force_online\n");
	fail_unless (tny_device_is_online(iface) == FALSE, str);
	g_free (str);
}
END_TEST

Suite *
create_tny_device_suite (void)
{
     Suite *s = suite_create ("Device");

     TCase *tc = tcase_create ("Is Online");
     tcase_add_checked_fixture (tc, tny_device_test_setup, tny_device_test_teardown);
     tcase_add_test (tc, tny_device_test_is_online);
     suite_add_tcase (s, tc);

     return s;
}

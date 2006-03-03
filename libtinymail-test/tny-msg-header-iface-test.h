#ifndef TNY_MSG_HEADER_IFACE_TEST_H
#define TNY_MSG_HEADER_IFACE_TEST_H

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

#include "tny-test-suite.h"

#include <tny-msg-header-iface.h>

/* Setup & Teardown */
void tny_msg_header_iface_test_setup    (void);
void tny_msg_header_iface_test_teardown (void);

/* Test cases */
void tny_msg_header_iface_test_set_bcc  (void);           /* Test case 01 */
void tny_msg_header_iface_test_set_cc   (void);           /* Test case 02 */
void tny_msg_header_iface_test_set_to   (void);           /* Test case 03 */
void tny_msg_header_iface_test_set_from (void);           /* Test case 04 */

/* Test suite */
GUnitTestSuite *create_tny_msg_header_iface_suite (void);


#endif

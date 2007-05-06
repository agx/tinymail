#ifndef TNY_IDLE_STOPPER_PRIV_H
#define TNY_IDLE_STOPPER_PRIV_H

/* libtinymail - The Tiny Mail base library
 * Copyright (C) 2006-2007 Philip Van Hoof <pvanhoof@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with self library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <glib.h>

#ifndef TINYMAIL_ENABLE_PRIVATE_API
#error "This is private API that should only be used by tinymail itself."
#endif

G_BEGIN_DECLS

typedef struct _TnyIdleStopper TnyIdleStopper;

TnyIdleStopper* tny_idle_stopper_new();
TnyIdleStopper* tny_idle_stopper_copy (TnyIdleStopper *stopper);

void tny_idle_stopper_stop (TnyIdleStopper *stopper);
void tny_idle_stopper_destroy(TnyIdleStopper *stopper);
gboolean tny_idle_stopper_is_stopped(TnyIdleStopper* stopper);

G_END_DECLS

#endif /* TNY_IDLE_STOPPER_PRIV_H */

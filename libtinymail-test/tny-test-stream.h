#ifndef TNY_TEST_STREAM_H
#define TNY_TEST_STREAM_H

/* libtinymail-test - The Tiny Mail test library
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
#include <libgnomevfs/gnome-vfs.h>
#include <glib-object.h>

#include <tny-test-stream.h>

#include "tny-test-suite.h"


G_BEGIN_DECLS

#define TNY_TYPE_TEST_STREAM             (tny_test_stream_get_type ())
#define TNY_TEST_STREAM(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_TEST_STREAM, TnyTestStream))
#define TNY_TEST_STREAM_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_TEST_STREAM, TnyTestStreamClass))
#define TNY_IS_TEST_STREAM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_TEST_STREAM))
#define TNY_IS_TEST_STREAM_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_TEST_STREAM))
#define TNY_TEST_STREAM_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_TEST_STREAM, TnyTestStreamClass))

typedef struct _TnyTestStream TnyTestStream;
typedef struct _TnyTestStreamClass TnyTestStreamClass;

struct _TnyTestStream
{
	GObject parent;

	gint eos_count;
};

struct _TnyTestStreamClass 
{
	GObjectClass parent;
};

GType                   tny_test_stream_get_type        (void);
TnyTestStream*          tny_test_stream_new             (void);

G_END_DECLS

#endif


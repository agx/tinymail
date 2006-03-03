#ifndef TNY_TEST_TUI_VIEW_H
#define TNY_TEST_TUI_VIEW_H

/* tinymail - Tiny Mail
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

#include <gunit.h>

G_BEGIN_DECLS

#define TNY_TEST_TUI_VIEW_TYPE             (tny_test_tui_view_get_type ())
#define TNY_TEST_TUI_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TEST_TUI_VIEW_TYPE, TnyTestTuiView))
#define TNY_TEST_TUI_VIEW_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TEST_TUI_VIEW_TYPE, TnyTestTuiViewClass))
#define TNY_IS_TEST_TUI_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TEST_TUI_VIEW_TYPE))
#define TNY_IS_TEST_TUI_VIEW_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TEST_TUI_VIEW_TYPE))
#define TNY_TEST_TUI_VIEW_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TEST_TUI_VIEW_TYPE, TnyTestTuiViewClass))

typedef struct _TnyTestTuiView TnyTestTuiView;
typedef struct _TnyTestTuiViewClass TnyTestTuiViewClass;

struct _TnyTestTuiView
{
	GObject parent;
};

struct _TnyTestTuiViewClass
{
	GObjectClass parent_class;
};

GType               tny_test_tui_view_get_type       (void);
TnyTestTuiView*     tny_test_tui_view_new            (void);


G_END_DECLS

#endif

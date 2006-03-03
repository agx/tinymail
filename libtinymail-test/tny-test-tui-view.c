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

#include "tny-test-tui-view.h"

static GObjectClass *parent_class = NULL;

typedef struct _TnyTestTuiViewPriv TnyTestTuiViewPriv;

struct _TnyTestTuiViewPriv
{
	GUnitTestRunner *runner;
};

#define TNY_TEST_TUI_VIEW_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TEST_TUI_VIEW_TYPE, TnyTestTuiViewPriv))


static void 
tny_test_tui_view_update (GUnitView *self)
{
}

static void 
tny_test_tui_view_set_test_runner (GUnitView *self, gpointer runner)
{
	TnyTestTuiViewPriv *priv = TNY_TEST_TUI_VIEW_GET_PRIVATE (self);

	if (priv->runner)
		g_object_unref (G_OBJECT (priv->runner));

	g_object_ref (runner);
	priv->runner = runner;

	return;
}

static void 
tny_test_tui_view_show (GUnitView *self)
{
}

static void 
tny_test_tui_view_reset (GUnitView *self)
{
}

static void 
tny_test_tui_view_failure (GUnitView *self, const gchar *msg)
{
	g_print ("Unit test failure: %s\n", msg);
}

static void 
tny_test_tui_view_error (GUnitView *self, const gchar *msg)
{
	g_print ("Unit test error: %s\n", msg);
}

static void 
tny_test_tui_view_update_progressbar (GUnitView *self, gint fraction, const gchar *msg)
{
}

static void 
tny_test_tui_view_update_statusbar (GUnitView *view, const gchar *msg)
{
}


/**
 * tny_test_tui_view_new:
 * 
 *
 * Return value: A new #GUnitView instance implemented for console
 **/
TnyTestTuiView*
tny_test_tui_view_new (void)
{
	TnyTestTuiView *self = g_object_new (TNY_TEST_TUI_VIEW_TYPE, NULL);

	return self;
}

static void
tny_test_tui_view_instance_init (GTypeInstance *instance, gpointer g_class)
{
	return;
}

static void
tny_test_tui_view_finalize (GObject *object)
{
	TnyTestTuiViewPriv *priv = TNY_TEST_TUI_VIEW_GET_PRIVATE (object);

	if (priv->runner)
		g_object_unref (G_OBJECT (priv->runner));

	(*parent_class->finalize) (object);
	return;
}


static void 
tny_test_tui_view_class_init (TnyTestTuiViewClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_test_tui_view_finalize;

	g_type_class_add_private (object_class, sizeof (TnyTestTuiViewPriv));

	return;
}

static void
gunit_view_init (gpointer g_iface, gpointer iface_data)
{
	GUnitViewClass *klass = (GUnitViewClass *)g_iface;

	klass->update = tny_test_tui_view_update;
	klass->set_test_runner = tny_test_tui_view_set_test_runner;
        klass->show = tny_test_tui_view_show;
        klass->reset = tny_test_tui_view_reset;
        klass->failure = tny_test_tui_view_failure;
        klass->error = tny_test_tui_view_error;
        klass->update_progressbar = tny_test_tui_view_update_progressbar;
	klass->update_statusbar = tny_test_tui_view_update_statusbar;

	return;
}

GType 
tny_test_tui_view_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyTestTuiViewClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_test_tui_view_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyTestTuiView),
		  0,      /* n_preallocs */
		  tny_test_tui_view_instance_init    /* instance_init */
		};

		static const GInterfaceInfo gunit_type_view_info = 
		{
		  (GInterfaceInitFunc) gunit_view_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyTestTuiView",
			&info, 0);


		g_type_add_interface_static (type, GUNIT_TYPE_VIEW, 
			&gunit_type_view_info);

	}

	return type;
}

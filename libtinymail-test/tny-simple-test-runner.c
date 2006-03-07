#include "tny-test-suite.h" 
#include <view.h>

static GObjectClass *parent_class = NULL;

typedef struct _TnyTestView TnyTestView;
typedef struct _TnyTestViewClass TnyTestViewClass;

struct _TnyTestView
{
	GObject parent;
};

struct _TnyTestViewClass
{
	GObjectClass parent;
};

static void
tny_test_view_update (GUnitView *self)
{
}

static void
tny_test_view_set_test_runner (GUnitView *self, gpointer runner)
{
}

static void
tny_test_view_show (GUnitView *self)
{
}

static void
tny_test_view_reset (GUnitView *self)
{
}

static void
tny_test_view_failure (GUnitView *self, const gchar *msg)
{
}

static void
tny_test_view_error (GUnitView *self, const gchar *msg)
{
}

static void
tny_test_view_update_progressbar (GUnitView *self, gint fraction, const gchar *msg)
{
}

static void
tny_test_view_update_statusbar (GUnitView *view, const gchar *msg)
{
}


static void 
gunit_view_init (GUnitViewClass *class)
{
	class->update = tny_test_view_update;
	class->set_test_runner = tny_test_view_set_test_runner;
	class->show = tny_test_view_show;
	class->reset = tny_test_view_reset;
	class->failure = tny_test_view_failure;
	class->error = tny_test_view_error;
	class->update_progressbar = tny_test_view_update_progressbar;
	class->update_statusbar = tny_test_view_update_statusbar;
}

static void 
tny_test_view_finalize (GObject *object)
{
	(*parent_class->finalize) (object);
}

static void
tny_test_view_class_init (TnyTestViewClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_test_view_finalize;
}


GType 
tny_test_view_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyTestViewClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_test_view_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyTestView),
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};

		static const GInterfaceInfo gunit_view_info = 
		{
		  (GInterfaceInitFunc) gunit_view_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyTestView",
			&info, 0);

		g_type_add_interface_static (type, GUNIT_TYPE_VIEW, 
			&gunit_view_info);
	}

	return type;
}

int
main (int argc, char **argv)
{
	GList *suites = NULL, *names = NULL;
	GUnitTestRunner *runner = NULL;
	GUnitView *view = NULL;

	g_type_init ();
	g_thread_init (NULL);

	view = GUNIT_VIEW (g_object_new (tny_test_view_get_type (), NULL));

	runner = gunit_test_runner_get_instance ();

	gunit_test_runner_set_view (runner, view);

	suites = gunit_get_test_suites ();

	while (suites)
	{
		gunit_test_runner_add_suite (runner, suites->data);
		
		suites = g_list_next (suites);
	}

	g_list_free (suites);

	names = gunit_test_runner_get_suite_name_list (runner);

	while (names)
	{
		const gchar *name = (const gchar*)names->data;

		g_print ("Unit test runner runs %s\n", name);
		gunit_test_runner_run_suite (runner, name);

		names = g_list_next (names);
	}

	g_list_free (names);

	return 0;
}

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
gunit_view_init (GUnitViewClass *class)
{
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
	GList *suites = NULL;
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

	gunit_test_runner_run_all  (runner);

	return 0;
}

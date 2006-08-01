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

#include <tny-list-iface-test.h>
#include <tny-list.h>

static GObjectClass *parent_class = NULL;

static TnyListIface *iface = NULL, *source = NULL;
static gchar *str;

typedef struct {
	GObject parent;
	gchar *str;
} TnyTestObject;
typedef struct {
	GObjectClass parent;
} TnyTestObjectClass;

static void
tny_test_object_finalize (GObject *object)
{
	TnyTestObject *tobj = (TnyTestObject*)object; 
	g_free (tobj->str); 
	(*parent_class->finalize) (object);
}

static void 
tny_test_object_class_init (TnyTestObjectClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;
	object_class->finalize = tny_test_object_finalize;

	return;
}

GType 
tny_test_object_get_type (void) 
{
	static GType type = 0;
	if (G_UNLIKELY(type == 0)) {
		static const GTypeInfo info = { sizeof (TnyTestObjectClass),
		  NULL,   /* base_init */ NULL,   /* base_finalize */
		  (GClassInitFunc) tny_test_object_class_init,   /* class_init */ 
		  NULL,  /* class_finalize */
		  NULL,   /* class_data */ sizeof (TnyTestObject),
		  0,      /* n_preallocs */ NULL    /* instance_init */ };
		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyTestObject",
			&info, 0); 
	}
	return type;
}

static GObject*
tny_test_object_new (gchar *str)
{
	TnyTestObject *obj = g_object_new (tny_test_object_get_type(), NULL);
	obj->str = str;
}

static void
tny_list_iface_test_setup (void)
{

	iface = TNY_LIST_IFACE (tny_list_new ());

	return;
}

/*
static void 
foreach_test_object (gpointer item, gpointer userdata)
{
	g_object_unref (G_OBJECT (item));
}
*/

static void 
tny_list_iface_test_teardown (void)
{

	/* tny_list_iface_foreach (iface, foreach_test_object, NULL); */
	g_object_unref (G_OBJECT (iface)); 

	return;
}

static void
tny_list_iface_test_list (void)
{
	TnyIteratorIface *iterator;
	TnyTestObject *item;

	tny_list_iface_append (iface, tny_test_object_new (g_strdup ("2")));
	tny_list_iface_append (iface, tny_test_object_new (g_strdup ("3")));
	tny_list_iface_append (iface, tny_test_object_new (g_strdup ("4")));
	tny_list_iface_prepend (iface, tny_test_object_new (g_strdup ("1")));

	str = g_strdup_printf ("Length should be 4 but is %d\n", tny_list_iface_length (iface));
	gunit_fail_unless (tny_list_iface_length (iface) == 4, str);
	g_free (str);

	iterator = tny_list_iface_create_iterator (iface);

	item = (TnyTestObject*)tny_iterator_iface_nth (iterator, 2);

	str = g_strdup_printf ("Item should be \"3\" but is %s\n", item->str);
	gunit_fail_unless (!strcmp (item->str, "3"), str);
	g_free (str);

	item = (TnyTestObject*)tny_iterator_iface_next (iterator);

	str = g_strdup_printf ("Item should be \"4\" but is %s\n", item->str);
	gunit_fail_unless (!strcmp (item->str, "4"), str);
	g_free (str);

	g_object_unref (G_OBJECT (iterator));

}


GUnitTestSuite*
create_tny_list_iface_suite (void)
{
	GUnitTestSuite *suite = NULL;

	/* Create test suite */
	suite = gunit_test_suite_new ("TnyListIface");

	/* Add test case objects to test suite */
	gunit_test_suite_add_test_case(suite,
               gunit_test_case_new_with_funcs("tny_list_iface_test_list",
                                      tny_list_iface_test_setup,
                                      tny_list_iface_test_list,
				      tny_list_iface_test_teardown));

	return suite;
}

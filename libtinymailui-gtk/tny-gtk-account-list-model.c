/* libtinymailui-gtk - The Tiny Mail UI library for Gtk+
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

#include <config.h>

#include <glib.h>
#include <gtk/gtk.h>

#ifdef GNOME
#include <libgnomeui/libgnomeui.h>
#endif

#include <tny-gtk-account-list-model.h>
#include <tny-mime-part.h>
#include <tny-iterator.h>
#include <tny-mime-part.h>
#include <tny-folder.h>

#include "tny-gtk-account-list-model-iterator-priv.h"


static GObjectClass *parent_class = NULL;


#define TNY_GTK_ACCOUNT_LIST_MODEL_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_GTK_ACCOUNT_LIST_MODEL, TnyGtkAccountListModelPriv))


/**
 * tny_gtk_account_list_model_new:
 *
 *
 * Return value: a new #GtkTreeModel instance suitable for showing  
 * #TnyMimePart instances
 **/
GtkTreeModel*
tny_gtk_account_list_model_new (void)
{
	TnyGtkAccountListModel *self = g_object_new (TNY_TYPE_GTK_ACCOUNT_LIST_MODEL, NULL);

	return GTK_TREE_MODEL (self);
}

static void
tny_gtk_account_list_model_finalize (GObject *object)
{
	TnyGtkAccountListModel *me = (TnyGtkAccountListModel*) object;

	(*parent_class->finalize) (object);
}

static void
tny_gtk_account_list_model_class_init (TnyGtkAccountListModelClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_gtk_account_list_model_finalize;

	return;
}

static void
tny_gtk_account_list_model_instance_init (GTypeInstance *instance, gpointer g_class)
{
	GtkListStore *store = (GtkListStore*) instance;
	TnyGtkAccountListModel *me = (TnyGtkAccountListModel*) instance;
	static GType types[] = { G_TYPE_STRING, G_TYPE_OBJECT };

	me->iterator_lock = g_mutex_new ();

	gtk_list_store_set_column_types (store, 
		TNY_GTK_ACCOUNT_LIST_MODEL_N_COLUMNS, types);

	return;
}

static TnyIterator*
tny_gtk_account_list_model_create_iterator (TnyList *self)
{
	TnyGtkAccountListModel *me = (TnyGtkAccountListModel*)self;

	/* Return a new iterator */

	return TNY_ITERATOR (_tny_gtk_account_list_model_iterator_new (me));
}



static void
tny_gtk_account_list_model_prepend (TnyList *self, GObject* item)
{
	TnyGtkAccountListModel *me = (TnyGtkAccountListModel*)self;
	GtkListStore *store = GTK_LIST_STORE (me);
	GtkTreeIter iter;
	TnyAccount *account = TNY_ACCOUNT (item);
    
	g_mutex_lock (me->iterator_lock);
	gtk_list_store_prepend (store, &iter);
	gtk_list_store_set (store, &iter, 
		TNY_GTK_ACCOUNT_LIST_MODEL_NAME_COLUMN, tny_account_get_name (account),
		TNY_GTK_ACCOUNT_LIST_MODEL_INSTANCE_COLUMN, account, -1);    
	g_mutex_unlock (me->iterator_lock);
}

static void
tny_gtk_account_list_model_append (TnyList *self, GObject* item)
{
	TnyGtkAccountListModel *me = (TnyGtkAccountListModel*)self;
	GtkListStore *store = GTK_LIST_STORE (me);
	GtkTreeIter iter;
	TnyAccount *account = TNY_ACCOUNT (item);
        
	g_mutex_lock (me->iterator_lock);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, 
		TNY_GTK_ACCOUNT_LIST_MODEL_NAME_COLUMN, tny_account_get_name (account),
		TNY_GTK_ACCOUNT_LIST_MODEL_INSTANCE_COLUMN, account, -1);    
	g_mutex_unlock (me->iterator_lock);
}

static guint
tny_gtk_account_list_model_length (TnyList *self)
{
	TnyGtkAccountListModel *me = (TnyGtkAccountListModel*)self;
	guint retval = 0;
	GtkTreeIter iter;
    
	g_mutex_lock (me->iterator_lock);
	gtk_tree_model_get_iter_first (GTK_TREE_MODEL (me), &iter);
	while (gtk_tree_model_iter_next (GTK_TREE_MODEL (me), &iter))
		retval++;
	g_mutex_unlock (me->iterator_lock);

	return retval;
}

static void
tny_gtk_account_list_model_remove (TnyList *self, GObject* item)
{
	TnyGtkAccountListModel *me = (TnyGtkAccountListModel*)self;
	GtkTreeModel *model = GTK_TREE_MODEL (me);
	GtkTreeIter iter;

	g_return_if_fail (G_IS_OBJECT (item));
	g_return_if_fail (G_IS_OBJECT (me));

	/* Remove something from the list */

	g_mutex_lock (me->iterator_lock);
	
	gtk_tree_model_get_iter_first (model, &iter);
	while (gtk_tree_model_iter_next (model, &iter))
	{
		TnyAccount *curacc;

		gtk_tree_model_get (model, &iter, 
			TNY_GTK_ACCOUNT_LIST_MODEL_INSTANCE_COLUMN, 
			&curacc, -1);

		if (curacc == (TnyAccount*)item)
		{
			gtk_list_store_remove (GTK_LIST_STORE (me), &iter);
			g_object_unref (G_OBJECT (item));
			g_object_unref (G_OBJECT (curacc));
			break;
		}
		g_object_unref (G_OBJECT (curacc));
	}

	g_mutex_unlock (me->iterator_lock);
}


typedef struct 
{
	gpointer user_data;
	GFunc func;
} ForeachHelpr;

static gboolean 
tny_gtk_account_list_model_foreach_in_the_list_impl (GtkTreeModel *model,
		GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	ForeachHelpr *dta = (ForeachHelpr*) data;
	TnyAccount *item;

	gtk_tree_model_get (model, iter, 
		TNY_GTK_ACCOUNT_LIST_MODEL_INSTANCE_COLUMN, &item, -1);
	dta->func (item, dta->user_data);
    
	g_object_unref (G_OBJECT (item));
	return FALSE;
}
	
static void 
tny_gtk_account_list_model_foreach_in_the_list (TnyList *self, GFunc func, gpointer user_data)
{
	TnyGtkAccountListModel *me = (TnyGtkAccountListModel*)self;
	ForeachHelpr *dta = g_new0 (ForeachHelpr, 1);
    
	dta->user_data = user_data;
	dta->func = func;
    
	/* Foreach item in the list (without using a slower iterator) */

	g_mutex_lock (me->iterator_lock);
	gtk_tree_model_foreach (GTK_TREE_MODEL (me), 
		tny_gtk_account_list_model_foreach_in_the_list_impl, dta);
	g_mutex_unlock (me->iterator_lock);

    	g_free (dta);

	return;
}


static TnyList*
tny_gtk_account_list_model_copy_the_list (TnyList *self)
{
	TnyGtkAccountListModel *me = (TnyGtkAccountListModel*)self;
	TnyGtkAccountListModel *copy = g_object_new (TNY_TYPE_GTK_ACCOUNT_LIST_MODEL, NULL);

	/* This only copies the TnyList pieces. The result is not a
	   correct or good TnyHeaderListModel. But it will be a correct
	   TnyList instance. It is the only thing the user of this
	   method expects.

	   The new list will point to the same instances, of course. It's
	   only a copy of the list-nodes of course. */

	g_mutex_lock (me->iterator_lock);
	tny_gtk_account_list_model_foreach_in_the_list (TNY_LIST (copy), 
			(GFunc)g_object_ref, NULL);
	g_mutex_unlock (me->iterator_lock);

	return TNY_LIST (copy);
}

static void
tny_list_init (TnyListIface *klass)
{
	klass->length_func = tny_gtk_account_list_model_length;
	klass->prepend_func = tny_gtk_account_list_model_prepend;
	klass->append_func = tny_gtk_account_list_model_append;
	klass->remove_func = tny_gtk_account_list_model_remove;
	klass->create_iterator_func = tny_gtk_account_list_model_create_iterator;
	klass->copy_func = tny_gtk_account_list_model_copy_the_list;
	klass->foreach_func = tny_gtk_account_list_model_foreach_in_the_list;

	return;
}

GType
tny_gtk_account_list_model_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyGtkAccountListModelClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_gtk_account_list_model_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyGtkAccountListModel),
		  0,      /* n_preallocs */
		  tny_gtk_account_list_model_instance_init,    /* instance_init */
		  NULL
		};

		type = g_type_register_static (GTK_TYPE_LIST_STORE, "TnyGtkAccountListModel",
					    &info, 0);


		static const GInterfaceInfo tny_list_info = {
			(GInterfaceInitFunc) tny_list_init,
			NULL,
			NULL
		};

		g_type_add_interface_static (type, TNY_TYPE_LIST,
					     &tny_list_info);

	}

	return type;
}


GType 
tny_gtk_account_list_model_column_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TNY_GTK_ACCOUNT_LIST_MODEL_NAME_COLUMN, "TNY_GTK_ACCOUNT_LIST_MODEL_NAME_COLUMN", "name" },
      { TNY_GTK_ACCOUNT_LIST_MODEL_INSTANCE_COLUMN, "TNY_GTK_ACCOUNT_LIST_MODEL_INSTANCE_COLUMN", "instance" },
      { TNY_GTK_ACCOUNT_LIST_MODEL_N_COLUMNS, "TNY_GTK_ACCOUNT_LIST_MODEL_N_COLUMNS", "n" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyGtkAccountListModelColumn", values);
  }
  return etype;
}


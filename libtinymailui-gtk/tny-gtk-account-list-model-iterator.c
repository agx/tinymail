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

#include <config.h>

#include <tny-gtk-account-list-model.h>

static GObjectClass *parent_class = NULL;

#include "tny-gtk-account-list-model-iterator-priv.h"

GType _tny_gtk_account_list_model_iterator_get_type (void);

static void
prep_iterator (TnyGtkAccountListModelIterator *self)
{
	if (!self->has_first) 
	{
	    	GtkTreeIter iter;
	    
		self->has_first = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (self->model), 
			self->current);	    	    
		self->has_next = gtk_tree_model_iter_next (GTK_TREE_MODEL (self->model), &iter);
	}
}

void 
_tny_gtk_account_list_model_iterator_set_model (TnyGtkAccountListModelIterator *self, TnyGtkAccountListModel *model)
{
	self->model = model;
	prep_iterator (self);
	return;
}



TnyIterator*
_tny_gtk_account_list_model_iterator_new (TnyGtkAccountListModel *model)
{
	TnyGtkAccountListModelIterator *self = g_object_new (TNY_TYPE_GTK_ACCOUNT_LIST_MODEL_ITERATOR, NULL);

	_tny_gtk_account_list_model_iterator_set_model (self, model);

	return TNY_ITERATOR (self);
}

static void
tny_gtk_account_list_model_iterator_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyGtkAccountListModelIterator *self = (TnyGtkAccountListModelIterator *)instance;

	self->model = NULL;
	self->current = NULL;
	self->has_first = FALSE;
	self->has_next = FALSE;
	self->previous = NULL;
    
	return;
}

static void
tny_gtk_account_list_model_iterator_finalize (GObject *object)
{
	TnyGtkAccountListModelIterator *self = (TnyGtkAccountListModelIterator *) object;
	self->has_first = FALSE;
	self->has_next = FALSE;
    
	(*parent_class->finalize) (object);

	return;
}


static void 
tny_gtk_account_list_model_iterator_next (TnyIterator *self)
{
	TnyGtkAccountListModelIterator *me = (TnyGtkAccountListModelIterator*) self;


	/* Move the iterator to the next node */

	g_mutex_lock (me->model->iterator_lock);
    
	prep_iterator (me);
    
    	me->previous = me->current;
	me->has_next = gtk_tree_model_iter_next (GTK_TREE_MODEL (me->model),
			me->current);
	
	g_mutex_unlock (me->model->iterator_lock);

	return;
}

static void
tny_gtk_account_list_model_iterator_prev (TnyIterator *self)
{
	TnyGtkAccountListModelIterator *me = (TnyGtkAccountListModelIterator*) self;

	if (G_UNLIKELY (!me || !me->current || !me->model))
		return;

	/* Move the iterator to the previous node (or the first) */

	g_mutex_lock (me->model->iterator_lock);
    
	prep_iterator (me);

    	if (me->previous)
		me->current = me->previous;
    
	g_mutex_unlock (me->model->iterator_lock);

	return;
}


static gboolean 
tny_gtk_account_list_model_iterator_is_done (TnyIterator *self)
{
	TnyGtkAccountListModelIterator *me = (TnyGtkAccountListModelIterator*) self;
    	
	if (G_UNLIKELY (!me || !me->model))
		return TRUE;

	return ((!me->has_first) || (!me->has_next));
}


static void
tny_gtk_account_list_model_iterator_first (TnyIterator *self)
{
	TnyGtkAccountListModelIterator *me = (TnyGtkAccountListModelIterator*) self;

    	if (G_UNLIKELY (!me || !me->model))
		return;

	/* Move the iterator to the first node. We know that model always 
	   keeps a reference to the first node, there's nothing wrong with 
	   using that one. */

	g_mutex_lock (me->model->iterator_lock);
    
	prep_iterator (me);
    
	gtk_tree_model_get_iter_first (GTK_TREE_MODEL (me->model), 
			me->current);
    
	g_mutex_unlock (me->model->iterator_lock);

	return;
}


static void
tny_gtk_account_list_model_iterator_nth (TnyIterator *self, guint nth)
{
	TnyGtkAccountListModelIterator *me = (TnyGtkAccountListModelIterator*) self;
	gboolean next = FALSE;
	gint i=0;

    	if (G_UNLIKELY (!me || !me->model))
		return;
    
	/* Move the iterator to the nth node. We'll count from zero,
	   so we start with the first node of which we know the model
	   stored a reference. */

	g_mutex_lock (me->model->iterator_lock);
	next = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (me->model), 
			me->current);
	while (next && i < nth)
	{
		next = gtk_tree_model_iter_next (GTK_TREE_MODEL (me->model), 
				me->current);
		i++;
	}
    
	g_mutex_unlock (me->model->iterator_lock);

	return;
}


static GObject* 
tny_gtk_account_list_model_iterator_get_current (TnyIterator *self)
{
	TnyGtkAccountListModelIterator *me = (TnyGtkAccountListModelIterator*) self;
	TnyAccount *retval;

	if (G_UNLIKELY (!me || !me->model))
		return NULL;

	/* Give the data of the current node */

	g_mutex_lock (me->model->iterator_lock);
	/* This indeed adds a reference (which is good) */
	gtk_tree_model_get (GTK_TREE_MODEL (me->model), me->current,
		TNY_GTK_ACCOUNT_LIST_MODEL_INSTANCE_COLUMN, retval, -1);
	g_mutex_unlock (me->model->iterator_lock);

	return (GObject*)retval;
}


static TnyList* 
tny_gtk_account_list_model_iterator_get_list (TnyIterator *self)
{
	TnyGtkAccountListModelIterator *me = (TnyGtkAccountListModelIterator*) self;

	/* Return the list */

	if (G_UNLIKELY (!me || !me->model))
		return NULL;

       	g_object_ref (G_OBJECT (me->model));

	return TNY_LIST (me->model);
}

static void
tny_iterator_init (TnyIteratorIface *klass)
{

	klass->next_func = tny_gtk_account_list_model_iterator_next;
	klass->prev_func = tny_gtk_account_list_model_iterator_prev;
	klass->first_func = tny_gtk_account_list_model_iterator_first;
	klass->nth_func = tny_gtk_account_list_model_iterator_nth;
	klass->get_current_func = tny_gtk_account_list_model_iterator_get_current;
	klass->get_list_func = tny_gtk_account_list_model_iterator_get_list;
	klass->is_done  = tny_gtk_account_list_model_iterator_is_done;
	
	return;
}

static void 
tny_gtk_account_list_model_iterator_class_init (TnyGtkAccountListModelIteratorClass *klass)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;

	object_class->finalize = tny_gtk_account_list_model_iterator_finalize;

	return;
}

GType 
_tny_gtk_account_list_model_iterator_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyGtkAccountListModelIteratorClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_gtk_account_list_model_iterator_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyGtkAccountListModelIterator),
		  0,      /* n_preallocs */
		  tny_gtk_account_list_model_iterator_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_iterator_info = 
		{
		  (GInterfaceInitFunc) tny_iterator_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyGtkAccountListModelIterator",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_ITERATOR, 
			&tny_iterator_info);
	}

	return type;
}

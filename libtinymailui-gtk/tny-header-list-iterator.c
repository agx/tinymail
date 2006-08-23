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

#include <tny-header-list-model.h>

static GObjectClass *parent_class = NULL;

#include "tny-header-list-iterator-priv.h"
#include "tny-header-list-model-priv.h"

GType _tny_header_list_iterator_get_type (void);



void 
_tny_header_list_iterator_set_model (TnyHeaderListIterator *self, TnyHeaderListModel *model, gboolean lock)
{
	self->model = model;

	/* It's not a list_copy, so don't free this list when 
	   destructing this iterator. Current is used as a ptr
	   to the 'current' GList node. 

	   When the iterator starts, it points to 'start', or,
	   the first node in the list. */

	if (lock)
		g_mutex_lock (self->model->iterator_lock);

	self->current = model->first;

	if (lock)
		g_mutex_unlock (self->model->iterator_lock);

	return;
}



TnyHeaderListIterator*
_tny_header_list_iterator_new (TnyHeaderListModel *model, gboolean lock)
{
	TnyHeaderListIterator *self = g_object_new (TNY_TYPE_HEADER_LIST_ITERATOR, NULL);

	_tny_header_list_iterator_set_model (self, model, lock);

	return self;
}

static void
tny_header_list_iterator_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyHeaderListIterator *self = (TnyHeaderListIterator *)instance;

	self->model = NULL;
	self->current = NULL;

	return;
}

static void
tny_header_list_iterator_finalize (GObject *object)
{
	(*parent_class->finalize) (object);

	return;
}


void 
_tny_header_list_iterator_next_nl (TnyHeaderListIterator *me)
{
	me->current = me->current?g_list_next (me->current):NULL;

	return;
}

static void 
tny_header_list_iterator_next (TnyIteratorIface *self)
{
	TnyHeaderListIterator *me = (TnyHeaderListIterator*) self;

	if (G_UNLIKELY (!me || !me->current || !me->model))
		return;

	/* Move the iterator to the next node */

	g_mutex_lock (me->model->iterator_lock);
	me->current = g_list_next (me->current);
	g_mutex_unlock (me->model->iterator_lock);

	return;
}

void
_tny_header_list_iterator_prev_nl (TnyHeaderListIterator *me)
{
	me->current = me->current?g_list_previous (me->current):NULL;

	return;
}

static void
tny_header_list_iterator_prev (TnyIteratorIface *self)
{
	TnyHeaderListIterator *me = (TnyHeaderListIterator*) self;

	if (G_UNLIKELY (!me || !me->current || !me->model))
		return;

	/* Move the iterator to the previous node */

	g_mutex_lock (me->model->iterator_lock);
	me->current = g_list_previous (me->current);
	g_mutex_unlock (me->model->iterator_lock);

	return;
}

gboolean 
_tny_header_list_iterator_is_done_nl (TnyHeaderListIterator *me)
{
	if (G_UNLIKELY (!me  || !me->model))
		return TRUE;

	return me->current == NULL;
}

static gboolean 
tny_header_list_iterator_is_done (TnyIteratorIface *self)
{
	TnyHeaderListIterator *me = (TnyHeaderListIterator*) self;
	gboolean retval = FALSE;
	
	g_mutex_lock (me->model->iterator_lock);
	
	if (G_UNLIKELY (!me  || !me->model))
		return TRUE;

	retval = (me->current == NULL);
	g_mutex_unlock (me->model->iterator_lock);
	
	return retval;
}


void
_tny_header_list_iterator_first_nl (TnyHeaderListIterator *me)
{
	me->current = me->model?me->model->first:NULL;

	return;
}

static void
tny_header_list_iterator_first (TnyIteratorIface *self)
{
	TnyHeaderListIterator *me = (TnyHeaderListIterator*) self;

	if (G_UNLIKELY (!me || !me->current || !me->model))
		return;

	/* Move the iterator to the first node. We know that model always 
	   keeps a reference to the first node, there's nothing wrong with 
	   using that one. */

	g_mutex_lock (me->model->iterator_lock);
	me->current = me->model->first;
	g_mutex_unlock (me->model->iterator_lock);

	return;
}


void
_tny_header_list_iterator_nth_nl (TnyHeaderListIterator *me, guint nth)
{
	me->current = me->model?g_list_nth (me->model->first, nth):NULL;

	return;
}

static void 
tny_header_list_iterator_nth (TnyIteratorIface *self, guint nth)
{
	TnyHeaderListIterator *me = (TnyHeaderListIterator*) self;

	if (G_UNLIKELY (!me || !me->current || !me->model))
		return;

	/* Move the iterator to the nth node. We'll count from zero,
	   so we start with the first node of which we know the model
	   stored a reference. */

	g_mutex_lock (me->model->iterator_lock);
	me->current = g_list_nth (me->model->first, nth);
	g_mutex_unlock (me->model->iterator_lock);

	return;
}

/* exception: don't ref */
gpointer 
_tny_header_list_iterator_current_nl (TnyHeaderListIterator *me)
{
	return me->current?me->current->data:NULL;
}

static GObject* 
tny_header_list_iterator_current (TnyIteratorIface *self)
{
	TnyHeaderListIterator *me = (TnyHeaderListIterator*) self;
	gpointer retval;

	if (G_UNLIKELY (!me || !me->model))
		return NULL;

	/* Give the data of the current node */

	g_mutex_lock (me->model->iterator_lock);
	retval = (G_UNLIKELY (me->current)) ? me->current->data : NULL;
	g_mutex_unlock (me->model->iterator_lock);

	if (retval)
		g_object_ref (G_OBJECT(retval));	
	return (GObject*)retval;
}


static TnyListIface* 
tny_header_list_iterator_get_list (TnyIteratorIface *self)
{
	TnyHeaderListIterator *me = (TnyHeaderListIterator*) self;

	/* Return the list */

	if (G_UNLIKELY (!me || !me->model))
		return NULL;

       	g_object_ref (G_OBJECT (me->model));

	return TNY_LIST_IFACE (me->model);
}

static void
tny_iterator_iface_init (TnyIteratorIfaceClass *klass)
{

	klass->next_func = tny_header_list_iterator_next;
	klass->prev_func = tny_header_list_iterator_prev;
	klass->first_func = tny_header_list_iterator_first;
	klass->nth_func = tny_header_list_iterator_nth;
	klass->current_func = tny_header_list_iterator_current;
	klass->get_list_func = tny_header_list_iterator_get_list;
	klass->is_done = tny_header_list_iterator_is_done;
	
	return;
}

static void 
tny_header_list_iterator_class_init (TnyHeaderListIteratorClass *klass)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;

	object_class->finalize = tny_header_list_iterator_finalize;

	return;
}

GType 
_tny_header_list_iterator_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyHeaderListIteratorClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_header_list_iterator_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyHeaderListIterator),
		  0,      /* n_preallocs */
		  tny_header_list_iterator_instance_init,    /* instance_init */
		  NULL
		};

		static const GInterfaceInfo tny_iterator_iface_info = 
		{
		  (GInterfaceInitFunc) tny_iterator_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyHeaderListIterator",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_ITERATOR_IFACE, 
			&tny_iterator_iface_info);
	}

	return type;
}

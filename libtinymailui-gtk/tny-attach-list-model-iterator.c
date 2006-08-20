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

#include <tny-attach-list-model.h>

static GObjectClass *parent_class = NULL;

#include "tny-attach-list-model-iterator-priv.h"
#include "tny-attach-list-model-priv.h"

GType _tny_attach_list_model_iterator_get_type (void);


void 
_tny_attach_list_model_iterator_set_model (TnyAttachListModelIterator *self, TnyAttachListModel *model)
{
	self->model = model;
	self->current = model->first;

	return;
}



TnyAttachListModelIterator*
_tny_attach_list_model_iterator_new (TnyAttachListModel *model)
{
	TnyAttachListModelIterator *self = g_object_new (TNY_TYPE_ATTACH_LIST_MODEL_ITERATOR, NULL);

	_tny_attach_list_model_iterator_set_model (self, model);

	return self;
}

static void
tny_attach_list_model_iterator_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyAttachListModelIterator *self = (TnyAttachListModelIterator *)instance;

	self->model = NULL;
	self->current = NULL;

	return;
}

static void
tny_attach_list_model_iterator_finalize (GObject *object)
{
	(*parent_class->finalize) (object);

	return;
}


static void 
tny_attach_list_model_iterator_next (TnyIteratorIface *self)
{
	TnyAttachListModelIterator *me = (TnyAttachListModelIterator*) self;

	if (G_UNLIKELY (!me || !me->current || !me->model))
		return;

	/* Move the iterator to the next node */

	g_mutex_lock (me->model->iterator_lock);
	me->current = g_list_next (me->current);
	g_mutex_unlock (me->model->iterator_lock);

	return;
}

static void
tny_attach_list_model_iterator_prev (TnyIteratorIface *self)
{
	TnyAttachListModelIterator *me = (TnyAttachListModelIterator*) self;

	if (G_UNLIKELY (!me || !me->current || !me->model))
		return;

	/* Move the iterator to the previous node */

	g_mutex_lock (me->model->iterator_lock);
	me->current = g_list_previous (me->current);
	g_mutex_unlock (me->model->iterator_lock);

	return;
}


static gboolean 
tny_attach_list_model_iterator_is_done (TnyIteratorIface *self)
{
	TnyAttachListModelIterator *me = (TnyAttachListModelIterator*) self;
	
	if (G_UNLIKELY (!me || !me->model))
		return TRUE;

	return me->current == NULL;
}



static void
tny_attach_list_model_iterator_first (TnyIteratorIface *self)
{
	TnyAttachListModelIterator *me = (TnyAttachListModelIterator*) self;

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


static void
tny_attach_list_model_iterator_nth (TnyIteratorIface *self, guint nth)
{
	TnyAttachListModelIterator *me = (TnyAttachListModelIterator*) self;

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


static GObject* 
tny_attach_list_model_iterator_current (TnyIteratorIface *self)
{
	TnyAttachListModelIterator *me = (TnyAttachListModelIterator*) self;
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
tny_attach_list_model_iterator_get_list (TnyIteratorIface *self)
{
	TnyAttachListModelIterator *me = (TnyAttachListModelIterator*) self;

	/* Return the list */

	if (G_UNLIKELY (!me || !me->model))
		return NULL;

	return TNY_LIST_IFACE (me->model);
}

static void
tny_iterator_iface_init (TnyIteratorIfaceClass *klass)
{

	klass->next_func = tny_attach_list_model_iterator_next;
	klass->prev_func = tny_attach_list_model_iterator_prev;
	klass->first_func = tny_attach_list_model_iterator_first;
	klass->nth_func = tny_attach_list_model_iterator_nth;
	klass->current_func = tny_attach_list_model_iterator_current;
	klass->get_list_func = tny_attach_list_model_iterator_get_list;
	klass->is_done  = tny_attach_list_model_iterator_is_done;
	
	return;
}

static void 
tny_attach_list_model_iterator_class_init (TnyAttachListModelIteratorClass *klass)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;

	object_class->finalize = tny_attach_list_model_iterator_finalize;

	return;
}

GType 
_tny_attach_list_model_iterator_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyAttachListModelIteratorClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_attach_list_model_iterator_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyAttachListModelIterator),
		  0,      /* n_preallocs */
		  tny_attach_list_model_iterator_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_iterator_iface_info = 
		{
		  (GInterfaceInitFunc) tny_iterator_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyAttachListModelIterator",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_ITERATOR_IFACE, 
			&tny_iterator_iface_info);
	}

	return type;
}

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

#include <tny-msg-header-list-model.h>
#include <tny-msg-header-list-iterator.h>

static GObjectClass *parent_class = NULL;

#include "tny-msg-header-list-model-priv.h"

struct _TnyMsgHeaderListIterator
{
	GObject parent;
	TnyMsgHeaderListModel *model;
	GList *current;
};

struct _TnyMsgHeaderListIteratorClass 
{
	GObjectClass parent;
};

/**
 * tny_msg_header_list_iterator_set_model:
 * @self: a #TnyMsgHeaderListIterator instance
 * @model: The model
 *
 *
 **/
void 
tny_msg_header_list_iterator_set_model (TnyMsgHeaderListIterator *self, TnyMsgHeaderListModel *model)
{



	if (self->model)
		g_object_unref (G_OBJECT (self->model));

	g_object_ref (G_OBJECT (model));

	self->model = model;

	/* It's not a list_copy, so don't free this list when 
	   destructing this iterator. Current is used as a ptr
	   to the 'current' GList node. 

	   When the iterator starts, it points to 'start', or,
	   the first node in the list. */

	g_mutex_lock (self->model->iterator_lock);
	self->current = model->first;
	g_mutex_unlock (self->model->iterator_lock);

	return;
}


/**
 * tny_msg_header_list_iterator_new:
 * @model: The model
 *
 *
 * Return value: a new #TnyMsgHeaderListIteratorIface instance
 **/
TnyMsgHeaderListIterator*
tny_msg_header_list_iterator_new (TnyMsgHeaderListModel *model)
{
	TnyMsgHeaderListIterator *self = g_object_new (TNY_TYPE_MSG_HEADER_LIST_ITERATOR, NULL);

	tny_msg_header_list_iterator_set_model (self, model);

	return self;
}

static void
tny_msg_header_list_iterator_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyMsgHeaderListIterator *self = (TnyMsgHeaderListIterator *)instance;

	self->model = NULL;
	self->current = NULL;

	return;
}

static void
tny_msg_header_list_iterator_finalize (GObject *object)
{
	TnyMsgHeaderListIterator *self = (TnyMsgHeaderListIterator *)object;

	if (self->model)
		g_object_unref (G_OBJECT (self->model));

	(*parent_class->finalize) (object);

	return;
}

void /* Protected method that speeds-up the TnyMsgHeaderListModel type */
_tny_msg_header_list_iterator_travel_to_nth (TnyMsgHeaderListIterator *self, guint cur, guint nth)
{
	if (cur < nth)
		while ((cur++ < nth) && self->current)
			self->current = self->current->next;
	else if (cur > nth)
		while ((cur-- > nth) && self->current)
			self->current = self->current->prev;
}

static gpointer 
tny_msg_header_list_iterator_next (TnyIteratorIface *self)
{
	TnyMsgHeaderListIterator *me = (TnyMsgHeaderListIterator*) self;

	if (G_UNLIKELY (!me || !me->current || !me->model))
		return NULL;

	g_mutex_lock (me->model->iterator_lock);
	me->current = g_list_next (me->current);
	g_mutex_unlock (me->model->iterator_lock);

	return me->current->data;
}

static gpointer 
tny_msg_header_list_iterator_prev (TnyIteratorIface *self)
{
	TnyMsgHeaderListIterator *me = (TnyMsgHeaderListIterator*) self;

	if (G_UNLIKELY (!me || !me->current || !me->model))
		return NULL;

	g_mutex_lock (me->model->iterator_lock);
	me->current = g_list_previous (me->current);
	g_mutex_unlock (me->model->iterator_lock);

	return me->current->data;
}

static gpointer 
tny_msg_header_list_iterator_first (TnyIteratorIface *self)
{
	TnyMsgHeaderListIterator *me = (TnyMsgHeaderListIterator*) self;

	if (G_UNLIKELY (!me || !me->current || !me->model))
		return NULL;

	g_mutex_lock (me->model->iterator_lock);
	me->current = me->model->first;
	g_mutex_unlock (me->model->iterator_lock);

	return me->current->data;
}


static gpointer 
tny_msg_header_list_iterator_nth (TnyIteratorIface *self, guint nth)
{
	TnyMsgHeaderListIterator *me = (TnyMsgHeaderListIterator*) self;

	if (G_UNLIKELY (!me || !me->current || !me->model))
		return NULL;

	g_mutex_lock (me->model->iterator_lock);
	me->current = g_list_nth (me->model->first, nth);
	g_mutex_unlock (me->model->iterator_lock);

	return me->current->data;
}


static gpointer 
tny_msg_header_list_iterator_current (TnyIteratorIface *self)
{
	TnyMsgHeaderListIterator *me = (TnyMsgHeaderListIterator*) self;
	gpointer retval;

	if (G_UNLIKELY (!me || !me->model))
		return NULL;

	g_mutex_lock (me->model->iterator_lock);
	retval = (G_UNLIKELY (me->current)) ? me->current->data : NULL;
	g_mutex_unlock (me->model->iterator_lock);

	return retval;
}

static gboolean 
tny_msg_header_list_iterator_has_next (TnyIteratorIface *self)
{
	TnyMsgHeaderListIterator *me = (TnyMsgHeaderListIterator*) self;
	gboolean retval;

	if (G_UNLIKELY (!me || !me->model))
		return FALSE;

	g_mutex_lock (me->model->iterator_lock);
	retval = (G_UNLIKELY (me->current) && me->current->next);
	g_mutex_unlock (me->model->iterator_lock);

	return retval;
}

static TnyListIface* 
tny_msg_header_list_iterator_get_list (TnyIteratorIface *self)
{
	TnyMsgHeaderListIterator *me = (TnyMsgHeaderListIterator*) self;

	if (G_UNLIKELY (!me || !me->model))
		return NULL;

	return TNY_LIST_IFACE (me->model);
}

static void
tny_iterator_iface_init (TnyIteratorIfaceClass *klass)
{

	klass->next_func = tny_msg_header_list_iterator_next;
	klass->prev_func = tny_msg_header_list_iterator_prev;
	klass->first_func = tny_msg_header_list_iterator_first;
	klass->nth_func = tny_msg_header_list_iterator_nth;
	klass->current_func = tny_msg_header_list_iterator_current;
	klass->has_next_func = tny_msg_header_list_iterator_has_next;
	klass->get_list_func = tny_msg_header_list_iterator_get_list;

	return;
}

static void 
tny_msg_header_list_iterator_class_init (TnyMsgHeaderListIteratorClass *klass)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;

	object_class->finalize = tny_msg_header_list_iterator_finalize;

	return;
}

GType 
tny_msg_header_list_iterator_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgHeaderListIteratorClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_msg_header_list_iterator_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMsgHeaderListIterator),
		  0,      /* n_preallocs */
		  tny_msg_header_list_iterator_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_iterator_iface_info = 
		{
		  (GInterfaceInitFunc) tny_iterator_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyMsgHeaderListIterator",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_ITERATOR_IFACE, 
			&tny_iterator_iface_info);
	}

	return type;
}

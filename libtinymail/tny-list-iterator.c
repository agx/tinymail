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

#include <glib.h>
#include <glib/gi18n-lib.h>

#include <tny-list.h>
#include "tny-list-priv.h"
#include "tny-list-iterator-priv.h"

static GObjectClass *parent_class = NULL;


void 
_tny_list_iterator_set_model (TnyListIterator *self, TnyList *model)
{
	TnyListPriv *lpriv;

	self->model = model;

	lpriv = TNY_LIST_GET_PRIVATE (self->model);

	g_mutex_lock (lpriv->iterator_lock);
	self->current = lpriv->first;
	g_mutex_unlock (lpriv->iterator_lock);

	return;
}



TnyListIterator*
_tny_list_iterator_new (TnyList *model)
{
	TnyListIterator *self = g_object_new (TNY_TYPE_LIST_ITERATOR, NULL);

	_tny_list_iterator_set_model (self, model);

	return self;
}

static void
tny_list_iterator_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyListIterator *self = (TnyListIterator *)instance;

	self->model = NULL;
	self->current = NULL;

	return;
}

static void
tny_list_iterator_finalize (GObject *object)
{
	TnyListIterator *self = (TnyListIterator *)object;

	(*parent_class->finalize) (object);

	return;
}


static gpointer 
tny_list_iterator_next (TnyIteratorIface *self)
{
	TnyListIterator *me = (TnyListIterator*) self;
	TnyListPriv *lpriv;

	if (G_UNLIKELY (!me || !me->current || !me->model))
		return NULL;

	lpriv = TNY_LIST_GET_PRIVATE (me->model);

	g_mutex_lock (lpriv->iterator_lock);
	me->current = g_list_next (me->current);
	g_mutex_unlock (lpriv->iterator_lock);

	return me->current->data;
}

static gpointer 
tny_list_iterator_prev (TnyIteratorIface *self)
{
	TnyListIterator *me = (TnyListIterator*) self;
	TnyListPriv *lpriv;

	if (G_UNLIKELY (!me || !me->current || !me->model))
		return NULL;

	lpriv = TNY_LIST_GET_PRIVATE (me->model);

	g_mutex_lock (lpriv->iterator_lock);
	me->current = g_list_previous (me->current);
	g_mutex_unlock (lpriv->iterator_lock);

	return me->current->data;
}

static gpointer 
tny_list_iterator_first (TnyIteratorIface *self)
{
	TnyListIterator *me = (TnyListIterator*) self;
	TnyListPriv *lpriv;

	if (G_UNLIKELY (!me || !me->current || !me->model))
		return NULL;

	lpriv = TNY_LIST_GET_PRIVATE (me->model);

	g_mutex_lock (lpriv->iterator_lock);
	me->current = lpriv->first;
	g_mutex_unlock (lpriv->iterator_lock);

	return me->current->data;
}


static gpointer 
tny_list_iterator_nth (TnyIteratorIface *self, guint nth)
{
	TnyListIterator *me = (TnyListIterator*) self;
	TnyListPriv *lpriv;

	if (G_UNLIKELY (!me || !me->current || !me->model))
		return NULL;

	lpriv = TNY_LIST_GET_PRIVATE (me->model);

	/* Move the iterator to the nth node. We'll count from zero,
	   so we start with the first node of which we know the model
	   stored a reference. */

	g_mutex_lock (lpriv->iterator_lock);
	me->current = g_list_nth (lpriv->first, nth);
	g_mutex_unlock (lpriv->iterator_lock);

	return me->current->data;
}


static gpointer 
tny_list_iterator_current (TnyIteratorIface *self)
{
	TnyListIterator *me = (TnyListIterator*) self;
	gpointer retval;
	TnyListPriv *lpriv;

	if (G_UNLIKELY (!me || !me->model))
		return NULL;

	lpriv = TNY_LIST_GET_PRIVATE (me->model);

	g_mutex_lock (lpriv->iterator_lock);
	retval = (G_UNLIKELY (me->current)) ? me->current->data : NULL;
	g_mutex_unlock (lpriv->iterator_lock);

	return retval;
}

static gboolean 
tny_list_iterator_has_next (TnyIteratorIface *self)
{
	TnyListIterator *me = (TnyListIterator*) self;
	gboolean retval;
	TnyListPriv *lpriv;

	if (G_UNLIKELY (!me || !me->model))
		return FALSE;

	lpriv = TNY_LIST_GET_PRIVATE (me->model);

	g_mutex_lock (lpriv->iterator_lock);
	retval = (G_UNLIKELY (me->current) && me->current->next);
	g_mutex_unlock (lpriv->iterator_lock);

	return retval;
}

static TnyListIface* 
tny_list_iterator_get_list (TnyIteratorIface *self)
{
	TnyListIterator *me = (TnyListIterator*) self;

	/* Return the list */

	if (G_UNLIKELY (!me || !me->model))
		return NULL;

	return TNY_LIST_IFACE (me->model);
}

static void
tny_iterator_iface_init (TnyIteratorIfaceClass *klass)
{

	klass->next_func = tny_list_iterator_next;
	klass->prev_func = tny_list_iterator_prev;
	klass->first_func = tny_list_iterator_first;
	klass->nth_func = tny_list_iterator_nth;
	klass->current_func = tny_list_iterator_current;
	klass->has_next_func = tny_list_iterator_has_next;
	klass->get_list_func = tny_list_iterator_get_list;

	return;
}

static void 
tny_list_iterator_class_init (TnyListIteratorClass *klass)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;

	object_class->finalize = tny_list_iterator_finalize;

	return;
}

GType 
_tny_list_iterator_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyListIteratorClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_list_iterator_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyListIterator),
		  0,      /* n_preallocs */
		  tny_list_iterator_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_iterator_iface_info = 
		{
		  (GInterfaceInitFunc) tny_iterator_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyListIterator",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_ITERATOR_IFACE, 
			&tny_iterator_iface_info);
	}

	return type;
}

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

#include <tny-iterator.h>

/**
 * tny_iterator_next:
 * @self: A #TnyIterator instance
 *
 * Moves the iterator to the next node 
 *
 **/
void 
tny_iterator_next (TnyIterator *self)
{
#ifdef DEBUG
	if (!TNY_ITERATOR_GET_IFACE (self)->next_func)
		g_critical ("You must implement tny_iterator_next\n");
#endif
	TNY_ITERATOR_GET_IFACE (self)->next_func (self);
}

/**
 * tny_iterator_prev:
 * @self: A #TnyIterator instance
 *
 * Moves the iterator to the previous node
 *
 **/
void
tny_iterator_prev (TnyIterator *self)
{
#ifdef DEBUG
	if (!TNY_ITERATOR_GET_IFACE (self)->prev_func)
		g_critical ("You must implement tny_iterator_prev\n");
#endif
	TNY_ITERATOR_GET_IFACE (self)->prev_func (self);
}


/**
 * tny_iterator_first:
 * @self: A #TnyIterator instance
 *
 * Moves the iterator to the first node
 *
 **/
void 
tny_iterator_first (TnyIterator *self)
{
#ifdef DEBUG
	if (!TNY_ITERATOR_GET_IFACE (self)->first_func)
		g_critical ("You must implement tny_iterator_first\n");
#endif

	TNY_ITERATOR_GET_IFACE (self)->first_func (self);
}

/**
 * tny_iterator_nth:
 * @self: A #TnyIterator instance
 * @nth: The nth position
 *
 * Moves the iterator to the nth node
 *
 **/
void
tny_iterator_nth (TnyIterator *self, guint nth)
{
#ifdef DEBUG
	if (!TNY_ITERATOR_GET_IFACE (self)->nth_func)
		g_critical ("You must implement tny_iterator_nth\n");
#endif
	TNY_ITERATOR_GET_IFACE (self)->nth_func (self, nth);
}


/**
 * tny_iterator_current:
 * @self: A #TnyIterator instance
 *
 * Does not move the iterator. Returns the object at the current position.
 * The returned object should be unreferenced after use.
 *
 * Return value: the currect object
 *
 **/
GObject* 
tny_iterator_current (TnyIterator *self)
{
#ifdef DEBUG
	if (!TNY_ITERATOR_GET_IFACE (self)->current_func)
		g_critical ("You must implement tny_iterator_current\n");
#endif
	return TNY_ITERATOR_GET_IFACE (self)->current_func (self);
}



/**
 * tny_iterator_is_done:
 * @self: A #TnyIterator instance
 *
 * Does the iterator point to some valid list item. You can use this property
 * to make loops like:
 * 
 * <informalexample><programlisting>
 * TnyList *list = tny_simple_list_new ();
 * TnyIterator *iter = tny_list_create_iterator (list);
 * while (!tny_iterator_is_done (iter))
 *    tny_iterator_next (iter);
 * g_object_unref (G_OBJECT (iter));
 * g_object_unref (G_OBJECT (list));
 * </programlisting></informalexample>
 *
 * Return value: TRUE if it points to a valid list item, FALSE otherwise
 *
 **/
gboolean
tny_iterator_is_done (TnyIterator *self)
{
#ifdef DEBUG
	if (!TNY_ITERATOR_GET_IFACE (self)->is_done)
		g_critical ("You must implement tny_iterator_is_done\n");
#endif

	return TNY_ITERATOR_GET_IFACE (self)->is_done (self);
}



/**
 * tny_iterator_get_list:
 * @self: A #TnyIterator instance
 *
 * Does not move the iterator. Returns the list of which this iterator is an
 * iterator. The returned list object should be unreferenced after use.
 *
 * Return value: The #TnyList instance being iterated
 *
 **/
TnyList*
tny_iterator_get_list (TnyIterator *self)
{
#ifdef DEBUG
	if (!TNY_ITERATOR_GET_IFACE (self)->get_list_func)
		g_critical ("You must implement tny_iterator_get_list\n");
#endif

	return TNY_ITERATOR_GET_IFACE (self)->get_list_func (self);
}



static void
tny_iterator_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_iterator_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyIteratorIface),
		  tny_iterator_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL,    /* instance_init */
		  NULL
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyIterator", &info, 0);

		/* g_type_interface_add_prerequisite (type, G_TYPE_OBJECT); */
	}

	return type;
}


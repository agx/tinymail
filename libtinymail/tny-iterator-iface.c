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

#include <tny-iterator-iface.h>

/**
 * tny_iterator_iface_next:
 * @self: A #TnyIteratorIface instance
 *
 * Moves the iterator to the next node 
 *
 **/
void 
tny_iterator_iface_next (TnyIteratorIface *self)
{
#ifdef DEBUG
	if (!TNY_ITERATOR_IFACE_GET_CLASS (self)->next_func)
		g_critical ("You must implement tny_iterator_iface_next\n");
#endif
	TNY_ITERATOR_IFACE_GET_CLASS (self)->next_func (self);
}

/**
 * tny_iterator_iface_prev:
 * @self: A #TnyIteratorIface instance
 *
 * Moves the iterator to the previous node
 *
 **/
void
tny_iterator_iface_prev (TnyIteratorIface *self)
{
#ifdef DEBUG
	if (!TNY_ITERATOR_IFACE_GET_CLASS (self)->prev_func)
		g_critical ("You must implement tny_iterator_iface_prev\n");
#endif
	TNY_ITERATOR_IFACE_GET_CLASS (self)->prev_func (self);
}


/**
 * tny_iterator_iface_first:
 * @self: A #TnyIteratorIface instance
 *
 * Moves the iterator to the first node
 *
 **/
void 
tny_iterator_iface_first (TnyIteratorIface *self)
{
#ifdef DEBUG
	if (!TNY_ITERATOR_IFACE_GET_CLASS (self)->first_func)
		g_critical ("You must implement tny_iterator_iface_first\n");
#endif

	TNY_ITERATOR_IFACE_GET_CLASS (self)->first_func (self);
}

/**
 * tny_iterator_iface_nth:
 * @self: A #TnyIteratorIface instance
 * @nth: The nth position
 *
 * Moves the iterator to the nth node
 *
 **/
void
tny_iterator_iface_nth (TnyIteratorIface *self, guint nth)
{
#ifdef DEBUG
	if (!TNY_ITERATOR_IFACE_GET_CLASS (self)->nth_func)
		g_critical ("You must implement tny_iterator_iface_nth\n");
#endif
	TNY_ITERATOR_IFACE_GET_CLASS (self)->nth_func (self, nth);
}


/**
 * tny_iterator_iface_current:
 * @self: A #TnyIteratorIface instance
 *
 * Does not move the iterator. Returns the object at the current position.
 * The returned object should be unreferenced after use.
 *
 * Return value: the currect object
 *
 **/
GObject* 
tny_iterator_iface_current (TnyIteratorIface *self)
{
#ifdef DEBUG
	if (!TNY_ITERATOR_IFACE_GET_CLASS (self)->current_func)
		g_critical ("You must implement tny_iterator_iface_current\n");
#endif
	return TNY_ITERATOR_IFACE_GET_CLASS (self)->current_func (self);
}



/**
 * tny_iterator_iface_is_done:
 * @self: A #TnyIteratorIface instance
 *
 * Does the iterator point to some valid list item. You can use this property
 * to make loops like:
 * 
 * <informalexample><programlisting>
 * TnyListIface *list = tny_list_new ();
 * TnyIteratorIface *iter = tny_list_iface_create_iterator (list);
 * while (!tny_iterator_iface_is_done (iter))
 *    tny_iterator_iface_next (iter);
 * g_object_unref (G_OBJECT (iter));
 * g_object_unref (G_OBJECT (list));
 * </programlisting></informalexample>
 *
 * Return value: TRUE if it points to a valid list item, FALSE otherwise
 *
 **/
gboolean
tny_iterator_iface_is_done (TnyIteratorIface *self)
{
#ifdef DEBUG
	if (!TNY_ITERATOR_IFACE_GET_CLASS (self)->is_done)
		g_critical ("You must implement tny_iterator_iface_is_done\n");
#endif

	return TNY_ITERATOR_IFACE_GET_CLASS (self)->is_done (self);
}



/**
 * tny_iterator_iface_get_list:
 * @self: A #TnyIteratorIface instance
 *
 * Does not move the iterator. Returns the list of which this iterator is an
 * iterator. The returned list object should be unreferenced after use.
 *
 * Return value: The #TnyListIface instance being iterated
 *
 **/
TnyListIface*
tny_iterator_iface_get_list (TnyIteratorIface *self)
{
#ifdef DEBUG
	if (!TNY_ITERATOR_IFACE_GET_CLASS (self)->get_list_func)
		g_critical ("You must implement tny_iterator_iface_get_list\n");
#endif

	return TNY_ITERATOR_IFACE_GET_CLASS (self)->get_list_func (self);
}



static void
tny_iterator_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_iterator_iface_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyIteratorIfaceClass),
		  tny_iterator_iface_base_init,   /* base_init */
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
			"TnyIteratorIface", &info, 0);

		/* g_type_interface_add_prerequisite (type, G_TYPE_OBJECT); */
	}

	return type;
}


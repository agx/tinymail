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

#include <tny-list-iface.h>

/**
 * tny_list_iface_length:
 * @self: A #TnyListIface instance
 *
 * Return value: the length of the list
 **/
guint
tny_list_iface_length (TnyListIface *self)
{
#ifdef DEBUG
	if (!TNY_LIST_IFACE_GET_CLASS (self)->length_func)
		g_critical ("You must implement tny_list_iface_length\n");
#endif
	return TNY_LIST_IFACE_GET_CLASS (self)->length_func (self);
}

/**
 * tny_list_iface_prepend:
 * @self: A #TnyListIface instance
 * @item: the item to prepend
 *
 * Prepends an item to a list
 *
 * Implementers: if you have to choose, make this one the fast one
 *
 **/
void
tny_list_iface_prepend (TnyListIface *self, GObject* item)
{
#ifdef DEBUG
	if (!TNY_LIST_IFACE_GET_CLASS (self)->prepend_func)
		g_critical ("You must implement tny_list_iface_prepend\n");
#endif

	TNY_LIST_IFACE_GET_CLASS (self)->prepend_func (self, item);
	return;
}

/**
 * tny_list_iface_append:
 * @self: A #TnyListIface instance
 * @item: the item to append
 *
 * Appends an item to a list
 *
 * Implementers: if you have to choose, make the prepend one the fast one
 *
 **/
void 
tny_list_iface_append (TnyListIface *self, GObject* item)
{
#ifdef DEBUG
	if (!TNY_LIST_IFACE_GET_CLASS (self)->append_func)
		g_critical ("You must implement tny_list_iface_append\n");
#endif

	TNY_LIST_IFACE_GET_CLASS (self)->append_func (self, item);
	return;
}

/**
 * tny_list_iface_remove:
 * @self: A #TnyListIface instance
 * @item: the item to remove
 *
 * Removes an item from a list
 *
 * Removing a item might invalidate all existing iterators or put them in an
 * unknown and unspecified state. You'll need to recreate the iterator(s) if you
 * remove an item to be certain.
 *
 * There's no guarantee whatsoever that existing iterators of 'self' will be
 * valid after this method returned.
 *
 **/
void 
tny_list_iface_remove (TnyListIface *self, GObject* item)
{
#ifdef DEBUG
	if (!TNY_LIST_IFACE_GET_CLASS (self)->remove_func)
		g_critical ("You must implement tny_list_iface_remove\n");
#endif

	TNY_LIST_IFACE_GET_CLASS (self)->remove_func (self, item);
	return;
}

/**
 * tny_list_iface_create_iterator:
 * @self: A #TnyListIface instance
 *
 * Creates a new iterator instance for the list. 
 *
 * An iterator is a position indicator for a list. It keeps the position
 * state of a list iteration. The list itself does not keep any position 
 * information. This makes it possible to have multiple list-iterations by
 * consuming multiple iterator instances simultanously.
 *
 * The reason why the method isn't called get_iterator is because it's a
 * object creation method. It's not a property. It effectively creates a new
 * instance of an iterator. The returned iterator object should (therefore) be
 * unreferenced after use.
 * 
 * Return value: A new iterator for this list
 *
 **/
TnyIteratorIface* 
tny_list_iface_create_iterator (TnyListIface *self)
{
#ifdef DEBUG
	if (!TNY_LIST_IFACE_GET_CLASS (self)->create_iterator_func)
		g_critical ("You must implement tny_list_iface_create_iterator\n");
#endif

	return TNY_LIST_IFACE_GET_CLASS (self)->create_iterator_func (self);
}

/**
 * tny_list_iface_foreach:
 * @self: A #TnyListIface instance
 * @func: the function to call with each element's data.
 * @user_data: user data to pass to the function.
 *
 * Calls a function for each element of a #TnyListIface. It will use an internal
 * iteration which you don't have to worry about. 
 *
 * The purpose of this method is to have a fast foreach iteration. Using this
 * is faster than inventing your own foreach loop using the is_done and next
 * methods. The order is guaranteed to be the first element first, the last 
 * element last. It's guaranteed that all current items will be iterated.
 *
 * In the func implementation and during the foreach operation you shouldn't
 * append, remove nor prepend items to the list. In multithreaded environments
 * it's advisable to introduce a lock when using this functionality. 
 *
 **/
void 
tny_list_iface_foreach (TnyListIface *self, GFunc func, gpointer user_data)
{
#ifdef DEBUG
	if (!TNY_LIST_IFACE_GET_CLASS (self)->foreach_func)
		g_critical ("You must implement tny_list_iface_foreach\n");
#endif

	TNY_LIST_IFACE_GET_CLASS (self)->foreach_func (self, func, user_data);
	return;
}


/**
 * tny_list_iface_copy:
 * @self: A #TnyListIface instance
 *
 * Creates a copy of the list. It doesn't copy the items. It, however, creates
 * a new list with new references to the same items.
 *
 * Because it's a new instance, the returned list object should be unreferenced
 * after use.
 *
 * Return value: A copy of this list
 *
 **/
TnyListIface*
tny_list_iface_copy (TnyListIface *self)
{
#ifdef DEBUG
	if (!TNY_LIST_IFACE_GET_CLASS (self)->copy_func)
		g_critical ("You must implement tny_list_iface_copy\n");
#endif

	return TNY_LIST_IFACE_GET_CLASS (self)->copy_func (self);
}

static void
tny_list_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_list_iface_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyListIfaceClass),
		  tny_list_iface_base_init,   /* base_init */
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
			"TnyListIface", &info, 0);

		/* g_type_interface_add_prerequisite (type, G_TYPE_OBJECT); */
	}

	return type;
}


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

#include <tny-list.h>

/**
 * tny_list_length:
 * @self: A #TnyList instance
 *
 * Return value: the length of the list
 **/
guint
tny_list_length (TnyList *self)
{
#ifdef DEBUG
	if (!TNY_LIST_GET_IFACE (self)->length_func)
		g_critical ("You must implement tny_list_length\n");
#endif
	return TNY_LIST_GET_IFACE (self)->length_func (self);
}

/**
 * tny_list_prepend:
 * @self: A #TnyList instance
 * @item: the item to prepend
 *
 * Prepends an item to a list
 *
 * Implementers: if you have to choose, make this one the fast one
 *
 **/
void
tny_list_prepend (TnyList *self, GObject* item)
{
#ifdef DEBUG
	if (!TNY_LIST_GET_IFACE (self)->prepend_func)
		g_critical ("You must implement tny_list_prepend\n");
#endif

	TNY_LIST_GET_IFACE (self)->prepend_func (self, item);
	return;
}

/**
 * tny_list_append:
 * @self: A #TnyList instance
 * @item: the item to append
 *
 * Appends an item to a list
 *
 * Implementers: if you have to choose, make the prepend one the fast one
 *
 **/
void 
tny_list_append (TnyList *self, GObject* item)
{
#ifdef DEBUG
	if (!TNY_LIST_GET_IFACE (self)->append_func)
		g_critical ("You must implement tny_list_append\n");
#endif

	TNY_LIST_GET_IFACE (self)->append_func (self, item);
	return;
}

/**
 * tny_list_remove:
 * @self: A #TnyList instance
 * @item: the item to remove
 *
 * Removes an item from a list.  Removing a item might invalidate all existing
 * iterators or put them in an unknown and unspecified state. You'll need to 
 * recreate the iterator(s) if you remove an item to be certain.
 *
 * If you want to clear a list, consider using the tny_list_foreach or simply
 * destroy the list instance and construct a new one.
 *
 * There's no guarantee whatsoever that existing iterators of @self will be
 * valid after this method returned.
 *
 **/
void 
tny_list_remove (TnyList *self, GObject* item)
{
#ifdef DEBUG
	if (!TNY_LIST_GET_IFACE (self)->remove_func)
		g_critical ("You must implement tny_list_remove\n");
#endif

	TNY_LIST_GET_IFACE (self)->remove_func (self, item);
	return;
}

/**
 * tny_list_create_iterator:
 * @self: A #TnyList instance
 *
 * Creates a new iterator instance for the list. 
 *
 * An iterator is a position indicator for a list. It keeps the position
 * state of a list iteration. The list itself does not keep any position 
 * information. Consuming multiple iterator instances makes it possible to
 * have multiple list iterations simultanously (i.e. multiple threads or in
 * in a loop that simultanously works with multiple position states in a
 * single list).
 *
 * Example:
 * <informalexample><programlisting>
 * TnyList *list = tny_simple_list_new ();
 * TnyIterator *iter1 = tny_list_create_iterator (list);
 * TnyIterator *iter2 = tny_list_create_iterator (list);
 * while (!tny_iterator_is_done (iter1))
 * {
 *	while (!tny_iterator_is_done (iter2))
 *		tny_iterator_next (iter2);
 *	tny_iterator_next (iter1);
 * }
 * g_object_unref (G_OBJECT (iter1));
 * g_object_unref (G_OBJECT (iter2));
 * g_object_unref (G_OBJECT (list));
 * </programlisting></informalexample>
 *
 * The reason why the method isn't called get_iterator is because it's a
 * object creation method. It's not a property. It effectively creates a new
 * instance of an iterator. The returned iterator object should (therefore) be
 * unreferenced after use.
 * 
 * Return value: A new iterator for this list
 *
 **/
TnyIterator* 
tny_list_create_iterator (TnyList *self)
{
#ifdef DEBUG
	if (!TNY_LIST_GET_IFACE (self)->create_iterator_func)
		g_critical ("You must implement tny_list_create_iterator\n");
#endif

	return TNY_LIST_GET_IFACE (self)->create_iterator_func (self);
}

/**
 * tny_list_foreach:
 * @self: A #TnyList instance
 * @func: the function to call with each element's data.
 * @user_data: user data to pass to the function.
 *
 * Calls a function for each element of a #TnyList. It will use an internal
 * iteration which you don't have to worry about. 
 *
 * <informalexample><programlisting>
 * static void
 * list_foreach_item (TnyHeader *header, gpointer user_data)
 * {
 *	g_print ("%s\n", tny_header_get_subject (header));
 * }
 * </programlisting></informalexample>
 *
 * <informalexample><programlisting>
 * TnyFolder *folder = ...
 * TnyList *headers = tny_simple_list_new ();
 * tny_folder_get_headers (folder, headers, FALSE);
 * tny_list_foreach (headers, list_foreach_item, NULL);
 * g_object_unref (G_OBJECT (list));
 * </programlisting></informalexample>
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
tny_list_foreach (TnyList *self, GFunc func, gpointer user_data)
{
#ifdef DEBUG
	if (!TNY_LIST_GET_IFACE (self)->foreach_func)
		g_critical ("You must implement tny_list_foreach\n");
#endif

	TNY_LIST_GET_IFACE (self)->foreach_func (self, func, user_data);
	return;
}


/**
 * tny_list_copy:
 * @self: A #TnyList instance
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
TnyList*
tny_list_copy (TnyList *self)
{
#ifdef DEBUG
	if (!TNY_LIST_GET_IFACE (self)->copy_func)
		g_critical ("You must implement tny_list_copy\n");
#endif

	return TNY_LIST_GET_IFACE (self)->copy_func (self);
}

static void
tny_list_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_list_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyListIface),
		  tny_list_base_init,   /* base_init */
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
			"TnyList", &info, 0);
	}

	return type;
}


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
#include <glib/gi18n-lib.h>

#include <tny-list-iface.h>
#include <tny-iterator-iface.h>
#include <tny-list.h>


static GObjectClass *parent_class;

#include "tny-list-priv.h"
#include "tny-list-iterator-priv.h"


static void
tny_list_append (TnyListIface *self, gpointer item)
{
	TnyListPriv *priv = TNY_LIST_GET_PRIVATE (self);

	g_mutex_lock (priv->iterator_lock);
	priv->first = g_list_append (priv->first, item);
	g_mutex_unlock (priv->iterator_lock);

	return;
}

static void
tny_list_prepend (TnyListIface *self, gpointer item)
{
	TnyListPriv *priv = TNY_LIST_GET_PRIVATE (self);

	g_mutex_lock (priv->iterator_lock);
	priv->first = g_list_prepend (priv->first, item);
	g_mutex_unlock (priv->iterator_lock);

	return;
}


static guint
tny_list_length (TnyListIface *self)
{
	TnyListPriv *priv = TNY_LIST_GET_PRIVATE (self);
	guint retval = 0;

	g_mutex_lock (priv->iterator_lock);
	retval = priv->first?g_list_length (priv->first):0;
	g_mutex_unlock (priv->iterator_lock);

	return retval;
}

static void
tny_list_remove (TnyListIface *self, gpointer item)
{
	TnyListPriv *priv = TNY_LIST_GET_PRIVATE (self);

	g_mutex_lock (priv->iterator_lock);

	priv->first = g_list_remove (priv->first, item);

	if (G_IS_OBJECT (item))
		g_object_unref (G_OBJECT (item));

	g_mutex_unlock (priv->iterator_lock);

	return;
}

static TnyIteratorIface*
tny_list_create_iterator (TnyListIface *self)
{
	return TNY_ITERATOR_IFACE (_tny_list_iterator_new (TNY_LIST (self)));
}

static TnyListIface*
tny_list_copy_the_list (TnyListIface *self)
{
	TnyList *me = (TnyList*)self;
	TnyList *copy = g_object_new (TNY_TYPE_LIST, NULL);

	TnyListPriv *priv = TNY_LIST_GET_PRIVATE (self);
	TnyListPriv *cpriv = TNY_LIST_GET_PRIVATE (copy);

	g_mutex_lock (priv->iterator_lock);
	GList *list_copy = g_list_copy (priv->first);
	cpriv->first = list_copy;
	g_mutex_unlock (priv->iterator_lock);

	return TNY_LIST_IFACE (copy);
}

static void 
tny_list_foreach_in_the_list (TnyListIface *self, GFunc func, gpointer user_data)
{
	TnyListPriv *priv = TNY_LIST_GET_PRIVATE (self);

	g_mutex_lock (priv->iterator_lock);
	g_list_foreach (priv->first, func, user_data);
	g_mutex_unlock (priv->iterator_lock);

	return;
}

static void
tny_list_iface_init (TnyListIfaceClass *klass)
{
	klass->length_func = tny_list_length;
	klass->prepend_func = tny_list_prepend;
	klass->append_func = tny_list_append;
	klass->remove_func = tny_list_remove;
	klass->create_iterator_func = tny_list_create_iterator;
	klass->copy_func = tny_list_copy_the_list;
	klass->foreach_func = tny_list_foreach_in_the_list;

	return;
}

static void
destroy_item (gpointer item, gpointer user_data)
{
	if (item && G_IS_OBJECT (item))
		g_object_unref (G_OBJECT (item));
}

static void
tny_list_finalize (GObject *object)
{
	TnyListPriv *priv = TNY_LIST_GET_PRIVATE (object);

	g_mutex_lock (priv->iterator_lock);

	if (priv->first)
	{
		g_list_foreach (priv->first, destroy_item, NULL);
		g_list_free (priv->first);
		priv->first = NULL;
	}
	g_mutex_unlock (priv->iterator_lock);

	g_mutex_free (priv->iterator_lock);
	priv->iterator_lock = NULL;

	parent_class->finalize (object);

	return;
}


static void
tny_list_class_init (TnyListClass *klass)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *)klass;
	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = tny_list_finalize;

	g_type_class_add_private (object_class, sizeof (TnyListPriv));

	return;
}

static void
tny_list_init (TnyList *self)
{
	TnyListPriv *priv = TNY_LIST_GET_PRIVATE (self);

	priv->iterator_lock = g_mutex_new ();
	priv->first = NULL;

	return;
}


/**
 * tny_list_new:
 * 
 * Return value: A general purpose #TnyListIface instance
 **/
TnyListIface*
tny_list_new (void)
{
	TnyList *self = g_object_new (TNY_TYPE_LIST, NULL);

	return TNY_LIST_IFACE (self);
}

GType
tny_list_get_type (void)
{
	static GType object_type = 0;

	if (G_UNLIKELY(object_type == 0))
	{
		static const GTypeInfo object_info = 
		{
			sizeof (TnyListClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) tny_list_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (TnyList),
			0,              /* n_preallocs */
			(GInstanceInitFunc) tny_list_init
		};

		static const GInterfaceInfo tny_list_iface_info = {
			(GInterfaceInitFunc) tny_list_iface_init,
			NULL,
			NULL
		};

		object_type = g_type_register_static (G_TYPE_OBJECT, 
						"TnyList", &object_info, 0);

		g_type_add_interface_static (object_type, TNY_TYPE_LIST_IFACE,
					     &tny_list_iface_info);

	}

	return object_type;
}

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

#include <tny-msg-folder.h>

#include <tny-list-iface.h>
#include <tny-iterator-iface.h>
#include <tny-msg-folder-iface.h>


static GObjectClass *parent_class;

#include "tny-msg-folder-list-priv.h"
#include "tny-msg-folder-list-iterator-priv.h"
#include "tny-msg-folder-priv.h"

static void
destroy_folder (gpointer data, gpointer user_data)
{
	g_object_unref (G_OBJECT (data));
	return;
}


static void
tny_msg_folder_list_append (TnyListIface *self, gpointer item)
{
	g_warning (_("Cannot add folders to a folder yet\n"));
}

static void
tny_msg_folder_list_prepend (TnyListIface *self, gpointer item)
{
	tny_msg_folder_list_append (self, item);
}

void
_tny_msg_folder_list_intern_prepend (TnyMsgFolderList *self, TnyMsgFolderIface *item)
{
	TnyMsgFolderPriv *priv = NULL;

	if (self->pfolder)
		priv = TNY_MSG_FOLDER_GET_PRIVATE (self->pfolder);

	/* Append something to the list */

	g_mutex_lock (self->iterator_lock);
	if (self->pfolder)
		g_mutex_lock (priv->folders_lock);

	self->first = g_list_prepend (self->first, item);
	
	g_object_ref (G_OBJECT (item));

	if (self->pfolder)
		g_mutex_unlock (priv->folders_lock);
	g_mutex_unlock (self->iterator_lock);

	/* Tell the observers */
	if (self->pfolder)
		g_signal_emit (self->pfolder, tny_msg_folder_iface_signals [FOLDER_INSERTED], 0, item);

}

void
_tny_msg_folder_list_set_folder (TnyMsgFolderList *self, TnyMsgFolderIface *pfolder)
{
	g_mutex_lock (self->iterator_lock);
	if (self->first)
	{
		g_list_foreach (self->first, destroy_folder, NULL);
		g_list_free (self->first);
		self->first = NULL;
	}

	self->pfolder = pfolder;
	g_mutex_unlock (self->iterator_lock);

	return;
}

static guint
tny_msg_folder_list_length (TnyListIface *self)
{
	TnyMsgFolderList *me = (TnyMsgFolderList*)self;
	guint retval = 0;

	g_mutex_lock (me->iterator_lock);
	retval = me->first?g_list_length (me->first):0;
	g_mutex_unlock (me->iterator_lock);

	return retval;
}

static void
tny_msg_folder_list_remove (TnyListIface *self, gpointer item)
{
	g_warning (_("Cannot remove folders from a folder yet\n"));
}

static TnyIteratorIface*
tny_msg_folder_list_create_iterator (TnyListIface *self)
{
	TnyMsgFolderList *me = (TnyMsgFolderList*)self;

	return TNY_ITERATOR_IFACE (_tny_msg_folder_list_iterator_new (me));
}

static TnyListIface*
tny_msg_folder_list_copy_the_list (TnyListIface *self)
{
	TnyMsgFolderList *me = (TnyMsgFolderList*)self;
	TnyMsgFolderList *copy = g_object_new (TNY_TYPE_MSG_FOLDER_LIST, NULL);

	g_mutex_lock (me->iterator_lock);
	GList *list_copy = g_list_copy (me->first);
	copy->first = list_copy;
	g_mutex_unlock (me->iterator_lock);

	return TNY_LIST_IFACE (copy);
}

static void 
tny_msg_folder_list_foreach_in_the_list (TnyListIface *self, GFunc func, gpointer user_data)
{
	TnyMsgFolderList *me = (TnyMsgFolderList*)self;

	/* Foreach item in the list (without using a slower iterator) */

	g_mutex_lock (me->iterator_lock);
	g_list_foreach (me->first, func, user_data);
	g_mutex_unlock (me->iterator_lock);

	return;
}

static void
tny_list_iface_init (TnyListIfaceClass *klass)
{
	klass->length_func = tny_msg_folder_list_length;
	klass->prepend_func = tny_msg_folder_list_prepend;
	klass->append_func = tny_msg_folder_list_append;
	klass->remove_func = tny_msg_folder_list_remove;
	klass->create_iterator_func = tny_msg_folder_list_create_iterator;
	klass->copy_func = tny_msg_folder_list_copy_the_list;
	klass->foreach_func = tny_msg_folder_list_foreach_in_the_list;

	return;
}



static void
tny_msg_folder_list_finalize (GObject *object)
{
	TnyMsgFolderList *self = (TnyMsgFolderList *)object;

	g_mutex_lock (self->iterator_lock);
	if (self->first)
	{
		g_list_foreach (self->first, destroy_folder, NULL);
		g_list_free (self->first);
		self->first = NULL;
	}
	g_mutex_unlock (self->iterator_lock);

	g_mutex_free (self->iterator_lock);
	self->iterator_lock = NULL;

	parent_class->finalize (object);

	return;
}


static void
tny_msg_folder_list_class_init (TnyMsgFolderListClass *klass)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *)klass;
	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = tny_msg_folder_list_finalize;

	return;
}

static void
tny_msg_folder_list_init (TnyMsgFolderList *self)
{
	self->pfolder = NULL;
	self->iterator_lock = g_mutex_new ();
	self->first = NULL;

	return;
}



TnyListIface*
_tny_msg_folder_list_new (TnyMsgFolderIface *pfolder)
{
	TnyMsgFolderList *self = g_object_new (TNY_TYPE_MSG_FOLDER_LIST, NULL);
	
	_tny_msg_folder_list_set_folder (self, pfolder);

	return TNY_LIST_IFACE (self);
}

GType
_tny_msg_folder_list_get_type (void)
{
	static GType object_type = 0;

	if (G_UNLIKELY(object_type == 0))
	{
		static const GTypeInfo object_info = 
		{
			sizeof (TnyMsgFolderListClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) tny_msg_folder_list_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (TnyMsgFolderList),
			0,              /* n_preallocs */
			(GInstanceInitFunc) tny_msg_folder_list_init
		};

		static const GInterfaceInfo tny_list_iface_info = {
			(GInterfaceInitFunc) tny_list_iface_init,
			NULL,
			NULL
		};

		object_type = g_type_register_static (G_TYPE_OBJECT, 
						"TnyMsgFolderList", &object_info, 0);

		g_type_add_interface_static (object_type, TNY_TYPE_LIST_IFACE,
					     &tny_list_iface_info);

	}

	return object_type;
}

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

#include <tny-folder-store-query.h>
#include <tny-list.h>

static GObjectClass *parent_class;
static GObjectClass *item_parent_class;

TnyFolderStoreQuery* 
tny_folder_store_query_new (void)
{
    TnyFolderStoreQuery *self = g_object_new (TNY_TYPE_FOLDER_STORE_QUERY, NULL);
    
    return self;
}

static void
tny_folder_store_query_item_finalize (GObject *object)
{
	TnyFolderStoreQueryItem *self = (TnyFolderStoreQueryItem*) object;
    
   	if (self->regex)
		regfree (self->regex);
	self->regex = NULL;
    
	item_parent_class->finalize (object);
}

static void
tny_folder_store_query_finalize (GObject *object)
{
	TnyFolderStoreQuery *self = (TnyFolderStoreQuery*) object;
    
	g_object_unref (G_OBJECT (self->items));
    
	parent_class->finalize (object);

	return;
}

static void
tny_folder_store_query_item_class_init (TnyFolderStoreQueryItemClass *klass)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *)klass;
	item_parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = tny_folder_store_query_item_finalize;

	return;
}

static void
tny_folder_store_query_class_init (TnyFolderStoreQueryClass *klass)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *)klass;
	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = tny_folder_store_query_finalize;

	return;
}

static void
tny_folder_store_query_item_init (TnyFolderStoreQueryItem *self)
{
	self->regex = NULL;
	self->options = 0;
    
	return;
}

static void
tny_folder_store_query_init (TnyFolderStoreQuery *self)
{
	self->items = TNY_LIST_IFACE (tny_list_new ());
	return;
}


GType
tny_folder_store_query_get_type (void)
{
	static GType object_type = 0;

	if (G_UNLIKELY(object_type == 0))
	{
		static const GTypeInfo object_info = 
		{
			sizeof (TnyFolderStoreQueryClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) tny_folder_store_query_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (TnyFolderStoreQuery),
			0,              /* n_preallocs */
			(GInstanceInitFunc) tny_folder_store_query_init,
			NULL
		};


		object_type = g_type_register_static (G_TYPE_OBJECT, 
			"TnyFolderStoreQuery", &object_info, 0);


	}

	return object_type;
}


GType
tny_folder_store_query_item_get_type (void)
{
	static GType object_type = 0;

	if (G_UNLIKELY(object_type == 0))
	{
		static const GTypeInfo object_info = 
		{
			sizeof (TnyFolderStoreQueryItemClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) tny_folder_store_query_item_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (TnyFolderStoreQuery),
			0,              /* n_preallocs */
			(GInstanceInitFunc) tny_folder_store_query_item_init,
			NULL
		};


		object_type = g_type_register_static (G_TYPE_OBJECT, 
			"TnyFolderStoreQueryItem", &object_info, 0);

	}

	return object_type;
}

static gchar*
get_regerror (int errcode, regex_t *compiled)
{
	size_t length = regerror (errcode, compiled, NULL, 0);
	gchar *buffer = (gchar*) g_malloc (length);
	(void) regerror (errcode, compiled, buffer, length);
	return buffer;
}

void 
tny_folder_store_query_add_item (TnyFolderStoreQuery *query, const gchar *pattern, TnyFolderStoreQueryOption options)
{
    	gint er=0;
	gboolean addit=pattern?TRUE:FALSE;
        regex_t *regex = NULL;
    
    	if (addit)
		er = regcomp (regex, (const char*)pattern, 0);
    
	if (addit && er != 0)
	{
		gchar *erstr = get_regerror (er, regex);
		g_warning (erstr);
		g_free (erstr);
		regfree (regex);
		regex = NULL;
	    	addit = FALSE;
	} else 
		addit = TRUE;
    
	if (addit)
	{
	    	TnyFolderStoreQueryItem *add = g_object_new (TNY_TYPE_FOLDER_STORE_QUERY_ITEM, NULL);
		add->options = options;
		add->regex = regex;
		tny_list_iface_prepend (query->items, G_OBJECT (add));
		g_object_unref (G_OBJECT (add));
	}    
}

TnyListIface*
tny_folder_store_query_get_items (TnyFolderStoreQuery *query)
{
    return g_object_ref (G_OBJECT (query->items));
}


TnyFolderStoreQueryOption 
tny_folder_store_query_item_get_options (TnyFolderStoreQueryItem *item)
{
	return item->options;
}


regex_t*
tny_folder_store_query_item_get_regex (TnyFolderStoreQueryItem *item)
{
    return item->regex;
}


GType
tny_folder_store_query_option_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TNY_FOLDER_STORE_QUERY_OPTION_SUBSCRIBED, "TNY_FOLDER_STORE_QUERY_OPTION_SUBSCRIBED", "subscribed" },
      { TNY_FOLDER_STORE_QUERY_OPTION_UNSUBSCRIBED, "TNY_FOLDER_STORE_QUERY_OPTION_UNSUBSCRIBED", "unsubscribed" },
      { TNY_FOLDER_STORE_QUERY_OPTION_MATCH_ON_NAME, "TNY_FOLDER_STORE_QUERY_OPTION_MATCH_ON_NAME", "match_on_name" },
      { TNY_FOLDER_STORE_QUERY_OPTION_MATCH_ON_ID, "TNY_FOLDER_STORE_QUERY_OPTION_MATCH_ON_ID", "match_on_id" },	
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TnyFolderStoreQueryOption", values);
  }
  return etype;
}

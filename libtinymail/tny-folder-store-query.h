#ifndef TNY_FOLDER_STORE_QUERY_H
#define TNY_FOLDER_STORE_QUERY_H

/* libtinymailui-gtk - The Tiny Mail library
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

#include <tny-shared.h>
#include <tny-list-iface.h>
#include <tny-iterator-iface.h>

#include <regex.h>

G_BEGIN_DECLS

#define TNY_TYPE_FOLDER_STORE_QUERY             (tny_folder_store_query_get_type ())
#define TNY_FOLDER_STORE_QUERY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_FOLDER_STORE_QUERY, TnyFolderStoreQuery))
#define TNY_FOLDER_STORE_QUERY_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_FOLDER_STORE_QUERY, TnyFolderStoreQueryClass))
#define TNY_IS_FOLDER_STORE_QUERY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_FOLDER_STORE_QUERY))
#define TNY_IS_FOLDER_STORE_QUERY_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_FOLDER_STORE_QUERY))
#define TNY_FOLDER_STORE_QUERY_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_FOLDER_STORE_QUERY, TnyFolderStoreQueryClass))

#define TNY_TYPE_FOLDER_STORE_QUERY_ITEM             (tny_folder_store_query_item_get_type ())
#define TNY_FOLDER_STORE_QUERY_ITEM(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_FOLDER_STORE_QUERY_ITEM, TnyFolderStoreQueryItem))
#define TNY_FOLDER_STORE_QUERY_ITEM_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_FOLDER_STORE_QUERY_ITEM, TnyFolderStoreQueryItemClass))
#define TNY_IS_FOLDER_STORE_QUERY_ITEM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_FOLDER_STORE_QUERY_ITEM))
#define TNY_IS_FOLDER_STORE_QUERY_ITEM_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_FOLDER_STORE_QUERY_ITEM))
#define TNY_FOLDER_STORE_QUERY_ITEM_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_FOLDER_STORE_QUERY_ITEM, TnyFolderStoreQueryItemClass))

#define TNY_TYPE_FOLDER_STORE_QUERY_OPTION (tny_folder_store_query_option_get_type())

enum _TnyFolderStoreQueryOption
{
   TNY_FOLDER_STORE_QUERY_OPTION_SUBSCRIBED = 1<<1,
   TNY_FOLDER_STORE_QUERY_OPTION_UNSUBSCRIBED = 1<<2,
   TNY_FOLDER_STORE_QUERY_OPTION_MATCH_ON_NAME = 1<<3,
   TNY_FOLDER_STORE_QUERY_OPTION_MATCH_ON_ID = 1<<4
};

struct _TnyFolderStoreQueryItem 
{
    	GObject parent;
    	TnyFolderStoreQueryOption options;
    	regex_t *regex;
};

struct _TnyFolderStoreQueryItemClass
{
    	GObjectClass parent;
};

struct _TnyFolderStoreQuery 
{
	GObject parent;
    	TnyListIface *items;
};

struct _TnyFolderStoreQueryClass 
{
	GObjectClass parent;
};

GType tny_folder_store_query_get_type (void);
GType tny_folder_store_query_item_get_type (void);
GType tny_folder_store_query_option_get_type (void);

TnyFolderStoreQuery* tny_folder_store_query_new (void);
void tny_folder_store_query_add_item (TnyFolderStoreQuery *query, const gchar *pattern, TnyFolderStoreQueryOption options);
TnyListIface* tny_folder_store_query_get_items (TnyFolderStoreQuery *query);
TnyFolderStoreQueryOption tny_folder_store_query_item_get_options (TnyFolderStoreQueryItem *item);
regex_t* tny_folder_store_query_item_get_regex (TnyFolderStoreQueryItem *item);

G_END_DECLS

#endif

/* tinymail - Tiny Mail
 * Copyright (C) 2006-2007 Philip Van Hoof <pvanhoof@gnome.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with self program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdlib.h>
#include <glib.h>

#include <tny-list-iface.h>
#include <tny-iterator-iface.h>
#include <tny-list.h>
#include <tny-account-store-iface.h>
#include <tny-store-account-iface.h>
#include <tny-folder-iface.h>
#include <tny-folder-store-iface.h>
#include <tny-folder-store-query.h>

#include <account-store.h>

static gint recursion_level=0;
static gchar *cachedir=NULL;
static gboolean online=FALSE;

static void
recurse_folders (TnyFolderStoreIface *store, TnyFolderStoreQuery *query)
{
	TnyIteratorIface *iter;
	TnyListIface *folders = TNY_LIST_IFACE (tny_list_new ());

	tny_folder_store_iface_get_folders (store, folders, query);
	iter = tny_list_iface_create_iterator (folders);

	while (!tny_iterator_iface_is_done (iter))
	{
		TnyFolderStoreIface *folder = (TnyFolderStoreIface*) tny_iterator_iface_current (iter);
		gint i=0;

		for (i=0; i<recursion_level; i++)
			g_print ("\t");

		g_print ("%s\n", tny_folder_iface_get_name (TNY_FOLDER_IFACE (folder)));

		recursion_level++;
		recurse_folders (folder, query);
		recursion_level--;
	    
 		g_object_unref (G_OBJECT (folder));

		tny_iterator_iface_next (iter);	    
	}

	 g_object_unref (G_OBJECT (iter));
	 g_object_unref (G_OBJECT (folders));
}

static const GOptionEntry options[] = 
{
	{ "cachedir", 'c', 0, G_OPTION_ARG_STRING, &cachedir,
		"Cache directory", NULL },
	{ "online", 'o', 0, G_OPTION_ARG_NONE, &online,
		"Online or offline", NULL },
    
	{ NULL }
};

int 
main (int argc, char **argv)
{
	GOptionContext *context;
	TnyAccountStoreIface *account_store;
	TnyListIface *accounts;
	TnyFolderStoreQuery *query;
	TnyStoreAccountIface *account; TnyIteratorIface *iter;
	TnyFolderStoreQueryOption qoptions;
    
	free (malloc (10));
    
	g_type_init ();

    	context = g_option_context_new ("- The tinymail functional tester");
	g_option_context_add_main_entries (context, options, "tinymail");

    	g_option_context_parse (context, &argc, &argv, NULL);

	account_store = TNY_ACCOUNT_STORE_IFACE (tny_account_store_new (online, cachedir));

	if (cachedir)
		g_print ("Using %s as cache directory\n", cachedir);

	g_option_context_free (context);
    
       
	accounts = tny_list_new ();

	tny_account_store_iface_get_accounts (account_store, accounts, 
	      TNY_ACCOUNT_STORE_IFACE_STORE_ACCOUNTS);
    
	iter = tny_list_iface_create_iterator (accounts);
	account = (TnyStoreAccountIface*) tny_iterator_iface_current (iter);

	recursion_level = 0;
	qoptions = TNY_FOLDER_STORE_QUERY_OPTION_SUBSCRIBED;
	qoptions |= TNY_FOLDER_STORE_QUERY_OPTION_UNSUBSCRIBED;
	//query = tny_folder_store_query_new ();
	//tny_folder_store_query_add_item (query, NULL, qoptions);
	    
	recurse_folders (TNY_FOLDER_STORE_IFACE (account), NULL);
    
	g_object_unref (G_OBJECT (account));
	g_object_unref (G_OBJECT (iter));
	g_object_unref (G_OBJECT (accounts));
    
	return 0;
}


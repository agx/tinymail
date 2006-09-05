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

#include <tny-list.h>
#include <tny-iterator.h>
#include <tny-simple-list.h>
#include <tny-account-store.h>
#include <tny-store-account.h>
#include <tny-folder.h>
#include <tny-camel-header.h>
#include <tny-folder-store.h>

#include <account-store.h>

#include <camel/camel.h>
#include <tny-camel-header-priv.h>

typedef void (*performer) (TnyFolder *folder);

static void
do_get_folder (TnyFolder *folder)
{   
	g_print ("Getting headers of %s ...\n", tny_folder_get_id (folder));
	tny_folder_refresh (folder);
}


static void
do_test_folder (TnyFolder *folder)
{
	TnyList *headers = tny_simple_list_new ();
	gint length, bytes;
	gdouble kbytes, mbytes;
    
	g_print ("Loading headers for %s ...\n", tny_folder_get_id (folder));
	tny_folder_get_headers (folder, headers, FALSE);
	length=tny_list_length (headers);
	
    	bytes = (sizeof (TnyCamelHeader) + sizeof (CamelMessageInfo) + 
		 sizeof (CamelMessageInfoBase) + 
		 sizeof (CamelMessageContentInfo) + sizeof (struct _CamelFlag) +
		 sizeof (struct _CamelTag)) * length;
    
	kbytes = ((gdouble)bytes) / 1024;
    	mbytes = kbytes / 1024;
    
	g_print ("Loaded %d headers\n\n", length);

    	g_print ("\tsizeof (TnyHeader) = %d - accounts for %d bytes (~%.2lfK)\n", sizeof (TnyCamelHeader), length * sizeof (TnyCamelHeader), ((gdouble)length * sizeof (TnyCamelHeader))/1024);
	g_print ("\tsizeof (CamelMessageInfo) = %d - accounts for %d bytes (~%.2lfK)\n", sizeof (CamelMessageInfo), length * sizeof (CamelMessageInfo), ((gdouble)length * sizeof (CamelMessageInfo))/1024);
	g_print ("\tsizeof (CamelMessageInfoBase) = %d - accounts for %d bytes (~%.2lfK)\n", sizeof (CamelMessageInfoBase), length * sizeof (CamelMessageInfoBase), ((gdouble)length * sizeof (CamelMessageInfoBase))/1024);
	g_print ("\tsizeof (CamelMessageContentInfo) = %d - accounts for %d bytes (~%.2lfK)\n", sizeof (CamelMessageContentInfo), length * sizeof (CamelMessageContentInfo), ((gdouble)length * sizeof (CamelMessageContentInfo))/1024);
	g_print ("\tsizeof (struct _CamelFlag) = %d - accounts for %d bytes (~%.2lfK)\n", sizeof (struct _CamelFlag), length * sizeof (struct _CamelFlag), ((gdouble)length * sizeof (struct _CamelFlag))/1024);
	g_print ("\tsizeof (struct _CamelTag) = %d - accounts for %d bytes (~%.2lfK)\n", sizeof (struct _CamelTag), length * sizeof (struct _CamelTag), ((gdouble)length * sizeof (struct _CamelTag))/1024);

	g_print ("\nThis means that (at least) %d bytes or ~%.2lfK or ~%.2lfM are needed for this folder\n", bytes, kbytes, mbytes);

    	g_print ("Sleeping to allow your valgrind to see this...\n");
	sleep (5);
	g_print ("Unloading headers ...\n");
	g_object_unref (G_OBJECT (headers));
       	g_print ("Sleeping to allow your valgrind to see this...\n");
	sleep (5);
}


static void
recurse_folders (TnyFolderStore *store, TnyFolderStoreQuery *query, const gchar *folname, performer func)
{
	TnyIterator *iter;
	TnyList *folders = tny_simple_list_new ();

	tny_folder_store_get_folders (store, folders, query);
	iter = tny_list_create_iterator (folders);

	while (!tny_iterator_is_done (iter))
	{
		TnyFolderStore *folder = (TnyFolderStore*) tny_iterator_current (iter);

		if (!strcmp (tny_folder_get_id (TNY_FOLDER (folder)), folname))
			func (TNY_FOLDER (folder));
	    
		recurse_folders (folder, query, folname, func);
	    
 		g_object_unref (G_OBJECT (folder));

		tny_iterator_next (iter);	    
	}

	 g_object_unref (G_OBJECT (iter));
	 g_object_unref (G_OBJECT (folders));
}


static gchar *cachedir=NULL;
static gboolean online=FALSE;

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
	TnyAccountStore *account_store;
	TnyList *accounts;
	TnyStoreAccount *account;
	TnyIterator *aiter;
	TnyList *folders;
    	gint i=0;
	gchar *folderids[14] = {
	    "INBOX/1", "INBOX/10","INBOX/100","INBOX/200",
	    "INBOX/400", "INBOX/800","INBOX/2000","INBOX/3000", "INBOX/5000",
	    "INBOX/15000", "INBOX/20000","INBOX/30000","INBOX/40000",
	    "INBOX/50000" };
    
    	free (malloc (10));
    
	g_type_init ();

	context = g_option_context_new ("- The tinymail memory tester");
	g_option_context_add_main_entries (context, options, "tinymail");

    
	g_option_context_parse (context, &argc, &argv, NULL);

	account_store = tny_test_account_store_new (online, cachedir);

	if (cachedir)
		g_print ("Using %s as cache directory\n", cachedir);

	g_option_context_free (context);

	accounts = tny_simple_list_new ();
	    
	tny_account_store_get_accounts (account_store, accounts, 
		TNY_ACCOUNT_STORE_STORE_ACCOUNTS);

	aiter = tny_list_create_iterator (accounts);
	tny_iterator_first (aiter);
	account = TNY_STORE_ACCOUNT (tny_iterator_current (aiter));
    
	if (online)
		for (i=0; i<14; i++)
			recurse_folders (TNY_FOLDER_STORE (account), NULL, folderids[i], do_get_folder);

    	for (i=0; i<7; i++)
		recurse_folders (TNY_FOLDER_STORE (account), NULL, folderids[i], do_test_folder);
    
err:
	g_object_unref (G_OBJECT (account));
	g_object_unref (G_OBJECT (aiter));
	g_object_unref (G_OBJECT (accounts));
    
	return 0;
}


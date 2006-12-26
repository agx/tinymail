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
#include <string.h>

#include <glib.h>

#include <tny-list.h>
#include <tny-iterator.h>
#include <tny-simple-list.h>
#include <tny-account-store.h>
#include <tny-store-account.h>

#include "platfact.h"
#include <tny-platform-factory.h>
#include <tny-send-queue.h>

#include <tny-camel-send-queue.h>
#include <tny-camel-mem-stream.h>

#include <account-store.h>

static gchar *cachedir=NULL;
static gboolean online=FALSE, mainloop=TRUE;

static const GOptionEntry options[] = 
{
	{ "cachedir", 'c', 0, G_OPTION_ARG_STRING, &cachedir,
		"Cache directory", NULL },
	{ "online", 'o', 0, G_OPTION_ARG_NONE, &online,
		"Online or offline", NULL },
	{ "mainloop", 'm', 0, G_OPTION_ARG_NONE, &mainloop,
		"Use the Gtk+ mainloop", NULL },

	{ NULL }
};



static gboolean
time_s_up (gpointer data)
{
	gtk_main_quit ();
	return FALSE;
}

static gboolean
dance (gpointer data)
{
	return FALSE;
}

#define TEST_STRING "This is a test E-mail"

static TnyMsg*
create_test_msg (TnyPlatformFactory *platfact)
{
	TnyMsg *retval = tny_platform_factory_new_msg (platfact);
	TnyStream *stream = tny_camel_mem_stream_new ();
	TnyHeader *header = tny_platform_factory_new_header (platfact);

	tny_header_set_subject (header, TEST_STRING);
	tny_header_set_from (header, "tinymailunittest@mail.tinymail.org");
	tny_header_set_to (header, "spam@pvanhoof.be");

	tny_msg_set_header (retval, header);

	tny_stream_write (stream, TEST_STRING, strlen (TEST_STRING));
	tny_stream_reset (stream);

	tny_mime_part_construct_from_stream (TNY_MIME_PART (retval), stream, "text/plain"); 

	g_object_unref (G_OBJECT (stream));

	return retval;
}

static void
on_message_sent (TnySendQueue *queue, TnyMsg *msg, guint nth, guint total)
{
	TnyHeader *header;

	header = tny_msg_get_header (msg);
	g_print ("Message \"%s\" got sent", tny_header_get_subject (header));
	g_object_unref (G_OBJECT (header));
}

int 
main (int argc, char **argv)
{
	GOptionContext *context;
	TnyAccountStore *account_store;
	TnyList *accounts;
	TnyFolderStoreQuery *query;
	TnyStoreAccount *account;
	TnyIterator *iter;
    TnySendQueue *queue;
	TnyMsg *msg;
	TnyPlatformFactory *platfact;

	free (malloc (10));
    
	g_type_init ();

	platfact = tny_test_platform_factory_get_instance ();

    context = g_option_context_new ("- The tinymail functional tester");
	g_option_context_add_main_entries (context, options, "tinymail");
    g_option_context_parse (context, &argc, &argv, NULL);

	account_store = tny_test_account_store_new (online, cachedir);

	if (cachedir)
		g_print ("Using %s as cache directory\n", cachedir);

	g_option_context_free (context);
    
	accounts = tny_simple_list_new ();

	tny_account_store_get_accounts (account_store, accounts, 
	      TNY_ACCOUNT_STORE_TRANSPORT_ACCOUNTS);
	g_object_unref (G_OBJECT (account_store));
    
	iter = tny_list_create_iterator (accounts);
	account = (TnyStoreAccount*) tny_iterator_get_current (iter);

	msg = create_test_msg (platfact);

	queue = tny_camel_send_queue_new (TNY_CAMEL_TRANSPORT_ACCOUNT (account));
	tny_send_queue_add (queue, msg);

    if (mainloop)
	{
		g_print ("Using the Gtk+ mainloop (will wait 4 seconds in the loop)\n");
	    
	    	g_timeout_add (1, dance, account);	    
	    	g_timeout_add (5000 * 4, time_s_up, NULL);
	    
		gtk_main ();
	    
	} else {
		g_print ("Not using a mainloop (will sleep 4 seconds)\n");
	    
		dance (account);
		sleep (4);
	}
    
	g_object_unref (G_OBJECT (account));
	g_object_unref (G_OBJECT (iter));
	g_object_unref (G_OBJECT (accounts));
   	g_object_unref (G_OBJECT (platfact));

	return 0;
}


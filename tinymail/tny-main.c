/*
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

#include <gtk/gtk.h>

#include <tny-msg-account-iface.h>
#include <tny-msg-account.h>

#include <tny-msg-folder-iface.h>
#include <tny-msg-account-tree-model.h>
#include <tny-msg-header-iface.h>
#include <tny-msg-header-list-model.h>

static void
on_mailbox_view_tree_selection_changed (GtkTreeSelection *selection, 
		gpointer user_data)
{
/*
	GtkTreeView *header_view = GTK_TREE_VIEW (user_data);
	GtkTreeIter iter;
	GtkTreeModel *model;
	TnyMsgFolderIface *folder;

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		GtkTreeModel *header_model;
		GList *headers;

		gtk_tree_model_get (model, &iter, 
			TNY_MSG_ACCOUNT_TREE_MODEL_INSTANCE_COLUMN, 
			&folder, -1);

		g_print ("You selected folder: %s\n", 
			tny_msg_folder_iface_get_name (folder));

		headers = (GList*)tny_msg_folder_iface_get_headers (folder);

		header_model = GTK_TREE_MODEL (
			tny_msg_header_list_model_new ());

		tny_msg_header_list_model_inject (
			TNY_MSG_HEADER_LIST_MODEL (header_model), headers);

		gtk_tree_view_set_model (GTK_TREE_VIEW (header_view), header_model);

	}
*/
}

int 
main (int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *hbox;
	GtkWidget *mailbox_view;
	GtkWidget *header_view;
	GtkWidget *mailbox_sw;
	GtkWidget *header_sw;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeModel *mailbox_model;
	GtkTreeSelection *select;
	gint t = 0, i = 0;
	TnyMsgAccountIface *account;

	gtk_init (&argc, &argv);
	g_thread_init (NULL);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Tiny Mail");
	g_signal_connect (window, "destroy",
		G_CALLBACK (gtk_exit), &window);
	gtk_container_set_border_width (GTK_CONTAINER (window), 8);
	hbox = gtk_hbox_new (FALSE, 8);
	gtk_container_add (GTK_CONTAINER (window), hbox);

	header_sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (header_sw), 
			GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (header_sw),
			GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	mailbox_sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (mailbox_sw),
		GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (mailbox_sw),
		                        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	
	header_view = gtk_tree_view_new ();
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (header_view), TRUE);
	gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW(header_view), TRUE);
	mailbox_view = gtk_tree_view_new ();
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (mailbox_view), TRUE);
	
	gtk_container_add (GTK_CONTAINER (header_sw), header_view);
	gtk_container_add (GTK_CONTAINER (mailbox_sw), mailbox_view);

	/* mailbox_view columns */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Folder", renderer,
			"text", TNY_MSG_ACCOUNT_TREE_MODEL_NAME_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(mailbox_view), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Folder", renderer,
			"text", TNY_MSG_ACCOUNT_TREE_MODEL_UNREAD_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(mailbox_view), column);

	/* header_view columns */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("ID", renderer,
		"text", TNY_MSG_HEADER_LIST_MODEL_ID_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, 0); 
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 200);
	gtk_tree_view_append_column (GTK_TREE_VIEW(header_view), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("From", renderer,
		"text", TNY_MSG_HEADER_LIST_MODEL_FROM_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, 0);			  
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 200);
	gtk_tree_view_append_column (GTK_TREE_VIEW(header_view), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("To", renderer,
		"text", TNY_MSG_HEADER_LIST_MODEL_TO_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, 0);			  
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 200);
	gtk_tree_view_append_column (GTK_TREE_VIEW(header_view), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Subject", renderer,
		"text", TNY_MSG_HEADER_LIST_MODEL_SUBJECT_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, 0);			  
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 200);
	gtk_tree_view_append_column (GTK_TREE_VIEW(header_view), column);

	mailbox_model = GTK_TREE_MODEL (tny_msg_account_tree_model_new ());
	account = TNY_MSG_ACCOUNT_IFACE(tny_msg_account_new ());
	tny_msg_account_tree_model_add (TNY_MSG_ACCOUNT_TREE_MODEL 
		(mailbox_model), account);

	gtk_tree_view_set_model (GTK_TREE_VIEW (mailbox_view), mailbox_model);

	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (mailbox_view));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (select), "changed",
		G_CALLBACK (on_mailbox_view_tree_selection_changed), header_view);

	gtk_box_pack_start (GTK_BOX (hbox), mailbox_sw, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), header_sw, FALSE, FALSE, 0);

	gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);
	gtk_widget_show_all (GTK_WIDGET(window));

	gtk_main();

	return 0;
}

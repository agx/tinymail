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

#include <string.h>
#include <gtk/gtk.h>

#include <tny-password-dialog.h>
#include <tny-account-store-iface.h>
#include <tny-account-store.h>
#include <tny-account-iface.h>
#include <tny-account.h>

#include <tny-msg-folder-iface.h>
#include <tny-account-tree-model.h>
#include <tny-msg-header-iface.h>
#include <tny-msg-header-list-model.h>

#include <tny-summary-window.h>
#include <tny-summary-window-iface.h>

static GObjectClass *parent_class = NULL;


typedef struct _TnySummaryWindowPriv TnySummaryWindowPriv;

struct _TnySummaryWindowPriv
{
	TnyAccountStoreIface *account_store;
	GtkTreeView *mailbox_view, *header_view;
};

#define TNY_SUMMARY_WINDOW_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_SUMMARY_WINDOW_TYPE, TnySummaryWindowPriv))

static void 
reload_accounts (TnySummaryWindowPriv *priv)
{
	TnyAccountStoreIface *account_store = priv->account_store;
	GtkTreeModel *mailbox_model = GTK_TREE_MODEL (tny_account_tree_model_new ());
	const GList* accounts;

	accounts = tny_account_store_iface_get_accounts (account_store);
	
	while (accounts)
	{
		TnyAccountIface *account = accounts->data;

		tny_account_tree_model_add (TNY_ACCOUNT_TREE_MODEL 
			(mailbox_model), account);

		accounts = g_list_next (accounts);
	}

	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->mailbox_view), mailbox_model);

	return;
}

static void
tny_summary_window_set_account_store (TnySummaryWindowIface *self, TnyAccountStoreIface *account_store)
{
	TnySummaryWindowPriv *priv = TNY_SUMMARY_WINDOW_GET_PRIVATE (self);

	if (priv->account_store)
		g_object_unref (G_OBJECT (priv->account_store));

	g_object_ref (G_OBJECT (account_store));

	priv->account_store = account_store;

	reload_accounts (priv);

	return;
}

static void
on_mailbox_view_tree_selection_changed (GtkTreeSelection *selection, 
		gpointer user_data)
{

	GtkTreeView *header_view = GTK_TREE_VIEW (user_data);
	GtkTreeIter iter;
	GtkTreeModel *model;
	TnyMsgFolderIface *folder;

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		GtkTreeModel *header_model;
		GList *headers;

		gtk_tree_model_get (model, &iter, 
			TNY_ACCOUNT_TREE_MODEL_INSTANCE_COLUMN, 
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

	return;
}

static GtkWidget*
create_msg_window (TnyMsgIface *msg)
{
	TnyMsgHeaderIface *header;
	GList *attachments;
	TnyMsgBodyIface *body;

	GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkTextView *textview = GTK_TEXT_VIEW (gtk_text_view_new ());
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (textview);
	const gchar *text;

	header = TNY_MSG_HEADER_IFACE (tny_msg_iface_get_header (msg));
	body = TNY_MSG_BODY_IFACE (tny_msg_iface_get_body (msg));
	attachments = (GList*)tny_msg_iface_get_attachments (msg);
	text = tny_msg_body_iface_get_data (body);


	gtk_window_set_title (GTK_WINDOW (window), tny_msg_header_iface_get_subject (header));
	gtk_text_buffer_set_text (buffer, text, strlen(text));

	gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (textview));

	return window;
}


static void
on_header_view_tree_row_activated (GtkTreeView *treeview, GtkTreePath *path,
			GtkTreeViewColumn *col,  gpointer userdata)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
		
	model = gtk_tree_view_get_model(treeview);
	
	if (gtk_tree_model_get_iter(model, &iter, path))
	{
		TnyMsgHeaderIface *header;
		

		GtkWindow *msgwin;

		gtk_tree_model_get (model, &iter, 
			TNY_MSG_HEADER_LIST_MODEL_INSTANCE_COLUMN, 
			&header, -1);
		
		if (header)
		{

			const TnyMsgFolderIface *folder;
			const TnyMsgIface *msg;
			const TnyMsgHeaderIface *nheader;

			folder = tny_msg_header_iface_get_folder (TNY_MSG_HEADER_IFACE (header));
			msg = tny_msg_folder_iface_get_message (TNY_MSG_FOLDER_IFACE (folder), header);
			nheader = tny_msg_iface_get_header (TNY_MSG_IFACE (msg));

			g_print ("You activated header: %s\n", 
				tny_msg_header_iface_get_subject (TNY_MSG_HEADER_IFACE (header)));
			g_print ("You activated header: %s\n", 
				tny_msg_header_iface_get_subject (TNY_MSG_HEADER_IFACE (nheader)));
	
			msgwin = GTK_WINDOW (create_msg_window (TNY_MSG_IFACE (msg)));
	
			gtk_widget_show_all (GTK_WIDGET (msgwin));
		}
	}
}

static void
on_header_view_tree_selection_changed (GtkTreeSelection *selection, 
		gpointer user_data)
{
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		TnyMsgHeaderIface *header;

		gtk_tree_model_get (model, &iter, 
			TNY_MSG_HEADER_LIST_MODEL_INSTANCE_COLUMN, 
			&header, -1);

		g_print ("You selected header: %s\n", 
			tny_msg_header_iface_get_subject (header));

	}

	return;
}

TnySummaryWindow*
tny_summary_window_new (void)
{
	TnySummaryWindow *self = g_object_new (TNY_SUMMARY_WINDOW_TYPE, NULL);

	return self;
}

static void
tny_summary_window_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnySummaryWindow *self = (TnySummaryWindow *)instance;
	TnySummaryWindowPriv *priv = TNY_SUMMARY_WINDOW_GET_PRIVATE (self);
	GtkWindow *window = GTK_WINDOW (self);


	GtkWidget *hbox;
	GtkWidget *mailbox_sw;
	GtkWidget *header_sw;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeModel *mailbox_model;
	GtkTreeSelection *select;
	gint t = 0, i = 0;

	gtk_window_set_title (window, "Tinymail");
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
	
	priv->header_view = GTK_TREE_VIEW (gtk_tree_view_new ());

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (priv->header_view), TRUE);
	gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW(priv->header_view), TRUE);

	priv->mailbox_view = GTK_TREE_VIEW (gtk_tree_view_new ());
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (priv->mailbox_view), TRUE);
	
	gtk_container_add (GTK_CONTAINER (header_sw), GTK_WIDGET (priv->header_view));
	gtk_container_add (GTK_CONTAINER (mailbox_sw), GTK_WIDGET (priv->mailbox_view));

	/* mailbox_view columns */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Folder", renderer,
			"text", TNY_ACCOUNT_TREE_MODEL_NAME_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(priv->mailbox_view), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Folder", renderer,
			"text", TNY_ACCOUNT_TREE_MODEL_UNREAD_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(priv->mailbox_view), column);

	/* header_view columns */
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("From", renderer,
		"text", TNY_MSG_HEADER_LIST_MODEL_FROM_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, 0);			  
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 200);

	gtk_tree_view_append_column (GTK_TREE_VIEW(priv->header_view), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("To", renderer,
		"text", TNY_MSG_HEADER_LIST_MODEL_TO_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, 0);			  
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 200);

	gtk_tree_view_append_column (GTK_TREE_VIEW(priv->header_view), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Subject", renderer,
		"text", TNY_MSG_HEADER_LIST_MODEL_SUBJECT_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, 0);			  
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 200);

	gtk_tree_view_append_column (GTK_TREE_VIEW(priv->header_view), column);

	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->mailbox_view));

	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (select), "changed",
		G_CALLBACK (on_mailbox_view_tree_selection_changed), priv->header_view);

	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->header_view));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (select), "changed",
		G_CALLBACK (on_header_view_tree_selection_changed), priv->header_view);

	g_signal_connect(G_OBJECT (priv->header_view), "row-activated", 
		G_CALLBACK (on_header_view_tree_row_activated), priv->header_view);

	gtk_box_pack_start (GTK_BOX (hbox), mailbox_sw, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), header_sw, TRUE, TRUE, 0);

	gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);

	gtk_widget_show_all (GTK_WIDGET(hbox));

	return;
}

static void
tny_summary_window_finalize (GObject *object)
{
	TnySummaryWindow *self = (TnySummaryWindow *)object;	
	
	(*parent_class->finalize) (object);

	return;
}

static void
tny_summary_window_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnySummaryWindowIfaceClass *klass = (TnySummaryWindowIfaceClass *)g_iface;

	klass->set_account_store_func = tny_summary_window_set_account_store;

	return;
}

static void 
tny_summary_window_class_init (TnySummaryWindowClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_summary_window_finalize;

	g_type_class_add_private (object_class, sizeof (TnySummaryWindowPriv));

	return;
}

GType 
tny_summary_window_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnySummaryWindowClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_summary_window_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnySummaryWindow),
		  0,      /* n_preallocs */
		  tny_summary_window_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_summary_window_iface_info = 
		{
		  (GInterfaceInitFunc) tny_summary_window_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (GTK_TYPE_WINDOW,
			"TnySummaryWindow",
			&info, 0);

		g_type_add_interface_static (type, TNY_SUMMARY_WINDOW_IFACE_TYPE, 
			&tny_summary_window_iface_info);

	}

	return type;
}

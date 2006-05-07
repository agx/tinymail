/* tinymail - Tiny Mail
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

/* TODO: Refactory this type to libtinymailui-gtk */

#include <string.h>
#include <gtk/gtk.h>

#include <tny-password-dialog.h>
#include <tny-account-store-iface.h>
#include <tny-account-store.h>

#include <tny-account-iface.h>
#include <tny-store-account-iface.h>
#include <tny-transport-account-iface.h>
#include <tny-store-account.h>
#include <tny-transport-account.h>

#include <tny-msg-view-iface.h>
#include <tny-msg-view.h>

#include <tny-msg-window-iface.h>
#include <tny-msg-window.h>

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
	TnyMsgViewIface *msg_view;
	guint accounts_reloaded_signal;
	GtkWidget *status, *progress;
	guint status_id;
};

#define TNY_SUMMARY_WINDOW_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_SUMMARY_WINDOW, TnySummaryWindowPriv))

static void 
reload_accounts (TnySummaryWindowPriv *priv)
{
	static GtkTreeModel *empty_model;

	TnyAccountStoreIface *account_store = priv->account_store;
	GtkTreeModel *sortable, *mailbox_model = GTK_TREE_MODEL (tny_account_tree_model_new ());
	const GList* accounts;

	if (G_UNLIKELY (!empty_model))
		empty_model = GTK_TREE_MODEL (gtk_list_store_new 
			(1, G_TYPE_STRING));

	accounts = tny_account_store_iface_get_store_accounts (account_store);
	
	while (G_LIKELY (accounts))
	{
		TnyStoreAccountIface *account = accounts->data;

		tny_account_tree_model_add (TNY_ACCOUNT_TREE_MODEL 
			(mailbox_model), account);

		accounts = g_list_next (accounts);
	}

	sortable = gtk_tree_model_sort_new_with_model (mailbox_model);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (sortable),
				TNY_ACCOUNT_TREE_MODEL_NAME_COLUMN, 
				GTK_SORT_ASCENDING);

	
	/* Clear the header_view by giving it an empty model */
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->header_view), 
		empty_model);

	/* Set the model of the mailbox_view */
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->mailbox_view), 
		sortable);

	return;
}

static void
accounts_reloaded (TnyAccountStoreIface *store, gpointer user_data)
{
	TnySummaryWindowPriv *priv = user_data;

	reload_accounts (priv);
	
	return;
}


static void
tny_summary_window_set_account_store (TnySummaryWindowIface *self, TnyAccountStoreIface *account_store)
{
	TnySummaryWindowPriv *priv = TNY_SUMMARY_WINDOW_GET_PRIVATE (self);

	if (G_UNLIKELY (priv->account_store))
	{ /* You typically set it once, so unlikely */

		g_signal_handler_disconnect (G_OBJECT (priv->account_store),
			priv->accounts_reloaded_signal);

		g_object_unref (G_OBJECT (priv->account_store));
	}

	g_object_ref (G_OBJECT (account_store));

	priv->account_store = account_store;

	priv->accounts_reloaded_signal = g_signal_connect 
		(G_OBJECT (account_store), "accounts_reloaded",
		G_CALLBACK (accounts_reloaded), priv);

	reload_accounts (priv);

	return;
}

static void
on_header_view_tree_selection_changed (GtkTreeSelection *selection, 
		gpointer user_data)
{
	TnySummaryWindow *self  = user_data;
	TnySummaryWindowPriv *priv = TNY_SUMMARY_WINDOW_GET_PRIVATE (self);
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (G_LIKELY (gtk_tree_selection_get_selected (selection, &model, &iter)))
	{
		TnyMsgHeaderIface *header;

		gtk_tree_model_get (model, &iter, 
			TNY_MSG_HEADER_LIST_MODEL_INSTANCE_COLUMN, 
			&header, -1);

		if (G_LIKELY (header))
		{

			const TnyMsgFolderIface *folder;
			const TnyMsgIface *msg;
			const TnyMsgHeaderIface *nheader;

			folder = tny_msg_header_iface_get_folder (TNY_MSG_HEADER_IFACE (header));
			msg = tny_msg_folder_iface_get_message (TNY_MSG_FOLDER_IFACE (folder), header);
			nheader = tny_msg_iface_get_header (TNY_MSG_IFACE (msg));

			tny_msg_view_iface_set_msg (priv->msg_view, TNY_MSG_IFACE (msg));	
		}
	}

	return;
}


static void
refresh_current_folder (TnyMsgFolderIface *folder, gpointer user_data)
{
	TnySummaryWindowPriv *priv = user_data;

	GtkTreeView *header_view = GTK_TREE_VIEW (priv->header_view);
	GtkTreeModel *header_model, *sortable;

	header_model = GTK_TREE_MODEL (
		tny_msg_header_list_model_new ());

#ifdef ASYNC_HEADERS
	tny_msg_header_list_model_set_folder (
		TNY_MSG_HEADER_LIST_MODEL (header_model), folder, FALSE);
#else
	tny_msg_header_list_model_set_folder (
		TNY_MSG_HEADER_LIST_MODEL (header_model), folder, TRUE);
#endif

	sortable = gtk_tree_view_get_model (GTK_TREE_VIEW (header_view));

	if (G_LIKELY (sortable) && G_LIKELY (GTK_IS_TREE_MODEL_SORT (sortable)))
	{
		GtkTreeModel *model = gtk_tree_model_sort_get_model 
			(GTK_TREE_MODEL_SORT (sortable));

		if (G_LIKELY (model))
			g_object_unref (G_OBJECT (model));

		g_object_unref (G_OBJECT (sortable));
	}

	sortable = gtk_tree_model_sort_new_with_model (header_model);

	/* TODO: Implement a fast sorting algorithm (not easy) */

	/* gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (sortable),
			TNY_MSG_HEADER_LIST_MODEL_FROM_COLUMN, 
			GTK_SORT_ASCENDING); */
		
	gtk_tree_view_set_model (GTK_TREE_VIEW (header_view), sortable);

	gtk_widget_hide (GTK_WIDGET (priv->progress));

	return;
}

static void
refresh_current_folder_status_update (TnyMsgFolderIface *folder, const gchar *what, gint status, gpointer user_data)
{
	gdk_threads_enter ();
	{
		TnySummaryWindowPriv *priv = user_data;

		gtk_progress_bar_pulse (GTK_PROGRESS_BAR (priv->progress));

		gtk_statusbar_pop (GTK_STATUSBAR (priv->status), priv->status_id);
		gtk_statusbar_push (GTK_STATUSBAR (priv->status), priv->status_id, what);

	}
	gdk_threads_leave ();

	/* g_print ("%s = %d\n", what, status); */
}

static void
on_mailbox_view_tree_selection_changed (GtkTreeSelection *selection, 
		gpointer user_data)
{
	TnySummaryWindowPriv *priv = user_data;
	GtkTreeView *header_view = priv->header_view;
	GtkTreeIter iter;
	GtkTreeModel *model;

	gtk_widget_show (GTK_WIDGET (priv->progress));

	if (G_LIKELY (gtk_tree_selection_get_selected (selection, &model, &iter)))
	{
		TnyMsgFolderIface *folder;
		
		gtk_tree_model_get (model, &iter, 
			TNY_ACCOUNT_TREE_MODEL_INSTANCE_COLUMN, 
			&folder, -1);

#ifdef ASYNC_HEADERS
		tny_msg_folder_iface_refresh_headers_async (folder, 
			refresh_current_folder, 
			refresh_current_folder_status_update, user_data);
#else
		refresh_current_folder (folder, user_data);
#endif
	}

	return;
}


static void
on_header_view_tree_row_activated (GtkTreeView *treeview, GtkTreePath *path,
			GtkTreeViewColumn *col,  gpointer userdata)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
		
	model = gtk_tree_view_get_model(treeview);
	
	if (G_LIKELY (gtk_tree_model_get_iter(model, &iter, path)))
	{
		TnyMsgHeaderIface *header;
		TnyMsgWindowIface *msgwin;

		gtk_tree_model_get (model, &iter, 
			TNY_MSG_HEADER_LIST_MODEL_INSTANCE_COLUMN, 
			&header, -1);
		
		if (G_LIKELY (header))
		{

			const TnyMsgFolderIface *folder;
			const TnyMsgIface *msg;
			const TnyMsgHeaderIface *nheader;

			folder = tny_msg_header_iface_get_folder (TNY_MSG_HEADER_IFACE (header));
			msg = tny_msg_folder_iface_get_message (TNY_MSG_FOLDER_IFACE (folder), header);
			nheader = tny_msg_iface_get_header (TNY_MSG_IFACE (msg));

			msgwin = TNY_MSG_WINDOW_IFACE (tny_msg_window_new ());
			tny_msg_window_iface_set_msg (msgwin, TNY_MSG_IFACE (msg));
	
			gtk_widget_show (GTK_WIDGET (msgwin));
		}
	}
}

/**
 * tny_summary_window_new:
 * 
 *
 * Return value: A new #TnySummaryWindowIface instance implemented for Gtk+
 **/
TnySummaryWindow*
tny_summary_window_new (void)
{
	TnySummaryWindow *self = g_object_new (TNY_TYPE_SUMMARY_WINDOW, NULL);

	return self;
}

static void
tny_summary_window_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnySummaryWindow *self = (TnySummaryWindow *)instance;
	TnySummaryWindowPriv *priv = TNY_SUMMARY_WINDOW_GET_PRIVATE (self);
	GtkWindow *window = GTK_WINDOW (self);
	GtkWidget *mailbox_sw;
	GtkWidget *header_sw;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeModel *mailbox_model;
	GtkTreeSelection *select;
	gint t = 0, i = 0;
	GtkWidget *hpaned1;
	GtkWidget *vpaned1;
	GtkWidget *vbox;


	/* TODO: Persist application UI status (of the panes) */

	hpaned1 = gtk_hpaned_new ();
	gtk_widget_show (hpaned1);
	vbox = gtk_vbox_new (FALSE, 5);
	priv->status = GTK_WIDGET (gtk_statusbar_new ());
	priv->progress = gtk_progress_bar_new ();
	priv->status_id = gtk_statusbar_get_context_id (GTK_STATUSBAR (priv->status), "default");

	gtk_box_pack_start (GTK_BOX (priv->status), priv->progress, FALSE, FALSE, 0);
	gtk_widget_show (priv->status);
	gtk_widget_show (vbox);

	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (hpaned1), TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (priv->status), FALSE, TRUE, 0);

	vpaned1 = gtk_vpaned_new ();
	gtk_widget_show (vpaned1);
	

	/* TODO: Only if visible */
	priv->msg_view = TNY_MSG_VIEW_IFACE (tny_msg_view_new ());
	gtk_widget_show (GTK_WIDGET (priv->msg_view));	
	gtk_paned_pack2 (GTK_PANED (vpaned1), GTK_WIDGET (priv->msg_view), TRUE, TRUE);

	gtk_window_set_title (window, "Tinymail");
	gtk_container_set_border_width (GTK_CONTAINER (window), 8);


	mailbox_sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_paned_pack1 (GTK_PANED (hpaned1), mailbox_sw, TRUE, TRUE);
	gtk_paned_pack2 (GTK_PANED (hpaned1), vpaned1, TRUE, TRUE);
	gtk_widget_show (GTK_WIDGET (mailbox_sw));

	header_sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_paned_pack1 (GTK_PANED (vpaned1), header_sw, TRUE, TRUE);
	gtk_widget_show (GTK_WIDGET (header_sw));

	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (header_sw), 
			GTK_SHADOW_ETCHED_IN);

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (header_sw),
			GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (mailbox_sw),
		GTK_SHADOW_ETCHED_IN);

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (mailbox_sw),
		                        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	
	priv->header_view = GTK_TREE_VIEW (gtk_tree_view_new ());
	gtk_widget_show (GTK_WIDGET (priv->header_view));

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (priv->header_view), TRUE);
	gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW(priv->header_view), TRUE);

	priv->mailbox_view = GTK_TREE_VIEW (gtk_tree_view_new ());
	gtk_widget_show (GTK_WIDGET (priv->mailbox_view));

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (priv->mailbox_view), TRUE);
	
	gtk_container_add (GTK_CONTAINER (header_sw), GTK_WIDGET (priv->header_view));
	gtk_container_add (GTK_CONTAINER (mailbox_sw), GTK_WIDGET (priv->mailbox_view));


	/* TODO: Persist application UI status */
	/* mailbox_view columns */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Folder", renderer,
			"text", TNY_ACCOUNT_TREE_MODEL_NAME_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, TNY_ACCOUNT_TREE_MODEL_NAME_COLUMN);

	gtk_tree_view_append_column (GTK_TREE_VIEW(priv->mailbox_view), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Folder", renderer,
			"text", TNY_ACCOUNT_TREE_MODEL_UNREAD_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, TNY_ACCOUNT_TREE_MODEL_UNREAD_COLUMN);

	gtk_tree_view_append_column (GTK_TREE_VIEW(priv->mailbox_view), column);


	/* TODO: Persist application UI status */
	/* header_view columns */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("From", renderer,
		"text", TNY_MSG_HEADER_LIST_MODEL_FROM_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, TNY_MSG_HEADER_LIST_MODEL_FROM_COLUMN);			  
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 100);
	gtk_tree_view_append_column (GTK_TREE_VIEW(priv->header_view), column);

	if (G_UNLIKELY (FALSE))
	{ /* Unlikely ;-) */
		renderer = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes ("To", renderer,
			"text", TNY_MSG_HEADER_LIST_MODEL_TO_COLUMN, NULL);
		gtk_tree_view_column_set_sort_column_id (column, TNY_MSG_HEADER_LIST_MODEL_TO_COLUMN);			  
		gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_fixed_width (column, 100);
		gtk_tree_view_append_column (GTK_TREE_VIEW(priv->header_view), column);
	}

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Subject", renderer,
		"text", TNY_MSG_HEADER_LIST_MODEL_SUBJECT_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, TNY_MSG_HEADER_LIST_MODEL_SUBJECT_COLUMN);			  
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 200);
	gtk_tree_view_append_column (GTK_TREE_VIEW(priv->header_view), column);


	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Date", renderer,
		"text", TNY_MSG_HEADER_LIST_MODEL_DATE_RECEIVED_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, TNY_MSG_HEADER_LIST_MODEL_DATE_RECEIVED_COLUMN);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 100);
	gtk_tree_view_append_column (GTK_TREE_VIEW(priv->header_view), column);


	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->mailbox_view));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (select), "changed",
		G_CALLBACK (on_mailbox_view_tree_selection_changed), priv);


	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->header_view));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	g_signal_connect(G_OBJECT (priv->header_view), "row-activated", 
		G_CALLBACK (on_header_view_tree_row_activated), priv->header_view);
	g_signal_connect (G_OBJECT (select), "changed",
		G_CALLBACK (on_header_view_tree_selection_changed), self);


	/* TODO: Persist application UI status */
	gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);

	gtk_widget_hide (priv->progress);

	return;
}

static void
tny_summary_window_finalize (GObject *object)
{
	TnySummaryWindow *self = (TnySummaryWindow *)object;	
	TnySummaryWindowPriv *priv = TNY_SUMMARY_WINDOW_GET_PRIVATE (self);

	if (G_LIKELY (priv->account_store))
	{
		g_signal_handler_disconnect (G_OBJECT (priv->account_store),
			priv->accounts_reloaded_signal);

		g_object_unref (G_OBJECT (priv->account_store));
	}

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

	if (G_UNLIKELY(type == 0))
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

		g_type_add_interface_static (type, TNY_TYPE_SUMMARY_WINDOW_IFACE, 
			&tny_summary_window_iface_info);

	}

	return type;
}

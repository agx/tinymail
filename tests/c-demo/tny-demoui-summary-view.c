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

#include <config.h>

#include <glib/gi18n-lib.h>

#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <tny-simple-list.h>
#include <tny-iterator.h>
#include <tny-platform-factory.h>

#if PLATFORM==1
#include <tny-gnome-platform-factory.h>
#include <tny-gnome-account-store.h>
#endif

#if PLATFORM==2
#include <tny-maemo-platform-factory.h>
#include <tny-maemo-account-store.h>
#endif

#if PLATFORM==3
#include <tny-gpe-platform-factory.h>
#include <tny-gpe-account-store.h>
#endif

#if PLATFORM==4
#include <tny-olpc-platform-factory.h>
#include <tny-olpc-account-store.h>
#endif

#include <tny-status.h>
#include <tny-account-store.h>
#include <tny-account.h>
#include <tny-store-account.h>
#include <tny-transport-account.h>
#include <tny-msg-view.h>
#include <tny-msg-window.h>
#include <tny-gtk-msg-window.h>
#include <tny-folder.h>
#include <tny-gtk-account-list-model.h>
#include <tny-gtk-folder-store-tree-model.h>
#include <tny-header.h>
#include <tny-gtk-header-list-model.h>
#include <tny-demoui-summary-view.h>
#include <tny-summary-view.h>
#include <tny-account-store-view.h>
#include <tny-merge-folder.h>

#define GO_ONLINE_TXT _("Go online")
#define GO_OFFLINE_TXT _("Go offline")

static GObjectClass *parent_class = NULL;

#include <tny-camel-send-queue.h>
#include <tny-camel-transport-account.h>
#include <tny-folder-monitor.h>


#ifdef QUEUES
#include <tny-get-msg-queue.h>
#endif

static TnySendQueue *queue = NULL;

typedef struct _TnyDemouiSummaryViewPriv TnyDemouiSummaryViewPriv;

struct _TnyDemouiSummaryViewPriv
{
	TnyAccountStore *account_store;
 	GtkComboBox *account_view;
	GtkTreeView *mailbox_view, *header_view;
	TnyMsgView *msg_view;
	guint accounts_reloaded_signal;
	GtkWidget *status, *progress, *online_button;
	guint status_id;
	gulong mailbox_select_sid;
	GtkTreeSelection *mailbox_select;
	GtkTreeIter last_mailbox_correct_select;
 	gboolean last_mailbox_correct_select_set;
	guint connchanged_signal, online_button_signal;
	TnyList *current_accounts;
	TnyFolderMonitor *monitor; GMutex *monitor_lock;
	gboolean handle_recon;
};

#define TNY_DEMOUI_SUMMARY_VIEW_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_DEMOUI_SUMMARY_VIEW, TnyDemouiSummaryViewPriv))


static void
set_header_view_model (GtkTreeView *header_view, GtkTreeModel *model)
{
	GtkTreeModel *oldsortable = gtk_tree_view_get_model (GTK_TREE_VIEW (header_view));

	if (oldsortable && GTK_IS_TREE_MODEL_SORT (oldsortable))
	{ 
		GtkTreeModel *oldmodel = gtk_tree_model_sort_get_model 
					(GTK_TREE_MODEL_SORT (oldsortable));
		if (oldmodel)
			g_object_unref (G_OBJECT (oldmodel));
		g_object_unref (G_OBJECT (oldsortable));
	} 

	gtk_tree_view_set_model (header_view, model);

	return;
}

static GtkTreeModel *empty_model;

static void 
reload_accounts (TnyDemouiSummaryViewPriv *priv)
{
	TnyAccountStore *account_store = priv->account_store;
	GtkTreeModel *sortable, *maccounts, *mailbox_model;
	TnyFolderStoreQuery *query;

	g_mutex_lock (priv->monitor_lock);
	{

		if (priv->monitor)
		{
			tny_folder_monitor_stop (priv->monitor);
			g_object_unref (G_OBJECT (priv->monitor));
		}
		priv->monitor = NULL;
	}
	g_mutex_unlock (priv->monitor_lock);

	/* Show only subscribed folders */
	query = tny_folder_store_query_new ();
	tny_folder_store_query_add_item (query, NULL, 
					 TNY_FOLDER_STORE_QUERY_OPTION_SUBSCRIBED);

	/* TnyGtkFolderStoreTreeModel is also a TnyList (it simply implements both the
	   TnyList and the GtkTreeModel interfaces) */
#if PLATFORM==1
	mailbox_model = tny_gtk_folder_store_tree_model_new (TRUE, NULL);
#else
	mailbox_model = tny_gtk_folder_store_tree_model_new (FALSE, NULL);
#endif

	g_object_unref (G_OBJECT (query));

	TnyList *accounts = TNY_LIST (mailbox_model);

	maccounts = tny_gtk_account_list_model_new ();

	/* Clear the header_view by giving it an empty model */
	if (G_UNLIKELY (!empty_model))
		empty_model = GTK_TREE_MODEL (gtk_list_store_new (1, G_TYPE_STRING));
	set_header_view_model (GTK_TREE_VIEW (priv->header_view), empty_model);

	tny_msg_view_clear (priv->msg_view);

	if (priv->current_accounts)
	{
		g_object_unref (G_OBJECT (priv->current_accounts));
		priv->current_accounts = NULL;
	}

	/* This method uses the TnyFolderStoreTreeModel as a TnyList */
	tny_account_store_get_accounts (account_store, accounts,
		TNY_ACCOUNT_STORE_STORE_ACCOUNTS);

	priv->current_accounts = TNY_LIST (g_object_ref (G_OBJECT (accounts)));

	tny_account_store_get_accounts (account_store, TNY_LIST (maccounts),
			TNY_ACCOUNT_STORE_STORE_ACCOUNTS);
	gtk_combo_box_set_model (priv->account_view, maccounts);

	/* Here we use the TnyFolderStoreTreeModel as a GtkTreeModel */
	sortable = gtk_tree_model_sort_new_with_model (mailbox_model);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (sortable),
				TNY_GTK_FOLDER_STORE_TREE_MODEL_NAME_COLUMN, 
				GTK_SORT_ASCENDING);

	/* Set the model of the mailbox_view */
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->mailbox_view), 
		sortable);

	return;
}

static void
accounts_reloaded (TnyAccountStore *store, gpointer user_data)
{
	TnyDemouiSummaryViewPriv *priv = user_data;
	reload_accounts (priv);
	return;
}

static void 
online_button_toggled (GtkToggleButton *togglebutton, gpointer user_data)
{
	TnySummaryView *self = user_data;
	TnyDemouiSummaryViewPriv *priv = TNY_DEMOUI_SUMMARY_VIEW_GET_PRIVATE (self);

	if (!priv->handle_recon)
		return;

	if (priv->account_store)
	{
		TnyDevice *device = tny_account_store_get_device (priv->account_store);

		if (gtk_toggle_button_get_active (togglebutton))
			tny_device_force_online (device);
		else
			tny_device_force_offline (device);

		g_object_unref (G_OBJECT (device));
	}
}

static void
connection_changed (TnyDevice *device, gboolean online, gpointer user_data)
{
	TnySummaryView *self = user_data;
	TnyDemouiSummaryViewPriv *priv = TNY_DEMOUI_SUMMARY_VIEW_GET_PRIVATE (self);

	priv->handle_recon = FALSE;

	if (online)
	{
		gtk_button_set_label  (GTK_BUTTON (priv->online_button), GO_OFFLINE_TXT);
		g_signal_handler_block (G_OBJECT (priv->online_button), priv->online_button_signal);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->online_button), TRUE);
		g_signal_handler_unblock (G_OBJECT (priv->online_button), priv->online_button_signal);
	} else {

		gtk_button_set_label  (GTK_BUTTON (priv->online_button), GO_ONLINE_TXT);
		g_signal_handler_block (G_OBJECT (priv->online_button), priv->online_button_signal);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->online_button), FALSE);
		g_signal_handler_unblock (G_OBJECT (priv->online_button), priv->online_button_signal);
	}

	priv->handle_recon = TRUE;

	return;
}

static void
tny_demoui_summary_view_set_account_store (TnyAccountStoreView *self, TnyAccountStore *account_store)
{
	TnyDemouiSummaryViewPriv *priv = TNY_DEMOUI_SUMMARY_VIEW_GET_PRIVATE (self);
	TnyDevice *device = tny_account_store_get_device (account_store);

	if (priv->account_store)
	{ 
		TnyDevice *odevice = tny_account_store_get_device (priv->account_store);

		if (g_signal_handler_is_connected (G_OBJECT (odevice), priv->connchanged_signal))
		{
			g_signal_handler_disconnect (G_OBJECT (odevice), 
				priv->connchanged_signal);
		}

		g_signal_handler_disconnect (G_OBJECT (priv->account_store),
			priv->accounts_reloaded_signal);

		g_object_unref (G_OBJECT (priv->account_store));
		g_object_unref (G_OBJECT (odevice));
	}

	g_object_ref (G_OBJECT (account_store));

	priv->connchanged_signal =  g_signal_connect (
			G_OBJECT (device), "connection_changed",
			G_CALLBACK (connection_changed), self);	

	priv->account_store = account_store;

	priv->accounts_reloaded_signal = g_signal_connect (
		G_OBJECT (account_store), "accounts_reloaded",
		G_CALLBACK (accounts_reloaded), priv);

	reload_accounts (priv);

	g_object_unref (G_OBJECT (device));

	return;
}

static void
on_header_view_key_press_event (GtkTreeView *header_view, GdkEventKey *event, gpointer user_data)
{
	if (event->keyval == GDK_Delete)
	{
		GtkTreeSelection *selection = gtk_tree_view_get_selection (header_view);
		GtkTreeModel *model, *mymodel;
		GtkTreeIter iter;

		if (gtk_tree_selection_get_selected (selection, &model, &iter))
		{
			TnyHeader *header;

			gtk_tree_model_get (model, &iter, 
				TNY_GTK_HEADER_LIST_MODEL_INSTANCE_COLUMN, 
				&header, -1);

			if (header)
			{
				GtkWidget *dialog = gtk_message_dialog_new (NULL, 
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, 
					_("This will remove the message with subject \"%s\""),
					tny_header_get_subject (header));

				if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_YES)
				{
					TnyFolder *folder;

					if (GTK_IS_TREE_MODEL_SORT (model))
					{
						mymodel = gtk_tree_model_sort_get_model 
							(GTK_TREE_MODEL_SORT (model));
					} else mymodel = model;

					tny_list_remove (TNY_LIST (mymodel), G_OBJECT (header));
					folder = tny_header_get_folder (header);
					tny_folder_remove_msg (folder, header, NULL);
					tny_folder_sync (folder, TRUE, NULL);
					g_object_unref (G_OBJECT (folder));
				}

				gtk_widget_destroy (dialog);
				g_object_unref (G_OBJECT (header));
			}
		}
		
	}

	return;
}

static void
on_header_view_tree_selection_changed (GtkTreeSelection *selection, 
		gpointer user_data)
{
	TnyDemouiSummaryView *self  = user_data;
	TnyDemouiSummaryViewPriv *priv = TNY_DEMOUI_SUMMARY_VIEW_GET_PRIVATE (self);
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (G_LIKELY (gtk_tree_selection_get_selected (selection, &model, &iter)))
	{
		TnyHeader *header;

		gtk_tree_model_get (model, &iter, 
			TNY_GTK_HEADER_LIST_MODEL_INSTANCE_COLUMN, 
			&header, -1);
	
		if (G_LIKELY (header))
		{
			TnyFolder *folder;
			TnyMsg *msg;

			folder = tny_header_get_folder (header);
			if (G_LIKELY (folder))
			{
				GError *err = NULL;

				msg = tny_folder_get_msg (folder, header, &err);
				if (G_LIKELY (msg))
				{
					tny_msg_view_set_msg (priv->msg_view, msg);
					g_object_unref (G_OBJECT (msg));
				} else { 
					tny_msg_view_set_unavailable (priv->msg_view);
				}

				if (err != NULL)
				{
					GtkWidget *edialog;

					edialog = gtk_message_dialog_new (
							  GTK_WINDOW (gtk_widget_get_parent (GTK_WIDGET (self))),
							  GTK_DIALOG_DESTROY_WITH_PARENT,
							  GTK_MESSAGE_ERROR,
							  GTK_BUTTONS_CLOSE,
							  err->message);
					g_signal_connect_swapped (edialog, "response",
						G_CALLBACK (gtk_widget_destroy), edialog);
					gtk_widget_show_all (edialog);
					g_error_free (err);
				}

				g_object_unref (G_OBJECT (folder));
			}

			g_object_unref (G_OBJECT (header));
		
		} else {
			tny_msg_view_set_unavailable (priv->msg_view);
		}
	}

	return;
}

static gboolean
cleanup_statusbar (gpointer data)
{
	TnyDemouiSummaryViewPriv *priv = data;

	gtk_widget_hide (GTK_WIDGET (priv->progress));
	gtk_statusbar_pop (GTK_STATUSBAR (priv->status), priv->status_id);

	return FALSE;
}


static void
refresh_current_folder (TnyFolder *folder, gboolean cancelled, GError **err, gpointer user_data)
{
	TnyDemouiSummaryViewPriv *priv = user_data;
	GtkTreeModel *select_model;

	if (!cancelled)
	{
		GtkSelectionMode mode;

		g_idle_add (cleanup_statusbar, priv);

		mode = gtk_tree_selection_get_mode (priv->mailbox_select);

		if (mode == GTK_SELECTION_SINGLE)
		{
			gtk_tree_selection_get_selected (priv->mailbox_select, &select_model, 
				&priv->last_mailbox_correct_select);
			priv->last_mailbox_correct_select_set = TRUE;
		}

		gtk_widget_set_sensitive (GTK_WIDGET (priv->header_view), TRUE);

	} else {
		/* Restore selection */
		g_signal_handler_block (G_OBJECT (priv->mailbox_select), 
			priv->mailbox_select_sid);
		gtk_tree_selection_select_iter (priv->mailbox_select, 
			&priv->last_mailbox_correct_select);
		g_signal_handler_unblock (G_OBJECT (priv->mailbox_select), 
			priv->mailbox_select_sid);
	}

	return;
}


static void
refresh_current_folder_status_update (GObject *sender, TnyStatus *status, gpointer user_data)
{
	gchar *new_what;

	TnyDemouiSummaryViewPriv *priv = user_data;

	if (!user_data)
		return;

	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (priv->progress), 
		tny_status_get_fraction (status));
	gtk_statusbar_pop (GTK_STATUSBAR (priv->status), priv->status_id);

	new_what = g_strdup_printf ("%s (%d/%d)", status->message, status->position, 
		status->of_total);

	gtk_statusbar_push (GTK_STATUSBAR (priv->status), priv->status_id, new_what);
	g_free (new_what);

	return;
}

static void
on_mailbox_view_tree_selection_changed (GtkTreeSelection *selection, 
		gpointer user_data)
{
	TnyDemouiSummaryViewPriv *priv = user_data;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreeModel *hmodel;
	GtkTreeView *header_view = GTK_TREE_VIEW (priv->header_view);
	GtkTreeModel *sortable;
	gboolean refresh = FALSE;
	GtkSelectionMode mode;

#ifndef ASYNC_HEADERS
	refresh = TRUE;
#endif

	mode = gtk_tree_selection_get_mode (selection);

	if (mode == GTK_SELECTION_SINGLE)
	{
	  if (gtk_tree_selection_get_selected (selection, &model, &iter))
	  {
		TnyFolder *folder;
		gint type;

		gtk_tree_model_get (model, &iter, 
			TNY_GTK_FOLDER_STORE_TREE_MODEL_TYPE_COLUMN, 
			&type, -1);

		if (type == TNY_FOLDER_TYPE_ROOT) 
		{ 
			/* If an "account name"-row was clicked */
			if (priv->last_mailbox_correct_select_set) 
			{
				g_signal_handler_block (G_OBJECT (priv->mailbox_select), priv->mailbox_select_sid);
				gtk_tree_selection_select_iter (priv->mailbox_select, &priv->last_mailbox_correct_select);
				g_signal_handler_unblock (G_OBJECT (priv->mailbox_select), priv->mailbox_select_sid);
			}

			priv->last_mailbox_correct_select_set = FALSE;

		} else {

			priv->last_mailbox_correct_select_set = FALSE;

			gtk_tree_model_get (model, &iter, 
				TNY_GTK_FOLDER_STORE_TREE_MODEL_INSTANCE_COLUMN, 
				&folder, -1);

			gtk_widget_show (GTK_WIDGET (priv->progress));
			gtk_widget_set_sensitive (GTK_WIDGET (priv->header_view), FALSE);

			hmodel = tny_gtk_header_list_model_new ();
			tny_gtk_header_list_model_set_folder (TNY_GTK_HEADER_LIST_MODEL (hmodel), folder, FALSE);

			g_mutex_lock (priv->monitor_lock);
			{
				if (priv->monitor) {
					tny_folder_monitor_stop (priv->monitor);
					g_object_unref (G_OBJECT (priv->monitor));
				}
				priv->monitor = TNY_FOLDER_MONITOR (tny_folder_monitor_new (folder));
				tny_folder_monitor_add_list (priv->monitor, TNY_LIST (hmodel));
				tny_folder_monitor_start (priv->monitor);
			}
			g_mutex_unlock (priv->monitor_lock);

			sortable = gtk_tree_model_sort_new_with_model (GTK_TREE_MODEL (hmodel));

			gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (sortable),
				TNY_GTK_HEADER_LIST_MODEL_DATE_RECEIVED_COLUMN,
				tny_gtk_header_list_model_received_date_sort_func, 
				NULL, NULL);

			gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (sortable),
				TNY_GTK_HEADER_LIST_MODEL_DATE_SENT_COLUMN,
				tny_gtk_header_list_model_sent_date_sort_func, 
				NULL, NULL);

			set_header_view_model (header_view, sortable);

			tny_folder_refresh_async (folder, 
				refresh_current_folder, 
				refresh_current_folder_status_update, user_data);

			g_object_unref (G_OBJECT (folder));
		}
	  }
	} else {
		GList *list;
		TnyFolder *merge = tny_merge_folder_new ();
		GtkTreeView *header_view = GTK_TREE_VIEW (priv->header_view);

		list = gtk_tree_selection_get_selected_rows (priv->mailbox_select, &model);

		while (list)
		{
			GtkTreePath *path = list->data;
			GtkTreeIter iter;
			TnyFolder *folder;

			gtk_tree_model_get_iter (model, &iter, path);
			gtk_tree_model_get (model, &iter, 
				TNY_GTK_FOLDER_STORE_TREE_MODEL_INSTANCE_COLUMN, 
				&folder, -1);

			if (folder && TNY_IS_FOLDER (folder))
			{
				tny_merge_folder_add_folder (TNY_MERGE_FOLDER (merge), folder);
				g_object_unref (folder);
			}

			gtk_tree_path_free (path);
			list = g_list_next (list);
		}

		hmodel = tny_gtk_header_list_model_new ();
		tny_gtk_header_list_model_set_folder (TNY_GTK_HEADER_LIST_MODEL (hmodel), 
			merge, FALSE);

		g_mutex_lock (priv->monitor_lock);
		{
			if (priv->monitor) {
				tny_folder_monitor_stop (priv->monitor);
				g_object_unref (G_OBJECT (priv->monitor));
			}
			priv->monitor = TNY_FOLDER_MONITOR (tny_folder_monitor_new (merge));
			tny_folder_monitor_add_list (priv->monitor, TNY_LIST (hmodel));
			tny_folder_monitor_start (priv->monitor);
		}
		g_mutex_unlock (priv->monitor_lock);

		sortable = gtk_tree_model_sort_new_with_model (GTK_TREE_MODEL (hmodel));

		gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (sortable),
			TNY_GTK_HEADER_LIST_MODEL_DATE_RECEIVED_COLUMN,
			tny_gtk_header_list_model_received_date_sort_func, 
			NULL, NULL);

		gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (sortable),
			TNY_GTK_HEADER_LIST_MODEL_DATE_SENT_COLUMN,
			tny_gtk_header_list_model_sent_date_sort_func, 
			NULL, NULL);

		set_header_view_model (header_view, sortable);

		tny_folder_refresh_async (merge, 
			refresh_current_folder, 
			refresh_current_folder_status_update, user_data);

		g_list_free (list);
	}

	return;
}


static void
on_header_view_tree_row_activated (GtkTreeView *treeview, GtkTreePath *path,
			GtkTreeViewColumn *col,  gpointer userdata)
{
	TnyDemouiSummaryViewPriv *priv = userdata;
	GtkTreeModel *model;
	GtkTreeIter iter;
		
	model = gtk_tree_view_get_model(treeview);
    
	if (G_LIKELY (gtk_tree_model_get_iter(model, &iter, path)))
	{
		TnyHeader *header;
		TnyMsgWindow *msgwin;

		gtk_tree_model_get (model, &iter, 
			TNY_GTK_HEADER_LIST_MODEL_INSTANCE_COLUMN, 
			&header, -1);
		
		if (G_LIKELY (header))
		{
			TnyFolder *folder;
			TnyMsg *msg;
			TnyPlatformFactory *platfact;


#if PLATFORM==1
			platfact = tny_gnome_platform_factory_get_instance ();    
#endif

#if PLATFORM==2
			platfact = tny_maemo_platform_factory_get_instance ();    
#endif

#if PLATFORM==3
			platfact = tny_gpe_platform_factory_get_instance ();
#endif

#if PLATFORM==4
			platfact = tny_olpc_platform_factory_get_instance ();    
#endif

			folder = tny_header_get_folder (TNY_HEADER (header));

			if (G_LIKELY (folder))
			{
				msg = tny_folder_get_msg (folder, header, NULL);
				if (G_LIKELY (msg))
				{

/* DEBUG		
					TnyAccountStore *astore = priv->account_store;
					TnyList *accs = tny_simple_list_new ();
					tny_account_store_get_accounts (astore, accs, TNY_ACCOUNT_STORE_TRANSPORT_ACCOUNTS);
					TnyIterator *iter = tny_list_create_iterator (accs);
					TnyCamelTransportAccount *acc = (TnyCamelTransportAccount *) tny_iterator_get_current (iter);

					g_print ("--> %s\n", tny_account_get_name (TNY_ACCOUNT (acc)));
		
					if (!queue)
						queue = tny_camel_send_queue_new (acc);
		
					tny_send_queue_add (queue, msg, NULL);

					g_object_unref (G_OBJECT (acc));
					g_object_unref (G_OBJECT (iter));
					g_object_unref (G_OBJECT (accs));
*/

					msgwin = tny_gtk_msg_window_new (
						tny_platform_factory_new_msg_view (platfact));

					tny_msg_view_set_msg (TNY_MSG_VIEW (msgwin), msg);
					g_object_unref (G_OBJECT (msg));
				
					gtk_widget_show (GTK_WIDGET (msgwin));
				} else {



					msgwin = tny_gtk_msg_window_new (
						tny_platform_factory_new_msg_view (platfact));

					tny_msg_view_set_unavailable (TNY_MSG_VIEW (msgwin));
			
					gtk_widget_show (GTK_WIDGET (msgwin));
				}
				g_object_unref (G_OBJECT (folder));
			}
			g_object_unref (G_OBJECT (header));
		}
	}
}

#ifdef QUEUES
static TnyGetMsgQueue *fullqueue = NULL;

static void 
on_full_download_folder_activate (GtkMenuItem *mitem, gpointer user_data)
{
	TnyDemouiSummaryView *self = user_data;
	TnyDemouiSummaryViewPriv *priv = TNY_DEMOUI_SUMMARY_VIEW_GET_PRIVATE (self);
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (gtk_tree_selection_get_selected (priv->mailbox_select, &model, &iter))
	{
		gint type;

		gtk_tree_model_get (model, &iter, 
			TNY_GTK_FOLDER_STORE_TREE_MODEL_TYPE_COLUMN, 
			&type, -1);

		if (type != TNY_FOLDER_TYPE_ROOT) 
		{ 
			TnyFolder *folder;

			gtk_tree_model_get (model, &iter, 
				TNY_GTK_FOLDER_STORE_TREE_MODEL_INSTANCE_COLUMN, 
				&folder, -1);

			if (!fullqueue)
				fullqueue = tny_get_msg_queue_new ();

			tny_get_msg_queue_full_msg_retrieval (fullqueue, folder, 
				NULL, NULL, refresh_current_folder_status_update, priv);

			g_object_unref (G_OBJECT (folder));

		}
	}
}
#endif

static void 
on_rename_folder_activate (GtkMenuItem *mitem, gpointer user_data)
{
	TnyDemouiSummaryView *self = user_data;
	TnyDemouiSummaryViewPriv *priv = TNY_DEMOUI_SUMMARY_VIEW_GET_PRIVATE (self);
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (gtk_tree_selection_get_selected (priv->mailbox_select, &model, &iter))
	{
		gint type;

		gtk_tree_model_get (model, &iter, 
			TNY_GTK_FOLDER_STORE_TREE_MODEL_TYPE_COLUMN, 
			&type, -1);

		if (type != TNY_FOLDER_TYPE_ROOT) 
		{ 
			TnyFolder *folder;
			GtkWidget *dialog, *entry;
			gint result;

			gtk_tree_model_get (model, &iter, 
				TNY_GTK_FOLDER_STORE_TREE_MODEL_INSTANCE_COLUMN, 
				&folder, -1);

			dialog = gtk_dialog_new_with_buttons (_("Rename a folder"),
												  GTK_WINDOW (gtk_widget_get_parent (GTK_WIDGET (self))),
												  GTK_DIALOG_MODAL,
												  GTK_STOCK_OK,
												  GTK_RESPONSE_ACCEPT,
												  GTK_STOCK_CANCEL,
												  GTK_RESPONSE_REJECT,
												  NULL);

			entry = gtk_entry_new ();
			gtk_entry_set_text (GTK_ENTRY (entry), tny_folder_get_name (folder));
			gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), entry);
			gtk_widget_show (entry);

			result = gtk_dialog_run (GTK_DIALOG (dialog));

			switch (result)
			{
				case GTK_RESPONSE_ACCEPT: 
				{
					GError *err = NULL;
					const gchar *newname = gtk_entry_get_text (GTK_ENTRY (entry));
					tny_folder_set_name (folder, newname, &err);

					if (err != NULL)
					{
						GtkWidget *edialog;

						gtk_widget_destroy (dialog);
						dialog = NULL;

						edialog = gtk_message_dialog_new (
										  GTK_WINDOW (gtk_widget_get_parent (GTK_WIDGET (self))),
										  GTK_DIALOG_DESTROY_WITH_PARENT,
										  GTK_MESSAGE_ERROR,
										  GTK_BUTTONS_CLOSE,
										  err->message);
						g_signal_connect_swapped (edialog, "response",
							G_CALLBACK (gtk_widget_destroy), edialog);
						gtk_widget_show_all (edialog);
						g_error_free (err);
					}
				}
				break;

				default:
				break;
			}

			if (dialog)
				gtk_widget_destroy (dialog);
			g_object_unref (G_OBJECT (folder));
		}

	}

}

static void 
on_delete_folder_activate (GtkMenuItem *mitem, gpointer user_data)
{
	TnyDemouiSummaryView *self = user_data;
	TnyDemouiSummaryViewPriv *priv = TNY_DEMOUI_SUMMARY_VIEW_GET_PRIVATE (self);
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (gtk_tree_selection_get_selected (priv->mailbox_select, &model, &iter))
	{
		gint type;

		gtk_tree_model_get (model, &iter, 
			TNY_GTK_FOLDER_STORE_TREE_MODEL_TYPE_COLUMN, 
			&type, -1);

		if (type != TNY_FOLDER_TYPE_ROOT) 
		{ 
			TnyFolder *folder;
			GtkWidget *dialog, *label;
			gint result;

			gtk_tree_model_get (model, &iter, 
				TNY_GTK_FOLDER_STORE_TREE_MODEL_INSTANCE_COLUMN, 
				&folder, -1);

			dialog = gtk_dialog_new_with_buttons (_("Delete a folder"),
												  GTK_WINDOW (gtk_widget_get_parent (GTK_WIDGET (self))),
												  GTK_DIALOG_MODAL,
												  GTK_STOCK_OK,
												  GTK_RESPONSE_ACCEPT,
												  GTK_STOCK_CANCEL,
												  GTK_RESPONSE_REJECT,
												  NULL);

			label = gtk_label_new (_("Are you sure you want to delete this folder?"));
			gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
			gtk_widget_show (label);

			result = gtk_dialog_run (GTK_DIALOG (dialog));

			switch (result)
			{
				case GTK_RESPONSE_ACCEPT: 
				{
					GError *err = NULL;
					TnyFolderStore *folderstore = tny_folder_get_folder_store (folder);

					tny_folder_store_remove_folder (folderstore, folder, &err);

					if (err != NULL)
					{
						GtkWidget *edialog;

						gtk_widget_destroy (dialog);
						dialog = NULL;

						edialog = gtk_message_dialog_new (
										  GTK_WINDOW (gtk_widget_get_parent (GTK_WIDGET (self))),
										  GTK_DIALOG_DESTROY_WITH_PARENT,
										  GTK_MESSAGE_ERROR,
										  GTK_BUTTONS_CLOSE,
										  err->message);
						g_signal_connect_swapped (edialog, "response",
							G_CALLBACK (gtk_widget_destroy), edialog);
						gtk_widget_show_all (edialog);
						g_error_free (err);
					}

					g_object_unref (G_OBJECT (folderstore));
				}
				break;

				default:
				break;
			}

			if (dialog)
				gtk_widget_destroy (dialog);
			g_object_unref (G_OBJECT (folder));
		}

	}
}

static void 
on_create_folder_activate (GtkMenuItem *mitem, gpointer user_data)
{
	TnyDemouiSummaryView *self = user_data;
	TnyDemouiSummaryViewPriv *priv = TNY_DEMOUI_SUMMARY_VIEW_GET_PRIVATE (self);
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (gtk_tree_selection_get_selected (priv->mailbox_select, &model, &iter))
	{
		gint type;

		gtk_tree_model_get (model, &iter, 
			TNY_GTK_FOLDER_STORE_TREE_MODEL_TYPE_COLUMN, 
			&type, -1);

		if (type != TNY_FOLDER_TYPE_ROOT) 
		{ 
			TnyFolderStore *folderstore;
			GtkWidget *dialog, *entry;
			gint result;

			gtk_tree_model_get (model, &iter, 
				TNY_GTK_FOLDER_STORE_TREE_MODEL_INSTANCE_COLUMN, 
				&folderstore, -1);

			dialog = gtk_dialog_new_with_buttons (_("Create a folder"),
												  GTK_WINDOW (gtk_widget_get_parent (GTK_WIDGET (self))),
												  GTK_DIALOG_MODAL,
												  GTK_STOCK_OK,
												  GTK_RESPONSE_ACCEPT,
												  GTK_STOCK_CANCEL,
												  GTK_RESPONSE_REJECT,
												  NULL);

			entry = gtk_entry_new ();
			gtk_entry_set_text (GTK_ENTRY (entry), _("New folder"));
			gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), entry);
			gtk_widget_show (entry);

			result = gtk_dialog_run (GTK_DIALOG (dialog));

			switch (result)
			{
				case GTK_RESPONSE_ACCEPT: 
				{
					GError *err = NULL;
					const gchar *newname = gtk_entry_get_text (GTK_ENTRY (entry));

					TnyFolder *created = tny_folder_store_create_folder (folderstore, newname, &err);

					if (created) /* Can be NULL on failure */
						g_object_unref (G_OBJECT (created));

					if (err != NULL)
					{
						GtkWidget *edialog;

						gtk_widget_destroy (dialog);
						dialog = NULL;

						edialog = gtk_message_dialog_new (
										  GTK_WINDOW (gtk_widget_get_parent (GTK_WIDGET (self))),
										  GTK_DIALOG_DESTROY_WITH_PARENT,
										  GTK_MESSAGE_ERROR,
										  GTK_BUTTONS_CLOSE,
										  err->message);
						g_signal_connect_swapped (edialog, "response",
							G_CALLBACK (gtk_widget_destroy), edialog);
						gtk_widget_show_all (edialog);
						g_error_free (err);
					}
				}
				break;

				default:
				break;
			}

			if (dialog)
				gtk_widget_destroy (dialog);
			g_object_unref (G_OBJECT (folderstore));
		}

	}
}


static void 
on_merge_view_activate (GtkMenuItem *mitem, gpointer user_data)
{
	TnyDemouiSummaryView *self = user_data;
	TnyDemouiSummaryViewPriv *priv = TNY_DEMOUI_SUMMARY_VIEW_GET_PRIVATE (self);
	GtkSelectionMode mode;

	mode = gtk_tree_selection_get_mode (priv->mailbox_select);

	if (mode == GTK_SELECTION_SINGLE)
		gtk_tree_selection_set_mode (priv->mailbox_select, GTK_SELECTION_MULTIPLE);
	else
		gtk_tree_selection_set_mode (priv->mailbox_select, GTK_SELECTION_SINGLE);

}

static void
header_view_do_popup_menu (GtkWidget *my_widget, GdkEventButton *event, gpointer user_data)
{
	GtkWidget *menu;
	int button, event_time;

	menu = gtk_menu_new ();
	g_signal_connect (menu, "deactivate", G_CALLBACK (gtk_widget_destroy), NULL);

	/* ... add menu items ... */

	if (event)
	{
		button = event->button;
		event_time = event->time;
	}
	else
	{
		button = 0;
		event_time = gtk_get_current_event_time ();
	}

	gtk_menu_attach_to_widget (GTK_MENU (menu), my_widget, NULL);
	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, 
					button, event_time);
}

static gboolean 
on_header_view_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	if (event->button == 3 && event->type == GDK_BUTTON_PRESS)
	{
		header_view_do_popup_menu (widget, event, user_data);
		return TRUE;
	}

	return FALSE;
}

static gboolean
header_view_popup_menu_event (GtkWidget *widget, gpointer user_data)
{
	header_view_do_popup_menu (widget, NULL, user_data);
	return TRUE;
}


static void
mailbox_view_do_popup_menu (GtkWidget *my_widget, GdkEventButton *event, gpointer user_data)
{
	TnyDemouiSummaryView *self = user_data;
	TnyDemouiSummaryViewPriv *priv = TNY_DEMOUI_SUMMARY_VIEW_GET_PRIVATE (self);
	GtkWidget *menu;
	int button, event_time;
	GtkSelectionMode mode;
	GtkWidget *mrename, *mdelete, *mcreate, *mmerge;
#ifdef QUEUES
	GtkWidget *fdown;
#endif

	menu = gtk_menu_new ();

	mrename = gtk_menu_item_new_with_label (_("Rename folder"));
	mcreate = gtk_menu_item_new_with_label (_("Create folder"));
	mdelete = gtk_menu_item_new_with_label (_("Delete folder"));

#ifdef QUEUES
	fdown = gtk_menu_item_new_with_label (_("Download entire folder"));
#endif

	mode = gtk_tree_selection_get_mode (priv->mailbox_select);

	if (mode == GTK_SELECTION_SINGLE)
		mmerge = gtk_menu_item_new_with_label (_("Merge view of selected mode"));
	else
		mmerge = gtk_menu_item_new_with_label (_("Select one folder mode"));

	g_signal_connect (G_OBJECT (mrename), "activate",
		G_CALLBACK (on_rename_folder_activate), user_data);
	g_signal_connect (G_OBJECT (mcreate), "activate",
		G_CALLBACK (on_create_folder_activate), user_data);
	g_signal_connect (G_OBJECT (mdelete), "activate",
		G_CALLBACK (on_delete_folder_activate), user_data);
	g_signal_connect (G_OBJECT (mmerge), "activate",
		G_CALLBACK (on_merge_view_activate), user_data);
#ifdef QUEUES
	g_signal_connect (G_OBJECT (fdown), "activate",
		G_CALLBACK (on_full_download_folder_activate), user_data);
#endif

	gtk_menu_prepend (menu, mrename);
	gtk_menu_prepend (menu, mcreate);
	gtk_menu_prepend (menu, mdelete);
	gtk_menu_prepend (menu, mmerge);
#ifdef QUEUES
	gtk_menu_prepend (menu, fdown);

	gtk_widget_show (fdown);
#endif
	gtk_widget_show (mrename);
	gtk_widget_show (mcreate);
	gtk_widget_show (mdelete);
	gtk_widget_show (mmerge);

	if (event)
	{
		button = event->button;
		event_time = event->time;
	}
	else
	{
		button = 0;
		event_time = gtk_get_current_event_time ();
	}

	gtk_menu_attach_to_widget (GTK_MENU (menu), my_widget, NULL);
	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, 
					button, event_time);
}

static gboolean 
on_mailbox_view_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	if (event->button == 3 && event->type == GDK_BUTTON_PRESS)
	{
		mailbox_view_do_popup_menu (widget, event, user_data);
		return TRUE;
	}

	return FALSE;
}

static gboolean
mailbox_view_popup_menu_event (GtkWidget *widget, gpointer user_data)
{
	mailbox_view_do_popup_menu (widget, NULL, user_data);
	return FALSE;
}

/**
 * tny_demoui_summary_view_new:
 * 
 *
 * Return value: A new #TnySummaryView instance implemented for Gtk+
 **/
TnySummaryView*
tny_demoui_summary_view_new (void)
{
	TnyDemouiSummaryView *self = g_object_new (TNY_TYPE_DEMOUI_SUMMARY_VIEW, NULL);

	return TNY_SUMMARY_VIEW (self);
}

static void
tny_demoui_summary_view_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyDemouiSummaryView *self = (TnyDemouiSummaryView *)instance;
	TnyDemouiSummaryViewPriv *priv = TNY_DEMOUI_SUMMARY_VIEW_GET_PRIVATE (self);
	TnyPlatformFactory *platfact;
	GtkVBox *vbox = GTK_VBOX (self);
	GtkWidget *mailbox_sw, *widget;
	GtkWidget *header_sw;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *select;
	GtkWidget *hpaned1;
	GtkWidget *vpaned1;
	GtkBox *vpbox;

	/* TODO: Persist application UI status (of the panes) */

	priv->handle_recon = TRUE;
	priv->monitor_lock = g_mutex_new ();
	priv->monitor = NULL;

	priv->last_mailbox_correct_select_set = FALSE;
	priv->online_button = gtk_toggle_button_new_with_label (GO_ONLINE_TXT);
	priv->current_accounts = NULL;

	priv->online_button_signal = g_signal_connect (G_OBJECT (priv->online_button), "toggled", 
		G_CALLBACK (online_button_toggled), self);

#if PLATFORM==1
	platfact = tny_gnome_platform_factory_get_instance ();
#endif

#if PLATFORM==2
	platfact = tny_maemo_platform_factory_get_instance ();
#endif

#if PLATFORM==3
	platfact = tny_gpe_platform_factory_get_instance ();
#endif

#if PLATFORM==4
	platfact = tny_olpc_platform_factory_get_instance ();
#endif


	hpaned1 = gtk_hpaned_new ();
	gtk_widget_show (hpaned1);
	priv->status = GTK_WIDGET (gtk_statusbar_new ());
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (priv->status), FALSE);
	priv->progress = gtk_progress_bar_new ();
	priv->status_id = gtk_statusbar_get_context_id (GTK_STATUSBAR (priv->status), "default");

	gtk_box_pack_start (GTK_BOX (priv->status), priv->progress, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (priv->status), priv->online_button, FALSE, FALSE, 0);

	gtk_widget_show (priv->online_button);
	gtk_widget_show (priv->status);
	gtk_widget_show (GTK_WIDGET (vbox));

	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (hpaned1), TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (priv->status), FALSE, TRUE, 0);

	vpaned1 = gtk_vpaned_new ();
	gtk_widget_show (vpaned1);
	
	priv->msg_view = tny_platform_factory_new_msg_view (platfact);

	gtk_widget_show (GTK_WIDGET (priv->msg_view));

	widget = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (widget), 
				GTK_SHADOW_NONE);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (widget), 
			GTK_WIDGET (priv->msg_view));

	gtk_widget_show (widget);

	gtk_paned_pack2 (GTK_PANED (vpaned1), widget, TRUE, TRUE);

	priv->account_view = GTK_COMBO_BOX (gtk_combo_box_new ());
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start((GtkCellLayout *)priv->account_view, renderer, TRUE);
	gtk_cell_layout_set_attributes((GtkCellLayout *)priv->account_view, renderer, "text", 0, NULL);

	mailbox_sw = gtk_scrolled_window_new (NULL, NULL);
	vpbox = GTK_BOX (gtk_vbox_new (FALSE, 0));
	gtk_box_pack_start (GTK_BOX (vpbox), GTK_WIDGET (priv->account_view), FALSE, FALSE, 0);    
	gtk_box_pack_start (GTK_BOX (vpbox), mailbox_sw, TRUE, TRUE, 0);
	gtk_widget_show (GTK_WIDGET (vpbox));    
	gtk_widget_show (GTK_WIDGET (priv->account_view));

	gtk_paned_pack1 (GTK_PANED (hpaned1), GTK_WIDGET (vpbox), TRUE, TRUE);
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
	column = gtk_tree_view_column_new_with_attributes (_("Folder"), renderer,
			"text", TNY_GTK_FOLDER_STORE_TREE_MODEL_NAME_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, TNY_GTK_FOLDER_STORE_TREE_MODEL_NAME_COLUMN);

	gtk_tree_view_append_column (GTK_TREE_VIEW(priv->mailbox_view), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Unread"), renderer,
		"text", TNY_GTK_FOLDER_STORE_TREE_MODEL_UNREAD_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, TNY_GTK_FOLDER_STORE_TREE_MODEL_UNREAD_COLUMN);
	gtk_tree_view_append_column (GTK_TREE_VIEW(priv->mailbox_view), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Total"), renderer,
		"text", TNY_GTK_FOLDER_STORE_TREE_MODEL_ALL_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, TNY_GTK_FOLDER_STORE_TREE_MODEL_ALL_COLUMN);
	gtk_tree_view_append_column (GTK_TREE_VIEW(priv->mailbox_view), column);

	/* TODO: Persist application UI status */
	/* header_view columns */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("From"), renderer,
		"text", TNY_GTK_HEADER_LIST_MODEL_FROM_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, TNY_GTK_HEADER_LIST_MODEL_FROM_COLUMN);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 100);
	gtk_tree_view_append_column (GTK_TREE_VIEW(priv->header_view), column);

	if (G_UNLIKELY (FALSE))
	{ /* Unlikely ;-) */
		renderer = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes (_("To"), renderer,
			"text", TNY_GTK_HEADER_LIST_MODEL_TO_COLUMN, NULL);
		gtk_tree_view_column_set_sort_column_id (column, TNY_GTK_HEADER_LIST_MODEL_TO_COLUMN);
		gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_fixed_width (column, 100);
		gtk_tree_view_append_column (GTK_TREE_VIEW(priv->header_view), column);
	}

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Subject"), renderer,
		"text", TNY_GTK_HEADER_LIST_MODEL_SUBJECT_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, TNY_GTK_HEADER_LIST_MODEL_SUBJECT_COLUMN);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 200);
	gtk_tree_view_append_column (GTK_TREE_VIEW(priv->header_view), column);


	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Date"), renderer,
		"text", TNY_GTK_HEADER_LIST_MODEL_DATE_RECEIVED_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, TNY_GTK_HEADER_LIST_MODEL_DATE_RECEIVED_COLUMN);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 100);
	gtk_tree_view_append_column (GTK_TREE_VIEW(priv->header_view), column);


	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Flags"), renderer,
		"text", TNY_GTK_HEADER_LIST_MODEL_FLAGS_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, TNY_GTK_HEADER_LIST_MODEL_FLAGS_COLUMN);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 100);
	gtk_tree_view_append_column (GTK_TREE_VIEW(priv->header_view), column);


	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Size"), renderer,
		"text", TNY_GTK_HEADER_LIST_MODEL_MESSAGE_SIZE_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, TNY_GTK_HEADER_LIST_MODEL_MESSAGE_SIZE_COLUMN);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 100);
	gtk_tree_view_append_column (GTK_TREE_VIEW(priv->header_view), column);

	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->mailbox_view));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	priv->mailbox_select = select;
	priv->mailbox_select_sid = g_signal_connect (G_OBJECT (select), "changed",
		G_CALLBACK (on_mailbox_view_tree_selection_changed), priv);


	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->header_view));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	g_signal_connect(G_OBJECT (priv->header_view), "row-activated", 
		G_CALLBACK (on_header_view_tree_row_activated), priv);

	g_signal_connect(G_OBJECT (priv->header_view), "key-press-event", 
		G_CALLBACK (on_header_view_key_press_event), self);

	g_signal_connect (G_OBJECT (select), "changed",
		G_CALLBACK (on_header_view_tree_selection_changed), self);


	g_signal_connect (G_OBJECT (priv->header_view), "button-press-event",
		G_CALLBACK (on_header_view_button_press_event), self);
	g_signal_connect (G_OBJECT (priv->header_view), "popup-menu",
		G_CALLBACK (header_view_popup_menu_event), self);

	g_signal_connect (G_OBJECT (priv->mailbox_view), "button-press-event",
		G_CALLBACK (on_mailbox_view_button_press_event), self);
	g_signal_connect (G_OBJECT (priv->mailbox_view), "popup-menu",
		G_CALLBACK (mailbox_view_popup_menu_event), self);

	gtk_widget_hide (priv->progress);

	return;
}

static void
tny_demoui_summary_view_finalize (GObject *object)
{
	TnyDemouiSummaryView *self = (TnyDemouiSummaryView *)object;	
	TnyDemouiSummaryViewPriv *priv = TNY_DEMOUI_SUMMARY_VIEW_GET_PRIVATE (self);

	if (priv->current_accounts)
		g_object_unref (G_OBJECT (priv->current_accounts));

	if (G_LIKELY (priv->account_store))
	{
		g_signal_handler_disconnect (G_OBJECT (priv->account_store),
			priv->accounts_reloaded_signal);

		g_object_unref (G_OBJECT (priv->account_store));
	}

	g_mutex_lock (priv->monitor_lock);
	if (priv->monitor)
	{
		tny_folder_monitor_stop (priv->monitor);
		g_object_unref (G_OBJECT (priv->monitor));
	}
	g_mutex_unlock (priv->monitor_lock);

	g_mutex_free (priv->monitor_lock);

	(*parent_class->finalize) (object);

	return;
}

static void
tny_summary_view_init (gpointer g, gpointer iface_data)
{
	return;
}

static void
tny_account_store_view_init (gpointer g, gpointer iface_data)
{
	TnyAccountStoreViewIface *klass = (TnyAccountStoreViewIface *)g;

	klass->set_account_store_func = tny_demoui_summary_view_set_account_store;

	return;
}

static void 
tny_demoui_summary_view_class_init (TnyDemouiSummaryViewClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_demoui_summary_view_finalize;

	g_type_class_add_private (object_class, sizeof (TnyDemouiSummaryViewPriv));

	return;
}

GType 
tny_demoui_summary_view_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyDemouiSummaryViewClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_demoui_summary_view_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyDemouiSummaryView),
		  0,      /* n_preallocs */
		  tny_demoui_summary_view_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_summary_view_info = 
		{
		  (GInterfaceInitFunc) tny_summary_view_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		static const GInterfaceInfo tny_account_store_view_info = 
		{
		  (GInterfaceInitFunc) tny_account_store_view_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (GTK_TYPE_VBOX,
			"TnyDemouiSummaryView",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_SUMMARY_VIEW, 
			&tny_summary_view_info);

		g_type_add_interface_static (type, TNY_TYPE_ACCOUNT_STORE_VIEW, 
			&tny_account_store_view_info);

	}

	return type;
}

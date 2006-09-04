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

#include <tny-platform-factory-iface.h>

#if PLATFORM==1
#include <tny-gnome-platform-factory.h>
#include <tny-gnome-password-dialog.h>
#include <tny-gnome-account-store.h>
#endif

#if PLATFORM==2
#include <tny-maemo-platform-factory.h>
#include <tny-maemo-password-dialog.h>
#include <tny-maemo-account-store.h>
#endif

#if PLATFORM==3
#include <tny-gpe-platform-factory.h>
#include <tny-gpe-password-dialog.h>
#include <tny-gpe-account-store.h>
#endif

#if PLATFORM==4
#include <tny-olpc-platform-factory.h>
#include <tny-olpc-password-dialog.h>
#include <tny-olpc-account-store.h>
#endif

#include <tny-account-store-iface.h>
#include <tny-account-iface.h>
#include <tny-store-account-iface.h>
#include <tny-transport-account-iface.h>
#include <tny-msg-view-iface.h>
#include <tny-msg-window-iface.h>
#include <tny-gtk-msg-window.h>
#include <tny-folder-iface.h>
#include <tny-gtk-account-tree-model.h>
#include <tny-header-iface.h>
#include <tny-gtk-header-list-model.h>
#include <tny-summary-view.h>
#include <tny-summary-view-iface.h>
#include <tny-account-store-view-iface.h>
#include <tny-list.h>

#define GO_ONLINE_TXT _("Go online")
#define GO_OFFLINE_TXT _("Go offline")

static GObjectClass *parent_class = NULL;


typedef struct _TnySummaryViewPriv TnySummaryViewPriv;

struct _TnySummaryViewPriv
{
	TnyAccountStoreIface *account_store;
	GtkTreeView *mailbox_view, *header_view;
	TnyMsgViewIface *msg_view;
	guint accounts_reloaded_signal;
	GtkWidget *status, *progress, *online_button;
	guint status_id;
	gulong mailbox_select_sid;
	GtkTreeSelection *mailbox_select;
	GtkTreeIter last_mailbox_correct_select;
	guint connchanged_signal, online_button_signal;
	TnyListIface *current_accounts;
};

#define TNY_SUMMARY_VIEW_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_SUMMARY_VIEW, TnySummaryViewPriv))



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
reload_accounts (TnySummaryViewPriv *priv)
{
	TnyAccountStoreIface *account_store = priv->account_store;
	GtkTreeModel *sortable;

	/* TnyAccountTreeModel is also a TnyListIface (it simply both the
	   TnyListIface and the GtkTreeModelIface interfaces interfaces) */

	GtkTreeModel *mailbox_model = tny_gtk_account_tree_model_new ();
	TnyListIface *accounts = TNY_LIST_IFACE (mailbox_model);

	/* Clear the header_view by giving it an empty model */
	if (G_UNLIKELY (!empty_model))
		empty_model = GTK_TREE_MODEL (gtk_list_store_new (1, G_TYPE_STRING));
	set_header_view_model (GTK_TREE_VIEW (priv->header_view), empty_model);
	
    	tny_msg_view_iface_clear (priv->msg_view);
    
	if (priv->current_accounts)
	{
		g_object_unref (G_OBJECT (priv->current_accounts));
		priv->current_accounts = NULL;
	}

	/* This method uses the TnyAccountTreeModel as a TnyListIface */
	tny_account_store_iface_get_accounts (account_store, accounts,
		TNY_ACCOUNT_STORE_IFACE_STORE_ACCOUNTS);


	/* Here we use the TnyAccountTreeModel as a GtkTreeModelIface */
	sortable = gtk_tree_model_sort_new_with_model (mailbox_model);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (sortable),
				TNY_GTK_ACCOUNT_TREE_MODEL_NAME_COLUMN, 
				GTK_SORT_ASCENDING);

	
	/* Set the model of the mailbox_view */
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->mailbox_view), 
		sortable);

	return;
}

static void
accounts_reloaded (TnyAccountStoreIface *store, gpointer user_data)
{
	TnySummaryViewPriv *priv = user_data;
	
	reload_accounts (priv);
	
	return;
}

static void 
online_button_toggled (GtkToggleButton *togglebutton, gpointer user_data)
{
	TnySummaryViewIface *self = user_data;
	TnySummaryViewPriv *priv = TNY_SUMMARY_VIEW_GET_PRIVATE (self);

	if (priv->account_store)
	{
		TnyDeviceIface *device = tny_account_store_iface_get_device (priv->account_store);

		if (gtk_toggle_button_get_active (togglebutton))
			tny_device_iface_force_online (device);
		else
			tny_device_iface_force_offline (device);
	}
}

static void
connection_changed (TnyDeviceIface *device, gboolean online, gpointer user_data)
{
	TnySummaryViewIface *self = user_data;
	TnySummaryViewPriv *priv = TNY_SUMMARY_VIEW_GET_PRIVATE (self);

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

	return;
}

static void
tny_summary_view_set_account_store (TnyAccountStoreViewIface *self, TnyAccountStoreIface *account_store)
{
	TnySummaryViewPriv *priv = TNY_SUMMARY_VIEW_GET_PRIVATE (self);
	TnyDeviceIface *device = tny_account_store_iface_get_device (account_store);

	if (G_UNLIKELY (priv->account_store))
	{ /* You typically set it once, so unlikely */

		TnyDeviceIface *odevice = tny_account_store_iface_get_device (priv->account_store);

		if (g_signal_handler_is_connected (G_OBJECT (odevice), priv->connchanged_signal))
		{
			g_signal_handler_disconnect (G_OBJECT (odevice), 
				priv->connchanged_signal);
		}

		g_signal_handler_disconnect (G_OBJECT (priv->account_store),
			priv->accounts_reloaded_signal);

		g_object_unref (G_OBJECT (priv->account_store));
	}

	g_object_ref (G_OBJECT (account_store));

	if (G_LIKELY (device))
	{
		priv->connchanged_signal =  g_signal_connect (
				G_OBJECT (device), "connection_changed",
				G_CALLBACK (connection_changed), self);	
	}

	priv->account_store = account_store;

	priv->accounts_reloaded_signal = g_signal_connect (
		G_OBJECT (account_store), "accounts_reloaded",
		G_CALLBACK (accounts_reloaded), priv);

	reload_accounts (priv);

	return;
}

static void
on_header_view_key_press_event (GtkTreeView *header_view, GdkEventKey *event, gpointer user_data)
{
	/* If the user presses the [Del] button on his keyboard */

	if (event->keyval == GDK_Delete)
	{
		//TnySummaryView *self  = user_data;
		//TnySummaryViewPriv *priv = TNY_SUMMARY_VIEW_GET_PRIVATE (self);
		GtkTreeSelection *selection = gtk_tree_view_get_selection (header_view);
		GtkTreeModel *model, *mymodel; //, *sortable;
		GtkTreeIter iter;

		if (G_LIKELY (gtk_tree_selection_get_selected (selection, &model, &iter)))
		{
			TnyHeaderIface *header;

			gtk_tree_model_get (model, &iter, 
				TNY_GTK_HEADER_LIST_MODEL_INSTANCE_COLUMN, 
				&header, -1);

			if (G_LIKELY (header))
			{
				GtkWidget *dialog = gtk_message_dialog_new (NULL, 
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, 
					_("This will remove the message with subject \"%s\""),
					tny_header_iface_get_subject (header));

				if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_YES)
				{
					TnyFolderIface *folder;
					//TnyMsgIface *msg;

					if (GTK_IS_TREE_MODEL_SORT (model))
					{
						mymodel = gtk_tree_model_sort_get_model 
							(GTK_TREE_MODEL_SORT (model));
					} else mymodel = model;

					folder = (TnyFolderIface*)tny_header_iface_get_folder (header);
					tny_folder_iface_remove_message (folder, header);

					tny_list_iface_remove (TNY_LIST_IFACE (mymodel), (GObject*)header);

					/* This demo-ui does not support hiding marked-as-deleted 
					   messages. A normal deletion will only *mark* a message
					   as deleted. That way undeletion is still possible.

					   You shouldn't *use* this demo-ui, so I'm doing destructive
					   irreversible deletes: I immediately expunge the folder! */

					tny_folder_iface_expunge (folder);
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
	TnySummaryView *self  = user_data;
	TnySummaryViewPriv *priv = TNY_SUMMARY_VIEW_GET_PRIVATE (self);
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (G_LIKELY (gtk_tree_selection_get_selected (selection, &model, &iter)))
	{
		TnyHeaderIface *header;

		gtk_tree_model_get (model, &iter, 
			TNY_GTK_HEADER_LIST_MODEL_INSTANCE_COLUMN, 
			&header, -1);
	    
		if (G_LIKELY (header))
		{
			TnyFolderIface *folder;
			TnyMsgIface *msg;

			folder = tny_header_iface_get_folder (header);
			if (G_LIKELY (folder))
			{
				msg = tny_folder_iface_get_message ((TnyFolderIface*)folder, header);
				if (G_LIKELY (msg))
				{
					tny_msg_view_iface_set_msg (priv->msg_view, TNY_MSG_IFACE (msg));
					g_object_unref (G_OBJECT (msg));
				} else { 
					tny_msg_view_iface_set_unavailable (priv->msg_view);
				}
			    	g_object_unref (G_OBJECT (folder));
			}

		    	g_object_unref (G_OBJECT (header));
		    
		} else {
			tny_msg_view_iface_set_unavailable (priv->msg_view);
		}
	}

	return;
}

static gboolean
cleanup_statusbar (gpointer data)
{
	TnySummaryViewPriv *priv = data;

	gtk_widget_hide (GTK_WIDGET (priv->progress));
	gtk_statusbar_pop (GTK_STATUSBAR (priv->status), priv->status_id);

	return FALSE;
}

static void
refresh_current_folder (TnyFolderIface *folder, gboolean cancelled, gpointer user_data)
{
	TnySummaryViewPriv *priv = user_data;

	if (!cancelled)
	{
		GtkTreeView *header_view = GTK_TREE_VIEW (priv->header_view);
		GtkTreeModel *sortable; // *oldsortable;
		GtkTreeModel *select_model;
		GtkTreeModel *model = tny_gtk_header_list_model_new ();
		gboolean refresh = FALSE;

#ifndef ASYNC_HEADERS
		refresh = TRUE;
#endif

		tny_gtk_header_list_model_set_folder (TNY_GTK_HEADER_LIST_MODEL (model), folder, refresh);
		sortable = gtk_tree_model_sort_new_with_model (GTK_TREE_MODEL (model));

		gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (sortable),
			TNY_GTK_HEADER_LIST_MODEL_DATE_RECEIVED_COLUMN,
			tny_gtk_header_list_model_received_date_sort_func, 
			NULL, NULL);

		gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (sortable),
			TNY_GTK_HEADER_LIST_MODEL_DATE_SENT_COLUMN,
			tny_gtk_header_list_model_sent_date_sort_func, 
			NULL, NULL);

		set_header_view_model (header_view, sortable); 		

		g_idle_add (cleanup_statusbar, priv);

		gtk_tree_selection_get_selected (priv->mailbox_select, &select_model, 
			&priv->last_mailbox_correct_select);

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
refresh_current_folder_status_update (TnyFolderIface *folder, const gchar *what, gint status, gpointer user_data)
{
	TnySummaryViewPriv *priv = user_data;

	gtk_progress_bar_pulse (GTK_PROGRESS_BAR (priv->progress));
	gtk_statusbar_pop (GTK_STATUSBAR (priv->status), priv->status_id);
	gtk_statusbar_push (GTK_STATUSBAR (priv->status), priv->status_id, what);

	return;
}

static void
on_mailbox_view_tree_selection_changed (GtkTreeSelection *selection, 
		gpointer user_data)
{
	TnySummaryViewPriv *priv = user_data;
	//GtkTreeView *header_view = priv->header_view;
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (G_LIKELY (gtk_tree_selection_get_selected (selection, &model, &iter)))
	{
		TnyFolderIface *folder;
		gint type;

		gtk_tree_model_get (model, &iter, 
			TNY_GTK_ACCOUNT_TREE_MODEL_TYPE_COLUMN, 
			&type, -1);

		if (type == -1) 
		{ 
			/* If an "account name"-row was clicked */
			g_signal_handler_block (G_OBJECT (priv->mailbox_select), priv->mailbox_select_sid);
			gtk_tree_selection_select_iter (priv->mailbox_select, &priv->last_mailbox_correct_select);
			g_signal_handler_unblock (G_OBJECT (priv->mailbox_select), priv->mailbox_select_sid);
			return; 
		}

		gtk_tree_model_get (model, &iter, 
			TNY_GTK_ACCOUNT_TREE_MODEL_INSTANCE_COLUMN, 
			&folder, -1);

		gtk_widget_show (GTK_WIDGET (priv->progress));
		gtk_widget_set_sensitive (GTK_WIDGET (priv->header_view), FALSE);
		
#ifdef ASYNC_HEADERS

		tny_folder_iface_refresh_async (folder, 
			refresh_current_folder, 
			refresh_current_folder_status_update, user_data);
#else
		refresh_current_folder (folder, FALSE, user_data);
#endif
	    
	    	g_object_unref (G_OBJECT (folder));
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
		TnyHeaderIface *header;
		TnyMsgWindowIface *msgwin;

		gtk_tree_model_get (model, &iter, 
			TNY_GTK_HEADER_LIST_MODEL_INSTANCE_COLUMN, 
			&header, -1);
		
		if (G_LIKELY (header))
		{
			TnyFolderIface *folder;
			TnyMsgIface *msg;
			TnyPlatformFactoryIface *platfact;


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

		    
			folder = tny_header_iface_get_folder (TNY_HEADER_IFACE (header));

			if (G_LIKELY (folder))
			{
				msg = tny_folder_iface_get_message (TNY_FOLDER_IFACE (folder), header);
				if (G_LIKELY (msg))
				{
					msgwin = TNY_MSG_WINDOW_IFACE (tny_gtk_msg_window_new (
						tny_platform_factory_iface_new_msg_view (platfact)));

					tny_msg_view_iface_set_msg (TNY_MSG_VIEW_IFACE (msgwin), 
						TNY_MSG_IFACE (msg));
					g_object_unref (G_OBJECT (msg));
				    
					gtk_widget_show (GTK_WIDGET (msgwin));
				} else {
					msgwin = TNY_MSG_WINDOW_IFACE (tny_gtk_msg_window_new (
						tny_platform_factory_iface_new_msg_view (platfact)));

					tny_msg_view_iface_set_unavailable (TNY_MSG_VIEW_IFACE (msgwin));
			
					gtk_widget_show (GTK_WIDGET (msgwin));
				}
			    	g_object_unref (G_OBJECT (folder));
			}
			g_object_unref (G_OBJECT (header));
		}
	}
}

/**
 * tny_summary_view_new:
 * 
 *
 * Return value: A new #TnySummaryViewIface instance implemented for Gtk+
 **/
TnySummaryViewIface*
tny_summary_view_new (void)
{
	TnySummaryView *self = g_object_new (TNY_TYPE_SUMMARY_VIEW, NULL);

	return TNY_SUMMARY_VIEW_IFACE (self);
}

static void
tny_summary_view_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnySummaryView *self = (TnySummaryView *)instance;
	TnySummaryViewPriv *priv = TNY_SUMMARY_VIEW_GET_PRIVATE (self);
	TnyPlatformFactoryIface *platfact;

	GtkVBox *vbox = GTK_VBOX (self);
	GtkWidget *mailbox_sw;
	GtkWidget *header_sw;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	//GtkTreeModel *mailbox_model;
	GtkTreeSelection *select;
	//gint t = 0, i = 0;
	GtkWidget *hpaned1;
	GtkWidget *vpaned1;
	
	/* TODO: Persist application UI status (of the panes) */

	priv->online_button = gtk_toggle_button_new ();
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
	
	priv->msg_view = tny_platform_factory_iface_new_msg_view (platfact);

	gtk_widget_show (GTK_WIDGET (priv->msg_view));	
	gtk_paned_pack2 (GTK_PANED (vpaned1), GTK_WIDGET (priv->msg_view), TRUE, TRUE);

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
	column = gtk_tree_view_column_new_with_attributes (_("Folder"), renderer,
			"text", TNY_GTK_ACCOUNT_TREE_MODEL_NAME_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, TNY_GTK_ACCOUNT_TREE_MODEL_NAME_COLUMN);

	gtk_tree_view_append_column (GTK_TREE_VIEW(priv->mailbox_view), column);

	if (G_UNLIKELY (FALSE))
	{ /* Not really working yet */
		renderer = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes (_("Unread"), renderer,
			"text", TNY_GTK_ACCOUNT_TREE_MODEL_UNREAD_COLUMN, NULL);
		gtk_tree_view_column_set_sort_column_id (column, TNY_GTK_ACCOUNT_TREE_MODEL_UNREAD_COLUMN);

		gtk_tree_view_append_column (GTK_TREE_VIEW(priv->mailbox_view), column);
	}

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


	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->mailbox_view));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	priv->mailbox_select = select;
	priv->mailbox_select_sid = g_signal_connect (G_OBJECT (select), "changed",
		G_CALLBACK (on_mailbox_view_tree_selection_changed), priv);


	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->header_view));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	g_signal_connect(G_OBJECT (priv->header_view), "row-activated", 
		G_CALLBACK (on_header_view_tree_row_activated), priv->header_view);

	g_signal_connect(G_OBJECT (priv->header_view), "key-press-event", 
		G_CALLBACK (on_header_view_key_press_event), self);

	g_signal_connect (G_OBJECT (select), "changed",
		G_CALLBACK (on_header_view_tree_selection_changed), self);


	gtk_widget_hide (priv->progress);

	return;
}

static void
tny_summary_view_finalize (GObject *object)
{
	TnySummaryView *self = (TnySummaryView *)object;	
	TnySummaryViewPriv *priv = TNY_SUMMARY_VIEW_GET_PRIVATE (self);

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
tny_summary_view_iface_init (gpointer g_iface, gpointer iface_data)
{
	return;
}

static void
tny_account_store_view_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyAccountStoreViewIfaceClass *klass = (TnyAccountStoreViewIfaceClass *)g_iface;

	klass->set_account_store_func = tny_summary_view_set_account_store;

	return;
}

static void 
tny_summary_view_class_init (TnySummaryViewClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_summary_view_finalize;

	g_type_class_add_private (object_class, sizeof (TnySummaryViewPriv));

	return;
}

GType 
tny_summary_view_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnySummaryViewClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_summary_view_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnySummaryView),
		  0,      /* n_preallocs */
		  tny_summary_view_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_summary_view_iface_info = 
		{
		  (GInterfaceInitFunc) tny_summary_view_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		static const GInterfaceInfo tny_account_store_view_iface_info = 
		{
		  (GInterfaceInitFunc) tny_account_store_view_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (GTK_TYPE_VBOX,
			"TnySummaryView",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_SUMMARY_VIEW_IFACE, 
			&tny_summary_view_iface_info);

		g_type_add_interface_static (type, TNY_TYPE_ACCOUNT_STORE_VIEW_IFACE, 
			&tny_account_store_view_iface_info);

	}

	return type;
}

using System;
using Gtk;

public partial class MainWindow: Gtk.Window
{	

	private Tny.Ui.MsgView msg_view;
	private Tny.Folder cur_folder = null;
	
	public MainWindow (): base (Gtk.WindowType.Toplevel)
	{
		Tny.Platform.Init.Initialize();
		Build ();
		PrepareUI ();
	}

#region UI stuff
	private void PrepareUI () 
	{
		this.msg_view = new Tny.Ui.GTK.MsgView();
		((Gtk.Widget) this.msg_view).Show ();
		this.msg_scrolledwindow.AddWithViewport ((Gtk.Widget) this.msg_view);

		Gtk.TreeViewColumn FolderColumn = new Gtk.TreeViewColumn ();
		Gtk.CellRendererText FolderNameCell = new Gtk.CellRendererText ();
		FolderColumn.PackStart (FolderNameCell, true);
		FolderColumn.AddAttribute (FolderNameCell, "text", (int) Tny.Ui.GTK.FolderStoreTreeModelColumn.NameColumn);		
		FolderColumn.Title = "Folder";
		this.folders_treeview.AppendColumn (FolderColumn);

		Gtk.TreeViewColumn FromColumn = new Gtk.TreeViewColumn ();
		Gtk.CellRendererText FromNameCell = new Gtk.CellRendererText ();
		FromColumn.PackStart (FromNameCell, true);
		FromColumn.AddAttribute (FromNameCell, "text", (int) Tny.Ui.GTK.HeaderListModelColumn.FromColumn);		
		FromColumn.Title = "From";
		this.headers_treeview.AppendColumn (FromColumn);
	
		Gtk.TreeViewColumn SubjectColumn = new Gtk.TreeViewColumn ();
		Gtk.CellRendererText SubjectNameCell = new Gtk.CellRendererText ();
		SubjectColumn.PackStart (SubjectNameCell, true);
		SubjectColumn.AddAttribute (SubjectNameCell, "text", (int) Tny.Ui.GTK.HeaderListModelColumn.SubjectColumn);		
		SubjectColumn.Title = "Subject";
		this.headers_treeview.AppendColumn (SubjectColumn);

		this.headers_treeview.Selection.Changed += OnMailSelected;
		this.folders_treeview.Selection.Changed += OnFolderChanged;
	}
#endregion	

	protected void OnDeleteEvent (object sender, DeleteEventArgs a)
	{
		Application.Quit ();
		a.RetVal = true;
	}

	private void GetHeadersCallback (Tny.Folder folder, bool cancel, Tny.List model, IntPtr user_data)
	{
		if (model != null && !cancel)
			this.headers_treeview.Model = (Gtk.TreeModel) model;
	}
	
	private void StatusCallback (GLib.Object sender, Tny.Status status)
	{
		this.progressbar.Fraction = status.Fraction;
	}

	private void GetMsgCallBack (Tny.Folder folder, bool cancel, Tny.Msg msg, IntPtr user_data)
	{
		if (msg != null && !cancel)
			this.msg_view.Msg = msg;
	}
	
			
	private void OnMailSelected (object o, EventArgs args)
	{
		Gtk.TreeModel model;
		Gtk.TreeIter iter;
		Gtk.TreeSelection selection = (Gtk.TreeSelection) o;

	 	if (selection.GetSelected (out model, out iter)) {
	 		Tny.Ui.GTK.HeaderListModel headers_model = (Tny.Ui.GTK.HeaderListModel) model;	
 			Tny.Header header = headers_model.GetHeader (iter);

			if (header != null) {
				Console.WriteLine ("Message selected: " + header.From);				
				if (this.cur_folder != null)
					this.cur_folder.GetMsgAsync (header, GetMsgCallBack, StatusCallback);
			}
		}
	}
	
	private void OnFolderChanged (object o, EventArgs args)
	{
		Gtk.TreeModel model;
		Gtk.TreeIter iter;
		Gtk.TreeSelection selection = (Gtk.TreeSelection) o;

	 	if (selection.GetSelected (out model, out iter)) {
	 		Tny.Ui.GTK.FolderStoreTreeModel folders_model = (Tny.Ui.GTK.FolderStoreTreeModel) model;	
 			Tny.Folder folder = folders_model.GetFolder (iter);

			if (folder != null) {
				this.cur_folder = folder;
				Console.WriteLine ("Folder selected: " + folder.Name);		
				Tny.Ui.GTK.HeaderListModel headers_model = new Tny.Ui.GTK.HeaderListModel();
				folder.GetHeadersAsync (headers_model, true, GetHeadersCallback, StatusCallback);
			}
		}
	}

	
	private void OnConnectButtonClicked (object sender, System.EventArgs e)
	{
		Tny.AccountStore store = new Tny.Platform.GnomeAccountStore ();
		Tny.Ui.GTK.FolderStoreTreeModel model = new Tny.Ui.GTK.FolderStoreTreeModel (new Tny.FolderStoreQuery ());
		store.GetAccounts (model, Tny.GetAccountsRequestType.StoreAccounts);		
		this.folders_treeview.Model = model;
	}
}

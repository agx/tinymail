import gtk
import gnome
import tinymail
import tinymail.ui
import tinymail.uigtk
import tinymail.camel
import gc

from pyplatformfactory import PyPlatformFactory

def on_refresh_folder (folder, cancelled, headerstree):
	listm = tinymail.uigtk.GtkHeaderListModel ()
	listm.set_folder (folder, False)
	headerstree.set_model (listm)
	progressbar.hide ()

	# I know. But this does significantly speed
	# up the garbage collecting. Only use it for
	# targets that have few memory resources

	gc.collect()


def on_status (folder, what, status, oftotal, headerstree) :
	progressbar.set_fraction (status / oftotal)

def on_headerstree_selected (treeselection, msgview) :
	model, iter = treeselection.get_selected ()
	if iter:
		header = model.get_value (iter, tinymail.uigtk.GTK_HEADER_LIST_MODEL_INSTANCE_COLUMN)
		if header:
			folder = header.get_folder ()
			msg = folder.get_msg (header)
			msgview.set_msg (msg)

def on_folderstree_selected (treeselection, headerstree) :
	model, iter = treeselection.get_selected ()
	folder = model.get_value(iter, tinymail.uigtk.GTK_FOLDER_STORE_TREE_MODEL_INSTANCE_COLUMN)
	if folder:
		progressbar.show ()
		folder.refresh_async (on_refresh_folder, on_status, headerstree)

props = { gnome.PARAM_APP_DATADIR : "/usr/share" }
pr = gnome.program_init ("E-Mail", "1.0", properties=props)
builder = gtk.Builder()
builder.add_from_file("tinymail-python-test.ui")
widget = builder.get_object("window")
progressbar = builder.get_object ("progressbar")
progressbar.hide ()
folderstree = builder.get_object ("folderstree")
headerstree = builder.get_object ("headerstree")
vpaned = builder.get_object ("vpaned")
renderer = gtk.CellRendererText ();
column = gtk.TreeViewColumn ("Folder", renderer, text=0)
column.set_fixed_width (100)
column.set_sizing (gtk.TREE_VIEW_COLUMN_FIXED)
folderstree.append_column (column)
renderer = gtk.CellRendererText ();
column = gtk.TreeViewColumn ("From", renderer, text=0)
column.set_fixed_width (100)
column.set_sizing (gtk.TREE_VIEW_COLUMN_FIXED)
headerstree.append_column (column)
renderer = gtk.CellRendererText ();
column = gtk.TreeViewColumn ("Subject", renderer, text=2)
column.set_fixed_width (200)
column.set_sizing (gtk.TREE_VIEW_COLUMN_FIXED)
headerstree.append_column (column)
renderer = gtk.CellRendererText ();
column = gtk.TreeViewColumn ("Received", renderer, text=7)
column.set_fixed_width (100)
column.set_sizing (gtk.TREE_VIEW_COLUMN_FIXED)
headerstree.append_column (column)
platfact = PyPlatformFactory()
msgview = platfact.new_msg_view ()
msgview.show ()
vpaned.pack2 (msgview, True, True)
account_store = platfact.new_account_store ()
device = account_store.get_device ()
device.force_online ()
query = tinymail.FolderStoreQuery ()
query.add_item ("", tinymail.FOLDER_STORE_QUERY_OPTION_SUBSCRIBED)
accounts = tinymail.uigtk.GtkFolderStoreTreeModel (query)
account_store.get_accounts (accounts, tinymail.ACCOUNT_STORE_STORE_ACCOUNTS)
folderstree.set_model (accounts)
folderstree.get_selection().connect("changed", on_folderstree_selected, headerstree)
headerstree.get_selection().connect("changed", on_headerstree_selected, msgview)
builder.connect_signals(self)
gtk.main()

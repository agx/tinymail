import gtk
import gtk.glade
import gnome
import tinymail
import tinymail.ui
import tinymail.uigtk
import tinymail.platform

def on_folderstree_selected (treeselection, headerstree) :
	model, iter = treeselection.get_selected ()
	folder = model.get_value(iter, 3)
	if folder:
		print folder.get_name()
		listm = tinymail.uigtk.MsgHeaderListModel ()
		listm.set_folder (folder, False)
		print listm.length()
		headerstree.set_model (listm)

props = { gnome.PARAM_APP_DATADIR : "/usr/share" }
pr = gnome.program_init ("E-Mail", "1.0", properties=props)
xml = gtk.glade.XML ("tinymail-python-test.glade", domain="email")
widget = xml.get_widget ("window")
folderstree = xml.get_widget ("folderstree")
headerstree = xml.get_widget ("headerstree")
hpaned = xml.get_widget ("hpaned")
vpaned = xml.get_widget ("vpaned")
renderer = gtk.CellRendererText ();
column = gtk.TreeViewColumn ("Folder", renderer, text=0)
folderstree.append_column (column)
renderer = gtk.CellRendererText ();
column = gtk.TreeViewColumn ("From", renderer, text=0)
headerstree.append_column (column)
renderer = gtk.CellRendererText ();
column = gtk.TreeViewColumn ("Subject", renderer, text=2)
headerstree.append_column (column)
platfact = tinymail.platform.tny_platform_factory_get_instance ()
account_store = platfact.new_account_store ()
accounts = tinymail.uigtk.AccountTreeModel ()
account_store.get_accounts (accounts, tinymail.ACCOUNT_STORE_IFACE_STORE_ACCOUNTS)
folderstree.set_model (accounts)
folderstree.get_selection().connect("changed", on_folderstree_selected, headerstree)
xml.signal_connect("on_window_delete_event", gtk.main_quit)
gtk.main()

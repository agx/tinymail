import gtk
import gtk.glade
import gnome
import gnome.ui
import tinymail
import tinymail.ui
import tinymail.uigtk
import tinymail.platform

from tinymail.ui import PlatformFactoryIface

props = { gnome.PARAM_APP_DATADIR : "/usr/share" }
pr = gnome.program_init ("E-Mail", "1.0", properties=props)
xml = gtk.glade.XML ("tinymail-python-test.glade", domain="email")

widget = xml.get_widget ("window")
foldertree = xml.get_widget ("folderstree")
headerstree = xml.get_widget ("headerstree")
hbox = xml.get_widget ("hbox")
vbox = xml.get_widget ("vbox")

platfact = tinymail.platform.tny_platform_factory_get_instance ()
account_store = platfact.new_account_store ()
accounts = tinymail.uigtk.AccountTreeModel ()
account_store.get_accounts (accounts, 1)

folderstree.set_model (account_store)

gtk.main()

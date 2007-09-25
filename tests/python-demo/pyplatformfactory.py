#! /usr/bin/env python 
import gobject

from tinymail.ui import PlatformFactory
from xmlaccountstore import XmlAccountStore
from tinymail.uigtk import GtkMsgView
from tinymail.camel import CamelMsg
from tinymail.camel import CamelMimePart
from tinymail.uigtk import GtkPasswordDialog

class PyPlatformFactory(gobject.GObject, PlatformFactory):
	"""
	An implementation of the Tinymail platform facory interface
	that depends on Gtk, Network manager, Dbus, and Camel. Uses
	an XML file in the users home directory to store accounts. 
	"""
	def __init__(self):
		gobject.GObject.__init__(self)

	def do_new_account_store_func(self):
		return XmlAccountStore('tinymailacc.xml')

	def do_new_msg_view_func(self):
		return GtkMsgView()

	def do_new_msg_func(self):
		return CamelMsg()

	def do_new_mime_part_func(self):
		return CamelMimePart()

	def do_new_password_getter_func(self):
		return GtkPasswordDialog()

gobject.type_register(PyPlatformFactory)

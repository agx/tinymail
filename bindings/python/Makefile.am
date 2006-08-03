INCLUDES = $(PYTHON_INCLUDES) $(LIBTINYMAIL_CFLAGS) 		\
	$(PYGTK_CFLAGS)						\
	$(LIBTINYMAILUI_GTK_CFLAGS) $(TINYMAIL_PYTHON_CFLAGS) 	\
	-I$(top_srcdir)/libtinymail				\
	-I$(top_srcdir)/libtinymailui				\
	-I$(top_srcdir)/libtinymailui-gtk			\
	-I$(top_srcdir)/libtinymail-camel			\
	-I$(top_srcdir)/$(PLATFORMDIR)

common_ldflags = -module -avoid-version

pthdir = $(pythondir)
pth_DATA = tinymail.pth

tinymaildir = $(pythondir)/tinymail-$(API_VERSION)/tinymail
ptinymaildir = $(pythondir)/tinymail-$(API_VERSION)/tinymail
tinymail_LTLIBRARIES = _tinymail.la ui.la uigtk.la platform.la
ptinymail_PYTHON = __init__.py

_tinymail_la_LDFLAGS = $(common_ldflags) -export-symbols-regex init_tinymail
_tinymail_la_LIBADD = $(LIBTINYMAIL_LIBS) $(TINYMAIL_PYTHON_LIBS) 	\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la 
_tinymail_la_SOURCES = tinymailmodule.c
nodist__tinymail_la_SOURCES = tinymail-glue.c

ui_la_LDFLAGS = $(common_ldflags) -export-symbols-regex initui
ui_la_LIBADD = $(LIBTINYMAILUI_LIBS) $(TINYMAIL_PYTHON_LIBS) 		\
	$(LIBTINYMAIL_LIBS)						\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la	\
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la
ui_la_SOURCES = tinymailuimodule.c
nodist_ui_la_SOURCES = tinymailui-glue.c
 
uigtk_la_LDFLAGS = $(common_ldflags) -export-symbols-regex inituigtk
uigtk_la_LIBADD = $(LIBTINYMAILUI_GTK_LIBS) $(LIBTINYMAILUI_LIBS) 	\
	$(LIBTINYMAIL_LIBS) $(TINYMAIL_PYTHON_LIBS) 			\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la	\
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la	\
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la
uigtk_la_SOURCES = tinymailui-gtkmodule.c
nodist_uigtk_la_SOURCES = tinymailui-gtk-glue.c

platform_la_LDFLAGS = $(common_ldflags) -export-symbols-regex initplatform
platform_la_LIBADD = $(LIBTINYMAILUI_LIBS) $(LIBTINYMAIL_LIBS) \
	$(TINYMAIL_PYTHON_LIBS) 					\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la	\
	$(top_builddir)/$(PLATFORMDIR)/$(PLATFORMDIR)-$(API_VERSION).la	\
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la
platform_la_SOURCES = tinymail-platformmodule.c
nodist_platform_la_SOURCES = tinymail-platform-glue.c

CLEANFILES = tinymail-glue.c tinymailui-glue.c tinymailui-gtk-glue.c 	\
	tinymail-platform-glue.c platform_include.h tinymail.defs	\
	tinymailui.defs tinymailui-gtk.defs tinymail-platform.defs

EXTRA_DIST = 				\
	filter.py.in			\
	filter.py			\
	tinymail.defs.extra 		\
	tinymail.override 		\
	tinymailui.defs.extra		\
	tinymailui.override		\
	tinymailui-gtk.override		\
	tinymailui-gtk.defs.extra 	\
	tinymail-platform.defs.extra	\
	tinymail-platform.override

# To update:
# ls DIR/*h | grep -v \\-priv | sed s/^/\\t\$\(top_srcdir\)\\//g | sed s/$/\\t\\\\/g

tinymail.defs: tinymail.defs.extra $(top_srcdir)/bindings/python/filter.py
	$(PYTHON) $(PYGTK_CODEGENDIR)/h2def.py \
	$(top_srcdir)/libtinymail/tny-account-iface.h		\
	$(top_srcdir)/libtinymail/tny-account-store-iface.h	\
	$(top_srcdir)/libtinymail/tny-device-iface.h		\
	$(top_srcdir)/libtinymail/tny-fs-stream.h		\
	$(top_srcdir)/libtinymail/tny-iterator-iface.h		\
	$(top_srcdir)/libtinymail/tny-list.h			\
	$(top_srcdir)/libtinymail/tny-list-iface.h		\
	$(top_srcdir)/libtinymail/tny-msg-folder-iface.h	\
	$(top_srcdir)/libtinymail/tny-msg-header-iface.h	\
	$(top_srcdir)/libtinymail/tny-msg-iface.h		\
	$(top_srcdir)/libtinymail/tny-msg-mime-part-iface.h	\
	$(top_srcdir)/libtinymail/tny-shared.h			\
	$(top_srcdir)/libtinymail/tny-store-account-iface.h 	\
	$(top_srcdir)/libtinymail/tny-stream-iface.h 		\
	$(top_srcdir)/libtinymail/tny-transport-account-iface.h | \
	$(PYTHON) $(top_srcdir)/bindings/python/filter.py --extra=tinymail.defs.extra > $@

tinymailui.defs: tinymailui.defs.extra $(top_srcdir)/bindings/python/filter.py
	$(PYTHON) $(PYGTK_CODEGENDIR)/h2def.py 			   \
	$(top_srcdir)/libtinymailui/tny-account-store-view-iface.h \
	$(top_srcdir)/libtinymailui/tny-msg-header-view-iface.h	   \
	$(top_srcdir)/libtinymailui/tny-msg-view-iface.h	   \
	$(top_srcdir)/libtinymailui/tny-msg-window-iface.h	   \
	$(top_srcdir)/libtinymailui/tny-platform-factory-iface.h   \
	$(top_srcdir)/libtinymailui/tny-save-strategy-iface.h	   \
	$(top_srcdir)/libtinymailui/tny-summary-view-iface.h |	   \
	$(PYTHON) $(top_srcdir)/bindings/python/filter.py --extra=tinymailui.defs.extra > $@


tinymailui-gtk.defs: tinymailui-gtk.defs.extra $(top_srcdir)/bindings/python/filter.py
	$(PYTHON) $(PYGTK_CODEGENDIR)/h2def.py				\
	$(top_srcdir)/libtinymailui-gtk/tny-account-tree-model.h	\
	$(top_srcdir)/libtinymailui-gtk/tny-attach-list-model.h		\
	$(top_srcdir)/libtinymailui-gtk/tny-msg-header-list-model.h	\
	$(top_srcdir)/libtinymailui-gtk/tny-msg-header-view.h		\
	$(top_srcdir)/libtinymailui-gtk/tny-msg-view.h			\
	$(top_srcdir)/libtinymailui-gtk/tny-msg-window.h		\
	$(top_srcdir)/libtinymailui-gtk/tny-save-strategy.h		\
	$(top_srcdir)/libtinymailui-gtk/tny-text-buffer-stream.h |	\
	$(PYTHON) $(top_srcdir)/bindings/python/filter.py --extra=tinymailui-gtk.defs.extra > $@

tinymail-platform.defs : tinymail-platform.defs.extra $(top_srcdir)/bindings/python/filter.py
	$(PYTHON) $(PYGTK_CODEGENDIR)/h2def.py				  \
	`ls $(top_srcdir)/$(PLATFORMDIR)/*h | grep -v \\\-priv | xargs` | \
	$(PYTHON) $(top_srcdir)/bindings/python/filter.py --extra=tinymail-platform.defs.extra > $@

tinymail-glue.c: tinymail.defs tinymail.override
	$(PYGTK_CODEGEN) --prefix pytinymail		\
	--override tinymail.override			\
	tinymail.defs > $@

tinymailui-glue.c: tinymailui.defs tinymailui.override
	$(PYGTK_CODEGEN) --prefix pyui			\
	--register tinymail.defs			\
	--override tinymailui.override			\
	tinymailui.defs > $@

tinymailui-gtk-glue.c: tinymailui-gtk.defs tinymailui-gtk.override
	$(PYGTK_CODEGEN) --prefix pyuigtk		\
	--register $(PYGTK_DEFSDIR)/gdk-types.defs      \
	--register $(PYGTK_DEFSDIR)/gtk-types.defs      \
	--register tinymail.defs			\
	--register tinymailui.defs			\
	--override tinymailui-gtk.override		\
	tinymailui-gtk.defs > $@

platform_include.h:
	ls $(top_srcdir)/$(PLATFORMDIR)/*h | grep -v \\-priv | sed 's/^/\#include\ \"/g' | sed 's/\.h/\.h\"/g' > $@

tinymail-platform-glue.c: tinymail-platform.defs tinymail-platform.override platform_include.h
	$(PYGTK_CODEGEN) --prefix pyplatform		\
	--register tinymailui.defs			\
	--register tinymail.defs			\
	--override tinymail-platform.override		\
	tinymail-platform.defs > $@


INCLUDES = $(PYTHON_INCLUDES) $(LIBTINYMAIL_CFLAGS) $(PYGTK_CFLAGS) \
	$(LIBTINYMAILUI_GTK_CFLAGS) $(TINYMAIL_PYTHON_CFLAGS) \
	-I$(top_srcdir)/libtinymail -I$(top_srcdir)/libtinymailui \
	-I$(top_srcdir)/libtinymailui-gtk -I$(top_srcdir)/libtinymail-camel \
	-I$(top_srcdir)/$(PLATFORMDIR)

common_ldflags = -module -avoid-version

pthdir = $(pythondir)
pth_DATA = tinymail.pth

tinymaildir = $(pythondir)/tinymail-$(API_VERSION)/tinymail
ptinymaildir = $(pythondir)/tinymail-$(API_VERSION)/tinymail

tinymail_LTLIBRARIES = _tinymail.la ui.la 
if BUILD_UIGTK
tinymail_LTLIBRARIES += uigtk.la 
endif
tinymail_LTLIBRARIES += platform.la 

ptinymail_PYTHON = __init__.py

_tinymail_la_LDFLAGS = $(common_ldflags) -export-symbols-regex init_tinymail
_tinymail_la_LIBADD = $(LIBTINYMAIL_LIBS) $(TINYMAIL_PYTHON_LIBS) \
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la 
_tinymail_la_SOURCES = tinymailmodule.c
nodist__tinymail_la_SOURCES = tinymail-glue.c

ui_la_LDFLAGS = $(common_ldflags) -export-symbols-regex initui
ui_la_LIBADD = $(LIBTINYMAILUI_LIBS) $(TINYMAIL_PYTHON_LIBS) \
	$(LIBTINYMAIL_LIBS) \
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la
ui_la_SOURCES = tinymailuimodule.c
nodist_ui_la_SOURCES = tinymailui-glue.c
 
uigtk_la_LDFLAGS = $(common_ldflags) -export-symbols-regex inituigtk
uigtk_la_LIBADD = $(LIBTINYMAILUI_GTK_LIBS) $(LIBTINYMAILUI_LIBS) \
	$(LIBTINYMAIL_LIBS) $(TINYMAIL_PYTHON_LIBS) \
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la
uigtk_la_SOURCES = tinymailui-gtkmodule.c
nodist_uigtk_la_SOURCES = tinymailui-gtk-glue.c

platform_la_LDFLAGS = $(common_ldflags) -export-symbols-regex initplatform
platform_la_LIBADD = $(LIBTINYMAILUI_LIBS) $(LIBTINYMAIL_LIBS) \
	$(TINYMAIL_PYTHON_LIBS) \
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la \
	$(top_builddir)/$(PLATFORMDIR)/$(PLATFORMDIR)-$(API_VERSION).la \
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la
platform_la_SOURCES = tinymail-platformmodule.c
nodist_platform_la_SOURCES = tinymail-platform-glue.c

CLEANFILES = tinymail-glue.c tinymailui-glue.c tinymailui-gtk-glue.c \
	tinymail-platform-glue.c platform_include.h tinymail.defs \
	tinymailui.defs tinymailui-gtk.defs tinymail-platform.defs

EXTRA_DIST = \
	filter.py \
	tinymail.defs.extra \
	tinymail.override \
	tinymailui.defs.extra \
	tinymailui.override \
	tinymailui-gtk.override \
	tinymailui-gtk.defs.extra \
	tinymail-platform.defs.extra \
	tinymail-platform.override

# To update:
# ls DIR/*h | grep -v \\-priv | sed s/^/\\t\$\(top_srcdir\)\\//g | sed s/$/\\t\\\\/g

tinymail_h_files = \
	$(top_srcdir)/libtinymail/tny-account.h \
	$(top_srcdir)/libtinymail/tny-account-store.h \
	$(top_srcdir)/libtinymail/tny-device.h \
	$(top_srcdir)/libtinymail/tny-folder-store-query.h \
	$(top_srcdir)/libtinymail/tny-folder-store.h \
	$(top_srcdir)/libtinymail/tny-folder.h \
	$(top_srcdir)/libtinymail/tny-fs-stream.h \
	$(top_srcdir)/libtinymail/tny-header.h \
	$(top_srcdir)/libtinymail/tny-iterator.h \
	$(top_srcdir)/libtinymail/tny-list.h \
	$(top_srcdir)/libtinymail/tny-mime-part.h \
	$(top_srcdir)/libtinymail/tny-msg.h \
	$(top_srcdir)/libtinymail/tny-shared.h \
	$(top_srcdir)/libtinymail/tny-simple-list.h \
	$(top_srcdir)/libtinymail/tny-store-account.h \
	$(top_srcdir)/libtinymail/tny-stream.h \
	$(top_srcdir)/libtinymail/tny-transport-account.h \
	$(top_srcdir)/libtinymail/tny-msg-remove-strategy.h \
	$(top_srcdir)/libtinymail/tny-send-queue.h

tinymail.defs: $(tinymail_h_files) tinymail.defs.extra $(top_srcdir)/bindings/python/filter.py 
	$(PYTHON) $(PYGTK_CODEGENDIR)/h2def.py $(tinymail_h_files) | \
	$(PYTHON) $(top_srcdir)/bindings/python/filter.py \
	--extra=$(top_srcdir)/bindings/python/tinymail.defs.extra \
	--topsrcdir=$(top_srcdir) > $@

#	$(PYTHON) $(top_srcdir)/bindings/python/scanvirtuals.py
#	$(tinymail_h_files) >> $@

tinymailui_h_files = \
	$(top_srcdir)/libtinymailui/tny-account-store-view.h \
	$(top_srcdir)/libtinymailui/tny-header-view.h \
	$(top_srcdir)/libtinymailui/tny-mime-part-save-strategy.h \
	$(top_srcdir)/libtinymailui/tny-mime-part-saver.h \
	$(top_srcdir)/libtinymailui/tny-mime-part-view.h \
	$(top_srcdir)/libtinymailui/tny-msg-view.h \
	$(top_srcdir)/libtinymailui/tny-msg-window.h \
	$(top_srcdir)/libtinymailui/tny-platform-factory.h \
	$(top_srcdir)/libtinymailui/tny-summary-view.h

tinymailui.defs: $(tinymailui_h_files) tinymailui.defs.extra $(top_srcdir)/bindings/python/filter.py
	$(PYTHON) $(PYGTK_CODEGENDIR)/h2def.py $(tinymailui_h_files) | \
	$(PYTHON) $(top_srcdir)/bindings/python/filter.py \
	--extra=$(top_srcdir)/bindings/python/tinymailui.defs.extra \
	--topsrcdir=$(top_srcdir) > $@

#	$(PYTHON) $(top_srcdir)/bindings/python/scanvirtuals.py
#	$(tinymailui_h_files) >> $@


tinymailui_gtk_h_files = \
	$(top_srcdir)/libtinymailui-gtk/tny-gtk-account-list-model.h \
	$(top_srcdir)/libtinymailui-gtk/tny-gtk-account-tree-model.h \
	$(top_srcdir)/libtinymailui-gtk/tny-gtk-attach-list-model.h \
	$(top_srcdir)/libtinymailui-gtk/tny-gtk-attachment-mime-part-view.h \
	$(top_srcdir)/libtinymailui-gtk/tny-gtk-folder-tree-model.h \
	$(top_srcdir)/libtinymailui-gtk/tny-gtk-header-list-model.h \
	$(top_srcdir)/libtinymailui-gtk/tny-gtk-header-view.h \
	$(top_srcdir)/libtinymailui-gtk/tny-gtk-mime-part-save-strategy.h \
	$(top_srcdir)/libtinymailui-gtk/tny-gtk-msg-view.h \
	$(top_srcdir)/libtinymailui-gtk/tny-gtk-msg-window.h \
	$(top_srcdir)/libtinymailui-gtk/tny-gtk-text-buffer-stream.h \
	$(top_srcdir)/libtinymailui-gtk/tny-gtk-text-mime-part-view.h

tinymailui-gtk.defs: $(tinymailui_gtk_h_files) tinymailui-gtk.defs.extra $(top_srcdir)/bindings/python/filter.py
	$(PYTHON) $(PYGTK_CODEGENDIR)/h2def.py $(tinymailui_gtk_h_files) | \
	$(PYTHON) $(top_srcdir)/bindings/python/filter.py 		   \
	--extra=$(top_srcdir)/bindings/python/tinymailui-gtk.defs.extra    \
	--topsrcdir=$(top_srcdir) > $@

#	$(PYTHON) $(top_srcdir)/bindings/python/scanvirtuals.py
#	$(tinymailui_gtk_h_files) >> $@

# tinymail_platform_h_files = `ls $(top_srcdir)/$(PLATFORMDIR)/*h | grep -v \\\-priv | xargs`

tinymail-platform.defs : $(tinymail_platform_h_files) tinymail-platform.defs.extra $(top_srcdir)/bindings/python/filter.py
	$(PYTHON) $(PYGTK_CODEGENDIR)/h2def.py \
	`ls $(top_srcdir)/$(PLATFORMDIR)/*h | grep -v \\\-priv | xargs`	| \
	$(PYTHON) $(top_srcdir)/bindings/python/filter.py \
	--extra=$(top_srcdir)/bindings/python/tinymail-platform.defs.extra \
	--topsrcdir=$(top_srcdir) > $@
	$(PYTHON) $(top_srcdir)/bindings/python/scanvirtuals.py
	$(tinymail_platform_h_files) >> $@

tinymail-glue.c: tinymail.defs tinymail.override
	$(PYGTK_CODEGEN) --prefix pytinymail \
	--override $(top_srcdir)/bindings/python/tinymail.override \
	tinymail.defs > $@

tinymailui-glue.c: tinymailui.defs tinymailui.override
	$(PYGTK_CODEGEN) --prefix pyui \
	--register tinymail.defs \
	--override $(top_srcdir)/bindings/python/tinymailui.override \
	tinymailui.defs > $@

tinymailui-gtk-glue.c: tinymail.defs $(PYGTK_DEFSDIR)/gdk-types.defs $(PYGTK_DEFSDIR)/gtk-types.defs tinymailui-gtk.defs tinymailui-gtk.override
	$(PYGTK_CODEGEN) --prefix pyuigtk --register $(PYGTK_DEFSDIR)/gdk-types.defs \
	--register $(PYGTK_DEFSDIR)/gtk-types.defs --register tinymail.defs \
	--register tinymailui.defs --override $(top_srcdir)/bindings/python/tinymailui-gtk.override \
	tinymailui-gtk.defs > $@

platform_include.h:
	ls $(top_srcdir)/$(PLATFORMDIR)/*h | grep -v \\-priv | sed 's/^/\#include\ \"/g' | sed 's/\.h/\.h\"/g' > $@

tinymail-platform-glue.c: tinymail-platform.defs tinymail-platform.override platform_include.h
	$(PYGTK_CODEGEN) --prefix pyplatform \
	--register tinymailui.defs \
	--register tinymail.defs \
	--override $(top_srcdir)/bindings/python/tinymail-platform.override \
	tinymail-platform.defs > $@


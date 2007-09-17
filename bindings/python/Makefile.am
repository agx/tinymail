INCLUDES = $(PYTHON_INCLUDES) $(LIBTINYMAIL_CFLAGS) $(PYGTK_CFLAGS) \
	$(LIBTINYMAILUI_GTK_CFLAGS) $(TINYMAIL_PYTHON_CFLAGS) \
	-I$(top_srcdir)/libtinymail -I$(top_srcdir)/libtinymailui \
	-I$(top_srcdir)/libtinymailui-gtk -I$(top_srcdir)/libtinymail-camel \
	-I$(top_srcdir)/$(PLATFORMDIR) -I$(top_srcdir)/libtinymail-camel/camel-lite

common_ldflags = -module -avoid-version

pthdir = $(pythondir)
pth_DATA = tinymail.pth

tinymaildir = $(pythondir)/tinymail-$(API_VERSION)/tinymail
ptinymaildir = $(pythondir)/tinymail-$(API_VERSION)/tinymail

tinymail_LTLIBRARIES = _tinymail.la camel.la ui.la 
if BUILD_UIGTK
tinymail_LTLIBRARIES += uigtk.la 
endif
tinymail_LTLIBRARIES += platform.la 

ptinymail_PYTHON = __init__.py

_tinymail_la_LDFLAGS = $(common_ldflags) -export-symbols-regex init_tinymail
_tinymail_la_LIBADD = $(LIBTINYMAIL_LIBS) $(TINYMAIL_PYTHON_LIBS) \
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la 
_tinymail_la_SOURCES = tinymailmodule.c pytny-status.c
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

camel_la_LDFLAGS = $(common_ldflags) -export-symbols-regex initcamel
camel_la_LIBADD = $(LIBTINYMAILUI_LIBS) $(LIBTINYMAIL_LIBS) \
        $(TINYMAIL_PYTHON_LIBS) \
        $(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la \
        $(top_builddir)/$(PLATFORMDIR)/$(PLATFORMDIR)-$(API_VERSION).la \
        $(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la
camel_la_SOURCES = tinymail-camelmodule.c pytny-session-camel.c
nodist_camel_la_SOURCES = tinymail-camel-glue.c

CREATEDEFS = $(PYTHON) $(top_srcdir)/bindings/python/createdefs.py

CLEANFILES = tinymail-glue.c tinymailui-glue.c tinymailui-gtk-glue.c \
	tinymail-platform-glue.c platform_include.h tinymail.defs \
	tinymailui.defs tinymailui-gtk.defs \
	tinymail-camel.defs tinymail-camel-glue.c

TINYMAIL_DEFS = $(top_srcdir)/bindings/python/tinymail-base.defs \
	$(top_srcdir)/bindings/python/tinymail-extra.defs

tinymail.defs: $(TINYMAIL_DEFS)
	$(CREATEDEFS) $@ $(TINYMAIL_DEFS)

tinymail-glue.c: tinymail.defs tinymail.override
	$(PYGTK_CODEGEN) --prefix pytinymail \
	--override $(top_srcdir)/bindings/python/tinymail.override \
	tinymail.defs > $@

TINYMAILUI_DEFS = $(top_srcdir)/bindings/python/tinymailui-base.defs

tinymailui.defs: $(TINYMAILUI_DEFS)
	$(CREATEDEFS) $@ $(TINYMAILUI_DEFS)

tinymailui-glue.c: tinymailui.defs tinymailui.override
	$(PYGTK_CODEGEN) --prefix pyui \
	--register tinymail.defs \
	--override $(top_srcdir)/bindings/python/tinymailui.override \
	tinymailui.defs > $@

TINYMAILUI_GTK_DEFS = $(top_srcdir)/bindings/python/tinymailui-gtk-base.defs

tinymailui-gtk.defs: $(TINYMAILUI_GTK_DEFS)
	$(CREATEDEFS) $@ $(TINYMAILUI_GTK_DEFS)

tinymailui-gtk-glue.c: tinymail.defs $(PYGTK_DEFSDIR)/gdk-types.defs $(PYGTK_DEFSDIR)/gtk-types.defs tinymailui-gtk.defs tinymailui-gtk.override
	$(PYGTK_CODEGEN) --prefix pyuigtk --register $(PYGTK_DEFSDIR)/gdk-types.defs \
	--register $(PYGTK_DEFSDIR)/gtk-types.defs --register tinymail.defs \
	--register tinymailui.defs --override $(top_srcdir)/bindings/python/tinymailui-gtk.override \
	tinymailui-gtk.defs > $@

TINYMAIL_PLATFORM_DEFS = $(top_srcdir)/bindings/python/tinymail-platform-base.defs

tinymail-platform.defs: $(TINYMAIL_PLATFORM_DEFS)
	$(CREATEDEFS) $@ $(TINYMAIL_PLATFORM_DEFS)

platform_include.h:
	ls $(top_srcdir)/libtinymail-gnome-desktop/*h | grep -v \\-priv | sed 's/^/\#include\ \"/g' | sed 's/\.h/\.h\"/g' > $@

tinymail-platform-glue.c: tinymail-platform.defs tinymail-platform.override platform_include.h
	$(PYGTK_CODEGEN) --prefix pyplatform \
	--register tinymailui.defs \
	--register tinymail.defs \
	--register tinymail-camel.defs \
	--override $(top_srcdir)/bindings/python/tinymail-platform.override \
	tinymail-platform.defs > $@

TINYMAIL_CAMEL_DEFS = $(top_srcdir)/bindings/python/tinymail-camel-base.defs \
	$(top_srcdir)/bindings/python/tinymail-camel-extra.defs

tinymail-camel.defs: $(TINYMAIL_CAMEL_DEFS)
	$(CREATEDEFS) $@ $(TINYMAIL_CAMEL_DEFS)

tinymail-camel-glue.c: tinymail-camel.defs tinymail-camel.override
	$(PYGTK_CODEGEN) --prefix pycamel \
	--register tinymail.defs \
	--override $(top_srcdir)/bindings/python/tinymail-camel.override \
	tinymail-camel.defs > $@

EXTRA_DIST = filter.py tinymail.override \
	tinymailui.override tinymailui-gtk.override \
	tinymail-platform.override tinymail-camel.override \
	tinymail-camel-base.defs tinymail-camel-extra.defs \
	tinymail-base.defs tinymail-extra.defs \
	tinymailui-gtk-base.defs tinymail-platform-base.defs \
	tinymailui-base.defs $(CREATEDEFS) pytny-status.h \
	platform_include.h pytny-session-camel.h

#
#	pytny-status.c pytny-session-camel.c tinymailmodule.c \
#	tinymail-camelmodule.c tinymail-platformmodule.c \
#	tinymailui-gtkmodule.c tinymailuimodule.c
#
#
DIST_SUBDIRS=gendefs


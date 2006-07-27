INCLUDES = $(PYTHON_INCLUDES) $(LIBTINYMAIL_CFLAGS) 		\
	$(LIBTINYMAILUI_GTK_CFLAGS) $(TINYMAIL_PYTHON_CFLAGS) 	\
	-I$(top_srcdir)/libtinymail				\
	-I$(top_srcdir)/libtinymailui				\
	-I$(top_srcdir)/libtinymailui-gtk			\
	-I$(top_srcdir)/libtinymail-camel			\
	-I$(top_srcdir)/$(PLATFORMDIR)

tinymaildir = $(pythondir)/tinymail
tinymail_LTLIBRARIES = tinymail.la tinymailui.la tinymailui-gtk.la tinymail-platform.la

tinymail_la_LDFLAGS = -module -avoid-version 
# -export-symbols-regex inittinymail
tinymail_la_LIBADD = $(LIBTINYMAIL_LIBS) $(TINYMAIL_PYTHON_LIBS) 	\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la 
tinymail_la_SOURCES = tinymailmodule.c tinymail-glue.c

tinymailui_la_LDFLAGS = -module -avoid-version
# -export-symbols-regex inittinymail
tinymailui_la_LIBADD = $(LIBTINYMAILUI_LIBS) $(TINYMAIL_PYTHON_LIBS) 	\
	$(LIBTINYMAIL_LIBS)						\
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la
tinymailui_la_SOURCES = tinymailuimodule.c tinymailui-glue.c
          
tinymailui_gtk_la_LDFLAGS = -module -avoid-version 
# -export-symbols-regex inittinymailui_gtk
tinymailui_gtk_la_LIBADD = $(LIBTINYMAILUI_GTK_LIBS) $(LIBTINYMAILUI_LIBS) 	\
	$(LIBTINYMAIL_LIBS) $(TINYMAIL_PYTHON_LIBS) 				\
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la
tinymailui_gtk_la_SOURCES = tinymailui-gtkmodule.c tinymailui-gtk-glue.c

tinymail_platform_la_LDFLAGS = -module -avoid-version
# -export-symbols-regex inittinymail_platform
tinymail_platform_la_LIBADD = $(LIBTINYMAILUI_LIBS) $(LIBTINYMAIL_LIBS) \
	$(TINYMAIL_PYTHON_LIBS) 					\
	$(top_builddir)/$(PLATFORMDIR)/$(PLATFORMDIR)-$(API_VERSION).la	\
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la
tinymail_platform_la_SOURCES = tinymail-platformmodule.c tinymail-platform-glue.c

CLEANFILES = tinymail-glue.c tinymailui-glue.c tinymailui-gtk-glue.c 	\
	tinymail-platform-glue.c platform_include.h tinymail.defs	\
	tinymailui.defs tinymailui-gtk.defs tinymail-platform.defs

EXTRA_DIST = 			\
	tinymail.defs 		\
	tinymail.override 	\
	tinymailui.defs		\
	tinymailui.override	\
	tinymailui-gtk.override	\
	tinymailui-gtk.defs 	\
	tinymail-platform.defs	\
	tinymail-platform.override

# To update:
# ls DIR/*h | grep -v \\-priv | sed s/^/\\t\$\(top_srcdir\)\\//g | sed s/$/\\t\\\\/g

tinymail.defs: $(PYTHON) $(PYGTK_CODEGENDIR)/h2def.py
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
	$(top_srcdir)/libtinymail/tny-transport-account-iface.h > $@
 

tinymailui.defs: $(PYTHON) $(PYGTK_CODEGENDIR)/h2def.py
	$(PYTHON) $(PYGTK_CODEGENDIR)/h2def.py 			   \
	$(top_srcdir)/libtinymailui/tny-account-store-view-iface.h \
	$(top_srcdir)/libtinymailui/tny-msg-header-view-iface.h	   \
	$(top_srcdir)/libtinymailui/tny-msg-view-iface.h	   \
	$(top_srcdir)/libtinymailui/tny-msg-window-iface.h	   \
	$(top_srcdir)/libtinymailui/tny-platform-factory-iface.h   \
	$(top_srcdir)/libtinymailui/tny-save-strategy-iface.h	   \
	$(top_srcdir)/libtinymailui/tny-summary-view-iface.h	   \
	$(top_srcdir)/libtinymailui/tny-summary-window-iface.h > $@

tinymailui-gtk.defs: $(PYTHON) $(PYGTK_CODEGENDIR)/h2def.py
	$(PYTHON) $(PYGTK_CODEGENDIR)/h2def.py				\
	$(top_srcdir)/libtinymailui-gtk/tny-account-tree-model.h	\
	$(top_srcdir)/libtinymailui-gtk/tny-attach-list-model.h		\
	$(top_srcdir)/libtinymailui-gtk/tny-msg-header-list-model.h	\
	$(top_srcdir)/libtinymailui-gtk/tny-msg-header-view.h		\
	$(top_srcdir)/libtinymailui-gtk/tny-msg-view.h			\
	$(top_srcdir)/libtinymailui-gtk/tny-msg-window.h		\
	$(top_srcdir)/libtinymailui-gtk/tny-save-strategy.h		\
	$(top_srcdir)/libtinymailui-gtk/tny-text-buffer-stream.h > $@

tinymail-platform.defs : $(PYTHON) $(PYGTK_CODEGENDIR)/h2def.py
	$(PYTHON) $(PYGTK_CODEGENDIR)/h2def.py				\
	`ls $(top_srcdir)/$(PLATFORMDIR)/*h | grep -v \\\-priv | xargs` > $@

tinymail-glue.c: tinymail.defs tinymail.override
	$(PYGTK_CODEGEN) --prefix tinymail		\
	--override tinymail.override			\
	tinymail.defs > $@

tinymailui-glue.c: tinymailui.defs tinymailui.override
	$(PYGTK_CODEGEN) --prefix tinymailui		\
	--override tinymailui.override			\
	tinymailui.defs > $@

tinymailui-gtk-glue.c: tinymailui-gtk.defs tinymailui-gtk.override
	$(PYGTK_CODEGEN) --prefix tinymailui_gtk	\
	--register $(PYGTK_DEFSDIR)/gdk-types.defs      \
	--register $(PYGTK_DEFSDIR)/gtk-types.defs      \
	--override tinymailui-gtk.override		\
	tinymailui-gtk.defs > $@

platform_include.h:
	ls $(top_srcdir)/$(PLATFORMDIR)/*h | grep -v \\-priv | sed 's/^/\#include\ \"/g' | sed 's/\.h/\.h\"/g' > $@

tinymail-platform-glue.c: tinymail-platform.defs tinymail-platform.override platform_include.h
	$(PYGTK_CODEGEN) --prefix tinymail_platform	\
	--override tinymail-platform.override		\
	tinymail-platform.defs > $@

.defs.c:
	(cd $(srcdir) && $(PYGTK_CODEGEN) --override $*.override \
	--prefix py$* $*.defs) > gen-$*.c && cp gen-$*.c $*.c && \
	rm -f gen-$*.c


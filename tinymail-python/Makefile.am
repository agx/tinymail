INCLUDES = $(PYTHON_INCLUDES) $(LIBTINYMAIL_CFLAGS) \
	$(LIBTINYMAIL_CAMEL_CFLAGS) $(TINYMAIL_PYTHON_CFLAGS) \
	-I$(top_srcdir)/libtinymail		\
	-I$(top_srcdir)/libtinymailui		\
	-I$(top_srcdir)/libtinymailui-gtk	\
	-I$(top_srcdir)/libtinymail-camel	\
	-I$(top_srcdir)/$(PLATFORMDIR)

tinymaildir = $(pythondir)/tinymail
tinymail_LTLIBRARIES = tinymail.la tinymailui.la 
#tinymail_camel.la

tinymail_la_LDFLAGS = -module -avoid-version 
# -export-symbols-regex inittinymail
tinymail_la_LIBADD = $(LIBTINYMAIL_LIBS) $(TINYMAIL_PYTHON_LIBS) $(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la
tinymail_la_SOURCES = tinymailmodule.c tinymail-glue.c


tinymailui_la_LDFLAGS = -module -avoid-version
# -export-symbols-regex inittinymail
tinymailui_la_LIBADD = $(LIBTINYMAILUI_LIBS) $(TINYMAIL_PYTHON_LIBS) $(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la
tinymailui_la_SOURCES = tinymailuimodule.c tinymailui-glue.c
          
tinymail_camel_la_LDFLAGS = -module -avoid-version 
# -export-symbols-regex inittinymail_camel
tinymail_camel_la_LIBADD = $(LIBTINYMAIL_CAMEL_LIBS) $(TINYMAIL_PYTHON_LIBS) $(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la
tinymail_camel_la_SOURCES = tinymail_camelmodule.c tinymail_camel-glue.c

CLEANFILES = tinymail-glue.c tinymailui-glue.c
# tinymail_camel-glue.c

EXTRA_DIST = 			\
	tinymail.defs 		\
	tinymail.override 	\
	tinymailui.defs		\
	tinymailui.override

#	tinymail_camel.override
#	tinymail_camel.defs 

# To update:
# ls DIR/*h | grep -v -priv | sed s/^/\\t\$\(top_srcdir\)\\//g | sed s/$/\\t\\\\/g

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

tinymail_camel.defs: $(PYTHON) $(PYGTK_CODEGENDIR)/h2def.py
	$(PYTHON) $(PYGTK_CODEGENDIR)/h2def.py			\
	$(top_srcdir)/libtinymail-camel/tny-account.h		\
	$(top_srcdir)/libtinymail-camel/tny-camel-shared.h	\
	$(top_srcdir)/libtinymail-camel/tny-camel-stream.h	\
	$(top_srcdir)/libtinymail-camel/tny-msg-folder.h	\
	$(top_srcdir)/libtinymail-camel/tny-msg.h		\
	$(top_srcdir)/libtinymail-camel/tny-msg-header.h	\
	$(top_srcdir)/libtinymail-camel/tny-msg-mime-part.h	\
	$(top_srcdir)/libtinymail-camel/tny-session-camel.h	\
	$(top_srcdir)/libtinymail-camel/tny-store-account.h	\
	$(top_srcdir)/libtinymail-camel/tny-stream-camel.h	\
	$(top_srcdir)/libtinymail-camel/tny-transport-account.h > $@

tinymail-glue.c: tinymail.defs tinymail.override
	$(PYGTK_CODEGEN) --prefix tinymail		\
	--override tinymail.override			\
	tinymail.defs > $@

tinymailui-glue.c: tinymailui.defs tinymailui.override
	$(PYGTK_CODEGEN) --prefix tinymail		\
	--override tinymail.override			\
	tinymail.defs > $@

tinymail_camel-glue.c: tinymail_camel.defs tinymail_camel.override
	$(PYGTK_CODEGEN) --prefix tinymail		\
	--override tinymail_camel.override		\
	tinymail_camel.defs > $@

.defs.c:
	(cd $(srcdir) && $(PYGTK_CODEGEN) --override $*.override \
	--prefix py$* $*.defs) > gen-$*.c && cp gen-$*.c $*.c && \
	rm -f gen-$*.c


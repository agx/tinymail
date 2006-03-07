INCLUDES = $(PYTHON_INCLUDES) $(LIBTINYMAIL_CFLAGS) $(LIBTINYMAIL_CAMEL_CFLAGS) $(TINYMAIL_PYTHON_CFLAGS)

tinymaildir = $(pythondir)/tinymail
tinymail_LTLIBRARIES = tinymail.la tinymail_camel.la

tinymail_la_LDFLAGS = -module -avoid-version -export-symbols-regex inittinymail
tinymail_la_LIBADD = $(LIBTINYMAIL_CAMEL_LIBS) $(TINYMAIL_PYTHON_LIBS) $(top_srcdir)/libtinymail/libtinymail-1.0.la
tinymail_la_SOURCES = \
	tinymail.c

nodist_tinymail_la_SOURCES = tinymailmodule.c

tinymail_camel_la_LDFLAGS = -module -avoid-version -export-symbols-regex inittinymail_camel
tinymail_camel_la_LIBADD = $(LIBTINYMAIL_CAMEL_LIBS) $(TINYMAIL_PYTHON_LIBS) $(top_srcdir)/libtinymail-camel/libtinymail-camel-1.0.la
tinymail_camel_la_SOURCES = \
	tinymail_camel.c

nodist_tinymail_camel_la_SOURCES = tinymail_camelmodule.c

CLEANFILES = tinymail.c tinymail_camel.c
EXTRA_DIST = \
    tinymail.defs \
    tinymail.override \
    tinymail_camel.defs \
    tinymail_camel.override

tinymail.c: tinymail.defs tinymail.override
tinymail-camel.c: tinymail_camel.defs tinymail_camel.override
.defs.c:
	(cd $(srcdir) \
	 && $(PYGTK_CODEGEN) \
    	--override $*.override \
	    --prefix py$* $*.defs) > gen-$*.c \
	&& cp gen-$*.c $*.c \
	&& rm -f gen-$*.c


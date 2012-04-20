INCLUDES = $(LIBTINYMAIL_CFLAGS) -I. -I$(top_srcdir) $(EXTRA_CFLAGS)

lib_LTLIBRARIES = libtinymail-2.0.la

tny-signals-marshal.h: tny-signals-marshal.list
	$(AM_V_GEN)$(GLIB_GENMARSHAL) --header --prefix=$(subst -,_,tny)_marshal $< > tny-signals-marshal.h

tny-signals-marshal.c: tny-signals-marshal.list tny-signals-marshal.h
	$(AM_V_GEN)$(GLIB_GENMARSHAL) --body --prefix=$(subst -,_,tny)_marshal $< > tny-signals-marshal.c
	sed -i "1i#include \"tny-signals-marshal.h\"" tny-signals-marshal.c

BUILT_SOURCES = \
	tny-signals-marshal.h \
	tny-signals-marshal.c \
	tny-type-builtins.h \
	tny-type-builtins.c

CLEANFILES= \
	$(BUILT_SOURCES) \
	stamp-tny-type-builtins.h \
	stamp-tny-type-builtins.c

tny-type-builtins.h: stamp-tny-type-builtins.h
	@true
stamp-tny-type-builtins.h: tny-type-builtins.h.template $(libtinymail_2_0_headers)
	$(AM_V_GEN)$(GLIB_MKENUMS) --template $< $(filter-out $<,$^) > xgen-etbh \
	&& (cmp -s xgen-etbh tny-type-builtins.h || cp xgen-etbh tny-type-builtins.h ) \
	&& rm -f xgen-etbh \
	&& echo timestamp > $(@F)

tny-type-builtins.c: stamp-tny-type-builtins.c
	@true
stamp-tny-type-builtins.c: tny-type-builtins.c.template $(libtinymail_2_0_headers)
	$(AM_V_GEN)$(GLIB_MKENUMS) --template $< $(filter-out $<,$^) > xgen-etbc \
	&& (cmp -s xgen-etbc tny-type-builtins.c || cp xgen-etbc tny-type-builtins.c ) \
	&& rm -f xgen-etbc \
	&& echo timestamp > $(@F)


private_headers = tny-common-priv.h observer-mixin-priv.h

libtinymail_2_0_headers = \
	tny-signals-marshal.h \
	tny-shared.h tny.h \
	tny-error.h \
	tny-status.h \
	tny-list.h \
	tny-iterator.h \
	tny-folder.h \
	tny-mime-part.h \
	tny-header.h \
	tny-pair.h \
	tny-folder-stats.h \
	tny-folder-change.h \
	tny-folder-store-change.h \
	tny-folder-observer.h \
	tny-folder-store-observer.h \
	tny-folder-monitor.h \
	tny-msg.h \
	tny-device.h \
	tny-lockable.h \
	tny-noop-lockable.h \
	tny-account.h \
	tny-store-account.h \
	tny-transport-account.h \
	tny-account-store.h \
	tny-stream.h \
	tny-seekable.h \
	tny-fs-stream.h \
	tny-simple-list.h \
	tny-folder-store.h \
	tny-folder-store-query.h \
	tny-msg-remove-strategy.h \
	tny-msg-receive-strategy.h \
	tny-send-queue.h \
	tny-password-getter.h \
	tny-merge-folder.h \
	tny-stream-cache.h \
	tny-fs-stream-cache.h \
	tny-cached-file.h \
	tny-cached-file-stream.h \
	tny-combined-account.h \
	tny-connection-policy.h \
	tny-merge-folder-store.h

libtinymail_2_0_la_SOURCES = \
	$(libtinymail_1_0_headers) \
	tny-expunged-header.c \
	tny-signals-marshal.c \
	tny-error.c \
	tny-status.c \
	tny-list.c \
	tny-iterator.c \
	tny-msg.c \
	tny-header.c \
	tny-pair.c \
	tny-folder-stats.c \
	tny-folder-change.c \
	tny-folder-store-change.c \
	tny-folder-observer.c \
	tny-folder-store-observer.c \
	tny-folder-monitor.c \
	tny-mime-part.c \
	tny-folder.c \
	tny-device.c \
	tny-lockable.c \
	tny-noop-lockable.c \
	tny-account.c \
	tny-store-account.c \
	tny-transport-account.c \
	tny-account-store.c \
	tny-stream.c \
	tny-seekable.c \
	tny-fs-stream.c \
	tny-simple-list-priv.h \
	tny-simple-list.c \
	tny-simple-list-iterator-priv.h \
	tny-simple-list-iterator.c \
	tny-folder-store.c \
	tny-folder-store-query.c \
	tny-msg-remove-strategy.c \
	tny-msg-receive-strategy.c \
	tny-send-queue.c \
	tny-password-getter.c \
	tny-stream-cache.c \
	tny-fs-stream-cache.c \
	tny-cached-file.c \
	tny-cached-file-stream.c \
	tny-merge-folder.c \
	tny-combined-account.c \
	tny-idle-stopper.c \
	tny-progress-info.c \
	tny-connection-policy.c \
	tny-merge-folder-store.c \
	observer-mixin.c \
	$(private_headers)

libtinymail_2_0_la_LIBADD = $(LIBTINYMAIL_LIBS)
libtinymail_2_0_la_LDFLAGS = -export-dynamic -version-info $(LT_VERSION_INFO) $(LIBTOOL_EXPORT_OPTIONS)
libtinymail_2_0_includedir = $(includedir)/libtinymail-$(API_VERSION)
libtinymail_2_0_include_HEADERS = $(libtinymail_2_0_headers)

if ENABLE_INTROSPECTION

Tinymail-$(API_VERSION).gir: libtinymail-2.0.la Makefile $(libtinymail_2_0_headers) $(filter %.c,$(libtinymail_2_0_la_SOURCES))
	$(AM_V_GEN) PKG_CONFIG_PATH=$(top_builddir):$$PKG_CONFIG_PATH \
	$(G_IR_SCANNER) -v --warn-all --namespace Tinymail \
	--strip-prefix=Tny \
	--nsversion=$(API_VERSION) \
	--include=GLib-2.0 \
	--include=GObject-2.0 \
	--library=tinymail-2.0 \
	--libtool="$(LIBTOOL)" \
	--output $@ \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	$(filter %.h,$^) \
	$(filter %.c,$^)

girdir = $(GIRDIR)
gir_DATA = Tinymail-$(API_VERSION).gir

typelibsdir = $(GIRTYPELIBDIR)
typelibs_DATA = Tinymail-$(API_VERSION).typelib

%.typelib: %.gir
	$(AM_V_GEN) LD_LIBRARY_PATH=$${LD_LIBRARY_PATH:+$$LD_LIBRARY_PATH:}. $(G_IR_COMPILER) \
	--includedir=$(srcdir) \
	$(G_IR_COMPILER_OPTS) \
	$< -o $@
endif #ENABLE_INSTROSPECTION


libtinymail-$(API_VERSION).pc: libtinymail.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtinymail-$(API_VERSION).pc
EXTRA_DIST = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in) \
	tny-type-builtins.c.template \
	tny-type-builtins.h.template \
	tny-signals-marshal.list

DISTCLEANFILES = $(pkgconfig_DATA)

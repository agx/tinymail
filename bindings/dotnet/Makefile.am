RAW_API=libtinymail-api.raw
API=libtinymail-api.xml
METADATA=libtinymail-1.0.metadata

ASSEMBLY_NAME=libtinymail-sharp
ASSEMBLY=$(ASSEMBLY_NAME).dll

sources = 

build_sources = $(addprefix $(srcdir)/, $(sources)) AssemblyInfo.cs

customs = List.custom Iterator.custom 

CLEANFILES = $(ASSEMBLY) generated-stamp generated/*.cs $(API) $(RAW_API)

DISTCLEANFILES= AssemblyInfo.cs

noinst_DATA = $(ASSEMBLY)

all: generated-stamp libtinymail-sharp.dll

$(RAW_API): $(PARSER) libtinymail-sources.xml
	$(PARSER) libtinymail-sources.xml

$(API): $(RAW_API) $(METADATA)
	cp libtinymail-api.raw libtinymail-api.xml
	cp libtinymailui-api.raw libtinymailui-api.xml
	cp libtinymailui-gtk-api.raw libtinymailui-gtk-api.xml
	cp libtinymail-camel-api.raw libtinymail-camel-api.xml
	cp $(PLATFORMDIR)-api.raw $(PLATFORMDIR)-api.xml
	chmod u+w libtinymail-api.xml libtinymailui-api.xml libtinymailui-gtk-api.xml libtinymail-camel-api.xml $(PLATFORMDIR)-api.xml
	$(FIXUP) --api=libtinymail-api.xml --metadata=$(METADATA)
	$(FIXUP) --api=libtinymailui-api.xml --metadata=$(METADATA)
	$(FIXUP) --api=libtinymailui-api.xml --metadata=$(METADATA)
	$(FIXUP) --api=libtinymailui-gtk-api.xml --metadata=$(METADATA)
#	$(FIXUP) --api=libtinymail-camel-api.xml --metadata=$(METADATA)
	$(FIXUP) --api=$(PLATFORMDIR)-api.xml --metadata=$(METADATA)

generated-stamp: $(API)
	rm -rf generated
	$(CODEGEN) --generate $(srcdir)/libtinymail-api.xml \
		$(GTK_SHARP_CFLAGS) --outdir=generated --customdir=$(srcdir) \
		--assembly-name=libtinymail-sharp
	$(CODEGEN) --generate $(srcdir)/libtinymailui-api.xml \
		$(GTK_SHARP_CFLAGS) --outdir=generated-ui --customdir=$(srcdir) \
		--assembly-name=libtinymailui-sharp
	$(CODEGEN) --generate $(srcdir)/libtinymailui-gtk-api.xml \
		$(GTK_SHARP_CFLAGS) --outdir=generated-ui-gtk --customdir=$(srcdir) \
		--assembly-name=libtinymailui-gtk-sharp
#	$(CODEGEN) --generate $(srcdir)/libtinymail-camel-api.xml \
#		$(GTK_SHARP_CFLAGS) --outdir=generated-camel --customdir=$(srcdir) \
#		--assembly-name=libtinymail-camel-sharp
	$(CODEGEN) --generate $(srcdir)/$(PLATFORMDIR)-api.xml \
		$(GTK_SHARP_CFLAGS) --outdir=generated-$(PLATFORMDIR) --customdir=$(srcdir) \
		--assembly-name=$(PLATFORMDIR)-sharp
	touch generated-stamp

libtinymail-sharp.dll: $(build_sources) generated-stamp generated
	$(CSC) --unsafe --target library $(GTK_SHARP_LIBS) \
		$(build_sources) generated/*.cs -o libtinymail-sharp.dll
	$(CSC) --unsafe --target library $(GTK_SHARP_LIBS) \
		$(build_sources) generated-ui/*.cs -o libtinymailui-sharp.dll
	$(CSC) --unsafe --target library $(GTK_SHARP_LIBS) \
		$(build_sources) generated-ui-gtk/*.cs -o libtinymailui-gtk-sharp.dll
#	$(CSC) --unsafe --target library $(GTK_SHARP_LIBS) \
#		$(build_sources) generated-camel/*.cs -o libtinymail-camel-sharp.dll
	$(CSC) --unsafe --target library $(GTK_SHARP_LIBS) \
		$(build_sources) generated-$(PLATFORMDIR)/*.cs -o $(PLATFORMDIR)-sharp.dll

install-data-local:
	echo "$(GACUTIL) /i libtinymail-sharp.dll /f /package libtinymail-sharp-$(API_VERSION) /root $(DESTDIR)$(libdir)";  \
      	  $(GACUTIL) /i libtinymail-sharp.dll /f /package libtinymail-sharp-$(API_VERSION) /root $(DESTDIR)$(libdir) || exit 1;
	echo "$(GACUTIL) /i libtinymailui-sharp.dll /f /package libtinymailui-sharp-$(API_VERSION) /root $(DESTDIR)$(libdir)";  \
      	  $(GACUTIL) /i libtinymailui-sharp.dll /f /package libtinymailui-sharp-$(API_VERSION) /root $(DESTDIR)$(libdir) || exit 1;
	echo "$(GACUTIL) /i libtinymailui-gtk-sharp.dll /f /package libtinymailui-gtk-sharp-$(API_VERSION) /root $(DESTDIR)$(libdir)";  \
      	  $(GACUTIL) /i libtinymailui-gtk-sharp.dll /f /package libtinymailui-sharp-$(API_VERSION) /root $(DESTDIR)$(libdir) || exit 1;
#	echo "$(GACUTIL) /i libtinymail-camel-sharp.dll /f /package libtinymail-camel-sharp-$(API_VERSION) /root $(DESTDIR)$(libdir)";  \
#     	  $(GACUTIL) /i libtinymail-camel-sharp.dll /f /package libtinymail-camel-sharp-$(API_VERSION) /root $(DESTDIR)$(libdir) || exit 1;
	echo "$(GACUTIL) /i $(PLATFORMDIR)-sharp.dll /f /package $(PLATFORMDIR)-sharp-$(API_VERSION) /root $(DESTDIR)$(libdir)";  \
      	  $(GACUTIL) /i $(PLATFORMDIR)-sharp.dll /f /package $(PLATFORMDIR)-sharp-$(API_VERSION) /root $(DESTDIR)$(libdir) || exit 1;

uninstall-local:
	echo "$(GACUTIL) /u libtinymail-sharp /package libtinymail-sharp-$(API_VERSION) /root $(DESTDIR)$(libdir)"; \
	        $(GACUTIL) /u libtinymail-sharp /package libtinymail-sharp-$(API_VERSION) /root $(DESTDIR)$(libdir) || exit 1;
	echo "$(GACUTIL) /u libtinymailui-sharp /package libtinymailui-sharp-$(API_VERSION) /root $(DESTDIR)$(libdir)"; \
	        $(GACUTIL) /u libtinymailui-sharp /package libtinymailui-sharp-$(API_VERSION) /root $(DESTDIR)$(libdir) || exit 1;
	echo "$(GACUTIL) /u libtinymailui-gtk-sharp /package libtinymailui-gtk-sharp-$(API_VERSION) /root $(DESTDIR)$(libdir)"; \
	        $(GACUTIL) /u libtinymailui-gtk-sharp /package libtinymailui-gtk-sharp-$(API_VERSION) /root $(DESTDIR)$(libdir) || exit 1;
#	echo "$(GACUTIL) /u libtinymail-camel-sharp /package libtinymail-camel-sharp-$(API_VERSION) /root $(DESTDIR)$(libdir)"; \
#	        $(GACUTIL) /u libtinymail-camel-sharp /package libtinymail-camel-sharp-$(API_VERSION) /root $(DESTDIR)$(libdir) || exit 1;
	echo "$(GACUTIL) /u $(PLATFORMDIR)-sharp /package $(PLATFORMDIR)-sharp-$(API_VERSION) /root $(DESTDIR)$(libdir)"; \
	        $(GACUTIL) /u $(PLATFORMDIR)-sharp /package $(PLATFORMDIR)-sharp-$(API_VERSION) /root $(DESTDIR)$(libdir) || exit 1;

EXTRA_DIST = libtinymail-api.raw libtinymailui-api.raw libtinymailui-gtk-api.raw libtinymail-camel-api.raw $(PLATFORMDIR)-api.raw \
	$(sources) $(customs) \
	libtinymail-1.0.metadata libtinymailui-1.0.metadata libtinymailui-gtk-1.0.metadata libtinymail-camel-1.0.metadata \
	AssemblyInfo.cs.in \
	libtinymail-1.0.config.in

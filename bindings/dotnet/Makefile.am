RAW_API=libtinymail-api.raw
API=libtinymail-api.xml
METADATA=libtinymail-1.0.metadata

ASSEMBLY_NAME=libtinymail-sharp
ASSEMBLY=$(ASSEMBLY_NAME).dll

sources = 

build_sources = $(addprefix $(srcdir)/, $(sources)) AssemblyInfo.cs

customs = 

CLEANFILES = $(ASSEMBLY) generated-stamp generated/*.cs $(API) $(RAW_API)

DISTCLEANFILES= AssemblyInfo.cs

noinst_DATA = $(ASSEMBLY)

all: generated-stamp libtinymail-sharp.dll

$(RAW_API): $(PARSER) libtinymail-sources.xml
	$(PARSER) libtinymail-sources.xml

$(API): $(RAW_API) $(METADATA)
	cp $(RAW_API) $(API)
	chmod u+w $(API)
	$(FIXUP) --api=$(API) --metadata=$(METADATA)

generated-stamp: $(API)
	rm -rf generated
	$(CODEGEN) --generate $(srcdir)/$(API) $(GTK_SHARP_CFLAGS) --outdir=generated --customdir=$(srcdir) --assembly-name=$(ASSEMBLY_NAME) && touch generated-stamp

libtinymail-sharp.dll: $(build_sources) generated-stamp generated
	$(CSC) --unsafe --target library $(GTK_SHARP_LIBS) \
	$(build_sources) generated/*.cs -o $(ASSEMBLY)

install-data-local:
	echo "$(GACUTIL) /i $(ASSEMBLY) /f /package $(ASSEMBLY_NAME)-1.0 /root $(DESTDIR)$(libdir)";  \
        $(GACUTIL) /i $(ASSEMBLY) /f /package $(ASSEMBLY_NAME)-1.0 /root $(DESTDIR)$(libdir) || exit 1;

uninstall-local:
	echo "$(GACUTIL) /u $(ASSEMBLY_NAME) /package $(ASSEMBLY_NAME)-1.0 /root $(DESTDIR)$(libdir)"; \
        $(GACUTIL) /u $(ASSEMBLY_NAME) /package $(ASSEMBLY_NAME)-1.0 /root $(DESTDIR)$(libdir) || exit 1;

EXTRA_DIST = $(RAW_API) $(sources) $(customs) $(METADATA) AssemblyInfo.cs.in $(ASSEMBLY).config.in

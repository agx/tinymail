SUBDIRS = .

pkg = libtinymail-camel

SYMBOLS =
INCLUDE_API =
METADATA = libtinymail-camel.metadata
references =
glue_includes =

sources =

customs =


%-$(API_VERSION).pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtinymail--camel-sharp-$(API_VERSION).pc
add_dist = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in)
DISTCLEANFILES = $(pkgconfig_DATA)

include $(top_srcdir)/bindings/dotnet/Makefile.include


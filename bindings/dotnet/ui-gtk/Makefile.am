SUBDIRS = .

pkg = libtinymailui-gtk

SYMBOLS =
INCLUDE_API = ../tny/libtinymail-api.xml ../ui/libtinymailui-api.xml
METADATA = libtinymailui-gtk.metadata
references =
glue_includes =

sources =	

customs =


%-$(API_VERSION).pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtinymailui-gtk-sharp-$(API_VERSION).pc
add_dist = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in)
DISTCLEANFILES = $(pkgconfig_DATA)

include $(top_srcdir)/bindings/dotnet/Makefile.include


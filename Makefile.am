SUBDIRS = m4 

ACLOCAL_AMFLAGS = -I m4

SUBDIRS += libtinymail
SUBDIRS += libtinymailui

if BUILD_GIO
SUBDIRS += libtinymail-gio
endif

if BUILD_GNOME
SUBDIRS += libtinymailui-gnome-keyring
endif

if BUILD_GNOMEVFS
SUBDIRS += libtinymail-gnomevfs
endif

if BUILD_UIGTK
SUBDIRS += libtinymailui-gtk 
endif

if BUILD_WEBKIT
SUBDIRS += libtinymailui-webkit
endif

if BUILD_GTKHTML
SUBDIRS += libtinymailui-gtkhtml
endif


SUBDIRS += libtinymail-camel $(PLATFORMDIR) 

if BUILD_ACAP
SUBDIRS += libtinymail-acap
endif

if BUILD_TP
SUBDIRS += libtinymail-tp
endif


SUBDIRS += tests po

#### Funny OR simulation :)
if BUILD_TESTS
if !BUILD_UNITTESTS
SUBDIRS += docs 
endif
endif

if BUILD_UNITTESTS
if !BUILD_TESTS
SUBDIRS += docs 
endif
endif

if BUILD_UNITTESTS
if BUILD_TESTS
SUBDIRS += docs 
endif
endif
###


if BUILD_UNITTESTS
SUBDIRS += libtinymail-test 
endif

DIST_SUBDIRS = \
	libtinymail \
	libtinymailui \
	libtinymail-gnomevfs \
	libtinymail-gio \
	libtinymailui-gtk \
	libtinymail-camel \
	docs \
	libtinymail-test tests \
	po \
	libtinymail-olpc \
	libtinymail-gpe m4 \
	libtinymailui-mozembed \
	libtinymailui-gtkhtml \
	libtinymailui-gnome-keyring \
	libtinymail-acap \
	libtinymail-gnome-desktop \
	libtinymail-tp \
	libtinymailui-webkit \
	libtinymail-moblin

DISTCHECK_CONFIGURE_FLAGS = --with-html-component=mozembed --enable-gtk-doc --enable-python-bindings --enable-dotnet-bindings --enable-vala-bindings --enable-tests


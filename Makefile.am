SUBDIRS = m4 

SUBDIRS += libtinymail 
SUBDIRS += libtinymailui 

if BUILD_GNOME
SUBDIRS += libtinymail-gnomevfs 
endif

if BUILD_UIGTK
SUBDIRS += libtinymailui-gtk 
endif

if BUILD_MOZEMBED
SUBDIRS += libtinymailui-mozembed 
endif

SUBDIRS += libtinymail-camel $(PLATFORMDIR) 

SUBDIRS += tests bindings po 

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
	libtinymailui-gtk \
	libtinymail-camel \
	tinymail docs \
	libtinymail-test tests \
	bindings po \
	libtinymail-maemo \
	libtinymail-olpc \
	libtinymail-gpe m4 \
	libtinymailui-mozembed \
	libtinymail-gnome-desktop

DISTCHECK_CONFIGURE_FLAGS = --with-html-component=mozembed --enable-gtk-doc --enable-python-bindings --enable-unit-tests --enable-tests


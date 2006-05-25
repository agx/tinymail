#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.
REQUIRED_AUTOMAKE_VERSION=1.7
PKG_NAME=tinymail

(test -f $srcdir/configure.ac \
  && test -f $srcdir/tinymail/tny-main.c) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level $PKG_NAME directory"
    exit 1
}


which gnome-autogen.sh || {
    echo "You need to install gnome-common from the GNOME CVS"
    exit 1
}
USE_GNOME2_MACROS=1 . gnome-autogen.sh

### Dirty lil hack #
patch -fp0 < gtk-doc.make.distcheck.patch

# TODO: Put this in a make target (stupid gtk-doc.make doesn't make this possible afaik)

find $srcdir -type f -iname "*.h" -printf "#include <%f>\n" | grep -v "\-priv" | grep -v "\-test" | grep -v "config.h" | grep -v "tny-summary-window.h" | grep -v camel > docs/devel/reference/libtinymail.types
echo >> docs/devel/reference/libtinymail.types
find $srcdir -type f -iname "*.h" -exec grep get_type {} \; | grep -v define | grep -v _test | grep -v tny_summary_window | grep -v camel | cut -d " " -f 2- | cut -d "(" -f 1| sed s/\ //g >> docs/devel/reference/libtinymail.types

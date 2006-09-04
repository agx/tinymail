
srcdir=$1
echo "#include <glib.h>" > docs/devel/reference/libtinymail.types

find '.' -type f -iname '*.h' -printf '#include <%f>\n' | grep -v "\-priv" | grep -v "\-test" | grep -v platform_include.h | grep -v camel.h | grep -v platfact.h | grep -v device.h | grep -v account-store.h | grep -v config.h | grep -v tny-summary-view.h | grep -v olpc | grep -v gnome | grep -v maemo | grep -v gpe >> docs/devel/reference/libtinymail.types

echo >> docs/devel/reference/libtinymail.types

find $srcdir -type f -iname "*.h" -exec grep get_type {} \; | grep -v _tny_list_iterator | grep -v tny_summary_view | grep -v define | grep -v _test | grep -v tny_maemo | grep -v tny_gpe | grep -v tny_gnome | grep -v tny_olpc | grep -v _camel_get_type | cut -d " " -f 2- | cut -d "(" -f 1 | sed s/\ //g >> docs/devel/reference/libtinymail.types


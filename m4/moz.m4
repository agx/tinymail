AC_DEFUN([AC_TNY_MOZ_CHECK],
[

mozilla_nspr="no"
AC_MSG_CHECKING(Mozilla NSPR pkg-config module name)
mozilla_nspr_pcs="nspr mozilla-nspr firefox-nspr xulrunner-nspr"
for pc in $mozilla_nspr_pcs; do
        if $PKG_CONFIG --exists $pc; then
                mozilla_nspr=$pc
                break;
        fi
done
AC_MSG_RESULT($mozilla_nspr)

mozilla_nss="no"
AC_MSG_CHECKING(Mozilla NSS pkg-config module name)
mozilla_nss_pcs="nss mozilla-nss firefox-nss xulrunner-nss"
for pc in $mozilla_nss_pcs; do
        if $PKG_CONFIG --exists $pc; then
                mozilla_nss=$pc
                break;
        fi
done
AC_MSG_RESULT($mozilla_nss)

mozilla_xpcom="no"
AC_MSG_CHECKING(Mozilla XPCOM pkg-config module name)
mozilla_xpcom_pcs="xpcom mozilla-xpcom firefox-xpcom xulrunner-xpcom"
for pc in $mozilla_xpcom_pcs; do
        if $PKG_CONFIG --exists $pc; then
                mozilla_xpcom=$pc
                break;
        fi
done
AC_MSG_RESULT($mozilla_xpcom)

mozilla_gtkmozembed="no"
AC_MSG_CHECKING(Mozilla gtkmozembed pkg-config module name)
mozilla_gtkmozembed_pcs="gtkmozembed mozilla-gtkmozembed firefox-gtkmozembed xulrunner-gtkmozembed"
for pc in $mozilla_gtkmozembed_pcs; do
        if $PKG_CONFIG --exists $pc; then
                mozilla_gtkmozembed=$pc
                break;
        fi
done
AC_MSG_RESULT($mozilla_gtkmozembed)

])


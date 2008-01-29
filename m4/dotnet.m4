AC_DEFUN([AC_TNY_DOTNET_CHECK],
[

if test x$build_net_bindings = xtrue; then
	PKG_CHECK_MODULES(MONO_DEPENDENCY, mono, has_mono=true, has_mono=false)
	if test "x$has_mono" = "xtrue"; then
		AC_PATH_PROG(RUNTIME, mono, no)
		AC_PATH_PROG(CSC, mcs, no)
		if test `uname -s` = "Darwin"; then
			LIB_PREFIX=
			LIB_SUFFIX=.dylib
		else
			LIB_PREFIX=.so
			LIB_SUFFIX=
		fi
	else
		AC_PATH_PROG(CSC, csc.exe, no)
		if test x$CSC = "xno"; then
			AC_MSG_ERROR([You need to install either mono or .Net])
		else
			RUNTIME=
			LIB_PREFIX=
			LIB_SUFFIX=.dylib
		fi
	fi


	PKG_CHECK_MODULES(GTK_SHARP, gtk-sharp-2.0 >= 2.11.91)
	PKG_CHECK_MODULES(GAPI, gapi-2.0 >= 2.11.91)

	AC_PATH_PROG(GACUTIL, gacutil, no)
	if test "x$GACUTIL" = "xno" ; then
	        AC_MSG_ERROR([No gacutil tool found])
	fi

	winbuild=no
	case "$host" in
 	      *-*-mingw*|*-*-cygwin*)
     	          winbuild=yes
      	         ;;
	esac
	if test "x$winbuild" = "xyes" ; then
		AC_PATH_PROG(PARSER, gapi-parser.exe, no)
		AC_PATH_PROG(CODEGEN, gapi-codegen.exe, no)
		AC_PATH_PROG(FIXUP, gapi-fixup.exe, no)
	else
		AC_PATH_PROG(PARSER, gapi2-parser, no)
		AC_PATH_PROG(CODEGEN, gapi2-codegen, no)
		AC_PATH_PROG(FIXUP, gapi2-fixup, no)
	fi

	if test "x$PARSER" = "xno" ; then
		AC_MSG_ERROR([No gapi-parser tool found])
	fi

	if test "x$CODEGEN" = "xno" ; then
		AC_MSG_ERROR([No gapi-codegen tool found])
	fi

	if test "x$FIXUP" = "xno" ; then
		AC_MSG_ERROR([No gapi-fixup tool found])
	fi
else
	PARSER=""
	CODEGEN=""
	FIXUP=""
	GACUTIL=""
	GAPI_CFLAGS=""
	GAPI_LIBS=""
	LIB_PREFIX=""
	LIB_SUFFIX=""
	GTK_SHARP_CFLAGS=""
	GTK_SHARP_LIBS=""
	RUNTIME=""
	CSC=""
	GAPI2_CODEGEN=""
	GAPI2_FIXUP=""
fi

AC_SUBST(PARSER)
AC_SUBST(CODEGEN)
AC_SUBST(FIXUP)
AC_SUBST(GAPI_CFLAGS)
AC_SUBST(GAPI_LIBS)
AC_SUBST(LIB_PREFIX)
AC_SUBST(LIB_SUFFIX)
AC_SUBST(GTK_SHARP_CFLAGS)
AC_SUBST(GTK_SHARP_LIBS)
AC_SUBST(CSC)
AC_SUBST(GAPI2_CODEGEN)
AC_SUBST(GAPI2_FIXUP)
AC_SUBST(RUNTIME)
AC_SUBST(GACUTIL)

])

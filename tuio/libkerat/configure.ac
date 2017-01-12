AC_PREREQ([2.66])
AC_INIT([libkerat], [1.0.0], [xrucka@fi.muni.cz], [libkerat], [https://sitola.cz/hci/index.php/Libkerat])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/session_manager.cpp])
AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([config.h])

KERAT_REVISION_VERSION=4

KERAT_LIBRARY_VERSION=1:0:0
KERAT_LIBRARY_RELEASE=0

KERAT_API_VERSION=1.0
AC_SUBST(KERAT_API_VERSION)
AC_SUBST(KERAT_LIBRARY_VERSION)
AC_SUBST(KERAT_LIBRARY_RELEASE)

LT_PREREQ([2.2])
LT_INIT

AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O

# Checks for compiler options
saved_cflags="$CPPFLAGS"

CHECK_CFLAGS=
CFLAGS=-pedantic
AC_MSG_CHECKING([whether $CC supports $CFLAGS])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
    [ AC_MSG_RESULT([yes]) ]
    [ CHECK_CFLAGS+=" $CFLAGS" ],
    [ AC_MSG_RESULT([no]) ]
)

CFLAGS=-Werror
AC_MSG_CHECKING([whether $CC supports $CFLAGS])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
    [ AC_MSG_RESULT([yes]) ]
    [ CHECK_CFLAGS+=" $CFLAGS" ],
    [ AC_MSG_RESULT([no]) ]
)

CFLAGS=-Wmissing-noreturn
AC_MSG_CHECKING([whether $CC supports $CFLAGS])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
    [ AC_MSG_RESULT([yes]) ]
    [ CHECK_CFLAGS+=" $CFLAGS" ],
    [ AC_MSG_RESULT([no]) ]
)

CFLAGS=-Wall
AC_MSG_CHECKING([whether $CC supports $CFLAGS])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
    [ AC_MSG_RESULT([yes]) ]
    [ CHECK_CFLAGS+=" $CFLAGS" ],
    [ AC_MSG_RESULT([no]) ]
)

CFLAGS=-Wextra
AC_MSG_CHECKING([whether $CC supports $CFLAGS])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
    [ AC_MSG_RESULT([yes]) ]
    [ CHECK_CFLAGS+=" $CFLAGS" ],
    [ AC_MSG_RESULT([no]) ]
)

AC_SUBST([CHECK_CFLAGS])

CFLAGS="$saved_cflags"

# Checks for libraries.
PKG_CHECK_MODULES([liblo], [liblo], [
	LIBKERAT_LIBS+=" ${liblo_LIBS}"
	LIBKERAT_CFLAGS+=" ${liblo_CFLAGS}"
], [
	AC_MSG_FAILURE([Liblo is required to build the kerat library!])
])

# Checks for header files.
AC_CHECK_HEADERS([inttypes.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library and posix functions.
AC_CHECK_FUNCS([memset sqrt clock_gettime])

LIB_CLOCK_GETTIME=
  AC_SEARCH_LIBS(clock_gettime, [rt posix4])
  case "$ac_cv_search_clock_gettime" in
    -l*) LIB_CLOCK_GETTIME=$ac_cv_search_clock_gettime;;
  esac
AC_SUBST(LIB_CLOCK_GETTIME)

# TUIO 2.0 draft non-compliance mode

enable_noncompliant=yes
AC_ARG_ENABLE(noncompliant, [  --disable-noncompliant  disable features that are not compliant with TUIO 2.0 draft] )

if test x$enable_noncompliant = xyes
then
    AC_DEFINE([DRAFT_NONCOMPLIANT], [1], [Enable features that are not compliant with TUIO 2.0 draft])
fi

DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN([libkerat], [Doxyfile], [doc])

AC_SUBST(LIBKERAT_CFLAGS)
AC_SUBST(LIBKERAT_LIBS)

AC_CONFIG_FILES([Makefile tools/Makefile tests/Makefile libkerat.pc Doxyfile])
AC_OUTPUT

AC_MSG_RESULT([
Configuration summary:

  Target ...................... $host
  Prefix ...................... $prefix

  Doxygen ..................... $DX_FLAG_doc
  TUIO 2.0 compliant .......... $enable_noncompliant

])


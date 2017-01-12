AC_PREREQ([2.66])
AC_INIT([dtuio], [0.0.1], [xrucka@fi.muni.cz], [libdtuio], [https://sitola.cz/hci/index.php/Dtuio])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/topology_neighbour.cpp])
AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([config.h])

DTUIO_LIBRARY_VERSION=0:1:0
DTUIO_LIBRARY_RELEASE=1

DTUIO_API_VERSION=0.0
AC_SUBST(DTUIO_API_VERSION)
AC_SUBST(DTUIO_LIBRARY_VERSION)
AC_SUBST(DTUIO_LIBRARY_RELEASE)

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
PKG_CHECK_MODULES([libkerat], [libkerat], [
    DTUIO_LIBS+=" ${libkerat_LIBS}"
    DTUIO_CFLAGS+=" ${libkerat_CFLAGS}"
], [
    AC_MSG_FAILURE([Libkerat is required to build this library!])
])

PKG_CHECK_MODULES([libuuid], [uuid], [
    DTUIO_LIBS+=" ${libuuid_LIBS}"
    DTUIO_CFLAGS+=" ${libuuid_CFLAGS}"
], [
    AC_MSG_FAILURE([Libuuid is required to build this library!])
])

# Checks for header files.
AC_CHECK_HEADERS([inttypes.h regex.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

#checks for library and posix functions.
AC_CHECK_FUNCS([memset sqrt clock_gettime])

LIB_CLOCK_GETTIME=
  AC_SEARCH_LIBS(clock_gettime, [rt posix4])
  case "$ac_cv_search_clock_gettime" in
    -l*) LIB_CLOCK_GETTIME=$ac_cv_search_clock_gettime;;
  esac
AC_SUBST(LIB_CLOCK_GETTIME)

DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(ON)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN([dtuio], [Doxyfile], [doc])

AC_SUBST(DTUIO_CFLAGS)
AC_SUBST(DTUIO_LIBS)

AC_CONFIG_FILES([Makefile libdtuio.pc Doxyfile tests/Makefile])
AC_OUTPUT


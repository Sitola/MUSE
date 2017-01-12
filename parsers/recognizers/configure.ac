AC_PREREQ([2.66])
AC_INIT([muse_recognizers], [0.0.0.1], [374178@mail.muni.cz], [libmuse_recognizers], [https://sitola.cz/hci/index.php/MUSE_RECOGNIZERS])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/dollar_recognizer.cpp])
AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([config.h])

MUSE_RECOGNIZERS_MAJOR_VERSION=0
MUSE_RECOGNIZERS_MINOR_VERSION=0
MUSE_RECOGNIZERS_MICRO_VERSION=0
MUSE_RECOGNIZERS_REVISION_VERSION=1

MUSE_RECOGNIZERS_LIBRARY_VERSION=0:1:0
MUSE_RECOGNIZERS_LIBRARY_RELEASE=1

MUSE_RECOGNIZERS_API_VERSION=0.0
AC_SUBST(MUSE_RECOGNIZERS_API_VERSION)
AC_SUBST(MUSE_RECOGNIZERS_LIBRARY_VERSION)
AC_SUBST(MUSE_RECOGNIZERS_LIBRARY_RELEASE)

LT_PREREQ([2.2])
LT_INIT

AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AC_LANG([C++])
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O

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
	MUSE_RECOGNIZERS_LIBS+=" ${libkerat_LIBS}"
	MUSE_RECOGNIZERS_CFLAGS+=" ${libkerat_CFLAGS}"
], [
	AC_MSG_FAILURE([Libkerat is required to build this library!])
])

# dtuio is extended communication channel
PKG_CHECK_MODULES([libdtuio], [libdtuio], [
	MUSE_RECOGNIZERS_LIBS+=" ${libdtuio_LIBS}"
	MUSE_RECOGNIZERS_CFLAGS+=" ${libdtuio_CFLAGS}"
], [
	AC_MSG_FAILURE([dTUIO is required to build the muse core library!])
])

AC_CHECK_HEADER(tinyxml.h, FOUND_TINYXML_H=yes, FOUND_TINYXML_H=no)
# once again, hacking around the stupid way c++ generates symbols
AC_CHECK_LIB(tinyxml, [_ZN13TiXmlDocument10ToDocumentEv], FOUND_TINYXML_L=yes, FOUND_TINYXML_L=no)

if test x$FOUND_TINYXML_H = xno -o x$FOUND_TINYXML_L = xno ; then
	AC_MSG_FAILURE([TinyXML is required to build this library!])
else
	AC_DEFINE([HAVE_TINYXML_H], [1], [Build with support for TinyXML])
	MUSE_RECOGNIZERS_LIBS+=" -ltinyxml"
fi

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
DX_INIT_DOXYGEN([muse_recognizers], [Doxyfile], [doc])

AC_SUBST(MUSE_RECOGNIZERS_CFLAGS)
AC_SUBST(MUSE_RECOGNIZERS_LIBS)

AC_CONFIG_FILES([Makefile tools/Makefile libmuse_recognizers.pc Doxyfile])
AC_OUTPUT


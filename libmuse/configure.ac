AC_PREREQ([2.66])
AC_INIT([libmuse], [0.0.1], [xrucka@fi.muni.cz], [libmuse], [https://sitola.cz/hci/index.php/Muse])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src])
AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([config.h])

LIBMUSE_LIBRARY_VERSION=0:1:0

LIBMUSE_API_VERSION=0.1
AC_SUBST(LIBMUSE_API_VERSION)
AC_SUBST(LIBMUSE_LIBRARY_VERSION)

LT_PREREQ([2.2])
LT_INIT

AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

AC_LANG([C++])

LIBMUSE_CFLAGS= 
LIBMUSE_LIBS= 

###################################### here the checks start

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

LIBMUSE_CFLAGS+=$CHECK_CFLAGS

CFLAGS="$saved_cflags"

# libkerat is used as base communication channel
PKG_CHECK_MODULES([libkerat], [libkerat], [
	LIBMUSE_LIBS+=" ${libkerat_LIBS}"
	LIBMUSE_CFLAGS+=" ${libkerat_CFLAGS}"
], [
	AC_MSG_FAILURE([Libkerat is required to build the muse core library!])
])

# dtuio is extended communication channel
PKG_CHECK_MODULES([libdtuio], [libdtuio], [
	LIBMUSE_LIBS+=" ${libdtuio_LIBS}"
	LIBMUSE_CFLAGS+=" ${libdtuio_CFLAGS}"
], [
	AC_MSG_FAILURE([dTUIO is required to build the muse core library!])
])

# libreco (muse recognizers) is the default gesture parsing library
PKG_CHECK_MODULES([libmuse_recognizers], [libmuse_recognizers], [
	LIBMUSE_LIBS+=" ${libmuse_recognizers_LIBS}"
	LIBMUSE_CFLAGS+=" ${libmuse_recognizers_CFLAGS}"
], [
	AC_MSG_FAILURE([libreco (MUSE recognizers) is required to build the muse framework core library!])
])

AC_CHECK_HEADER(tinyxml.h, FOUND_TINYXML_H=yes, FOUND_TINYXML_H=no)
# once again, hacking around the stupid way c++ generates symbols
AC_CHECK_LIB(tinyxml, [_ZN13TiXmlDocument10ToDocumentEv], FOUND_TINYXML_L=yes, FOUND_TINYXML_L=no)

if test x$FOUND_TINYXML_H = xno -o x$FOUND_TINYXML_L = xno ; then
    AC_MSG_FAILURE([TinyXML is required to build the muse core library!])
else
    AC_DEFINE([HAVE_TINYXML_H], [1], [Build with support for TinyXML])
    LIBMUSE_LIBS+=" -ltinyxml"
fi

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

DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN([libmuse], [Doxyfile], [doc])

AC_SUBST(LIBMUSE_CFLAGS)
AC_SUBST(LIBMUSE_LIBS)

AC_CONFIG_FILES([Makefile tools/Makefile tests/Makefile libmuse.pc Doxyfile])
AC_OUTPUT

AC_MSG_RESULT([
Configuration summary:

  Target ...................... $host
  Prefix ...................... $prefix

  Doxygen ..................... $DX_FLAG_doc
])


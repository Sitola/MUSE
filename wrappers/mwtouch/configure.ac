AC_PREREQ([2.66])
AC_INIT([mwtouch], [0.9.0.0], [359687@mail.muni.cz], [mwtouch], [https://sitola.cz/hci/index.php/mwtouch])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/wrapper.cpp])
AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([config.h])

MWTOUCH_MAJOR_VERSION=0
MWTOUCH_MINOR_VERSION=9
MWTOUCH_MICRO_VERSION=0
MWTOUCH_REVISION_VERSION=2

LT_PREREQ([2.2])
LT_INIT

AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_LANG([C++])
#AC_CXX_COMPILE_STDCXX_0X
AC_LANG_SAVE
AC_PROG_INSTALL

# Checks for libraries.
PKG_CHECK_MODULES([libkerat], [libkerat], [
	MWTOUCH_LIBS+=" ${libkerat_LIBS}"
	MWTOUCH_CFLAGS+=" ${libkerat_CFLAGS}"
], [
	AC_MSG_FAILURE([Libkerat >= 1.0 is required to build this wrapper!])
])

PKG_CHECK_MODULES([libX11], [x11], [
	MWTOUCH_CALIBRATE_LIBS+=" ${libX11_LIBS}"
	MWTOUCH_CALIBRATE_CFLAGS+=" ${libX11_CFLAGS}"
], [
	AC_MSG_FAILURE([Xlib (libX11) is required to build the calibration utility!])
])

PKG_CHECK_MODULES([xinerama], [xinerama], [
	MWTOUCH_CALIBRATE_LIBS+=" ${xinerama_LIBS}"
	MWTOUCH_CALIBRATE_CFLAGS+=" ${xinerama_CFLAGS}"
], [
	AC_MSG_FAILURE([Xinerama is required to build the calibration utility!])
])

PKG_CHECK_MODULES([xrandr], [xrandr], [
	MWTOUCH_CALIBRATE_LIBS+=" ${xrandr_LIBS}"
	MWTOUCH_CALIBRATE_CFLAGS+=" ${xrandr_CFLAGS}"
], [
	AC_MSG_FAILURE([XRandr is required to build the calibration utility!])
])

AC_CHECK_HEADER(tinyxml.h, FOUND_TINYXML_H=yes, FOUND_TINYXML_H=no)
# once again, hacking around the stupid way c++ generates symbols
AC_CHECK_LIB(tinyxml, [_ZN13TiXmlDocument10ToDocumentEv], FOUND_TINYXML_L=yes, FOUND_TINYXML_L=no)

if test x$FOUND_TINYXML_H = xno -o x$FOUND_TINYXML_L = xno ; then
    AC_MSG_FAILURE([TinyXML is required to build this wrapper!])
else
    AC_DEFINE([HAVE_TINYXML_H], [1], [Build with support for TinyXML])
    MWTOUCH_LIBS+=" -ltinyxml"
    MWTOUCH_CALIBRATE_LIBS+=" -ltinyxml"
fi

PKG_CHECK_MODULES([libdtuio], [libdtuio], [
	MWTOUCH_LIBS+=" ${libdtuio_LIBS}"
	MWTOUCH_CFLAGS+=" ${libdtuio_CFLAGS}"
	MWTOUCH_CALIBRATE_LIBS+=" ${libdtuio_LIBS}"
	MWTOUCH_CALIBRATE_CFLAGS+=" ${libdtuio_CFLAGS}"
], [
	AC_MSG_FAILURE([LibdTuio is required to build this wrapper!])
])

# Checks for header files.
AC_CHECK_HEADERS([linux/input.h], , [
	AC_MSG_FAILURE([Header file "linux/input.h" was not found in any include path. Linux headers are required to build this wrapper!])
])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library and posix functions.
AC_CHECK_FUNCS([clock_gettime memset sqrt fcntl read open pipe])

LIB_CLOCK_GETTIME=
  AC_SEARCH_LIBS(clock_gettime, [rt posix4])
  case "$ac_cv_search_clock_gettime" in
    -l*) LIB_CLOCK_GETTIME=$ac_cv_search_clock_gettime;;
  esac
AC_SUBST(LIB_CLOCK_GETTIME)

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

# check for c++0x, since some systems keep yelling
CFLAGS=-std=c++0x
AC_MSG_CHECKING([whether $CC supports $CFLAGS])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
    [ AC_MSG_RESULT([yes]) ]
    [ CHECK_CFLAGS+=" $CFLAGS" ],
    [ AC_MSG_RESULT([no])

# check for c++11, since some systems keep yelling
CFLAGS=-std=c++11
AC_MSG_CHECKING([whether $CC supports $CFLAGS])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
    [ AC_MSG_RESULT([yes]) ]
    [ CHECK_CFLAGS+=" $CFLAGS" ],
    [ AC_MSG_RESULT([no]) ]
)


 ]
)

MWTOUCH_CFLAGS+=" ${CHECK_CFLAGS}"
MWTOUCH_CALIBRATE_CFLAGS+=" ${CHECK_CFLAGS}"

CFLAGS="$saved_cflags"

AC_SUBST(MWTOUCH_CFLAGS)
AC_SUBST(MWTOUCH_LIBS)
AC_SUBST(MWTOUCH_CALIBRATE_CFLAGS)
AC_SUBST(MWTOUCH_CALIBRATE_LIBS)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

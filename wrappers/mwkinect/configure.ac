AC_PREREQ([2.66])
AC_INIT([mwkinect], [0.0.0.5], [359687@mail.muni.cz], [mwkinect], [https://sitola.cz/hci/index.php/mwkinect])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/wrapper.cpp])
AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([config.h])

MWKINECT_MAJOR_VERSION=0
MWKINECT_MINOR_VERSION=0
MWKINECT_MICRO_VERSION=0
MWKINECT_REVISION_VERSION=1

LT_PREREQ([2.2])
LT_INIT

AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_LANG([C++])
AC_LANG_SAVE
AC_PROG_INSTALL

# Checks for libraries.
PKG_CHECK_MODULES([libkerat], [libkerat], [
	MWKINECT_LIBS+=" ${libkerat_LIBS}"
	MWKINECT_CFLAGS+=" ${libkerat_CFLAGS}"
], [
	AC_MSG_FAILURE([Libkerat is required to build this wrapper!])
])

PKG_CHECK_MODULES([libdtuio], [libdtuio], [
	MWKINECT_LIBS+=" ${libdtuio_LIBS}"
	MWKINECT_CFLAGS+=" ${libdtuio_CFLAGS}"
], [
	AC_MSG_FAILURE([LibdTuio is required to build this wrapper!])
])

AC_CHECK_HEADER(tinyxml.h, FOUND_TINYXML_H=yes, FOUND_TINYXML_H=no)
# once again, hacking around the stupid way c++ generates symbols
AC_CHECK_LIB(tinyxml, [_ZN13TiXmlDocument10ToDocumentEv], FOUND_TINYXML_L=yes, FOUND_TINYXML_L=no)

if test x$FOUND_TINYXML_H = xno -o x$FOUND_TINYXML_L = xno ; then
    AC_MSG_FAILURE([TinyXML is required to build the muse core library!])
else
    AC_DEFINE([HAVE_TINYXML_H], [1], [Build with support for TinyXML])
    MWKINECT_LIBS+=" -ltinyxml"
fi

PKG_CHECK_MODULES([libfreenect], [libfreenect], [
	MWKINECT_LIBS+=" ${libfreenect_LIBS}"
	MWKINECT_CFLAGS+=" ${libfreenect_CFLAGS}"

], [
	AC_MSG_FAILURE([Libfreenect is required to build this wrapper!])
])


AC_CHECK_LIB([freenect], [freenect_sync_get_depth], , [
	AC_CHECK_LIB([freenect_sync], [freenect_sync_get_depth], [
		MWKINECT_LIBS+=" -lfreenect_sync "
	], [
		AC_MSG_FAILURE([Libfreenect sync functionality is required to build this
		wrapper, but was not found neither in -lfreenect nor in -lfreenect_sync!])
	], [
		${libfreenect_LIBS}
	])
])

# Checks for header files.
#AC_CHECK_HEADERS([libfreenect/libfreenect_cv.h], , [
#	AC_MSG_FAILURE([Libfreenect OpenCV functionality is required to build this wrapper!])
#])
#], [libfreenect/libfreenect.h, libfreenect.hpp])


PKG_CHECK_MODULES([opencv], [opencv], [
	MWKINECT_LIBS+=" ${opencv_LIBS}"
	MWKINECT_CFLAGS+=" ${opencv_CFLAGS}"
], [
	AC_MSG_FAILURE([OpenCV is required to build this wrapper!])
])

# todo make gui optionable
PKG_CHECK_MODULES([gtkmm], [gtkmm-3.0], [
	MWKINECT_LIBS+=" ${gtkmm_LIBS}"
	MWKINECT_CFLAGS+=" ${gtkmm_CFLAGS}"
], [
	AC_MSG_FAILURE([OpenCV is required to build this wrapper!])
])

#PKG_CHECK_MODULES([cvblob], [cvblob], [
#	MWKINECT_LIBS+=" ${cvblob_LIBS}"
#	MWKINECT_CFLAGS+=" ${cvblob_CFLAGS}"
#], [
#	AC_MSG_FAILURE([GTKmm-3.x is required to build this wrapper!])
#])

AC_CHECK_HEADER(tinyxml.h, FOUND_TINYXML_H=yes, FOUND_TINYXML_H=no)
# once again, hacking around the stupid way c++ generates symbols
AC_CHECK_LIB(tinyxml, [_ZN13TiXmlDocument10ToDocumentEv], FOUND_TINYXML_L=yes, FOUND_TINYXML_L=no)

if test x$FOUND_TINYXML_H = xno -o x$FOUND_TINYXML_L = xno ; then
    AC_MSG_FAILURE([TinyXML is required to build this wrapper!])
else
    AC_DEFINE([HAVE_TINYXML_H], [1], [Build with support for TinyXML])
fi

# whether to enable heatmap scaling
AC_ARG_ENABLE(heatmap, [  --disable-adaptive-heatmap        disable adaptive heatmap] )

if test x$enable_heatmap = x
then
    enable_heatmap=yes
fi

if test $enable_heatmap = yes
then
    AC_DEFINE([MWKINECT_ADAPTIVE_HEATMAP], [1], [Build with adaptive heatmap])
fi


# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library and posix functions.
AC_CHECK_FUNCS([clock_gettime memset sqrt ])

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

MWKINECT_CFLAGS+=" ${CHECK_CFLAGS}"

CFLAGS="$saved_cflags"

AC_SUBST(MWKINECT_CFLAGS)
AC_SUBST(MWKINECT_LIBS)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

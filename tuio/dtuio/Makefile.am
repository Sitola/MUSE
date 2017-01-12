AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS=-I m4
CFLAGS=-pedantic -Werror
CLEANFILES=

include aminclude.am

SUBDIRS = . # tools tests

lib_LTLIBRARIES = libdtuio.la
# message sources
libdtuio_la_SOURCES = src/topology_neighbour.cpp \
                    src/topology_group_member.cpp \
                    src/sensor_properties.cpp \
                    src/viewport.cpp \
                    src/gesture_identification.cpp \
                    src/dtuio_marker.cpp \
                    src/viewport_projector.cpp \
                    src/viewport_scaler.cpp \
                    src/helpers.cpp \
                    src/misc.cpp

libdtuio_la_LDFLAGS = -export-dynamic -version-info $(DTUIO_LIBRARY_VERSION) -release $(DTUIO_LIBRARY_RELEASE)
libdtuio_la_LIBADD =
libdtuio_la_CFLAGS =
libdtuio_la_CPPFLAGS = $(CHECK_CFLAGS)


library_includedir=$(includedir)/dtuio/
library_include_HEADERS = dtuio/*.hpp

dtuio_libincludedir = $(libdir)/dtuio
nodist_dtuio_libinclude_HEADERS = config.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libdtuio.pc

$(man3_MANS): doxygen-doc
doc:          doxygen-doc

all-local:

clean-local:
	rm -rf doc

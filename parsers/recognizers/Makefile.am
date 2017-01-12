AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS=-I m4
CFLAGS=-pedantic -Werror
CLEANFILES=

include aminclude.am

SUBDIRS = . tools # tests

lib_LTLIBRARIES = libmuse_recognizers.la

TESTS= test
check_PROGRAMS = test

# message sources
libmuse_recognizers_la_SOURCES = src/dollar_recognizer.cpp \
                    src/protractor.cpp \
                    src/dollar_n.cpp \
                    src/io_utils.cpp \
                    src/recognizers_auxiliary.cpp \
                    src/recognizers_utils.cpp
test_SOURCES = src/main.cpp

libmuse_recognizers_la_LDFLAGS = -export-dynamic -version-info $(MUSE_RECOGNIZERS_LIBRARY_VERSION) -release $(MUSE_RECOGNIZERS_LIBRARY_RELEASE)
libmuse_recognizers_la_LIBADD = $(MUSE_RECOGNIZERS_LIBS)
libmuse_recognizers_la_CFLAGS =
libmuse_recognizers_la_CPPFLAGS = $(CHECK_CFLAGS)

test_LDADD = libmuse_recognizers.la
test_CFLAGS = $(CHECK_CFLAGS)
test_LDFLAGS = $(MUSE_RECOGNIZERS_LIBS)
test_DEPENDENCIES = libmuse_recognizers.la


library_includedir=$(includedir)/muse/recognizers/
library_include_HEADERS = muse/recognizers/*.hpp

muse_recognizers_libincludedir = $(libdir)/muse/recognizers
nodist_muse_recognizers_libinclude_HEADERS = config.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libmuse_recognizers.pc

$(man3_MANS): doxygen-doc
doc:          doxygen-doc

all-local: #check-TESTS
	
clean-local:
	rm -rf doc

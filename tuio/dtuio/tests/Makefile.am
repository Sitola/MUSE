#AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS=-I m4
#include aminclude.am

LDADD = ../libdtuio.la # $(LDADD)
DEPENDENCIES = ../libdtuio.la
AM_LDFLAGS = $(DTUIO_LIBS)

TESTS = scaler_test
check_PROGRAMS = scaler_test

scaler_test_SOURCES = scaler_test.cpp ../src/viewport_scaler.cpp



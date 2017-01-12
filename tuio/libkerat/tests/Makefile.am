#AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS=-I m4
#include aminclude.am

TESTS= multiplexing_adaptor graph_basic parsers graph_connected_components graph_isomorphy
check_PROGRAMS = multiplexing_adaptor graph_basic parsers graph_connected_components graph_isomorphy

multiplexing_adaptor_SOURCES = multiplexing_adaptor_test.cpp
graph_basic_SOURCES = graph_basic_test.cpp
parsers_SOURCES = parsers_test.cpp
graph_connected_components_SOURCES = graph_connected_components.cpp
graph_isomorphy_SOURCES = graph_isomorphy.cpp

LDADD = ../libkerat.la # $(LDADD)
AM_LDFLAGS = $(LIBKERAT_LIBS)
DEPENDENCIES = ../libkerat.la



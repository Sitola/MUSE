AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS=-I m4
CLEANFILES=

include aminclude.am

SUBDIRS = . tools tests

lib_LTLIBRARIES = libkerat.la
# message sources
libkerat_la_SOURCES = src/message_helpers.cpp \
                      src/tuio_message_alive_associations.cpp \
                      src/tuio_message_alive.cpp \
                      src/tuio_message_bounds.cpp \
                      src/tuio_message_container_association.cpp \
                      src/tuio_message_link_association.cpp \
                      src/tuio_message_linked_list_association.cpp \
                      src/tuio_message_linked_tree_association.cpp \
                      src/tuio_message_convex_hull_geometry.cpp \
                      src/tuio_message_outer_contour_geometry.cpp \
                      src/tuio_message_inner_contour_geometry.cpp \
                      src/tuio_message_skeleton_geometry.cpp \
                      src/tuio_message_skeleton_volume_geometry.cpp \
                      src/tuio_message_area_geometry.cpp \
                      src/tuio_message_raw.cpp \
                      src/tuio_message_control.cpp \
                      src/tuio_message_data.cpp \
                      src/tuio_message_frame.cpp \
                      src/tuio_message_pointer.cpp \
                      src/tuio_message_signal.cpp \
                      src/tuio_message_symbol.cpp \
                      src/tuio_message_token.cpp \
                      src/generic_osc_message.cpp

# interface sources
libkerat_la_SOURCES += src/interfaces.cpp \
                       src/bundle.cpp \
                       src/simple_client.cpp \
                       src/simple_server.cpp

# utils sources
libkerat_la_SOURCES += src/session_manager.cpp \
                       src/frame_manager.cpp \
                       src/exceptions.cpp \
                       src/utils.cpp

# adaptors sources
libkerat_la_SOURCES += src/scaling_adaptor.cpp \
                       src/multiplexing_adaptor.cpp \
                       src/append_adaptor.cpp

# standard listeners sources
libkerat_la_SOURCES += src/forwarding_listener.cpp \
                       src/stdout_listener.cpp

libkerat_la_LDFLAGS = -export-dynamic -version-info $(KERAT_LIBRARY_VERSION) -release $(KERAT_LIBRARY_RELEASE)
libkerat_la_LIBADD = $(LIBKERAT_LIBS)
libkerat_la_CFLAGS =
libkerat_la_CPPFLAGS = $(CHECK_CFLAGS)

library_includedir=$(includedir)/kerat/
library_include_HEADERS = kerat/*.hpp

kerat_libincludedir = $(libdir)/kerat
nodist_kerat_libinclude_HEADERS = config.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libkerat.pc

doc: doxygen-doc

all-local: doc

clean-local:
	rm -rf doc

#installdirs-local:
#	for dir in "$(DESTDIR)$(htmldir)"; do \
#	  test -z "$$dir" || $(mkdir_p) "$$dir"; \
#	done

install-data-local: doc
	test -z "$(htmldir)" || $(mkdir_p) "$(DESTDIR)$(htmldir)"

	if test $(DX_CLEAN_HTML) -a -d $(DX_CLEAN_HTML) ; then \
		cd $(DX_CLEAN_HTML) ;\
		find ./ -type f -exec \
			$(INSTALL) -m 755 -D {} "$(DESTDIR)$(htmldir)/{}" \; ;\
	fi

uninstall-local:
	rm -rf "$(DESTDIR)$(htmldir)"

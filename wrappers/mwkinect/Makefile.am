AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS=-I m4

SUBDIRS = .

bin_PROGRAMS = mwkinect

mwkinect_SOURCES = src/mwkinect.cpp \
		src/wrapper.cpp \
		src/nodeconfig.cpp \
		src/kinect_device.cpp \
		src/cv_common.cpp \
		src/gtk_ui.cpp

mwkinect_CXXFLAGS = $(MWKINECT_CFLAGS)
# this linker flag is required to resolve the gtk-2 vs. gtk-3 conflict
mwkinect_LDFLAGS = -Wl,--as-needed
mwkinect_LDADD = $(MWKINECT_LIBS) $(LIB_CLOCK_GETTIME)


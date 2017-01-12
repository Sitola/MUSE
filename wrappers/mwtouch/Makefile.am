AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS=-I m4

SUBDIRS = .

bin_PROGRAMS = mwtouch mwtouch-calibrate

mwtouch_SOURCES = src/mwtouch.cpp \
                   src/wrapper.cpp \
                   src/geometry.cpp \
                   src/event_storage.cpp \
                   src/nodeconfig.cpp \
                   src/event.cpp \
                   src/axis.cpp \
                   src/eventdumper.cpp \
                   src/device.cpp 

mwtouch_CPPFLAGS = $(MWTOUCH_CFLAGS)
mwtouch_LDFLAGS = $(MWTOUCH_LIBS) $(LIB_CLOCK_GETTIME)


mwtouch_calibrate_SOURCES = src/mwtouch-calibrate.cpp \
                             src/xdevice.cpp \
                             src/device.cpp \
                             src/window.cpp \
                             src/axis.cpp \
                             src/geometry.cpp \
                             src/nodeconfig.cpp \
                             src/eventdumper.cpp

mwtouch_calibrate_CPPFLAGS = $(MWTOUCH_CALIBRATE_CFLAGS)
mwtouch_calibrate_LDFLAGS = $(MWTOUCH_CALIBRATE_LIBS)

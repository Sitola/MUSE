/**
 * @file      window.hpp
 * @brief     Provides the calibration window
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2011-05-02 14:33 UTC+2
 * @copyright BSD
 */

#ifndef MWTOUCH_WINDOW_HPP
#define	MWTOUCH_WINDOW_HPP

#include "xdevice.hpp"

#include <X11/Xlib.h>

class calibrate_window {
public:
    calibrate_window(x11_output_info info, unsigned int zone1, unsigned int zone2, unsigned int zone3);
    void draw_progress(float current);
    void draw_target_by_index(int index);

    Display * get_display() const;
    Window get_window() const;
    void flush() const;

    void clear();

    static const short TARGET_OFFSET;

private:

    static const short TARGET_SIZE;

    Window window;
    Display * dpy;
    int screen;
    GC gc_green;
    GC gc_red;
    GC gc_yellow;
    GC gc_white;
    GC gc_black;

    Visual visual;
    Colormap colormap;
    XColor green;
    XColor yellow;
    XColor red;
    XColor black;
    XColor white;

    const short width;
    const short height;

    const uint16_t progress_width;
    const uint16_t progress_height;
    const unsigned int w1;
    const unsigned int w2;
    const unsigned int w3;

    XPoint targets[4];

    int current_target_index;
    GC current_target_bg_gc;
    GC current_target_fg_gc;

    void draw_target(short x, short y);

};

#endif	// MWTOUCH_WINDOW_HPP


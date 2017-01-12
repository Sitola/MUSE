/**
 * @file      window.cpp
 * @brief     Provides the calibration window
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2011-05-02 14:33 UTC+2
 * @copyright BSD
 */

#include <endian.h>
#include <X11/Xlib.h>
#include <linux/stddef.h>
#include <string.h>
#include <iostream>
#include "window.hpp"

const short calibrate_window::TARGET_SIZE = 10;
const short calibrate_window::TARGET_OFFSET = 30;

calibrate_window::calibrate_window(x11_output_info info, unsigned int zone1, unsigned int zone2, unsigned int zone3)
    :width(info.width), height(info.height), progress_width(200), progress_height(40), w1((zone1*progress_width)/zone3),  w2(((zone2-zone1)*progress_width)/zone3),  w3(((zone3-zone2)*progress_width)/zone3)
{
    dpy = XOpenDisplay(NULL);
    screen = info.screen;
    colormap = XDefaultColormap(dpy, screen);

    targets[0].x = TARGET_OFFSET;
    targets[0].y = TARGET_OFFSET;

    targets[1].x = TARGET_OFFSET;
    targets[1].y = (short)(height - TARGET_OFFSET);

    targets[2].x = (short)(width - TARGET_OFFSET);
    targets[2].y = (short)(height - TARGET_OFFSET);

    targets[3].x = (short)(width - TARGET_OFFSET);
    targets[3].y = TARGET_OFFSET;

    // lets just make this easy
    XParseColor(dpy, colormap, "#ffffff", &white);
    XAllocColor(dpy, colormap, &white);
    XParseColor(dpy, colormap, "#000000", &black);
    XAllocColor(dpy, colormap, &black);
    XParseColor(dpy, colormap, "#ff3c3c", &red);
    XAllocColor(dpy, colormap, &red);
    XParseColor(dpy, colormap, "#5cff3c", &green);
    XAllocColor(dpy, colormap, &green);
    XParseColor(dpy, colormap, "#ffff3c", &yellow);
    XAllocColor(dpy, colormap, &yellow);

    Window root = RootWindow(dpy, screen);
    window = XCreateSimpleWindow(dpy, root, info.x, info.y, width, height, 0, 0, 0);

    gc_green = XCreateGC(dpy, window, 0, NULL);
    XSetForeground(dpy, gc_green, green.pixel);
    XSetBackground(dpy, gc_green, black.pixel);

    gc_red = XCreateGC(dpy, window, 0, NULL);
    XSetForeground(dpy, gc_red, red.pixel);
    XSetBackground(dpy, gc_red, black.pixel);

    gc_yellow = XCreateGC(dpy, window, 0, NULL);
    XSetForeground(dpy, gc_yellow, yellow.pixel);
    XSetBackground(dpy, gc_yellow, black.pixel);

    gc_white = XCreateGC(dpy, window, 0, NULL);
    XSetForeground(dpy, gc_white, white.pixel);
    XSetBackground(dpy, gc_white, black.pixel);

    gc_black = XCreateGC(dpy, window, 0, NULL);
    XSetForeground(dpy, gc_black, black.pixel);
    XSetBackground(dpy, gc_black, black.pixel);

    current_target_bg_gc = gc_black;
    current_target_fg_gc = gc_white;

    XMapWindow(dpy, window);
    XFlush(dpy);

    XEvent xev;
///*
    Atom wm_state = XInternAtom(dpy, "_NET_WM_STATE", true);
    Atom fullscreen = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", True);

    memset(&xev, 0, sizeof(xev));
    xev.type = ClientMessage;
    xev.xclient.window = window;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 1;
    xev.xclient.data.l[1] = fullscreen;
    xev.xclient.data.l[2] = 0;

    XSendEvent(dpy, root, False, SubstructureNotifyMask, &xev);
    XMoveResizeWindow(dpy, window, info.x, info.y, width, height);
    XFlush(dpy);
    std::cerr << "Drawing the calibration window to " << info.x << "x" << info.y << " (" << width << ", " << height << ")" << std::endl;

}

void calibrate_window::draw_progress(float current_decimal){

    size_t current = current_decimal * progress_width;

    int by = (height-progress_height)/2;
    int bx = (width-progress_width)/2;

    size_t c = 0;
    // w1
    if (c < current){
        int t = (w1<current)?w1:current;
        XFillRectangle(dpy, window, gc_yellow, bx, by, t, progress_height);
        current_target_bg_gc = gc_yellow;
        current_target_fg_gc = gc_black;
        c += t;
    }
    // w2
    if (c < current){
        int t = ((c+w2)<current)?w2:(current-c);
        XFillRectangle(dpy, window, gc_green, bx+c, by, t, progress_height);
        current_target_bg_gc = gc_green;
        current_target_fg_gc = gc_black;
        c += t;
    }
    // w3
    if (c < current){
        int t = ((c+w3)<current)?w3:(current-c);
        XFillRectangle(dpy, window, gc_red, bx+c, by, t, progress_height);
        current_target_bg_gc = gc_red;
        current_target_fg_gc = gc_black;
        c += t;
    }

    if (c < progress_width) {
        XClearArea(dpy, window, bx+c, by, progress_width - c, progress_height, false);
    }


    // progressbarr is drawn
}

Display * calibrate_window::get_display() const { return dpy; }

void calibrate_window::draw_target_by_index(int index){
    current_target_index = index%4;
    draw_target(targets[current_target_index].x, targets[current_target_index].y);

}

void calibrate_window::draw_target(short int x, short int y){
    XFillRectangle(dpy, window, current_target_bg_gc,
        (x-1)-TARGET_SIZE, (y-1)-TARGET_SIZE,
        2*(TARGET_SIZE+1) + 1, 2*(TARGET_SIZE+1) + 1);

    XDrawLine(dpy, window, current_target_fg_gc, (x-1)-TARGET_SIZE, y, (x-1), y);
    XDrawLine(dpy, window, current_target_fg_gc, (x+1)+TARGET_SIZE, y, (x+1), y);
    XDrawLine(dpy, window, current_target_fg_gc, x, (y-1)-TARGET_SIZE, x, (y-1));
    XDrawLine(dpy, window, current_target_fg_gc, x, (y+1)+TARGET_SIZE, x, (y+1));

}

void calibrate_window::flush() const { XFlush(dpy); }

Window calibrate_window::get_window() const { return window; }

void calibrate_window::clear() {

    // clean targets
    for (int i = 0; i < 4; i++){
        short int & x = targets[i].x;
        short int & y = targets[i].y;
        XClearArea(dpy, window, (x-1)-TARGET_SIZE, (y-1)-TARGET_SIZE, (2*TARGET_SIZE)+3, (2*TARGET_SIZE)+3, false);
    }

    // clean progressbar
    int by = (height-progress_height)/2;
    int bx = (width-progress_width)/2;
    XClearArea(dpy, window, bx, by, progress_width, progress_height, false);

    // set default target background
    current_target_bg_gc = gc_black;
    current_target_fg_gc = gc_white;


    flush();

}

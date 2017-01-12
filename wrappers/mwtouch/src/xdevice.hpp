/**
 * @file      xdevice.hpp
 * @brief     Provides functionality to detect X11 outputs
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2011-04-01 02:35 UTC+2
 * @copyright BSD
 */

#ifndef MWTOUCH_XDEVICE_HPP
#define	MWTOUCH_XDEVICE_HPP

#include <vector>
#include <string>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/Xinerama.h>

#include "device.hpp"

struct x11_output_info;

typedef std::vector<x11_output_info> x11_output_info_list;

struct x11_output_info {

// a moment ago I had to add this methods and now it works without them?! Stupid buggy G++
//    xOutputInfo():screen(0),width(0),height(0),x(0),y(0){ ; }
//    ~xOutputInfo(){ ; }

    std::string name;
    int screen;
    int width;
    int height;
    int x;
    int y;

};


x11_output_info_list detect_outputs(const char * displayname);
x11_output_info get_output_by_name(std::string name);

#endif	// MWTOUCH_DEVICE_HPP


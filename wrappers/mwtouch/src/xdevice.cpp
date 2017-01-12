/**
 * @file      xdevice.cpp
 * @brief     implements functionality to detect X11 outputs
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2011-04-01 02:35 UTC+2
 * @copyright BSD
 */

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/Xinerama.h>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "xdevice.hpp"

using std::cerr;
using std::cout;
using std::endl;

static x11_output_info_list detect_outputs_basic(Display * display);
static x11_output_info_list detect_outputs_xinerama(Display * display);
static x11_output_info_list detect_outputs_xrandr(Display * display);
static XRRModeInfo * get_mode_by_xid(XRRModeInfo * const modes, const int nmodes, RRMode modeID);

x11_output_info_list detect_outputs_xinerama(Display * display){

    x11_output_info_list resultset;

    { // check for xinerama
        int opcode, event, error;
        if (!XQueryExtension(display, "XINERAMA", &opcode, &event, &error)) {
            std::cerr << "Xinerama not available" << std::endl;
            return resultset;
        }
    }

    // asume server supports xinerama
    int screensCount = ScreenCount(display);

    for (int i = 0; i < screensCount; i++){

        Screen * scr = ScreenOfDisplay(display, i);
        if (scr != NULL){

            int monitorsCount = 0;
            XineramaScreenInfo * info = XineramaQueryScreens(display, &monitorsCount);

            for (int j = 0; j < monitorsCount; j++){
                x11_output_info monitor;
                {
                    std::stringstream sx;
                    sx << "Xinerama" << j;
                    sx >> monitor.name;
                }
                monitor.screen = info[j].screen_number;
                monitor.height = info[j].height;
                monitor.width = info[j].width;
                monitor.x = info[j].x_org;
                monitor.y = info[j].y_org;

                resultset.push_back(monitor);
            }
            XFree(info);

        }

    }

    return resultset;
}

x11_output_info_list detect_outputs(const char * displayname){

    x11_output_info_list resultset;

    Display * display = XOpenDisplay(displayname);
    if (display == NULL){
        std::cerr << "Failed to open display: " << ((displayname!=NULL)?displayname:XDisplayName(displayname)) << std::endl; // mozna hodi null
        return resultset;
    }

    resultset = detect_outputs_xrandr(display);
    {
        x11_output_info_list buff = detect_outputs_xinerama(display);
        resultset.reserve(resultset.size() + buff.size());
        resultset.insert(resultset.end(), buff.begin(), buff.end());
    }

    {
        x11_output_info_list buff = detect_outputs_basic(display);
        resultset.reserve(resultset.size() + buff.size());
        resultset.insert(resultset.end(), buff.begin(), buff.end());
    }

    XCloseDisplay(display);

    return resultset;
}

x11_output_info_list detect_outputs_xrandr(Display * display){

    x11_output_info_list resultset;

    { // check for RandR
        int opcode, event, error;
        if (!XQueryExtension(display, "RANDR", &opcode, &event, &error)) {
            std::cerr << "RandR not available" << std::endl;
            return resultset;
        }
    }

    int screensCount = ScreenCount(display);

    for (int i = 0; i < screensCount; i++){

        Screen * scr = ScreenOfDisplay(display, i);
        if (scr != NULL){

            Window root = RootWindow(display, i);
            XRRScreenResources * resources = XRRGetScreenResources(display, root);
            if (resources == NULL){
                std::cerr << "failed resource load" << std::endl; continue;
            }

            for (int j = 0; j < resources->noutput; j++){
                XRROutputInfo *output_info = XRRGetOutputInfo (display, resources, resources->outputs[j]);
                if (output_info == NULL){
                    std::cerr << "failed info load" << std::endl; continue;
                    std::cerr.flush();
                }

                if (output_info->connection != RR_Connected){ XFree(output_info); continue; }

                x11_output_info oinfo;
                oinfo.name = output_info->name;
                oinfo.screen = i;

                // detect if there is active crtc, if it is then select it and detect display position accordingly
                if ((output_info->ncrtc > 0) && (output_info->crtc != None)){
                    XRRCrtcInfo * crtc = XRRGetCrtcInfo(display, resources, output_info->crtc);

                    oinfo.height = crtc->height;
                    oinfo.width = crtc->width;
                    oinfo.x = crtc->x;
                    oinfo.y = crtc->y;

                    resultset.push_back(oinfo);

                    XFree(crtc);
                // at least detect the resolution
                } else if (output_info->nmode > 0){
                    XRRModeInfo * mode = get_mode_by_xid(resources->modes, resources->nmode, output_info->modes[output_info->npreferred-1]);

                    oinfo.height = mode->height;
                    oinfo.width = mode->width;
                    // xrandr source seems to handle position for non-existing crtc by setting the position to 0,0. Makes sense...
                    oinfo.x = 0;
                    oinfo.y = 0;

                    resultset.push_back(oinfo);

                    XFree(mode);

                } else {
                    std::cerr << "Unable to detect neither mode nor crtc" << std::endl;
                    std::cerr.flush();
                }

                XFree(output_info);
            }

            XFree(resources);



        }
    }

    return resultset;
}

x11_output_info_list detect_outputs_basic(Display * display){

    x11_output_info_list resultset;

    int screensCount = ScreenCount(display);

    for (int i = 0; i < screensCount; i++){

        Screen * scr = ScreenOfDisplay(display, i);
        if (scr != NULL){
            x11_output_info oinfo;
            oinfo.screen = XScreenNumberOfScreen(scr);
            {
                std::stringstream sx;
                sx << "RawXScreen" << oinfo.screen;
                std::getline(sx, oinfo.name);
            }
            oinfo.height = scr->height;
            oinfo.width = scr->width;
            oinfo.x = 0;
            oinfo.y = 0;

            resultset.push_back(oinfo);

        }
    }

    return resultset;
}

XRRModeInfo * get_mode_by_xid(XRRModeInfo * const modes, const int nmodes, RRMode modeID){

    XRRModeInfo * omode = NULL;

    for (int m = 0; m < nmodes; m++){
        if (modes[m].id == modeID){
            omode = &modes[m];
            m = nmodes;
        }
    }

    return omode;
}

x11_output_info get_output_by_name(std::string name){

    x11_output_info retval;
    x11_output_info_list outputs = detect_outputs(NULL);

    for (x11_output_info_list::iterator i = outputs.begin(); i != outputs.end(); i++){
        if (name.compare(i->name) == 0){
            retval = *i;
        }
    }

    return retval;
}


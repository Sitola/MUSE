/**
 * @file      eventdumper.cpp
 * @brief     Implements the "prettyprint" functionality for kernel event messages
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2011-08-23 11:19 UTC+2
 * @copyright BSD
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <linux/input.h>
#include <iostream>
#include <sstream>
#include <iosfwd>
#include <cstring>

#include "common.hpp"
#include "eventdumper.hpp"

static ev_code_t const AXES_CONSTANTS[] = { ABS_X, ABS_Y, ABS_Z, ABS_RX, ABS_RY, ABS_RZ, ABS_THROTTLE, ABS_RUDDER, ABS_WHEEL, ABS_GAS, ABS_BRAKE,
                                  ABS_HAT0X, ABS_HAT0Y, ABS_HAT1X, ABS_HAT1Y, ABS_HAT2X, ABS_HAT2Y, ABS_HAT3X, ABS_HAT3Y, ABS_PRESSURE,
                                  ABS_DISTANCE, ABS_TILT_X, ABS_TILT_Y, ABS_TOOL_WIDTH, ABS_VOLUME, ABS_MISC, ABS_MT_TOUCH_MAJOR,
                                  ABS_MT_TOUCH_MINOR, ABS_MT_WIDTH_MAJOR, ABS_MT_WIDTH_MINOR, ABS_MT_ORIENTATION, ABS_MT_POSITION_X,
                                  ABS_MT_POSITION_Y, ABS_MT_PRESSURE, ABS_MT_DISTANCE };

const char * get_event_type(int ev){
    switch (ev){
        case EV_SYN: return "EV_SYN";
        case EV_KEY: return "EV_KEY";
        case EV_REL: return "EV_REL";
        case EV_ABS: return "EV_ABS";
        case EV_MSC: return "EV_MSC";
        case EV_SW: return "EV_SW";
        case EV_LED: return "EV_LED";
        case EV_SND: return "EV_SND";
        case EV_REP: return "EV_REP";
        case EV_FF: return "EV_FF";
        case EV_PWR: return "EV_PWR";
        case EV_FF_STATUS: return "EV_FF_STATUS";
        case EV_MAX: return "EV_MAX";
        case EV_CNT: return "EV_CNT";
        default: return NULL;
    }
    return NULL;
}

const char * get_abs_ev_name(int code){
    switch (code){
        case ABS_X : return "ABS_X";
        case ABS_Y : return "ABS_Y";
        case ABS_Z : return "ABS_Z";
        case ABS_RX : return "ABS_RX";
        case ABS_RY : return "ABS_RY";
        case ABS_RZ : return "ABS_RZ";
        case ABS_THROTTLE : return "ABS_THROTTLE";
        case ABS_RUDDER : return "ABS_RUDDER";
        case ABS_WHEEL : return "ABS_WHEEL";
        case ABS_GAS : return "ABS_GAS";
        case ABS_BRAKE : return "ABS_BRAKE";
        case ABS_HAT0X : return "ABS_HAT0X";
        case ABS_HAT0Y : return "ABS_HAT0Y";
        case ABS_HAT1X : return "ABS_HAT1X";
        case ABS_HAT1Y : return "ABS_HAT1Y";
        case ABS_HAT2X : return "ABS_HAT2X";
        case ABS_HAT2Y : return "ABS_HAT2Y";
        case ABS_HAT3X : return "ABS_HAT3X";
        case ABS_HAT3Y : return "ABS_HAT3Y";
        case ABS_PRESSURE : return "ABS_PRESSURE";
        case ABS_DISTANCE : return "ABS_DISTANCE";
        case ABS_TILT_X : return "ABS_TILT_X";
        case ABS_TILT_Y : return "ABS_TILT_Y";
        case ABS_TOOL_WIDTH : return "ABS_TOOL_WIDTH";
        case ABS_VOLUME : return "ABS_VOLUME";
        case ABS_MISC : return "ABS_MISC";
        case ABS_MT_SLOT : return "ABS_MT_SLOT";
        case ABS_MT_TOUCH_MAJOR : return "ABS_MT_TOUCH_MAJOR";
        case ABS_MT_TOUCH_MINOR : return "ABS_MT_TOUCH_MINOR";
        case ABS_MT_WIDTH_MAJOR : return "ABS_MT_WIDTH_MAJOR";
        case ABS_MT_WIDTH_MINOR : return "ABS_MT_WIDTH_MINOR";
        case ABS_MT_ORIENTATION : return "ABS_MT_ORIENTATION";
        case ABS_MT_POSITION_X : return "ABS_MT_POSITION_X";
        case ABS_MT_POSITION_Y : return "ABS_MT_POSITION_Y";
        case ABS_MT_TOOL_TYPE : return "ABS_MT_TOOL_TYPE";
        case ABS_MT_BLOB_ID : return "ABS_MT_BLOB_ID";
        case ABS_MT_TRACKING_ID : return "ABS_MT_TRACKING_ID";
        case ABS_MT_PRESSURE : return "ABS_MT_PRESSURE";
#ifdef ABS_MT_DISTANCE
        case ABS_MT_DISTANCE : return "ABS_MT_DISTANCE";
#endif
        case ABS_MAX : return "ABS_MAX";
        case ABS_CNT : return "ABS_CNT";
        default: return NULL;
    }
     return NULL;
}

const char * get_abs_ev_name_short(int code){
    char * retval = (char *)get_abs_ev_name(code);
    if (retval != NULL){
        retval += 4;
    }
    return retval;
}

const char * get_syn_ev_name(int code){
    switch (code){
        case SYN_REPORT: return "SYN_REPORT";
        case SYN_CONFIG: return "SYN_CONFIG";
        case SYN_MT_REPORT: return "SYN_MT_REPORT";
        case SYN_DROPPED: return "SYN_DROPPED";
        default: return NULL;
    }
     return NULL;
}
const char * get_misc_ev_name(int code){
    switch (code){
        case MSC_SERIAL : return "MSC_SERIAL";
        case MSC_PULSELED : return "MSC_PULSELED";
        case MSC_GESTURE : return "MSC_GESTURE";
        case MSC_RAW : return "MSC_RAW";
        case MSC_SCAN : return "MSC_SCAN";
        case MSC_MAX : return "MSC_MAX";
        case MSC_CNT : return "MSC_CNT";
        default: return NULL;
    }
     return NULL;
}

const char * get_btn_ev_name(int code){
    switch (code){
//        case BTN_DIGI: return "BTN_DIGI";
        case BTN_TOOL_PEN: return "BTN_TOOL_PEN";
        case BTN_TOOL_RUBBER: return "BTN_TOOL_RUBBER";
        case BTN_TOOL_BRUSH: return "BTN_TOOL_BRUSH";
        case BTN_TOOL_PENCIL: return "BTN_TOOL_PENCIL";
        case BTN_TOOL_AIRBRUSH: return "BTN_TOOL_AIRBRUSH";
        case BTN_TOOL_FINGER: return "BTN_TOOL_FINGER";
        case BTN_TOOL_MOUSE: return "BTN_TOOL_MOUSE";
        case BTN_TOOL_LENS: return "BTN_TOOL_LENS";
        case BTN_TOUCH: return "BTN_TOUCH";
        case BTN_STYLUS: return "BTN_STYLUS";
        case BTN_STYLUS2: return "BTN_STYLUS2";
        case BTN_TOOL_DOUBLETAP: return "BTN_TOOL_DOUBLETAP";
        case BTN_TOOL_TRIPLETAP: return "BTN_TOOL_TRIPLETAP";
        case BTN_TOOL_QUADTAP: return "BTN_TOOL_QUADTAP";
        default: {
            std::stringstream sx;
            sx << code;
            std::string delta;
            sx >> delta;
            return delta.c_str(); // prasarna
        }
    }
}

const char * get_event_desc(int type, int code){
    switch (type){
        case EV_ABS: return get_abs_ev_name(code);
        case EV_MSC: return get_misc_ev_name(code);
        case EV_SYN: return get_syn_ev_name(code);
        case EV_KEY: return get_btn_ev_name(code);
        default: {
            std::stringstream sx;
            sx << code;
            std::string delta;
            sx >> delta;
            return delta.c_str(); // prasarna
        }
    }
}


void format(struct input_event data){
    std::cout << "type: " << get_event_type(data.type) << ", code: " << get_event_desc(data.type, data.code) << "/" << data.code << ", value: " << data.value << std::endl;
}


ev_code_t get_evcode_for_name(const char * name){

    ev_code_t code = MAPPING_IGNORE_CODE;
    size_t const names_count = sizeof(AXES_CONSTANTS)/sizeof(ev_code_t);

    for (size_t i = 0; i < names_count; i++){

        const char * cname = get_abs_ev_name(AXES_CONSTANTS[i]);
        if (strcmp(cname+4, name) == 0){
            return AXES_CONSTANTS[i];
        }

    }

    return code;
}

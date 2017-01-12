/**
 * @file      axis.cpp
 * @brief     Implements several methods to detect the axis mappings of the sensor
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2011-05-05 13:16 UTC+2
 * @copyright BSD
 */

#include <cstdlib>
#include <cstdio>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <cassert>
#include <sys/time.h>
#include <linux/input.h>

#include "axis.hpp"
#include "eventdumper.hpp"

inline void mwt_evdev_axis_range_init(axis_range * axis_info){ memset(axis_info, 0, sizeof(axis_range)); }
bool mwt_evdev_axis_range_delta_comparator(const axis_range & first, const axis_range & second){
    int delta_first = first.maximum - first.minimum;
    int delta_second = second.maximum - second.minimum;

    bool retval = delta_first > delta_second;
    if ((!retval) && (delta_first == delta_second)){
        retval = first.value > second.value;
    }
    return retval;
}
inline axis_range mwt_evdev_get_axis_map_entry(const axis_map::value_type & value){ return value.second; }
static axis_range mwt_evdev_get_axis_range(int device_fd, ev_code_t axis);

axis_vector mwt_evdev_detect_axes(int sensor_fd, int delta_time){

    axis_map axes;
    struct timeval last, timeout;
        last.tv_sec = 0;
        last.tv_usec = 0;

    bool running = true;
    struct input_event data;
    bzero(&data, sizeof(data));

    struct timeval current;
    while (running){
//        bool merged = false;

        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // wait for 100 ms

        fd_set monitors;
        FD_ZERO(&monitors);
        FD_SET(sensor_fd, &monitors);

        // select ready for correctly closing version, not realy needed now...
        int rval = select(sensor_fd+1, &monitors, NULL, NULL, &timeout);

        gettimeofday(&current, NULL);

        if (rval == 1){
            if (read(sensor_fd, &data, sizeof(struct input_event)) <= 0){
				std::cerr << "Unknown error has occoured reading from the device, aborting detection" << std::endl;
				running = false;
				continue;
			}
            switch (data.type){
                case EV_ABS: {
                    if (data.code == ABS_MT_TRACKING_ID){ break; }
                    
                    // if does not exist, create axis
                    if (axes.find(data.code) == axes.end()){
                        axis_range tmp;
                        mwt_evdev_axis_range_init(&tmp);
                        tmp.minimum = std::numeric_limits<__s32>::max();
                        tmp.maximum = std::numeric_limits<__s32>::min();
                        tmp.value = data.code;
                        
                        axes[data.code] = tmp;
                    }

                    // update axis
                    axis_range & tmp = axes[data.code];
                    if (tmp.maximum < data.value){ tmp.maximum = data.value; }
                    if (tmp.minimum > data.value){ tmp.minimum = data.value; }
                    
                    break;
                }
                case EV_SYN: {
                    switch (data.code){
                        case SYN_REPORT: {
                            last = current;
                            break;
                        }
                        case SYN_DROPPED: {
                            // SYN_DROPPED occured, reset the device
                            int capabilities = 0;
                            if(ioctl(sensor_fd, EVIOCGBIT(0, sizeof(capabilities)), &capabilities) < 0) {
                                continue;
                            }
                        }
                    }
                    break;
                }
                default: {
                    // seriously, are we still reading touch sensor?
                    // cerr << "Unhandled " << data.type << "/" << data.code << endl;
                }
            }
            bzero(&data, sizeof(data));
        }

        if ((last.tv_sec != 0) && (timeval_delta(current, last) > delta_time)) {
            running = false;
        }

    }

    axis_vector resultset;
    int s = axes.size();
    if (s > 0){
        resultset.resize(s);
        
        std::transform(axes.begin(), axes.end(), resultset.begin(), mwt_evdev_get_axis_map_entry);
        std::sort(resultset.begin(), resultset.end(), mwt_evdev_axis_range_delta_comparator);

        // now, drop all detected axes that have less than 0.71 delta of the greatest axis
        const int limit = (resultset[0].maximum - resultset[0].minimum)*0.71;
        int l = 0;
        while (resultset[l].maximum - resultset[l].minimum >= limit){ ++l; }
        resultset.resize(l);
    }

    return resultset;
}

axis_range mwt_evdev_get_axis_range(int device_fd, ev_code_t axis_code){

    struct input_absinfo axis_info;
    mwt_evdev_axis_range_init(&axis_info);
    
    if(ioctl(device_fd, EVIOCGABS(axis_code), &axis_info)) {
        perror(__FILE__": Failed getting axis range: ");
    }
    axis_info.value = axis_code;

    return axis_info;
}

axis_map mwt_evdev_get_supported_axes(int device_fd){
    axis_map retval;
    
    uint8_t axes_supported[ABS_MAX/8 + 1];
    ioctl(device_fd, EVIOCGBIT(EV_ABS, sizeof(axes_supported)), axes_supported);
    
    for (int i = 0; i < ABS_CNT; i++){
        if (bitfield_test_bit(i, axes_supported)){
            retval[i] = mwt_evdev_get_axis_range(device_fd, i);
        }
    }

    return retval;
}

/// SEE input_absinfo

/**
 * @file      axis.hpp
 * @brief     Provides several methods to detect the axis mappings of the sensor
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2011-05-05 13:16 UTC+2
 * @copyright BSD
 */

#ifndef MWTOUCH_AXIS_HPP
#define MWTOUCH_AXIS_HPP

#include <limits.h>
#include <vector>
#include <map>
#include <inttypes.h>

#include "common.hpp"

/**
 * Class to store data about the axis range - minimum and maximum coordinate
 */

typedef struct input_absinfo axis_range;
typedef std::map<ev_code_t, axis_range> axis_map;
typedef std::vector<axis_range> axis_vector;

/**
 * This struct stores the priority based mapping
 */
struct axis_mapping {
    axis_mapping():code(0),priority(0){ ; }
    axis_mapping(const ev_code_t ev_code, const priority_t map_priority):code(ev_code),priority(map_priority){ ; }

    ev_code_t code;
    priority_t priority;
};

typedef std::vector<axis_mapping> axis_mapping_vector;
typedef std::map<ev_code_t, axis_mapping> axis_mapping_map;

static const axis_mapping MAPPING_IGNORE(MAPPING_IGNORE_CODE, priority_t(-1));

class axis_detector;

axis_vector mwt_evdev_detect_axes(int fd, int delta_time);
axis_map mwt_evdev_get_supported_axes(int device_fd);
inline double mtw_evdev_axis_relative_value(const axis_range & range, ev_value_t value){
    double range_with_offset = range.maximum;  range_with_offset -= range.minimum;
    if (range_with_offset == 0){ return 0; }
    
    double value_with_offset = value; value_with_offset -= range.minimum;
    return (value_with_offset / range_with_offset);
}

inline bool operator==(const axis_range & first, const axis_range & second){
    return (first.flat == second.flat) && (first.fuzz == second.fuzz)
        && (first.maximum == second.maximum) && (first.minimum == second.minimum)
        && (first.resolution == second.resolution) && (first.value == second.value);
}

#endif  // MWTOUCH_AXIS_HPP

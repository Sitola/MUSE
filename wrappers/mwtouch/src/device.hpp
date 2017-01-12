/**
 * @file      device.hpp
 * @brief     Provides access and utility methods for the kernel input event layer
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2011-03-17 00:00 UTC+2
 * @copyright BSD
 */

#ifndef MWTOUCH_DEVICE_HPP
#define MWTOUCH_DEVICE_HPP

#include <list>
#include <map>
#include <string>

//#include "nodeconfig.hpp"

/**
 * @anchor ki_device This structure holds basic information about detected devices
 * @param path - path to device (eg. /dev/input/event11)
 * @param desc - device description provided by kernel
 */
typedef struct {
    std::string path;
    std::string desc;
    std::list <std::string> aliases;
    std::string sysfs_id;
} ki_device;

/**
 * Simple std::map holding the @ref ki_device ("ki_device") structure
 */
typedef std::map<std::string, ki_device> ki_device_map;

/**
 * Attempts to detect all touch interaction supporting devices _listed in /dev/input
 * @return _list of @ref ki_device ("ki_device") structs
 */
ki_device_map detect_devices();

/**
 * Make the device path given in config a valid path
 * @param path - path to be translated
 */
bool translate_device_path(std::string & path);

#endif  // MWTOUCH_DEVICE_HPP

/**
 * @file      device.hpp
 * @brief     Implements the access and utility methods for the kernel input event layer
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2011-03-17 00:06 UTC+2
 * @copyright BSD
 */

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE = 500 
#endif

#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <cstring>
#include <cassert>
#include <sys/stat.h>
#include <linux/input.h>
#include <linux/major.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <algorithm>
#include <iostream>
#include <set>

#include "device.hpp"

#define TESTBIT(which_bit, where) (((1<<(which_bit)) & (where)) == (1<<(which_bit)))

typedef std::set<std::string> indices_map;

static std::string resolve_real_path(const std::string & path){
    char * tmp_path = realpath(path.c_str(), NULL);
    assert(tmp_path != NULL);
    std::string retval(tmp_path);
    free(tmp_path);
    return retval;
}

static void get_indices(indices_map & ordering, const std::string & dir, const std::string & prefix){
    DIR * sysfs_dir = opendir(dir.c_str());
    assert (sysfs_dir != NULL);
    
    struct dirent64 * entry = NULL;
    while ((entry = readdir64(sysfs_dir)) != NULL){
        if (strncmp(prefix.c_str(), entry->d_name, prefix.length()) == 0){
            ordering.insert(entry->d_name);
        }
    }
    
    closedir(sysfs_dir);
}

static int get_index(const std::string & dir, const std::string & prefix, const std::string & match){
    indices_map ordering;
    get_indices(ordering, dir, prefix);
    
    indices_map::const_iterator pos = ordering.find(match);
    return (pos == ordering.end())?-1:std::distance(ordering.begin(), pos);
}

static std::string get_by_index(const std::string & dir, const std::string & prefix, size_t index){
    indices_map ordering;
    get_indices(ordering, dir, prefix);
    
    if (ordering.size() <= index){ return ""; }
    
    indices_map::const_iterator pos = ordering.begin();
    std::advance(pos, index);
    return *pos;
}

static bool add_sysfs_id(ki_device & dev){
    
    // first, construct subsystem event path
    std::string sysfs_event_path = "/sys/class/input";
        sysfs_event_path.append(dev.path.substr(dev.path.find_last_of('/'))); // append eventX
        sysfs_event_path = resolve_real_path(sysfs_event_path);
    // sysfs_event_path now contains the corrent full device path

    const std::string event_name(basename(sysfs_event_path.c_str()));
        
    // descend to input
    std::string sysfs_input_path = resolve_real_path(sysfs_event_path + "/device");

    // descend to device
    std::string sysfs_device_path = sysfs_input_path + "/device";
    { // check whether /device exists, otherwise use "/.."
        struct stat status;
        if (stat(sysfs_device_path.c_str(), &status) < 0){
            sysfs_device_path = sysfs_input_path + "/..";
        }
    }
    sysfs_device_path = resolve_real_path(sysfs_device_path);
    
    // get input name
    const std::string input_name(basename(sysfs_input_path.c_str()));
    
    // now, find out input index
    const int input_index = get_index(sysfs_device_path, "input", input_name);
    if (input_index < 0){
        std::cerr << "Unable to determine inputX for device " << dev.path << std::endl;
        return false;
    }
    
    const int event_index = get_index(sysfs_input_path, "event", event_name);
    if (event_index < 0){
        std::cerr << "Unable to determine eventX for device " << dev.path << std::endl;
        return false;
    }
    
    std::string final_id;
    final_id.resize(sysfs_device_path.length()+50);
    sprintf(&final_id.at(0), "%s/input[%d]/event[%d]", sysfs_device_path.c_str(), input_index, event_index);
    
    dev.sysfs_id = final_id;
    return true;
}

static int device_positioning_type(int device_fd){
    // determine device capabilities
    int capabilities = 0;
    if(ioctl(device_fd, EVIOCGBIT(0, sizeof(capabilities)), &capabilities) < 0) {
        return -1;
    }

    // there are two types of axis mapping on position devices - absolute and relative
    int mask = (1 << EV_ABS);// | (1 << EV_REL);

    return capabilities & mask;
}

static void search_device_directory(const std::string & directory, ki_device_map & detected_devices){
    errno = 0;
    DIR * dev_input_dir = opendir(directory.c_str());
    if (dev_input_dir == NULL) {
        if (errno != ENOENT) { perror(directory.c_str()); }
        return;
    }

    errno = 0;
    struct dirent64 * entry = NULL;
    while (((entry = readdir64(dev_input_dir)) != NULL) && (errno==0)){
        
        // setup errornous values
        int fd = -1;
        
        std::string full_pathname = directory + "/" + entry->d_name;
        std::string full_target_name(resolve_real_path(full_pathname));
        
        // check whether we havent't found this device already
        ki_device_map::iterator existing_entry = detected_devices.find(full_target_name);
        if (existing_entry != detected_devices.end()){
            if (full_pathname != full_target_name){
                existing_entry->second.aliases.push_back(full_pathname);
            }
            continue;
        }
        
        // perform input ABS device check
        {
            struct stat status;
            if (stat(full_target_name.c_str(), &status) != 0){
                perror(full_target_name.c_str());
                goto reset_skip;
            }

            if (!S_ISCHR(status.st_mode)){ continue; }
            if (major(status.st_rdev) != INPUT_MAJOR){ continue; }
        }
        
        // open device handle
        if ((fd = open(full_target_name.c_str(), O_RDONLY)) < 0) {
            std::cerr << full_target_name << ": open failed" << std::endl;
            goto reset_skip;
        }


        // determine device capabilities
        {
            int capabilities = 0;
            if(ioctl(fd, EVIOCGBIT(0, sizeof(capabilities)), &capabilities) < 0) {
                // sometimes a bad device is selected
                if ((errno != ENOTTY) && (errno != EINVAL)){ std::cerr << full_target_name << ": read failed" << std::endl; }
                goto skip_device;
            }

            int device_type = device_positioning_type(fd);
            if (device_type == -1){
                std::cerr << full_target_name << ": device capability read has failed!" << std::endl;
                goto skip_device;
            } else if (device_type == 0) {
                goto skip_device;
            }
        }
        
        //  device is fitting, add it
        {
            ki_device current_device;
            current_device.path = full_target_name;
            if (full_pathname != full_target_name){
                current_device.aliases.push_back(full_pathname);
            }
            
            // obtain device description string
            do {
                current_device.desc.resize(current_device.desc.size()+1024);
                if(ioctl(fd, EVIOCGNAME(current_device.desc.size()), &current_device.desc.at(0)) < 0) {
                    std::cerr << current_device.path << ": device read has failed!" << std::endl;
                }
            } while (current_device.desc.at(current_device.desc.size()-1) != 0);
            
            close(fd);

            // append the device to device list
            detected_devices[full_target_name] = current_device;
        }
        
        continue;
    skip_device:
        close(fd);
    reset_skip:
        errno = 0;
        continue;
    }
    if (errno != 0){
        perror(directory.c_str());
        return;
    }
    
    closedir(dev_input_dir);
}

ki_device_map detect_devices(){

    ki_device_map devices;
    search_device_directory("/dev/input/by-path", devices);
    search_device_directory("/dev/input/by-id", devices);
    search_device_directory("/dev/input", devices);
    
    std::for_each(devices.begin(), devices.end(), [](ki_device_map::value_type & val){ add_sysfs_id(val.second); });

    return devices;
}

bool translate_device_path(std::string & path){
    const std::string sys_prefix("/sys/");
    
    // check whether sys-mangeled path
    if (strncmp(path.c_str(), sys_prefix.c_str(), sys_prefix.length()) != 0){ return true; }
    
    size_t event_pos = path.find_last_of('/');
    size_t input_pos = path.find_last_of('/', event_pos-1);
    
    std::string mangeled_event = path.substr(event_pos+1);
    std::string mangeled_input = path.substr(input_pos+1, event_pos-1-input_pos);
    
    std::string dir_name = path.substr(0, input_pos);

    if (strncmp(mangeled_event.c_str(), "event[", 6) != 0) { return false; }
    if (strncmp(mangeled_input.c_str(), "input[", 6) != 0) { return false; }
    
    mangeled_event.erase(0, 6);
    mangeled_input.erase(0, 6);

    mangeled_event.erase(mangeled_event.length()-1);
    mangeled_input.erase(mangeled_input.length()-1);
    
    // extract indices
    char * err = NULL;
    int input_index = strtol(mangeled_input.c_str(), &err, 10);
    if (*err != 0){  return false; }
    int event_index = strtol(mangeled_event.c_str(), &err, 10);
    if (*err != 0){  return false; }
    
    std::string input_name = get_by_index(dir_name, "input", input_index);
    if (input_name.empty()){
        std::cerr << "Unable to convert mangeled sysfs path to real sysfs path! Reason: input index out of range!";
        return false;
    }
    dir_name += "/" + input_name;
    
    std::string event_name = get_by_index(dir_name, "event", event_index);
    if (event_name.empty()){
        std::cerr << "Unable to convert mangeled sysfs path to real sysfs path! Reason: event index out of range!";
        return false;
    }

    // result    
    path = "/dev/input/" + event_name;
    return true;
}
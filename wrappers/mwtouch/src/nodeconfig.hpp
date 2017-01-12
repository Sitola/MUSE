/**
 * @file      nodeconfig.hpp
 * @brief     Provides the struct that holds the wrapper configuration
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2011-03-17 19:14 UTC+2
 * @copyright BSD
 */

#ifndef MWTOUCH_NODECONFIG_HPP
#define MWTOUCH_NODECONFIG_HPP

#include <string>
#include <list>
#include <vector>
#include <algorithm>
#include <limits>
#include <inttypes.h>
#include <uuid/uuid.h>
#include <kerat/kerat.hpp>
#include <dtuio/dtuio.hpp>

#include "common.hpp"
#include "event.hpp"
#include "geometry_primitives.hpp"
#include "axis.hpp"

#define VERBOSITY_LEVEL_DUMP (1 << 0)
#define DIMMENSION_UNSET (std::numeric_limits<uint16_t>::min())

struct wrapper_config {
    struct cloner: std::unary_function<const libkerat::kerat_message *, libkerat::kerat_message *>{
        libkerat::kerat_message * operator()(const libkerat::kerat_message * original) const ;
    };
    
    struct destroyer: std::unary_function<libkerat::kerat_message *, libkerat::kerat_message *>{
        libkerat::kerat_message * operator()(libkerat::kerat_message * original) const ;
    };
    
    wrapper_config();
    wrapper_config(const wrapper_config & original);
    virtual ~wrapper_config();
    
    wrapper_config & operator=(const wrapper_config & original);
    
    /**
     * The x coordinate of the display/sensor pair
     */
    float x;

    /**
     * The y coordinate of the display/sensor pair
     */
    float y;

    /**
     * Coordinate of the top left pixel on sensor
     */
    geometry_point top_left;

    /**
     * Coordinate of the bottom left pixel on sensor
     */
    geometry_point bottom_left;

    /**
     * Coordinate of the bottom right pixel on sensor
     */
    geometry_point bottom_right;

    /**
     * Coordinate of the top right pixel on sensor
     */
    geometry_point top_right;

    /**
     * Width of the area that the sensor covers (in pixels)
     */
    uint16_t virtual_sensor_width;

    /**
     * Height of the area that the sensor covers (in pixels)
     */
    uint16_t virtual_sensor_height;

    /**
     * Target url (address:port) to send events to
     */
    std::string target_addr;

    /**
     * the application identifier
     */
    std::string app_name;

    /**
     * Source address (IPv4)
     */
    uint32_t local_ip;

    /**
     * Instance identifier
     */
    uint32_t instance;

    /**
     * Axis mappings
     */
     axis_mapping_map axes_mappings;
     
     /**
      * Axes ranges
      */
     axis_map axes_ranges;

     /**
      * Whether to disable coordinate transformation
      */
     bool disable_transformations;
     
     /**
      * Maximum distance for MT_TYPE_A device contacts to be joined
      */
     unsigned int join_distance_limit;
     
     typedef std::list<libkerat::kerat_message *> dtuio_list;
     dtuio_list prepared_dtuio;
};

struct node_config: public wrapper_config{
    
    node_config();

    /**
     * The path to device/file
     */
    std::string device_path;
    
    /**
     * Whether to store data to file or not
     */
    std::string store_path;
    
    struct timeval delay;
    
    unsigned int verbosity;
    
    std::string config_path;
    uuid_t uuid;
    
    std::string pidfile_name;
    bool disable_pidfile;

};

bool load_config_file(node_config & config);
bool complete_config(node_config & config);
void write_config(const node_config & config, int ofd);

#endif // MWTOUCH_NODECONFIG_HPP


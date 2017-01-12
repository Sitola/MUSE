/**
 * \file      misc.cpp
 * \brief     Implement the funtionality that didn't fit anywhere else
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-11-24 23:45 UTC+1
 * \copyright BSD
 */

#ifdef HAVE_CONFIG_H
    #include "../config.h"
#endif

#include <kerat/kerat.hpp>
#include <kerat/parsers.hpp>
#include <dtuio/parsers.hpp>
#include <dtuio/utils.hpp>
#include <dtuio/topology.hpp>
#include <dtuio/gesture_identification.hpp>
#include <dtuio/sensor_properties.hpp>
#include <dtuio/viewport.hpp>

namespace dtuio {
    
    namespace utils {
        libkerat::helpers::point_3d spherical_to_cartesian(const libkerat::angle_t azimuth, const libkerat::angle_t altitude, const libkerat::distance_t distance){
            libkerat::coord_t tmp_x = cos(azimuth)*cos(altitude)*distance;
            libkerat::coord_t tmp_y = sin(azimuth)*cos(altitude)*distance;
            libkerat::coord_t tmp_z = sin(altitude)*distance;
            
            return libkerat::helpers::point_3d(tmp_x, tmp_y, tmp_z);
        }
        
        void cartesian_to_spherical(const libkerat::helpers::point_3d & original, libkerat::angle_t & azimuth, libkerat::angle_t & altitude, libkerat::distance_t & distance){
            double px = original.get_x();
            double py = original.get_y();
            double pz = original.get_z();
            double dist = sqrt(px*px + py*py + pz*pz);
            
            distance = dist;
            if (dist > 0){ // prevent nan for point 0,0,0
                altitude = asin(pz/distance);
                azimuth = acos(px/(distance * cos(altitude)));
            }        
        }
    } // ns utils
    
    namespace internals {
        libkerat::internals::convertor_list get_dtuio_convertors() {
            using namespace dtuio::internals::parsers;
            using libkerat::internals::message_convertor_entry;
            libkerat::internals::convertor_list retval;
            
            retval.push_back(message_convertor_entry(dtuio::sensor_topology::group_member::PATH, &parse_topology_grm));
            retval.push_back(message_convertor_entry(dtuio::sensor_topology::neighbour::PATH, &parse_topology_nbr));
            retval.push_back(message_convertor_entry(dtuio::sensor::sensor_properties::PATH, &parse_sensor));
            retval.push_back(message_convertor_entry(dtuio::sensor::viewport::PATH, &parse_viewport));
            retval.push_back(message_convertor_entry(dtuio::gesture::gesture_identification::PATH, &parse_gesture_identification));
            
            return retval;
        }
    } // ns internals
} // ns dtuio
        

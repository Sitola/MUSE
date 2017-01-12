/**
 * \file      utils.hpp
 * \brief     Utiliti functions related to dTuio functionality
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2013-03-24 11:40 UTC+1
 * \copyright BSD
 */

#ifndef DTUIO_UTILS_HPP
#define DTUIO_UTILS_HPP

#include <kerat/kerat.hpp>

namespace dtuio {
    namespace utils {
        //! \brief Converts point given in spherical coordinate system used by dtuio to cartesian coordinate system
        libkerat::helpers::point_3d spherical_to_cartesian(const libkerat::angle_t azimuth, const libkerat::angle_t altitude, const libkerat::distance_t distance);
        
        //! \brief Converts point given in cartesian coordinate system to spherical coordinate system
        void cartesian_to_spherical(const libkerat::helpers::point_3d & original, libkerat::angle_t & azimuth, libkerat::angle_t & altitude, libkerat::distance_t & distance);
    } // ns utils
}

#endif // DTUIO_UTILS_HPP

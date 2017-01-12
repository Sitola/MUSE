/**
 * \file      polygonal_container_core.hpp
 * \brief     Provides the core for all matchers based on polygons
 * \author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-09-11 16:42 UTC+2
 * \copyright BSD
 */

#ifndef MUSE_POLYGONAL_CONTAINER_CORE_HPP
#define MUSE_POLYGONAL_CONTAINER_CORE_HPP

#include <kerat/kerat.hpp>
#include <map>
#include <list>

namespace muse {
    namespace internals {
        int inside_convex_polygonal_container(const std::list<libkerat::helpers::point_2d> & container, const libkerat::helpers::point_2d & point);
    }
}


#endif // MUSE_POLYGONAL_CONTAINER_CORE_HPP

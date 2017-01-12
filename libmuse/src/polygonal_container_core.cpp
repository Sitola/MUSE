/**
 * \file      polygonal_container_core.cpp
 * \brief     Provides the core for all matchers based on polygons
 * \author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-09-11 16:43 UTC+2
 * \copyright BSD
 */

#include <kerat/kerat.hpp>
#include <iostream>
#include "polygonal_container_core.hpp"

namespace muse {
    namespace internals {

        int inside_convex_polygonal_container(const std::list<libkerat::helpers::point_2d>& container, const libkerat::helpers::point_2d& point){

            if (container.size() <= 2){
                std::cerr << "MUSE:polygonal_container: Container must have at least 3 points!" << std::endl;
                return -1;
            }
            
            using libkerat::helpers::point_2d;
            typedef std::list<libkerat::helpers::point_2d> point_2d_list;
            
            int correction = 0;
            for (point_2d_list::const_iterator first_point = container.begin(); first_point != container.end(); ++first_point){
                // initialize variables
                point_2d_list::const_iterator second_point = first_point;
                ++second_point;
                if (second_point == container.end()){ second_point = container.begin(); }

                point_2d_list::const_iterator third_point = second_point;
                ++third_point;
                if (third_point == container.end()){ third_point = container.begin(); }
                
                // compute correction & run convexness check
                {
                    libkerat::helpers::point_2d a(second_point->get_x() - first_point->get_x(), second_point->get_y() - first_point->get_y());
                    libkerat::helpers::point_2d b(third_point->get_x() - second_point->get_x(), third_point->get_y() - second_point->get_y());
                    
                    int tmp_correction = (a.get_x() * b.get_y()) - (a.get_y() * b.get_x());
                    if (tmp_correction == 0){
                        std::cerr << "MUSE:polygonal_container: Container is not minimal!" << std::endl;
                        return -2;
                    }

                    tmp_correction = (tmp_correction > 0)?1:-1;
                    if (correction == 0){ correction = tmp_correction; }
                    
                    if (correction != tmp_correction){
                        std::cerr << "MUSE:polygonal_container: Container is not convex!" << std::endl;
                        return -3;
                    }
                }
                
                // primary check
                {
                    libkerat::helpers::point_2d a(first_point->get_x() - point.get_x(), first_point->get_y() - point.get_y());
                    libkerat::helpers::point_2d b(point.get_x() - second_point->get_x(), point.get_y() - second_point->get_y());

                    int determinant = (b.get_x() * a.get_y()) - (b.get_y() * a.get_x());

                    if ((determinant * correction) < 0){ return 1; }
                }
            }
            
            return 0;
        }
    }
}

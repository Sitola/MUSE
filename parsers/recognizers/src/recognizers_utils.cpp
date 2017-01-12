/**
 * \file      recognizers_utils.cpp
 * \brief     Implementation for basic utilities used by recognizers
 * \author    Martin Luptak <374178@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-11-08 10:18 UTC+1
 * \copyright BSD
 */

#include <muse/recognizers/recognizers_utils.hpp>

namespace libreco {
    namespace rutils {
        
        rectangle::rectangle(point_2d bottom_left, point_2d top_right) : bottom_left(bottom_left), top_right(top_right) {
        }
        
        //for future use
        rectangle::~rectangle() {
        }
        
    } // ns rutils
} // ns libreco

/*
 * \file      geometry_test.cpp
 * \brief     Test the geometry transformation subsystem of the wrapper
 * \author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-02-03 23:28 UTC+1
 * \copyright BSD
 */

#include <iostream>
#include "nodeconfig.hpp"
#include "geometry.hpp"

typedef bool (*test_handle)();

bool test_one(){

    wrapper_config config;
    config.top_left = geometry_point(34, 142 );
    config.bottom_left = geometry_point(36, 1878);
    config.bottom_right = geometry_point(1943, 1856);
    config.top_right = geometry_point(1939, 149);
    config.x = 0;
    config.y = 0;
    config.width = 1680;
    config.height = 1050;

    coordmapper mapper(config);

    geometry_point mapped_point;
        mapped_point.x = 1900;
        mapped_point.y = 1820;

    geometry_point mapped_point2 = mapper.transform(mapped_point);
    std::cout << mapped_point2.x << " " << mapped_point2.y << std::endl;

    return false;
}


static test_handle testsuite[] = { test_one, NULL };

int main(){

    bool failed = false;

    for (test_handle * current_test = testsuite; *current_test != NULL; current_test++){
        failed |= (*current_test)();
    }

    return failed?1:0;

}

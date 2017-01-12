/**
 * @file      geometry.hpp
 * @brief     Provides methods for manipulating the sensor geometry
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2011-03-29 00:28 UTC+2
 * @copyright BSD
 */

#ifndef MWTOUCH_GEOMETRY_HPP
#define	MWTOUCH_GEOMETRY_HPP

#include "common.hpp"
#include "geometry_primitives.hpp"
#include "nodeconfig.hpp"
#include "xdevice.hpp"


class coordmapper;

class coordmapper {
public:
    coordmapper(const wrapper_config & config);

    geometry_point transform(const geometry_point & old) const;

private:

    static geometry_point center(const geometry_point & a, const geometry_point & b);
    geometry_point rotate_with_correction_1(const geometry_point & a) const;
    geometry_point normalize_1(const geometry_point & c) const;
    geometry_point normalize_2(const geometry_point & c) const;

    geometry_point rotate_1_correction;
    geometry_point final_correction;

    double hor_axis_cos;
    double hor_axis_sin;
    double alpha_bottom;
    double alpha_top;
    double beta_bottom;
    double beta_top;
    double vert_axis_alpha;
    double vert_axis_beta;
    double alpha_left;
    double alpha_right;
    double beta_left;
    double beta_right;

    double ref_x;
    double ref_y;

    bool inverted_y;
    bool inverted_x;


    geometry_point corection;

};

#endif	// MWTOUCH_COORDINATEMAPPER_HPP


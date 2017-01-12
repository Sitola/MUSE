/**
 * @file      geometry.hpp
 * @brief     Implements all sensor-geometry related functionality
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2011-03-29 00:28 UTC+2
 * @copyright BSD
 */

#include <cmath>
#include <iostream>

#include "geometry.hpp"

geometry_point::geometry_point()
    :x(0),y(0)
{
    ;
}

geometry_point::geometry_point(const double nx, const double ny)
    :x(nx),y(ny)
{
    ;
}

geometry_point geometry_point::operator+(const geometry_point& second) const { return geometry_point(x + second.x, y + second.y); }
geometry_point geometry_point::operator-(const geometry_point& second) const { return geometry_point(x - second.x, y - second.y); }

geometry_point & geometry_point::operator+=(const geometry_point& second){ x += second.x; y += second.y; return *this; }
geometry_point & geometry_point::operator-=(const geometry_point& second){ x -= second.x; y -= second.y; return *this; }

geometry_point linear_extrude_distance(const geometry_point A, const geometry_vect direction, const short dist){
    double t = dist;
        t*=t;
    double nx = direction.x; nx *= nx;
    double ny = direction.y; ny *= ny;
    t /= (nx + ny);
    t = sqrt(t);


    return linear_extrude(A, direction * t);
}
geometry_point linear_extrude(const geometry_point A, const geometry_vect direction){
    geometry_point retval;
    retval.x = A.x + direction.x;
    retval.y = A.y + direction.y;
    return retval;
}

geometry_vect::geometry_vect():x(0),y(0){ ; }
geometry_vect::geometry_vect(const double nx, const double ny):x(nx),y(ny){ ; }
geometry_vect::geometry_vect(const geometry_point& a, const geometry_point& b):x(b.x - a.x), y(b.y - a.y){ ; }

double geometry_vect::norm() const { double lx = x, ly = y; return sqrt((lx*lx) + (ly*ly)); }

geometry_vect geometry_vect::operator *(double factor) const { return geometry_vect(x*factor, y*factor); }
geometry_vect geometry_vect::operator /(double factor) const { return geometry_vect(x/factor, y/factor); }
geometry_vect geometry_vect::operator -(const geometry_vect & second ) const { return geometry_vect(x-second.x, y-second.y); }
geometry_vect geometry_vect::operator -() const { return geometry_vect(-x, -y); }
geometry_vect geometry_vect::operator +(const geometry_vect & second ) const { return geometry_vect(x+second.x, y+second.y); }

geometry_vect & geometry_vect::operator /=(double factor) { x /= factor; y /= factor; return *this; }
geometry_vect & geometry_vect::operator *=(double factor) { x *= factor; y *= factor; return *this; }
geometry_vect & geometry_vect::operator -=(const geometry_vect & second ) { x -= second.x; y -= second.y; return *this; }
geometry_vect & geometry_vect::operator +=(const geometry_vect & second ) { x += second.x; y += second.y; return *this; }

coordmapper::coordmapper(const wrapper_config & config)
//    :topLeftX(config.topLeftX), topLeftY(config.topLeftY),
//    bottomRightX(config.bottomRightX), bottomRightY(config.bottomRightY),
    :ref_x(config.virtual_sensor_width/2.0), ref_y(config.virtual_sensor_height/2.0)
{
    geometry_point tl = config.top_left; // topLeft
    geometry_point tr = config.top_right;
    geometry_point bl = config.bottom_left;
    geometry_point br = config.bottom_right;

    geometry_point lc = center(bl, tl); // this shall be 0
    geometry_point rc = center(br, tr);

    geometry_vect horizontal_axis(lc, rc);
    //geometry_point hoziontal_center = lc;

    hor_axis_cos = horizontal_axis.x/horizontal_axis.norm();
    hor_axis_sin = horizontal_axis.y/horizontal_axis.norm();


    rotate_1_correction = geometry_point((hor_axis_cos*lc.x) + (hor_axis_sin*lc.y), (hor_axis_cos*lc.y) - (hor_axis_sin*lc.x));

    // rotate the quadrangle and move the left center to the zero point
    tl = rotate_with_correction_1(tl);
    bl = rotate_with_correction_1(bl);
    br = rotate_with_correction_1(br);
    tr = rotate_with_correction_1(tr);

    // compute directional vectors for top and bottom
    geometry_vect t_dir(tl, tr);
    geometry_vect b_dir(bl, br);

    // from now on. left side is the reference

    // set inversions
    inverted_x = !(tl.x < tr.x); // top left is zero
    inverted_y = !(tl.y < bl.y); // top left is zero

     // tangenty prvnich hran
    alpha_bottom = b_dir.y/b_dir.x; // smernice smerovych vektoru
    alpha_top = t_dir.y/t_dir.x;    // smernice smerovych vektoru

    // koeficienty - ziskavam rovnici horni a spodni hrany ve tvaru y = alpha_x*x + beta
    beta_bottom = bl.y-(alpha_bottom*bl.x);
    beta_top = tl.y-(alpha_top*tl.x);

    // normalizuji so both bottom nad top edges are paralell with the x axis
    tl = normalize_1(tl);
    bl = normalize_1(bl);
    tr = normalize_1(tr);
    br = normalize_1(br);

    // from now on, up is realy up and down is realy down

    geometry_point bc = center(bl, br);
    geometry_point tc = center(tl, tr);

    geometry_vect vertical_axis(tc, bc);
    geometry_vect l_dir(tl, bl);
    geometry_vect r_dir(tr, br);

    vert_axis_alpha = vertical_axis.x/vertical_axis.y;
    alpha_left = l_dir.x/l_dir.y;
    alpha_right = r_dir.x/r_dir.y;

    // koeficienty - ziskavam rovnici elve a prave hrany ve tvaru x = alpha_y*y + beta
    vert_axis_beta = tc.x-(vert_axis_alpha*tc.y);
    beta_right = tr.x-(alpha_right*tr.y);
    beta_left = tl.x-(alpha_left*tl.y);

    tl = normalize_2(tl);
    bl = normalize_2(bl);
    tr = normalize_2(tr);
    br = normalize_2(br);

    final_correction = tl;
}

geometry_point coordmapper::center(const geometry_point& a, const geometry_point& b){
    return geometry_point((a.x+b.x)/2.0, (a.y+b.y)/2.0);
}


// otestovano, funguje
geometry_point coordmapper::rotate_with_correction_1(const geometry_point & a) const{
    double x = ((hor_axis_cos * a.x) + (hor_axis_sin*a.y)) - rotate_1_correction.x;
    double y = ((-hor_axis_sin)*a.x) + (hor_axis_cos*a.y) - rotate_1_correction.y;
    return geometry_point(x, y);

}

geometry_point coordmapper::normalize_1(const geometry_point& c) const{

    // diky umisteni stredu souradneho systemu do Sa je ted delta X rovno X
    double factor = 1;

    if (c.y < 0){ // bottom has positive coordinates
        factor = ((alpha_top*c.x) + beta_top);
    } else {
        factor = ((alpha_bottom*c.x) + beta_bottom);
    }
    geometry_point retval(c.x, (c.y * ref_y)/std::abs(factor));
    if (inverted_y){ retval.y = -retval.y; }

    return retval;

}

geometry_point coordmapper::normalize_2(const geometry_point& c) const{

    // diky umisteni stredu souradneho systemu do Sa je ted delta X rovno X
    double factor = 1;

    double x0 = (vert_axis_alpha*c.y) + vert_axis_beta;

    if (c.x < x0){ // right has positive coordinates
        factor = ((alpha_left*c.y) + beta_left) - x0;
    } else {
        factor = ((alpha_right*c.y) + beta_right) -x0;
    }
    geometry_point retval((((c.x - x0) * ref_x)/std::abs(factor)), c.y);
    if (inverted_x){ retval.x = -retval.x; }

    return retval;

}

geometry_point coordmapper::transform(const geometry_point & old) const{

    geometry_point tmp = normalize_2(normalize_1(rotate_with_correction_1(old)));
    tmp.x -= final_correction.x;
    tmp.y -= final_correction.y;

    return tmp;

}


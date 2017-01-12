/**
 * @file      geometry_primitives.hpp
 * @brief     Provides basic geometry structs
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2011-10-28 02:00 UTC+2
 * @copyright BSD
 */

#ifndef MWTOUCH_GEOMETRY_PRIMITIVES_HPP
#define MWTOUCH_GEOMETRY_PRIMITIVES_HPP

struct geometry_point {
    geometry_point();
    geometry_point(const double a, const double b);

    geometry_point operator+(const geometry_point & second) const;
    geometry_point & operator+=(const geometry_point & second);
    geometry_point operator-(const geometry_point & second) const;
    geometry_point & operator-=(const geometry_point & second);

    double x;
    double y;
};

struct calibration_point: public geometry_point {
    struct timeval first;
    priority_t x_priority;

    priority_t y_priority;

    ev_value_t average_x;
    int count_x;
    ev_value_t average_y;
    int count_y;

    size_t samples;
};


struct geometry_vect {
public:
    geometry_vect();
    geometry_vect(const double nx, const double ny);
    geometry_vect(const geometry_point & a, const geometry_point & b);

    double norm() const;

    geometry_vect operator*(double factor) const;
    geometry_vect operator/(double factor) const;
    geometry_vect operator+(const geometry_vect & second) const;
    geometry_vect operator-(const geometry_vect & second) const;
    geometry_vect operator-() const;
    geometry_vect & operator/=(double factor);
    geometry_vect & operator*=(double factor);
    geometry_vect & operator+=(const geometry_vect & second);
    geometry_vect & operator-=(const geometry_vect & second);

    double x;
    double y;

};

geometry_point linear_extrude_distance(const geometry_point A, const geometry_vect direction, const short dist);
geometry_point linear_extrude(const geometry_point A, const geometry_vect direction);

#endif // MWTOUCH_GEOMETRY_PRIMITIVES_HPP

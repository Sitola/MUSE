/**
 * \file      utils.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-10-04 18:51 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/utils.hpp>
#include <kerat/message_helpers.hpp>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstring>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

extern "C" {
    int libkerat_present(){ return 1; }

#ifdef DRAFT_NONCOMPLIANT
    /**
     * \brief Exists when compiled as draft non-compliant
     * This function is defined for autoconf check purposes only if the library was
     * compiled as draft non-compliant. If not so, the <tt>libkerat_compliant</tt>
     * function of exactly the same type (and oposite properties) exists.
     * \return 1 (if defined), otherwise the symbol does not exist
     */
    int libkerat_noncompliant(){ return 1; }
#else
    /**
     * \ingroup global
     * \brief Exists when compiled as draft compliant
     * This function is defined for autoconf check purposes only if the library was
     * compiled as draft compliant. If not so, the <tt>libkerat_noncompliant</tt>
     * function of exactly the same type (and oposite properties) exists.
     * \return 1 (if defined), otherwise the symbol does not exist
     */
    int libkerat_compliant(){ return 1; }
#endif
    
    int libkerat_is_compliant(){
#ifdef DRAFT_NONCOMPLIANT
        return 0;
#else
        return 1;
#endif
    }
}

bool operator<(const libkerat::timetag_t & a, const libkerat::timetag_t & b){
    return (
        (a.sec < b.sec)
        || (
            (a.sec == b.sec) && (a.frac < b.frac)
        )
    );
}

bool operator==(const libkerat::timetag_t & a, const libkerat::timetag_t & b){ 
    return (a.sec == b.sec) && (a.frac == b.frac);
}

bool operator!=(const libkerat::timetag_t & a, const libkerat::timetag_t & b){ 
    return !operator==(a, b);
}

namespace libkerat {

    timetag_t timetag_diff_abs(const timetag_t & a, const timetag_t & b){
        if (a < b){
            return timetag_sub(b, a);
        }

        return timetag_sub(a, b);
    }

    timetag_t timetag_sub(const timetag_t & a, const timetag_t & b){

        libkerat::timetag_t retval = a;
        retval.sec -= b.sec;

        if (retval.frac < b.frac) {
            retval.frac = 0xFFFFFFFF; // maxval
            retval.frac -= b.frac;
            retval.frac += a.frac;
            --retval.sec;
        } else {
            retval.frac -= b.frac;
        }

        return retval;
    }

    timetag_t timetag_add(const timetag_t & a, const timetag_t & b){

        timetag_t retval = a;
        retval.sec += b.sec;

        if (((uint32_t)0xFFFFFFFF) - retval.frac < b.frac) {
            ++retval.sec;
            retval.frac = b.frac - (((uint32_t)0xFFFFFFFF) - retval.frac);
        } else {
            retval.frac += b.frac;
        }

        return retval;
    }

    angle_t strip_angle_period(angle_t value){
        // proposition
        // hard-wired constant for 2*pi
        return (float)std::fmod((double)value, (double)6.2831853072);
    }

    tu_id_t compile_tuid(const type_id_t type_id, const user_id_t user_id){
        return (
            (((tu_id_t)(0xFFFF)) & ((tu_id_t)user_id)) << 16)
            | (((tu_id_t)(0xFFFF)) & ((tu_id_t)type_id)
        );
    }

    void decompile_tuid(const tu_id_t tu_id, type_id_t & type_id, user_id_t & user_id){
        user_id = (tu_id >> 16) & 0xFFFF;
        type_id = tu_id & 0xFFFF;
    }

    dimmensions_t compile_dimmensions(const dimmension_t width, const dimmension_t height){
        return (
            (((dimmensions_t)(0xFFFF)) & ((dimmensions_t)width)) << 16)
            | (((dimmensions_t)(0xFFFF)) & ((dimmensions_t)height)
        );
    }

    void decompile_dimmensions(const dimmensions_t dim, dimmension_t & width, dimmension_t & height){
        width = (dim >> 16) & 0xFFFF;
        height = dim & 0xFFFF;
    }

    link_ports_t compile_link_ports(const link_port_t input, const link_port_t output){
        return (
            (((link_ports_t)(0xFFFF)) & ((link_ports_t)output)) << 16)
            | (((link_ports_t)(0xFFFF)) & ((link_ports_t)input)
        );
    }

    void decompile_link_ports(const link_ports_t ports, link_port_t & input, link_port_t & output){ 
        output = (ports >> 16) & 0xFFFF;
        input = ports & 0xFFFF;
    }

    struct timespec nanotimersub(const struct timespec & a, const struct timespec & b){
        struct timespec result;
        result.tv_sec = a.tv_sec - b.tv_sec;
        if (a.tv_nsec < b.tv_nsec) {
            result.tv_nsec = (a.tv_nsec + 1000000000) - b.tv_nsec;
          --result.tv_sec;
        } else {
            result.tv_nsec = a.tv_nsec - b.tv_nsec;
        }
        return result;
    }

    struct timespec nanotimeradd(const struct timespec & a, const struct timespec & b){
        struct timespec result;
        result.tv_sec = a.tv_sec + b.tv_sec;
        result.tv_nsec = a.tv_nsec + b.tv_nsec;
        if (result.tv_nsec >= 1000000000) {
          ++result.tv_sec;
          result.tv_sec -= 1000000000;
        }
        return result;
    }

    float normalize(float value, float minimum, float maximum){
        float retval;
        if (value < minimum){
            retval = minimum;
        } else if (value > maximum){
            retval = maximum;
        } else {
            float s = (minimum/2)+(maximum/2);
            value -= s;
            value *= 2;
            value /= (maximum - minimum);
            retval = value;
        }
        return retval;
    }

    std::string ipv4_to_str(const addr_ipv4_t ip){
        char buffer[20];
        memset(buffer, 0, 20);
        sprintf(buffer, "%u.%u.%u.%u", ip >> 24, (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff);
        return buffer;
    }

    session_set extract_removed_ids(const session_set & old, const session_set & update){

        typedef std::vector<session_id_t> alives_vector;
        alives_vector removed(old.size());

        alives_vector::iterator end = std::set_difference(old.begin(), old.end(),
                                                          update.begin(), update.end(),
                                                          removed.begin());
        return libkerat::session_set(removed.begin(), end);
    }
    session_set extract_new_ids(const session_set & old, const session_set & update){
        return extract_removed_ids(update, old);
    }
    
    void rotate_around_center(helpers::point_2d& point, const helpers::point_2d& center, angle_t angle){
        // substract center to use simplyfied rotation
        point -= center;
        
        double cos_a = cos(angle);
        double sin_a = sin(angle);
        
        coord_t tmp_x = cos_a*point.get_x() - sin_a*point.get_y();
        coord_t tmp_y = sin_a*point.get_x() + cos_a*point.get_y();
        
        point.set_x(tmp_x);
        point.set_y(tmp_y);
        
        // add center to move point to its original center position
        point += center;
    }
    
    void rotate_around_center_pitch(helpers::point_3d& point, const helpers::point_3d& center, angle_t pitch){
        // substract center to use simplyfied rotation
        point -= center;
        
        double cos_pitch = cos(pitch);
        double sin_pitch = sin(pitch);
        
        coord_t tmp_x = cos_pitch*point.get_x() - sin_pitch*point.get_z();
        coord_t tmp_z = sin_pitch*point.get_x() + cos_pitch*point.get_z();
        
        point.set_x(tmp_x);
        point.set_z(tmp_z);
        
        // add center to move point to its original center position
        point += center;
    }
    
    void rotate_around_center_roll(helpers::point_3d& point, const helpers::point_3d& center, angle_t roll){
        // substract center to use simplyfied rotation
        point -= center;
        
        double cos_roll = cos(roll);
        double sin_roll = sin(roll);
        
        coord_t tmp_y = cos_roll*point.get_y() - sin_roll*point.get_z();
        coord_t tmp_z = sin_roll*point.get_y() + cos_roll*point.get_z();
        
        point.set_y(tmp_y);
        point.set_z(tmp_z);
        
        // add center to move point to its original center position
        point += center;
    }
} // ns libkerat


/**
 * \file      utils.hpp
 * \brief     Stuff that does not belong anywhere else
 * \author    Lukas Rucka <359687\mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-11-18 23:35 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_UTILS_HPP
#define KERAT_UTILS_HPP

#include <kerat/typedefs.hpp>
#include <cmath>
#include <string>
#include <set>

/**
 * \ingroup global
 * \brief Lower-than comparator for the timetag type
 * \param a - first value
 * \param b - second value
 * \return true if a comes before b
 */
bool operator<(const libkerat::timetag_t & a, const libkerat::timetag_t & b);

bool operator==(const libkerat::timetag_t & a, const libkerat::timetag_t & b);
bool operator!=(const libkerat::timetag_t & a, const libkerat::timetag_t & b);

namespace libkerat {

    // forward declarations
    namespace helpers {
        class point_2d;
        class point_3d;
    } // ns helpers

    namespace internals {
        /**
         * \brief Standard bitfield mask bittest
         * \param bitfield - bitfield to test in
         * \param value - value to test for
         * \return true if value is subset of bitfield
         */
        template <typename T> inline bool testbit(const T bitfield, const T value){ return (bitfield & value) == value; }

        //! \brief Generic callback settin used in the kerat library
        template <typename T>
        struct callback_setting {
            callback_setting():m_callback(NULL),m_user_data(NULL){ ; }
            callback_setting(T callback, void * user_data):m_callback(callback), m_user_data(user_data){ ; }

            bool operator== (const callback_setting<T> & second) const {
                return (m_callback == second.m_callback)
                    && (m_user_data == second.m_user_data);
            }

            bool operator!= (const callback_setting<T> & second) const {
                return !operator==(second);
            }

            //! \brief Function callback type
            typedef T callback_type;

            //! \brief Function, that serves as the callback.
            T m_callback;

            //! \brief User data to call the callback with
            void * m_user_data;
        };

    } // ns internals

    /**
     * \ingroup global
     * \brief Compute absolute difference between two timetags
     * \param a - first value
     * \param b - second value
     * \return if (a \< b) than (b-a) else (a-b)
     */
    timetag_t timetag_diff_abs(const timetag_t & a, const timetag_t & b);

    /**
     * \ingroup global
     * \brief Subtracts two timetags
     * \note Does not care for overflow
     * \param a - first value
     * \param b - second value
     * \return a-b
     */
    timetag_t timetag_sub(const timetag_t & a, const timetag_t & b);

    /**
     * \ingroup global
     * \brief Add up two timetags
     * \param a - first value
     * \param b - second value
     * \return a+b
     */
    timetag_t timetag_add(const timetag_t & a, const timetag_t & b);

    /**
     * \brief Normalize the value to fit the range [-1f ; 1f] using it's bounds
     * \param value - value to be normalized
     * \param minimum - value that mapps to -1 normalized
     * \param maximum - value that mapps to +1 normalized
     * \return normalized value if minimum is less than value which is less than
     * maximum or proper bound if out of bounds
     */
    float normalize(float value, float minimum, float maximum);

    //! \brief Alias to \ref normalize normalize(value, 0, maximum)
    inline float normalize(float value, float maximum){ return normalize(value, 0, maximum); }

    /**
     * \brief Strip the angle period preserving the angle orientation
     * \param value - value to strip period from
     * \return angle normalized into range [-2*PI ; 2*PI]
     */
    angle_t strip_angle_period(angle_t value);

    /**
     * \brief Compile the individual type and user id's to the single type/user id attribute
     * \param type_id - type id attribute
     * \param user_id - user id attribute
     * \return type/user attribute with two upper bytes containing user id and two lower bytes containing type id
     */
    tu_id_t compile_tuid(const type_id_t type_id, const user_id_t user_id);

    /**
     * \brief Decompile the type/user id attribute into individual type and user ids
     * \param[in] tu_id - tu_id to decompose
     * \param[out] type_id - type component of the tu_id
     * \param[out] user_id - user component of the user_id
     */
    void decompile_tuid(const tu_id_t tu_id, type_id_t & type_id, user_id_t & user_id);

    /**
     * \brief Compile the frame's dimmensions attribute from individual dimmensions
     * \param width - sensor width
     * \param height - sensor height
     * \return dimmensions attribute with two upper bytes containing the width and two lower bytes containing the height
     */
    dimmensions_t compile_dimmensions(const dimmension_t width, const dimmension_t height);

    /**
     * \brief Decompile the frame's dimmensions attribute to width and height
     * \param[in] dim - frame's dimmensions attribute
     * \param[out] width - sensor width
     * \param[out] height - sensor height
     */
    void decompile_dimmensions(const dimmensions_t dim, dimmension_t & width, dimmension_t & height);

    /**
     * \brief Compile the link id attribute from individual ports
     * \param input - the input port
     * \param output - the output port
     * \return link id with two upper bytes containing the width and two lower bytes containing the height
     */
    link_ports_t compile_link_ports(const link_port_t input, const link_port_t output);

    /**
     * \brief Decompile the link id attribute to individual ports
     * \param[in] ports - link id to be decompiled
     * \param[out] input - input port
     * \param[out] output - output port
     */
    void decompile_link_ports(const link_ports_t ports, link_port_t & input, link_port_t & output);

    /**
     * \brief Checks whether the value given fits the range [ -1.0f ; 1.0f ]
     * \param value - value to check
     * \return true if fits
     */
    inline bool is_normalized(float value){ return (value >= -1.0f) && (value <= 1.0f); }

    /**
     * \brief Checks whether the value given exceeds the range [ -1.0f ; 1.0f ]
     * \param value - value to check
     * \return true if exceeds
     */
    inline bool is_not_normalized(float value){ return !is_normalized(value); }

    //! \brief Analogue of \ref libkerat::timetag_sub for struct timespec
    struct timespec nanotimersub(const struct timespec & a, const struct timespec & b);
    //! \brief Analogue of \ref libkerat::timetag_add for struct timespec
    struct timespec nanotimeradd(const struct timespec & a, const struct timespec & b);

    /**
     * \brief Converts the numeric ip representation to string one
     * \param ip - ip address to convert
     * \return String containing the text represenation
     */
    std::string ipv4_to_str(const addr_ipv4_t ip);

    /**
     * \brief Get set of session id's that are no longer present on sensor
     *
     * \param present - set of previously present ids
     * \param update  - set of session ids containing the current state
     * \return set of removed contacts
     */
    session_set extract_removed_ids(const session_set & present, const session_set & update);

    /**
     * \brief Get set of session id's that were not previously present sensor
     *
     * \param present - set of previously present ids
     * \param update  - set of session ids containing the current state
     * \return set of new contacts
     */
    session_set extract_new_ids(const session_set & present, const session_set & update);
    
    /**
     * \brief Rotates the poing around given center by given angle (radians)
     * \param point - point to rotate
     * \param center - center to rotate around
     * \param angle - angle to rotate by
     */
    void rotate_around_center(helpers::point_2d & point, const helpers::point_2d & center, angle_t angle);

    /**
     * \brief Rotates the poing around given center in yaw axis by given angle (radians)
     * \param point - point to rotate
     * \param center - center to rotate around
     * \param yaw - yaw angle to rotate by
     * \note Alias for \ref libkerat::rotate_around_center "rotate_around_center"
     */
    inline void rotate_around_center_yaw(helpers::point_3d & point, const helpers::point_3d & center, angle_t yaw){
        rotate_around_center((helpers::point_2d &)point, (const helpers::point_2d &)center, yaw);
    }
    
    /**
     * \brief Rotates the poing around given center in pitch axis by given angle (radians)
     * \param point - point to rotate
     * \param center - center to rotate around
     * \param pitch - pitch angle to rotate by
     */
    void rotate_around_center_pitch(helpers::point_3d & point, const helpers::point_3d & center, angle_t pitch);

    /**
     * \brief Rotates the poing around given center in roll axis by given angle (radians)
     * \param point - point to rotate
     * \param center - center to rotate around
     * \param roll - roll angle to rotate by
     */
    void rotate_around_center_roll(helpers::point_3d & point, const helpers::point_3d & center, angle_t roll);

} // ns libkerat

// Autoconf and library present test symbols
extern "C" {
    /**
    * \ingroup global
    * \brief Check for kerat library presence
    *
    * This symbol is defined for autoconf so it can be used to check
    * whether is the kerat library present.
    * \return 1. Always. No exceptions.
    * Unles frogs fall from the sky and water of Egypt turn into blood.
    */
    int libkerat_present();

    /**
     * \ingroup global
     * \brief Check whether the library was compiled as draft compliant (see
     * \ref lta-ambiguity for reasons)
     * \return 1 if compiled as draft compliant, 0 otherwise
     */
    int libkerat_is_compliant();
}

#endif // KERAT_UTILS_HPP

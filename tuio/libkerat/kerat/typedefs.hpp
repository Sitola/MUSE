/**
 * \file      typedefs.hpp
 * \brief     Contains basic and common type definitions for the kerat TUIO 2.0 library.
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-09-19 13:55 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_TYPE_HPP
#define KERAT_TYPE_HPP

#include <inttypes.h>
#include <lo/lo_types.h>
#include <set>

namespace libkerat {

    // basic type aliases for tuio2

    //! \brief TUIO 2.0 session id (contact id)
    typedef uint32_t session_id_t;

    //! \brief TUIO 2.0 component id
    //! Each contact can have several components, they all share the same session
    //! id but have different component id's. This data type is used to denote
    //! signal component id as well.
    typedef uint32_t component_id_t;

    //! \brief TUIO 2.0 type id
    //! eg. type of pointer - hand, finger, arm and such
    typedef uint16_t type_id_t;

    //! \brief TUIO 2.0 user id
    typedef uint16_t user_id_t;

    //! \brief compiled user and type id data type
    //! \note Two upper bytes are user id and two lower denote type id
    //! \see libkerat::compile_tuid libkerat::decompile_tuid
    typedef uint32_t tu_id_t;

    //! \brief TUIO 2.0 frame id
    //! \note Value 0 is protocol-reserved (see \ref libkerat::message::frame::OUT_OF_ORDER_FRAME_ID "OUT_OF_ORDER_FRAME_ID")
    typedef uint32_t frame_id_t;

    //! \brief TUIO 2.0 tracker instance id
    typedef uint32_t instance_id_t;

    //! \brief compiled sensor dimmensions
    //! \note Two upper bytes are sensor width and two lower denote sensor height
    //! \see libkerat::compile_dimmensions libkerat::decompile_dimmensions
    typedef uint32_t dimmensions_t;

    //! \brief TUIO 2.0 sensor dimmension
    typedef uint16_t dimmension_t;

    //! \brief TUIO 2.0 tracker IP (MAC) address
    typedef uint32_t addr_ipv4_t;

    //! \brief TUIO 2.0 container slot
    typedef uint32_t slot_t;

    //! \brief TUIO 2.0 coordinate
    //! \note For valid range and TUIO 2.0 draft compliance, see \ref normalized-coordinates
    typedef float coord_t;

    //! \brief TUIO 2.0 angle
    //! \note For valid range and TUIO 2.0 draft compliance, see \ref normalized-rotation
    typedef float angle_t;

    //! \brief TUIO 2.0 pressure
    //! \note For valid range and TUIO 2.0 draft compliance, see \ref normalized-pressure
    typedef float pressure_t;

    //! \brief TUIO 2.0 velocity
    //! \note For valid range and TUIO 2.0 draft compliance, see \ref normalized-velocity
    typedef float velocity_t;

    //! \brief TUIO 2.0 rotation velocity
    //! \note For valid range and TUIO 2.0 draft compliance, see \ref normalized-rotation-velocity
    typedef float rotation_velocity_t;

    //! \brief TUIO 2.0 acceleration
    //! \note For valid range and TUIO 2.0 draft compliance, see \ref normalized-acceleration
    typedef float accel_t;

    //! \brief TUIO 2.0 acceleration
    //! \note For valid range and TUIO 2.0 draft compliance, see \ref normalized-acceleration
    typedef float rotation_accel_t;

    //! \brief TUIO 2.0 distance
    //! \note For valid range and TUIO 2.0 draft compliance, see \ref normalized-distance
    typedef float distance_t;

    //! \brief TUIO 2.0 volume
    typedef float volume_t;

    //! \brief TUIO 2.0 area
    typedef float area_t;

    //! \brief TUIO 2.0 link I/O port number
    typedef uint16_t link_port_t;

    //! \brief compiled link I/O port numbers
    //! \note Two upper bytes are output port number and lower denote input port number
    //! \see libkerat::compile_link_ports libkerat::decompile_link_ports
    typedef uint32_t link_ports_t;

    //! \brief Type alias for OSC timetag
    typedef lo_timetag timetag_t;

    //! \brief TUIO 2.0 radius
    typedef distance_t radius_t;

    // utility types
    //! \brief Set of session id's, used in various messages
    typedef std::set<session_id_t> session_set;

    //! \brief Libkerat's internal graph node identifier
    typedef int32_t node_id_t;
}

#endif // KERAT_TYPE_HPP

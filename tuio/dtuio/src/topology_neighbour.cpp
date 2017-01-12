/**
 * \file      topology_neighbour.cpp
 * \brief     Implement the message that identifies neighbouring sensor
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-10-28 12:00 UTC+1
 * \copyright BSD
 */

#ifdef HAVE_CONFIG_H
    #include "../config.h"
#endif

#include <dtuio/topology_neighbour.hpp>
#include <dtuio/helpers.hpp>
#include <dtuio/parsers.hpp>
#include <kerat/utils.hpp>
#include <uuid/uuid.h>

namespace dtuio {
    namespace sensor_topology {

        const char * neighbour::PATH = "/dtuio/nbr";

        neighbour::neighbour():m_azimuth(0), m_altitude(0), m_distance(0){ ; }

        neighbour::neighbour(const dtuio::uuid_t & self, libkerat::angle_t azimuth, libkerat::angle_t altitude, libkerat::distance_t distance)
            :dtuio::helpers::uuid(self),
            m_azimuth(azimuth),
            m_altitude(altitude),
            m_distance(distance)
        { ; }

        neighbour::neighbour(const dtuio::uuid_t & self, libkerat::angle_t azimuth, libkerat::angle_t altitude, libkerat::distance_t distance, const dtuio::uuid_t & neighbour)
            :dtuio::helpers::uuid(self),
            dtuio::helpers::neighbour_uuid(neighbour),
            m_azimuth(azimuth),
            m_altitude(altitude),
            m_distance(distance)
        { ; }

        neighbour::neighbour(const neighbour & original)
            :dtuio::helpers::uuid(original),
            dtuio::helpers::neighbour_uuid(original),
            m_azimuth(original.m_azimuth),
            m_altitude(original.m_altitude),
            m_distance(original.m_distance)
        { ; }

        neighbour::~neighbour(){ ; }

        void neighbour::set_altitude(libkerat::angle_t altitude){
            m_altitude = libkerat::strip_angle_period(altitude);

            const libkerat::angle_t pi = acos(-1);
            const libkerat::angle_t pipul = acos(0);

            if (m_altitude < -pi){
                m_altitude += 2*pi;
            } else if (m_altitude < -pipul){
                m_altitude += pi;
            }

            // m_altitude is now within <0, pi>
            // if m_altitude falls in <pi/2, pi> then roatate azimuth and set correct
            if (m_altitude > pipul){
                set_azimuth(libkerat::strip_angle_period(m_azimuth + pi));
                m_altitude = pi-m_altitude;
            }
            // m_altitude is now within <0, pipul>
        }

        void neighbour::set_azimuth(libkerat::angle_t azimuth){
            m_azimuth = libkerat::strip_angle_period(azimuth);
        }

        void neighbour::set_distance(libkerat::distance_t distance){
            m_distance = distance;
            if (m_distance < 0){
                m_distance = -m_distance;
                m_azimuth = libkerat::strip_angle_period(m_azimuth + acos(-1));
                m_altitude = libkerat::strip_angle_period(-m_altitude);
            }
        }

        bool neighbour::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }
            lo_message msg = lo_message_new();

            char buffer[UUID_STRING_CAPACITY];
            memset(buffer, 0, UUID_STRING_CAPACITY);
            uuid_unparse(m_uuid, buffer);
            lo_message_add_string(msg, buffer);

            lo_message_add_float(msg, m_azimuth);
            lo_message_add_float(msg, m_altitude);
            lo_message_add_float(msg, m_distance);

            memset(buffer, 0, UUID_STRING_CAPACITY);
            uuid_unparse(m_neighbour_uuid, buffer);
            lo_message_add_string(msg, buffer);

            lo_bundle_add_message(target, PATH, msg);
            return true;
        }

        libkerat::kerat_message * neighbour::clone() const { return new neighbour(*this); }

        void neighbour::print(std::ostream & output) const { output << *this; }

    }
    namespace internals { namespace parsers {
        bool parse_topology_nbr(
            libkerat::internals::convertor_output_container & results,
            const char * path,
            const char * types,
            lo_arg ** argv,
            int argc __attribute__((unused)),
            void * user_data __attribute__((unused))
        ){
            results.clear();
            // check for message type
            if (strcmp(path, dtuio::sensor_topology::neighbour::PATH) != 0){ return false; }

            // check for argument types & message lenght
            size_t len = strlen(types);
            if (!(
                ((len == 4) && (strcmp(types, "sfff") == 0))
             || ((len == 5) && (strcmp(types, "sfffs") == 0))
            )){
                return false;
            }

            dtuio::uuid_t uuid;
            // check for correct UUID
            if (uuid_parse(&(argv[0]->s), uuid) != 0){ return false; }

            dtuio::sensor_topology::neighbour * msg_nbr =
                new dtuio::sensor_topology::neighbour(uuid, argv[1]->f, argv[2]->f, argv[3]->f);

            if (len == 5){
                if (uuid_parse(&(argv[4]->s), uuid) != 0){
                    delete msg_nbr;
                    return false;
                } else {
                    msg_nbr->set_neighbour_uuid(uuid);
                }
            }

            results.push_back(msg_nbr);

            return true;
        }
    } }
}

std::ostream & operator<<(std::ostream & output, const dtuio::sensor_topology::neighbour & nbr){
    const dtuio::helpers::uuid &           u_snr = nbr;
    const dtuio::helpers::neighbour_uuid & u_nbr = nbr;

    output << dtuio::sensor_topology::neighbour::PATH
        << " " << u_snr
        << " " << nbr.get_azimuth()
        << "/" << nbr.get_altitude()
        << "/" << nbr.get_distance() << " " << u_nbr;

    return output;
}

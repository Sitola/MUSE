/**
 * \file      topology_group_member.cpp
 * \brief     Implement the message that makes the sensor join group
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-11-21 01:10 UTC+1
 * \copyright BSD
 */

#ifdef HAVE_CONFIG_H
    #include "../config.h"
#endif

#include <dtuio/topology_group_member.hpp>
#include <dtuio/helpers.hpp>
#include <dtuio/parsers.hpp>
#include <kerat/utils.hpp>
#include <uuid/uuid.h>

namespace dtuio {
    namespace sensor_topology {

        const char * group_member::PATH = "/dtuio/grm";

        group_member::group_member(
            const dtuio::uuid_t & group_id,
            const dtuio::uuid_t & sensor_id
        ):dtuio::helpers::group_uuid(group_id), dtuio::helpers::uuid(sensor_id){ ; }

        group_member::group_member(const group_member & original)
            :dtuio::helpers::group_uuid(original), dtuio::helpers::uuid(original)
        { ; }

        group_member::~group_member(){ ; }

        libkerat::kerat_message * group_member::clone() const { return new group_member(*this); }

        void group_member::print(std::ostream & output) const { output << *this; }

        bool group_member::imprint_lo_messages(lo_bundle target) const {

            if (target == NULL){ return false; }
            lo_message msg = lo_message_new();

            char buffer[UUID_STRING_CAPACITY];
            memset(buffer, 0, UUID_STRING_CAPACITY);
            uuid_unparse(m_group_uuid, buffer);
            lo_message_add_string(msg, buffer);

            memset(buffer, 0, UUID_STRING_CAPACITY);
            uuid_unparse(m_uuid, buffer);
            lo_message_add_string(msg, buffer);

            lo_bundle_add_message(target, PATH, msg);
            return true;

        }
    }

    namespace internals { namespace parsers {
        bool parse_topology_grm(
            libkerat::internals::convertor_output_container & results,
            const char * path,
            const char * types,
            lo_arg ** argv,
            int argc __attribute__((unused)),
            void * user_data __attribute__((unused))
        ){
            results.clear();
            // check for message type
            if (strcmp(path, dtuio::sensor_topology::group_member::PATH) != 0){ return false; }

            // check for argument types
            if (strcmp(types, "ss") != 0){ return false; }

            dtuio::uuid_t group_uuid;
            if (uuid_parse(&(argv[0]->s), group_uuid) != 0){ return false; }

            dtuio::uuid_t sensor_uuid;
            if (uuid_parse(&(argv[1]->s), sensor_uuid) != 0){ return false; }

            dtuio::sensor_topology::group_member * msg_grm =
                new dtuio::sensor_topology::group_member(group_uuid, sensor_uuid);
            results.push_back(msg_grm);

            return true;
        }
    } }
}

std::ostream & operator<<(std::ostream & output, const dtuio::sensor_topology::group_member & grm){
    output << dtuio::sensor_topology::group_member::PATH
        << " " << (dtuio::helpers::group_uuid)grm
        << " " << (dtuio::helpers::uuid)grm;
    return output;
}

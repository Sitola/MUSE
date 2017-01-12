/**
 * \file      sensor_properties.cpp
 * \brief     Implement the message that identifies the sensor
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-11-03 15:51 UTC+1
 * \copyright BSD
 */

#ifdef HAVE_CONFIG_H
    #include "../config.h"
#endif

#include <dtuio/sensor_properties.hpp>
#include <dtuio/helpers.hpp>
#include <dtuio/parsers.hpp>
#include <kerat/utils.hpp>
#include <uuid/uuid.h>

namespace dtuio {
    namespace sensor {

        const char * sensor_properties::PATH = "/dtuio/snr";

        sensor_properties::sensor_properties():m_coordinate_translation(COORDINATE_INTACT), m_purpose(PURPOSE_EVENT_SOURCE){ ; }

        sensor_properties::sensor_properties(
            const sensor_properties::coordinate_translation_mode_t & mode,
            const sensor_properties::sensor_purpose_t & purpose
        ):m_coordinate_translation(mode), m_purpose(purpose){ ; }

        sensor_properties::sensor_properties(
            const dtuio::uuid_t & id,
            const sensor_properties::coordinate_translation_mode_t & mode,
            const sensor_properties::sensor_purpose_t & purpose
        ):dtuio::helpers::uuid(id), m_coordinate_translation(mode), m_purpose(purpose){ ; }

        sensor_properties::sensor_properties(const sensor_properties & original)
            :dtuio::helpers::uuid(original),
            m_coordinate_translation(original.m_coordinate_translation),
            m_purpose(original.m_purpose)
        { ; }

        sensor_properties::~sensor_properties(){ ; }

        void sensor_properties::set_coordinate_translation_mode(const sensor_properties::coordinate_translation_mode_t & mode){
            m_coordinate_translation = mode;
        }

        void sensor_properties::set_sensor_purpose(const sensor_properties::sensor_purpose_t& purpose){
            m_purpose = purpose;
        }

        libkerat::kerat_message * sensor_properties::clone() const { return new sensor_properties(*this); }

        void sensor_properties::print(std::ostream & output) const { output << *this; }

        bool sensor_properties::imprint_lo_messages(lo_bundle target) const {

            if (target == NULL){ return false; }
            lo_message msg = lo_message_new();

            char buffer[UUID_STRING_CAPACITY];
            memset(buffer, 0, UUID_STRING_CAPACITY);
            uuid_unparse(m_uuid, buffer);
            lo_message_add_string(msg, buffer);

            uint32_t tmp1 = 0;
            uint32_t tmp2 = 0;
            tmp1 = m_coordinate_translation;
            tmp2 = m_purpose;
            tmp1 <<= 16;
            tmp1 |= tmp2;
            lo_message_add_int32(msg, tmp1);

            lo_bundle_add_message(target, PATH, msg);
            return true;

        }
    }

    namespace internals { namespace parsers {
        bool parse_sensor(
            libkerat::internals::convertor_output_container & results,
            const char * path,
            const char * types,
            lo_arg ** argv,
            int argc __attribute__((unused)),
            void * user_data __attribute__((unused))
        ){
            results.clear();
            // check for message type
            if (strcmp(path, dtuio::sensor::sensor_properties::PATH) != 0){ return false; }

            // check for argument types
            if (strcmp(types, "si") != 0){ return false; }

            dtuio::uuid_t uuid;
            // check for correct UUID
            if (uuid_parse(&(argv[0]->s), uuid) != 0){ return false; }

            dtuio::sensor::sensor_properties::coordinate_translation_mode_t mode
                = static_cast<dtuio::sensor::sensor_properties::coordinate_translation_mode_t>(argv[1]->i >> 16);
            dtuio::sensor::sensor_properties::sensor_purpose_t purpose
                = static_cast<dtuio::sensor::sensor_properties::sensor_purpose_t>(argv[1]->i & 0x0000FFFF);

            dtuio::sensor::sensor_properties * msg_snr =
                new dtuio::sensor::sensor_properties(uuid, mode, purpose);
            results.push_back(msg_snr);

            return true;
        }
    } }
}

std::ostream & operator<<(std::ostream & output, const dtuio::sensor::sensor_properties & snr){
    output << dtuio::sensor::sensor_properties::PATH
        << " " << (dtuio::helpers::uuid)snr
        << " ";


    const char * coordinate_mode = "unrecognized";
    switch (snr.get_coordinate_translation_mode()){
        case dtuio::sensor::sensor_properties::COORDINATE_INTACT:
            coordinate_mode = "intact"; break;
        case dtuio::sensor::sensor_properties::COORDINATE_TRANSLATE_SETUP_ONCE:
            coordinate_mode = "setup_once"; break;
        case dtuio::sensor::sensor_properties::COORDINATE_TRANSLATE_SETUP_CONTINUOUS:
            coordinate_mode = "setup_continuous"; break;
    }

    output << coordinate_mode << " ";

    const char * sensor_purpose = "unknown";
    switch (snr.get_sensor_purpose()){
        case dtuio::sensor::sensor_properties::PURPOSE_OBSERVER:
            sensor_purpose = "observer"; break;
        case dtuio::sensor::sensor_properties::PURPOSE_EVENT_SOURCE:
            sensor_purpose = "event_source"; break;
        case dtuio::sensor::sensor_properties::PURPOSE_TAGGER:
            sensor_purpose = "tagger"; break;
    }

    output << sensor_purpose;

    return output;
}

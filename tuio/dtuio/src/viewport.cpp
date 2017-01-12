/**
 * \file      viewport.cpp
 * \brief     Implement the message that describes the sensor viewport
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2013-03-24 13:00 UTC+1
 * \copyright BSD
 */

#ifdef HAVE_CONFIG_H
    #include "../config.h"
#endif

#include <dtuio/viewport.hpp>
#include <dtuio/helpers.hpp>
#include <dtuio/parsers.hpp>
#include <kerat/utils.hpp>
#include <uuid/uuid.h>

namespace dtuio {
    namespace sensor {

        const char * viewport::PATH = "/dtuio/vpt";

        viewport::viewport():m_width(0),m_height(0),m_depth(0){ ; }

        viewport::viewport(
            const helpers::uuid & viewport_uuid,
            const libkerat::helpers::point_3d & center,
            const libkerat::helpers::angle_3d & angle,
            const libkerat::dimmension_t width,
            const libkerat::dimmension_t height,
            const libkerat::dimmension_t depth
        ){
            init(viewport_uuid, center, angle, width, height, depth);
        }

        viewport::viewport(
            const helpers::uuid & viewport_uuid,
            const libkerat::helpers::point_3d & center,
            const libkerat::dimmension_t width,
            const libkerat::dimmension_t height,
            const libkerat::dimmension_t depth
        ){
            init(viewport_uuid, center, libkerat::helpers::angle_3d(), width, height, depth);
        }

        viewport::viewport(
            const helpers::uuid & viewport_uuid,
            const libkerat::dimmension_t width,
            const libkerat::dimmension_t height,
            const libkerat::dimmension_t depth
        ){
            init(viewport_uuid, libkerat::helpers::point_3d(width/2, height/2, depth/2), libkerat::helpers::angle_3d(), width, height, depth);
        }

        viewport::viewport(const viewport & original)
            :helpers::uuid(original), libkerat::helpers::point_3d(original), libkerat::helpers::angle_3d(original)
        {
            set_width(original.m_width);
            set_height(original.m_height);
            set_depth(original.m_depth);
        }

        viewport::~viewport(){ ; }

        viewport & viewport::operator=(const viewport & original){
            init(original, original, original, original.m_width, original.m_height, original.m_depth);
            return *this;
        }

        void viewport::init(const helpers::uuid& viewport_uuid, const libkerat::helpers::point_3d& center, const libkerat::helpers::angle_3d& angle, const libkerat::dimmension_t width, const libkerat::dimmension_t height, const libkerat::dimmension_t depth){
            // use ancestor setters
            helpers::uuid::operator=(viewport_uuid);
            libkerat::helpers::point_3d::operator=(center);
            libkerat::helpers::angle_3d::operator=(angle);
            // set my own
            m_width = width;
            m_height = height;
            m_depth = depth;
        }

        bool viewport::operator ==(const viewport& second) const {
            bool retval = true;
            // ancestors
            retval &= helpers::uuid::operator==(second);
            retval &= libkerat::helpers::point_3d::operator==(second);
            retval &= libkerat::helpers::angle_3d::operator==(second);
            // myself
            retval &= m_width == second.m_width;
            retval &= m_height == second.m_height;
            retval &= m_depth == second.m_depth;

            return retval;
        }

        // simple setters
        void viewport::set_width(libkerat::distance_t width){ m_width = width; }
        void viewport::set_height(libkerat::distance_t height){ m_height = height; }
        void viewport::set_depth(libkerat::distance_t depth){ m_depth = depth; }

        void viewport::move_x(libkerat::coord_t factor){ set_x(get_x() + factor); }
        void viewport::move_y(libkerat::coord_t factor){ set_y(get_y() + factor); }
        void viewport::move_z(libkerat::coord_t factor){ set_z(get_z() + factor); }
        void viewport::scale_x(float factor){
            set_width(get_width()*factor);
        }
        void viewport::scale_y(float factor){
            set_height(get_height()*factor);
        }
        void viewport::scale_z(float factor){
            set_depth(get_depth()*factor);
        }
        void viewport::rotate_by(libkerat::angle_t angle, const libkerat::helpers::point_2d & center){
            libkerat::rotate_around_center(*this, center, angle);
        }
        void viewport::rotate_pitch(libkerat::angle_t angle, const libkerat::helpers::point_3d & center){
            libkerat::rotate_around_center_pitch(*this, center, angle);
        }
        void viewport::rotate_roll(libkerat::angle_t angle, const libkerat::helpers::point_3d & center){
            libkerat::rotate_around_center_roll(*this, center, angle);
        }
        void viewport::rotate_by(libkerat::angle_t angle){ set_angle(get_angle() + angle); }
        void viewport::rotate_pitch(libkerat::angle_t angle){ set_pitch(get_pitch() + angle); }
        void viewport::rotate_roll(libkerat::angle_t angle){ set_roll(get_roll() + angle); }



        libkerat::kerat_message * viewport::clone() const { return new viewport(*this); }

        void viewport::print(std::ostream & output) const { output << *this; }

        bool viewport::imprint_lo_messages(lo_bundle target) const {

            if (target == NULL){ return false; }
            lo_message msg = lo_message_new();


            char buffer[UUID_STRING_CAPACITY];
            memset(buffer, 0, UUID_STRING_CAPACITY);
            uuid_unparse(m_uuid, buffer);
            lo_message_add_string(msg, buffer);

            lo_message_add_float(msg, get_x());
            lo_message_add_float(msg, get_y());
            lo_message_add_float(msg, get_z());

            lo_message_add_float(msg, get_yaw());
            lo_message_add_float(msg, get_pitch());
            lo_message_add_float(msg, get_roll());

            lo_message_add_int32(msg, get_width());
            lo_message_add_int32(msg, get_height());
            lo_message_add_int32(msg, get_depth());

            lo_bundle_add_message(target, PATH, msg);
            return true;
        }
    }

    namespace internals { namespace parsers {
        bool parse_viewport(
            libkerat::internals::convertor_output_container & results,
            const char * path,
            const char * types,
            lo_arg ** argv,
            int argc __attribute__((unused)),
            void * user_data __attribute__((unused))
        ){
            results.clear();
            // check for message type
            if (strcmp(path, dtuio::sensor::viewport::PATH) != 0){ return false; }

            // check for argument types
            if (strcmp(types, "sffffffiii") != 0){ return false; }

            dtuio::uuid_t uuid;
            // check for correct UUID
            if (uuid_parse(&(argv[0]->s), uuid) != 0){ return false; }

            libkerat::helpers::point_3d coordinates;
            libkerat::helpers::angle_3d rotations;

            libkerat::dimmension_t width;
            libkerat::dimmension_t height;
            libkerat::dimmension_t depth;

            coordinates.set_x(argv[1]->f);
            coordinates.set_y(argv[2]->f);
            coordinates.set_z(argv[3]->f);

            rotations.set_yaw(argv[4]->f);
            rotations.set_pitch(argv[5]->f);
            rotations.set_roll(argv[6]->f);

            width = argv[7]->i;
            height = argv[8]->i;
            depth = argv[9]->i;

            sensor::viewport * vpt = new sensor::viewport(uuid, coordinates, rotations, width, height, depth);
            if (vpt == NULL){ return false; }
            results.push_back(vpt);

            return true;
        }
    } }
}

std::ostream & operator<<(std::ostream & output, const dtuio::sensor::viewport & vpr){
    output << dtuio::sensor::viewport::PATH
        << " " << (dtuio::helpers::uuid)vpr
        << " " << (libkerat::helpers::point_3d)vpr
        << " " << (libkerat::helpers::angle_3d)vpr
        << " " << vpr.get_width()
        << " " << vpr.get_height()
        << " " << vpr.get_depth();

    return output;
}

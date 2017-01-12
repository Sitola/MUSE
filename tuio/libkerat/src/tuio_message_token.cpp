/**
 * \file      tuio_message_token.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-10-25 15:51 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/tuio_message_token.hpp>
#include <kerat/utils.hpp>
#include <lo/lo.h>
#include <list>
#include <vector>
#include <cstring>

namespace libkerat {
    namespace message {

        const char * token::PATH_2D = "/tuio2/tok";
        const char * token::PATH_3D = "/tuio2/t3d";

        // empty
        token::token(){ ; }

        // short 2D
        token::token(const session_id_t session_id, const type_id_t type_id, const user_id_t user_id, const component_id_t component_id,
            const coord_t x, const coord_t y,
            const angle_t angle
        )
            :contact_session(session_id), contact_type_user(type_id, user_id), contact_component(component_id),
            point_3d(x, y, 0), angle_3d(angle, 0, 0),
            message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_2D)
        { ; }

        // full 2D
        token::token(const session_id_t session_id, const type_id_t type_id, const user_id_t user_id, const component_id_t component_id,
            const coord_t x, const coord_t y,
            const angle_t angle,
            const velocity_t x_velocity, const velocity_t y_velocity,
            const rotation_velocity_t rotation_velocity,
            const accel_t movement_accel, const rotation_accel_t rotation_accel
        )
            :contact_session(session_id), contact_type_user(type_id, user_id), contact_component(component_id),
            point_3d(x, y, 0), angle_3d(angle, 0, 0),
            velocity_3d(x_velocity, y_velocity, 0), rotation_velocity_3d(rotation_velocity, 0, 0),
            movement_acceleration(movement_accel), rotation_acceleration(rotation_accel),
            message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_2D)
        { ; }

        // short 3D
        token::token(const session_id_t session_id, const type_id_t type_id, const user_id_t user_id, const component_id_t component_id,
            const coord_t x, const coord_t y, const coord_t z,
            const angle_t yaw_angle, const angle_t pitch_angle, const angle_t roll_angle
        )
            :contact_session(session_id), contact_type_user(type_id, user_id), contact_component(component_id),
            point_3d(x, y, z), angle_3d(yaw_angle, pitch_angle, roll_angle),
            velocity_3d(0, 0, 0), rotation_velocity_3d(0 , 0, 0),
            movement_acceleration(0), rotation_acceleration(0),
            message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_3D)
        { ; }

        token::token(const session_id_t session_id, const type_id_t type_id, const user_id_t user_id, const component_id_t component_id,
            const coord_t x, const coord_t y, const coord_t z,
            const angle_t yaw_angle, const angle_t pitch_angle, const angle_t roll_angle,
            const velocity_t x_velocity, const velocity_t y_velocity, const velocity_t z_velocity,
            const rotation_velocity_t yaw_velocity, const rotation_velocity_t pitch_velocity, const rotation_velocity_t roll_velocity,
            const accel_t movement_accel, const rotation_accel_t rotation_accel
        )
            :contact_session(session_id), contact_type_user(type_id, user_id), contact_component(component_id),
            point_3d(x, y, z), angle_3d(yaw_angle, pitch_angle, roll_angle),
            velocity_3d(x_velocity, y_velocity, z_velocity), rotation_velocity_3d(yaw_velocity, pitch_velocity, roll_velocity),
            movement_acceleration(movement_accel), rotation_acceleration(rotation_accel),
            message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_3D)
        { ; }

        token::token(const token & original)
            :contact_session(original), contact_type_user(original), contact_component(original),
            point_3d(original), angle_3d(original),
            velocity_3d(original), rotation_velocity_3d(original),
            movement_acceleration(original), rotation_acceleration(original),
            message_output_mode(original)
        { ; }

        token & token::operator =(const token& second){
            contact_session::operator=(second);
            contact_type_user::operator=(second);
            contact_component::operator=(second);

            point_3d::operator=(second);
            angle_3d::operator=(second);

            velocity_3d::operator=(second);
            rotation_velocity_3d::operator=(second);

            movement_acceleration::operator=(second);
            rotation_acceleration::operator=(second);

            message_output_mode::operator=(second);

            return *this;
        }

        bool token::operator ==(const token& second) const {
            return (
                contact_session::operator==(second) &&
                contact_type_user::operator==(second) &&
                contact_component::operator==(second) &&

                point_3d::operator==(second) &&
                angle_3d::operator==(second) &&

                velocity_3d::operator==(second) &&
                rotation_velocity_3d::operator==(second) &&

                movement_acceleration::operator==(second) &&
                rotation_acceleration::operator==(second) &&

                message_output_mode::operator==(second)
            );
        }

        kerat_message * token::clone() const { return new token(*this); }

        void token::print(std::ostream & output) const { output << *this; }

        bool token::is_extended() const {
            return (
                // first, the common components for both 2D & 3D
                (get_x_velocity() != 0) ||
                (get_y_velocity() != 0) ||
                (get_yaw_velocity() != 0) ||
                (get_acceleration() != 0) ||
                (get_rotation_acceleration() != 0) ||
                // and if this is 3D, then the 3D only stuff
                ((internals::testbit(get_message_output_mode(), helpers::message_output_mode::OUTPUT_MODE_3D)) && (
                    (get_z_velocity() != 0) ||
                    (get_pitch_velocity() != 0) ||
                    (get_roll_velocity() != 0)
                ))
            );
        }

        void token::move_x(coord_t factor){ set_x(get_x()+factor); }
        void token::move_y(coord_t factor){ set_y(get_y()+factor); }
        void token::move_z(coord_t factor){ set_z(get_z()+factor); }
        void token::rotate_by(angle_t angle, const helpers::point_2d & center){
            rotate_around_center(*this, center, angle);
        }
        void token::rotate_pitch(angle_t angle, const helpers::point_3d & center){
            rotate_around_center_pitch(*this, center, angle);
        }
        void token::rotate_roll(angle_t angle, const helpers::point_3d & center){
            rotate_around_center_roll(*this, center, angle);
        }
        void token::rotate_by(angle_t angle){ set_angle(get_angle() + angle); }
        void token::rotate_pitch(angle_t angle){ set_pitch(get_pitch() + angle); }
        void token::rotate_roll(angle_t angle){ set_roll(get_roll() + angle); }
        void token::scale_x(float factor){
            // separate acceleration components
            double tmp_x = get_x_velocity()*get_x_velocity();
            double tmp_y = get_y_velocity()*get_y_velocity();
            double tmp_z = get_z_velocity()*get_z_velocity();

            double j = get_acceleration()/(tmp_x + tmp_y + tmp_z);

            // recompute acceleration components
            tmp_x = get_x_velocity()*j*factor;
            tmp_y = get_y_velocity()*j;
            tmp_z = get_z_velocity()*j;

            // scale acceleration
            set_acceleration(
                (tmp_x*tmp_x)
                + (tmp_y*tmp_y)
                + (tmp_z*tmp_z)
            );

            // scale velocity
            set_x_velocity(get_x_velocity()*factor);
        }
        void token::scale_y(float factor){
            // separate acceleration components
            double tmp_x = get_x_velocity()*get_x_velocity();
            double tmp_y = get_y_velocity()*get_y_velocity();
            double tmp_z = get_z_velocity()*get_z_velocity();

            double j = get_acceleration()/(tmp_x + tmp_y + tmp_z);

            // recompute acceleration components
            tmp_x = get_x_velocity()*j;
            tmp_y = get_y_velocity()*j*factor;
            tmp_z = get_z_velocity()*j;

            // scale acceleration
            set_acceleration(
                (tmp_x*tmp_x)
                + (tmp_y*tmp_y)
                + (tmp_z*tmp_z)
            );

            // scale velocity
            set_y_velocity(get_y_velocity()*factor);
        }
        void token::scale_z(float factor){
            // separate acceleration components
            double tmp_x = get_x_velocity()*get_x_velocity();
            double tmp_y = get_y_velocity()*get_y_velocity();
            double tmp_z = get_z_velocity()*get_z_velocity();

            double j = get_acceleration()/(tmp_x + tmp_y + tmp_z);

            // recompute acceleration components
            tmp_x = get_x_velocity()*j;
            tmp_y = get_y_velocity()*j;
            tmp_z = get_z_velocity()*j*factor;

            // scale acceleration
            set_acceleration(
                (tmp_x*tmp_x)
                + (tmp_y*tmp_y)
                + (tmp_z*tmp_z)
            );

            // scale velocity
            set_z_velocity(get_z_velocity()*factor);
        }

        bool token::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            libkerat::tu_id_t tu_id = libkerat::compile_tuid(get_type_id(), get_user_id());

            // 2D
            if (internals::testbit(get_message_output_mode(), helpers::message_output_mode::OUTPUT_MODE_2D)){
                lo_message msg2d = lo_message_new();
                lo_message_add_int32(msg2d, get_session_id());
                lo_message_add_int32(msg2d, tu_id);
                lo_message_add_int32(msg2d, get_component_id());

                lo_message_add_float(msg2d, get_x());
                lo_message_add_float(msg2d, get_y());
                lo_message_add_float(msg2d, get_yaw());

                if (is_extended()){
                    lo_message_add_float(msg2d, get_x_velocity());
                    lo_message_add_float(msg2d, get_y_velocity());
                    lo_message_add_float(msg2d, get_yaw_velocity());
                    lo_message_add_float(msg2d, get_acceleration());
                    lo_message_add_float(msg2d, get_rotation_acceleration());
                }

                lo_bundle_add_message(target, PATH_2D, msg2d);
            } // 2D

            // 3D
            if (internals::testbit(get_message_output_mode(), helpers::message_output_mode::OUTPUT_MODE_3D)){
                lo_message msg = lo_message_new();
                lo_message_add_int32(msg, get_session_id());
                lo_message_add_int32(msg, tu_id);
                lo_message_add_int32(msg, get_component_id());

                lo_message_add_float(msg, get_x());
                lo_message_add_float(msg, get_y());
                lo_message_add_float(msg, get_z());
                lo_message_add_float(msg, get_yaw());
                lo_message_add_float(msg, get_pitch());
                lo_message_add_float(msg, get_roll());

                if (is_extended()){
                    lo_message_add_float(msg, get_x_velocity());
                    lo_message_add_float(msg, get_y_velocity());
                    lo_message_add_float(msg, get_z_velocity());
                    lo_message_add_float(msg, get_yaw_velocity());
                    lo_message_add_float(msg, get_pitch_velocity());
                    lo_message_add_float(msg, get_roll_velocity());
                    lo_message_add_float(msg, get_acceleration());
                    lo_message_add_float(msg, get_rotation_acceleration());
                }

                lo_bundle_add_message(target, PATH_3D, msg);
            } // 3D

            return true;

        }
    } // ns message

    namespace internals {
        namespace parsers {

            /**
            * \brief TUIO 2.0 token 2D (/tuio2/tok) parser
            *
            * The parserd messages are returned through the results argument. To
            * understand the remaining arguments, see
            * \ref libkerat::simple_client::message_convertor_entry
            *
            * \return true if the message was sucessfully recognized, false if an
            * error has occured or was not recognized
            *
            * \see libkerat::simple_client
            * libkerat::simple_client::message_convertor_entry
            * libkerat::kerat_message
            */
            bool parse_tok_2d(
                std::vector<libkerat::kerat_message *> & results,
                const char * path,
                const char * types,
                lo_arg ** argv,
                int argc,
                void * user_data __attribute__((unused))
            ){
                if (strcmp(path, libkerat::message::token::PATH_2D) != 0){ return false; }
                const char * typestring = "iiifff";
                const char * typestring_ext = "fffff";

                int ac = strlen(typestring);
                int eac = strlen(typestring_ext);

                if (argc < ac){ return false; }
                if (strncmp(typestring, types, ac) != 0){ return false; }

                libkerat::message::token * msg_token = new libkerat::message::token;
                msg_token->set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_2D);
                msg_token->set_session_id(argv[0]->i);
                msg_token->set_tu_id(argv[1]->i);
                msg_token->set_component_id(argv[2]->i);
                msg_token->set_x(argv[3]->f);
                msg_token->set_y(argv[4]->f);
                msg_token->set_angle(argv[5]->f);

                if (argc == (ac + eac)){
                    if (strcmp(typestring_ext, types+ac) == 0){
                        msg_token->set_x_velocity(argv[6]->f);
                        msg_token->set_y_velocity(argv[7]->f);
                        msg_token->set_rotation_velocity(argv[8]->f);
                        msg_token->set_acceleration(argv[9]->f);
                        msg_token->set_rotation_acceleration(argv[10]->f);

                    } else {
                        delete msg_token;
                        msg_token = NULL;
                        return false;
                    }
                }

                results.push_back(msg_token);

                return true;
            }

            /**
            * \brief TUIO 2.0 token 3D (/tuio2/t3d) parser
            *
            * The parserd messages are returned through the results argument. To
            * understand the remaining arguments, see
            * \ref libkerat::simple_client::message_convertor_entry
            *
            * \return true if the message was sucessfully recognized, false if an
            * error has occured or was not recognized
            *
            * \see libkerat::simple_client
            * libkerat::simple_client::message_convertor_entry
            * libkerat::kerat_message
            */
            bool parse_tok_3d(
                std::vector<libkerat::kerat_message *> & results,
                const char * path,
                const char * types,
                lo_arg ** argv,
                int argc,
                void * user_data __attribute__((unused))
            ){
                if (strcmp(path, libkerat::message::token::PATH_3D) != 0){ return false; }
                const char * typestring = "iiiffffff";
                const char * typestring_ext = "ffffffff";

                int ac = strlen(typestring);
                int eac = strlen(typestring_ext);

                if (argc < ac){ return false; }
                if (strncmp(typestring, types, ac) != 0){ return false; }

                libkerat::message::token * msg_token = new libkerat::message::token;
                msg_token->set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_3D);
                msg_token->set_session_id(argv[0]->i);
                msg_token->set_tu_id(argv[1]->i);
                msg_token->set_component_id(argv[2]->i);

                msg_token->set_x(argv[3]->f);
                msg_token->set_y(argv[4]->f);
                msg_token->set_z(argv[5]->f);

                msg_token->set_yaw(argv[6]->f);
                msg_token->set_pitch(argv[7]->f);
                msg_token->set_roll(argv[8]->f);

                if (argc == (ac + eac)){
                    if (strcmp(typestring_ext, types+ac) == 0){
                        msg_token->set_x_velocity(argv[9]->f);
                        msg_token->set_y_velocity(argv[10]->f);
                        msg_token->set_z_velocity(argv[11]->f);

                        msg_token->set_yaw_velocity(argv[12]->f);
                        msg_token->set_pitch_velocity(argv[13]->f);
                        msg_token->set_roll_velocity(argv[14]->f);

                        msg_token->set_acceleration(argv[15]->f);
                        msg_token->set_rotation_acceleration(argv[16]->f);

                    } else {
                        delete msg_token;
                        msg_token = NULL;
                        return false;
                    }
                }

                results.push_back(msg_token);

                return true;
            }
        } // ns parsers
    } // ns internals
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::token & msg_tok){
    // 2D
    if (libkerat::internals::testbit(msg_tok.get_message_output_mode(), libkerat::helpers::message_output_mode::OUTPUT_MODE_2D)){

        output << libkerat::message::token::PATH_2D << " " << msg_tok.get_session_id() << " "
            << msg_tok.get_user_id() << "/" << msg_tok.get_type_id() << " "
            << msg_tok.get_component_id() << " "
            << msg_tok.get_x() << " "
            << msg_tok.get_y() << " "
            << msg_tok.get_yaw();

        if (msg_tok.is_extended()){
            output << " " << msg_tok.get_x_velocity() << " "
                << msg_tok.get_y_velocity() << " "
                << msg_tok.get_yaw_velocity() << " "
                << msg_tok.get_acceleration() << " "
                << msg_tok.get_rotation_acceleration();
        }
    } // 2D

    // 3D
    if (libkerat::internals::testbit(msg_tok.get_message_output_mode(), libkerat::helpers::message_output_mode::OUTPUT_MODE_3D)){

        output << libkerat::message::token::PATH_3D << " " << msg_tok.get_session_id() << " "
            << msg_tok.get_user_id() << "/" << msg_tok.get_type_id() << " "
            << msg_tok.get_component_id() << " "
            << msg_tok.get_x() << " "
            << msg_tok.get_y() << " "
            << msg_tok.get_z() << " "
            << msg_tok.get_yaw() << " "
            << msg_tok.get_pitch() << " "
            << msg_tok.get_roll();


        if (msg_tok.is_extended()){
            output << " " << msg_tok.get_x_velocity() << " "
                << msg_tok.get_y_velocity() << " "
                << msg_tok.get_z_velocity() << " "
                << msg_tok.get_yaw_velocity() << " "
                << msg_tok.get_pitch_velocity() << " "
                << msg_tok.get_roll_velocity() << " "
                << msg_tok.get_acceleration() << " "
                << msg_tok.get_rotation_acceleration();
        }
    } // 3D

    return output;
}

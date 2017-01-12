/**
 * \file      tuio_message_pointer.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-09-19 13:51 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/tuio_message_pointer.hpp>
#include <kerat/utils.hpp>
#include <lo/lo.h>
#include <list>
#include <vector>
#include <cstring>

namespace libkerat {

    namespace message {

        const char * pointer::PATH_2D = "/tuio2/ptr";
        const char * pointer::PATH_3D = "/tuio2/p3d";

        pointer::pointer():m_width(0), m_press(0){ ; }

        // full 2D version
        pointer::pointer(const session_id_t session_id, const type_id_t type_id, const user_id_t user_id, const component_id_t component_id,
                const coord_t x, const coord_t y,
                const distance_t contact_width, const pressure_t contact_pressure,
                const velocity_t x_velocity, const velocity_t y_velocity,
                const accel_t pointer_acc
        )
            :contact_session(session_id), contact_type_user(type_id, user_id), contact_component(component_id),
            point_3d(x, y, 0), velocity_3d(x_velocity, y_velocity, 0), movement_acceleration(pointer_acc),
            message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_2D),
            m_width(contact_width), m_press(contact_pressure)
        { ; }

        // short 2D version
        pointer::pointer(const session_id_t session_id, const type_id_t type_id, const user_id_t user_id, const component_id_t component_id,
                const coord_t x, const coord_t y,
                const distance_t contact_width, const pressure_t contact_pressure
        ):contact_session(session_id), contact_type_user(type_id, user_id), contact_component(component_id),
            point_3d(x, y, 0), message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_2D),
            m_width(contact_width), m_press(contact_pressure)
        { ; }

        // full 3D version
        pointer::pointer(const session_id_t session_id, const type_id_t type_id, const user_id_t user_id, const component_id_t component_id,
                const coord_t x, const coord_t y, const coord_t z,
                const distance_t contact_width, const pressure_t contact_pressure,
                const velocity_t x_velocity, const velocity_t y_velocity, const velocity_t z_velocity,
                const accel_t pointer_acc
        )
            :contact_session(session_id), contact_type_user(type_id, user_id), contact_component(component_id),
            point_3d(x, y, z), velocity_3d(x_velocity, y_velocity, z_velocity), movement_acceleration(pointer_acc),
            message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_3D),
            m_width(contact_width), m_press(contact_pressure)
        { ; }

        // short 3D version
        pointer::pointer(const session_id_t session_id, const type_id_t type_id, const user_id_t user_id, const component_id_t component_id,
                const coord_t x, const coord_t y, const coord_t z,
                const distance_t contact_width, const pressure_t contact_pressure
        )
            :contact_session(session_id), contact_type_user(type_id, user_id), contact_component(component_id),
            point_3d(x, y, z), message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_3D),
            m_width(contact_width), m_press(contact_pressure)
        { ; }

        pointer::pointer(const pointer & original)
            :contact_session(original), contact_type_user(original), contact_component(original),
            point_3d(original), velocity_3d(original), movement_acceleration(original),
            message_output_mode(original),
            m_width(original.m_width), m_press(original.m_press)
        { ; }

        pointer & pointer::operator =(const pointer& second){
            contact_session::operator=(second);
            contact_type_user::operator=(second);
            contact_component::operator=(second);

            point_3d::operator=(second);
            velocity_3d::operator=(second);
            movement_acceleration::operator=(second);

            m_width = second.m_width;
            m_press = second.m_press;

            message_output_mode::operator=(second);

            return *this;
        }
        bool pointer::operator ==(const pointer& second) const {
            return (
                contact_session::operator==(second) &&
                contact_type_user::operator==(second) &&
                contact_component::operator==(second) &&

                point_3d::operator==(second) &&
                velocity_3d::operator==(second) &&
                movement_acceleration::operator==(second) &&

                (m_width == second.m_width) &&
                (m_press == second.m_press) &&

                message_output_mode::operator==(second)
            );
        }

        pressure_t pointer::set_pressure(pressure_t pressure) {
            pressure_t oldval = m_press;
            m_press = pressure;
            return oldval;
        }

        distance_t pointer::set_width(distance_t contact_width) {
            distance_t oldval = m_width;
            m_width = contact_width;
            return oldval;
        }

        bool pointer::is_extended() const {
            return (
                // first, the common components for both 2D & 3D
                (get_x_velocity() != 0) ||
                (get_y_velocity() != 0) ||
                (get_acceleration() != 0) ||
                // and if this is 3D, then the 3D only stuff
                ((internals::testbit(get_message_output_mode(), helpers::message_output_mode::OUTPUT_MODE_3D)) && (
                    (get_z_velocity() != 0)
                ))
            );
        }

        kerat_message * pointer::clone() const { return new pointer(*this); }

        void pointer::print(std::ostream & output) const { output << *this; }

        void pointer::move_x(coord_t factor){ set_x(get_x()+factor); }
        void pointer::move_y(coord_t factor){ set_y(get_y()+factor); }
        void pointer::move_z(coord_t factor){ set_z(get_z()+factor); }
        void pointer::rotate_by(angle_t angle, const helpers::point_2d & center){
            rotate_around_center(*this, center, angle);
        }
        void pointer::rotate_pitch(angle_t angle, const helpers::point_3d & center){
            rotate_around_center_pitch(*this, center, angle);
        }
        void pointer::rotate_roll(angle_t angle, const helpers::point_3d & center){
            rotate_around_center_roll(*this, center, angle);
        }
        void pointer::scale_x(float factor){
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
        void pointer::scale_y(float factor){
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
        void pointer::scale_z(float factor){
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

        bool pointer::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            libkerat::tu_id_t tu_id = libkerat::compile_tuid(get_type_id(), get_user_id());

            // 2D
            if (internals::testbit(get_message_output_mode(), message_output_mode::OUTPUT_MODE_2D)){

                lo_message msg2d = lo_message_new();
                lo_message_add_int32(msg2d, get_session_id());
                lo_message_add_int32(msg2d, tu_id);
                lo_message_add_int32(msg2d, get_component_id());

                lo_message_add_float(msg2d, get_x());
                lo_message_add_float(msg2d, get_y());
                lo_message_add_float(msg2d, get_width());
                lo_message_add_float(msg2d, get_pressure());

                if (is_extended()){
                    lo_message_add_float(msg2d, get_x_velocity());
                    lo_message_add_float(msg2d, get_y_velocity());
                    lo_message_add_float(msg2d, get_acceleration());
                }

                lo_bundle_add_message(target, PATH_2D, msg2d);
            } // 2D

            // 3D
            if (internals::testbit(get_message_output_mode(), message_output_mode::OUTPUT_MODE_3D)){
                lo_message msg = lo_message_new();
                lo_message_add_int32(msg, get_session_id());
                lo_message_add_int32(msg, tu_id);
                lo_message_add_int32(msg, get_component_id());

                lo_message_add_float(msg, get_x());
                lo_message_add_float(msg, get_y());
                lo_message_add_float(msg, get_z());
                lo_message_add_float(msg, get_width());
                lo_message_add_float(msg, get_pressure());

                if (is_extended()){
                    lo_message_add_float(msg, get_x_velocity());
                    lo_message_add_float(msg, get_y_velocity());
                    lo_message_add_float(msg, get_z_velocity());
                    lo_message_add_float(msg, get_acceleration());
                }

                lo_bundle_add_message(target, PATH_3D, msg);
            } // 3D

            return true;

        }

    }

    namespace internals { namespace parsers {
            /**
            * \brief TUIO 2.0 pointer 2D (/tuio2/ptr) parser
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
            bool parse_ptr_2d(
                std::vector<libkerat::kerat_message *> & results,
                const char * path,
                const char * types,
                lo_arg ** argv,
                int argc,
                void * user_data __attribute__((unused))
            ){
                if (strcmp(path, libkerat::message::pointer::PATH_2D) != 0){ return false; }
                const char * typestring = "iiiffff";
                const char * typestring_ext = "fff";

                int ac = strlen(typestring);
                int eac = strlen(typestring_ext);

                if (argc < ac){ return false; }
                if (strncmp(typestring, types, ac) != 0){ return false; }

                libkerat::message::pointer * msg_pointer = new libkerat::message::pointer;
                msg_pointer->set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_2D);
                msg_pointer->set_session_id(argv[0]->i);
                msg_pointer->set_tu_id(argv[1]->i);
                msg_pointer->set_component_id(argv[2]->i);
                msg_pointer->set_x(argv[3]->f);
                msg_pointer->set_y(argv[4]->f);
                msg_pointer->set_width(argv[5]->f);
                msg_pointer->set_pressure(argv[6]->f);

                if (argc == (ac + eac)){
                    if (strcmp(typestring_ext, types+ac) == 0){
                        msg_pointer->set_x_velocity(argv[7]->f);
                        msg_pointer->set_y_velocity(argv[8]->f);
                        msg_pointer->set_acceleration(argv[9]->f);
                    } else {
                        delete msg_pointer;
                        msg_pointer = NULL;
                        return false;
                    }
                }

                results.push_back(msg_pointer);

                return true;
            }

            /**
            * \brief TUIO 2.0 pointer 3D (/tuio2/p3d) parser
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
            bool parse_ptr_3d(
                std::vector<libkerat::kerat_message *> & results,
                const char * path,
                const char * types,
                lo_arg ** argv,
                int argc,
                void * user_data __attribute__((unused))
            ){
                if (strcmp(path, libkerat::message::pointer::PATH_3D) != 0){ return false; }
                const char * typestring = "iiifffff";
                const char * typestring_ext = "ffff";

                int ac = strlen(typestring);
                int eac = strlen(typestring_ext);

                if (argc < ac){ return false; }
                if (strncmp(typestring, types, ac) != 0){ return false; }

                libkerat::message::pointer * msg_pointer = new libkerat::message::pointer;
                msg_pointer->set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_3D);
                msg_pointer->set_session_id(argv[0]->i);
                msg_pointer->set_tu_id(argv[1]->i);
                msg_pointer->set_component_id(argv[2]->i);
                msg_pointer->set_x(argv[3]->f);
                msg_pointer->set_y(argv[4]->f);
                msg_pointer->set_z(argv[5]->f);
                msg_pointer->set_width(argv[6]->f);
                msg_pointer->set_pressure(argv[7]->f);

                if (argc == (ac + eac)){
                    if (strcmp(typestring_ext, types+ac) == 0){
                        msg_pointer->set_x_velocity(argv[8]->f);
                        msg_pointer->set_y_velocity(argv[9]->f);
                        msg_pointer->set_z_velocity(argv[10]->f);
                        msg_pointer->set_acceleration(argv[11]->f);

                    } else {
                        delete msg_pointer;
                        msg_pointer = NULL;
                        return false;
                    }
                }

                results.push_back(msg_pointer);

                return true;
            }
        } // ns parsers
    } // ns internals
} // ns libkerat

/**
 * \ingroup global
 * \brief Pretty-print the \ref libkerat::message::pointer "pointer" message to given output stream
 * \note Format for 2D variant: <tt>
 * {OSC path} {session id} {type id}/{user id} {component id}
 * {\ref libkerat::helpers::point_2d "coordinates"} {contact width} {contact pressure}
 * [ {\ref libkerat::helpers::velocity_2d "velocity"} {acceleration}]
 * </tt>
 * \note Format for 3D variant: <tt>
 * {OSC path} {session id} {type id}/{user id} {component id}
 * {\ref libkerat::helpers::point_3d "coordinates"} {contact width} {contact pressure}
 * [ {\ref libkerat::helpers::velocity_3d "velocity"} {acceleration}]
 * </tt>
 * \param output - output stream to write to
 * \param msg_ptr - pointer message to print
 * \return modified output stream
 * \see operator<<(std::ostream &, const libkerat::helpers::point_2d &)
 * \see operator<<(std::ostream &, const libkerat::helpers::point_3d &)
 * \see operator<<(std::ostream &, const libkerat::helpers::velocity_2d &)
 * \see operator<<(std::ostream &, const libkerat::helpers::velocity_3d &)
 */
std::ostream & operator<<(std::ostream & output, const libkerat::message::pointer & msg_ptr){
    // 2D
    if (libkerat::internals::testbit(msg_ptr.get_message_output_mode(), libkerat::helpers::message_output_mode::OUTPUT_MODE_2D)){

        output << libkerat::message::pointer::PATH_2D << " " << msg_ptr.get_session_id() << " "
            << msg_ptr.get_user_id() << "/" << msg_ptr.get_type_id() << " "
            << msg_ptr.get_component_id() << " "
            << (libkerat::helpers::point_2d)msg_ptr << " "
            << msg_ptr.get_width() << " "
            << msg_ptr.get_pressure();

        if (msg_ptr.is_extended()){
            output << " " << (libkerat::helpers::velocity_2d)msg_ptr << " "
                << msg_ptr.get_acceleration();
        }
    } // 2D

    // 3D
    if (libkerat::internals::testbit(msg_ptr.get_message_output_mode(), libkerat::helpers::message_output_mode::OUTPUT_MODE_3D)){

        output << libkerat::message::pointer::PATH_3D << " " << msg_ptr.get_session_id() << " "
            << msg_ptr.get_user_id() << "/" << msg_ptr.get_type_id() << " "
            << msg_ptr.get_component_id() << " "
            << msg_ptr.get_x() << " "
            << msg_ptr.get_y() << " "
            << msg_ptr.get_z() << " "
            << msg_ptr.get_width() << " "
            << msg_ptr.get_pressure();

        if (msg_ptr.is_extended()){
            output << " " << msg_ptr.get_x_velocity() << " "
                << msg_ptr.get_y_velocity() << " "
                << msg_ptr.get_z_velocity() << " "
                << msg_ptr.get_acceleration();
        }
    } // 3D

    return output;
}

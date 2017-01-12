/**
 * \file      tuio_message_bounds.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-10-29 12:41 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/tuio_message_bounds.hpp>
#include <kerat/utils.hpp>
#include <lo/lo.h>
#include <list>
#include <vector>
#include <cstring>
#include <ostream>

namespace libkerat {

    namespace message {

        const char * bounds::PATH_2D = "/tuio2/bnd";
        const char * bounds::PATH_3D = "/tuio2/b3d";

        // empty
        bounds::bounds()
            :m_width(0), m_height(0), m_area(0), m_depth(0), m_vol(0)
        { ; }

        // simple 2D
        bounds::bounds(const session_id_t session_id,
            const coord_t x, const coord_t y,
            const angle_t angle,
            const distance_t bbox_width, const distance_t bbox_height,
            const area_t region_area
        )
            :contact_session(session_id), point_3d(x, y, 0), angle_3d(angle, 0, 0),
            message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_2D),
            m_width(bbox_width), m_height(bbox_height),
            m_area(region_area), m_depth(0), m_vol(0)
        { ; }

        // full 2D
        bounds::bounds(const session_id_t session_id,
            const coord_t x, const coord_t y,
            const angle_t angle,
            const distance_t bbox_width, const distance_t bbox_height,
            const area_t region_area,
            const velocity_t x_velocity, const velocity_t y_velocity,
            const rotation_velocity_t rotation_velocity,
            const accel_t movement_accel, const rotation_accel_t rotation_accel
        )
            :contact_session(session_id), point_3d(x, y, 0), angle_3d(angle, 0, 0),
            velocity_3d(x_velocity, y_velocity, 0), rotation_velocity_3d(rotation_velocity, 0, 0),
            movement_acceleration(movement_accel), rotation_acceleration(rotation_accel),
            message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_2D),
            m_width(bbox_width), m_height(bbox_height),
            m_area(region_area), m_depth(0), m_vol(0)
        { ; }

        // simple 3D
        bounds::bounds(const session_id_t session_id,
            const coord_t x, const coord_t y, const coord_t z,
            const angle_t yaw_angle, const angle_t pitch_angle, const angle_t roll_angle,
            const distance_t bbox_width, const distance_t bbox_height, const distance_t bbox_depth,
            const volume_t region_volume
        )
            :contact_session(session_id), point_3d(x, y, z), angle_3d(yaw_angle, pitch_angle, roll_angle),
            message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_3D),
            m_width(bbox_width), m_height(bbox_height),
            m_area(0), m_depth(bbox_depth), m_vol(region_volume)
        { ; }

        // full 3D
        bounds::bounds(const session_id_t session_id,
            const coord_t x, const coord_t y, const coord_t z,
            const angle_t yaw_angle, const angle_t pitch_angle, const angle_t roll_angle,
            const distance_t bbox_width, const distance_t bbox_height, const distance_t bbox_depth,
            const volume_t region_volume,
            const velocity_t x_velocity, const velocity_t y_velocity, const velocity_t z_velocity,
            const rotation_velocity_t yaw_velocity, const rotation_velocity_t pitch_velocity, const rotation_velocity_t roll_velocity,
            const accel_t movement_accel, const rotation_accel_t rotation_accel
        )
            :contact_session(session_id), point_3d(x, y, z), angle_3d(yaw_angle, pitch_angle, roll_angle),
            velocity_3d(x_velocity, y_velocity, z_velocity), rotation_velocity_3d(yaw_velocity, pitch_velocity, roll_velocity),
            movement_acceleration(movement_accel), rotation_acceleration(rotation_accel),
            message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_3D),
            m_width(bbox_width), m_height(bbox_height),
            m_area(0), m_depth(bbox_depth), m_vol(region_volume)
        { ; }

        bounds & bounds::operator =(const bounds& second){
            contact_session::operator=(second);
            point_3d::operator=(second);
            angle_3d::operator=(second);

            velocity_3d::operator=(second);
            rotation_velocity_3d::operator=(second);

            movement_acceleration::operator=(second);
            rotation_acceleration::operator=(second);

            message_output_mode::operator=(second);

            m_width = second.m_width;
            m_height = second.m_height;
            m_area = second.m_area;
            m_depth = second.m_depth;
            m_vol = second.m_vol;

            return *this;
        }

        bool bounds::operator ==(const bounds& second) const {
            return (
                contact_session::operator==(second) &&

                point_3d::operator==(second) &&
                angle_3d::operator==(second) &&

                velocity_3d::operator==(second) &&
                rotation_velocity_3d::operator==(second) &&

                movement_acceleration::operator==(second) &&
                rotation_acceleration::operator==(second) &&

                message_output_mode::operator==(second) &&

                (m_width == second.m_width) &&
                (m_height == second.m_height) &&
                (m_area == second.m_area) &&
                (m_depth == second.m_depth) &&
                (m_vol == second.m_vol)
            );
        }

        bool bounds::is_extended() const {
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

        volume_t bounds::set_volume(volume_t region_volume){
            volume_t retval = m_vol;
            m_vol = region_volume;
            return retval;
        }

        distance_t bounds::set_depth(distance_t bbox_depth){
            distance_t retval = m_depth;
            m_depth = bbox_depth;
            return retval;
        }

        area_t bounds::set_area(area_t region_area){
            area_t retval = m_area;
            m_area = region_area;
            return retval;
        }

        distance_t bounds::set_height(distance_t bbox_height){
            distance_t retval = m_height;
            m_height = bbox_height;
            return retval;
        }

        distance_t bounds::set_width(distance_t bbox_width){
            distance_t retval = m_width;
            m_width = bbox_width;
            return retval;
        }

        kerat_message * bounds::clone() const { return new bounds(*this); }

        void bounds::print(std::ostream & output) const { output << *this; }

        bool bounds::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            // 2D
            if (internals::testbit(get_message_output_mode(), message_output_mode::OUTPUT_MODE_2D)){
                lo_message msg2d = lo_message_new();
                lo_message_add_int32(msg2d, get_session_id());

                lo_message_add_float(msg2d, get_x());
                lo_message_add_float(msg2d, get_y());

                lo_message_add_float(msg2d, get_angle());

                lo_message_add_float(msg2d, get_width());
                lo_message_add_float(msg2d, get_height());
                lo_message_add_float(msg2d, get_area());

                if (is_extended()){
                    lo_message_add_float(msg2d, get_x_velocity());
                    lo_message_add_float(msg2d, get_y_velocity());
                    lo_message_add_float(msg2d, get_rotation_velocity());
                    lo_message_add_float(msg2d, get_acceleration());
                    lo_message_add_float(msg2d, get_rotation_acceleration());
                }

                lo_bundle_add_message(target, PATH_2D, msg2d);
            } // 2D

            // 3D
            if (internals::testbit(get_message_output_mode(), message_output_mode::OUTPUT_MODE_3D)){

                lo_message msg = lo_message_new();
                lo_message_add_int32(msg, get_session_id());

                lo_message_add_float(msg, get_x());
                lo_message_add_float(msg, get_y());
                lo_message_add_float(msg, get_z());

                lo_message_add_float(msg, get_yaw());
                lo_message_add_float(msg, get_pitch());
                lo_message_add_float(msg, get_roll());

                lo_message_add_float(msg, get_width());
                lo_message_add_float(msg, get_height());
                lo_message_add_float(msg, get_depth());
                lo_message_add_float(msg, get_volume());


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

        void bounds::move_x(coord_t factor){ set_x(get_x() + factor); }
        void bounds::move_y(coord_t factor){ set_y(get_y() + factor); }
        void bounds::move_z(coord_t factor){ set_z(get_z() + factor); }
        void bounds::scale_x(float factor){
            set_width(get_width()*factor);

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
        void bounds::scale_y(float factor){
            set_height(get_height()*factor);

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
        void bounds::scale_z(float factor){
            set_depth(get_depth()*factor);

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
        void bounds::rotate_by(angle_t angle, const helpers::point_2d & center){
            rotate_around_center(*this, center, angle);
        }
        void bounds::rotate_pitch(angle_t angle, const helpers::point_3d & center){
            rotate_around_center_pitch(*this, center, angle);
        }
        void bounds::rotate_roll(angle_t angle, const helpers::point_3d & center){
            rotate_around_center_roll(*this, center, angle);
        }
        void bounds::rotate_by(angle_t angle){ set_angle(get_angle() + angle); }
        void bounds::rotate_pitch(angle_t angle){ set_pitch(get_pitch() + angle); }
        void bounds::rotate_roll(angle_t angle){ set_roll(get_roll() + angle); }

    }

    // bnd parser
    namespace internals {
        namespace parsers {
            /**
            * \brief TUIO 2.0 bounds 2D (/tuio2/bnd) parser
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
            bool parse_bnd_2d(
                std::vector<libkerat::kerat_message *> & results,
                const char * path,
                const char * types,
                lo_arg ** argv,
                int argc,
                void * user_data __attribute__((unused))
            ){
                if (strcmp(path, libkerat::message::bounds::PATH_2D) != 0){ return false; }
                const char * typestring = "iffffff";
                const char * typestring_ext = "fffff";

                int ac = strlen(typestring);
                int eac = strlen(typestring_ext);

                if (argc < ac){ return false; }
                if (strncmp(typestring, types, ac) != 0){ return false; }

                libkerat::message::bounds * msg_bound = new libkerat::message::bounds;
                msg_bound->set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_2D);
                msg_bound->set_session_id(argv[0]->i);
                msg_bound->set_x(argv[1]->f);
                msg_bound->set_y(argv[2]->f);

                msg_bound->set_angle(argv[3]->f);
                msg_bound->set_width(argv[4]->f);
                msg_bound->set_height(argv[5]->f);
                msg_bound->set_area(argv[6]->f);

                if (argc == (ac+eac)){
                    if (strcmp(typestring_ext, types+ac) == 0){
                        msg_bound->set_x_velocity(argv[7]->f);
                        msg_bound->set_y_velocity(argv[8]->f);

                        msg_bound->set_rotation_velocity(argv[9]->f);
                        msg_bound->set_acceleration(argv[10]->f);
                        msg_bound->set_rotation_acceleration(argv[11]->f);

                    } else {
                        delete msg_bound;
                        msg_bound = NULL;
                        return false;
                    }
                }

                results.push_back(msg_bound);

                return true;
            }

            /**
            * \brief TUIO 2.0 bounds 3D (/tuio2/b3d) parser
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
            bool parse_bnd_3d(
                std::vector<libkerat::kerat_message *> & results,
                const char * path,
                const char * types,
                lo_arg ** argv,
                int argc,
                void * user_data __attribute__((unused))
            ){
                if (strcmp(path, libkerat::message::bounds::PATH_3D) != 0){ return false; }
                const char * typestring = "iffffffffff";
                const char * typestring_ext = "ffffffff";

                int ac = strlen(typestring);
                int eac = strlen(typestring_ext);

                if (argc < ac){ return false; }
                if (strncmp(typestring, types, ac) != 0){ return false; }

                libkerat::message::bounds * msg_bound = new libkerat::message::bounds;
                msg_bound->set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_3D);
                msg_bound->set_session_id(argv[0]->i);
                msg_bound->set_x(argv[1]->f);
                msg_bound->set_y(argv[2]->f);
                msg_bound->set_z(argv[3]->f);

                msg_bound->set_yaw(argv[4]->f);
                msg_bound->set_pitch(argv[5]->f);
                msg_bound->set_roll(argv[6]->f);

                msg_bound->set_width(argv[7]->f);
                msg_bound->set_height(argv[8]->f);
                msg_bound->set_depth(argv[9]->f);
                msg_bound->set_volume(argv[10]->f);

                if (argc == (ac + eac)){
                    if (strcmp(typestring_ext, types+ac) == 0){
                        msg_bound->set_x_velocity(argv[11]->f);
                        msg_bound->set_y_velocity(argv[12]->f);
                        msg_bound->set_z_velocity(argv[13]->f);

                        msg_bound->set_yaw_velocity(argv[14]->f);
                        msg_bound->set_pitch_velocity(argv[15]->f);
                        msg_bound->set_roll_velocity(argv[16]->f);

                        msg_bound->set_acceleration(argv[17]->f);
                        msg_bound->set_rotation_acceleration(argv[18]->f);

                    } else {
                        delete msg_bound;
                        msg_bound = NULL;
                        return false;
                    }
                }

                results.push_back(msg_bound);

                return true;
            }

        } // ns parsers
    } // ns internal
} // ns libkerat

/**
 * \ingroup global
 * \brief Pretty-print the \ref libkerat::message::bounds "bounds" message to given output stream
 * \note Format for 2D variant: <tt>
 * {OSC path} {session id}
 * {\ref libkerat::helpers::point_2d "coordinates"} {\ref libkerat::helpers::angle_2d "angle"}
 * {width} {height} {area}
 * [
 *   {\ref libkerat::helpers::velocity_2d "velocity"} {\ref libkerat::helpers::rotation_velocity_2d "rotation velocity"}
 *   {acceleration} {rotation acceleration}
 * ]
 * </tt>
 * \note Format for 3D variant: <tt>
 * {OSC path} {session id}
 * {\ref libkerat::helpers::point_3d "coordinates"} {\ref libkerat::helpers::angle_3d "angle"}
 * {width} {height} {depth} {area}
 * [
 *   {\ref libkerat::helpers::velocity_3d "velocity"} {\ref libkerat::helpers::rotation_velocity_3d "rotation velocity"}
 *   {acceleration} {rotation acceleration}
 * ]
 * </tt>
 * \param output - output stream to write to
 * \param msg_bnd - bounds message to print
 * \return modified output stream
 * \see operator<<(std::ostream &, const libkerat::helpers::point_2d &)
 * \see operator<<(std::ostream &, const libkerat::helpers::point_3d &)
 * \see operator<<(std::ostream &, const libkerat::helpers::velocity_2d &)
 * \see operator<<(std::ostream &, const libkerat::helpers::velocity_3d &)
 * \see operator<<(std::ostream &, const libkerat::helpers::angle_2d &)
 * \see operator<<(std::ostream &, const libkerat::helpers::angle_3d &)
 * \see operator<<(std::ostream &, const libkerat::helpers::rotation_velocity_2d &)
 * \see operator<<(std::ostream &, const libkerat::helpers::rotation_velocity_3d &)
 */
std::ostream & operator<<(std::ostream & output, const libkerat::message::bounds & msg_bnd){
    // 2D
    if (libkerat::internals::testbit(msg_bnd.get_message_output_mode(), libkerat::helpers::message_output_mode::OUTPUT_MODE_2D)){

        output << libkerat::message::bounds::PATH_2D << " " << msg_bnd.get_session_id() << " "
            << (libkerat::helpers::point_2d)msg_bnd << " "
            << msg_bnd.get_angle() << " "
            << msg_bnd.get_width() << " "
            << msg_bnd.get_height() << " "
            << msg_bnd.get_area();

        if (msg_bnd.is_extended()){
            output << " " << (libkerat::helpers::velocity_2d)msg_bnd << " "
                << msg_bnd.get_rotation_velocity() << " "
                << msg_bnd.get_acceleration() << " "
                << msg_bnd.get_rotation_acceleration();
        }
    } // 2D

    // 3D
    if (libkerat::internals::testbit(msg_bnd.get_message_output_mode(), libkerat::helpers::message_output_mode::OUTPUT_MODE_3D)){

        output << libkerat::message::bounds::PATH_3D << " " << msg_bnd.get_session_id() << " "
            << msg_bnd.get_x() << " "
            << msg_bnd.get_y() << " "
            << msg_bnd.get_z() << " "
            << msg_bnd.get_yaw() << " "
            << msg_bnd.get_pitch() << " "
            << msg_bnd.get_roll() << " "
            << msg_bnd.get_width() << " "
            << msg_bnd.get_height() << " "
            << msg_bnd.get_depth() << " "
            << msg_bnd.get_volume();


        if (msg_bnd.is_extended()){
            output << " " << msg_bnd.get_x_velocity() << " "
                << msg_bnd.get_y_velocity() << " "
                << msg_bnd.get_z_velocity() << " "
                << msg_bnd.get_yaw_velocity() << " "
                << msg_bnd.get_pitch_velocity() << " "
                << msg_bnd.get_roll_velocity() << " "
                << msg_bnd.get_acceleration() << " "
                << msg_bnd.get_rotation_acceleration();
        }
    } // 3D

    return output;
}

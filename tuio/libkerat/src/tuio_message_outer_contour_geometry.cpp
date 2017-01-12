/**
 * \file      tuio_message_outer_contour_geometry.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-04-21 15:26 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/tuio_message_outer_contour_geometry.hpp>
#include <kerat/utils.hpp>
#include <lo/lo.h>
#include <list>
#include <vector>
#include <cstring>
#include <lo/lo_lowlevel.h>
#include <lo/lo_osc_types.h>

namespace libkerat {
    namespace message {

        const char * outer_contour::PATH = "/tuio2/ocg";

        outer_contour::outer_contour(){ ; }

        outer_contour::outer_contour(const session_id_t session_id)
            :contact_session(session_id)
        { ; }

        outer_contour::outer_contour(const session_id_t session_id, const point_2d_list& points)
            :contact_session(session_id), m_contour(points)
        { ; }

        outer_contour::outer_contour(const outer_contour & second)
            :contact_session(second), m_contour(second.m_contour)
        { ; }

        outer_contour::~outer_contour(){ ; }

        outer_contour & outer_contour::operator=(const outer_contour & second){

            contact_session::operator=(second);
            m_contour = second.m_contour;

            return *this;
        }
        bool outer_contour::operator ==(const outer_contour & second) const {
            return (
                contact_session::operator==(second) &&
                m_contour==(second.m_contour)
            );
        }

        outer_contour::point_2d_list outer_contour::set_contour(const outer_contour::point_2d_list & points) {
            outer_contour::point_2d_list retval = m_contour;
            m_contour = points;
            return retval;
        }

        bool outer_contour::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            lo_message msg = lo_message_new();
            lo_message_add_int32(msg, m_session_id);

            for (libkerat::message::outer_contour::point_2d_list::const_iterator i = m_contour.begin(); i != m_contour.end(); i++){
                lo_message_add_float(msg, i->get_x());
                lo_message_add_float(msg, i->get_y());
            }

            lo_bundle_add_message(target, PATH, msg);

            return true;
        }

        outer_contour * outer_contour::clone() const { return new outer_contour(*this); }

        void outer_contour::print(std::ostream & output) const { output << *this; }

        void outer_contour::move_x(coord_t factor){
            for (point_2d_list::iterator pt = m_contour.begin(); pt != m_contour.end(); ++pt){
                pt->set_x(pt->get_x() + factor);
            }
        }
        void outer_contour::move_y(coord_t factor){
            for (point_2d_list::iterator pt = m_contour.begin(); pt != m_contour.end(); ++pt){
                pt->set_y(pt->get_y() + factor);
            }
        }

        void outer_contour::scale_x(float factor, const helpers::point_2d & center){
            for (point_2d_list::iterator pt = m_contour.begin(); pt != m_contour.end(); ++pt){
                pt->set_x(center.get_x() + factor*(pt->get_x()-center.get_x()));
            }
        }
        void outer_contour::scale_y(float factor, const helpers::point_2d & center){
            for (point_2d_list::iterator pt = m_contour.begin(); pt != m_contour.end(); ++pt){
                pt->set_y(center.get_y() + factor*(pt->get_y()-center.get_y()));
            }
        }

        void outer_contour::rotate_by(angle_t angle, const helpers::point_2d & center){
            for (point_2d_list::iterator pt = m_contour.begin(); pt != m_contour.end(); ++pt){
                rotate_around_center(*pt, center, angle);
            }
        }

    } // ns message

    namespace internals { namespace parsers {
        /**
        * \brief TUIO 2.0 outer contour geometry (/tuio2/ocg) parser
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
        bool parse_ocg(
            std::vector<libkerat::kerat_message *> & results,
            const char * path,
            const char * types,
            lo_arg ** argv,
            int argc,
            void * user_data __attribute__((unused))
        ){
            if (strcmp(path, libkerat::message::outer_contour::PATH) != 0){ return false; }
            if (argc < 1){ return false; }
            if (types[0] != 'i'){ return false; }

            // each point si represented by 2 coordinates, + session id
            if (argc % 2 != 1){ return false; }

            message::outer_contour::point_2d_list points;
            for (int i = 1; i+1 < argc; i+=2){
                if ((types[i] != 'f') || (types[i+1] != 'f')){ return false; }
                points.push_back(helpers::point_2d(argv[i]->f, argv[i+1]->f));
            }

            message::outer_contour * msg_ocg = new message::outer_contour(argv[0]->i32, points);
            results.push_back(msg_ocg);

            return true;
        }
    }} // ns parsers // ns internals
} // ns libkerat

/**
 * \ingroup global
 * \brief Pretty-print the \ref libkerat::message::outer_contour "outer contour" message to given output stream
 * \note Format: <tt>
 * {OSC path} {session id}[ {contour_point}]+
 * </tt>
 * \param output - output stream to write to
 * \param msg_ocg - outer contour message to print
 * \return modified output stream
 */
std::ostream & operator<<(std::ostream & output, const libkerat::message::outer_contour & msg_ocg){
    output << libkerat::message::outer_contour::PATH << " "
        << msg_ocg.get_session_id();

    const libkerat::message::outer_contour::point_2d_list & points = msg_ocg.get_contour();
    for (libkerat::message::outer_contour::point_2d_list::const_iterator i = points.begin(); i != points.end(); i++){
        output << " " << *i;
    }

    return output;
}


/**
 * \file      tuio_message_convex_hull_geometry.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-04-21 15:26 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/tuio_message_convex_hull_geometry.hpp>
#include <lo/lo.h>
#include <list>
#include <vector>
#include <cstring>
#include <lo/lo_lowlevel.h>
#include <lo/lo_osc_types.h>

namespace libkerat {
    namespace message {

        const char * convex_hull::PATH = "/tuio2/chg";

        convex_hull::convex_hull(const session_id_t session_id)
            :contact_session(session_id)
        { ; }

        convex_hull::convex_hull(const session_id_t session_id, const point_2d_list& points)
            :contact_session(session_id), m_hull(points)
        { ; }

        convex_hull::convex_hull(const convex_hull & second)
            :contact_session(second), m_hull(second.m_hull)
        { ; }

        convex_hull::~convex_hull(){ ; }

        convex_hull & convex_hull::operator=(const convex_hull & second){

            contact_session::operator=(second);
            m_hull = second.m_hull;

            return *this;
        }
        bool convex_hull::operator ==(const convex_hull & second) const {
            return (
                contact_session::operator==(second) &&
                m_hull==(second.m_hull)
            );
        }

        convex_hull::point_2d_list convex_hull::set_hull(const convex_hull::point_2d_list & points) {
            //! \todo check whether really setting convex hull

            convex_hull::point_2d_list retval = m_hull;
            m_hull = points;
            return retval;
        }

        bool convex_hull::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            lo_message msg = lo_message_new();
            lo_message_add_int32(msg, m_session_id);

            for (libkerat::message::convex_hull::point_2d_list::const_iterator i = m_hull.begin(); i != m_hull.end(); i++){
                lo_message_add_float(msg, i->get_x());
                lo_message_add_float(msg, i->get_y());
            }

            lo_bundle_add_message(target, PATH, msg);

            return true;
        }

        convex_hull * convex_hull::clone() const { return new convex_hull(*this); }

        void convex_hull::print(std::ostream & output) const { output << *this; }

        void convex_hull::move_x(coord_t factor){
            for (point_2d_list::iterator pt = m_hull.begin(); pt != m_hull.end(); ++pt){
                pt->set_x(pt->get_x() + factor);
            }
        }
        void convex_hull::move_y(coord_t factor){
            for (point_2d_list::iterator pt = m_hull.begin(); pt != m_hull.end(); ++pt){
                pt->set_y(pt->get_y() + factor);
            }
        }

        void convex_hull::scale_x(float factor, const helpers::point_2d & center){
            for (point_2d_list::iterator pt = m_hull.begin(); pt != m_hull.end(); ++pt){
                pt->set_x(center.get_x() + factor*(pt->get_x()-center.get_x()));
            }
        }
        void convex_hull::scale_y(float factor, const helpers::point_2d & center){
            for (point_2d_list::iterator pt = m_hull.begin(); pt != m_hull.end(); ++pt){
                pt->set_y(center.get_y() + factor*(pt->get_y()-center.get_y()));
            }
        }

        void convex_hull::rotate_by(angle_t angle, const helpers::point_2d & center){
            for (point_2d_list::iterator pt = m_hull.begin(); pt != m_hull.end(); ++pt){
                rotate_around_center(*pt, center, angle);
            }
        }

    } // ns message

    namespace internals { namespace parsers {
        /**
         * \brief TUIO 2.0 convex hull geometry (/tuio2/chg) parser
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
        bool parse_chg(
            std::vector<libkerat::kerat_message *> & results,
            const char * path,
            const char * types,
            lo_arg ** argv,
            int argc,
            void * user_data __attribute__((unused))
        ){
            if (strcmp(path, libkerat::message::convex_hull::PATH) != 0){ return false; }

            if (argc < 1){ return false; }
            if (types[0] != 'i'){ return false; }

            // each point si represented by 2 coordinates, + session id
            if (argc % 2 != 1){ return false; }

            message::convex_hull::point_2d_list points;
            for (int i = 1; i+1 < argc; i+=2){
                if ((types[i] != 'f') || (types[i+1] != 'f')){ return false; }
                points.push_back(helpers::point_2d(argv[i]->f, argv[i+1]->f));
            }

            message::convex_hull * msg_chg = new message::convex_hull(argv[0]->i32, points);
            results.push_back(msg_chg);

            return true;
        }
    }} // ns parsers // ns internals
} // ns libkerat

/**
 * \ingroup global
 * \brief Pretty-print the \ref libkerat::message::convex_hull "convex hull geometry" message to given output stream
 * \note Format: <tt>
 * {OSC path} {session id}[ {\ref libkerat::helpers::point_2d "point"}]*
 * </tt>
 * \param output - output stream to write to
 * \param msg_chg - convex hull geometry message to print
 * \return modified output stream
 */
std::ostream & operator<<(std::ostream & output, const libkerat::message::convex_hull & msg_chg){
    output << libkerat::message::convex_hull::PATH << " "
        << msg_chg.get_session_id();

    const libkerat::message::convex_hull::point_2d_list & points = msg_chg.get_hull();
    for (libkerat::message::convex_hull::point_2d_list::const_iterator i = points.begin(); i != points.end(); i++){
        output << " " << *i;
    }

    return output;
}

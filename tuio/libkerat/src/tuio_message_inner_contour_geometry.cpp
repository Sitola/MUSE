/**
 * \file      tuio_message_inner_contour_geometry.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-04-21 18:50 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/tuio_message_inner_contour_geometry.hpp>
#include <kerat/utils.hpp>
#include <vector>
#include <list>
#include <cstring>
#include <lo/lo.h>

namespace libkerat {
    namespace message {

        const char * inner_contour::PATH = "/tuio2/icg";

        inner_contour::inner_contour(){ ; }

        inner_contour::inner_contour(const session_id_t session_id)
            :contact_session(session_id)
        { ; }

        inner_contour::inner_contour(const session_id_t session_id, const inner_contour::contour_list & contours)
            :contact_session(session_id), m_contours(contours)
        { ; }

        inner_contour::inner_contour(const inner_contour & second)
            :contact_session(second), m_contours(second.m_contours)
        { ; }

        inner_contour::~inner_contour(){ ; }

        inner_contour & inner_contour::operator=(const inner_contour & second){

            contact_session::operator=(second);
            m_contours = second.m_contours;

            return *this;
        }
        bool inner_contour::operator ==(const inner_contour & second) const {
            return (
                contact_session::operator==(second) &&
                m_contours==(second.m_contours)
            );
        }

        inner_contour::contour_list inner_contour::set_contours(const inner_contour::contour_list & contours) {
            inner_contour::contour_list retval = m_contours;
            m_contours = contours;
            return retval;
        }

        const inner_contour::point_2d_list & inner_contour::get_contour(size_t contour_id) const throw (std::out_of_range){
            if (contour_id != get_contours_count()){
                throw std::out_of_range("No such contour!");
            }



            return *(m_contours.begin() + contour_id);
        }
        void inner_contour::add_contour(const inner_contour::point_2d_list & points){
            m_contours.push_back(points);
        }

        bool inner_contour::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            lo_message msg = lo_message_new();
            lo_message_add_int32(msg, m_session_id);

            for (libkerat::message::inner_contour::contour_list::const_iterator i = m_contours.begin(); i != m_contours.end(); i++){
                // separate individual contours
                if (i != m_contours.begin()){ lo_message_add_true(msg); }
                for (libkerat::message::inner_contour::point_2d_list::const_iterator j = i->begin(); j != i->end(); j++){
                    lo_message_add_float(msg, j->get_x());
                    lo_message_add_float(msg, j->get_y());
                }
            }

            lo_bundle_add_message(target, PATH, msg);

            return true;
        }

        inner_contour * inner_contour::clone() const { return new inner_contour(*this); }

        void inner_contour::print(std::ostream & output) const { output << *this; }

        void inner_contour::move_x(coord_t factor){
            for (contour_list::iterator cntr = m_contours.begin(); cntr != m_contours.end(); ++cntr){
                for (point_2d_list::iterator pt = cntr->begin(); pt != cntr->end(); ++pt){
                    pt->set_x(pt->get_x() + factor);
                }
            }
        }
        void inner_contour::move_y(coord_t factor){
            for (contour_list::iterator cntr = m_contours.begin(); cntr != m_contours.end(); ++cntr){
                for (point_2d_list::iterator pt = cntr->begin(); pt != cntr->end(); ++pt){
                    pt->set_y(pt->get_y() + factor);
                }
            }
        }

        void inner_contour::scale_x(float factor, const helpers::point_2d & center){
            for (contour_list::iterator cntr = m_contours.begin(); cntr != m_contours.end(); ++cntr){
                for (point_2d_list::iterator pt = cntr->begin(); pt != cntr->end(); ++pt){
                    pt->set_x(center.get_x() + factor*(pt->get_x()-center.get_x()));
                }
            }
        }
        void inner_contour::scale_y(float factor, const helpers::point_2d & center){
            for (contour_list::iterator cntr = m_contours.begin(); cntr != m_contours.end(); ++cntr){
                for (point_2d_list::iterator pt = cntr->begin(); pt != cntr->end(); ++pt){
                    pt->set_y(center.get_y() + factor*(pt->get_y()-center.get_y()));
                }
            }
        }

        void inner_contour::rotate_by(angle_t angle, const helpers::point_2d & center){
            for (contour_list::iterator cntr = m_contours.begin(); cntr != m_contours.end(); ++cntr){
                for (point_2d_list::iterator pt = cntr->begin(); pt != cntr->end(); ++pt){
                    rotate_around_center(*pt, center, angle);
                }
            }
        }

    } // ns message

    namespace internals { namespace parsers {
        /**
         * \brief TUIO 2.0 internal contour geometry (/tuio2/icg) parser
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
        bool parse_icg(
            std::vector<libkerat::kerat_message *> & results,
            const char * path,
            const char * types,
            lo_arg ** argv,
            int argc,
            void * user_data __attribute__((unused))
        ){
            if (strcmp(path, libkerat::message::inner_contour::PATH) != 0){ return false; }
            if (argc < 1){ return false; }
            if (types[0] != 'i'){ return false; }

            // each point si represented by 2 coordinates, + session id
            message::inner_contour::point_2d_list points;
            message::inner_contour::contour_list contours;

            int i = 1;
            while (i+1 < argc){
                if (types[i] == LO_TRUE){
                    if (!points.empty()){
                        contours.push_back(points);
                        points.clear();
                    }
                    ++i;
                    continue;
                }

                // malformed message
                if ((types[i] != 'f') || (types[i+1] != 'f')){ return false; }
                points.push_back(helpers::point_2d(argv[i]->f, argv[i+1]->f));
                i += 2;
            }

            if (!points.empty()){
                contours.push_back(points);
            }

            message::inner_contour * msg_icg = new message::inner_contour(argv[0]->i32, contours);
            results.push_back(msg_icg);

            return true;
        }
    }} // ns parsers // ns internals
} // ns libkerat

/**
 * \ingroup global
 * \brief Pretty-print the \ref libkerat::message::inner_contour "inner contour" message to given output stream
 * \note Format: <tt>
 * {OSC path} {session id}[[ {contour_point}]+[ TRUE {contour_point}]*]?
 * </tt>
 * \param output - output stream to write to
 * \param msg_icg - inner contour message to print
 * \return modified output stream
 */
std::ostream & operator<<(std::ostream & output, const libkerat::message::inner_contour & msg_icg){
    output << libkerat::message::inner_contour::PATH << " "
        << msg_icg.get_session_id();

    for (libkerat::message::inner_contour::contour_list::const_iterator i = msg_icg.get_contours().begin(); i != msg_icg.get_contours().end(); i++){
        // separate individual contours
        if (i != msg_icg.get_contours().begin()){
            output << " " << std::boolalpha << true;
        }
        for (libkerat::message::inner_contour::point_2d_list::const_iterator j = i->begin(); j != i->end(); j++){
            output << " " << *j;
        }
    }

    return output;
}


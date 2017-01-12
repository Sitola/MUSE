/**
 * \file      tuio_message_area_geometry.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-04-21 15:26 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/tuio_message_area_geometry.hpp>
#include <lo/lo.h>
#include <list>
#include <vector>
#include <cstring>

namespace libkerat {
    namespace message {

        const char * area::PATH = "/tuio2/arg";

        area::area(const session_id_t session_id)
            :contact_session(session_id)
        { ; }

        area::area(const session_id_t session_id, const area::span_map & spans)
            :contact_session(session_id), m_spans(spans)
        { join_spans(); }

        area::area(const area & second)
            :contact_session(second), m_spans(second.m_spans)
        { ; }

        area::~area(){ ; }

        area & area::operator=(const area & second){

            contact_session::operator=(second);
            m_spans = second.m_spans;

            return *this;
        }
        bool area::operator ==(const area & second) const {
            return (
                contact_session::operator==(second) &&
                m_spans==(second.m_spans)
            );
        }

        area::span_map area::set_spans(const area::span_map & spans) {
            area::span_map retval = m_spans;
            m_spans = spans;
            join_spans();
            return retval;
        }

        void area::join_spans(){
            typedef libkerat::message::area::span_map span_map;

            span_map::iterator current = m_spans.begin();

            while (current != m_spans.end()){
                span_map::iterator next = current; ++next;
                // check whether this is not the last one
                if (next == m_spans.end()){ current = next; continue; }

                // done with this line?
                if (current->first.get_y() != next->first.get_y()){
                    current = next;
                    continue;
                }

                // if the next span intersects with this one, join them
                if ((current->first.get_x() + current->second) >= next->first.get_x()){
                    current->second = (next->first.get_x() - current->first.get_x()) + next->second;
                    m_spans.erase(next);
                    continue;
                }

                current = next;
            }
        }

        size_t area::get_points_count() const {
            size_t count = 0;

            for (libkerat::message::area::span_map::const_iterator i = m_spans.begin(); i != m_spans.end(); i++){
                count += ceil(i->second);
            }

            return count;
        }

        bool area::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            lo_message msg = lo_message_new();
            lo_message_add_int32(msg, m_session_id);

            for (libkerat::message::area::span_map::const_iterator i = m_spans.begin(); i != m_spans.end(); i++){
                lo_message_add_float(msg, i->first.get_x());
                lo_message_add_float(msg, i->first.get_y());
                lo_message_add_float(msg, i->second);
            }

            lo_bundle_add_message(target, PATH, msg);

            return true;
        }

        area * area::clone() const { return new area(*this); }

        void area::print(std::ostream & output) const { output << *this; }

        void area::scale_x(float factor){
            span_map spans;
            for (span_map::iterator segment = m_spans.begin(); segment != m_spans.end(); ++segment){
                spans.insert(span_map::value_type(
                    helpers::point_2d(segment->first.get_x()*factor, segment->first.get_y()),
                    factor*(segment->second)
                ));
            }
            m_spans = spans;
        }

        void area::scale_y(float factor){
            span_map spans;
            for (span_map::iterator segment = m_spans.begin(); segment != m_spans.end(); ++segment){
                spans.insert(span_map::value_type(
                    helpers::point_2d(segment->first.get_x(), segment->first.get_y()*factor),
                    segment->second
                ));
            }
            m_spans = spans;
        }

    } // ns message

    namespace internals { namespace parsers {
        /**
         * \brief TUIO 2.0 area geometry (/tuio2/arg) parser
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
        bool parse_arg(
            std::vector<libkerat::kerat_message *> & results,
            const char * path,
            const char * types,
            lo_arg ** argv,
            int argc,
            void * user_data __attribute__((unused))
        ){
            if (strcmp(path, libkerat::message::area::PATH) != 0){ return false; }
            if (argc < 1){ return false; }
            if (types[0] != 'i'){ return false; }

            message::area::span_map spans;
            int i = 1;
            while (i+2 < argc){
                // check for mallformed type
                if (strncmp(types+i, "fff", 3) != 0){ return false; }
                spans.insert(message::area::span_map::value_type(libkerat::helpers::point_2d(argv[i]->f, argv[i+1]->f), argv[i+2]->f));
                i+=3;
            }

            message::area * msg_arg = new message::area(argv[0]->i32, spans);
            results.push_back(msg_arg);

            return true;
        }
    }} // ns parsers // ns internals
} // ns libkerat

/**
 * \ingroup global
 * \brief Pretty-print the \ref libkerat::message::area "area geometry" message to given output stream
 * \note Format: <tt>{OSC path} {container session id}[
 * ({\ref libkerat::helpers::point_2d "point"} -> {width})]+</tt>
 * \param output - output stream to write to
 * \param msg_arg - container associations message to print
 * \return modified output stream
 * \see operator<<(std::ostream &, const libkerat::helpers::point_2d &)
 */
std::ostream & operator<<(std::ostream & output, const libkerat::message::area & msg_arg){
    output << libkerat::message::area::PATH << " "
        << msg_arg.get_session_id();

    const libkerat::message::area::span_map & spans = msg_arg.get_spans();
    for (libkerat::message::area::span_map::const_iterator i = spans.begin(); i != spans.end(); i++){
        output << " (" << i->first << " -> " << i->second << ")";
    }

    return output;
}


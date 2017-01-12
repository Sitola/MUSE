/**
 * \file      tuio_message_skeleton_volume_geometry.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-04-21 15:26 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/tuio_message_skeleton_volume_geometry.hpp>
#include <lo/lo.h>
#include <list>
#include <vector>
#include <cstring>

namespace libkerat {
    namespace message {

        const char * skeleton_volume::PATH = "/tuio2/svg";

        skeleton_volume::skeleton_volume(const session_id_t session_id)
            :contact_session(session_id)
        { ; }

        skeleton_volume::skeleton_volume(const session_id_t session_id, const skeleton_volume::radius_list& radiuses)
            :contact_session(session_id), m_radiuses(radiuses)
        { ; }

        skeleton_volume::skeleton_volume(const skeleton_volume & second)
            :contact_session(second), m_radiuses(second.m_radiuses)
        { ; }

        skeleton_volume::~skeleton_volume(){ ; }

        skeleton_volume & skeleton_volume::operator=(const skeleton_volume & second){

            contact_session::operator=(second);
            m_radiuses = second.m_radiuses;

            return *this;
        }
        bool skeleton_volume::operator ==(const skeleton_volume & second) const {
            return (
                contact_session::operator==(second) &&
                m_radiuses==(second.m_radiuses)
            );
        }

        skeleton_volume::skeleton_volume::radius_list skeleton_volume::set_radiuses(const skeleton_volume::skeleton_volume::radius_list & radiuses) {
            skeleton_volume::skeleton_volume::radius_list retval = m_radiuses;
            m_radiuses = radiuses;
            return retval;
        }

        bool skeleton_volume::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            lo_message msg = lo_message_new();
            lo_message_add_int32(msg, m_session_id);

            for (libkerat::message::skeleton_volume::skeleton_volume::radius_list::const_iterator i = m_radiuses.begin(); i != m_radiuses.end(); i++){
                lo_message_add_float(msg, *i);
            }

            lo_bundle_add_message(target, PATH, msg);

            return true;
        }

        skeleton_volume * skeleton_volume::clone() const { return new skeleton_volume(*this); }

        void skeleton_volume::print(std::ostream & output) const { output << *this; }

    } // ns message

    namespace internals { namespace parsers {
        /**
         * \brief TUIO 2.0 skeleton volume geometry (/tuio2/svg) parser
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
        bool parse_svg(
            std::vector<libkerat::kerat_message *> & results,
            const char * path,
            const char * types,
            lo_arg ** argv,
            int argc,
            void * user_data __attribute__((unused))
        ){
            if (strcmp(path, libkerat::message::skeleton_volume::PATH) != 0){ return false; }
            if (argc < 1){ return false; }
            if (types[0] != 'i'){ return false; }

            message::skeleton_volume::skeleton_volume::radius_list radiuses;
            for (int i = 1; i < argc; ++i){
                if (types[i] != 'f'){ return false; }
                radiuses.push_back(argv[i]->f);
            }

            message::skeleton_volume * msg_svg = new message::skeleton_volume(argv[0]->i32, radiuses);
            results.push_back(msg_svg);

            return true;
        }
    }} // ns parsers // ns internals
} // ns libkerat

/**
 * \ingroup global
 * \brief Pretty-print the \ref libkerat::message::skeleton_volume "skeleton volume geometry" message to given output stream
 * \note Format: <tt>
 * {OSC path} {session id}[ {radius}]*
 * </tt>
 * \param output - output stream to write to
 * \param msg_svg - skeleton volume geometry message to print
 * \return modified output stream
 */
std::ostream & operator<<(std::ostream & output, const libkerat::message::skeleton_volume & msg_svg){
    output << libkerat::message::skeleton_volume::PATH << " "
        << msg_svg.get_session_id();

    const libkerat::message::skeleton_volume::skeleton_volume::radius_list & radiuses = msg_svg.get_radiuses();
    for (libkerat::message::skeleton_volume::skeleton_volume::radius_list::const_iterator i = radiuses.begin(); i != radiuses.end(); i++){
        output << " " << *i;
    }

    return output;
}


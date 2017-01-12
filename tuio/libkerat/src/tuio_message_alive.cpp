/**
 * \file      tuio_message_alive.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-09-26 23:51 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/tuio_message_alive.hpp>
#include <cstring>
#include <lo/lo.h>
#include <list>
#include <vector>
#include <algorithm>

namespace libkerat {
    namespace message {

        const char * alive::PATH = "/tuio2/alv";

        alive::alive(const alive & original){
            m_alives = original.m_alives;
        }

        alive::alive(const alive_ids & alives_to_add)
            :m_alives(alives_to_add)
        { ; }

        alive & alive::operator =(const alive& second){
            m_alives = second.m_alives;
            return *this;
        }

        bool alive::operator ==(const alive& second) const {
            return (m_alives == second.m_alives);
        }

        alive::alive_ids alive::set_alives(const alive::alive_ids & alives){
            alive_ids retval = m_alives;
            m_alives = alives;
            return retval;
        }

        // alv to tuio/net
        bool alive::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            lo_message msg = lo_message_new();

            alive_ids::const_iterator end   = m_alives.end();

            for (alive_ids::const_iterator i = m_alives.begin(); i != end; i++){
                lo_message_add_int32(msg, *i);
            }

            lo_bundle_add_message(target, PATH, msg);

            return true;
        }

        kerat_message * alive::clone() const { return new alive(*this); }

        void alive::print(std::ostream & output) const { output << *this; }

    }

    // alv parser
    namespace internals { namespace parsers {
        /**
         * \brief TUIO 2.0 alive (/tuio2/alv) parser
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
        bool parse_alv(
            std::vector<libkerat::kerat_message *> & results,
            const char * path,
            const char * types,
            lo_arg ** argv,
            int argc,
            void * user_data __attribute__((unused))
        ){
            if (strcmp(path, libkerat::message::alive::PATH) != 0){ return false; }
            libkerat::message::alive::alive_ids alives;
            for (int i = 0; i < argc; i++){
                if (types[i] != 'i'){
                    return false;
                }
                alives.insert(argv[i]->i);
            }
            libkerat::message::alive * msg_alive = new libkerat::message::alive(alives);
            results.push_back(msg_alive);

            return true;
        }
    } } // ns parsers // ns internal
} // ns libkerat

/**
 * \ingroup global
 * \brief Pretty-print the \ref libkerat::message::alive "alive" message to given output stream
 * \note Format: <tt>{OSC path}[ {session id}]+</tt>
 * \param output - output stream to write to
 * \param msg_alv - alive message to print
 * \return modified output stream
 */
std::ostream & operator<<(std::ostream & output, const libkerat::message::alive & msg_alv){
    output << libkerat::message::alive::PATH;

    libkerat::message::alive::alive_ids alives = msg_alv.get_alives();
    for (libkerat::message::alive::alive_ids::const_iterator alive = alives.begin(); alive != alives.end(); alive++){
        output << " " << *alive;
    }

    return output;
}


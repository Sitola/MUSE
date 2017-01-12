/**
 * \file      tuio_message_alive_associations.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-11-19 19:40 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/tuio_message_alive_associations.hpp>
#include <cstring>
#include <lo/lo.h>
#include <vector>
#include <list>

namespace libkerat {
    namespace message {

        const char * alive_associations::PATH = "/tuio2/ala";

        alive_associations::alive_associations(const alive_associations& original){
            m_associations = original.m_associations;
        }

        alive_associations::alive_associations(const associated_ids & associations_to_add)
            :m_associations(associations_to_add)
        { ; }

        alive_associations & alive_associations::operator =(const alive_associations& second){ m_associations = second.m_associations; return *this; }
        bool alive_associations::operator ==(const alive_associations& second) const { return m_associations == second.m_associations; }

        alive_associations::associated_ids alive_associations::set_associations(const associated_ids& associations){
            associated_ids retval = m_associations;
            m_associations = associations;
            return retval;
        }

        kerat_message * alive_associations::clone() const { return new alive_associations(*this); }

        void alive_associations::print(std::ostream & output) const { output << *this; }

        bool alive_associations::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            lo_message msg = lo_message_new();

            associated_ids::const_iterator end   = m_associations.end();

            for (associated_ids::const_iterator i = m_associations.begin(); i != end; i++){
                lo_message_add_int32(msg, *i);
            }

            lo_bundle_add_message(target, PATH, msg);

            return true;
        }
    } // ns message

    // ala parser
    namespace internals { namespace parsers {
        /**
         * \brief TUIO 2.0 alive associations (/tuio2/ala) parser
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
        bool parse_ala(
            std::vector<libkerat::kerat_message *> & results,
            const char * path,
            const char * types,
            lo_arg ** argv,
            int argc,
            void * user_data __attribute__((unused))
        ){
            if (strcmp(path, libkerat::message::alive_associations::PATH) != 0){ return false; }
            libkerat::message::alive_associations::associated_ids associations;
            for (int i = 0; i < argc; i++){
                if (types[i] != 'i'){
                    return false;
                }
                associations.insert(argv[i]->i);
            }
            libkerat::message::alive_associations * msg_associations = new libkerat::message::alive_associations(associations);
            results.push_back(msg_associations);

            return true;
        }
    } } // ns parsers // ns internal
} // ns libkerat

/**
 * \ingroup global
 * \brief Pretty-print the \ref libkerat::message::alive_associations "alive_associations" message to given output stream
 * \note Format: <tt>{OSC path}[ {session id}]+</tt>
 * \param output - output stream to write to
 * \param msg_ala - alive associations message to print
 * \return modified output stream
 */
std::ostream & operator<<(std::ostream & output, const libkerat::message::alive_associations & msg_ala){
    output << libkerat::message::alive_associations::PATH;

    libkerat::message::alive_associations::associated_ids associations = msg_ala.get_associations();
    for (libkerat::message::alive_associations::associated_ids::const_iterator association = associations.begin(); association != associations.end(); association++){
        output << " " << *association;
    }

    return output;
}

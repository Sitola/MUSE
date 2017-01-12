/**
 * \file      tuio_message_signal.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-09-26 23:51 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/tuio_message_signal.hpp>
#include <lo/lo.h>
#include <set>
#include <list>
#include <vector>
#include <cstring>

namespace libkerat {
    namespace message {

        const char * signal::PATH = "/tuio2/sig";

        signal::signal(){ ; }

        signal::signal(const signal & original)
            :contact_session(original.m_session_id), contact_component(original.m_component_id)
        { m_targets = original.m_targets; }

        signal::signal(const session_id_t session_id, const component_id_t& component_id, const target_ids& targets_to_add)
            :contact_session(session_id), contact_component(component_id)
        { set_targets(targets_to_add); }

        signal::signal(const session_id_t session_id, const component_id_t& component_id)
            :contact_session(session_id), contact_component(component_id)
        { ; }

        signal & signal::operator=(const signal & second){

            contact_session::operator=(second);
            contact_component::operator=(second);

            m_targets = second.m_targets;

            return *this;
        }
        bool signal::operator ==(const signal& second) const {
            return (
                contact_session::operator==(second) &&
                contact_component::operator==(second) &&

                (m_targets == second.m_targets)
            );
        }

        signal::target_ids signal::set_targets(const signal::target_ids & targets){
            target_ids retval = m_targets;
            m_targets = targets;
            return retval;
        }

        kerat_message * signal::clone() const { return new signal(*this); }

        void signal::print(std::ostream & output) const { output << *this; }

        bool signal::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            lo_message msg = lo_message_new();
            lo_message_add_int32(msg, m_session_id);
            lo_message_add_int32(msg, m_component_id);

            target_ids::const_iterator end   = m_targets.end();

            for (target_ids::const_iterator i = m_targets.begin(); i != end; i++){
                lo_message_add_int32(msg, *i);
            }

            lo_bundle_add_message(target, PATH, msg);

            return true;
        }
    } // ns message

    namespace internals { namespace parsers {
        /**
         * \brief TUIO 2.0 signal (/tuio2/sig) parser
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
        bool parse_sig(
            std::vector<libkerat::kerat_message *> & results,
            const char * path,
            const char * types,
            lo_arg ** argv,
            int argc,
            void * user_data __attribute__((unused))
        ){
            if (strcmp(path, libkerat::message::signal::PATH) != 0){ return false; }
            if (argc < 3){ return false; }
            if (strncmp(types, "ii", 2) != 0){ return false; }

            libkerat::message::signal::target_ids targets;

            for (int i = 2; i < argc; i++){
                if (types[i] != 'i'){ return false; }
                targets.insert(argv[i]->i);
            }

            libkerat::message::signal * msg_signal = new libkerat::message::signal(argv[0]->i, argv[1]->i, targets);
            results.push_back(msg_signal);
            return true;
        }
    }} // ns parsers // ns internals
} // ns libkerat

/**
 * \ingroup global
 * \brief Pretty-print the \ref libkerat::message::signal "signal" message to given output stream
 * \note Format: <tt>
 * {OSC path} {session id} {component id (signal)} [ {target session id}]+
 * </tt>
 * \param output - output stream to write to
 * \param msg_sig - signal message to print
 * \return modified output stream
 */
std::ostream & operator<<(std::ostream & output, const libkerat::message::signal & msg_sig){
    output << libkerat::message::signal::PATH << " " << msg_sig.get_session_id() << " " << msg_sig.get_component_id();

    libkerat::message::signal::target_ids targets = msg_sig.get_targets();
    for (libkerat::message::signal::target_ids::const_iterator target = targets.begin(); target != targets.end(); target++){
        output << " " << *target;
    }

    return output;
}


/**
 * \file      tuio_message_container_association.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-09-26 23:51 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/tuio_message_container_association.hpp>
#include <lo/lo.h>
#include <list>
#include <vector>
#include <cstring>

namespace libkerat {
    namespace message {

        const char * container_association::PATH = "/tuio2/coa";

        container_association::container_association()
            :m_slot(0)
        { ; }

        container_association::container_association(const container_association& original)
            :contact_session(original), m_slot(original.m_slot)
        { m_associations = original.m_associations; }

        container_association::container_association(const session_id_t session_id, const slot_t slot_id, const associated_ids & associations)
            :contact_session(session_id), m_slot(slot_id), m_associations(associations)
        { ; }

        container_association::container_association(const session_id_t session_id, const slot_t slot_id)
            :contact_session(session_id), m_slot(slot_id)
        { ; }

        container_association & container_association::operator=(const container_association& original){
            contact_session::operator=(original);
            m_slot = original.m_slot;
            m_associations = original.m_associations;
            return *this;
        }

        bool container_association::operator ==(const container_association& second) const {
            return (
                contact_session::operator==(second) &&
                (m_slot == second.m_slot) &&
                (m_associations == second.m_associations)
            );
        }

        container_association::associated_ids container_association::set_associations(const associated_ids& associations){
            associated_ids retval = m_associations;
            m_associations = associations;
            return retval;
        }

        slot_t container_association::set_slot(const slot_t slot) {
            slot_t oldval = m_slot;
            m_slot = slot;
            return oldval;
        }

        bool container_association::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            lo_message msg = lo_message_new();
            lo_message_add_int32(msg, get_session_id());
            lo_message_add_int32(msg, get_slot());

            for (associated_ids::const_iterator i = m_associations.begin(); i != m_associations.end(); i++){
                lo_message_add_int32(msg, *i);
            }

            lo_bundle_add_message(target, PATH, msg);

            return true;
        }

        kerat_message * container_association::clone() const { return new container_association (*this); }

        void container_association::print(std::ostream & output) const { output << *this; }

    } // ns message

    namespace internals { namespace parsers {
        /**
         * \brief TUIO 2.0 container association (/tuio2/coa) parser
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
        bool parse_coa(
            std::vector<libkerat::kerat_message *> & results,
            const char * path,
            const char * types,
            lo_arg ** argv,
            int argc,
            void * user_data __attribute__((unused))
        ){
            if (strcmp(path, libkerat::message::container_association::PATH) != 0){ return false; }
            if (argc < 3){ return false; }
            if (strncmp(types, "ii", 2) != 0){ return false; }

            libkerat::message::container_association::associated_ids associations;

            for (int i = 2; i < argc; i++){
                if (types[i] != 'i'){
                    return false;
                }
                associations.insert(argv[i]->i);
            }
            libkerat::message::container_association * msg_associations = new libkerat::message::container_association(argv[0]->i, argv[1]->i, associations);
            results.push_back(msg_associations);
            return true;
        } // fn parse_coa
    } } // ns parsers // ns internal

} // ns libkerat

/**
 * \ingroup global
 * \brief Pretty-print the \ref libkerat::message::container_association "container_association" message to given output stream
 * \note Format: <tt>{OSC path}{container session id} {container slot id}[ {associated session id}]+</tt>
 * \param output - output stream to write to
 * \param msg_coa - container associations message to print
 * \return modified output stream
 */
std::ostream & operator<<(std::ostream & output, const libkerat::message::container_association & msg_coa){
    output << libkerat::message::container_association::PATH << " " << msg_coa.get_session_id() << " "
        << msg_coa.get_slot();

    libkerat::message::container_association::associated_ids associations = msg_coa.get_associations();
    for (libkerat::message::container_association::associated_ids::const_iterator association = associations.begin(); association != associations.end(); association++){
        output << " " << *association;
    }

    return output;
}

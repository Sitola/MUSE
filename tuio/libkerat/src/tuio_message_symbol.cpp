/**
 * \file      tuio_message_symbol.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-10-30 23:31 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/tuio_message_symbol.hpp>
#include <kerat/utils.hpp>
#include <lo/lo.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace libkerat {

    namespace message {

        const char * symbol::PATH = "/tuio2/sym";

        // string
        void symbol::set_data(const std::string & data_str){
            // safe cleanup
            set_data(data_str.length()+1, data_str.c_str());
            m_type = DATA_TYPE_STRING;

        }

        // blob
        void symbol::set_data(const uint32_t data_len, const void* data_buffer){

            do_safe_dealloc();

            m_data_length = data_len;
            m_type = DATA_TYPE_BLOB;

            if (m_data_length > 0){
                m_data = new uint8_t[m_data_length];
                memcpy(m_data, data_buffer, m_data_length);
            }
        }

        void symbol::do_safe_dealloc(){

            if (m_data != NULL){
                delete m_data;
                m_data = NULL;
            }

            m_data = NULL;
            m_type = DATA_TYPE_STRING;
            m_data_length = 0;

        }

        // empty
        symbol::symbol()
            :m_data(NULL)
        {
            set_group("");
            do_safe_dealloc();
        }

        // string
        symbol::symbol(const session_id_t session_id, const type_id_t type_id, const user_id_t user_id, const component_id_t component_id, const std::string& grp, const std::string& data_str)
            :contact_session(session_id), contact_type_user(type_id, user_id), contact_component(component_id), m_data(NULL)
        {
            set_group(grp);
            set_data(data_str);
        }

        // blob
        symbol::symbol(const session_id_t session_id, const type_id_t type_id, const user_id_t user_id, const component_id_t component_id, const std::string& grp, const uint32_t data_len, const void* data_val)
            :contact_session(session_id), contact_type_user(type_id, user_id), contact_component(component_id), m_data(NULL)
        {
            set_group(grp);
            set_data(data_len, data_val);
        }

        // copy
        symbol::symbol(const symbol & original)
            :contact_session(original.m_session_id), contact_type_user(original.m_type_id, original.m_user_id), contact_component(original.m_component_id),
            m_type(original.m_type), m_data_length(original.m_data_length), m_data(NULL)
        {
            m_group = original.m_group;
            set_data(original.m_data_length, original.m_data);
            m_type = original.m_type;
        }

        symbol::~symbol(){
            do_safe_dealloc();
        }

        symbol & symbol::operator=(const symbol & orig){
            if (this != &orig){
                contact_session::operator =(orig);
                contact_type_user::operator =(orig);
                contact_component::operator =(orig);
                m_group = orig.m_group;
                set_data(orig.m_data_length, orig.m_data);
                m_type = orig.m_type;
            }
            return *this;
        }

        kerat_message * symbol::clone() const { return new symbol(*this); }

        void symbol::print(std::ostream & output) const { output << *this; }

        bool symbol::operator ==(const symbol & second) const {
            return (
                (contact_session::operator ==(second)) &&
                (contact_type_user::operator ==(second)) &&
                (contact_component::operator == (second)) &&
                // data identity check
                (m_type == second.m_type) &&
                (m_data_length == second.m_data_length) &&
                (m_group.compare(second.m_group) == 0) &&
                (memcmp(m_data, second.m_data, m_data_length) == 0)
            );
        }

        bool symbol::operator !=(const symbol & second) const {
            return !operator==(second);
        }

        bool symbol::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            libkerat::tu_id_t tu_id = libkerat::compile_tuid(get_type_id(), get_user_id());

            lo_message msg = lo_message_new();

            lo_message_add_int32(msg, get_session_id());
            lo_message_add_int32(msg, tu_id);
            lo_message_add_int32(msg, get_component_id());

            lo_message_add_string(msg, m_group.c_str());

            switch (m_type){
                case DATA_TYPE_BLOB: {
                    lo_blob b = lo_blob_new(m_data_length, m_data);
                    lo_message_add_blob(msg, b);
                    break;
                }
                case DATA_TYPE_STRING: {
                    lo_message_add_string(msg, (const char *)m_data);
                    break;
                }
            }

            lo_bundle_add_message(target, PATH, msg);

            return true;

        }
    } // ns message

    namespace internals { namespace parsers {
        /**
         * \brief TUIO 2.0 symbol (/tuio2/sym) parser
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
        bool parse_sym(
            std::vector<libkerat::kerat_message *> & results,
            const char * path,
            const char * types,
            lo_arg ** argv,
            int argc __attribute__((unused)),
            void * user_data __attribute__((unused))
        ){
            if (strcmp(path, libkerat::message::symbol::PATH) != 0){ return false; }
            const char * typestring_string = "iiiss";
            const char * typestring_blob   = "iiisb";

            const char * type = (strcmp(types, typestring_string) == 0)?typestring_string:((strcmp(types, typestring_blob) == 0)?typestring_blob:NULL);

            if (type == NULL){ return false; }

            libkerat::message::symbol * msg_symbol = new libkerat::message::symbol;

            msg_symbol->set_session_id(argv[0]->i);
            msg_symbol->set_tu_id(argv[1]->i);
            msg_symbol->set_component_id(argv[2]->i);
            msg_symbol->set_group(&(argv[3]->s));

            if (type == typestring_string){
                msg_symbol->set_data(&(argv[4]->s));
            } else {
                uint8_t * blob_data = (uint8_t *)lo_blob_dataptr(argv[4]);
                size_t blob_length = lo_blob_datasize(argv[4]);
                msg_symbol->set_data(blob_length, blob_data);
            }

            results.push_back(msg_symbol);

            return true;
        }

    }} // ns parsers // ns internals
} // ns libkerat

/**
 * \ingroup global
 * \brief Pretty-print the \ref libkerat::message::symbol "symbol" message to given output stream
 * \note Format for string data: <tt>
 * {OSC path} {session id} {type id}/{user id} {component id} {symbol group} \"{data}\"
 * </tt>
 * \note Format for blob data: <tt>
 * {OSC path} {session id} {type id}/{user id} {component id} {symbol group} \<blob/{count of samples}/0x{hexadecimal data}\>
 * </tt>
 * \param output - output stream to write to
 * \param msg_sym - symbol message to print
 * \return modified output stream
 */
std::ostream & operator<<(std::ostream & output, const libkerat::message::symbol & msg_sym){
    output << libkerat::message::symbol::PATH << " " << msg_sym.get_session_id() << " "
        << msg_sym.get_user_id() << "/" << msg_sym.get_type_id() << " "
        << msg_sym.get_component_id() << " "
        << msg_sym.get_group();

    if (msg_sym.get_data_type() == libkerat::message::symbol::DATA_TYPE_BLOB){
        const uint8_t * data = (const uint8_t *)msg_sym.get_data();
        output << " <blob/" << msg_sym.get_data_length() << "/0x";
        char prevfill = output.fill('0');
        int prevwidth = output.width(2);
        for (size_t bs = 0; bs < msg_sym.get_data_length(); bs++){
            output << std::hex << data[bs];
        }
        output.fill(prevfill);
        output.width(prevwidth);

        output << ">";
    } else {
        output << " \"" << (const char *)msg_sym.get_data() << "\"";
    }

    return output;
}

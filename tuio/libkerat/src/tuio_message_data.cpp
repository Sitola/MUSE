/**
 * \file      tuio_message_data.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-11-15 23:15 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/tuio_message_data.hpp>
#include <lo/lo.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace libkerat {

    namespace message {

        const char * data::PATH = "/tuio2/dat";

        // string
        void data::set_data(const std::string & data_str){
            // safe cleanup
            set_data(data_str.length()+1, data_str.c_str());
            m_type = DATA_TYPE_STRING;

        }

        // blob
        void data::set_data(const uint32_t data_len, const void* data_buffer){

            do_safe_dealloc();

            m_data_length = data_len;
            m_type = DATA_TYPE_BLOB;

            if (m_data_length > 0){
                m_data = new uint8_t[m_data_length];
                memcpy(m_data, data_buffer, m_data_length);
            }
        }

        void data::do_safe_dealloc(){

            if (m_data != NULL){
                delete m_data;
                m_data = NULL;
            }

            m_data = NULL;
            m_type = DATA_TYPE_STRING;
            m_data_length = 0;

        }

        // empty
        data::data()
            :m_data(NULL)
        {
            set_mime_type("text/plain");
            do_safe_dealloc();
        }

        // string
        data::data(const session_id_t session_id, const std::string& data_str, const std::string & mime)
            :contact_session(session_id), m_data(NULL)
        {
            set_mime_type(mime);
            set_data(data_str);
        }

        // blob
        data::data(const session_id_t session_id, const uint32_t data_len, const void* data_val, const std::string & mime)
            :contact_session(session_id), m_data(NULL)
        {
            set_mime_type(mime);
            set_data(data_len, data_val);
        }

        // copy
        data::data(const data & original)
            :contact_session(original.m_session_id), m_type(original.m_type), m_data_length(original.m_data_length), m_data(NULL)
        {
            m_mime_type = original.m_mime_type;
            set_data(original.m_data_length, original.m_data);
            m_type = original.m_type;

        }

        data::~data(){
            do_safe_dealloc();
        }

        data & data::operator=(const data & orig){
            if (this != &orig){
                contact_session::operator=(orig);
                m_mime_type = orig.m_mime_type;
                set_data(orig.m_data_length, orig.m_data);
                m_type = orig.m_type;
            }
            return *this;
        }

        bool data::operator ==(const data & second) const {
            return (
                contact_session::operator==(second) &&
                // data identity check
                (m_type == second.m_type) &&
                (m_data_length == second.m_data_length) &&
                (m_mime_type.compare(second.m_mime_type) == 0) &&
                (memcmp(m_data, second.m_data, m_data_length) == 0)
            );
        }

        bool data::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            lo_message msg = lo_message_new();

            lo_message_add_int32(msg, m_session_id);
            lo_message_add_string(msg, m_mime_type.c_str());

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

        kerat_message * data::clone() const { return new data(*this); }

        void data::print(std::ostream & output) const { output << *this; }

    } // ns message

    namespace internals { namespace parsers {
        /**
         * \brief TUIO 2.0 data (/tuio2/dat) parser
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
        bool parse_dat(
            std::vector<libkerat::kerat_message *> & results,
            const char * path,
            const char * types,
            lo_arg ** argv,
            int argc __attribute__((unused)),
            void * user_data __attribute__((unused))
        ){
            if (strcmp(path, libkerat::message::data::PATH) != 0){ return false; }
            const char * typestring_string = "iss";
            const char * typestring_blob   = "isb";

            const char * type = (strcmp(types, typestring_string) == 0)?typestring_string:((strcmp(types, typestring_blob) == 0)?typestring_blob:NULL);

            if (type == NULL){ return false; }

            libkerat::message::data * msg_data = new libkerat::message::data;

            msg_data->set_session_id(argv[0]->i);
            msg_data->set_mime_type(&(argv[1]->s));

            if (type == typestring_string){
                msg_data->set_data(&(argv[2]->s));
            } else {
                uint8_t * blob_data = (uint8_t *)lo_blob_dataptr(argv[2]);
                size_t blob_length = lo_blob_datasize(argv[2]);
                msg_data->set_data(blob_length, blob_data);
            }

            results.push_back(msg_data);

            return true;
        }
    }} // ns parsers // ns internals
} // ns libkerat

/**
 * \ingroup global
 * \brief Pretty-print the \ref libkerat::message::data "data" message to given output stream
 * \note Format for string data: <tt>
 * {OSC path} {session id} {mime type} \"{data}\"
 * </tt>
 * \note Format for blob data: <tt>
 * {OSC path} {session id} {mime type} \<blob/{data_length}/0x{hexadecimal data}\>
 * </tt>
 * \param output - output stream to write to
 * \param msg_dat - data message to print
 * \return modified output stream
 */
std::ostream & operator<<(std::ostream & output, const libkerat::message::data & msg_dat){
    output << libkerat::message::data::PATH << " " << msg_dat.get_session_id() << " "
        << msg_dat.get_mime_type();

    if (msg_dat.get_data_type() == libkerat::message::data::DATA_TYPE_BLOB){

        output << " <blob/" << msg_dat.get_data_length() << "/0x";
        char prevfill = output.fill('0');
        int prevwidth = output.width(2);

        const uint8_t * data_begin = (const uint8_t *)msg_dat.get_data();
        const uint8_t * data_end = data_begin + msg_dat.get_data_length();

        while (data_begin != data_end){
            output << std::hex << *data_begin;
            ++data_begin;
        }

        output.fill(prevfill);
        output.width(prevwidth);

        output << ">";
    } else {
        output << " \"" << (const char *)msg_dat.get_data() << "\"";
    }

    return output;
}

/**
 * \file      tuio_message_raw.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-11-15 23:15 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/tuio_message_raw.hpp>
#include <lo/lo.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace libkerat {

    namespace message {

        const char * raw::PATH = "/tuio2/raw";

        void raw::set_samples(const uint32_t samples_count, const uint8_t* samples){

            do_safe_dealloc();

            m_samples_count = samples_count;

            if (m_samples_count > 0){
                m_samples = new uint8_t[m_samples_count];
                memcpy(m_samples, samples, m_samples_count);
            }
        }

        void raw::do_safe_dealloc(){
            if (m_samples != NULL){
                delete m_samples;
                m_samples = NULL;
            }

            m_samples = NULL;
            m_samples_count = 0;
        }

        // empty
        raw::raw()
            :m_width(1), m_samples_count(0), m_samples(NULL)
        { ; }

        raw::raw(const session_id_t session_id, const distance_t width, const size_t samples_count, const uint8_t* samples)
            :contact_session(session_id), m_width(width), m_samples_count(0), m_samples(NULL)
        {
            set_samples(samples_count, samples);
        }

        // copy
        raw::raw(const raw & original)
            :contact_session(original), m_width(original.m_width), m_samples_count(0), m_samples(NULL)
        {
            set_samples(original.m_samples_count, original.m_samples);
        }

        raw::~raw(){
            do_safe_dealloc();
        }

        raw & raw::operator=(const raw & orig){
            if (this != &orig){
                contact_session::operator=(orig);
                //m_samples_count == orig.m_samples_count;
                set_samples(orig.m_samples_count, orig.m_samples);
            }
            return *this;
        }

        bool raw::operator ==(const raw & second) const {
            return (
                contact_session::operator==(second) &&
                (m_width == second.m_width) &&
                (m_samples_count == second.m_samples_count) &&
                (memcmp(m_samples, second.m_samples, m_samples_count) == 0)
            );
        }

        distance_t raw::set_sample_distance(distance_t sample_distance) {
            distance_t oldval = m_width;
            m_width = sample_distance;
            return oldval;
        }

        bool raw::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            lo_message msg = lo_message_new();

            lo_message_add_int32(msg, m_session_id);
            lo_message_add_float(msg, m_width);

            lo_blob b = lo_blob_new(m_samples_count, m_samples);
            lo_message_add_blob(msg, b);

            lo_bundle_add_message(target, PATH, msg);

            return true;

        }

        kerat_message * raw::clone() const { return new raw(*this); }

        void raw::print(std::ostream & output) const { output << *this; }

    } // ns message

    namespace internals { namespace parsers {
        /**
         * \brief TUIO 2.0 raw (/tuio2/raw) parser
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
        bool parse_raw(
            std::vector<libkerat::kerat_message *> & results,
            const char * path,
            const char * types,
            lo_arg ** argv,
            int argc __attribute__((unused)),
            void * user_raw __attribute__((unused))
        ){
            if (strcmp(path, libkerat::message::raw::PATH) != 0){ return false; }
            if (strcmp(types, "ifb") != 0){ return false; }
            uint8_t * blob_data = (uint8_t *)lo_blob_dataptr(argv[2]);
            size_t blob_length = lo_blob_datasize(argv[2]);

            libkerat::message::raw * msg_raw = new libkerat::message::raw(
                argv[0]->i,
                argv[1]->f,
                blob_length,
                blob_data
            );
            results.push_back(msg_raw);

            return true;
        }
    }} // ns parsers // ns internals
} // ns libkerat

/**
 * \ingroup global
 * \brief Pretty-print the \ref libkerat::message::raw "raw" message to given output stream
 * \note Format: <tt>
 * {OSC path} {session id} {sample distance} \<blob/{count of samples}/0x{hexadecimal data}\>
 * </tt>
 * \param output - output stream to write to
 * \param msg_raw - raw message to print
 * \return modified output stream
 */
std::ostream & operator<<(std::ostream & output, const libkerat::message::raw & msg_raw){
    output << libkerat::message::raw::PATH << " " << msg_raw.get_session_id() << " "
        << msg_raw.get_width();

    const uint8_t * data = msg_raw.get_samples();

    output << " <blob/" << msg_raw.get_samples_count() << "/0x";
    char prevfill = output.fill('0');
    int prevwidth = output.width(2);
    for (size_t bs = 0; bs < msg_raw.get_samples_count(); bs++){
        output << std::hex << data[bs];
    }
    output.fill(prevfill);
    output.width(prevwidth);

    output << ">";

    return output;
}

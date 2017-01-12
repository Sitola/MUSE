/**
 * \file      tuio_message_control.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-11-18 22:51 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/tuio_message_control.hpp>
#include <kerat/utils.hpp>
#include <cstring>
#include <algorithm>
#include <lo/lo.h>
#include <vector>

namespace libkerat {
    namespace message {

        const char * control::PATH = "/tuio2/ctl";

        control::control(){ ; }

        control::control(const control& original)
            :contact_session(original)
        {
            m_controls = original.m_controls;
        }
        control::control(const session_id_t session_id, const control::controls_list & controls_to_add) throw (std::out_of_range)
            :contact_session(session_id)
        {
            set_controls(controls_to_add);
        }

        control & control::operator =(const control& second){
            contact_session::operator=(second);

            m_controls = second.m_controls;

            return *this;
        }

        bool control::operator ==(const control& second) const {
            return (
                contact_session::operator==(second) &&
                (m_controls == second.m_controls)
            );
        }


        control::controls_list control::set_controls(const controls_list & controls) throw (std::out_of_range){
            controls_list retval = m_controls;

            // check whether unnormalized value has occured
            if (std::find_if(controls.begin(), controls.end(), is_not_normalized) != controls.end()){
                throw std::out_of_range("Value not in normalized range [-1.0f ; 1.0f]");
            }

            m_controls = controls;

            return retval;
        }


        bool control::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            lo_message msg = lo_message_new();

            controls_list::const_iterator end = m_controls.end();

            lo_message_add_int32(msg, get_session_id());

            for (controls_list::const_iterator i = m_controls.begin(); i != end; i++){
                lo_message_add_float(msg, *i);
            }

            lo_bundle_add_message(target, PATH, msg);

            return true;
        }

        kerat_message * control::clone() const { return new control(*this); }

        void control::print(std::ostream & output) const { output << *this; }

    } // ns message

    namespace internals { namespace parsers {
        /**
         * \brief TUIO 2.0 control (/tuio2/ctl) parser
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
        bool parse_ctl(
            std::vector<libkerat::kerat_message *> & results,
            const char * path,
            const char * types,
            lo_arg ** argv,
            int argc,
            void * user_data __attribute__((unused))
        ){
            if (strcmp(path, libkerat::message::control::PATH) != 0){ return false; }
            if (argc < 2){ return false; }
            if (types[0] != 'i'){ return false; }

            libkerat::message::control::controls_list controls;

            for (int i = 1; i < argc; i++){
                if (types[i] != 'f'){
                    return false;
                }
                controls.push_back(argv[i]->f);
            }

            libkerat::message::control * msg_control = new libkerat::message::control(argv[0]->i, controls);
            results.push_back(msg_control);
            return true;
        }
    }} // ns parsers // ns internals
} // ns libkerat

/**
 * \ingroup global
 * \brief Pretty-print the \ref libkerat::message::control "control" message to given output stream
 * \note Format: <tt>
 * {OSC path} {session id}[ {control_X}]*
 * </tt>
 * \param output - output stream to write to
 * \param msg_ctl - bounds message to print
 * \return modified output stream
 */
std::ostream & operator<<(std::ostream & output, const libkerat::message::control & msg_ctl){
    output << libkerat::message::control::PATH << " " << msg_ctl.get_session_id();

    libkerat::message::control::controls_list controls = msg_ctl.get_controls();
    for (libkerat::message::control::controls_list::const_iterator control = controls.begin(); control != controls.end(); control++){
        output << " " << *control;
    }

    return output;
}

/**
 * \file      tuio_message_generic_osc_message.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2013-04-06 17:13 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/generic_osc_message.hpp>
#include <cstring>
#include <lo/lo.h>
#include <list>
#include <algorithm>
#include <stdexcept>
#include <cassert>
#include <iostream>

namespace libkerat {
    namespace message {

        generic_osc_message::generic_osc_message(const std::string & path, const lo_message & message) throw (std::invalid_argument, std::runtime_error)
            :m_message(NULL)
        {
            set_path(path);
            set_osc_message(message);
        }

        generic_osc_message::generic_osc_message(const generic_osc_message & original)
            :m_message(NULL)
        {
            *this = original;
        }
        
        generic_osc_message::~generic_osc_message(){
            clear_osc_message();
        }
        
        std::string generic_osc_message::set_path(const std::string& path) throw (std::invalid_argument) {
            std::string retval = m_osc_path;
            
            m_osc_path = path;
            if (m_osc_path.empty()){ 
                throw std::invalid_argument("OSC path cannot be empty!");
            }
            
            return retval;
        }
        
        void generic_osc_message::set_osc_message(const lo_message& message) throw (std::invalid_argument, std::runtime_error) {
            lo_message cloned = clone_message(message);
            clear_osc_message();
            m_message = cloned;
            
        }

        lo_message generic_osc_message::clone_message(const lo_message& message) throw (std::invalid_argument, std::runtime_error) {
            if (message == NULL){
                throw std::invalid_argument("OSC message cannot be NULL!");
            }
            
            size_t bytes_serialsed = 0;
            void * serialised = lo_message_serialise(message, "/dummy", NULL, &bytes_serialsed);
            if (serialised == NULL){
                throw std::runtime_error("Failed to clone the message (serialise phase)!");
            }
            
            int retval = 0;
            lo_message tmp_message = lo_message_deserialise(serialised, bytes_serialsed, &retval);

            free(serialised);
            serialised = NULL;

            if (tmp_message == NULL){
                throw std::runtime_error("Failed to clone the message (deserialise phase)!");
            }
            
            return tmp_message;
            
        }
        
        void generic_osc_message::clear_osc_message(){
            if (m_message != NULL) { 
                lo_message_free(m_message); 
                m_message = NULL;
            }        
        }

        generic_osc_message & generic_osc_message::operator =(const generic_osc_message& second){
            try {
                set_path(second.m_osc_path);
                set_osc_message(second.m_message);
            } catch (const std::exception & ex){
                std::cerr << "Libkerat internal failure: inconsistent generic_osc_message detected (" << ex.what() << ")!!!" << std::endl;
                assert(0);
            }
            
            return *this;
        }

        bool generic_osc_message::operator ==(const generic_osc_message& second) const {
            bool retval = true;

            size_t bytes_serialised_1 = 0;
            size_t bytes_serialised_2 = 0;
            void * serialised_1 = lo_message_serialise(m_message, m_osc_path.c_str(), NULL, &bytes_serialised_1);
            void * serialised_2 = lo_message_serialise(second.m_message, second.m_osc_path.c_str(), NULL, &bytes_serialised_2);

            if ((serialised_1 == NULL) || (serialised_2 == NULL)){
                free (serialised_1);
                free (serialised_2);
                
                std::cerr << "Libkerat internal failure: inconsistent generic_osc_message detected!!!" << std::endl;
                assert(0);
            }
            
            if (bytes_serialised_1 != bytes_serialised_2) { 
                retval = false; 
            } else {
                retval = memcmp(serialised_1, serialised_2, bytes_serialised_1);
            }
            
            free (serialised_1);
            free (serialised_2);
            
            return retval;
        }

        bool generic_osc_message::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            lo_message cloned_msg = NULL;
            try {
                cloned_msg = clone_message(m_message);
            } catch (const std::exception & ex){
                std::cerr << "Libkerat internal failure: inconsistent generic_osc_message detected (" << ex.what() << ")!!!" << std::endl;
                assert(0);
            }

            lo_bundle_add_message(target, m_osc_path.c_str(), cloned_msg);

            return true;
        }

        kerat_message * generic_osc_message::clone() const { return new generic_osc_message(*this); }

        void generic_osc_message::print(std::ostream & output) const { output << *this; }

    }

    // alv parser
    namespace internals { namespace parsers {
        /**
         * \brief Generic OSC message parser
         *
         * This parser encapsulates the received, unrecognized lo_message into
         * kerat_message, so it can be passed throught the library routines.
         * This parser is not registered with standard parsers call and has to
         * be enabled by argument in \ref libkerat::simple_client "simple_client"
         * or by any means in different client implementations.
         *
         * \return true if the message was sucessfully encapsulated, false if an
         * error has occured.
         *
         * \see libkerat::simple_client
         */
        bool parse_generic_osc_message(
            libkerat::kerat_message ** result,
            const char * path,
            const lo_message & message
        ){
            *result = NULL;
            
            try {
                *result = new libkerat::message::generic_osc_message(path, message);
            } catch (const std::exception & ex){
                std::cerr << "Failed to pass unknown message: " << ex.what() << "!" << std::endl; 
                return false;
            }

            return true;
        }
    } } // ns parsers // ns internal
} // ns libkerat

/**
 * \ingroup global
 * \brief Pretty-print the \ref libkerat::message::generic_osc_message "generic_osc_message" message to given output stream
 * \note Format: <tt>{OSC path} {typetag}</tt>
 * \param output - output stream to write to
 * \param osc_msg - generic_osc_message message to print
 * \return modified output stream
 */
std::ostream & operator<<(std::ostream & output, const libkerat::message::generic_osc_message & osc_msg){
    return (output << "[UNKNOWN] " << osc_msg.get_path() << " " << lo_message_get_types(osc_msg.get_osc_message()));
}


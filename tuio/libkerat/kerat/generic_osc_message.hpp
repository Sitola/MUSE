/**
 * \file      generic_osc_message.hpp
 * \brief     Support for unknown/unrecognized OSC messages
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2013-04-06 17:13 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_GENERIC_OSC_MESSAGE_HPP
#define KERAT_GENERIC_OSC_MESSAGE_HPP

#include <kerat/typedefs.hpp>
#include <kerat/message.hpp>
#include <set>
#include <ostream>
#include <stdexcept>
#include <string>

namespace libkerat {
    namespace message {

        //! \brief Class representing unknown/unrecognized OSC messages
        //! \note This message cannot be registered using standard message convertor API
        class generic_osc_message: public libkerat::kerat_message {
        public:

            generic_osc_message(const std::string & path, const lo_message & captured) 
                throw (std::invalid_argument, std::runtime_error);
            generic_osc_message(const generic_osc_message & original);
            virtual ~generic_osc_message();

            bool operator == (const generic_osc_message & second) const ;
            inline bool operator != (const generic_osc_message & second) const { return !operator==(second); }

            inline lo_message get_osc_message() const { return m_message; }
            inline const std::string & get_path() const { return m_osc_path; }

            kerat_message * clone() const ;

            void print(std::ostream & output) const ;

        private:

            std::string m_osc_path;
            lo_message m_message;

            static lo_message clone_message(const lo_message & message) throw (std::invalid_argument, std::runtime_error);
            
            void set_osc_message(const lo_message & message) throw (std::invalid_argument, std::runtime_error);
            std::string set_path(const std::string & path) throw (std::invalid_argument);
            void clear_osc_message();
            
            generic_osc_message & operator=(const generic_osc_message & original);
            
            bool imprint_lo_messages(lo_bundle target) const;

        }; // cls generic_osc_message
    } // ns message
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::generic_osc_message & osc_msg);

#endif // KERAT_GENERIC_OSC_MESSAGE_HPP

/**
 * \file      stdout_listener.hpp
 * \brief     Provides the basic tuio 2 stdout listener.
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-03-22 09:52 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_STDOUT_LISTENER_HPP
#define KERAT_STDOUT_LISTENER_HPP

#include <kerat/typedefs.hpp>
#include <kerat/listener.hpp>
#include <kerat/tuio_messages.hpp>
#include <kerat/bundle.hpp>
#include <ostream>

namespace libkerat {
    namespace listeners {
        
        //! \brief TUIO listener that prints the received messages to output stream
        class stdout_listener: public libkerat::listener {
        public:
            //! Creates a new stdout listener instance with output into stdout
            stdout_listener();

            /**
             * \brief Creates a new stdout_listener instance with output into given output stream
             * \param output_stream - ostream instance to write output to
             */
            stdout_listener(std::ostream & output_stream);
            
            virtual ~stdout_listener();

            void notify(const client * notifier);
            
            void process_bundle(libkerat::bundle_handle f);

            /**
             * \brief Prints the standard TUIO messages
             * \param message - message to print
             * \return true if message was recognized as standard TUIO 2.0 message and printed
             */
            virtual bool print(const libkerat::kerat_message * message);

        protected:

            std::ostream & m_output;
        }; // cls stdout_listener
    } // ns listeners
} // ns libkerat

#endif // KERAT_STDOUT_LISTENER_HPP

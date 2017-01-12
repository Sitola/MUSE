/**
 * \file      forwarding_listener.hpp
 * \brief     Listener that serves as message forwarder
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-01-19 17:13 UTC+1
 * \copyright BSD
 */

#ifndef KERAT_FORWARDING_LISTENER_HPP
#define KERAT_FORWARDING_LISTENER_HPP

#include <kerat/typedefs.hpp>
#include <kerat/server.hpp>
#include <kerat/listener.hpp>
#include <kerat/exceptions.hpp>
#include <kerat/server_adaptor.hpp>
#include <ostream>

namespace libkerat {

    namespace listeners {

        //! \brief Forwarding listener allows to capture the TUIO bundle, append messages and send it out again
        class forwarding_listener: public libkerat::listener {
        public:

            /**
             * \brief Create a new forwarding listener that forwards through given server
             * \throw libkerat::exception::net_setup_error when given server is null
             */
            forwarding_listener(bool alter_alive = true, bool replace_frame = true);

            ~forwarding_listener();

            /**
             * \brief Processes the bundle stack held by the client
             * 
             * Loads the bundle stack from client appends messages that were
             * received and calls server's \ref libkerat::server::send "send"
             * subsequently. To append data to bundle, see 
             * 
             * \ref libkerat::server::add_presend_callback.
             * \param notifier - client that has notified the listener
             */
            void notify(const libkerat::client * notifier);
            
            /**
             * \brief Access the internal server adaptor
             * \return pointer to internal server adaptor
             */
            server_adaptor * get_server_adaptor();

        private:
            struct altering_adaptor: public server_adaptor {
                altering_adaptor(bool alter_alive, bool replace_frame);
                ~altering_adaptor();
                
                virtual int process_bundle(bundle_handle & to_process);
                void prepare_bundle(bundle_handle & to_inject);
                void commit();
                
                bundle_handle m_bundle;
                bool m_replace_frame;
                bool m_alter_alive;
            };
            
            altering_adaptor m_adaptor;
        };

    } // ns listeners
    
} // ns libkerat

#endif // KERAT_FORWARDING_LISTENER_HPP

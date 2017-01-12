/**
 * \file      simple_client.hpp
 * \brief     Provides basic tuio client.
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-09-21 14:21 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_SIMPLE_CLIENT_HPP
#define KERAT_SIMPLE_CLIENT_HPP

#include <lo/lo.h>
#include <kerat/typedefs.hpp>
#include <kerat/client.hpp>
#include <kerat/bundle.hpp>
#include <kerat/utils.hpp>
#include <kerat/parsers.hpp>

#include <deque>
#include <list>
#include <string>

namespace libkerat {

    //! \brief Basic TUIO 2.0 network client
    class simple_client: public client {
    public:
        
        //! \alias libkerat::internals::message_convertor_entry
        typedef internals::message_convertor_entry message_convertor_entry;

        //! \alias libkerat::internals::message_convertor
        typedef internals::message_convertor message_convertor;

        //! \brief Functor that enables given message convertor
        struct convertor_enable_functor: public std::unary_function<const libkerat::internals::message_convertor_entry, void> {
            void operator()(const libkerat::internals::message_convertor_entry & entry);
        private:    
            friend class simple_client;
            convertor_enable_functor(libkerat::simple_client & client);
            libkerat::simple_client & m_client;
        };

        /**
         * \brief Create a new raw tuio client instance, listening on given UDP port
         * \param port - udp port to listen on
         * \param accept_unknown - whether to accept unknown messages and pass
         * them over as \ref libkerat::message::generic_osc_message; defaults to true
         * \throw libkerat::exception::net_setup_error when fails to setup listenning on given port
         */
        simple_client(uint16_t port, bool accept_unknown = true) throw (libkerat::exception::net_setup_error);

        /**
         * \brief Create a new raw tuio client binded to given liblo server instance
         * \param instance - liblo osc server to bind to
         * \param accept_unknown - whether to accept unknown messages and pass
         * them over as \ref libkerat::message::generic_osc_message; defaults to false
         * \throw libkerat::exception::net_setup_error when fails to bind with given OSC server
         */
        simple_client(lo_server instance, bool accept_unknown = false) throw (libkerat::exception::net_setup_error);

        bool load(int count = 1);

        bool load(int count, struct timespec timeout);

        void purge();

        /**
         * \brief Gets functor that enables given message convertor
         * \return corresponding functor, valid as-long as this client instance
         * remains valid
         */
        convertor_enable_functor get_enabler_functor();
        
        /**
         * \brief Enable given message convertor in this client
         * \param convertor - convertor entry that should be enabled
         * \return convertor entry containing the paths successfully enabled
         */
        message_convertor_entry enable_convertor(const message_convertor_entry & convertor);

        /**
         * \brief Disable convertors for given OSC message paths
         * \param pathlist - list of paths for which to disable convertors
         */
        void disable_convertors(const std::list<std::string> & pathlist);

        /**
         * \brief Disable convertor for given OSC message path
         * \param path - OSC path for which to disable conversion
         * \return convertor entry containing the paths for which the convertor still applies
         */
        message_convertor_entry disable_convertor(const std::string & path);

        /**
         * \brief Gets the lo server handle for custom user message binding.
         * If this object was constructed using existing handle, no action except removing the
         * standard TUIO message handlers is removed. If this was constructed using port number,
         * the lo server is destroyed with this object.
         * \return liblo OSC server handle
         */
        lo_server get_lo_server_handle() const;

        /**
         * \brief Check whether unknown messages are passed through
         * 
         * Check whether to accept unknown messages and pass them over as 
         * \ref libkerat::message::generic_osc_message
         * 
         * \return true if so
         */
        inline bool get_accept_unknown() const { return m_accept_unknown; }
        
        /**
         * \brief Make the client pass over the unknown messages
         * 
         * Set whether to accept unknown messages and pass them over as 
         * \ref libkerat::message::generic_osc_message
         * \param flag - true if so
         * \return previous setting
         */
        bool set_accept_unknown(bool flag);

        ~simple_client();

        bundle_stack get_stack() const ;

    private:
        typedef void (simple_client::*reader_func)(int, timespec);
        
        //! \brief Convertor map type (path->convertor)
        typedef std::map<std::string, message_convertor_entry *> convertor_map;

        //! \brief Runs the convertor over the message received and integrates the result into the client's stack
        static int lo_message_handler(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg __attribute__((unused)), void *user_data);

        void read_socket_fd_existing(int count, timespec timeout);
        void read_socket_fd_nonexisting(int count, timespec timeout);

        //! \brief Initializes convertors for standard TUIO 2.0 messages
        void init_standard_convertors();

        lo_server m_lo_serv;
        bool m_foreign;
        bool m_accept_unknown;

        bundle_stack m_received_frames;
        bundle_handle m_current_bundle;

        convertor_map m_convertors;
        internals::convertor_output_container m_results;

        int m_last_events_count;

        reader_func m_reader;
        int m_lo_fd;

    }; // cls simple client

} // ns libkerat

#endif // KERAT_SIMPLE_CLIENT_HPP

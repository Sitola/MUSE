/**
 * \file      simple_server.hpp
 * \brief     Provides basic tuio 2.0 server.
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-10-22 00:21 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_SIMPLE_SERVER_HPP
#define KERAT_SIMPLE_SERVER_HPP

#include <lo/lo.h>
#include <kerat/typedefs.hpp>
#include <kerat/server.hpp>
#include <kerat/stdout_listener.hpp>
#include <set>
#include <map>
#include <deque>
#include <string>

namespace libkerat {

    //! \brief Basic TUIO 2.0 network server implementation
    class simple_server: public server {
    public:

        //! \brief Create a new simple_server instance with empty target
        simple_server();

        /**
         * \brief Create a new simple_server instance for given target address
         * \param target_client - target TUIO client address
         * \throw libkerat::exception::net_setup_error when an error has occured during target setup
         */
        simple_server(lo_address target_client) throw (libkerat::exception::net_setup_error);

        /**
         * \brief Create a new simple server instance for given uri and extended frame message parameters
         *
         * \param target_url - host:port to send data to or liblo url
         * \param appname    - name of the sender
         * \param address    - the IPv4 address of the server (this is usualy the publicly available address)
         * \param instance   - instance ID of the sender
         * \param sensor_width - width of the sensor that this sender covers
         * \param sensor_height - height of the sensor that this sender covers
         * \throw libkerat::exception::net_setup_error when an error has occured during target setup
         */
        simple_server(std::string target_url, std::string appname, addr_ipv4_t address,
            instance_id_t instance, dimmension_t sensor_width, dimmension_t sensor_height
        ) throw (libkerat::exception::net_setup_error);

        ~simple_server();

        /**
         * \brief Set target TUIO client address
         * \param target_client - address to send data to
         * \throw libkerat::exception::net_setup_error when an error has occured during target setup
         */
        void set_target(const lo_address target_client) throw (libkerat::exception::net_setup_error);

        /**
         * Get current target TUIO client setting
         * \return NULL if not set
         */
        inline lo_address get_target() const { return m_target; }

        /**
         * \brief Set the underlying OSC bundle timetag
         *
         * The timeteg is reset to "immediate" value after each call to \ref send
         *
         * \param bundle_timetag - timetag that the OSC bundle should carry
         */
        void set_timetag(lo_timetag bundle_timetag);

        /**
         * \brief Get the current OSC bundle timetag setting
         * \return current timetag setting
         */
        inline lo_timetag get_timetag() const { return m_timetag; }

        /**
         * \brief Append clone of the message (see \ref kerat_message::clone) to
         * the TUIO bundle being constructed. Accepted messages are all kerat_message
         * descendents except for frame and alive messages.
         *
         * \li Frame message that was given is used as template for future send() calls
         * (the timestamp and frame ID information is set by the server implementation).
         * The return value in this case is true.
         * \li Alive messages are ignored with no exception, since they're generated by
         * the this TUIO server implementation automatically from id's that were registered
         * in this server instance by explicit call to \ref register_session_id or obtained
         * by calling the \ref get_auto_session_id. Session ID can be removed by respective
         * call to \ref unregister_session_id
         *
         * \param msg - message to add
         * \return true if message was accepted, false otherwise
         */
        bool append_clone(const kerat_message * msg);

        //! \brief Cleans the added message stack (except frame message)
        void clear_message_stack();

    private:
        lo_address m_target;
        lo_timetag m_timetag;
        
        bool prepare_bundle();
        bool commit();

        message::frame m_frame_template;
        listeners::stdout_listener m_printer;

    }; // cls simple_server

} // ns libkerat

#endif // KERAT_SIMPLE_SERVER_HPP

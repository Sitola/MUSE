/**
 * \file      listener.hpp
 * \brief     Provides the abstract class that provides automatic update calls when new TUIO messages are received.
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-09-22 12:29 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_LISTENER_HPP
#define KERAT_LISTENER_HPP

#include <kerat/typedefs.hpp>
#include <kerat/session_manager.hpp>
#include <set>

namespace libkerat {

    class client;

    //! \brief TUIO Listener interface, notified when data arrives to the client
    class listener {
    public:
        virtual ~listener();

        /**
         * \brief Callback called when any of the connected clients receive data
         * \param notifier - client to be contacted for the data request
         */
        virtual void notify(const client * notifier) = 0;
        
        /**
         * \brief Callback called when client binds with the listener
         * \param notifier - client that is binding in
         */
        virtual void notify_client_bind(client * notifier);

        /**
         * \brief Callback called when client disconnects from the listener
         * \param notifier - client that is disconnecting
         */
        virtual void notify_client_release(client * notifier);

        /**
         * \brief Checks whether is this listener connected to any client
         * \return true if this listener is connected
         */
        virtual bool is_connected() const;

    protected:

        //! set of clients this listener is binded with
        typedef std::set<client *> client_set;
        
        client_set m_connected_clients;

    }; // cls listener

} // ns libkerat

#endif // KERAT_LISTENER_HPP

/**
 * \file      client.hpp
 * \brief     Provides the abstract class that all libkerat compatible TUIO 2.0 client implementations must inherit.
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-09-21 14:22 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_CLIENT_HPP
#define KERAT_CLIENT_HPP

#include <kerat/typedefs.hpp>
#include <kerat/session_manager.hpp>
#include <kerat/listener.hpp>
#include <kerat/bundle.hpp>
#include <set>
#include <iostream>

namespace libkerat {

    class bundle_handle;
    class bundle_stack;

    //! \brief TUIO Client interface, notifies the listeners when data arrives
    class client: protected internals::bundle_manipulator {
    public:

        /**
         * \brief Ensure safe client disconnection
         *
         * Disconnects all enlisted clients
         */
        virtual ~client();

        /**
         * \brief Add the \ref listener to listeners list
         *
         * Adds the listener to internal listener stack and then calls lstnr->notify_client_bind. If the listener is already present, no action is performed.
         * \param lstnr - listener to add
         */
        void add_listener(listener * lstnr);

        /**
         * \brief Delete the \ref listener from listeners list
         *
         * Calls lstnr->notify_client_release and then removes the listener from internal listener stack. If the listener is not present, no action is performed.
         * \param lstnr - listener to remove
         */
        void del_listener(listener * lstnr);


        /**
         * \brief Get the event bundle stack
         *
         * Returns the frame stack generated with load method
         * \return frame stack containing all (and only) frames loaded since previous \ref load() or \ref purge() call
         */
        virtual bundle_stack get_stack() const = 0;


        /**
         * \brief Load events from underlying layer
         *
         * Calls the \ref load with 60 second timeout
         * \param count - maximum count of event bundles to load, defaults to 1
         * \return true if any tuio frames were loaded
         */
        virtual bool load(int count = 1) = 0;

        /**
         * \brief Load events from underlying layer
         *
         * Loads up to count event bundles from osc layer or until the timeout passes
         * \param count - maximum count of event bundles to load
         * \param timeout - maximum time to wait for event
         * \return true if any tuio frames were loaded
         */
        virtual bool load(int count, struct timespec timeout) = 0;

        //! \brief Reset the event bundle stack of this client
        virtual void purge() = 0;

    protected:
        typedef std::set<listener *> listener_set;

        client();
        void notify_listeners();

        listener_set m_listeners;

    };

}

#endif // KERAT_CLIENT_HPP

/**
 * \file      adaptor.hpp
 * \brief     Provides the unified interface common for all adaptors.
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-12-21 23:41 UTC+1
 * \copyright BSD
 */

#ifndef KERAT_ADAPTOR_HPP
#define KERAT_ADAPTOR_HPP

#include <lo/lo.h>
#include <kerat/typedefs.hpp>
#include <kerat/client.hpp>
#include <kerat/listener.hpp>

namespace libkerat {
    
    //! \brief Adaptor interface, allows preprocessing of the received event bundles
    class adaptor: public client, public listener {
    public:
        
        virtual ~adaptor();

        /**
         * \brief Run the adaptor on given bundle
         * \param to_process - bundle handle to be processed by this adaptor adaptor
         * \param output_bundle - output bundle handle
         * \return 0 if processing done, negative number indicates that error has occured
         */
        virtual int process_bundle(const bundle_handle & to_process, bundle_handle & output_bundle) = 0;
        
        /**
         * \brief Runs the \ref process_bundle on each received bundle, see details below
         * 
         * In general, when notify is called the \ref purge method cleans up the
         * bundle stack. Then, for each received bundle the \ref process_bundle is
         * called. After the processing is complete, all listeners connected to 
         * this adaptor are notified.
         * 
         * \param notifier - client that notified the adaptor
         */
        virtual void notify(const client * notifier) = 0;

        /**
         * \brief Calls the load method on all connected clients
         * \param count - used as in \ref libkerat::client::load(int)
         * \return true if any of the connected client's load returned true
         */
        virtual bool load(int count = 1);

        /**
         * \brief Calls the load method on all connected clients
         * \param count - handled by each client, behavior might differ
         * \param timeout - handled by each client, behavior might differ
         * \return true if any of the connected client's load returned true
         */
        virtual bool load(int count, struct timespec timeout);
    protected:

    }; // cls adaptor

} // ns libkerat

#endif // KERAT_ADAPTOR_HPP

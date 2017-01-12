/**
 * \file      server_adaptor.hpp
 * \brief     Provides the unified interface common for all server-side adaptors.
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-12-25 23:40 UTC+1
 * \copyright BSD
 */

#ifndef KERAT_SERVER_ADAPTOR_HPP
#define KERAT_SERVER_ADAPTOR_HPP

#include <lo/lo.h>
#include <kerat/typedefs.hpp>
#include <kerat/bundle.hpp>

namespace libkerat {
    
    class server;
    
    //! \brief Server-side adaptor interface, allows preprocessing of event bundles before being sent
    class server_adaptor: protected internals::bundle_manipulator {
    public:
        virtual ~server_adaptor();
        
        /**
         * \brief Run the server-side adaptor on given bundle
         * \param to_process - bundle handle to be processed inplace by this adaptor
         * Bundles that do not begin with frame or end with alive message shall be dropped
         * \return 0 if ok
         */
        virtual int process_bundle(bundle_handle & to_process) = 0;
        
        /**
         * \brief Callback called when server binds with the adaptor
         * \param notifier - server that is binding in
         */
        virtual void notify_server_bind(server * notifier);

        /**
         * \brief Callback called when server disconnects from the adaptor
         * \param notifier - server that is disconnecting
         */
        virtual void notify_server_release(server * notifier);
    protected:
        typedef std::set<server *> server_set;
        
        server_set m_connected_servers;
        
    };

} // ns libkerat

#endif // KERAT_SERVER_ADAPTOR_HPP

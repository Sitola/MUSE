/**
 * \file      server.hpp
 * \brief     Provides the abstract class that all libkerat compatible TUIO 2.0 server implementations must inherit.
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-10-20 13:48 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_SERVER_HPP
#define KERAT_SERVER_HPP

#include <kerat/typedefs.hpp>
#include <kerat/session_manager.hpp>
#include <kerat/frame_manager.hpp>
#include <kerat/bundle.hpp>
#include <kerat/server_adaptor.hpp>
#include <kerat/utils.hpp>
#include <lo/lo.h>
#include <map>
#include <list>

namespace libkerat {

    class server;
    class kerat_message;

    //! \brief TUIO Server interface, tracker-side endpoint of the TUIO protocol
    class server: public internals::session_manager, public internals::frame_manager, protected internals::bundle_manipulator {
    public:
        //! Create a new, empty (dummy) server
        server();
        
        virtual ~server();

        /**
         * \brief Creates a copy of given message and stores it into the output queue
         * \note This method stores copy of the message aquired calling the
         * \ref libkerat::kerat_message::clone "clone" method
         * \param message - message to add
         * \return false - if message was rejected, true if the server implementation accepted the message
         */
        virtual bool append_clone(const kerat_message * message) = 0;

        /**
         * \brief Close the bundle adding the alive message and send it.
         * \return true if the message was sucessfully sent, false otherwise
         */
        virtual bool send();

        /**
         * \brief Appends the \ref adaptor to adaptors list
         *
         * Adds the adaptor to internal adaptor list and then calls adptr->notify_server_bind. If the adaptor is already present, no action is performed.
         * \param adptr - adaptor to add
         */
        void add_adaptor(server_adaptor * adptr);

        /**
         * \brief Delete the \ref adaptor from adaptors list
         *
         * Calls adptr->notify_server_release and then removes the adaptor from internal adaptor list. If the adaptor is not present, no action is performed.
         * \param adptr - adaptor to remove
         */
        void del_adaptor(server_adaptor * adptr);
        
    protected:
        //! \brief list of pointers to server-side adaptors
        typedef std::list<libkerat::server_adaptor *> adaptor_list;

        //! \brief this is where bundles are stored before send is called
        bundle_handle m_bundle;
        
        /**
         * \brief Calls kerat_message::imprint_lo_messages of the message onto the given bundle.
         * \param bundle - bundle to imprint messages to
         * \param message - kerat message from which imprints its data to bundle
         * \return the result of corresponding \ref kerat_message::imprint_lo_messages "imprint_lo_messages" call
         */
        static inline bool imprint_bundle(lo_bundle bundle, const kerat_message * message){ return message->imprint_lo_messages(bundle); }
        
        //! \brief safely erases the adaptor list
        virtual void cleanup_adaptors();
        
        /**
         * \brief Makes the bundle ready to be processed by server-side adaptors
         * \return true if ok, false if an error has occured
         */
        virtual bool prepare_bundle() = 0;

        /**
         * \brief Run the server-side adaptors
         * \param paranoid - stop processing after first adaptor returns non-zero
         * \return true if ok, false if an error has occured in any adaptor
         */
        virtual bool run_adaptors(bool paranoid = false);
        
        /**
         * \brief Run output check
         * 
         * Run ouput check to ensure that bundle_handle begins with frame and
         * ends with alv messages. Multiple frame (or alv) messages are not allowed.
         * 
         * \return true if ok, false if an error has occured in bundle structure
         */
        virtual bool output_check();
        
        /**
         * \brief The very sending of the bundle
         * \return true if ok, false if an error has occured
         */
        virtual bool commit() = 0;
        
        adaptor_list m_server_adaptors;

    };

} // ns libkerat


#endif // KERAT_SERVER_HPP

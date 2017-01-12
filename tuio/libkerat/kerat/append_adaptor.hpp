/**
 * \file      append_adaptor.hpp
 * \brief     An adaptor to periodically append messages to processed bundles
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2013-01-29 18:38 UTC+1
 * \copyright BSD
 */

#ifndef KERAT_APPEND_ADAPTOR_HPP
#define KERAT_APPEND_ADAPTOR_HPP

#include <kerat/adaptor.hpp>
#include <kerat/server_adaptor.hpp>
#include <string>
#include <list>
#include <stdexcept>

namespace libkerat {
    namespace adaptors {
        //! \brief Basic adaptor used to iject messages into outgoing TUIO 2.0 bundles
        class append_adaptor: public adaptor, public server_adaptor {
        public:
            typedef std::list<kerat_message *> message_list;
            
            append_adaptor(const message_list & messages_prepend, const message_list & messages_append, size_t update_interval);
            append_adaptor(const append_adaptor & second);
            virtual ~append_adaptor();
            
            //! \brief get messages appended at the end of bundle
            const message_list & get_append_messages() const { return m_append_messages; }
            //! \brief set messages appended at the end of bundle
            virtual void set_append_messages(const message_list & messages);
            
            //! \brief get messages appended at the beginning of bundle
            const message_list & get_prepend_messages() const { return m_prepend_messages; }
            //! \brief set messages appended at the beginning of bundle
            virtual void set_prepend_messages(const message_list & messages);

            size_t get_update_interval() const { return m_interval; }
            virtual void set_update_interval(size_t interval);
            
            virtual int process_bundle(libkerat::bundle_handle & to_process);
            virtual int process_bundle(const bundle_handle & to_process, bundle_handle & output_bundle);
            virtual void notify(const client * notifier);

            bundle_stack get_stack() const { return m_processed_frames; }
            
            append_adaptor & operator=(const append_adaptor & second);
            
        private:
            void clear();
            int internal_process_bundle(bundle_handle & to_process);
            bundle_stack m_processed_frames;
            void purge();
            
            size_t m_interval;
            message_list m_append_messages;
            message_list m_prepend_messages;
            
        };
    }
}

#endif // KERAT_APPEND_ADAPTOR_HPP

/**
 * \file      append_adaptor.cpp
 * \brief     An adaptor to periodically append messages to processed bundles
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2013-01-29 18:38 UTC+1
 * \copyright BSD
 */

#include <kerat/append_adaptor.hpp>

static void deletor(libkerat::kerat_message * d){
    delete d;
    d = NULL;
}

static libkerat::kerat_message * cloner(libkerat::kerat_message * d){
    return d->clone();
}

namespace libkerat {
    namespace adaptors {

        append_adaptor::~append_adaptor(){
            clear();
        }
        
        append_adaptor::append_adaptor(
            const append_adaptor::message_list & messages_prepend, 
            const append_adaptor::message_list & messages_append, 
            size_t update_interval
        ){
            set_update_interval(update_interval);
            set_prepend_messages(messages_prepend);
            set_append_messages(messages_append);
        }
        
        append_adaptor::append_adaptor(const append_adaptor & second){
            *this = second;
        }
        
        void append_adaptor::set_append_messages(const message_list& messages){
            std::for_each(m_append_messages.begin(), m_append_messages.end(), deletor);
            m_append_messages.resize(messages.size());
            std::transform(messages.begin(), messages.end(), m_append_messages.begin(), cloner);
        }

        void append_adaptor::set_prepend_messages(const message_list& messages){
            std::for_each(m_prepend_messages.begin(), m_prepend_messages.end(), deletor);
            m_prepend_messages.resize(messages.size());
            std::transform(messages.begin(), messages.end(), m_prepend_messages.begin(), cloner);
        }
        
        void append_adaptor::set_update_interval(size_t interval){
            if (interval == 0){ m_interval = 1; }
        }
        
        void append_adaptor::clear(){
            std::for_each(m_prepend_messages.begin(), m_prepend_messages.end(), deletor);
            std::for_each(m_append_messages.begin(), m_append_messages.end(), deletor);
        }
        
        append_adaptor & append_adaptor::operator=(const append_adaptor & second){
            if (&second == this){ return *this; }
            
            clear();
            m_prepend_messages.resize(second.m_prepend_messages.size());
            std::transform(second.m_prepend_messages.begin(), second.m_prepend_messages.end(), m_prepend_messages.begin(), cloner);
            m_append_messages.resize(second.m_append_messages.size());
            std::transform(second.m_append_messages.begin(), second.m_append_messages.end(), m_append_messages.begin(), cloner);
            
            return *this;
        }
        
        void append_adaptor::notify(const client * cl){
            purge();
            
            bundle_stack data = cl->get_stack();

            while (data.get_length() > 0){
                bundle_handle * tmphx = new bundle_handle;
                bundle_handle current_frame = data.get_update(bundle_stack::INDEX_OLDEST);
                process_bundle(current_frame, *tmphx);
                bm_stack_append(m_processed_frames, tmphx);
            }
            
            notify_listeners();
        }

        int append_adaptor::process_bundle(const bundle_handle & to_process, bundle_handle & output_frame){

            // if we're not asked to run inplace, commance copy
            if (&to_process != &output_frame){
                bm_handle_copy(to_process, output_frame);
            }
            
            return internal_process_bundle(output_frame);
        }
        
        int append_adaptor::process_bundle(bundle_handle & to_process){
            return internal_process_bundle(to_process);
        }

        int append_adaptor::internal_process_bundle(bundle_handle & to_process){
            internals::bundle_manipulator::handle_iterator frame = bm_handle_end(to_process);
            internals::bundle_manipulator::handle_iterator alive = bm_handle_end(to_process);
            
            for (internals::bundle_manipulator::handle_iterator tmp = bm_handle_begin(to_process); tmp != bm_handle_end(to_process); ++tmp){
                if ((frame == bm_handle_end(to_process)) && (dynamic_cast<message::frame*>(*tmp) != NULL)){
                    frame = tmp;
                } else if ((alive == bm_handle_end(to_process)) && (dynamic_cast<message::alive*>(*tmp) != NULL)){
                    alive = tmp;
                }
            }
            
            if ((frame == bm_handle_end(to_process)) || (alive == bm_handle_end(to_process))){
                return 1;
            }
            ++frame;
            
            for (message_list::const_iterator i = m_prepend_messages.begin(); i != m_prepend_messages.end(); ++i){
                bm_handle_insert(to_process, frame, (*i)->clone());
            }
            for (message_list::const_iterator i = m_append_messages.begin(); i != m_append_messages.end(); ++i){
                bm_handle_insert(to_process, alive, (*i)->clone());
            }
            
            return 0;
        }

        void append_adaptor::purge(){ 
            bm_stack_clear(m_processed_frames); 
        }

        
    } // ns adaptors
} // ns libkerat

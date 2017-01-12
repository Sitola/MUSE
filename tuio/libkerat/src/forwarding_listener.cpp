/**
 * \file      forwarding_listener.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-01-19 17:13 UTC+1
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/client.hpp>
#include <kerat/forwarding_listener.hpp>
#include <kerat/tuio_messages.hpp>

namespace libkerat {
    namespace listeners {

        forwarding_listener::altering_adaptor::altering_adaptor(bool alter_alive, bool replace_frame)
            :m_replace_frame(replace_frame), m_alter_alive(alter_alive)
        { ; }
        
        forwarding_listener::altering_adaptor::~altering_adaptor(){
            ;
        }

        void forwarding_listener::altering_adaptor::commit(){
            for (server_set::iterator i = m_connected_servers.begin(); i != m_connected_servers.end(); ++i){
                (*i)->send();
            }
        }
        
        void forwarding_listener::altering_adaptor::prepare_bundle(bundle_handle & to_inject){
            m_bundle = to_inject;
        }
        
        int forwarding_listener::altering_adaptor::process_bundle(bundle_handle & to_process){
            handle_iterator iter = bm_handle_begin(to_process);

            // frame
            if (m_replace_frame){
                message::frame * frm = dynamic_cast<message::frame *>(*iter);
                if (frm != NULL){
                    *frm = *(m_bundle.get_frame());
                }
            }
            ++iter;
            
            // find alive
            handle_iterator alive = iter;
            while ((alive != bm_handle_end(to_process)) && (dynamic_cast<message::alive *>(*alive) == NULL)){ 
                ++alive;
            }
            
            // check for valid tuio bundle
            if (alive == bm_handle_end(to_process)){
                std::cerr << "libkerat::listeners::forwarding_listener: invalid bundle!" << std::endl;
            }
            
            // inject messages
            bundle_handle::const_iterator current = m_bundle.begin();
            ++current; // skip frame;
            bundle_handle::const_iterator next = current; ++next;
            
            while (next != m_bundle.end()){
                bm_handle_insert(to_process, alive, (*current)->clone());
                current = next;
                ++next;
            }
            
            // update alive?
            if (m_alter_alive){
                message::alive * alv_original = dynamic_cast<message::alive *>(*alive);
                message::alive * alv_new = dynamic_cast<message::alive *>(*current);
                
                message::alive::alive_ids original_ids = alv_original->get_alives();
                message::alive::alive_ids new_ids = alv_new->get_alives();
                
                original_ids.insert(new_ids.begin(), new_ids.end());
                
                alv_original->set_alives(original_ids);
            }
           
            return 0;
        }

        forwarding_listener::forwarding_listener(bool alter_alive, bool replace_frame)
            :m_adaptor(replace_frame, alter_alive)
        { ; }
        
        forwarding_listener::~forwarding_listener(){ ; }
        
        server_adaptor * forwarding_listener::get_server_adaptor(){ return &m_adaptor; }
        
        void forwarding_listener::notify(const libkerat::client * notifier){

            typedef libkerat::bundle_handle::const_iterator iterator;

            libkerat::bundle_stack stack = notifier->get_stack();

            while (stack.get_length() > 0){
                libkerat::bundle_handle f = stack.get_update();
                m_adaptor.prepare_bundle(f);
                m_adaptor.commit();
            }
        }
    } // ns listeners
} // ns libkerat

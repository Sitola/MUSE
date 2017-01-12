/**
 * \file      interfaces.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-10-10 01:00 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/client.hpp>
#include <kerat/server.hpp>
#include <kerat/listener.hpp>
#include <kerat/adaptor.hpp>
#include <kerat/server_adaptor.hpp>
#include <kerat/bundle.hpp>
#include <kerat/parsers.hpp>
#include <set>
#include <iostream>

namespace libkerat {

    // client
    
    client::client(){ ; }

    client::~client(){

        listener_set::const_iterator end = m_listeners.end();
        for (listener_set::const_iterator i = m_listeners.begin(); i != end; i++){
            (*i)->notify_client_release(this);
        }
        m_listeners.clear();

    }

    void client::add_listener(listener * lstnr){
        if (m_listeners.find(lstnr) == m_listeners.end()){
            m_listeners.insert(lstnr);
            lstnr->notify_client_bind(this);
        }
    }

    void client::del_listener(listener * lstnr){
        if (m_listeners.find(lstnr) != m_listeners.end()){
            lstnr->notify_client_release(this);
            m_listeners.erase(lstnr);
        }
    }

    void client::notify_listeners(){
        listener_set::const_iterator end = m_listeners.end();
        for (listener_set::const_iterator i = m_listeners.begin(); i != end; i++){
            (*i)->notify(this);
        }
    }
    
    // listener

    listener::~listener(){
        // disconnect from all connected clients
        client_set buff = m_connected_clients;
        for (client_set::iterator i = buff.begin(); i != buff.end(); i++){
            (*i)->del_listener(this);
        }
    }

    void listener::notify_client_bind(client* notifier){
        assert(notifier != NULL);
        m_connected_clients.insert(notifier);
    }

    void listener::notify_client_release(client* notifier){
        m_connected_clients.erase(notifier);
    }

    bool listener::is_connected() const {
        return !m_connected_clients.empty();
    }

    // server
    
    server::server(){ ; }
    server::~server(){ cleanup_adaptors(); }
    
    bool server::append_clone(const kerat_message* msg){
        if (msg == NULL){ return false; }
        return bm_handle_insert(m_bundle, bm_handle_end(m_bundle), msg->clone());
    }
    
    void server::cleanup_adaptors(){
        for (
            adaptor_list::iterator adaptor = m_server_adaptors.begin();
            adaptor != m_server_adaptors.end();
            ++adaptor
        ){
            (*adaptor)->notify_server_release(this);
            *adaptor = NULL;
        }
        
        m_server_adaptors.clear();
    }
    
    bool server::send(){
        return prepare_bundle() && run_adaptors() && output_check() && commit();
    }
    
    bool server::run_adaptors(bool paranoid){
        bool retval = true;
        
        for (
            adaptor_list::iterator adaptor = m_server_adaptors.begin();
            adaptor != m_server_adaptors.end();
            ++adaptor
        ){
            int rv = (*adaptor)->process_bundle(m_bundle);
            retval &= (rv == 0);
            if (paranoid && !retval){ return retval; }
        }
        
        return retval;
    }
    
    bool server::output_check(){
        // check whether has frame
        const libkerat::message::frame * frm_0 = m_bundle.get_message_of_type<libkerat::message::frame>(0);
        if (frm_0 == NULL){ return false; }
        
        // check whether begins with frame
        if ((*m_bundle.begin()) != frm_0){ return false; }
        
        // check whether has only one frame
        frm_0 = m_bundle.get_message_of_type<libkerat::message::frame>(1);
        if (frm_0 != NULL){ return false; }
        
        // check whether has alive
        const libkerat::message::alive * alv_0 = m_bundle.get_message_of_type<libkerat::message::alive>(0);
        if (alv_0 == NULL){ return false; }

        // check whether ends with alive
        if ((*(--(m_bundle.end()))) != alv_0){ return false; }
        
        // check whether has only one alive
        alv_0 = m_bundle.get_message_of_type<libkerat::message::alive>(1);
        if (alv_0 != NULL){ return false; }
        
        return true;
    }

    void server::add_adaptor(server_adaptor * adptr){
        assert(adptr != NULL);
        if (std::find(m_server_adaptors.begin(), m_server_adaptors.end(), adptr) == m_server_adaptors.end()){
            m_server_adaptors.push_back(adptr);
            adptr->notify_server_bind(this);
        }
    }

    void server::del_adaptor(server_adaptor * adptr){
        adaptor_list::iterator found = std::find(m_server_adaptors.begin(), m_server_adaptors.end(), adptr);
        if (found != m_server_adaptors.end()){
            adptr->notify_server_release(this);
            m_server_adaptors.erase(found);
        }
    }

    // adaptor

    bool adaptor::load(int count){
        purge();
        bool retval = false;

        typedef listener::client_set::iterator citerator;
        for (citerator i = m_connected_clients.begin(); i != m_connected_clients.end(); i++){
            (*i)->load(count);
        }

        return retval;
    }

    bool adaptor::load(int count, struct timespec timeout){
        purge();
        bool retval = false;

        typedef listener::client_set::iterator citerator;
        for (citerator i = m_connected_clients.begin(); i != m_connected_clients.end(); i++){
            (*i)->load(count, timeout);
        }

        return retval;
    }    
    
    adaptor::~adaptor(){ ; }
    
    server_adaptor::~server_adaptor(){
        for (server_set::iterator i = m_connected_servers.begin(); i != m_connected_servers.end(); i++){
            (*i)->del_adaptor(this);
        }
        m_connected_servers.clear();
    }
    
    void server_adaptor::notify_server_bind(server * notifier){
        assert(notifier != NULL);
        m_connected_servers.insert(notifier);
    }
    void server_adaptor::notify_server_release(server * notifier){
        m_connected_servers.erase(notifier);
    }

    namespace internals {
        convertor_list get_libkerat_convertors() {
            using namespace libkerat::internals::parsers;
            convertor_list retval;
            
            /*** global messages */
            retval.push_back(message_convertor_entry(libkerat::message::frame::PATH, &parse_frm));
            retval.push_back(message_convertor_entry(libkerat::message::alive::PATH, &parse_alv));

            /*** component messages */
            retval.push_back(message_convertor_entry(libkerat::message::pointer::PATH_2D, &parse_ptr_2d));
            retval.push_back(message_convertor_entry(libkerat::message::token::PATH_2D, &parse_tok_2d));
            retval.push_back(message_convertor_entry(libkerat::message::bounds::PATH_2D, &parse_bnd_2d));
            retval.push_back(message_convertor_entry(libkerat::message::symbol::PATH, &parse_sym));

            retval.push_back(message_convertor_entry(libkerat::message::pointer::PATH_3D, &parse_ptr_3d));
            retval.push_back(message_convertor_entry(libkerat::message::token::PATH_3D, &parse_tok_3d));
            retval.push_back(message_convertor_entry(libkerat::message::bounds::PATH_3D, &parse_bnd_3d));

            /*** geometry messages */
            retval.push_back(message_convertor_entry(libkerat::message::convex_hull::PATH, &parse_chg));
            retval.push_back(message_convertor_entry(libkerat::message::outer_contour::PATH, &parse_ocg));
            retval.push_back(message_convertor_entry(libkerat::message::inner_contour::PATH, &parse_icg));
            retval.push_back(message_convertor_entry(libkerat::message::skeleton::PATH_2D, &parse_skg_2d));
            retval.push_back(message_convertor_entry(libkerat::message::skeleton::PATH_3D, &parse_skg_3d));
            retval.push_back(message_convertor_entry(libkerat::message::skeleton_volume::PATH, &parse_svg));
            retval.push_back(message_convertor_entry(libkerat::message::area::PATH, &parse_arg));
            retval.push_back(message_convertor_entry(libkerat::message::raw::PATH, &parse_raw));

            /*** content messages */
            retval.push_back(message_convertor_entry(libkerat::message::control::PATH, &parse_ctl));
            retval.push_back(message_convertor_entry(libkerat::message::data::PATH, &parse_dat));
            retval.push_back(message_convertor_entry(libkerat::message::signal::PATH, &parse_sig));

            /*** association messages */
            retval.push_back(message_convertor_entry(libkerat::message::alive_associations::PATH, &parse_ala));
            retval.push_back(message_convertor_entry(libkerat::message::container_association::PATH, &parse_coa));
            retval.push_back(message_convertor_entry(libkerat::message::link_association::PATH, &parse_lia));
            retval.push_back(message_convertor_entry(libkerat::message::linked_list_association::PATH, &parse_lla));
            retval.push_back(message_convertor_entry(libkerat::message::linked_tree_association::PATH, &parse_lta));
            
            return retval;
        }
        
        
    }
    
}

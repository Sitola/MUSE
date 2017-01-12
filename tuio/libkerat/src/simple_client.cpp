/**
 * \file      simple_client.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-09-27 11:11 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/tuio_messages.hpp>
#include <kerat/simple_client.hpp>
#include <kerat/utils.hpp>
#include <kerat/parsers.hpp>
#include <lo/lo.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>

namespace libkerat {

    using namespace message;

    int simple_client_lo_message_handler(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data);

    simple_client::simple_client(uint16_t port, bool accept_unknown)
        throw (libkerat::exception::net_setup_error)
        :m_foreign(false), m_accept_unknown(accept_unknown), m_last_events_count(0)
    {

        char buffer[8]; //5 should be enough
        memset(buffer, 0, 8);
        sprintf(buffer, "%hu", port);

        m_lo_serv = lo_server_new(buffer, NULL);
        if (m_lo_serv == NULL){
            char buffer[40];
            sprintf(buffer, "Unable to bind to port %u!", port);
            throw libkerat::exception::net_setup_error(buffer);
        }

        // standard tuio messages
        init_standard_convertors();

        // anonymous (unknown) messages
        if (accept_unknown) {
            lo_server_add_method(m_lo_serv, NULL, NULL, &lo_message_handler, this);
        }
        
        m_lo_fd = lo_server_get_socket_fd(m_lo_serv);

        m_reader = (m_lo_fd > 0)?&simple_client::read_socket_fd_existing:&simple_client::read_socket_fd_nonexisting;
    }

    simple_client::simple_client(lo_server instance, bool accept_unknown)
        throw (libkerat::exception::net_setup_error)
        :m_lo_serv(instance), m_foreign(true), 
        m_accept_unknown(accept_unknown), m_last_events_count(0)
    {
        if (m_lo_serv == NULL){
            throw libkerat::exception::net_setup_error("Given lo_server instance is NULL!");
        }
        
        // standard tuio messages
        init_standard_convertors();
        
        // anonymous (unknown) messages
        if (accept_unknown) {
            lo_server_add_method(m_lo_serv, NULL, NULL, &lo_message_handler, this);
        }

        m_lo_fd = lo_server_get_socket_fd(m_lo_serv);
        m_reader = (m_lo_fd > 0)?&simple_client::read_socket_fd_existing:&simple_client::read_socket_fd_nonexisting;
    }

    simple_client::~simple_client(){
        if (m_foreign){
            lo_server_del_method(m_lo_serv, "/tuio2/[!_]*", NULL);
        } else {
            lo_server_free(m_lo_serv);
        }

        for (convertor_map::iterator i = m_convertors.begin(); i != m_convertors.end(); i++){
            delete i->second;
            i->second = NULL;
        }
        m_convertors.clear();
    }

    bool simple_client::load(int count){
        struct timespec timeout;
        timeout.tv_sec = 2;
        timeout.tv_nsec = 0;
        return load(count, timeout);
    }
    bool simple_client::load(int count, timespec timeout){
        purge();

        (this->*m_reader)(count, timeout);
        notify_listeners();

        return m_received_frames.get_length() > 0;
    }

    void simple_client::purge(){ bm_stack_clear(m_received_frames); }

    simple_client::message_convertor_entry::message_convertor_entry(){ ; }

    simple_client::message_convertor_entry::message_convertor_entry(std::string path, message_convertor conv, void * data)
        :callback_stg(conv, data)
    { m_osc_paths.push_back(path); }
    
    simple_client::message_convertor_entry::message_convertor_entry(std::list<std::string> paths, message_convertor conv, void * data)
        :callback_stg(conv, data), m_osc_paths(paths)
    { ; }

    bool simple_client::message_convertor_entry::operator==(const simple_client::message_convertor_entry & other){
        return callback_stg::operator==(other)
            && (m_osc_paths == other.m_osc_paths);
    }
    
    int simple_client::lo_message_handler(const char* path, const char* types, lo_arg** argv, int argc, lo_message msg, void* user_data){
        if (user_data == NULL){ return -1; }
        simple_client * cl = static_cast<simple_client *>(user_data);

        cl->m_results.clear();

        convertor_map::const_iterator entry = cl->m_convertors.find(path);
        bool retval = false;

        // convertor for given message type not found!
        if (entry == cl->m_convertors.end()){
            if (cl->m_accept_unknown){
                kerat_message * rslt = NULL;
                retval = internals::parsers::parse_generic_osc_message(&rslt, path, msg);
                if (retval && (rslt != NULL)){
                    cl->m_results.push_back(rslt);
                }
            } else {
                if (!cl->m_foreign){
                    lo_server_del_method(cl->m_lo_serv, path, types);
                }
                return -2;
            }
        } else {
            message_convertor ptr = entry->second->m_callback;
            retval = (*ptr)(cl->m_results, path, types, argv, argc, entry->second->m_user_data);
        }
            
        bool accept_this_message = !cl->m_current_bundle.empty();

        for (internals::convertor_output_container::iterator result = cl->m_results.begin(); result != cl->m_results.end(); ++result){

            const libkerat::message::frame * frm = dynamic_cast<const libkerat::message::frame *>(*result);

            // frm means ensure that the "buffer" is empty
            // current budle is empty only if the previous bundle was ended with alive message
            // There for, incomplete bundle leaves mess and has to be cleared
            if (frm != NULL){
                bm_handle_clear(cl->m_current_bundle);
                accept_this_message = true;
            }

            if (accept_this_message) {
                bm_handle_insert(cl->m_current_bundle, bm_handle_end(cl->m_current_bundle), *result);
                const libkerat::message::alive * alv = dynamic_cast<const libkerat::message::alive *>(*result);
                if (alv != NULL){
                    bm_stack_append(cl->m_received_frames, bm_handle_clone(cl->m_current_bundle));
                    bm_handle_clear(cl->m_current_bundle);
                }
                *result = NULL;
            }

            if (*result != NULL){ delete *result; }
        }
        cl->m_results.clear();
        
        return !retval;
    }

    simple_client::message_convertor_entry simple_client::enable_convertor(const message_convertor_entry& convertor){
        message_convertor_entry * conv = new message_convertor_entry;

        conv->m_user_data = convertor.m_user_data;
        conv->m_callback = convertor.m_callback;

        // for each path that does not exist yet, add
        for (std::list<std::string>::const_iterator path = convertor.m_osc_paths.begin(); path != convertor.m_osc_paths.end(); path++){
            convertor_map::const_iterator i = m_convertors.find(*path);
            if (i == m_convertors.end()){
                conv->m_osc_paths.push_back(*path);
                m_convertors.insert(convertor_map::value_type(*path, conv));
                lo_server_add_method(m_lo_serv, path->c_str(), NULL, &lo_message_handler, this);
            }
        }

        return *conv;
    }

    simple_client::message_convertor_entry simple_client::disable_convertor(const std::string & path){
        message_convertor_entry retval;

        convertor_map::iterator i = m_convertors.find(path);
        if (i != m_convertors.end()){
            i->second->m_osc_paths.remove(path);
            retval = *(i->second);

            // cleanup
            if (i->second->m_osc_paths.empty()){ delete i->second; }
            lo_server_del_method(m_lo_serv, path.c_str(), NULL);
            m_convertors.erase(i);
        }

        return retval;
    }

    void simple_client::disable_convertors(const std::list<std::string> & pathlist){
        for (std::list<std::string>::const_iterator path = pathlist.begin(); path != pathlist.end(); path++){
            disable_convertor(*path);
        }
    }

    bool simple_client::set_accept_unknown(bool accept){
        bool retval = m_accept_unknown;
        m_accept_unknown = accept;
        return retval;
    }

    bundle_stack simple_client::get_stack() const { return m_received_frames; }

    void simple_client::read_socket_fd_existing(int count, timespec timeout){

        bool keep = true;
        m_last_events_count = 0;

        struct timespec currtime;
        clock_gettime(CLOCK_REALTIME, &currtime);
        struct timespec limit = nanotimeradd(currtime, timeout);

        while (keep){

            // check whether we're still supposed to be running
            clock_gettime(CLOCK_REALTIME, &currtime);
            struct timespec remaining = nanotimersub(limit, currtime);
            if (((count - m_last_events_count) <= 0) || (remaining.tv_sec < 0)){ keep = false; continue; }

            fd_set keeper;
            FD_ZERO(&keeper);
            FD_SET(m_lo_fd, &keeper);

            int retval = pselect(m_lo_fd + 1, &keeper, NULL, NULL, &remaining, NULL);

            if (retval == -1) {
                std::cerr << "Failed to read data from lo_fd" << std::endl;
            } else if (retval > 0) {
                if (FD_ISSET(m_lo_fd, &keeper)) {
                    lo_server_recv_noblock(m_lo_serv, 0);
                    ++m_last_events_count;
                }
            }

        }
    }


    void simple_client::read_socket_fd_nonexisting(int count, timespec timeout){

        bool keep = true;
        m_last_events_count = 0;

        struct timespec currtime;
        clock_gettime(CLOCK_MONOTONIC, &currtime);
        struct timespec limit = nanotimeradd(currtime, timeout);

        while (keep){
            // check whether we're still supposed to be running
            clock_gettime(CLOCK_MONOTONIC, &currtime);
            struct timespec remaining = nanotimersub(limit, currtime);
            if (((m_last_events_count - count) <= 0) || (remaining.tv_sec < 0)){ keep = false; continue; }

            fd_set keeper;
            FD_ZERO(&keeper);
            FD_SET(0, &keeper);

            int retval = pselect(m_lo_fd + 1, &keeper, NULL, NULL, &remaining, NULL);

            if (retval == -1) {
                std::cerr << "Failed to read data from lo_fd" << std::endl;
            }

            lo_server_recv_noblock(m_lo_serv, 0);
        }
    }

    void simple_client::init_standard_convertors(){
        internals::convertor_list convertors(internals::get_libkerat_convertors());
        std::for_each(convertors.begin(), convertors.end(), get_enabler_functor());
    }
    
    simple_client::convertor_enable_functor simple_client::get_enabler_functor() {
        return convertor_enable_functor(*this);
    }
    
    simple_client::convertor_enable_functor::convertor_enable_functor(simple_client & client)
        :m_client(client)
    { ; }

    void simple_client::convertor_enable_functor::operator()(const simple_client::message_convertor_entry & convertor){
        m_client.enable_convertor(convertor);
    }
} // ns libkerat


/**
 * \file      simple_server.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-10-20 10:17 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/simple_server.hpp>
#include <lo/lo.h>
#include <string>
#include <iostream>

namespace libkerat {

    simple_server::simple_server()
        :m_target(NULL), m_frame_template(OUT_OF_ORDER_ID), m_printer(std::cerr)
    {
        m_timetag.sec = 0;
        m_timetag.frac = 1;
    }
    
    simple_server::simple_server(lo_address target_client) throw (libkerat::exception::net_setup_error)
        :m_target(NULL), m_frame_template(OUT_OF_ORDER_ID), m_printer(std::cerr)
    {
        m_timetag.sec = 0;
        m_timetag.frac = 1;

        set_target(target_client);
    }

    simple_server::simple_server(std::string target_url, std::string appname, addr_ipv4_t address,
            instance_id_t instance, dimmension_t sensor_width, dimmension_t sensor_height
    ) throw (libkerat::exception::net_setup_error)
        :m_target(NULL), m_frame_template(OUT_OF_ORDER_ID), m_printer(std::cerr)
    {
        size_t proto = target_url.find("://");
        if ((proto == std::string::npos) || (proto > target_url.find_first_of(":/"))){
            target_url = std::string("osc.udp://").append(target_url);
        }
        m_target = lo_address_new_from_url(target_url.c_str());

        if (m_target == NULL){
            std::string error_message = "Failed to create OSC client for url: ";
            error_message.append(target_url);
            throw libkerat::exception::net_setup_error(error_message);
        }
        
        m_frame_template.set_app_name(appname);
        m_frame_template.set_address(address);
        m_frame_template.set_instance(instance);
        m_frame_template.set_sensor_width(sensor_width);
        m_frame_template.set_sensor_height(sensor_height);
    }



    simple_server::~simple_server(){
        if (m_target != NULL){
            lo_address_free(m_target);
            m_target = NULL;
        }
    }

    void simple_server::set_target(const lo_address target_client)
        throw (libkerat::exception::net_setup_error)
    {
        if (m_target != NULL){
            lo_address_free(m_target);
            m_target = NULL;
        }

        if (target_client != NULL){
            m_target = lo_address_new_with_proto(
                lo_address_get_protocol(target_client),
                lo_address_get_hostname(target_client),
                lo_address_get_port(target_client)
            );

            if (m_target == NULL){
                throw libkerat::exception::net_setup_error("Failed to create OSC client");
            }

            lo_address_set_ttl(m_target, lo_address_get_ttl(target_client));
        }
    }

    bool simple_server::append_clone(const kerat_message* msg){
        if (msg == NULL){ return false; }

        bool retval = false;

        if (dynamic_cast<const libkerat::message::frame*>(msg) != NULL){
            m_frame_template = *dynamic_cast<const libkerat::message::frame*>(msg);
            retval = true;
        } else if (dynamic_cast<const libkerat::message::alive*>(msg) != NULL) {
            retval = false;
        } else {
            retval = server::append_clone(msg);
        }

        return retval;
    }

    void simple_server::clear_message_stack(){
        for (handle_iterator i = bm_handle_begin(m_bundle); i != bm_handle_end(m_bundle); i++){
            delete *i;
            *i = NULL;
        }

        bm_handle_clear(m_bundle);
    }
    
    void simple_server::set_timetag(lo_timetag bundle_timetag){
        m_timetag = bundle_timetag;
    }

    bool simple_server::prepare_bundle(){
        bool retval = true;
        
        // setup & append frame message
        message::frame msg_frame = m_frame_template;
        lo_timetag frame_timestamp;
        lo_timetag_now(&frame_timestamp);
        msg_frame.set_frame_id(get_next_frame_id());
        msg_frame.set_timestamp(frame_timestamp);
        
        bm_handle_insert(m_bundle, bm_handle_begin(m_bundle), msg_frame.clone());

        // append alv message
        message::alive msg_alive(get_session_ids());
        bm_handle_insert(m_bundle, bm_handle_end(m_bundle), msg_alive.clone());

        return retval;
    }

    bool simple_server::commit(){

        bool retval = true;

        // setup bundle
        lo_bundle bundle = lo_bundle_new(m_timetag);

        // setup & append frame message
        for (bundle_handle::const_iterator i = m_bundle.begin(); i != m_bundle.end(); i++){
            bool tmpretval = imprint_bundle(bundle, *i);
            // if an error has occured, print the message which failed
            if (!tmpretval){ 
                std::cerr << "libkerat::simple_server::send: Failed to imprint message: ";
                m_printer.print(*i);
                std::cerr << std::endl;
            }
            retval &= tmpretval;
        }

        if (m_target != NULL){
            // send bundle
            lo_send_bundle(m_target, bundle);
        }

        // cleanup
        lo_bundle_free_messages(bundle);
        bundle = NULL;
        clear_message_stack();

        // next bundle timetag setup
        m_timetag.frac = 1;
        m_timetag.sec = 0;

        return retval;
    }
    
    
} // ns libkerat

/**
 * @file      wrapper.hpp
 * @brief     hold the actual wrapper core
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2011-08-11 13:28 UTC+2
 * @copyright BSD
 */

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <linux/input.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <endian.h>

#include <algorithm>
#include <cstring>
#include <cmath>
#include <iostream>
#include <map>
#include <list>
#include <kerat/kerat.hpp>

#include "event.hpp"
#include "common.hpp"
#include "eventdumper.hpp"
#include "wrapper.hpp"
#include "geometry.hpp"
#include "axis.hpp"

#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

struct server_imprinter: std::unary_function<const libkerat::kerat_message *, void> {
    server_imprinter(libkerat::server * target_server):m_server(target_server){ ; }
    ~server_imprinter(){ ; }

    void operator()(const libkerat::kerat_message * msg) const { m_server->append_clone(msg);  }

    libkerat::server * m_server;
};

ev_code_t kinput_wrapper::mt_type_b_get_slot(event_t & ev){
    event_t::component_map_t::iterator slot = ev.m_components.find(ABS_MT_SLOT);
    if (slot == ev.m_components.end()){
        ev.m_components[ABS_MT_SLOT].value = m_main_buffer.begin()->first;
        return mt_type_b_get_slot(ev);
    } else {
        return slot->second.value;
    }
}

void kinput_wrapper::mt_type_b_update_buffer(event_buffer_t & buffer, event_t event){
    if (!event.m_components.empty()){
        #ifdef DEBUG
            print_event(event);
        #endif
        ev_value_t slot = mt_type_b_get_slot(event);
        buffer[slot] = event;
    }
}

void kinput_wrapper::mt_type_a_update_buffer(event_buffer_t & buffer, const event_t & event){
    if (!event.m_components.empty()){
        #ifdef DEBUG
            print_event(event);
        #endif
        buffer[buffer.size()] = event;
    }
}

kinput_wrapper::kinput_wrapper(const wrapper_config& config)
    :m_tuio_server(config.target_addr, config.app_name, config.local_ip, config.instance, config.virtual_sensor_width, config.virtual_sensor_height),
    m_dtuio_sa(config.prepared_dtuio, libkerat::adaptors::append_adaptor::message_list(), 1),
    m_axes_mappings(config.axes_mappings), m_axes_ranges(config.axes_ranges), m_geometry(config),
    m_btn_touch_active(-1), m_empty_cycle(true),
    m_type(MULTITOUCH_TYPE_B),
    m_transformations_enabled(!config.disable_transformations),
    m_user_id(getuid()),
    m_offset_x(config.x), m_offset_y(config.y)

{
    init_mappings();

    // setup dtuio
    m_tuio_server.add_adaptor(&m_dtuio_sa);
    
    // send pre-connect echo, serves as recovery message against false contacts
    //m_tuio_server.send();
    force_commit();
    m_dtuio_sa.set_update_interval(7);
}


kinput_wrapper::~kinput_wrapper(){

    // close relation with clients
    commit();
    m_tuio_server.clear_session_registry();
    m_tuio_server.send();
    
}

static void mt_type_generic_coords(event_t ev, int * coords);
static int64_t mt_type_a_distance(int const * acords, int const * bcords);
ev_code_t mt_type_b_get_slot(event_t & ev);
static bool has_component(const event_t::component_map_t & components, int component_id);

void kinput_wrapper::init_mappings(){
    // higher priority mappings

    bool pressure_found = false;
    bool x_found = false;
    bool y_found = false;
    bool z_found = false;
    for (axis_mapping_map::const_iterator i = m_axes_mappings.begin(); i != m_axes_mappings.end(); i++){
        switch (i->second.code){
            case ABS_PRESSURE: { pressure_found |= true; break; }
            case ABS_X: { x_found |= true; break; }
            case ABS_Y: { y_found |= true; break; }
            case ABS_Z: { z_found |= true; break; }
        }
    }
    
    if (!pressure_found) {
        m_axes_mappings.insert(axis_mapping_map::value_type(ABS_MT_PRESSURE, axis_mapping(ABS_PRESSURE, ABS_CNT + 1)));
    } else {
        // prevent default behaviour
        axis_mapping_map::const_iterator default_setting = m_axes_mappings.find(ABS_PRESSURE);
        if (default_setting == m_axes_mappings.end()){
            m_axes_mappings.insert(axis_mapping_map::value_type(ABS_PRESSURE, MAPPING_IGNORE));
        }
    }
    
    if (!x_found) {
        m_axes_mappings.insert(axis_mapping_map::value_type(ABS_MT_POSITION_X, axis_mapping(ABS_X, ABS_CNT + 1)));
    } else {
        // prevent default behaviour
        axis_mapping_map::const_iterator default_setting = m_axes_mappings.find(ABS_X);
        if (default_setting == m_axes_mappings.end()){
            m_axes_mappings.insert(axis_mapping_map::value_type(ABS_X, MAPPING_IGNORE));
        }
    }
    
    if (!y_found) {
        m_axes_mappings.insert(axis_mapping_map::value_type(ABS_MT_POSITION_Y, axis_mapping(ABS_Y, ABS_CNT + 1)));
    } else {
        // prevent default behaviour
        axis_mapping_map::const_iterator default_setting = m_axes_mappings.find(ABS_Y);
        if (default_setting == m_axes_mappings.end()){
            m_axes_mappings.insert(axis_mapping_map::value_type(ABS_Y, MAPPING_IGNORE));
        }
    }
    
    if (!z_found) {
        m_axes_mappings.insert(axis_mapping_map::value_type(ABS_MT_DISTANCE,   axis_mapping(ABS_Z, 0)));
    } else {
        // prevent default behaviour
        axis_mapping_map::const_iterator default_setting = m_axes_mappings.find(ABS_Z);
        if (default_setting == m_axes_mappings.end()){
            m_axes_mappings.insert(axis_mapping_map::value_type(ABS_Z, MAPPING_IGNORE));
        }
    }
}

// auxiliary for merge_buffers_mt_type_a
static void mt_type_generic_coords(event_t ev, int * coords){

    ev_code_t coord_ids[] = { ABS_X, ABS_Y, ABS_Z };

    for (unsigned int i = 0; i < DIMENSIONS; i++){
        coords[i] = ev.m_components[coord_ids[i]].value;
    }
}

// auxiliary for merge_buffers_mt_type_a
static int64_t mt_type_a_distance(int const * acords, int const * bcords){

    int64_t retval = 0;

    for (unsigned int i = 0; i < DIMENSIONS; i++){

        int acord = acords[i];
        int bcord = bcords[i];

        unsigned int dist = (acord > bcord)?acord-bcord:bcord-acord;

        // prevent overflow
        int64_t dist2 = dist;
            dist2 *= dist;

        retval += dist2;

    }

    return retval;
}

void kinput_wrapper::mt_type_b_merge_buffers(){
    for (event_buffer_t::iterator i = m_op_buffer.begin(); i != m_op_buffer.end(); ++i){

        event_t & the_event = i->second;

        ev_value_t slot = mt_type_b_get_slot(the_event);
        if (m_main_buffer.find(slot) != m_main_buffer.end()){

            event_t & previous_event = m_main_buffer[slot];
            if (event_should_send(previous_event, the_event)){
                 previous_event.flag(event_t::SEND_BIT, 1);
            }

            previous_event.update(the_event);

            event_t::component_map_t::const_iterator id = the_event.m_components.find(ABS_MT_TRACKING_ID);
            if ((id != the_event.m_components.end()) && (id->second.value == -1)){
                m_main_buffer[slot].flag(event_t::REMOVE_BIT, 1);
            }
        } else {
            the_event.flag(event_t::SEND_BIT, 1);
            m_main_buffer[slot] = the_event;
        }

    }
}

void kinput_wrapper::mt_type_a_merge_buffers(){

    // this constructs the distances grid for nearest-neighbour search

    size_t const main_size = m_main_buffer.size();
    size_t const update_size = m_op_buffer.size();

    uint64_t * distances = (uint64_t *)alloca(update_size * main_size * sizeof(uint64_t));

    ev_value_t * precomputed_coordinates_m = (ev_value_t *)alloca(main_size * DIMENSIONS * sizeof(ev_value_t));
    ev_value_t * precomputed_coordinates_u = (ev_value_t *)alloca(update_size*DIMENSIONS * sizeof(ev_value_t));

    // precompute the coordinates and set updates status
    size_t i = 0;
    for (event_buffer_t::iterator current = m_main_buffer.begin(); current != m_main_buffer.end(); ++current, ++i){
        current->second.flag(event_t::REMOVE_BIT, 1);
        current->second.flag(event_t::SEND_BIT, 0);
        mt_type_generic_coords(current->second, &precomputed_coordinates_m[(i * DIMENSIONS) + 0]);
    }
    
    i = 0;
    for (event_buffer_t::iterator current = m_op_buffer.begin(); current != m_op_buffer.end(); ++current, ++i){
        m_op_buffer[i].flag(event_t::SEND_BIT, 1);
        mt_type_generic_coords(m_op_buffer[i], &precomputed_coordinates_u[(i * DIMENSIONS) + 0]);
    }

    for (size_t i = 0; i < update_size; i++){
        for (size_t j = 0; j < main_size; j++){
            distances[(i * main_size) + j] = mt_type_a_distance(&precomputed_coordinates_u[i * DIMENSIONS], &precomputed_coordinates_m[j * DIMENSIONS]);
        }
    }

    // now, perform the matching

    bool keep_nn = true;
    uint64_t const maxdist = -1;
    for (size_t update_index = 0; (update_index < update_size) && keep_nn; update_index++){

        uint64_t nn_distance = maxdist;
        
        event_buffer_t::iterator nn_update = m_op_buffer.end();
        event_buffer_t::iterator nn_main = m_main_buffer.end();
        
        size_t i = 0;
        for (event_buffer_t::iterator update_iter = m_op_buffer.begin(); update_iter != m_op_buffer.end(); ++update_iter, ++i){
            size_t j = 0;
            for (event_buffer_t::iterator main_iter = m_main_buffer.begin(); main_iter != m_main_buffer.end(); ++main_iter, ++j){
                uint64_t cd = distances[(i * main_size) + j];
                if (cd < nn_distance){ // < instead of <= due to use-first policy, update neighbour
                    nn_update = update_iter;
                    nn_main = main_iter;
                    nn_distance = cd;
                }
            }
        }

        // this is valid for both under and overlimit events
        { 
            size_t main_distance = std::distance(m_main_buffer.begin(), nn_main);
            for (size_t i = 0; i < update_size; i++) {
                distances[(i * main_size) + main_distance] = maxdist;
            }
        }
        
        // join_limit is squared due to no-square-root policy
        #if defined ENABLE_LIMIT
            uint64_t limit = m_join_distance_limit; limit *= limit;
            if ((nn_distance != maxdist) && (nn_distance < (limit))){
        #else
            if (nn_distance != maxdist){
        #endif
                // perform merging

                // values changed, send
                if (event_should_send(nn_main->second, nn_update->second)){
                    nn_main->second.flag(event_t::SEND_BIT, 1);
                }
                nn_main->second.update(nn_update->second);
                nn_main->second.flag(event_t::REMOVE_BIT, 0);
                nn_update->second.flag(event_t::SEND_BIT, 0);
                nn_update->second.flag(event_t::REMOVE_BIT, 1);

                size_t update_distance = std::distance(m_op_buffer.begin(), nn_update);
                for (size_t j = 0; j < main_size; j++) {
                    distances[(update_distance * main_size) + j] = maxdist;
                }
        #if defined ENABLE_LIMIT
            } else {
                keep_nn = false;
        #endif
        }

    }

    for (event_buffer_t::iterator update_iter = m_op_buffer.begin(); update_iter != m_op_buffer.end(); ++update_iter){
        if (update_iter->second.flag(event_t::SEND_BIT)){
            m_main_buffer[m_main_buffer.size()] = update_iter->second;
        }
    }

    m_op_buffer.clear();
}



int kinput_wrapper::process_event(const struct input_event* data){

    int retval = 0;

    // compatibility hack for MT_TYPE_A devices - all fingers are considered removed
    // if empty sync arrives (this prevents the clean on unrecognized events)

    switch (data->type){

        case EV_ABS: {
            // for N:1 axis mapping with priority
            if (data->code > ABS_MAX){
                std::cerr << "Invalid data code " << data->code << " received, is this even compiled for this kernel?";
                return -2;
            }

            axis_mapping mapping = get_mapping(data->code);
            retval = 1;

            // proces EV_ABS
            switch (mapping.code){
                case MAPPING_IGNORE_CODE: {
                    break;
                }
                case ABS_MT_TRACKING_ID: {
                    if (m_type != MULTITOUCH_TYPE_B_FORCED){ m_type = MULTITOUCH_TYPE_A; }
                }
                case ABS_X:
                case ABS_Y:
                case ABS_Z:
                case ABS_PRESSURE:
                case ABS_MT_WIDTH_MAJOR:
                case ABS_MT_WIDTH_MINOR:
                case ABS_MT_TOUCH_MAJOR:
                case ABS_MT_TOUCH_MINOR:
                case ABS_MT_ORIENTATION:
                case ABS_TOOL_WIDTH: {
                    m_current_event.update_based_on_priority(mapping.code, event_component(mapping.priority, data->value, data->code));
                    m_current_event.m_timestamp = data->time;
                    m_empty_cycle = false;
                    break;
                }

                case ABS_MT_SLOT: {

                    // if slot received, set multitouch type to B
                    m_type = MULTITOUCH_TYPE_B_FORCED;

                    // slot is kind of small sync, so update operational buffer
                    mt_type_b_update_buffer(m_op_buffer, m_current_event);

                    // clean the event
                    m_current_event.m_components.clear();

                    // slot information needs to be stored as well for merge to work properly
                    m_current_event.update_based_on_priority(mapping.code, event_component(mapping.priority, data->value, data->code));

                    break;
                }
                default: {
                    std::cerr << "Unhandled mapping/event " << mapping.code << "/" << data->type << ":" << data->code << std::endl;
                }

            }
            break;
        }
        case EV_KEY: {
            retval = 1;

            switch (data->code){
                // ddr fix
                case BTN_TOUCH: 
                case 288: 
                case 289: 
                case 290: 
                case 291: 
                case 292: {
                    if ((data->value == 0) && (m_btn_touch_active > 0)){
                        m_btn_touch_active--;
                    } else if (data->value != 0){
                        if (m_btn_touch_active < 0){ m_btn_touch_active = 0; }
                        m_btn_touch_active++;
                    }
                    break;
                }
                // TODO respond to BTN_TOOL_FINGER
            }
            break;
        }
        case EV_SYN: {
            // Process sync events -
            switch (data->code){
                case SYN_MT_REPORT: {
                    // SYN_MT_REPORT is similar to SYN_MT_SLOT - it is used by drivers that don't use SYN_MT_SLOT
                    mt_type_a_update_buffer(m_op_buffer, m_current_event);
                    m_current_event.m_components.clear();
                    break;
                }
                case SYN_REPORT: {
                    // commented out since the single-touch is almost identical with both multi and single touch
                    // if (type == MULTITOUCH_TYPE_B){ // in case of being re-enabled, do not forget to change it appropriately
                    // MT_TYPE_B does not call explicit store, that means that the last event was not stored yet
                    // Apple's magick trackpad multitouch results in empty event being stored
                    if (!m_current_event.m_components.empty()){
                        if (m_type == MULTITOUCH_TYPE_A){
                            mt_type_a_update_buffer(m_op_buffer, m_current_event);
                        } else {
                            mt_type_b_update_buffer(m_op_buffer, m_current_event);
                        }
                        m_current_event.m_components.clear();
                    }
                    // }

                    commit();
                    m_empty_cycle = true;
                    retval = 1;
                    break;
                }
                case SYN_DROPPED: {
                    retval = -1;
                    break;
                }
                default: {
                    // whops, this should not occur - and if it does, it should not affect the function
                    std::cerr << "Unhandled EV_SYN/" << data->code << std::endl;
                }
            }

            break;
        }
        default: {
            // seriously, are we still reading touch sensor?
            std::cerr << "Unhandled " << data->type << "/" << data->code << std::endl;
        }
    }

    return retval;
}

void kinput_wrapper::commit(){
    
    // merge buffers and clean the op buffer but only if nonempty pass
//    if ((!m_current_event.components.empty()) || (!m_op_buffer.empty())){
    if (!m_op_buffer.empty()){
        if (m_type == MULTITOUCH_TYPE_A){
            mt_type_a_merge_buffers();
        } else {
            mt_type_b_merge_buffers();
            // DDR devices workaround
         
            if (m_btn_touch_active == 0){
                for (event_buffer_t::iterator current = m_main_buffer.begin(); current != m_main_buffer.end(); current++){
                    // remove all event groups and do not send them
                    current->second.flag(event_t::REMOVE_BIT, 1);
                    current->second.flag(event_t::SEND_BIT, 0);
                }
            }
        }
        m_op_buffer.clear();

    } else if ((m_btn_touch_active <= 0) && m_empty_cycle){
        // MT type A devices tend to emmit contact removal event (btn_touch_active)
        // if no contacts are present anymore. However, some (Iiyama touchscreens)
        // emmit finger (btn_touch_active), yet they do not emmit empty cycle, so
        // this can be used to distinguish this two types

        for (event_buffer_t::iterator current = m_main_buffer.begin(); current != m_main_buffer.end(); current++){
            // remove all event groups and do not send them
            current->second.flag(event_t::REMOVE_BIT, 1);
            current->second.flag(event_t::SEND_BIT, 0);
        }
    }


    size_t pointers_to_send = 0;
    // add pointers into the message and mark non-updated messages to be removed
    for (event_buffer_t::iterator current = m_main_buffer.begin(); current != m_main_buffer.end(); current++){
        event_t & current_group = current->second;

        // remove empty event groups and send non-empty ones
        if (current_group.m_components.empty()){
            current_group.flag(event_t::REMOVE_BIT, 1);
        } else if (current_group.flag(event_t::SEND_BIT)) {

            if (current_group.m_session_id == 0){
                current_group.m_session_id = m_tuio_server.get_auto_session_id();
            }

            geometry_point original_point(current_group.m_components[ABS_X].value, current_group.m_components[ABS_Y].value);
            geometry_point mapped_point = original_point;
            if (m_transformations_enabled){
                geometry_point mapped_point2 = m_geometry.transform(mapped_point);
/*
                if (mapped_point2.x < 0){
                    std::cerr << "error in point " << mapped_point.x << "->" << mapped_point2.x << " " << mapped_point.y << "->" << mapped_point2.y << std::endl;
                }
*/
                mapped_point = mapped_point2;
            }
            
            
            current_group.update_history(current_group.m_timestamp, mapped_point);

            libkerat::helpers::message_output_mode output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_2D);
            if (current_group.m_components.find(ABS_Z) != current_group.m_components.end()){
                output_mode.set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_3D);
            }
            
            // add pointer 
            {
                double width = current_group.m_components[ABS_TOOL_WIDTH].value;
                double pressure = mtw_evdev_axis_relative_value(
                    m_axes_ranges[current_group.m_components[ABS_PRESSURE].source], 
                    current_group.m_components[ABS_PRESSURE].value
                );
                
                libkerat::message::pointer pointer(current_group.m_session_id, libkerat::helpers::contact_type_user::TYPEID_UNKNOWN, m_user_id, 0,
                    mapped_point.x + m_offset_x, mapped_point.y + m_offset_y,
                    width, pressure
                );

                pointer.set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_2D);
                
                { // check for velocity
                    libkerat::velocity_t vel_x, vel_y;
                    if (current_group.get_velocity(vel_x, vel_y)){
                        pointer.set_x_velocity(vel_x);
                        pointer.set_y_velocity(vel_y);
                    }
                }
                { // check for acceleration
                    libkerat::accel_t accel;
                    if (current_group.get_accel(accel)){
                        pointer.set_acceleration(accel);
                    }
                }
                
                m_tuio_server.append_clone(&pointer);
            }
            
            // add bounds (if exists) TODO: proper units for bounds ellipse
            if (has_component(current_group.m_components, ABS_MT_TOUCH_MAJOR)){
                double original_angle = current_group.m_components[ABS_MT_ORIENTATION].value;
                original_angle = 0.5 + mtw_evdev_axis_relative_value(m_axes_ranges[current_group.m_components[ABS_MT_ORIENTATION].source], original_angle);
                // rotate angle to be compatible with CCW with 0 for right-direction of x-axis
                //double angle = -original_angle; angle -=1;
                const double runtime_pi = acos(-1.0);
                double angle = (original_angle -0.5) * runtime_pi; // convert to radians with proper orientation
                
                double main_axis = current_group.m_components[ABS_MT_TOUCH_MAJOR].value;
                // specs say that in case of circular contact, ABS_MT_TOUCH_MINOR might be left out
                double aux_axis = (current_group.m_components.find(ABS_MT_TOUCH_MINOR) != current_group.m_components.end())
                    ?current_group.m_components[ABS_MT_TOUCH_MINOR].value
                    :current_group.m_components[ABS_MT_TOUCH_MAJOR].value
                ;

                // rescale the contact bounds
                if (m_transformations_enabled){
                    geometry_point corner(main_axis /= 2, aux_axis /= 2);
                    geometry_point right_upper_corner = original_point + corner;
                    geometry_point left_bottom_corner = original_point - corner;
                    
                    right_upper_corner = m_geometry.transform(right_upper_corner);
                    left_bottom_corner = m_geometry.transform(left_bottom_corner);
                    
                    main_axis = abs(right_upper_corner.x - left_bottom_corner.x);
                    aux_axis = abs(right_upper_corner.y - left_bottom_corner.y);
                }
                
                double area = acos(-1.0)*main_axis*aux_axis;
                
                libkerat::message::bounds bounds(current_group.m_session_id,
                    mapped_point.x + m_offset_x, mapped_point.y + m_offset_y,
                    angle,
                    main_axis, aux_axis,
                    area
                );
                
                m_tuio_server.append_clone(&bounds);
            }
            
            current_group.flag(event_t::SEND_BIT, 0);
            ++pointers_to_send;

        }
    }

    // send data
    if (pointers_to_send > 0){ m_tuio_server.send(); }

    // remove the dead pointers - has to come after server.send
    for (event_buffer_t::iterator current = m_main_buffer.begin(); current != m_main_buffer.end(); ){
        event_buffer_t::iterator next = current;
        ++next;
        
        // remove marked event groups
        if (current->second.flag(event_t::REMOVE_BIT)){
            m_tuio_server.unregister_session_id(current->second.m_session_id);
            m_main_buffer.erase(current);
        }
        
        current = next;
    }

    // if no pointer remainds, clean the sensor since the next "iteration" might be a bit long
    if (m_main_buffer.empty()){ m_tuio_server.send(); }

    std::cout << "\r" << "Tracking " << m_main_buffer.size() << " fingers"; std::cout.flush();

}

bool has_component(const event_t::component_map_t & components, int component_id){
    return components.find(component_id) != components.end();
}

axis_mapping kinput_wrapper::get_mapping(ev_code_t axis) const {
    axis_mapping_map::const_iterator mapping_exists = m_axes_mappings.find(axis);
    if (mapping_exists != m_axes_mappings.end()){
        return mapping_exists->second;
    } else {
        return axis_mapping(axis, 2);
    }
}

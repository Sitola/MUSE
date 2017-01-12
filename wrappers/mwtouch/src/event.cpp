/**
 * @file      event.cpp
 * @brief     Implements the event storage related functionality
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2011-08-11 17:14 UTC+2
 * @copyright BSD
 */

#include <inttypes.h>
#include <linux/input.h>
#include <deque>
#include <iostream>

#include "common.hpp"
#include "eventdumper.hpp"
#include "event.hpp"
#include "geometry_primitives.hpp"


event_component::event_component():priority(0),value(0),source(ABS_CNT){ ; }
event_component::event_component(priority_t prio, ev_value_t val, ev_code_t source_axis)
    :priority(prio),value(val),source(source_axis)
{ ; }

event_t::event_t()
    :m_operation(0),m_session_id(0)
{
    m_timestamp.tv_sec = 0;
    m_timestamp.tv_usec = 0;
}

void event_t::update_based_on_priority(const ev_code_t component, const event_component & update_data){
    event_t::component_map_t::iterator i = m_components.find(component);
    if (i != m_components.end()){
        if (i->second.priority <= update_data.priority){ i->second = update_data; }
    } else {
        m_components[component] = update_data;
    }
  
}

void print_event(const event_t & event){
    std::cout.flush();
//    std::cout << std::endl << "---------------------------------------------- event" << std::endl;
    event_t::component_map_t::const_iterator end = event.m_components.end();
    for (event_t::component_map_t::const_iterator curr = event.m_components.begin(); curr != end; curr++){
        std::cout << get_abs_ev_name(curr->first) << "=" << curr->second.value << "[" << curr->second.priority << "]" << std::endl;
    }
    std::cout << "--------" << std::endl;

//    std::cout << "---------------------------------------------- event" << std::endl;
}

priority_t event_t::get_component_priority(const ev_code_t component){
    component_map_t::const_iterator i = m_components.find(component);
    return (i != m_components.end())?i->second.priority:0;
}


bool event_t::flag(event_t::op_t flag, uint8_t value){
    bool has = this->flag(flag);

    op_t tmp = 1 << flag;
    tmp = ~tmp;
    m_operation &= tmp;
    m_operation |= value << flag;

    return has;
}


bool event_t::flag(event_t::op_t flag){
    return (m_operation & (1 << flag)) == (1 << flag);
}

void event_t::update(const event_t & update){
    event_t::component_map_t::const_iterator end = update.m_components.end();
    for (event_t::component_map_t::const_iterator curr = update.m_components.begin(); curr != end; curr++){
        m_components[curr->first] = curr->second;
    }
    m_timestamp = update.m_timestamp;
}

void event_t::update_history(const timeval contact_time, const geometry_point & pt){
    // if already full, cut the oldest one
    if (m_history.size() > 4){ m_history.erase(m_history.begin()); }
    
    double timestamp = contact_time.tv_usec;
        timestamp /= 1000000;
        timestamp += contact_time.tv_sec;
        
    m_history.push_back(position_record(timestamp, pt));
}

bool event_t::get_velocity(libkerat::velocity_t& vel_x, libkerat::velocity_t& vel_y) const {
    size_t hist_size = m_history.size();
    if (hist_size < 2){ return false; }
    
    geometry_vect velocity_vect(m_history[hist_size-2].position, m_history[hist_size-1].position);
    velocity_vect /= (m_history[hist_size-1].timestamp - m_history[hist_size-2].timestamp);
    
    vel_x = velocity_vect.x;
    vel_y = velocity_vect.y;
    return true;
}

bool event_t::get_accel(libkerat::accel_t& accel) const {
    const int POLYNOMIAL_DEGREE = 3;
    
    if (m_history.size() < (POLYNOMIAL_DEGREE + 1)){ return false; }
    
    double timestamps[POLYNOMIAL_DEGREE];
    double velocities[POLYNOMIAL_DEGREE];
    
    // prepare velocity history
    for (int i = 0; i < POLYNOMIAL_DEGREE; i++) { 
        // timestamps store relative timestamp
        timestamps[i] = m_history[i+1].timestamp - m_history[0].timestamp;
        geometry_vect tmp(m_history[i+1].position, m_history[i].position);
//        tmp /= (m_history[i+1].timestamp - m_history[i].timestamp);
        velocities[i] = tmp.norm();
    }
    // use Lagrange's polynomial interpolation
    // then derive it - warning, this is the 3rd degree polynomial only!
    int compute_for = POLYNOMIAL_DEGREE-1;
    accel = 0;
    
    for (int i = 0; i < POLYNOMIAL_DEGREE; i++) {
        double tmp = velocities[i];
        
        double aux = 0;
        for (int j = 0; i < POLYNOMIAL_DEGREE; i++) { if (i != j){ aux += timestamps[j]; } }
        tmp *= (2*velocities[compute_for]) - aux;
        
        aux = 1;
        for (int j = 0; i < POLYNOMIAL_DEGREE; i++) { if (i != j){ aux *= (timestamps[i] - timestamps[j]); } }
        tmp /= aux;
        accel += tmp;
    }

    return true;
}

bool event_should_send(const event_t & old, const event_t & current){

    bool should = false;

    ev_code_t tags[] = { ABS_X, ABS_Y, ABS_PRESSURE, ABS_TOOL_WIDTH };

    for (unsigned int i = 0; i < ((sizeof(tags)/sizeof(ev_code_t))); i++){
        event_t::component_map_t::const_iterator a = old.m_components.find(tags[i]);
        event_t::component_map_t::const_iterator b = current.m_components.find(tags[i]);

        // update if value added or if different
        should |= (
            ((a != old.m_components.end()) && ((b != current.m_components.end()) && (a->second.value != b->second.value))) ||
            ((a == old.m_components.end()) && (b != current.m_components.end()))
        );
    }

    return should;

}



event_t::op_t const event_t::SEND_BIT = 1;
event_t::op_t const event_t::REMOVE_BIT = 2;
event_t::op_t const event_t::NOOP       = 0;


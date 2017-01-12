/**
 * @file      event.hpp
 * @brief     Provides the event storage
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2011-03-25 17:14 UTC+2
 * @copyright BSD
 */

#ifndef MWTOUCH_EVENT_HPP
#define MWTOUCH_EVENT_HPP

#include <inttypes.h>
#include <linux/input.h>
#include <kerat/typedefs.hpp>
#include <map>
#include <list>
#include <deque>
#include <iostream>

#include "common.hpp"
#include "eventdumper.hpp"
#include "geometry_primitives.hpp"

struct event_component {
public:
    event_component();
    event_component(priority_t prio, ev_value_t val, ev_code_t source_axis);

    /**
     * Higher priority overwrites lower priority
     */
    priority_t priority;
    
    /**
     * Holds the event value (input_event.value)
     */
    ev_value_t value;
    
    /**
     * Holds the original source axis of this event
     */
    ev_code_t source;
};

class event_t {
public:
    event_t();

    typedef std::map<ev_code_t, event_component> component_map_t;
    typedef uint8_t op_t;

    /**
     * Holds individual event components
     * map criteria: (input_event.code)
     */
    component_map_t m_components;

    /**
     * Holds the operation over the event buffer for this event
     */
    op_t m_operation;

    /**
     * session id assigned to the event
     */
    session_t m_session_id;
    
    struct timeval m_timestamp;
    
    priority_t get_component_priority(const ev_code_t component);
    void update(const event_t & update);
    bool flag(event_t::op_t flag, uint8_t value);
    bool flag(event_t::op_t flag);

    static op_t const SEND_BIT;
    static op_t const REMOVE_BIT;
    static op_t const NOOP;

    void update_based_on_priority(const ev_code_t component, const event_component & update_data);
    
    void update_history(const timeval contact_time, const geometry_point & pt);
    
    bool get_velocity(libkerat::velocity_t & vel_x, libkerat::velocity_t & vel_y) const;
    bool get_accel(libkerat::accel_t & accel) const;

private:    
    struct position_record {
        position_record(double n_timestamp, const geometry_point & pt):timestamp(n_timestamp), position(pt){ ; }
        position_record():timestamp(0), position(0, 0){ ; }
        
        double timestamp;
        geometry_point position;
    };

    typedef std::vector<position_record> position_history_t;

    /**
     * For computing the acceleration and velocity
     */
    position_history_t m_history;

};

void print_event(const event_t & event);

typedef std::map<ev_value_t, event_t> event_buffer_t;


bool event_should_send(const event_t & old, const event_t & current);

#endif // MWTOUCH_EVENT_HPP


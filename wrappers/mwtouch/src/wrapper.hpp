/**
 * @file      wrapper.hpp
 * @brief     hold the public wrapper core functionality and provides it's module (can be 'directly' imported into other programs)
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2011-08-11 13:28 UTC+2
 * @copyright BSD
 */

#ifndef MWTOUCH_WRAPPER_HPP
#define MWTOUCH_WRAPPER_HPP

#include <stdlib.h>
#include <errno.h>
#include <linux/input.h>
#include <sys/stat.h>
#include <endian.h>

#include <cstring>
#include <iostream>
#include <algorithm>
#include <map>
#include <list>

#include <kerat/kerat.hpp>

#include "nodeconfig.hpp"
#include "event.hpp"
#include "common.hpp"
#include "geometry.hpp"
#include "axis.hpp"

/**
 * \brief This class represents the actual kernel input to tuio wrapper.
 */
class kinput_wrapper {
public:
    /**
     * As of the kernel multitouch input protocol, two versions exist:
     * - anonymous (A) - full event set sent each time, no ABS_MT_SLOT - finger liftup signalized by removal from the set
     * - anonymous (B) - registers each ABS_MT_TRACKINGID to ABS_MT_SLOT - finger liftup signalized by ABS_MT_TRACKINGID -1
     */
    typedef enum {MULTITOUCH_TYPE_A, MULTITOUCH_TYPE_B, MULTITOUCH_TYPE_B_FORCED} multitouch_t;
    
    kinput_wrapper(const wrapper_config & config);
//    kinput_wrapper(const kinput_wrapper & original);
    ~kinput_wrapper();
    
//    kinput_wrapper & operator=(const kinput_wrapper & original);
    
    /**
     * Process the event. 
     * 
     * @param data - event to be processed
     * @return 0 if ignored, positive if accepted and -1 if syn_dropped and device reset is required
     */
    int process_event(const struct input_event * data);
    
    /**
     * No data shall be read anymore! Force the commit!
     */
    inline void force_commit(){ commit(); }

    inline multitouch_t get_type() const { return m_type; }
    
    inline bool is_empty() const {
        return ((m_btn_touch_active <= 0) && (!m_main_buffer.empty()));
    }
    
    inline bool set_empty(){
        m_op_buffer.clear();
        m_main_buffer.clear();
        m_tuio_server.clear_session_registry();
        return true;
    }
    
private:
    unsigned int m_join_distance_limit;
    
    libkerat::simple_server m_tuio_server;
    libkerat::adaptors::append_adaptor m_dtuio_sa;
    axis_mapping_map m_axes_mappings;
    axis_map m_axes_ranges;
    
    
    coordmapper m_geometry;

    int m_btn_touch_active;
    bool m_empty_cycle;
    
    event_t m_current_event;
    multitouch_t m_type;

    event_buffer_t m_op_buffer;
    bool m_transformations_enabled;
    libkerat::user_id_t m_user_id;
    event_buffer_t m_main_buffer;
    
    // offsets
    libkerat::coord_t m_offset_x;
    libkerat::coord_t m_offset_y;
    
    // mt type differentiation
    void mt_type_b_merge_buffers();
    void mt_type_a_merge_buffers();
    ev_code_t mt_type_b_get_slot(event_t & ev);
    void mt_type_b_update_buffer(event_buffer_t & buffer, event_t event);
    void mt_type_a_update_buffer(event_buffer_t & buffer, const event_t & event);
    
    axis_mapping get_mapping(ev_code_t axis) const;
    
    // auxiliary
    void init_mappings();

    /**
     * \brief do the TUIO 2 communication and send the data
     */
    void commit();
    
};

#endif // MWTOUCH_WRAPPER_HPP

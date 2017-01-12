/**
 * \file      primitive_touch.hpp
 * \brief     Provides the means of matching the pointer inside the bounding box generated by the client
 * \author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-03-18 16:55 UTC+1
 * \copyright BSD
 */

#ifndef MUSE_PRIMITIVE_VIRTUAL_TOUCH_HPP
#define MUSE_PRIMITIVE_VIRTUAL_TOUCH_HPP

#include <kerat/kerat.hpp>

namespace muse {
    namespace virtual_sensors {

        class primitive_touch: public libkerat::adaptor,
            private libkerat::internals::frame_manager,
            private libkerat::internals::session_manager
        {
        public:

            primitive_touch();
            primitive_touch(uint32_t join_treshold, libkerat::timetag_t timetag);

            ~primitive_touch();

            void notify(const libkerat::client * notifier);

            libkerat::bundle_stack get_stack() const { return m_processed_frames; }

            int process_bundle(const libkerat::bundle_handle & to_process, libkerat::bundle_handle & output_frame);

            void purge();

            bool load(int count = 1);
            bool load(int count, struct timespec timeout);

            static const int32_t COORD_OUT_OF_BOUNDS = -65535;
        private:

            struct id_stamp {
                id_stamp(){
                    mapped_sid = 0;
                    since.sec = 0;
                    since.frac = 0;
                }

                libkerat::session_id_t mapped_sid;
                libkerat::timetag_t since;
            };

            struct alv_stamp {
                alv_stamp();

                libkerat::timetag_t since;
                libkerat::timetag_t waiting_since;
                libkerat::coord_t last_x;
                libkerat::coord_t last_y;
                libkerat::coord_t last_z;
                libkerat::session_id_t session_id;
            };

            struct source_key_type {
                bool operator<(const source_key_type & second) const {
                    if (addr < second.addr) {
                        return true;
                    } else if (addr == second.addr){
                        if (instance < second.instance){ return true; }
                    }
                    return false;
                }

                libkerat::addr_ipv4_t addr;
                libkerat::instance_id_t instance;
            };

            typedef std::map<libkerat::session_id_t, id_stamp> internal_session_id_map;
            typedef std::map<source_key_type, internal_session_id_map> session_id_map;
            typedef std::vector<alv_stamp> free_vector;
            typedef std::map<libkerat::session_id_t, libkerat::helpers::point_2d> sid_point_map;
            typedef std::set<libkerat::client *> client_set;

            libkerat::session_id_t get_mapped_id(const libkerat::message::frame & msg_frame, const libkerat::session_id_t sid);

            void update_idmap(const libkerat::message::frame & msg_frame, libkerat::message::alive::alive_ids update);

            id_stamp allocate_session_id(const alv_stamp & whom);

            void imprint_coordinates(alv_stamp & stamp, const libkerat::session_id_t session_id) const;

            void clean_idmap();

            bool m_updates_waiting;

            uint32_t m_join_treshold_squared;
            libkerat::timetag_t m_delta_time;

            free_vector m_avail_to_join;
            session_id_map m_mapping;
            sid_point_map m_objects;

            libkerat::bundle_stack m_processed_frames;
        };

    }
}

#endif // MUSE_PRIMITIVE_VIRTUAL_TOUCH_HPP

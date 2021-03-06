/**
 * \file      primitive_touch.cpp
 * \brief     Provides the means of matching the pointer inside the bounding box generated by the client
 * \author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-03-18 16:55 UTC+1
 * \copyright BSD
 */

#include <kerat/kerat.hpp>
#include <muse/primitive_touch.hpp>

namespace muse {

    namespace virtual_sensors {

        void primitive_touch::notify(const libkerat::client * cl){
            // do not forget to clean the previous stack!
            purge();

            libkerat::bundle_stack data = cl->get_stack();
            while (data.get_length() > 0){
                libkerat::bundle_handle current_frame = data.get_update(libkerat::bundle_stack::INDEX_OLDEST);
                libkerat::bundle_handle * tmphx = new libkerat::bundle_handle;
                process_bundle(current_frame, *tmphx);

                if (tmphx->begin() == tmphx->end()){ delete tmphx; continue; }

                bm_stack_append(m_processed_frames, tmphx);

            }

            clean_idmap();
            if (m_processed_frames.get_length()){ notify_listeners(); }
        }

        void primitive_touch::purge(){ bm_stack_clear(m_processed_frames); }
        
        primitive_touch::alv_stamp::alv_stamp(){
            since.sec = 0;
            since.frac = 0;
            last_x = COORD_OUT_OF_BOUNDS;
            last_y = COORD_OUT_OF_BOUNDS;
            last_z = COORD_OUT_OF_BOUNDS;
            session_id = 0;
            lo_timetag_now(&waiting_since);
        }

        primitive_touch::primitive_touch()
            :m_updates_waiting(false),m_join_treshold_squared(4000)
        {
            m_delta_time.sec = 0;
            m_delta_time.frac = (((uint32_t)-1)/10)*4;
        }

        primitive_touch::~primitive_touch(){
            ;
        }

        primitive_touch::primitive_touch(uint32_t join_treshold, libkerat::timetag_t time_treshold)
            :m_updates_waiting(false),m_join_treshold_squared(join_treshold * join_treshold), m_delta_time(time_treshold)
        {
            ;
        }

        bool primitive_touch::load(int count){

            if (!m_avail_to_join.empty()){
                timespec t;
                t.tv_sec = 0;
                t.tv_nsec = 100000000;
                return load(count, t);
            }

            bool retval = libkerat::adaptor::load(count);

            // clean_ids
            if (!retval){
                clean_idmap();
                if (m_updates_waiting){
                    get_next_frame_id();
                    notify_listeners();
                }
            }
            return retval;
        }

        bool primitive_touch::load(int count, timespec timeout){

            bool retval = libkerat::adaptor::load(count, timeout);

            // clean_ids
            if (!retval){
                clean_idmap();
                if (m_updates_waiting){
                    get_next_frame_id();
                    notify_listeners();
                }
            }
            return retval;
        }

        int primitive_touch::process_bundle(const libkerat::bundle_handle & to_process, libkerat::bundle_handle & output_frame){

            typedef libkerat::bundle_handle::const_iterator const_iterator;

            const libkerat::message::frame * orig_msg_frm = to_process.get_frame();
            const libkerat::message::alive * orig_msg_alv = to_process.get_alive();
            if ((orig_msg_frm == NULL) || (orig_msg_alv == NULL)){
                // cannot not-have frame and alive message
                return -1;
            }

            for (const_iterator i = to_process.begin(); i != to_process.end(); i++){

                libkerat::kerat_message * tmp = (*i)->clone();

                libkerat::helpers::point_2d * hp_point = dynamic_cast<libkerat::helpers::point_2d *>(tmp);
                libkerat::helpers::contact_session * hp_session = dynamic_cast<libkerat::helpers::contact_session *>(tmp);
                libkerat::message::frame * msg_frm = dynamic_cast<libkerat::message::frame *>(tmp);
                libkerat::message::alive * msg_alv = dynamic_cast<libkerat::message::alive *>(tmp);

                if ((hp_point != NULL) && (hp_session != NULL)){

                    libkerat::session_id_t tmpsid = hp_session->get_session_id();
                    libkerat::session_id_t sid = get_mapped_id(*orig_msg_frm, tmpsid);
                    if (sid == 0){
                        alv_stamp st;
                        //st.session_id = tmpsid;
                        st.last_x = hp_point->get_x();
                        st.last_y = hp_point->get_y();
                        st.since = orig_msg_frm->get_timestamp();

                        id_stamp ids = allocate_session_id(st);

                        source_key_type source;
                        source.addr = orig_msg_frm->get_address();
                        source.instance = orig_msg_frm->get_instance();

                        (m_mapping[source])[tmpsid] = ids;
                        sid = ids.mapped_sid;
                    }

                    // from now on, consider id safely mapped
                    // update and store last known coordinates
                    m_objects[sid] = *hp_point;
                    hp_session->set_session_id(sid);
                } else if (msg_frm != NULL){
                    msg_frm->set_frame_id(get_next_frame_id());
                } else if (msg_alv != NULL){
                    msg_alv->clear();
                    libkerat::session_set alives;

                    for (session_id_map::const_iterator sr = m_mapping.begin(); sr != m_mapping.end(); sr++){
                        for (internal_session_id_map::const_iterator sid = sr->second.begin(); sid != sr->second.end(); sid++){
                            alives.insert(sid->second.mapped_sid);
                        }
                    }
                    for (primitive_touch::free_vector::iterator i = m_avail_to_join.begin(); i != m_avail_to_join.end(); i++){
                        alives.insert(i->session_id);
                    }

                    msg_alv->set_alives(alives);
                }

                bm_handle_insert(output_frame, bm_handle_end(output_frame), tmp);
                tmp = NULL;
            }

            update_idmap(*orig_msg_frm, orig_msg_alv->get_alives());
            
            return 0;
        }

        session_id_t primitive_touch::get_mapped_id(const libkerat::message::frame & msg_frame, const libkerat::session_id_t sid){

            source_key_type source;
            source.addr = msg_frame.get_address();
            source.instance = msg_frame.get_instance();

            session_id_t retval;
            internal_session_id_map & tmpmap = m_mapping[source];

            internal_session_id_map::const_iterator sval = tmpmap.find(sid);
            if (sval == tmpmap.end()){
                retval = 0;
            } else {
                retval = sval->second.mapped_sid;
            }

            return retval;

        }

        void primitive_touch::update_idmap(const libkerat::message::frame & frame, libkerat::message::alive::alive_ids update){

            typedef libkerat::message::alive::alive_ids alive_ids;

            source_key_type source;
            source.addr = frame.get_address();
            source.instance = frame.get_instance();
            const libkerat::timetag_t frame_timestamp = frame.get_timestamp();
            internal_session_id_map & srcmap = m_mapping[source];


            { // remove mappings for already-present nodes
                const internal_session_id_map srcmap_full(srcmap);
                alive_ids::const_iterator update_end = update.end();
                alive_ids::const_iterator update_begin = update.begin();

                for (internal_session_id_map::const_iterator current = srcmap_full.begin(); current != srcmap_full.end(); current++){
                    alive_ids::const_iterator cs = std::find(update_begin, update_end, current->first);
                    if (cs != update_end){
                        update.erase(cs);
                    } else {
                        alv_stamp tmpst;
                        tmpst.since = frame_timestamp;
                        tmpst.session_id = get_mapped_id(frame, current->first);
                        imprint_coordinates(tmpst, tmpst.session_id);
                        m_avail_to_join.push_back(tmpst);
                        srcmap.erase(current->first);
                    }
                }
            }


            // create new id mappings immediately
            { // add mappings for yet unmapped id's
                alive_ids::const_iterator update_end   = update.end();
                alive_ids::const_iterator update_begin = update.begin();

                for (alive_ids::iterator current = update_begin; current != update_end; current++){
                    // allocate pure new id using empty source stamp
                    
                    alv_stamp tmpst;
                    srcmap[*current] = allocate_session_id(tmpst);
                }
            }

        }

        void primitive_touch::imprint_coordinates(primitive_touch::alv_stamp & stamp, const libkerat::session_id_t session_id) const {
            sid_point_map::const_iterator i = m_objects.find(session_id);

            stamp.last_x = COORD_OUT_OF_BOUNDS;
            stamp.last_y = COORD_OUT_OF_BOUNDS;
            stamp.last_z = COORD_OUT_OF_BOUNDS;

            if (i == m_objects.end()) return;

            stamp.last_x = i->second.get_x();
            stamp.last_y = i->second.get_y();

        }

        primitive_touch::id_stamp primitive_touch::allocate_session_id(const primitive_touch::alv_stamp & whom){

            typedef primitive_touch::free_vector::iterator iterator;
            using libkerat::coord_t;
            using libkerat::session_id_t;
            using libkerat::timetag_t;

            id_stamp retval;
            
            for (iterator i = m_avail_to_join.begin(); (retval.mapped_sid == 0) && (i != m_avail_to_join.end()); i++){
                alv_stamp & scanned = *i;

                coord_t dx = scanned.last_x - whom.last_x; dx *= dx;
                coord_t dy = scanned.last_y - whom.last_y; dy *= dy;
                coord_t dz = scanned.last_z - whom.last_z; dz *= dz;
                timetag_t timediff = libkerat::timetag_diff_abs(whom.since, scanned.since);

                if ((dx+dy+dz) <= m_join_treshold_squared){
                    if (timediff < m_delta_time){
                        retval.since = scanned.since;
                        retval.mapped_sid = scanned.session_id;
                        m_avail_to_join.erase(i);
                    }
                }
            }

            if (retval.mapped_sid == 0) {
                retval.since = whom.since;
                retval.mapped_sid = get_auto_session_id();
            }

            return retval;
        }

        void primitive_touch::clean_idmap(){
            libkerat::timetag_t currtime;
            lo_timetag_now(&currtime);

            libkerat::timetag_t droptime = libkerat::timetag_add(m_delta_time, m_delta_time);

            typedef primitive_touch::free_vector::iterator iterator;
            iterator end = m_avail_to_join.end();
            for (iterator i = m_avail_to_join.begin(); (i != end); i++){
                alv_stamp & scanned = *i;
                libkerat::timetag_t timediff = libkerat::timetag_diff_abs(currtime, scanned.waiting_since);
                if (droptime < timediff){
                    m_updates_waiting = true;
                    m_objects.erase(scanned.session_id);
                    m_avail_to_join.erase(i);
                }
            }
        }
    }
}

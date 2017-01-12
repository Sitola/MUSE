/**
 * \file      dtuio_marker.cpp
 * \brief     Adaptor that changes plain TUIO 2.0 bundles into dTUIO bundles
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-12-03 15:33 UTC+1
 * \copyright BSD
 */

#ifdef HAVE_CONFIG_H
    #include "../config.h"
#endif

#include <dtuio/sensor_properties.hpp>
#include <dtuio/dtuio_marker.hpp>
#include <dtuio/helpers.hpp>
#include <kerat/utils.hpp>
#include <uuid/uuid.h>

namespace dtuio {
    namespace adaptors {
        marker::marker(
            sensor::sensor_properties::coordinate_translation_mode_t mode, 
            sensor::sensor_properties::sensor_purpose_t purpose
        )
            :m_mode(mode), m_purpose(purpose)
        { ; }
        
        marker::~marker(){
            for (injections_map::iterator i = m_injections.begin(); i != m_injections.end(); ++i){
                delete i->second;
                i->second = NULL;
            }
            m_injections.clear();
        }
        
        void marker::set_default_coordinate_translation_mode(const sensor::sensor_properties::coordinate_translation_mode_t & mode){
            m_mode = mode;
            
            // update
            for (injections_map::iterator i = m_injections.begin(); i != m_injections.end(); ++i){
                i->second->set_coordinate_translation_mode(m_mode);
            }
        }

        sensor::sensor_properties::coordinate_translation_mode_t marker::get_default_coordinate_translation_mode() const {
            return m_mode;
        }

        void marker::set_default_sensor_purpose(const sensor::sensor_properties::sensor_purpose_t & purpose){
            m_purpose = purpose;
            
            // update
            for (injections_map::iterator i = m_injections.begin(); i != m_injections.end(); ++i){
                i->second->set_sensor_purpose(m_purpose);
            }
        }

        sensor::sensor_properties::sensor_purpose_t marker::get_default_sensor_purpose() const {
            return m_purpose;
        }
        
        
        libkerat::bundle_stack marker::get_stack() const { return m_processed_frames; }
        void marker::purge(){ bm_stack_clear(m_processed_frames); }
        
        void marker::notify(const libkerat::client * cl){
            purge();

            libkerat::bundle_stack data = cl->get_stack();
            while (data.get_length() > 0){
                libkerat::bundle_handle current_frame = data.get_update(libkerat::bundle_stack::INDEX_OLDEST);
                libkerat::bundle_handle * processed_bundle = new libkerat::bundle_handle;
                process_bundle(current_frame, *processed_bundle);
                bm_stack_append(m_processed_frames, processed_bundle);
            }

            notify_listeners();
        }

        int marker::process_bundle(const libkerat::bundle_handle& to_process, libkerat::bundle_handle& output_frame){

            // if not the same already
            if (&to_process != &output_frame){
                bm_handle_clear(output_frame);
                bm_handle_copy(to_process, output_frame);
            }
            
            return process_bundle(output_frame);

        }

        int marker::process_bundle(libkerat::bundle_handle & to_process){
            
            // test whether this one requires injection...
            if (to_process.get_message_of_type<sensor::sensor_properties>(0) != NULL){ return 0; }
            
            const libkerat::message::frame * msg_frame = to_process.get_frame();
            if (msg_frame == NULL){ return 1; }
            
            // fill up the signature & look for existing uuid
            source_key_type tuio_signature;
            tuio_signature.addr = msg_frame->get_address();
            tuio_signature.application = msg_frame->get_app_name();
            tuio_signature.instance = msg_frame->get_instance();

            sensor::sensor_properties * sensor_properties = NULL;
            
            injections_map::iterator found = m_injections.find(tuio_signature);
            if (found == m_injections.end()){
                // prepare new
                dtuio::uuid_t uuid;
                uuid_clear(uuid);
                uuid_generate(uuid);

                sensor_properties = new sensor::sensor_properties(uuid, m_mode, m_purpose);
                
                // register new
                m_injections.insert(injections_map::value_type(tuio_signature, sensor_properties));
            } else {
                sensor_properties = found->second;
            }
            
            // commit bundle
            inject(to_process, sensor_properties);
            
            return 0;
        }

        void marker::inject(libkerat::bundle_handle & to_process, sensor::sensor_properties * message){
            handle_iterator target = bm_handle_begin(to_process);
            assert (target == bm_handle_end(to_process));
            ++target; // skip frame
            assert (target == bm_handle_end(to_process));
            
            // insert right after frame
            bm_handle_insert(to_process, target, message->clone());
        }

        bool marker::source_key_type::operator<(const marker::source_key_type & second) const {
            if (addr < second.addr){
                return true;
            } else if (addr == second.addr) {
                if (instance < second.instance){
                    return true;
                } else if (instance == second.instance) {
                    return (application.compare(second.application) < 0);
                }
            }

            return false;
        }
        
    } // ns adaptors
} // ns dtuio

/**
 * \file      viewport_scaler.cpp
 * \brief     Adaptor that scales the received viewport data to target viewport (autoconfiguration support)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2013-05-08 23:49 UTC+2
 * \copyright BSD
 */

#ifdef HAVE_CONFIG_H
    #include "../config.h"
#endif

#include <dtuio/viewport_scaler.hpp>
#include <kerat/utils.hpp>
#include <uuid/uuid.h>
#include <limits>
#include <list>

namespace dtuio {
    namespace adaptors {

        viewport_scaler::viewport_scaler(const sensor::viewport & target) { 
            set_target_viewport(target);
        }
        
        viewport_scaler::~viewport_scaler(){
            
        }
        
        libkerat::bundle_stack viewport_scaler::get_stack() const { return m_processed_frames; }
        void viewport_scaler::purge(){ bm_stack_clear(m_processed_frames); }
        
        void viewport_scaler::notify(const libkerat::client * cl){
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

        int viewport_scaler::process_bundle(const libkerat::bundle_handle& to_process, libkerat::bundle_handle& output_frame){
            // if not the same already
            if (&to_process != &output_frame){
                bm_handle_copy(to_process, output_frame);
            }
            
            // update the current viewport & drop other viewport intel
            process_bundle(output_frame);
            
            return 0;

        }
        
        int viewport_scaler::process_bundle(libkerat::bundle_handle& to_process){
            typedef std::list<libkerat::internals::bundle_manipulator::handle_iterator> iterator_list;
            
            // list of viewports to invalidate
            iterator_list remove;
            bool interresting = false;
            
            // scalings
            double factor_x = 1.0;
            double factor_y = 1.0;
            double factor_z = 1.0;
            libkerat::helpers::point_3d scale_center;
            
            for (handle_iterator current = bm_handle_begin(to_process); current != bm_handle_end(to_process); ++current){
                sensor::viewport * msg_vpt = dynamic_cast<sensor::viewport *>(*current);
                if (msg_vpt != NULL){
                    // compare uuids, process only if matching, otherwise set remove list
                    if (*((dtuio::helpers::uuid *)msg_vpt) == m_target){
                        interresting = true;
                        scale_center = *msg_vpt;

                        // setup scaling factors                        
                        if ((m_target.get_width() != 0) && (msg_vpt->get_width() != 0)){
                            factor_x = m_target.get_width(); // typecast
                            factor_x /= msg_vpt->get_width();
                        }
                        if ((m_target.get_height() != 0) && (msg_vpt->get_height() != 0)){
                            factor_y = m_target.get_height();
                            factor_y /= msg_vpt->get_height();
                        }
                        if ((m_target.get_depth() != 0) && (msg_vpt->get_depth() != 0)){
                            factor_z = m_target.get_depth();
                            factor_z /= msg_vpt->get_depth();
                        }
                        
                        // replace viewport dimmensions in output
                        msg_vpt->set_width(m_target.get_width());
                        msg_vpt->set_height(m_target.get_height());
                        msg_vpt->set_depth(m_target.get_depth());
                        
                        continue;
                    } else {
                        remove.push_back(current);
                    }
                }
                
                // so far not interresting, does not make sence to do anything here
                if (!interresting){ continue; }
                

                apply_scale(*current, factor_x, factor_y, factor_z, scale_center);
            }
            
            // clean up unused viewports
            while  (interresting && !remove.empty()){
                bm_handle_erase(to_process, remove.front());
                remove.pop_front();
            }
            
            return 0;
        }

        void viewport_scaler::apply_scale(libkerat::kerat_message* msg, const double& factor_x, const double& factor_y, const double& factor_z, const libkerat::helpers::point_3d& scale_center){
            // commance proper scaling
            libkerat::helpers::scalable_2d * msg_sc_2d = dynamic_cast<libkerat::helpers::scalable_2d *>(msg);
            libkerat::helpers::scalable_independent_2d * msg_sci_2d = dynamic_cast<libkerat::helpers::scalable_independent_2d *>(msg);

            if (msg_sc_2d != NULL){
                msg_sc_2d->scale_x(factor_x, scale_center);
                msg_sc_2d->scale_y(factor_y, scale_center);

                libkerat::helpers::scalable_3d * msg_sc_3d = dynamic_cast<libkerat::helpers::scalable_3d *>(msg);
                if (msg_sc_3d != NULL){ msg_sc_3d->scale_z(factor_z, scale_center); }
            } else if (msg_sci_2d != NULL){
                msg_sci_2d->scale_x(factor_x);
                msg_sci_2d->scale_y(factor_y);

                libkerat::helpers::scalable_independent_3d * msg_sci_3d = dynamic_cast<libkerat::helpers::scalable_independent_3d *>(msg);
                if (msg_sci_3d != NULL){ msg_sci_3d->scale_z(factor_z); }
            }
            
            // commance movement of objects
            libkerat::helpers::point_2d * msg_pt_2d = dynamic_cast<libkerat::helpers::point_2d *>(msg);
            libkerat::helpers::movable_2d * msg_mov_2d = dynamic_cast<libkerat::helpers::movable_2d *>(msg);
            if ((msg_mov_2d != NULL) && (msg_pt_2d != NULL)) {
                msg_mov_2d->move_x(factor_x*(msg_pt_2d->get_x() - scale_center.get_x())+scale_center.get_x()-msg_pt_2d->get_x());
                msg_mov_2d->move_y(factor_y*(msg_pt_2d->get_y() - scale_center.get_y())+scale_center.get_y()-msg_pt_2d->get_y());
                
                libkerat::helpers::point_3d * msg_pt_3d = dynamic_cast<libkerat::helpers::point_3d *>(msg);
                libkerat::helpers::movable_3d * msg_mov_3d = dynamic_cast<libkerat::helpers::movable_3d *>(msg);
                if ((msg_mov_3d != NULL) && (msg_pt_3d != NULL)) {
                    msg_mov_3d->move_z(factor_z*(msg_pt_3d->get_z() - scale_center.get_z())+scale_center.get_z()-msg_pt_3d->get_z());
                }
            }
        }
        
        void viewport_scaler::set_target_viewport(const sensor::viewport& target){
            m_target = target;
        }
        
    } // ns adaptors
} // ns dtuio

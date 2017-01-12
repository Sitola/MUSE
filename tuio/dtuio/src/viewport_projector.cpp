/**
 * \file      viewport_projector.cpp
 * \brief     Adaptor that strips the bundles of contacts & information that do belong in given viewport
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-12-03 15:33 UTC+1
 * \copyright BSD
 */

#ifdef HAVE_CONFIG_H
    #include "../config.h"
#endif

#include <dtuio/viewport_projector.hpp>
#include <kerat/utils.hpp>
#include <uuid/uuid.h>
#include <limits>
#include <list>

namespace dtuio {
    namespace adaptors {

        viewport_projector::viewport_projector(helpers::uuid uuid_to_follow, bool strip)
            :m_adaptive(true), m_follow(uuid_to_follow), m_strip(strip)
        {
            m_match.set_uuid(uuid_to_follow.get_uuid());
        }
        
        viewport_projector::viewport_projector(const sensor::viewport & viewport_to_match, bool strip)
            :m_adaptive(false), m_match(viewport_to_match), m_strip(strip)
        { ; }
        
        viewport_projector::~viewport_projector(){
            
        }
        
        libkerat::bundle_stack viewport_projector::get_stack() const { return m_processed_frames; }
        void viewport_projector::purge(){ bm_stack_clear(m_processed_frames); }
        
        void viewport_projector::notify(const libkerat::client * cl){
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

        libkerat::helpers::point_3d viewport_projector::apply_viewport_cs(const libkerat::helpers::point_3d & original, const sensor::viewport & vpt){
            libkerat::helpers::point_3d retval(original);
            
            libkerat::rotate_around_center_yaw(retval, vpt, vpt.get_yaw());
            libkerat::rotate_around_center_pitch(retval, vpt, vpt.get_pitch());
            libkerat::rotate_around_center_roll(retval, vpt, vpt.get_roll());
            
            return retval;
        }

        bool viewport_projector::in_viewport_box(const libkerat::helpers::point_3d & original, const sensor::viewport & vpt){
            libkerat::coord_t tmp_x = original.get_x();
            libkerat::coord_t tmp_y = original.get_y();
            libkerat::coord_t tmp_z = original.get_z();
            
            return (
                (tmp_x >= (vpt.get_x()-vpt.get_width()/2) && tmp_x <= (vpt.get_x()+vpt.get_width()/2))
             && (tmp_y >= (vpt.get_y()-vpt.get_height()/2) && tmp_y <= (vpt.get_y()+vpt.get_height()/2))
             && (tmp_z >= (vpt.get_z()-vpt.get_depth()/2) && tmp_z <= (vpt.get_z()+vpt.get_depth()/2))
            );
        }
        
        int viewport_projector::process_bundle(const libkerat::bundle_handle& to_process, libkerat::bundle_handle& output_frame){
            libkerat::bundle_handle input;
            // if not the same already
            if (&to_process == &output_frame){
                bm_handle_copy(to_process, input);
            } else {
                input = to_process;
            }
            
            bm_handle_clear(output_frame);

            // search for update
            process_viewport_updates(input);
            
            // scan all messages & check whether they are within this viewport; if so then remap
            for (libkerat::bundle_handle::const_iterator i = input.begin(); i != input.end(); ++i){
                if (dynamic_cast<const libkerat::message::frame *>(*i) != NULL){
                    bm_handle_insert(output_frame, bm_handle_end(output_frame), (*i)->clone());
                    
                    // create viewport
                    // no rotations
                    sensor::viewport tmp(m_match);
                    tmp.set_yaw(0);
                    tmp.set_pitch(0);
                    tmp.set_roll(0);
                    
                    // claim only box
                    tmp.set_x(tmp.get_width()/2);
                    tmp.set_y(tmp.get_height()/2);
                    tmp.set_z(tmp.get_depth()/2);
                    
                    bm_handle_insert(output_frame, bm_handle_end(output_frame), tmp.clone());
                    
                    continue;
                } else if (dynamic_cast<const libkerat::message::alive *>(*i) != NULL){
                    bm_handle_insert(output_frame, bm_handle_end(output_frame), (*i)->clone());
                    continue;
                } else if (dynamic_cast<const sensor::viewport *>(*i) != NULL){
                    // stop all outgoing viewport messages if strip is enabled
                    //! \todo make the added viewports rotatable & strip the received viewport
                    if (!m_strip){
                        bm_handle_insert(output_frame, bm_handle_end(output_frame), (*i)->clone());
                    }
                    continue;
                }
                
                // that means not a frame, not a viewport, run tests & rotations
                
                libkerat::kerat_message * new_message = (*i)->clone();
                
                libkerat::helpers::point_2d * hp_2pt = dynamic_cast<libkerat::helpers::point_2d *>(new_message);
                if (hp_2pt != NULL){
                    libkerat::helpers::point_3d * hp_3pt = dynamic_cast<libkerat::helpers::point_3d *>(new_message);
                    
                    libkerat::helpers::point_3d original_position = (hp_3pt != NULL)?*hp_3pt:*hp_2pt;
                    
                    libkerat::helpers::point_3d tmp_coord = apply_viewport_cs(original_position, m_match);
                    // test whether the message isn't trash
                    if (!in_viewport_box(tmp_coord, m_match)){ 
                        delete new_message;
                        new_message = NULL;
                        continue; 
                    }
                    
                    libkerat::helpers::movable_2d * hp_2mv = dynamic_cast<libkerat::helpers::movable_2d *>(new_message);
                    if (hp_2mv != NULL){
                        libkerat::helpers::movable_3d * hp_3mv = dynamic_cast<libkerat::helpers::movable_3d *>(new_message);
                        
                        hp_2mv->move_x(m_match.get_width()/2 - m_match.get_x() + tmp_coord.get_x() - original_position.get_x());
                        hp_2mv->move_y(m_match.get_height()/2 - m_match.get_y() + tmp_coord.get_y() - original_position.get_y());
                        
                        if (hp_3mv != NULL) { 
                            hp_3mv->move_z(m_match.get_depth()/2 - m_match.get_z() + tmp_coord.get_z() - original_position.get_z()); 
                        }
                    }
                }
                
                // yet, it still can be rotated
                libkerat::helpers::rotatable_independent_2d * hp_2r = dynamic_cast<libkerat::helpers::rotatable_independent_2d *>(new_message);
                if (hp_2r != NULL) {
                    libkerat::helpers::rotatable_independent_3d * hp_3r = dynamic_cast<libkerat::helpers::rotatable_independent_3d *>(new_message);
                    
                    if (hp_3r != NULL) {
                        hp_3r->rotate_yaw(m_match.get_yaw());
                        hp_3r->rotate_pitch(m_match.get_pitch());
                        hp_3r->rotate_roll(m_match.get_roll());
                    } else {
                        hp_2r->rotate_by(m_match.get_yaw());
                    }
                }
                
                bm_handle_insert(output_frame, bm_handle_end(output_frame), new_message);
            }
            
            return 0;

        }

        void viewport_projector::get_corners(const sensor::viewport& vpt, libkerat::helpers::point_3d* corners){
            using libkerat::helpers::point_3d;
            
            // setup corners
            corners[0] = point_3d(0, 0, 0);
            corners[1] = point_3d(0, vpt.get_height(), 0);
            corners[2] = point_3d(vpt.get_width(), vpt.get_height(), 0);
            corners[3] = point_3d(vpt.get_width(), 0, 0);
            corners[4] = point_3d(0, 0, vpt.get_depth());
            corners[5] = point_3d(0, vpt.get_height(), vpt.get_depth());
            corners[6] = point_3d(vpt.get_width(), vpt.get_height(), vpt.get_depth());
            corners[7] = point_3d(vpt.get_width(), 0, vpt.get_depth());

            for (size_t i = 0; i < 8; ++i){
                // substract center to get real positions
                corners[i] -= vpt;

                // perform rotations
                libkerat::rotate_around_center_yaw(corners[i], point_3d(), vpt.get_yaw());
                libkerat::rotate_around_center_pitch(corners[i], point_3d(), vpt.get_pitch());
                libkerat::rotate_around_center_roll(corners[i], point_3d(), vpt.get_roll());
            }
        }
        
        void viewport_projector::process_viewport_updates(const libkerat::bundle_handle& to_process){
            if (!m_adaptive) { return; }

            viewport_list viewports;
            const sensor::viewport * msg_vpr = NULL;
            
            // wildcard adaptive matching
            if (helpers::uuid::empty_uuid() == m_match) {
                viewports.push_back(m_match);
                for (size_t i = 0; (msg_vpr = to_process.get_message_of_type<sensor::viewport>(i)) != NULL; ++i){
                    viewports.push_back(*msg_vpr);
                }
                
                m_match = calculate_bounding_viewport(viewports);
            } else {
                for (size_t i = 0; (msg_vpr = to_process.get_message_of_type<sensor::viewport>(i)) != NULL; ++i) {
                    if (m_follow != *msg_vpr){ continue; }
                    m_match = *msg_vpr;
                }
            }
        }

        sensor::viewport viewport_projector::calculate_bounding_viewport(const viewport_projector::viewport_list & viewports){
            libkerat::coord_t min_x = std::numeric_limits<libkerat::coord_t>::max();
            libkerat::coord_t max_x = -std::numeric_limits<libkerat::coord_t>::max();
            libkerat::coord_t min_y = std::numeric_limits<libkerat::coord_t>::max();
            libkerat::coord_t max_y = -std::numeric_limits<libkerat::coord_t>::max();
            libkerat::coord_t min_z = std::numeric_limits<libkerat::coord_t>::max();
            libkerat::coord_t max_z = -std::numeric_limits<libkerat::coord_t>::max();
            libkerat::helpers::point_3d buffer[8];
            
            for (viewport_list::const_iterator viewport = viewports.begin(); viewport != viewports.end(); ++viewport){

                get_corners(*viewport, buffer);

                // find limits
                for (size_t j = 0; j < 8; ++j){
                    libkerat::coord_t tmp_x = buffer[j].get_x();
                    libkerat::coord_t tmp_y = buffer[j].get_y();
                    libkerat::coord_t tmp_z = buffer[j].get_z();

                    min_x = (tmp_x < min_x)?tmp_x:min_x;
                    max_x = (tmp_x > max_x)?tmp_x:max_x;
                    min_y = (tmp_y < min_y)?tmp_y:min_y;
                    max_y = (tmp_y > max_y)?tmp_y:max_y;
                    min_z = (tmp_z < min_z)?tmp_z:min_z;
                    max_z = (tmp_z > max_z)?tmp_z:max_z;
                }
            }
            
            libkerat::distance_t width = max_x - min_x;
            libkerat::distance_t height = max_y - min_y;
            libkerat::distance_t depth = max_z - min_z;
                
            return sensor::viewport(dtuio::helpers::uuid::empty_uuid(),
                libkerat::helpers::point_3d(width/2, height/2, depth/2),
                libkerat::helpers::angle_3d(0, 0, 0),
                width, height, depth
            );
        }
        
    } // ns adaptors
} // ns dtuio

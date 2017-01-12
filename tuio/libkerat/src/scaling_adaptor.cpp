/**
 * \file      scaling_adaptor.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-12-21 23:07 UTC+1
 * \copyright BSD
 */

#include <istream>
#include <kerat/typedefs.hpp>
#include <kerat/scaling_adaptor.hpp>
#include <kerat/tuio_messages.hpp>
#include <kerat/message_helpers.hpp>
#include <cmath>

using namespace libkerat::message;

namespace libkerat {
    namespace adaptors {

        scaling_adaptor::scaling_adaptor(dimmension_t width, dimmension_t height, bool scale_accel)
            :m_x_scaling(1.0), m_y_scaling(1.0), m_z_scaling(1.0),
            m_autoconf(true), m_sc_accel(scale_accel),
            m_x_axis_length(width), m_y_axis_length(height) 
        {
            ;
        }
        
        scaling_adaptor::scaling_adaptor(double x_axis_scaling, double y_axis_scaling, double z_axis_scaling, bool scale_accel)
            :m_x_scaling(x_axis_scaling), m_y_scaling(y_axis_scaling), m_z_scaling(z_axis_scaling),
            m_autoconf(false), m_sc_accel(scale_accel),
            m_x_axis_length(1920), m_y_axis_length(1080) 
        {
            ;
        }
        
        scaling_adaptor::~scaling_adaptor(){
            ;
        }
        
        void scaling_adaptor::notify(const client * cl){
            purge();
            
            bundle_stack data = cl->get_stack();

            while (data.get_length() > 0){
                bundle_handle * tmphx = new bundle_handle;
                bundle_handle current_frame = data.get_update(bundle_stack::INDEX_OLDEST);
                process_bundle(current_frame, *tmphx);
                bm_stack_append(m_processed_frames, tmphx);
            }
            
            notify_listeners();
        }

        accel_t scaling_adaptor::scale_accel(const velocity_t x, const velocity_t y, const velocity_t z, const accel_t acc){
            if (acc == 0){ return 0; }

            accel_t retval = acc;

            long double xs = x; xs *= xs;
            long double ys = y; ys *= ys;
            long double zs = z; zs *= zs;
            long double accs = acc; accs *= accs;

            if (xs != 0){
                long double sfy = ys/xs;
                long double sfz = zs/xs;

                long double mxs = accs/(1+sfy+sfz);
                retval = sqrt(
                    (      mxs * m_x_scaling * m_x_scaling) + // scaled x component of resulting acceleration vector
                    (sfy * mxs * m_y_scaling * m_y_scaling) + // y
                    (sfz * mxs * m_z_scaling * m_z_scaling)   // z
                );
            } else if (ys != 0){
                long double sfx = xs/ys;
                long double sfz = zs/ys;

                long double mys = accs/(sfx+1+sfz);
                retval = sqrt(
                    (sfx * mys * m_x_scaling * m_x_scaling) + // scaled x component of resulting acceleration vector
                    (      mys * m_y_scaling * m_y_scaling) + // y
                    (sfz * mys * m_z_scaling * m_z_scaling)   // z
                );
            } else if (zs != 0){
                long double sfx = xs/zs;
                long double sfy = ys/zs;

                long double mzs = accs/(sfx+sfy+1);
                retval = sqrt(
                    (sfx * mzs * m_x_scaling * m_x_scaling) + // scaled x component of resulting acceleration vector
                    (sfy * mzs * m_y_scaling * m_y_scaling) + // y
                    (      mzs * m_z_scaling * m_z_scaling)   // z
                );
            }

            return retval;
        }

        accel_t scaling_adaptor::scale_accel(const velocity_t x, const velocity_t y, const accel_t acc){
            if (acc == 0){ return 0; }

            accel_t retval = acc;

            long double xs = x; xs *= xs;
            long double ys = y; ys *= ys;
            long double accs = acc; accs *= accs;

            if (xs != 0){
                long double sfy = ys/xs;

                long double mxs = accs/(1+sfy);
                retval = sqrt(
                    (      mxs * m_x_scaling * m_x_scaling) + // scaled x component of resulting acceleration vector
                    (sfy * mxs * m_y_scaling * m_y_scaling)   // y
                );
            }             if (xs != 0){
                long double sfy = ys/xs;

                long double mxs = accs/(1+sfy);
                retval = sqrt(
                    (      mxs * m_x_scaling * m_x_scaling) + // scaled x component of resulting acceleration vector
                    (sfy * mxs * m_y_scaling * m_y_scaling)   // y
                );
            } else if (ys != 0){
                long double sfx = xs/ys;

                long double mys = accs/(sfx+1);
                retval = sqrt(
                    (sfx * mys * m_x_scaling * m_x_scaling) + // scaled x component of resulting acceleration vector
                    (      mys * m_y_scaling * m_y_scaling)   // y
                );
            }
            
            return retval;
        }
        
        int scaling_adaptor::process_bundle(const bundle_handle & to_process, bundle_handle & output_frame){

            // if we're not asked to run inplace, commance copy
            if (&to_process != &output_frame){
                bm_handle_copy(to_process, output_frame);
            }
            
            internal_process_bundle(output_frame);

            // scaling done
            return 0;
        }
        
        int scaling_adaptor::process_bundle(bundle_handle & to_process){
            // autoconf is not allowed in server mode
            if (m_autoconf == true){ return -1; }
            
            internal_process_bundle(to_process);

            // scaling done
            return 0;
        }

        int scaling_adaptor::internal_process_bundle(bundle_handle & to_process){

            // process intermediate messages
            for (handle_iterator i = bm_handle_begin(to_process); i != bm_handle_end(to_process); ++i){

                // let's make sure first, that this message is not both being scaled and setting scaling
                message::frame * msg_frame = dynamic_cast<message::frame *>(*i);
                if (msg_frame != NULL){
                    if (msg_frame->is_extended()){
                        // commit automatic configuration
                        if (m_autoconf){
                            m_x_scaling = m_x_axis_length;
                            m_x_scaling /= msg_frame->get_sensor_width();

                            m_y_scaling = m_y_axis_length;
                            m_y_scaling /= msg_frame->get_sensor_height();
                        }

                        msg_frame->set_sensor_width(m_x_axis_length);
                        msg_frame->set_sensor_height(m_y_axis_length);
                    }
                } else { // from now on, this is considered to be any other kind than msg_frame

                    helpers::scalable_independent_2d * hp_2scalable_i = dynamic_cast<helpers::scalable_independent_2d*>(*i);
                    if (hp_2scalable_i != NULL){
                        helpers::scalable_independent_3d * hp_3scalable_i = dynamic_cast<helpers::scalable_independent_3d*>(*i);
                        hp_2scalable_i->scale_x(m_x_scaling);
                        hp_2scalable_i->scale_y(m_y_scaling);
                        
                        if (hp_3scalable_i != NULL){
                            hp_3scalable_i->scale_z(m_z_scaling);
                        }
                    }
                    
                    helpers::point_2d * hp_2point = dynamic_cast<helpers::point_2d*>(*i);
                    if (hp_2point != NULL) {
                        helpers::point_3d * hp_3point = dynamic_cast<helpers::point_3d*>(*i);

                        helpers::movable_2d * hp_2movable = dynamic_cast<helpers::movable_2d*>(*i);
                        if (hp_2movable != NULL){
                            helpers::movable_3d * hp_3movable = dynamic_cast<helpers::movable_3d*>(*i);
                            hp_2movable->move_x(hp_2point->get_x()*(1-m_x_scaling));
                            hp_2movable->move_y(hp_2point->get_y()*(1-m_y_scaling));

                            if ((hp_3movable != NULL) && (hp_3point != NULL)){
                                hp_3movable->move_z(hp_3point->get_z()*(1-m_z_scaling));
                            }
                        }

                        helpers::scalable_2d * hp_2scalable = dynamic_cast<helpers::scalable_2d*>(*i);                        
                        if (hp_2scalable != NULL){
                            helpers::scalable_3d * hp_3scalable = dynamic_cast<helpers::scalable_3d*>(*i);
                            hp_2scalable->scale_x(m_x_scaling, *hp_2point);
                            hp_2scalable->scale_y(m_y_scaling, *hp_2point);
                            
                            if ((hp_3scalable != NULL) && (hp_3point != NULL)){
                                hp_3scalable->scale_z(m_z_scaling, *hp_3point);
                            }
                        }
                    }
                }
            }

            // scaling done
            return 0;
        }

        double scaling_adaptor::set_y_scaling(const double factor){ 
            double oldval = m_y_scaling; 
            m_y_scaling = factor; 
            return oldval; 
        }    
        
        double scaling_adaptor::set_x_scaling(const double factor){ 
            double oldval = m_x_scaling; 
            m_x_scaling = factor; 
            return oldval; 
        }    
        
        double scaling_adaptor::set_z_scaling(const double factor){ 
            double oldval = m_z_scaling; 
            m_z_scaling = factor; 
            return oldval; 
        }    
        
        bool scaling_adaptor::set_auto(const bool autoconf){ 
            bool oldval = m_autoconf; 
            m_autoconf = autoconf; 
            return oldval; 
        }
        
        bool scaling_adaptor::set_scale_accel(const bool scale){ 
            bool oldval = m_sc_accel; 
            m_sc_accel = scale; 
            return oldval; 
        }
        
        dimmension_t scaling_adaptor::set_x_length(const dimmension_t length){ 
            dimmension_t oldval = m_x_axis_length; 
            m_x_axis_length = length; 
            return oldval; 
        }

        dimmension_t scaling_adaptor::set_y_length(const dimmension_t length){ 
            dimmension_t oldval = m_y_axis_length; 
            m_y_axis_length = length; 
            return oldval; 
        }

        // commented out because someone ignores every email I send him...
        //dimmension_t scaling_adaptor::set_z_length(const dimmension_t length){ 
        //    dimmension_t oldval = m_z_axis_length; 
        //    m_z_axis_length = length; 
        //    return oldval; 
        //}

        void scaling_adaptor::purge(){ 
            bm_stack_clear(m_processed_frames); 
        }

        
    } // ns adaptors
} // ns libkerat

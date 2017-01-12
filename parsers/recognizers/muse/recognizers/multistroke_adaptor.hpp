/**
 * \file      multistroke_adaptor.hpp
 * \brief     Provides adaptor for multistroke recognizers
 * \author    Martin Luptak <374178@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2013-03-07 10:30 UTC+1
 * \copyright BSD
 */

#ifndef MULTISTROKE_ADAPTOR_HPP
#define	MULTISTROKE_ADAPTOR_HPP

#include <kerat/client.hpp>
#include <kerat/typedefs.hpp>
#include <kerat/adaptor.hpp>
#include <kerat/bundle.hpp>
#include <kerat/tuio_message_pointer.hpp>
#include <kerat/message_helpers.hpp>

#include <muse/recognizers/recognizers_utils.hpp>
#include <muse/recognizers/recognizers_auxiliary.hpp>

#include <dtuio/dtuio.hpp>

#include <map>
#include <vector>
#include <algorithm>
#include <stdint.h>


namespace libreco {
    namespace adaptors {
        using std::map;
        using std::vector;

        using libkerat::user_id_t;
        using libkerat::session_id_t;
        using libkerat::timetag_t;
        using libkerat::helpers::point_2d;
        using dtuio::gesture::gesture_identification;
        using dtuio::sensor::sensor_properties;
        using libreco::rutils::point_time;
        
        typedef uint32_t area_id;
        
        //! \brief Template class which provides adaptor for unistroke gesture recognizers
        template<class MRECOGNIZER>
        class multistroke_adaptor : public libkerat::adaptor {
        public:
            
            //! \brief holds points with time stamp that belongs to the same contact (stroke)
            typedef std::map<libkerat::session_id_t, std::vector<libreco::rutils::point_time> > strokes_map;
            
            //! \brief holds strokes that belongs to the same component
            typedef std::map<area_id, strokes_map> components_map;
            
            //! \brief holds all components of all users
            typedef std::map<libkerat::user_id_t, components_map> multistrokes_map;
            
            /**
             * \brief creates new multistroke_adaptor instance with given parameters
             * 
             * \param m_reco            multistroke recognizer
             * \param timeout_sec       seconds to wait for next stroke (if exceeded recognition process is launched)
             * \param timeout_frac      fractions of second to wait for next stroke (if exceeded recognition process is launched)
             * \param radius            components radius
             * \param uuid_arg          universally unique ID of the sensor
             */
            multistroke_adaptor(const MRECOGNIZER & m_reco, const dtuio::helpers::uuid & uuid_arg,
                                uint32_t timeout_sec = 2, uint32_t timeout_frac = 0, uint32_t radius = 0)
                : multistroke_recognizer(m_reco), sensor_uuid(uuid_arg), radius_area(radius), start_frame_id(0) {
                
                timeout.sec = timeout_sec;
                timeout.frac = timeout_frac;
            }
            
            /**
             * \brief Holds received frames from client and messages with recognition results
             * 
             * Holds all received frames from client. Adaptor does not perform any modifications of messages in the received frame.
             * Holds also recognition messages created by adaptor, after recognition of unknown gestures is done.
             * Messages with recognition results contains scores for recognized templates and scores are sorted in descending way (from highest to lowest).
             * Results messages are created only in case that timeout of some gesture is reached.
             * 
             * \see dtuio::gesture::gesture_identification
             * 
             * \return  stack of processed frames
             */
            libkerat::bundle_stack get_stack() const {
                return m_processed_frames;
            }
            
            //! \brief Clears the stack of processed messages
            void purge();
            
            /**
             * \brief Loads frames from given client and runs process_bundle method for every frame
             * 
             * This method also runs recognition process if timeout for some of multistroke gestures was reached.
             * 
             * \param cl    client
             */
            void notify(const libkerat::client * cl);
            
            /**
             * \brief Process messages contained in to_process frame 
             * 
             * Takes points from TUIO 2.0 pointer messages and sorts them according to user id, area id and session id.
             * In addition for each session id points are sorted according to arrival time. It means that 
             * this adaptor is able to work with UDP protocol and can restore original order of points.
             * Sorting according to user id, area id and session id means that unlimited number of multistroke gestures can be
             * done at the same time by unlimited number of unique users on the same touch surface.
             * Each user can have multiple unique components, which are defined by radius area in constructor. 
             * 
             * \param to_process    received frame
             * \param output_frame  contains all processed (unmodified) messages from to_process frame
             * \return              0 if everything was OK, negative number if error has occurred
             */
            int process_bundle(const libkerat::bundle_handle & to_process, libkerat::bundle_handle & output_frame);
            

        private:
            MRECOGNIZER multistroke_recognizer;
            libkerat::timetag_t timeout;
            uint32_t radius_area;
            dtuio::helpers::uuid sensor_uuid;
            libkerat::frame_id_t start_frame_id;            
            libkerat::bundle_stack m_processed_frames;
            multistrokes_map multistrokes;
            libkerat::message::alive last_alive;
                        
            //! \brief inserts new point to correct user, correct component and correct stroke in correct time order
            void assign_correctly(libkerat::user_id_t, libkerat::session_id_t, const libreco::rutils::point_time & new_point);
            
            //! \brief computes distance between new point and last point of given component
            float point_to_component_dist(const libreco::rutils::point_time & new_point, const libreco::rutils::point_time & last_point);
            
            //! \brief checks whether timeout was reached or not
            inline bool timeout_reached(const timetag_t & current_time, const timetag_t & last_point_time);
            
            //! \brief executes recognition of multistroke gestures
            void launch_recognition(std::vector<dtuio::gesture::gesture_identification *> & resutls_messages);
        };
        
        //! \brief converts vector of point_time structures to vector of point_2d instances
        static vector<point_2d> convert_to_vector_point2d(const std::pair<session_id_t, vector<point_time> > & original) {
            vector<point_2d> tmp_stroke;
            std::transform(original.second.begin(), original.second.end(), std::back_inserter(tmp_stroke), point_time::get_point2d);
            return tmp_stroke;
        }
        
                
        template<class MRECOGNIZER>
        void multistroke_adaptor<MRECOGNIZER>::purge() {
            libkerat::internals::bundle_manipulator::bm_stack_clear(m_processed_frames);
        }

        template<class MRECOGNIZER>
        void multistroke_adaptor<MRECOGNIZER>::notify(const libkerat::client * cl) {
            purge();
            
            //vector which holds pointers to messages with recognition results
            vector<gesture_identification *> results_messages;
            results_messages.clear();
            
            //launch recognition
            launch_recognition(results_messages);
            
            if(!results_messages.empty()) {
                //create new bundle with tuio frame, alive, and dtuio gesture_identification and sensor_properties messages
                libkerat::bundle_handle * results_frame = new bundle_handle;
                libkerat::message::frame * my_frame = new libkerat::message::frame(++start_frame_id);
                sensor_properties sensor_prop(sensor_uuid.get_uuid(), dtuio::sensor::sensor_properties::COORDINATE_INTACT,
                                              dtuio::sensor::sensor_properties::PURPOSE_TAGGER);
                
                bm_handle_insert(*results_frame, bm_handle_begin(*results_frame), my_frame);
                bm_handle_insert(*results_frame, bm_handle_end(*results_frame), sensor_prop.clone());
                bm_handle_insert(*results_frame, bm_handle_end(*results_frame), last_alive.clone());
                
                for(vector<gesture_identification *>::const_iterator iter = results_messages.begin(); iter != results_messages.end(); iter++) {
                    bm_handle_insert(*results_frame, --bm_handle_end(*results_frame), *iter);
                }
                libkerat::internals::bundle_manipulator::bm_stack_append(m_processed_frames, results_frame);
            }
            
            libkerat::bundle_stack data = cl->get_stack();
            libkerat::bundle_stack temp_data = data;
            
            //store last alive message
            size_t last_bundle_index = temp_data.get_length();
            if(last_bundle_index > 0) {
                last_alive = *(temp_data.get_update(last_bundle_index - 1).get_alive());
            }
            
            while (data.get_length() > 0) {
                bundle_handle current_frame = data.get_update();
                bundle_handle * tmphx = new bundle_handle;
                process_bundle(current_frame, *tmphx);

                //drop empty bundles
                if (tmphx->begin() == tmphx->end()) {
                    delete tmphx;
                    continue;
                }
                libkerat::internals::bundle_manipulator::bm_stack_append(m_processed_frames, tmphx);
            }
            
            //breaks recognizers
            //if (m_processed_frames.get_length()) {
            libkerat::client::notify_listeners();
            //}
        }
        
        template<class MRECOGNIZER>
        inline bool multistroke_adaptor<MRECOGNIZER>::timeout_reached(const timetag_t & current_time, const timetag_t & last_point_time) {
            timetag_t diff = libkerat::timetag_diff_abs(current_time, last_point_time);
            return ((timeout < diff) || (timeout == diff));
        }
        
        template<class MRECOGNIZER>
        void multistroke_adaptor<MRECOGNIZER>::launch_recognition(vector<gesture_identification *> & results_messages) {
            libkerat::timetag_t now;
           
			multistrokes_map::iterator user_iter = multistrokes.begin();
			while(user_iter != multistrokes.end()) {
				components_map::iterator comp_iter = user_iter->second.begin();
            	while(comp_iter != user_iter->second.end()) {
                	//last stroke in current component
                    strokes_map::iterator strokes_iter = comp_iter->second.end();
					--strokes_iter;
                    
					//load current time
                    lo_timetag_now(&now);
                    if(timeout_reached(now, strokes_iter->second.back().arrival_time)) {
                    	vector<vector<point_2d> > tmp_strokes;
                        
                        std::transform(comp_iter->second.begin(), comp_iter->second.end(), std::back_inserter(tmp_strokes), convert_to_vector_point2d);
						//run recognition and create message
                        std::multimap<float, std::string, std::greater<float> > final_scores = multistroke_recognizer.recognize(tmp_strokes);
                        
                        libkerat::user_id_t u_id = user_iter->first;
                        libkerat::session_set s_id_set;
                        for(strokes_map::iterator iter = comp_iter->second.begin(); iter != comp_iter->second.end(); iter++) {
                            s_id_set.insert(iter->first);
                        }                        
                        results_messages.push_back(new gesture_identification(final_scores, u_id, s_id_set, libreco::rutils::recognizer_name<MRECOGNIZER>::NAME));
						user_iter->second.erase(comp_iter++);
                    } else {
                    	++comp_iter;
                    }
                }
				//if user has not got any components he is erased from map
                if(user_iter->second.empty()) { multistrokes.erase(user_iter++); }
				else { user_iter++; }
            }
        }
        
        template<class MRECOGNIZER>
        int multistroke_adaptor<MRECOGNIZER>::process_bundle(const libkerat::bundle_handle & to_process, libkerat::bundle_handle & output_frame) {
           	
			typedef bundle_handle::const_iterator message_iterator;
            typedef libkerat::message::pointer message_pointer;

            //frame message holds time for given bundle
            libkerat::timetag_t curr_timestamp = to_process.get_frame()->get_timestamp();

            //iterate through messages in bundle
            for (message_iterator msg_iter = to_process.begin(); msg_iter != to_process.end(); msg_iter++) {

                //dynamic check whether this message is pointer message
                message_pointer * ptr_test = dynamic_cast<message_pointer *> (*msg_iter);
                if (ptr_test != NULL) {

                    //create new 2D point based on information received in pointer message
                    point_time curr_point(curr_timestamp, libkerat::helpers::point_2d(ptr_test->get_x(), ptr_test->get_y()));
                    user_id_t curr_uid = ptr_test->get_user_id();
                    session_id_t curr_sid = ptr_test->get_session_id();

                    //insert this point at the right place based on user_id, session_id and its position
                    assign_correctly(curr_uid, curr_sid, curr_point);
                }

                //insert message to processed messages bundle
                bm_handle_insert(output_frame, bm_handle_end(output_frame), (*msg_iter)->clone());
            }
            
            return 0;
        }

        template<class MRECOGNIZER>
        void multistroke_adaptor<MRECOGNIZER>::assign_correctly(libkerat::user_id_t uid, libkerat::session_id_t sid, const libreco::rutils::point_time & new_point) {
            typedef std::pair<libkerat::session_id_t, std::vector<libreco::rutils::point_time> > strokes_map_pair;

            //create new vector and insert first point
            vector<point_time> tmp_vect;
            tmp_vect.push_back(new_point);
            strokes_map tmp_stroke;
            tmp_stroke.insert(strokes_map_pair(sid, tmp_vect));

            multistrokes_map::iterator user_iter;
            user_iter = multistrokes.find(uid);

            /*user_id was not found in the map*/

            //new user will be added
            //first component with id 0 will be created for this user
            //first stroke with first point will be inserted to this component
            if (user_iter == multistrokes.end()) {
                //create first component of strokes (first component id is 0)
                components_map tmp_component;
                tmp_component.insert(std::pair<area_id, strokes_map > (0, tmp_stroke));

                //finally insert new user to multistrokes map which holds all strokes of all users
                multistrokes.insert(std::pair<user_id_t, components_map > (uid, tmp_component));
                return;
            }

            /*if function reaches this point, user_id already exists*/

            strokes_map::iterator session_iter;

            //if this adaptor was created with radius area 0
            //it means it does not support more than one component per user
            //it means that simple insertion can be performed
            if (radius_area == 0) {
                //there must be exactly one component at this point (because radius_area is zero) so no iteration is needed
                components_map::iterator comp_iter = user_iter->second.begin();
                session_iter = comp_iter->second.find(sid);

                //if stroke with given session id was found, point is inserted at the right place
                //else new stroke is created
                if (session_iter != comp_iter->second.end()) {
                    libreco::rauxiliary::insert_in_order(session_iter->second, new_point);
                } else {
                    comp_iter->second.insert(std::pair<session_id_t, vector<point_time> >(sid, tmp_vect));
                }
                //no more actions need to be executed
                return;
            }
            
            
            /*if function reaches this point user_id was found and radius is greater than zero*/
            
            //iterate through every strokes component of given user
            for (components_map::iterator comp_iter = user_iter->second.begin(); comp_iter != user_iter->second.end(); comp_iter++) {
                //check whether given session_id is already present in any of the strokes components of given user
                session_iter = comp_iter->second.find(sid);

                //session_id found, insert point in the right place in vector
                if (session_iter != comp_iter->second.end()) {
                    libreco::rauxiliary::insert_in_order(session_iter->second, new_point);
                    //no more actions need to be done
                    return;
                }
            }
            
            /*if function reaches this point, user_id already exists but session_id was not found (it means new contact is present)*/

            //upper limit for minimal distance
            bool component_found = false;
            float min_distance = (float)(radius_area);
            components_map::iterator nearest_component;
            
            //iterate through every strokes component of given user
            for (components_map::iterator comp_iter = user_iter->second.begin(); comp_iter != user_iter->second.end(); comp_iter++) {
                //obtain iterator to last stroke of given component
                //last stroke must be the last item in the map
                session_iter = comp_iter->second.end();
                session_iter--;
                
                float distance = point_to_component_dist(new_point, session_iter->second.back());
                //finds the nearest component of new point
                //if two components have got the same distance from the point, newer one is selected
                if(distance <= min_distance) {
                    component_found = true;
                    min_distance = distance;
                    nearest_component = comp_iter;
                }
            }
            
            //insert new stroke to the nearest component or create new component
            if(component_found) {
                nearest_component->second.insert(std::pair<session_id_t, vector<point_time> >(sid, tmp_vect));
            } else {
                components_map::iterator comp_iter = user_iter->second.end();
                area_id a_id = (--comp_iter)->first;
                a_id++;
                
                user_iter->second.insert(std::pair<area_id, strokes_map > (a_id, tmp_stroke));
            }
            
            return;
        }

        template<class MRECOGNIZER>
        float multistroke_adaptor<MRECOGNIZER>::point_to_component_dist(const libreco::rutils::point_time & new_point, const libreco::rutils::point_time & last_point) {
            const libkerat::helpers::point_2d & tmp_new_point = new_point.point;
            const libkerat::helpers::point_2d & tmp_last_point = last_point.point;
            return libreco::rauxiliary::distance_between_points(tmp_new_point, tmp_last_point);
        }


    } //ns adaptors
} //ns libreco

#endif	/* MULTISTROKE_ADAPTOR_HPP */

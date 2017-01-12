/**
 * \file      unistroke_adaptor.hpp
 * \brief     Provides adaptor for uni-stroke recognizers
 * \author    Martin Luptak <374178@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2013-03-05 09:30 UTC+1
 * \copyright BSD
 */

#ifndef UNISTROKE_ADAPTOR_HPP
#define	UNISTROKE_ADAPTOR_HPP

#include <kerat/kerat.hpp>

#include <dtuio/gesture_identification.hpp>

#include <muse/recognizers/unistroke_adaptor.hpp>
#include <muse/recognizers/recognizers_utils.hpp>
#include <muse/recognizers/recognizers_auxiliary.hpp>

#include <map>
#include <vector>

namespace libreco {
    namespace adaptors {
        using libkerat::bundle_handle;
        using libkerat::session_set;
                        
        using libreco::rutils::point_time;
        using dtuio::gesture::gesture_identification;
        using std::vector;
        
        //! \brief Template class which provides adaptor for unistroke gesture recognizers
        template<class RECOGNIZER>
        class unistroke_adaptor : public libkerat::adaptor {
        private:
            RECOGNIZER unistroke_recognizer;
            libkerat::bundle_stack m_processed_frames;
            std::map<libreco::rutils::stroke_identity , std::vector<libreco::rutils::point_time> > unistrokes_map;

        public:
            /**
             * \brief Creates new adaptor for given unistroke recognizer
             * 
             * Creates new adaptor for given unistroke recognizer passed to the constructor as argument.
             * Main responsibility is preprocessing of input points obtained from client.
             * After preprocessing is done, recognition process is executed using given unistroke recognizer. 
             * 
             * \param u_reco    unistroke recognizer
             */
            unistroke_adaptor(const RECOGNIZER & u_reco) : unistroke_recognizer(u_reco) { ; }
            
            /**
             * \brief Holds received frames from client and messages with recognition results
             * 
             * Holds all received frames from client. Adaptor does not perform any modifications of messages in the received frame.
             * It only adds dtuio::gesture::gesture_identification messages with scores of unknown gesture
             * for each template in given unistroke recognizer. Scores are stored in multimap in descending way (from the highest to the lowest).
             * Message with recognition results is not present in every frame, it is inserted to the frame only after the recognition process is done.
             * 
             * \see dtuio::gesture::gesture_identification
             * 
             * \return  stack of processed frames
             */
            libkerat::bundle_stack get_stack() const { return m_processed_frames; }
            
            
            //! \brief Clears the stack of processed messages
            void purge();
            
            /**
             * \brief Loads frames from given client and runs process_bundle method for every frame
             * 
             * \param cl    client
             */
            void notify(const libkerat::client * cl);
            
            /**
             * \brief Takes input points from received frame and sorts these points according to session_id
             * 
             * Takes points from TUIO 2.0 pointer messages and stores them in map according to session_id.
             * In addition for each session_id points are sorted according to arrival time. It means that 
             * this adaptor is able to work with UDP protocol and can restore original order of points.
             * Session_id must be unique so this ensures that unlimited number of unistroke gestures can be
             * done at the same time on the same touch-screen. After contact is lost (gesture is done, so session_id is no more present in alive message)
             * recognition process is executed and message with results is added to the output frame.
             * 
             * \see dtuio::gesture::gesture_identification
             * 
             * \param to_process    received frame handle
             * \param output_frame  contains all messages from to_process frame and holds dtuio::gesture::gesture_identification message if recognition process was performed
             * \return              0 if everything was OK, negative number if error has occurred
             */
            int process_bundle(const libkerat::bundle_handle & to_process, libkerat::bundle_handle & output_frame);
            
        };

        
        template<class RECOGNIZER>
        void unistroke_adaptor<RECOGNIZER>::purge() {
            libkerat::internals::bundle_manipulator::bm_stack_clear(m_processed_frames);
        }
        
        template<class RECOGNIZER>
        void unistroke_adaptor<RECOGNIZER>::notify(const libkerat::client * cl) {
            purge();
            
            libkerat::bundle_stack data = cl->get_stack();
            
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

        template<class RECOGNIZER>
        int unistroke_adaptor<RECOGNIZER>::process_bundle(const bundle_handle & to_process, bundle_handle & output_frame) {
            typedef bundle_handle::const_iterator message_iterator;
            typedef libkerat::message::pointer message_pointer;
            typedef std::pair<libreco::rutils::stroke_identity, vector<point_time> > unistrokes_map_pair;

            std::map<libreco::rutils::stroke_identity, vector<point_time> >::iterator unistrokes_map_iter;

            //time in the frame message is the same for every other message in given bundle
            libkerat::timetag_t curr_timestamp = to_process.get_frame()->get_timestamp();

            //go through every message in the bundle
            for (message_iterator msg_iter = to_process.begin(); msg_iter != to_process.end(); msg_iter++) {

                //check whether this message is pointer message
                message_pointer * ptr_test = dynamic_cast<message_pointer *> (*msg_iter);
                if (ptr_test != NULL) {
                    //insert received session id to map
                    libreco::rutils::stroke_identity stroke_id(ptr_test->get_user_id(), ptr_test->get_session_id());
                    unistrokes_map_iter = unistrokes_map.insert(unistrokes_map.begin(), unistrokes_map_pair(stroke_id, vector<point_time> ()));
                    //create new 2D point based on information stored in pointer message
                    point_time curr_point(curr_timestamp, libkerat::helpers::point_2d(ptr_test->get_x(), ptr_test->get_y()));

                    //insert new point in correct order
                    libreco::rauxiliary::insert_in_order(unistrokes_map_iter->second, curr_point);
                }
                
                //each message is inserted to output_frame
                bm_handle_insert(output_frame, bm_handle_end(output_frame), (*msg_iter)->clone());
            }
            
            //from now, recognition process is launched if any contact previously present is no more in alive message
            session_set removed_ids;
                        
            //static because needs to survive between function calls
            static libkerat::session_set previous_alive_ids;
            
            //extract id-s of removed contacts
            removed_ids = libkerat::extract_removed_ids(previous_alive_ids, to_process.get_alive()->get_alives());
            previous_alive_ids = to_process.get_alive()->get_alives();
                        
            //go through all contacts and launch recognition if contact was removed
            unistrokes_map_iter = unistrokes_map.begin();
            while ((unistrokes_map_iter != unistrokes_map.end()) && (!removed_ids.empty())) {
                session_set::const_iterator removed_iter = removed_ids.find(unistrokes_map_iter->first.s_id);
                
                if (removed_iter != removed_ids.end()) {
                    vector<libkerat::helpers::point_2d> tmp_stroke;
                    
                    std::transform(unistrokes_map_iter->second.begin(), unistrokes_map_iter->second.end(), std::back_inserter(tmp_stroke), point_time::get_point2d);
                    std::multimap<float, std::string, std::greater<float> > final_scores = unistroke_recognizer.recognize(tmp_stroke);
                        
                    //create message containing recognition results
                    libkerat::user_id_t user = unistrokes_map_iter->first.u_id;
                    libkerat::session_set session;
                    session.insert(unistrokes_map_iter->first.s_id);
                    gesture_identification * result_msg = new gesture_identification(final_scores, user, session, libreco::rutils::recognizer_name<RECOGNIZER>::NAME);
                    
                    //recognition message is inserted before the alive message
                    bm_handle_insert(output_frame, --bm_handle_end(output_frame), result_msg);
                
                    //remove processed stroke
                    removed_ids.erase(removed_iter);
                    unistrokes_map.erase(unistrokes_map_iter++);                
                } else {
                    ++unistrokes_map_iter;
                }
            }
            return 0;
        }
        
    } //ns adaptors
} //ns libreco

#endif	/* UNISTROKE_ADAPTOR_HPP */


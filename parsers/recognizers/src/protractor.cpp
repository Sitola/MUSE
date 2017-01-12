/**
 * \file      protractor.cpp
 * \brief     Implements functionality for protractor (unistroke) recognizer
 * \author    Martin Luptak <374178@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-11-14 13:10 UTC+1
 * \copyright BSD
 */

#include <muse/recognizers/protractor.hpp>
#include <muse/recognizers/recognizers_auxiliary.hpp>
#include <muse/recognizers/recognizers_utils.hpp>
#include <float.h>
#include <cmath>

#ifndef M_PI
    #define M_PI acos(-1);
#endif

#ifdef TEST_PERFORMANCE
    #include <muse/recognizers/io_utils.hpp>
    #include <lo/lo_lowlevel.h>
#endif

namespace libreco {
    namespace recognizers {

        using std::vector;
        using std::map;
        using libkerat::helpers::point_2d;
        using libreco::rutils::unistroke_gesture;
        
        //preprocessing of templates
        void protractor::process_templates() {
            //temporary vector of gestures which are processed also in reverse order of points
            vector<unistroke_gesture> revert_gestures;

            //goes through every template and transforms the template according to protractor attributes and template parameters
            for (vector<unistroke_gesture>::iterator iter = prot_templates.begin(); iter != prot_templates.end(); iter++) {

                //creates also gesture in reverse order if template attribute revert is set true
                if (iter->revert) {
                    vector<point_2d> revert_stroke;
                    revert_stroke.insert(revert_stroke.begin(), iter->points.rbegin(), iter->points.rend());

                    libreco::rauxiliary::resample(revert_stroke, number_of_points);
                    libreco::rauxiliary::translate_to(revert_stroke, origin);
                    vectorize(revert_stroke, iter->sensitive);

                    revert_gestures.push_back(unistroke_gesture(iter->name, iter->revert, iter->sensitive, revert_stroke));
                }
                
                libreco::rauxiliary::resample(iter->points, number_of_points);
                libreco::rauxiliary::translate_to(iter->points, origin);
                vectorize(iter->points, iter->sensitive);
            }

            //reverse templates are added to protractor templates
            prot_templates.reserve(prot_templates.size() + revert_gestures.size());
            prot_templates.insert(prot_templates.end(), revert_gestures.begin(), revert_gestures.end());
        }
        
        //create protractor instance with specified parameters
        protractor::protractor(const std::vector<libreco::rutils::unistroke_gesture> & tmpls, uint16_t num_of_pts, const libkerat::helpers::point_2d & orig)
        : number_of_points(num_of_pts), origin(orig) {

            prot_templates.reserve(tmpls.size());
            prot_templates = tmpls;
            process_templates();
        }
        
        //create protractor instance with default parameters
        protractor::protractor(const std::vector<libreco::rutils::unistroke_gesture> & tmpls)
        : number_of_points(16), origin(point_2d(0, 0)) {

            prot_templates.reserve(tmpls.size());
            prot_templates = tmpls;
            process_templates();
        }


        //Create a vector representation of the gesture
        void protractor::vectorize(vector<point_2d> & points, bool o_sensitive) {
            
            float angle = atan2(points[0].get_y(), points[0].get_x());
            float delta = 0.0;

            //decision based on fact whether gesture is orientation sensitive or invariant
            if (o_sensitive) {
                float base_orientation = (M_PI / 2.0) * std::floor((angle + (M_PI / 4.0)) / (M_PI / 2.0));
                delta = base_orientation - angle;
                
            } else {
                delta = -angle;
                
            }
            float cos_value = cos(delta);
            float sin_value = sin(delta);
            float sum = 0.0;

            //create vector representation of gesture
            for (vector<point_2d>::iterator iter = points.begin(); iter != points.end(); iter++) {
                float p_coord_x = iter->get_x();
                float p_coord_y = iter->get_y();
                iter->set_x((p_coord_x * cos_value) - (p_coord_y * sin_value));
                iter->set_y((p_coord_y * cos_value) + (p_coord_x * sin_value));
                sum += (iter->get_x() * iter->get_x()) + (iter->get_y() * iter->get_y());
            }
            
            //normalize created vector, so its length is 1
            float magnitude = sqrt(sum);
            for (vector<point_2d>::iterator iter = points.begin(); iter != points.end(); iter++) {
                iter->set_x(iter->get_x() / magnitude);
                iter->set_y(iter->get_y() / magnitude);
            }
        }

        //Match unknown gesture (points parameter) against the given template (pattern parameter)
        float protractor::optimal_cosine_distance(const vector<point_2d> & points, const vector<point_2d> & pattern) {
            float a = 0.0;
            float b = 0.0;

            for (unsigned int i = 0; i < points.size(); i++) {
                a += (pattern[i].get_x() * points[i].get_x()) + (pattern[i].get_y() * points[i].get_y());
                b += (pattern[i].get_x() * points[i].get_y()) - (pattern[i].get_y() * points[i].get_x());
            }
            
            float angle = atan(b / a);
            
            //alternative way (more intuitive) how to compute distance, but it is slower
            /*
            vector<point_2d> local_copy = pattern;
            double dot_product = 0.0;
            for (unsigned int i = 0; i < points.size(); i++) {
                float tmpx = local_copy[i].get_x();
                float tmpy = local_copy[i].get_y();
                local_copy[i].set_x(tmpx * cos(angle) - tmpy * sin(angle));
                local_copy[i].set_y(tmpx * sin(angle) + tmpy * cos(angle));
                dot_product += (points[i].get_x() * local_copy[i].get_x()) + (points[i].get_y() * local_copy[i].get_y());
            }
            */
            if(abs(angle) < M_PI / 4.0) {
                return acos(a * cos(angle) + b * sin(angle));
            } 
            
            return FLT_MAX;
        }
        
        //recognize if gesture is represented as vector of point_time structure instances
        libreco::recognizers::recognized_gestures protractor::recognize(const std::vector<libreco::rutils::point_time> & unknown_gesture) const {
            vector<point_2d> gesture_points;
            gesture_points.reserve(unknown_gesture.size());
            
            std::transform(unknown_gesture.begin(), unknown_gesture.end(), std::back_inserter(gesture_points), libreco::rutils::point_time::get_point2d);
            return recognize(gesture_points);
        }

        libreco::recognizers::recognized_gestures protractor::recognize(const std::vector<libkerat::helpers::point_2d> & unknown_gesture) const {

#ifdef TEST_PERFORMANCE
            libkerat::timetag_t recognize_start;
            lo_timetag_now(&recognize_start);
#endif
            
            //only local copy of gesture passed to the method as parameter
            vector<point_2d> gesture_points = unknown_gesture;
                        
            libreco::rauxiliary::resample(gesture_points, number_of_points);
            libreco::rauxiliary::translate_to(gesture_points, origin);
            
            //first: unknown gesture is treated as orientation invariant
            vector<point_2d> o_invar_gest = gesture_points;
            vectorize(o_invar_gest, false);

            //second: unknown gesture is treated as orientation sensitive
            vector<point_2d> o_sens_gest = gesture_points;
            vectorize(o_sens_gest, true);

            float score = 0.0;
            libreco::recognizers::recognized_gestures scores;

            for (vector<unistroke_gesture>::const_iterator iter = prot_templates.begin(); iter != prot_templates.end(); iter++) {
                if (iter->sensitive) {
                    score = optimal_cosine_distance(o_sens_gest, iter->points);
                } else {
                    score = optimal_cosine_distance(o_invar_gest, iter->points);
                }
                if(score < FLT_MAX) {
                    scores.insert(std::pair<float, std::string > (1.0 / score, iter->name));
                }
            }
#ifdef TEST_PERFORMANCE
            libkerat::timetag_t recognize_end;
            lo_timetag_now(&recognize_end);
            libreco::iotools::log_performance_test(recognize_start, recognize_end, "../parsers/recognizers/performanceTests/protractor.log",
                                                   "Protractor recognizer", scores);
#endif
            return scores;
        }

    } // ns recognizers
    
    namespace rutils {
        //! \brief protractor name identifier
        const char * recognizer_name<libreco::recognizers::protractor>::NAME = "protractor";
    }
} // ns libreco

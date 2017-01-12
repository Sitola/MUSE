/**
 * \file      dollar_recognizer.cpp
 * \brief     Implements functionality for dollar one (unistroke) recognizer
 * \author    Martin Luptak <374178@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-11-05 12:10 UTC+1
 * \copyright BSD
 */

#include <muse/recognizers/dollar_recognizer.hpp>
#include <muse/recognizers/recognizers_auxiliary.hpp>
#include <muse/recognizers/typedefs.hpp>

#include <math.h>
#include <algorithm>

#ifdef TEST_PERFORMANCE
    #include <muse/recognizers/io_utils.hpp>
    #include <lo/lo_lowlevel.h>
#endif

namespace libreco {
    namespace recognizers {

        using std::vector;
        using libreco::rutils::unistroke_gesture;
        using libreco::rauxiliary::degrees_to_radians;
        using libkerat::helpers::point_2d;
                
        //runs all methods needed for correct gesture preprocessing
        void dollar_recognizer::process_gesture(vector<point_2d> & gest) const {
            libreco::rauxiliary::resample(gest, number_of_points);
            float angle = libreco::rauxiliary::indicative_angle(gest);
            libreco::rauxiliary::rotate_by_angle(gest, -angle);
            scale_to_bounding_box(gest);
            libreco::rauxiliary::translate_to(gest, origin);
        }
        
        //process templates passed to the constructor as parameter
        void dollar_recognizer::process_templates() {
            vector<unistroke_gesture> revert_gestures;
            
            for (vector<unistroke_gesture>::iterator iter = dollar_templates.begin(); iter != dollar_templates.end(); iter++) {
                //creates also gesture in reverse order if template attribute revert is set true
                if(iter->revert) {
                    vector<point_2d> revert_stroke;
                    revert_stroke.reserve(iter->points.size());
                    revert_stroke.insert(revert_stroke.begin(), iter->points.rbegin(), iter->points.rend());
                    process_gesture(revert_stroke);
                    revert_gestures.push_back(unistroke_gesture(iter->name, iter->revert, iter->sensitive, revert_stroke));
                }
                process_gesture(iter->points);
            }
            
            //reverse templates are added to dollar one templates
            dollar_templates.reserve(dollar_templates.size() + revert_gestures.size());
            dollar_templates.insert(dollar_templates.end(), revert_gestures.begin(), revert_gestures.end());
        }
        
        
        //create dollar one instance with specified parameters
        dollar_recognizer::dollar_recognizer(const std::vector<libreco::rutils::unistroke_gesture> & tmpls, uint16_t num_of_pts,
                                             const libkerat::helpers::point_2d & orig, float rot_down, float rot_up, float rot_thresh, uint16_t scale_box)
        : number_of_points(num_of_pts), origin(orig), rotation_down_limit(degrees_to_radians(rot_down)),
          rotation_up_limit(degrees_to_radians(rot_up)), rotation_threshold(degrees_to_radians(rot_thresh)), scale_box_size(scale_box) {

            dollar_templates.reserve(tmpls.size());
            dollar_templates = tmpls;
            process_templates();
        }
        
        //create dollar one instance with default parameters
        dollar_recognizer::dollar_recognizer(const std::vector<libreco::rutils::unistroke_gesture> & tmpls)
        : number_of_points(64), origin(point_2d(0, 0)), rotation_down_limit(degrees_to_radians(-45.0)),
          rotation_up_limit(degrees_to_radians(45.0)), rotation_threshold(degrees_to_radians(2.0)),  scale_box_size(250) {

            dollar_templates.reserve(tmpls.size());
            dollar_templates = tmpls;
            process_templates();
        }
        
        //scales gesture to bounding box with side specified as constructor parameter
        void dollar_recognizer::scale_to_bounding_box(vector<point_2d> & points) const {
            libreco::rutils::rectangle min_bounding_box = libreco::rauxiliary::bounding_box(points);
            for (vector<point_2d>::iterator iter = points.begin(); iter != points.end(); iter++) {
                iter->set_x(iter->get_x() * (scale_box_size / (min_bounding_box.get_top_right().get_x() - min_bounding_box.get_bottom_left().get_x())));
                iter->set_y(iter->get_y() * (scale_box_size / (min_bounding_box.get_top_right().get_y() - min_bounding_box.get_bottom_left().get_y())));
            }
        }
        
        //transforms points from point_time representation to point_2d representation
        libreco::recognizers::recognized_gestures dollar_recognizer::recognize(const std::vector<libreco::rutils::point_time> & unknown_gesture) const {
            vector<point_2d> gesture_points;
            gesture_points.reserve(unknown_gesture.size());
            
            std::transform(unknown_gesture.begin(), unknown_gesture.end(), std::back_inserter(gesture_points), libreco::rutils::point_time::get_point2d);
            return recognize(gesture_points);
        }

        libreco::recognizers::recognized_gestures dollar_recognizer::recognize(const std::vector<libkerat::helpers::point_2d> & unknown_gesture) const {

#ifdef TEST_PERFORMANCE
            libkerat::timetag_t recognize_start;
            lo_timetag_now(&recognize_start);
#endif
            
            //local copy of gesture
            vector<point_2d> gesture_points = unknown_gesture;
            
            libreco::rauxiliary::resample(gesture_points, number_of_points);
            
            float angle = libreco::rauxiliary::indicative_angle(gesture_points);
            libreco::rauxiliary::rotate_by_angle(gesture_points, -angle);
                       
            scale_to_bounding_box(gesture_points);
            libreco::rauxiliary::translate_to(gesture_points, origin);
            
            float half_diagonal = 0.5 * sqrt((scale_box_size * scale_box_size) + (scale_box_size * scale_box_size));
            libreco::recognizers::recognized_gestures scores;
            float score = 0.0;

            for (vector<unistroke_gesture>::const_iterator iter = dollar_templates.begin(); iter != dollar_templates.end(); iter++) {
                score = libreco::rauxiliary::distance_at_best_angle(gesture_points, iter->points, rotation_down_limit, rotation_up_limit, rotation_threshold);
                score = 1.0 - (score / half_diagonal);
                scores.insert(std::pair<float, std::string > (score, iter->name));
            }
            
#ifdef TEST_PERFORMANCE
            libkerat::timetag_t recognize_end;
            lo_timetag_now(&recognize_end);
            libreco::iotools::log_performance_test(recognize_start, recognize_end, "../parsers/recognizers/performanceTests/dollarOne.log",
                                                   "Dollar 1 recognizer ($1)", scores);
#endif
            
            return scores;
        }
    } // ns recognizers
    
    namespace rutils {
        //! \brief dollar_recognizer name identifier
        const char * recognizer_name<libreco::recognizers::dollar_recognizer>::NAME = "dollar_recognizer ($1)";
    }
    
} // ns libreco
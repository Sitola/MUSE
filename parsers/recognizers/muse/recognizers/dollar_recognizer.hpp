/**
 * \file      dollar_recognizer.hpp
 * \brief     Provides the header for the dollar one (unistroke) recognizer
 * \author    Martin Luptak <374178@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-11-05 12:10 UTC+1
 * \copyright BSD
 */

#ifndef DOLLAR_RECOGNIZER_HPP_
#define DOLLAR_RECOGNIZER_HPP_


#include <kerat/message_helpers.hpp>
#include <muse/recognizers/recognizers_utils.hpp>
#include <muse/recognizers/typedefs.hpp>

#include <vector>
#include <map>

namespace libreco {
    namespace recognizers {
        
        /**
         * \brief Dollar one ($1) recognizer class for unistroke gestures recognition 
         */
        class dollar_recognizer {
        public:
            
            /**
             * \brief Creates new dollar one ($1) instance with given parameters
             * 
             * First $1 recognizer executes initial phase of preprocessing templates passed to the constructor.
             * Input templates are stored as vector of \ref libreco::rutils::unistroke_gesture structures.
             * This structure holds all information needed for correct gesture preprocessing.
             * Each template is resampled so it has got n-evenly spaced points (n is num_of_pts argument in constructor).
             * Then indicative angle between centroid and first point of template is computed and template is rotated so this angle is set to 0°.
             * Template is scaled uniformly to the bounding box (side length of bounding box is scale_box parameter of constructor).
             * Each template is translated so its new centroid is the origin (origin is orig argument in constructor).
             * In addition if template has got revert attribute set to true, it is processed also in reverse order of points,
             * this ensures that for example circle template made clockwise will be also stored in reverse order (counterclockwise).
             * 
             * \param tmpls         unprocessed templates, which will be processed according to template attributes and constructor parameters
             * \param num_of_pts    number of points in each gesture after resample method call
             * \param orig          gesture will be translated so this point is the new centroid of gesture
             * \param rot_down      angle which specifies down limit of rotation range used for searching distance at best angle
             * \param rot_up        angle which specifies up limit of rotation range used for searching distance at best angle
             * \param rot_thresh    if rotation change in recognition process is less than threshold recognition is done
             * \param scale_box     side length of bounding box where gesture will be scaled
             */
            dollar_recognizer(const std::vector<libreco::rutils::unistroke_gesture> & tmpls, uint16_t num_of_pts, const libkerat::helpers::point_2d & orig,
                    float rot_down, float rot_up, float rot_thresh, uint16_t scale_box);
            
            /**
             * \brief Creates new dollar one ($1) instance with default parameters
             * 
             * Performs exactly the same preprocessing steps as
             * \ref libreco::recognizers::dollar_recognizer::dollar_recognizer(const std::vector<libreco::rutils::unistroke_gesture> & tmpls, uint16_t num_of_pts, const libkerat::helpers::point_2d & orig, float rot_down, float rot_up, float rot_thresh, uint16_t scale_box).
             * These parameters are set to default values: number of points = 64, origin = (0, 0),
             * lower bound of rotation range = -45°, upper bound of rotation range = 45°, rotation threshold = 2°, bounding box side length = 250.  
             * 
             * \param tmpls     unprocessed templates, which will be processed according to template attributes and default parameters of constructor
             */
            dollar_recognizer(const std::vector<libreco::rutils::unistroke_gesture> & tmpls);
            
            /**
             * \brief Method for unistroke gesture recognition
             * 
             * Unknown unistroke gesture is represented as vector of libkerat::helpers::point_2d instances.
             * First, unknown gesture is processed in exactly the same way as all templates were.
             * Then it is compared to every template loaded and preprocessed in constructor.
             * By comparing template to unknown gesture, score for each template is computed
             * and multimap of scores and names for each template is returned as result.
             * 
             * \see libkerat::helpers::point_2d
             * \see libreco::recognizers::recognized_gestures
             * \see libreco::recognizers::dollar_recognizer::dollar_recognizer(const std::vector<libreco::rutils::unistroke_gesture> & tmpls, uint16_t num_of_pts, const libkerat::helpers::point_2d & orig, float rot_down, float rot_up, float rot_thresh, uint16_t scale_box)
             * 
             * \param unknown_gesture   vector of points which represents unknown gesture made by user
             * \return                  multimap of scores for each template loaded in constructor (scores are sorted in descending order)
             */
            libreco::recognizers::recognized_gestures recognize(const std::vector<libkerat::helpers::point_2d> & unknown_gesture) const;
            
            /**
             * \brief Method for unistroke gesture recognition
             * 
             * Unknown unistroke gesture is represented as vector of \ref libreco::rutils::point_time instances.
             * This method was created only as a support for method:
             * \ref libreco::recognizers::dollar_recognizer::recognize "libreco::recognizers::dollar_recognizer::recognize(const std::vector<libkerat::helpers::point_2d> & unknown_gesture) const".
             * It only transforms vector of \ref libreco::rutils::point_time instances to vector of libkerat::helpers::point_2d instances and runs recognition process.
             * Time attribute is removed and it is not used in any way in recognition process.
             *
             * \see libkerat::helpers::point_2d
             * \see libreco::recognizers::recognized_gestures
             * 
             * \param unknown_gesture   vector of points which represents unknown gesture made by user
             * \return                  multimap of scores and names for each template loaded in constructor (scores are sorted in descending order)
             */
            libreco::recognizers::recognized_gestures recognize(const std::vector<libreco::rutils::point_time> & unknown_gesture) const;

        private:
            std::vector<libreco::rutils::unistroke_gesture> dollar_templates;
            uint16_t number_of_points;
            libkerat::helpers::point_2d origin;
            float rotation_down_limit;
            float rotation_up_limit;
            float rotation_threshold;
            uint16_t scale_box_size;
            
            //! \brief Uniformly scales gesture to bounding box so gestures of all sizes are treated the same
            void scale_to_bounding_box(std::vector<libkerat::helpers::point_2d> & points) const;
            
            //! \brief Used for preprocessing input templates passed to the recognizer as constructor argument
            void process_templates();
            
            //! \brief Steps for gesture resampling, rotating, scaling and translating to the origin
            void process_gesture(std::vector<libkerat::helpers::point_2d> & gest) const;
        };

    } // ns recognizers
    
    namespace rutils {
    
         //! \brief dollar_recognizer name
        template <>
        struct recognizer_name<libreco::recognizers::dollar_recognizer> {
            static const char * NAME;
        };
    }
    
} // ns libreco

#endif /* DOLLAR_RECOGNIZER_HPP_ */

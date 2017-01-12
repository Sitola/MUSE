/**
 * \file      protractor.hpp
 * \brief     Provides the header for the protractor (unistroke) recognizer
 * \author    Martin Luptak <374178@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-11-14 13:10 UTC+1
 * \copyright BSD
 */

#ifndef PROTRACTOR_HPP_
#define	PROTRACTOR_HPP_

#include <kerat/message_helpers.hpp>
#include <muse/recognizers/recognizers_utils.hpp>
#include <muse/recognizers/typedefs.hpp>

#include <vector>
#include <map>

namespace libreco {
    namespace recognizers {

        /**
         * \brief Protractor recognizer class for unistroke gestures recognition 
         */
        class protractor {
        public:
            
            /**
             * \brief Creates new protractor instance with given parameters
             * 
             * First protractor executes initial phase of preprocessing templates passed to the constructor.
             * Templates are stored as vector of \ref libreco::rutils::unistroke_gesture structures.
             * This structure holds all information needed for correct gesture preprocessing.
             * Each template is resampled so it has got n-evenly spaced points (n is specified as num_of_pts parameter of constructor).
             * Then template is translated so its new centroid is the origin (origin is specified as orig parameter of constructor).
             * Based on sensitivity attribute of template gesture is treated as orientation sensitive (true) or invariant (false).
             * In the last step template is transformed and unit vector representation of gesture is created based on sensitivity attribute.
             * In addition if template has got revert attribute set to true, it is processed also in reverse order of points,
             * this ensures that for example circle template made clockwise will be also stored in reverse order (counterclockwise). 
             * 
             * \param tmpls         unprocessed templates, they will be processed according to template attributes and constructor parameters
             * \param num_of_pts    number of points in each gesture after resample method call
             * \param orig          gesture will be translated so this point is the new centroid of gesture
             */
            protractor(const std::vector<libreco::rutils::unistroke_gesture> & tmpls, uint16_t num_of_pts, const libkerat::helpers::point_2d & orig);
            
            /**
             * \brief Creates new protractor instance with default parameters
             * 
             * Performs exactly the same preprocessing steps as
             * \ref libreco::recognizers::protractor::protractor(const std::vector<libreco::rutils::unistroke_gesture> & tmpls, uint16_t num_of_pts, const libkerat::helpers::point_2d & orig).
             * These parameters are set to default values, origin is set to (0, 0) and number of points is set to 16.
             * 
             * \param tmpls     unprocessed templates, they will be processed according to template attributes and default parameters origin = (0, 0), number_of_points = 16
             */
            protractor(const std::vector<libreco::rutils::unistroke_gesture> & tmpls);

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
             * \see libreco::recognizers::protractor::protractor(const std::vector<libreco::rutils::unistroke_gesture> & tmpls, uint16_t num_of_pts, const libkerat::helpers::point_2d & orig).
             * 
             * \param unknown_gesture   vector of points which represents unknown gesture made by user
             * \return                  multimap of scores and names for each template loaded in protractor`s constructor (scores are sorted in descending order)
             */
            libreco::recognizers::recognized_gestures recognize(const std::vector<libkerat::helpers::point_2d> & unknown_gesture) const;
            
            /**
             * \brief Method for unistroke gesture recognition
             * 
             * Unknown unistroke gesture is represented as vector of \ref libreco::rutils::point_time instances.
             * This method was created only as a support for method:
             * \ref libreco::recognizers::protractor::recognize "libreco::recognizers::protractor::recognize(const std::vector<libkerat::helpers::point_2d> & unknown_gesture) const".
             * It only transforms vector of \ref libreco::rutils::point_time instances to vector of libkerat::helpers::point_2d instances and runs recognition process.
             * Time attribute is removed and it is not used in any way in recognition process.
             *
             * \see libkerat::helpers::point_2d
             * \see libreco::recognizers::recognized_gestures
             * 
             * \param unknown_gesture   vector of points which represents unknown gesture made by user
             * \return                  multimap of scores and names for each template loaded in protractor`s constructor (scores are sorted in descending order)
             */
            libreco::recognizers::recognized_gestures recognize(const std::vector<libreco::rutils::point_time> & unknown_gesture) const;
            
        private:
            
            std::vector<libreco::rutils::unistroke_gesture> prot_templates;
            uint16_t number_of_points;
            libkerat::helpers::point_2d origin;

            //! \brief Computes optimal cosine distance between unknown gesture and given pattern
            static float optimal_cosine_distance(const std::vector<libkerat::helpers::point_2d> & points, const std::vector<libkerat::helpers::point_2d> & pattern);
            
            //! \brief Rotates the gesture according to its sensitivity and creates normalized vector representation of given gesture
            static void vectorize(std::vector<libkerat::helpers::point_2d> & points, bool o_sensitive);
            
            //! \brief Used for processing input templates passed to the recognizer as constructor argument
            void process_templates();
            
            
        }; //cls protractor
        
    } // ns recognizers
    
    namespace rutils {
        //! \brief protractor recognizer name
        template <>
        struct recognizer_name<libreco::recognizers::protractor> {
            static const char * NAME;
        };
    }
    
} // ns libreco

#endif	/* PROTRACTOR_HPP_ */


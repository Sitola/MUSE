/**
 * \file      dollar_n.hpp
 * \brief     Provides the header for the dollar N ($N) (multistroke) recognizer
 * \author    Martin Luptak <374178@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-11-19 14:37 UTC+1
 * \copyright BSD
 */

#ifndef DOLLAR_N_HPP_
#define	DOLLAR_N_HPP_

#include <kerat/message_helpers.hpp>
#include <muse/recognizers/recognizers_auxiliary.hpp>
#include <muse/recognizers/typedefs.hpp>

#include <vector>
#include <map>

namespace libreco {
    namespace recognizers {
        
        using std::vector;
        using libkerat::helpers::point_2d;
                        
        //! \brief Dollar N ($N) recognizer class for multistroke gesture recognition
        class dollar_n {
        public:
            
            /**
             * \brief Holds multistroke templates loaded from XML file
             *
             * In this map, key value is \ref libreco::rutils::gesture_identity structure which holds information needed for correct gesture preprocessing. 
             * Mapped value is vector of partial strokes for given multistroke gesture.
             */
            typedef std::map<libreco::rutils::gesture_identity, vector<vector<point_2d> > > libreco_generic_multistroke_map;
                                    
            /**
             * \brief Creates new Dollar N ($N) recognizer instance with default parameters
             * 
             * Performs exactly the same preprocessing steps as
             * \ref libreco::recognizers::dollar_n::dollar_n(const libreco_generic_multistroke_map & tmpls, uint16_t num_of_pts, const point_2d orig, float rot_down, float rot_up, float rot_thresh, uint16_t scale_box, float scale_thresh, uint16_t start_index, float start_thresh, bool eq_strokes).
             * These parameters are set to default values:
             * number of points = 96, origin = (0, 0), lower bound for rotation range = -45°, upper bound for rotation range = 45°, rotation threshold = 2°,
             * scale box side length = 250, scaling threshold = 0.30, start unit vector index = 12, start unit vector threshold = 30°, equal strokes number = true
             * 
             * \param tmpls     unprocessed templates, they will be processed according to template attributes and constructor parameters
             */
            dollar_n(const libreco_generic_multistroke_map & tmpls);
            
            /**
             * \brief Creates new Dollar N ($N) recognizer instance with given parameters
             * 
             * First $N recognizer executes initial phase of preprocessing templates passed to the constructor.
             * Input templates are stored as map in the following format: \ref libreco::recognizers::dollar_n::libreco_generic_multistroke_map.
             * Each multistroke template is transformed to multiple unistroke templates. All possible orders and directions
             * of partial strokes are created for each multistroke template. Then based on these permutations every possible unistroke is created as 
             * combination of partial strokes.
             * 
             * Each created unistroke template is processed according to following steps.
             * It is resampled so it has got n-evenly spaced points (n is num_of_pts argument in constructor).
             * Then indicative angle between centroid and first point of template is computed and template is rotated so this angle is set to 0°.
             * It is scaled to the bounding box (side length of bounding box is scale_box parameter of constructor).
             * Templates are scaled non-uniformly based on scaling threshold (specified in constructor as scale_thresh argument).
             * Next if template has got sensitivity attribute set to true, it is rotated back to its initial orientation.
             * Each template is translated so its new centroid is the origin (origin is orig argument in constructor).
             * Finally start unit vector is computed for each unistroke. Start unit vector helps to save time in recognition process.
             * Unknown gesture is compared only to those templates which start unit vector is pointing approximately the same direction as
             * start unit vector of unknown gesture.
             * 
             * \param tmpls         unprocessed templates, which will be processed according to template attributes and constructor parameters
             * \param num_of_pts    number of points in each gesture after resample method call
             * \param orig          gesture will be translated so this point is the new centroid of gesture
             * \param rot_down      angle which specifies down limit of rotation range used for searching distance at best angle
             * \param rot_up        angle which specifies up limit of rotation range used for searching distance at best angle
             * \param rot_thresh    if rotation change in recognition process is less than threshold recognition is done
             * \param scale_box     side length of bounding box where gesture will be scaled (non-uniformly)
             * \param scale_thresh  threshold which tells whether gesture will be scaled uniformly (no aspect ratio preserved) or non-uniformly (aspect ratio preserved).
             *                      If ratio of minimal bounding box sides of gesture is less then or equals threshold, it will be scaled non-uniformly, otherwise uniformly
             * \param start_index   index of point in gesture from where start unit vector will be computed
             * \param start_thresh  used in recognition process, if unknown gesture start vector points approximately the same direction as
             *                      template start unit vector they are compared, otherwise no. This parameter specifies the threshold for angle
             *                      between start vectors. If angle between template and gesture start vector is greater than this threshold, they are not compared to each other.
             * \param eq_strokes    if true, unknown gesture will be compered only to templates with same number of strokes in recognition process.
             */
            dollar_n(const libreco_generic_multistroke_map & tmpls, uint16_t num_of_pts, const point_2d orig, float rot_down, float rot_up,
                     float rot_thresh, uint16_t scale_box, float scale_thresh, uint16_t start_index, float start_thresh, bool eq_strokes);
            
            /**
             * \brief Method for gesture recognition
             * 
             * First, unknown gesture is transformed to unistroke representation by simple combination of its strokes.
             * Then this (combined) unistroke gesture is processed in exactly the same way as all templates were processed.
             *  
             * It is compared to templates loaded and preprocessed in constructor.
             * Unknown gesture is compared only to templates with the same number of strokes (if eq_strokes parameter in constructor was set to true). 
             * And only to those which start unit vector points in approximately the same direction as start unit vector of unknown gesture.
             * It means that angle between start vectors must be less than start vector threshold specified in constructor.
             * Otherwise template and unknown gesture are not compared to each other. 
             * Score is computed for each template that meets this requirements and multimap of scores and names for each template is returned as result.
             * 
             * \see libkerat::helpers::point_2d
             * \see libreco::recognizers::recognized_gestures
             * \see libreco::recognizers::dollar_n::dollar_n(const libreco_generic_multistroke_map & tmpls, uint16_t num_of_pts, const point_2d orig, float rot_down, float rot_up, float rot_thresh, uint16_t scale_box, float scale_thresh, uint16_t start_index, float start_thresh, bool eq_strokes);
             * 
             * \param unknown_gesture   vector of strokes which represents unknown gesture made by user
             * \return                  multimap of scores for recognized templates loaded in constructor (scores are sorted in descending order)
             */
            libreco::recognizers::recognized_gestures recognize(const vector<vector<point_2d> > & unknown_gesture) const;
            
        private:
            vector<libreco::rutils::multistroke_gesture> dollar_n_templates;
            uint16_t number_of_points;
            point_2d origin;
            float rotation_down_limit;
            float rotation_up_limit;
            float rotation_threshold;
            uint16_t scale_box_size;
            float scaling_threshold;
            uint16_t start_vector_index;
            float start_vector_threshold;
            bool equal_strokes_numbers;
                        
            //! \brief transforms multistroke templates to all possible uni-stroke permutations
            void generate_unistroke_permutations(const libreco_generic_multistroke_map & tmpls);
            
            //! \brief computes all possible orders of strokes made
            static void heap_permute(uint16_t n, vector<uint16_t> & order, vector<vector<uint16_t> > & orders);
            
            //! \brief creates all possible unistrokes for given multistroke gesture
            static void make_unistrokes(const vector<vector<point_2d> > & multistroke,
                                 const vector<vector<uint16_t> > & orders,
                                 vector<std::pair<point_2d, vector<point_2d> > > & out_unistrokes);
            
            //! \brief transforms multistroke gesture to unistroke gesture (simple merge of points)
            static void combine_strokes(const vector<vector<point_2d> > & multistrokes, vector<point_2d> & unistroke);
            
            //! \brief scales gesture uniformly or non-uniformly based on scaling threshold specified in constructor
            void dimensional_scale(vector<point_2d> & unistroke) const;
            
            //! \brief computes start unit vector for given unistroke (start unit vector is computed for point at index specified in constructor)
            point_2d start_unit_vector(const vector<point_2d> & unistroke) const;
            
            //! \brief computes angle between two vectors
            static float angle_between_vectors(const point_2d & p1, const point_2d & p2);
            
            //! \brief computes distance at best angle between unknown gesture and given template
            float compare_gestures(const vector<libreco::rutils::multistroke_gesture>::const_iterator & tmpls_iter,
                                   const point_2d & start_vector, const vector<point_2d> & unistroke) const;
          
        
        };
        
    } // ns recognizers
    
    namespace rutils {
    
         //! \brief dollar_n recognizer name
        template <>
        struct recognizer_name<libreco::recognizers::dollar_n> {
            static const char * NAME;
        };
    }
    
} // ns libreco

#endif	/* DOLLAR_N_HPP_ */


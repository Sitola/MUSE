/**
 * \file      recognizers_utils.hpp
 * \brief     Provides the header file for classes and structures (utils) used by implemented recognizers
 * \author    Martin Luptak <374178@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-11-08 10:18 UTC+1
 * \copyright BSD
 */

#ifndef RECOGNIZERS_UTILS_HPP_
#define RECOGNIZERS_UTILS_HPP_

#include <kerat/message_helpers.hpp>
#include <kerat/typedefs.hpp>

#include <vector>

namespace libreco {
    namespace rutils {

        using libkerat::helpers::point_2d;

        //! \brief Holds information needed for correct preprocessing and recognition of unistroke gesture templates
        struct unistroke_gesture {
            /**
             * \brief name of the gesture (template)
             * This name is used as template identifier in dtuio::gesture::gesture_identification message.
             * 
             * \see dtuio::gesture::gesture_identification
             */
            std::string name;
            
            /**
             * \brief specifies whether gesture (template) should also be loaded in reverse order of points
             * For example, it is appropriate that circle will be loaded in both directions (clockwise and counterclockwise).
             * This attribute is important for \ref libreco::recognizers::protractor and also for \ref libreco::recognizers::dollar_recognizer in preprocessing steps.
             */
            bool revert;
            
            /**
             * \brief specifies whether gesture (template) should be treated as orientation sensitive or invariant
             * For example up arrow is orientation sensitive, but circle is orientation invariant.
             * Only \ref libreco::recognizers::protractor needs (uses) this attribute because \ref libreco::recognizers::dollar_recognizer is orientation invariant by default.
             */
            bool sensitive;
            
            //! \brief vector of points which represents gesture (template)
            std::vector<libkerat::helpers::point_2d> points;
            
            //! \brief default constructor
            unistroke_gesture() { ; }
            
            /**
             * \brief Creates new unistroke gesture (template)
             * 
             * \see libkerat::helpers::point_2d
             * 
             * \param arg_name  name of the gesture (template)
             * \param rev       specifies whether gesture (template) should also be loaded in reverse order of points (true) or not (false)
             * \param sens      specifies whether gesture (template) is orientation sensitive (true) or invariant (false)
             * \param pts       vector of points, which represents given gesture (template)
             */
            unistroke_gesture(std::string arg_name, bool rev, bool sens, std::vector<libkerat::helpers::point_2d> pts)
                    : name(arg_name), revert(rev), sensitive(sens), points(pts) { ; }
        };
        
        //! \brief Holds information needed for correct preprocessing and recognition of uni and multistroke gesture templates
        struct multistroke_gesture {
            /**
             * \brief name of the gesture (template)
             * This name is used as template identifier in dtuio::gesture::gesture_identification message
             * 
             * \see dtuio::gesture::gesture_identification
             */
            std::string name;
            
            /**
             * \brief specifies whether gesture (template) should be treated as orientation sensitive or invariant
             * For example up arrow is orientation sensitive, but circle is orientation invariant.
             */
            bool sensitive;
            
            /** \brief vector of pairs, where each pair holds start unit vector and gesture (template) points
             * In preprocessing steps each multistroke template is transformed to multiple unistroke templates by combining multistrokes to unistrokes.
             * Then start unit vectors (first item in pair) for each unistroke (second item) are stored in vector as pairs.
             * Start unit vector for each unistroke is important in recognition process and it is used as for optimizations.
             * 
             * \see libreco::recognizers::dolar_n
             */
            std::vector<std::pair<point_2d, std::vector<point_2d> > > unistrokes;
            
            //! \brief number of partial strokes in multi-stroke gesture
            uint16_t number_of_strokes;
            
            //! brief default constructor
            multistroke_gesture() { ; }
            
            /**
             * \brief Creates new multistroke gesture (template)
             * 
             * \see libkerat::helpers::point_2d
             * 
             * \param arg_name      name of the gesture (template)
             * \param sens          specifies whether gesture (template) is orientation sensitive (true) or invariant (false)
             * \param unistr        vector of pairs, where each pair holds start unit vector and gesture (template) points
             */
            multistroke_gesture(std::string arg_name, bool sens, std::vector<std::pair<libkerat::helpers::point_2d, std::vector<libkerat::helpers::point_2d> > > unistr)
                    : name(arg_name), sensitive(sens), unistrokes(unistr) { ; }
        };
        
        //! \brief Holds point_2d instance (point in 2D space) and time stamp for this point
        struct point_time {
            //! \brief time stamp of given point (needed for sorting points according their creation time in case that UDP protocol is used)
            libkerat::timetag_t arrival_time;
            //! \brief point_2d instance (point in 2D space)
            point_2d point;
            
            /**
             * \brief Creates new point_time structure with given parameters
             * 
             * \see libkerat::helpers::point_2d
             * \see libkerat::timetag_t
             * 
             * \param time  time stamp for given point
             * \param p     point_2d instance (point in 2D space)
             */
            point_time(const libkerat::timetag_t & time, const point_2d & p) : arrival_time(time), point(p) {
                ;
            }
            
            /**
             * \brief Transforms point_time to point_2d instance (used in case time stamp is no more needed)
             * 
             * \see libkerat::helpers::point_2d
             * 
             * \param p     point_time parameter
             * \return      point_2d instance from given parameter (time stamp is forgotten)
             */
            static inline point_2d get_point2d(const point_time & p) { return p.point; }
        };
        
        /**
         * \brief Holds identification information about gesture (template)
         * This structure serves as unique identification for each template.
         * It is used while loading templates from XML file.
         * 
         * \see libmuse/framework_init.cpp
         */
        struct gesture_identity {
            //! \brief unique identifier for each template in XML file
            uint16_t gesture_id;
            
            /**
             * \brief name of the gesture (template)
             * This name is used as template identifier in dtuio::gesture::gesture_identification message
             * 
             * \see dtuio::gesture::gesture_identification
             */
			std::string gesture_name;
            
            /**
             * \brief specifies whether gesture (template) should be treated as orientation sensitive or invariant
             * For example up arrow is orientation sensitive, but circle is orientation invariant.
             */
            bool orientation_sensitive;
            
            /**
             * \brief specifies whether gesture (template) should also be loaded in reverse order of points
             * For example, it is appropriate that circle will be loaded in both directions (clockwise and counterclockwise).
             */
            bool revert;
            
            //! \brief default constructor
            gesture_identity() : gesture_id(0), gesture_name(""), orientation_sensitive(false), revert(false) { ; }
            
            /**
             * \brief Creates new structure used for template identification
             * 
             * \param g_id      unique identifier for each template stored in XML file
             * \param g_name    name of the gesture (template)
             * \param o_sens    specifies whether gesture (template) is orientation sensitive (true) or invariant (false)
             * \param rev       specifies whether gesture (template) should also be loaded in reverse order of points (true) or not (false)
             */
            gesture_identity(uint16_t g_id, std::string g_name, bool o_sens, bool rev = false)
            : gesture_id(g_id), gesture_name(g_name), orientation_sensitive(o_sens), revert(rev) {
                ;
            }
            
            //! \brief in loading process this structure is used as key in map so this operator must be defined (comparison is based on unique template id)
			bool operator<(const gesture_identity & gest) const {
				return this->gesture_id < gest.gesture_id;
			}
        };
        
        /**
         * \brief Holds information about stroke (user id and session id)
         */
        struct stroke_identity {
            //! \brief id of user who made the stroke
            libkerat::user_id_t u_id;
            
            //! \brief stroke unique identifier
            libkerat::session_id_t s_id;
            
            stroke_identity(libkerat::user_id_t uid_arg, libkerat::session_id_t sid_arg)
            : u_id(uid_arg), s_id(sid_arg) {
                ;
            }
            
            //! \brief sorts instances in the map according to stroke id
            bool operator<(const stroke_identity & stroke) const {
				return this->s_id < stroke.s_id;
			}
        };
        
        //! auxiliary class (represents minimal bounding box of gesture)
        class rectangle {
        public:
            /**
             * \brief Creates new minimal bounding box of gesture (rectangle)
             * 
             * \see libkerat::helpers::point_2d
             * 
             * \param bottom_left   bottom left point of rectangle
             * \param top_right     top right point of rectangle
             */
            rectangle(point_2d bottom_left, point_2d top_right);
            
            //! \brief for future use
            virtual ~rectangle();
            
            //! \brief bottom left point of minimal bounding box (rectangle)
            inline point_2d get_bottom_left() const {
                return bottom_left;
            }
            
            //! \brief top right point of minimal bounding box (rectangle)
            inline point_2d get_top_right() const {
                return top_right;
            }
            
            //! \brief width of minimal bounding box (width of rectangle)
            inline float width() const {
                return top_right.get_x() - bottom_left.get_x();
            }
            
            //! \brief height of minimal bounding box (height of rectangle)
            inline float height() const {
                return top_right.get_y() - bottom_left.get_y();
            }
            
        private:
            point_2d bottom_left;
            point_2d top_right;
        };
        
        
        //! \brief defines names for recognizers
        template <class RECOGNIZER>
        struct recognizer_name;
        
        

    } // ns rutils
} // ns libreco
#endif /* RECOGNIZERS_UTILS_HPP_ */

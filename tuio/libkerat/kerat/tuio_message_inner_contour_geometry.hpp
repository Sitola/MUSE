/**
 * \file      tuio_message_inner_contour_geometry.hpp
 * \brief     TUIO 2.0 inner contour geometry (/tuio2/icg)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-04-21 15:21 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGE_INNER_CONTOUR_GEOMETRY_HPP
#define KERAT_TUIO_MESSAGE_INNER_CONTOUR_GEOMETRY_HPP

#include <kerat/message.hpp>
#include <kerat/typedefs.hpp>
#include <kerat/message_helpers.hpp>
#include <ostream>
#include <list>
#include <vector>

namespace libkerat {
    namespace message {

        //! \brief TUIO 2.0 inner contour geometry (/tuio2/icg)
        class inner_contour: public libkerat::kerat_message,
            public libkerat::helpers::contact_session,
            public libkerat::helpers::movable_2d,
            public libkerat::helpers::scalable_2d,
            public libkerat::helpers::rotatable_cs_2d
        {
        public:
            //! \brief List of points that belong to the contour
            typedef std::list<helpers::point_2d> point_2d_list;
            //! \brief Vector of contours
            typedef std::vector<point_2d_list> contour_list;

            //! \brief Creates new, empty inner contour message
            inner_contour();
            
            /**
             * \brief Creates a new inner contour message for given contact
             * \param session_id - session id of the contact
             */
            inner_contour(const session_id_t session_id);
            
            /**
             * \brief Creates a new inner contour message from given data
             * \param session_id - session id of the contact
             * \param contours - vector of individual inner contours of this contact
             */
            inner_contour(const session_id_t session_id, const contour_list & contours);
            
            //! \brief Creates a deep copy of this message
            inner_contour(const inner_contour & second);
            
            ~inner_contour();

            inner_contour & operator=(const inner_contour & second);
            bool operator == (const inner_contour & second) const ;
            inline bool operator != (const inner_contour & second) const { return !operator==(second); }

            /**
             * \brief Sets the contours being held
             * \param contours - vector of contours to set
             * \return previous setting
             */
            contour_list set_contours(const contour_list & contours);
            
            /**
             * \brief Get the contours vector of this message
             * \return vector of individual inner contours
             */
            inline const contour_list & get_contours() const { return m_contours; }
            
            /**
             * \brief Directly access the inner contour
             * \param contour_index - index of the contour in the stored contours vector
             * \throws std::out_of_range if contour with given contour_index does not exist
             * \return inner contour if the contour_index is valid, throws exception otherwise
             */
            const point_2d_list & get_contour(size_t contour_index) const throw (std::out_of_range);
            
            /**
             * \brief Gets count of inner contours stored in this message
             * \return non-negative contour count
             */
            inline size_t get_contours_count() const { return m_contours.size(); }

            /**
             * \brief Appends contour to the contours vector held
             * \param contour - contour to add
             */
            void add_contour(const point_2d_list & contour);

            inner_contour * clone() const;

            void print(std::ostream & output) const ;

            // move, scale a rotatable overrides
            void scale_x(float factor, const helpers::point_2d & center);
            void scale_y(float factor, const helpers::point_2d & center);
            void move_x(coord_t factor);
            void move_y(coord_t factor);
            void rotate_by(angle_t angle, const helpers::point_2d & center);
            
            //! OSC path for TUIO 2.0 inner contour message
            static const char * PATH;
        private:

            bool imprint_lo_messages(lo_bundle target) const;

            contour_list m_contours;

        }; // cls inner_contour

    } // ns message
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::inner_contour &  msg_icg);

#endif // KERAT_TUIO_MESSAGE_INNER_CONTOUR_GEOMETRY_HPP

/**
 * \file      tuio_message_outer_contour_geometry.hpp
 * \brief     TUIO 2.0 outer contour geometry (/tuio2/ocg)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-04-21 15:21 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGE_OUTER_CONTOUR_GEOMETRY_HPP
#define KERAT_TUIO_MESSAGE_OUTER_CONTOUR_GEOMETRY_HPP

#include <kerat/message.hpp>
#include <kerat/typedefs.hpp>
#include <kerat/message_helpers.hpp>
#include <ostream>

namespace libkerat {
    namespace message {

        //! \brief TUIO 2.0 outer contour geometry (/tuio2/ocg)
        class outer_contour: public libkerat::kerat_message,
            public libkerat::helpers::contact_session,
            public libkerat::helpers::scalable_2d,
            public libkerat::helpers::movable_2d,
            public libkerat::helpers::rotatable_cs_2d
        {
        public:
            //! \brief List of points that belong to the contour
            typedef std::list<helpers::point_2d> point_2d_list;

            //! \brief Creates new, empty outer contour message
            outer_contour();
            
            /**
             * \brief Creates a new outer contour message for given contact
             * \param session_id - session id of the contact
             */
            outer_contour(const session_id_t session_id);
            
            /**
             * \brief Creates a new outer contour message from given data
             * \param session_id - session id of the contact
             * \param points - list of points that create the contour
             */
            outer_contour(const session_id_t session_id, const point_2d_list & points);
            
            //! \brief Creates a deep copy of this message
            outer_contour(const outer_contour & second);
            
            ~outer_contour();

            outer_contour & operator=(const outer_contour & second);
            bool operator == (const outer_contour & second) const ;
            inline bool operator != (const outer_contour & second) const { return !operator==(second); }

            /**
             * \brief Sets the contour being held
             * \param points - vector of contour points to set
             * \return previous setting
             */
            point_2d_list set_contour(const point_2d_list & points);

            /**
             * \brief Get the contour being held
             * \return list of individual contour points
             */
            inline const point_2d_list & get_contour() const { return m_contour; }

            outer_contour * clone() const;

            void print(std::ostream & output) const ;
            
            // move, scale a rotatable overrides
            void scale_x(float factor, const helpers::point_2d & center);
            void scale_y(float factor, const helpers::point_2d & center);
            void move_x(coord_t factor);
            void move_y(coord_t factor);
            void rotate_by(angle_t angle, const helpers::point_2d & center);

            //! OSC path for TUIO 2.0 outer contour message
            static const char * PATH;
        private:

            bool imprint_lo_messages(lo_bundle target) const;

            point_2d_list m_contour;

        }; // outer_contour

    } // ns message
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::outer_contour &  msg_ocg);

#endif // KERAT_TUIO_MESSAGE_OUTER_CONTOUR_GEOMETRY_HPP

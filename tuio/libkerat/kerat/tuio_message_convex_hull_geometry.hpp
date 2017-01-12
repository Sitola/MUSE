/**
 * \file      tuio_message_convex_hull_geometry.hpp
 * \brief     TUIO 2.0 convex hull geometry (/tuio2/chg)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-04-21 15:21 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGE_CONVEX_HULL_GEOMETRY_HPP
#define KERAT_TUIO_MESSAGE_CONVEX_HULL_GEOMETRY_HPP

#include <kerat/message.hpp>
#include <kerat/typedefs.hpp>
#include <kerat/message_helpers.hpp>
#include <kerat/utils.hpp>
#include <ostream>

namespace libkerat {
    namespace message {

        //! \brief TUIO 2.0 convex hull geometry (/tuio2/chg)
        class convex_hull: public libkerat::kerat_message,
            public libkerat::helpers::contact_session,
            public libkerat::helpers::movable_2d,
            public libkerat::helpers::scalable_2d,
            public libkerat::helpers::rotatable_cs_2d
        {
        public:
            typedef std::list<helpers::point_2d> point_2d_list;
            
            //! \brief Create a new, empty convex hull geometry message
            convex_hull();
            
            /**
             * \brief Create a new convex hull geometry message for given contact
             * \param session_id - session id of the contact
             */
            convex_hull(const session_id_t session_id);
            
            /**
             * \brief Create a new convex hull geometry message from given data
             * \param session_id - session id of the contact
             * \param points - points of the convex hull
             * \todo check whether the given points fit the convex hull definition
             */
            convex_hull(const session_id_t session_id, const point_2d_list & points);
            
            /**
             * \brief Create a copy of given convex hull geometry message
             * \param original - original message to make copy of
             */
            convex_hull(const convex_hull & original);
            ~convex_hull();

            convex_hull & operator=(const convex_hull & second);
            bool operator == (const convex_hull & second) const ;
            inline bool operator != (const convex_hull & second) const { return !operator==(second); }
            
            /**
             * \brief Set the convex hull points
             * \param points - points of the convex hull
             * \todo check whether the given points fit the convex hull definition
             * \return previous setting
             */
            point_2d_list set_hull(const point_2d_list & points);
            
            /**
             * \brief Get the convex hull points
             * \return list of \ref libkerat::helpers::point_2d "2D points" that create the hull
             */
            inline const point_2d_list & get_hull() const { return m_hull; }
            
            convex_hull * clone() const;
            
            void print(std::ostream & output) const ;
            
            // move, scale a rotatable overrides
            void scale_x(float factor, const helpers::point_2d & center);
            void scale_y(float factor, const helpers::point_2d & center);
            void move_x(coord_t factor);
            void move_y(coord_t factor);
            void rotate_by(angle_t angle, const helpers::point_2d & center);

            //! OSC path for TUIO 2.0 convex hull geometry
            static const char * PATH;
        private:

            bool imprint_lo_messages(lo_bundle target) const;
            
            point_2d_list m_hull;

        }; // cls convex_hull

    } // ns message
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::convex_hull &  msg_chg);

#endif // KERAT_TUIO_MESSAGE_CONVEX_HULL_GEOMETRY_HPP

/**
 * \file      tuio_message_area_geometry.hpp
 * \brief     TUIO 2.0 area geometry (/tuio2/arg)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-04-26 15:38 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGE_AREA_GEOMETRY_HPP
#define KERAT_TUIO_MESSAGE_AREA_GEOMETRY_HPP

#include <kerat/message.hpp>
#include <kerat/typedefs.hpp>
#include <kerat/message_helpers.hpp>
#include <ostream>

#include "message_helpers.hpp"

namespace libkerat {
    namespace message {

        //! \brief TUIO 2.0 area geometry (/tuio2/arg)
        class area: public libkerat::kerat_message,
            public libkerat::helpers::contact_session,
            public libkerat::helpers::scalable_independent_2d
        {
        public:

            //! map of spans according to their origin points
            typedef std::map<libkerat::helpers::point_2d, libkerat::distance_t> span_map;
            typedef std::pair<libkerat::helpers::point_2d, libkerat::distance_t> span;

            //! \brief create a new, empty area geometry message
            area();
            
            /*
             * \brief create a new area geometry message for given session id
             * \param session_id - session id for which this message is valid
             */
            area(const session_id_t session_id);
            
            /*
             * \brief create a new area geometry message for given session id holding following spans
             * \param session_id - session id for which this message is valid
             * \param spans - spans of this message
             * \note Overlaping spans are merged when being set
             */
            area(const session_id_t session_id, const span_map & spans);
            
            //! \brief Create deep copy
            area(const area & second);
            ~area();

            area & operator=(const area & second);
            bool operator == (const area & second) const ;
            inline bool operator != (const area & second) const { return !operator==(second); }

            /**
             * \brief Sets the spans held within this message
             * \param spans - spans to set
             * \note Overlaping spans are merged when being set
             * \return previous setting
             */
            span_map set_spans(const span_map & spans);
            
            /**
             * \brief Gets the spans held within this message
             * \return spans held
             */
            inline const span_map & get_spans() const { return m_spans; }
            
            /**
             * \brief Counts the spans held by this message
             * \return count of spans
             */
            inline size_t get_spans_count() const { return m_spans.size(); }

            /**
             * \brief Counts the points count held by this message
             * \return count of points held in
             */
            size_t get_points_count() const ;

            area * clone() const;

            void print(std::ostream & output) const ;

            // overrides of scalable and movable helpers
            void scale_x(float factor);
            void scale_y(float factor);
            
            //! OSC path for TUIO 2.0 area geometry
            static const char * PATH;
            
        private:
            
            //! \brief Joins the overlaping spans
            void join_spans();

            bool imprint_lo_messages(lo_bundle target) const;

            span_map m_spans;

        }; // cls area

    } // ns message
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::area &  msg_arg);

#endif // KERAT_TUIO_MESSAGE_AREA_GEOMETRY_HPP

/**
 * \file      tuio_message_skeleton_volume_geometry.hpp
 * \brief     TUIO 2.0 skeleton volume (/tuio2/svg)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-04-26 15:38 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGE_SKELETON_VOLUME_GEOMETRY_HPP
#define KERAT_TUIO_MESSAGE_SKELETON_VOLUME_GEOMETRY_HPP

#include <kerat/message.hpp>
#include <kerat/typedefs.hpp>
#include <kerat/message_helpers.hpp>
#include <ostream>

namespace libkerat {

    namespace message {

        //! \brief TUIO 2.0 skeleton volume (/tuio2/svg)
        class skeleton_volume: public libkerat::kerat_message,
            public libkerat::helpers::contact_session
        {
        public:
            //! \brief List of radiuses
            typedef std::list<libkerat::radius_t> radius_list;

            //! \brief Creates a new, empty skeleton volume message
            skeleton_volume();
            
            /**
             * \brief Creates a new skeleton volume message for given session id and empty radiuses list
             * \param session_id - session id of the skeleton contact
             */
            skeleton_volume(const session_id_t session_id);
            
            /**
             * \brief Creates a new skeleton volume message for given session id and empty radiuses list
             * \param session_id - session id of the skeleton contact
             * \param radiuses - radiuses to apply on points of skeleton geometry message with the same session id
             */
            skeleton_volume(const session_id_t session_id, const radius_list & radiuses);
            
            //! \brief Deep copy given message
            skeleton_volume(const skeleton_volume & second);
            
            ~skeleton_volume();

            skeleton_volume & operator=(const skeleton_volume & second);
            bool operator == (const skeleton_volume & second) const ;
            inline bool operator != (const skeleton_volume & second) const { return !operator==(second); }

            /**
             * \brief Set radiuses to apply on each point of the skeleton message with the same session id
             * \param radiuses - radiuses to apply on points of skeleton geometry message with the same session id
             * \return previous setting
             */
            radius_list set_radiuses(const radius_list & radiuses);
            
            /**
             * \brief Get radiuses list
             * \return radiuses list or empty list if not set
             */
            inline const radius_list & get_radiuses() const { return m_radiuses; }
            
            skeleton_volume * clone() const;

            void print(std::ostream & output) const ;

            //! OSC path for TUIO 2.0 skeleton volume message
            static const char * PATH;
        private:

            bool imprint_lo_messages(lo_bundle target) const;

            radius_list m_radiuses;

        }; // cls skeleton volume

    } // ns message
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::skeleton_volume &  msg_svg);

#endif // KERAT_TUIO_MESSAGE_SKELETON_VOLUME_GEOMETRY_HPP

/**
 * \file      tuio_message_container_association.hpp
 * \brief     TUIO 2.0 container association (/tuio2/coa)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-11-19 18:22 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGE_CONTAINER_ASSOCIATION_HPP
#define KERAT_TUIO_MESSAGE_CONTAINER_ASSOCIATION_HPP

#include <string>
#include <kerat/typedefs.hpp>
#include <kerat/message.hpp>
#include <kerat/message_helpers.hpp>
#include <set>
#include <ostream>

namespace libkerat {
    namespace message {

        //! \brief TUIO 2.0 container association (/tuio2/coa)
        class container_association : public libkerat::kerat_message,
            public libkerat::helpers::contact_session
        {
        public:
            typedef libkerat::session_set associated_ids;

            //! \brief Creates a new empty container association message
            container_association ();

            //! \brief Creates a copy of given container association message
            container_association (const container_association  & original);

            /**
             * \brief Creates a new container association message for given session id and slot
             * \param session_id - session id of the conteiner
             * \param slot - container's slot that the associated session id are within
             */
            container_association (const session_id_t session_id, const slot_t slot);

            /**
             * \brief Creates a new container association message from given components
             * \param session_id - session id of the conteiner
             * \param slot - container's slot that the associated session id are within
             * \param associations - set of session id's of associated contacts
             */
            container_association (const session_id_t session_id, const slot_t slot, const associated_ids & associations);

            container_association  & operator=(const container_association  & second);
            bool operator == (const container_association  & second) const ;
            inline bool operator != (const container_association  & second) const { return !operator==(second); }

            /**
             * \brief Gets the container's slot
             * \return container's slot id or 0 if default
             */
            inline slot_t get_slot() const { return m_slot; }

            /**
             * \brief Sets the container's slot that the associated objects are within
             * \param slot id - 0 is a default slot id
             * \return previous setting
             */
            slot_t set_slot(const slot_t slot);

            /**
             * \brief Gets the set of associated session id's
             * \return associated stored in the message
             */
            const associated_ids & get_associations() const { return m_associations; }

            /**
             * \brief Sets the associated session id's held in this message
             * \param associations - set of session id's that are in associated state
             * \return previous setting
             */
            associated_ids set_associations(const associated_ids & associations);

            //! \brief Clears the associated list held
            inline void clear(){ m_associations.clear(); }

            kerat_message * clone() const ;

            void print(std::ostream & output) const ;

            //! OSC path for TUIO 2.0 alive associations
            static const char * PATH;

        private:

            slot_t m_slot;
            associated_ids m_associations;

            bool imprint_lo_messages(lo_bundle target) const;

        }; // cls container_association
    } // ns message
} // libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::container_association &  msg_coa);

#endif // KERAT_TUIO_MESSAGE_CONTAINER_ASSOCIATION_HPP

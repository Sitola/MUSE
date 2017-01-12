/**
 * \file      tuio_message_link_association.hpp
 * \brief     TUIO 2.0 linked list association (/tuio2/lia)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-03-15 12:08 UTC+1
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGE_LINK_ASSOCIATION_HPP
#define KERAT_TUIO_MESSAGE_LINK_ASSOCIATION_HPP

#include <string>
#include <kerat/typedefs.hpp>
#include <kerat/message.hpp>
#include <kerat/message_helpers.hpp>
#include <list>
#include <ostream>

namespace libkerat {
    namespace message {

        using libkerat::helpers::link_topology;

        //! \brief TUIO 2.0 linked list association (/tuio2/lia)
        class link_association: public libkerat::kerat_message,
            public libkerat::helpers::contact_session,
            public link_topology
        {
        public:
            /**
             * \brief Creates a new, empty link association message with link type
             * \ref libkerat::helpers::link_topology::LINK_PHYSICAL "LINK_PHYSICAL"
             */
            link_association();

            //! \brief Creates a deep copy of this message
            link_association(const link_association & original);

            /**
             * \brief Creates a new link_association message for the given id and type
             * \param session_id - session of the link origin
             * \param type - type of the link
             */
            link_association(const session_id_t session_id, const link_topology::link_association_type_t type);

            /**
             * \brief Creates a new link_association message containing the data
             * Session id is extracted from the link graph
             * \param type - type of the link
             * \param links - links graph representing the associations - see
             * \ref set_link_graph for details and input requirements
             * \throws libkerat::exception::invalid_graph_topology when the link graph does not fulfill the input requrements
             */
            link_association(
                const link_topology::link_association_type_t type,
                const link_topology::internal_link_graph & links
            ) throw (libkerat::exception::invalid_graph_topology);

            link_association & operator=(const link_association & second);
            bool operator == (const link_association & second) const ;
            inline bool operator != (const link_association & second) const { return !operator==(second); }

            /**
             * \brief Sets the link graph representing the links that origin in this contact
             * \param links - link graph representing the links
             * \throws libkerat::exception::invalid_graph_topology when the link graph does not fulfill the input requrements
             * \note The link graph must fulfill all of the following requirements:
             *   \li Single component graph
             *   \li Star topology
             *   \li All edges must originate in the central node
             * \note Setting the graph affects the session id, since the value
             * of the central node is the origin session id
             * \return previous setting
             */
            link_topology::internal_link_graph set_link_graph(const internal_link_graph & links)
                throw (libkerat::exception::invalid_graph_topology)
            ;

            /**
             * \brief Sets the session id of this message
             * \param sid - session id of the origin of the links
             * \note Setting the session id affects the link graph since
             * the central node of the graph has to hold the session id as well
             * \return previous setting
             */
            session_id_t set_session_id(session_id_t sid);

            //! Clears the link graph
            void clear();

            kerat_message * clone() const ;

            void print(std::ostream & output) const ;

            //! OSC path for TUIO 2.0 linked list association message
            static const char * PATH;

        private:

            //! Ensures that the link graph is always nonempty and holds the correct session id
            void update_center();

            bool imprint_lo_messages(lo_bundle target) const;

        }; // cls link_association
    } // ns message
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::link_association &  msg_lia);

#endif // KERAT_TUIO_MESSAGE_LINK_ASSOCIATION_HPP


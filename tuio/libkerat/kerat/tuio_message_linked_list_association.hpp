/**
 * \file      tuio_message_linked_list_association.hpp
 * \brief     TUIO 2.0 linked list association (/tuio2/lla)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-03-18 13:51 UTC+1
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGE_LINKED_LIST_ASSOCIATION_HPP
#define KERAT_TUIO_MESSAGE_LINKED_LIST_ASSOCIATION_HPP

#include <string>
#include <kerat/typedefs.hpp>
#include <kerat/message.hpp>
#include <kerat/message_helpers.hpp>
#include <list>
#include <ostream>

namespace libkerat {
    namespace message {

        using libkerat::helpers::contact_session;
        using libkerat::helpers::link_topology;

        //! \brief TUIO 2.0 linked list association (/tuio2/lla)
        class linked_list_association: public libkerat::kerat_message,
            public contact_session,
            public link_topology
        {
        public:

            /**
             * \brief Creates a new, empty linked list association message with link type
             * \ref libkerat::helpers::link_topology::LINK_PHYSICAL "LINK_PHYSICAL"
             */
            linked_list_association();

            //! \brief Creates a deep copy of this message
            linked_list_association(const linked_list_association & original);

            /**
             * \brief Creates a new linked list association message for the given id and type
             * \param session_id - session of the link origin
             * \param type - type of the link
             */
            linked_list_association(const session_id_t session_id, const link_association_type_t type);

            /**
             * \brief Creates a new linked list association message containing the data
             * Session id is extracted from the link graph
             * \param type - type of the link
             * \param links - links graph representing the associations - see
             * \ref set_link_graph for details and input requirements
             * \throws libkerat::exception::invalid_graph_topology when the link graph does not fulfill the input requrements
             */
            linked_list_association(
                const link_topology::link_association_type_t type,
                const link_topology::internal_link_graph & links
            ) throw (libkerat::exception::invalid_graph_topology) ;

            linked_list_association & operator=(const linked_list_association & second);
            bool operator == (const linked_list_association & second) const ;
            inline bool operator != (const linked_list_association & second) const { return !operator==(second); }

            /**
             * \brief Sets the link graph representing the link chain that origin in this contact
             * \param links - link graph representing the link chain
             * \throws libkerat::exception::invalid_graph_topology when the link graph does
             * not fulfill the input requrements
             * \note The link graph must fulfill all of the following requirements:
             *   \li Single component oriented graph
             *   \li Accyclic chain topology - if the graph has at least 2 nodes,
             *   then there is 1 node with output degree 1 and input degree 0
             *   (origin node), 1 node with input degree 1 and output degree 0
             *   (end node) and every other node has both input and output degrees equal to 1
             * \note Setting the graph affects the session id, since the value
             * of the origin node is the link origin session id
             * \return previous setting
             */
            link_topology::internal_link_graph set_link_graph(const link_topology::internal_link_graph & links)
                throw (libkerat::exception::invalid_graph_topology)
            ;

            /**
             * \brief Sets the session id of this message
             * \param sid - session id of the origin of the link chain
             * \note Setting the session id affects the link graph since
             * the origin node of the graph has to hold the session id as well
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
            //! Ensures that the link graph origin node always carries correct session id
            void fix_graph();

            bool imprint_lo_messages(lo_bundle target) const;

        }; // cls linked_list_association

    } // ns message
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::linked_list_association &  msg_lla);

#endif // KERAT_TUIO_MESSAGE_LINKED_LIST_ASSOCIATION_HPP


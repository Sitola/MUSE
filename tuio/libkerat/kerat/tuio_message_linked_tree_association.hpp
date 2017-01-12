/**
 * \file      tuio_message_linked_tree_association.hpp
 * \brief     TUIO 2.0 linked tree association (/tuio2/lta)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-03-18 13:51 UTC+1
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGE_LINKED_TREE_ASSOCIATION_HPP
#define KERAT_TUIO_MESSAGE_LINKED_TREE_ASSOCIATION_HPP

#include <string>
#include <kerat/typedefs.hpp>
#include <kerat/message.hpp>
#include <kerat/message_helpers.hpp>
#include <list>
#include <ostream>

namespace libkerat {
    namespace message {

        /**
         * \brief TUIO 2.0 linked tree association (/tuio2/lta)
         * \note See \ref lta-ambiguity for reason of voluntary non-compliance
         * with the TUIO 2.0 draft.
         *
         * Since this message - as specified in TUIO 2.0 draft - is unparsable,
         * a change in output form of the message has been made - each NODE command
         * is predecesed with TRUE value. To enable this feature, compile the kerat
         * library with DRAFT_NONCOMPLIANT macro defined (or use --enable-noncompliant during
         * configure phase)
         */
        class linked_tree_association: public libkerat::kerat_message,
            public libkerat::helpers::contact_session,
            public libkerat::helpers::link_topology
        {
        public:

            /**
             * \brief Creates a new, empty linked tree association message with link type
             * \ref libkerat::helpers::link_topology::LINK_PHYSICAL "LINK_PHYSICAL"
             */
            linked_tree_association();

            //! \brief Creates a deep copy of this message
            linked_tree_association(const linked_tree_association & original);

            /**
             * \brief Creates a new linked tree association message for the given id and type
             * \param session_id - session of the link origin
             * \param type - type of the link
             */
            linked_tree_association(const session_id_t session_id, const link_association_type_t type);

            /**
             * \brief Creates a new linked tree association message containing the data
             * Session id is extracted from the link graph
             * \param type - type of the link
             * \param links - links graph representing the associations - see
             * \ref set_link_graph for details and input requirements
             * \throws libkerat::exception::invalid_graph_topology when the link graph does not fulfill the input requrements
             */
            linked_tree_association(
                const link_topology::link_association_type_t type,
                const link_topology::internal_link_graph & links
            ) throw (libkerat::exception::invalid_graph_topology) ;

            linked_tree_association & operator=(const linked_tree_association & second);
            bool operator == (const linked_tree_association & second) const ;
            inline bool operator != (const linked_tree_association & second) const { return !operator==(second); }

            /**
             * \brief Sets the link graph representing the link tree that origin in this contact
             * \param links - link graph representing the link chain
             * \throws libkerat::exception::invalid_graph_topology when the link graph does
             * not fulfill the input requrements
             * \note The link graph must fulfill all of the following requirements:
             *   \li Single component oriented graph
             *   \li Tree-like topology (trunk-tree) - the graph begins with leaf (origin leaf)
             *   from which leads the oriented edge to the first node, from this node
             *   to next until a split-node is found (like a trunk of real tree) - from
             *   now on, the rest of this tree can be considered classical oriented tree.
             *   All nodes and leafs are reachable from the origin leaf.
             * \note Setting the graph affects the session id, since the value
             * of the origin node is the link origin session id
             * \return previous setting
             */
            link_topology::internal_link_graph set_link_graph(const link_topology::internal_link_graph & links)
                throw (libkerat::exception::invalid_graph_topology)
            ;

            /**
             * \brief Sets the session id of this message
             * \param sid - session id of the origin of the link tree
             * \note Setting the session id affects the link graph since
             * the trunk-tree origin node of the graph has to hold the session id as well
             * \return previous setting
             */
            session_id_t set_session_id(session_id_t sid);

            //! Clears the link graph
            inline void clear(){ m_link_graph.clear(); }

            kerat_message * clone() const ;

            void print(std::ostream & output) const ;

            //! OSC path for TUIO 2.0 linked tree association message
            static const char * PATH;

        private:
            //! Ensures that the link graph trunk-tree origin node always carries correct session id
            void fix_graph();

            bool imprint_lo_messages(lo_bundle target) const;

        }; // cls linked tree association

    } // ns message
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::linked_tree_association &  msg_lta);

#endif // KERAT_TUIO_MESSAGE_LINKED_TREE_ASSOCIATION_HPP

/**
 * \file      tuio_message_link_association.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-03-15 23:18 UTC+1
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/tuio_message_link_association.hpp>
#include <kerat/utils.hpp>
#include <cstring>
#include <lo/lo.h>
#include <vector>
#include <algorithm>

namespace libkerat {
    namespace message {

        using libkerat::helpers::contact_session;
        using libkerat::helpers::link_topology;

        const char * link_association::PATH = "/tuio2/lia";

        link_association::link_association(){ ; }

        link_association::link_association(const link_association& original)
            :contact_session(original), link_topology(original)
        {
            ;
        }

        link_association::link_association(
            const link_topology::link_association_type_t type,
            const link_topology::internal_link_graph & links
        )
            throw (libkerat::exception::invalid_graph_topology)
            :link_topology(type)
        {
            set_link_graph(links);
        }

        link_association::link_association(const session_id_t session_id, const link_association_type_t type)
            :contact_session(session_id), link_topology(type)
        { ; }

        link_association & link_association::operator=(const link_association& original){
            contact_session::operator=(original);
            link_topology::operator=(original);
            return *this;
        }

        bool link_association::operator ==(const link_association& second) const {
            return (
                contact_session::operator==(second) &&
                link_topology::operator==(second)
            );
        }

        link_topology::internal_link_graph link_association::set_link_graph(const internal_link_graph & links)
            throw (libkerat::exception::invalid_graph_topology)
        {
            link_topology::internal_link_graph retval = m_link_graph;

            internal_link_graph::const_node_iterator center = internals::graph_utils::topology_star_get_oriented_inout_center(links);

            // ensure correct topology and center session id
            bool is_empty = links.empty();
            if ((!is_empty) && (center == links.nodes_end())){
                throw libkerat::exception::invalid_graph_topology("Center-to-out oriented star topology expected!");
            }
            link_topology::set_link_graph(links);

            if (is_empty){
                update_center();
            } else {
                contact_session::set_session_id(center->get_value());
            }

            return retval;
        }

        session_id_t link_association::set_session_id(session_id_t sid){
            session_id_t retval = contact_session::set_session_id(sid);
            update_center();
            return retval;
        }

        void link_association::update_center(){
            // ensure non-empty graph
            if (m_link_graph.empty()){ m_link_graph.create_node(); }
            // no-check needed
            internal_link_graph::const_node_iterator center = internals::graph_utils::topology_star_get_oriented_inout_center(m_link_graph);
            // just in case
            assert(center != m_link_graph.nodes_end());
            internal_link_graph::node_iterator central_node = m_link_graph.get_node(center->get_node_id());
            assert(central_node != m_link_graph.nodes_end());
            central_node->set_value(get_session_id());
        }

        void link_association::clear() {
            m_link_graph.clear();
            update_center();
        }

        bool link_association::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            lo_message msg = lo_message_new();
            lo_message_add_int32(msg, get_session_id());
            if (get_link_type() == LINK_PHYSICAL){
                lo_message_add_true(msg);
            } else {
                lo_message_add_false(msg);
            }

            internal_link_graph::const_node_iterator center = internals::graph_utils::topology_star_get_oriented_inout_center(m_link_graph);
            // somehow, this got broken
            if (center == m_link_graph.nodes_end()){ return false; }

            for (internal_link_graph::const_edge_iterator edge = center->edges_begin(); edge != center->edges_end(); edge++){
                link_topology::link_entry link_id = edge->get_value();
                lo_message_add_int32(msg, edge->to_node()->get_value());
                lo_message_add_int32(msg, compile_link_ports(link_id.input_port, link_id.output_port));
            }

            lo_bundle_add_message(target, PATH, msg);

            return true;
        }
        kerat_message * link_association::clone() const { return new link_association(*this); }

        void link_association::print(std::ostream & output) const { output << *this; }

    } // ns message

    namespace internals { namespace parsers {
        /**
         * \brief TUIO 2.0 link association (/tuio2/lia) parser
         *
         * The parserd messages are returned through the results argument. To
         * understand the remaining arguments, see
         * \ref libkerat::simple_client::message_convertor_entry
         *
         * \return true if the message was sucessfully recognized, false if an
         * error has occured or was not recognized
         *
         * \see libkerat::simple_client
         * libkerat::simple_client::message_convertor_entry
         * libkerat::kerat_message
         */
        bool parse_lia(
            std::vector<libkerat::kerat_message *> & results,
            const char * path,
            const char * types,
            lo_arg ** argv,
            int argc,
            void * user_data __attribute__((unused))
        ){
            if (strcmp(path, libkerat::message::link_association::PATH) != 0){ return false; }
            if ((argc%2) != 0){ return false; }
            if (types[0] != LO_INT32){ return false; }

            libkerat::message::link_association * msg_associations = new libkerat::message::link_association;
            msg_associations->set_session_id(argv[0]->i);
            if (types[1] == LO_TRUE){
                msg_associations->set_link_type(libkerat::message::link_association::LINK_PHYSICAL);
            } else if (types[1] == LO_FALSE){
                msg_associations->set_link_type(libkerat::message::link_association::LINK_LOGICAL);
            } else {
                goto xfalse;
            }

            { // load the topology
                using libkerat::helpers::link_topology;
                typedef libkerat::helpers::link_topology::internal_link_graph internal_link_graph;
                internal_link_graph links_graph;
                internal_link_graph::const_node_iterator central_node = links_graph.create_node(msg_associations->get_session_id());

                for (int i = 2; i < argc; i+=2){
                    if ((types[i] != LO_INT32) || (types[i+1] != LO_INT32)){ goto xfalse; }

                    internal_link_graph::const_node_iterator leaf_node = links_graph.create_node(argv[i]->i);
                    link_topology::link_entry entry;
                    libkerat::decompile_link_ports(argv[i+1]->i, entry.input_port, entry.output_port);
                    links_graph.create_edge(*central_node, *leaf_node, entry);
                }

                msg_associations->set_link_graph(links_graph);
            }

            results.push_back(msg_associations);
            return true;

        xfalse:
            delete msg_associations;
            msg_associations = NULL;
            return false;
        }
    }} // ns parsers // ns internals
} // ns libkerat

/**
 * \ingroup global
 * \brief Pretty-print the \ref libkerat::message::link_association "link association" message to given output stream
 * \note Format: <tt>
 * {OSC path} {session id}[ {link target session id}:{link output port}:{link input port}]*
 * </tt>
 * \param output - output stream to write to
 * \param msg_lia - link association message to print
 * \return modified output stream
 */
std::ostream & operator<<(std::ostream & output, const libkerat::message::link_association & msg_lia){
    output << libkerat::message::link_association::PATH << " " << msg_lia.get_session_id() << " "
        << ((msg_lia.get_link_type()==libkerat::message::link_association::LINK_PHYSICAL)?"True":"False");

    const libkerat::helpers::link_topology::internal_link_graph & link_graph = msg_lia.get_link_graph();

    libkerat::helpers::link_topology::internal_link_graph::const_node_iterator center =
        libkerat::internals::graph_utils::topology_star_get_oriented_inout_center(link_graph);
    // somehow, this got broken
    if (center == link_graph.nodes_end()){
        output << " <<broken>>";
    } else {
        for (
            libkerat::helpers::link_topology::internal_link_graph::const_edge_iterator edge = center->edges_begin();
            edge != center->edges_end();
            edge++
        ){
            libkerat::helpers::link_topology::link_entry link_id = edge->get_value();
            output << " " << edge->to_node()->get_value() << ":" << link_id.output_port << ":" << link_id.input_port;
        }
    }

    return output;
}

/**
 * \file      tuio_message_linked_list_association.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-03-15 23:18 UTC+1
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/tuio_message_linked_list_association.hpp>
#include <kerat/utils.hpp>
#include <cstring>
#include <lo/lo.h>
#include <algorithm>
#include <cassert>
#include <vector>

struct link_node_lo_writer {
    // just dummy, since the lta message expects the sid:lid pair in this (a bit illogical shape)
    struct link_edge_lo_writer {
        link_edge_lo_writer():m_link_id(0){ ; }
        void operator()(const libkerat::helpers::link_topology::internal_link_graph::edge_handle & handle){
            libkerat::helpers::link_topology::link_entry entry = handle.get_value();
            m_link_id = libkerat::compile_link_ports(entry.input_port, entry.output_port);
        }

        libkerat::link_ports_t m_link_id;
    };

    link_node_lo_writer(lo_message msg):m_counter(0),m_message(msg){ ; }
    void operator()(const libkerat::helpers::link_topology::internal_link_graph::node_handle & handle){
        // first node to be written is already present in session id
        if (m_counter++ != 0){
            lo_message_add_int32(m_message, handle.get_value());
            lo_message_add_int32(m_message, m_edge_writer.m_link_id);
        }
    }

    size_t m_counter;
    link_edge_lo_writer m_edge_writer;
    lo_message m_message;
};
struct link_node_ostream_writer {
    // just dummy, since the lta message expects the sid:lid pair in this (a bit illogical shape)
    struct link_edge_ostream_writer {
        link_edge_ostream_writer():m_link_id(0){ ; }
        void operator()(const libkerat::helpers::link_topology::internal_link_graph::edge_handle & handle){
            libkerat::helpers::link_topology::link_entry entry = handle.get_value();
            m_link_id = libkerat::compile_link_ports(entry.input_port, entry.output_port);
        }

        libkerat::link_ports_t m_link_id;
    };

    link_node_ostream_writer(std::ostream & output):m_counter(0),m_output(output){ ; }
    void operator()(const libkerat::helpers::link_topology::internal_link_graph::node_handle & handle){
        // first node to be written is already present in session id
        if (m_counter++ != 0){
            libkerat::link_port_t input_port = 0;
            libkerat::link_port_t output_port = 0;
            libkerat::decompile_link_ports(m_edge_writer.m_link_id, input_port, output_port);
            m_output << " " << handle.get_value() << ":" << output_port << ":" << input_port;
        }
    }

    size_t m_counter;
    link_edge_ostream_writer m_edge_writer;
    std::ostream & m_output;
};

namespace libkerat {
    namespace message {

        const char * linked_list_association::PATH = "/tuio2/lla";

        linked_list_association::linked_list_association(){ ; }

        linked_list_association::linked_list_association(const linked_list_association& original)
            :contact_session(original), link_topology(original)
        {
            ;
        }

        linked_list_association::linked_list_association(
            const link_topology::link_association_type_t type,
            const link_topology::internal_link_graph & links
        ) throw (libkerat::exception::invalid_graph_topology)
            :link_topology(type)
        {
            set_link_graph(links);
        }

        linked_list_association::linked_list_association(const session_id_t session_id, const link_topology::link_association_type_t type)
            :contact_session(session_id), link_topology(type)
        { ; }

        linked_list_association & linked_list_association::operator=(const linked_list_association& original){
            contact_session::operator=(original);
            link_topology::operator=(original);
            return *this;
        }

        bool linked_list_association::operator ==(const linked_list_association& second) const {
            return (
                contact_session::operator==(second)
                && link_topology::operator ==(second)
            );
        }

        void linked_list_association::clear() {
            m_link_graph.clear();
            fix_graph();
        }

        void linked_list_association::fix_graph(){
            // ensure non-empty graph
            if (m_link_graph.empty()){
                m_link_graph.create_node(get_session_id());
            } else {
                typedef link_topology::internal_link_graph::const_node_iterator const_node_iterator;
                typedef link_topology::internal_link_graph::node_iterator node_iterator;
                const_node_iterator node = internals::graph_utils::get_origin_leaf(m_link_graph);
                assert(node != m_link_graph.nodes_end());
                internal_link_graph::node_iterator primary_node = m_link_graph.get_node(node->get_node_id());
                assert(primary_node != m_link_graph.nodes_end());
                primary_node->set_value(get_session_id());
            }
        }

        link_topology::internal_link_graph linked_list_association::set_link_graph(const internal_link_graph & links)
            throw (libkerat::exception::invalid_graph_topology)
        {
            link_topology::internal_link_graph retval = m_link_graph;

            bool is_empty = links.empty();
            // ensure correct topology
            if (!is_empty) {
                if (!internals::graph_utils::topology_is_linear_oriented(links)){
                    throw libkerat::exception::invalid_graph_topology("Linear oriented topology expected!");
                }

                // previous garantees that there will always be origin leaf
                link_topology::internal_link_graph::const_node_iterator node
                    = internals::graph_utils::get_origin_leaf(links);
                assert(node != links.nodes_end());

                contact_session::set_session_id(node->get_value());
            }

            link_topology::set_link_graph(links);

            if (is_empty){ fix_graph(); }

            return retval;
        }

        session_id_t linked_list_association::set_session_id(session_id_t sid){
            session_id_t retval = contact_session::set_session_id(sid);
            fix_graph();
            return retval;
        }

        bool linked_list_association::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            lo_message msg = lo_message_new();
            lo_message_add_int32(msg, get_session_id());
            if (m_link_type == LINK_PHYSICAL){
                lo_message_add_true(msg);
            } else {
                lo_message_add_false(msg);
            }

            link_node_lo_writer node_writer(msg);
            libkerat::graph_topology_linear_walk(m_link_graph, node_writer, node_writer.m_edge_writer);

            lo_bundle_add_message(target, PATH, msg);

            return true;
        }

        kerat_message * linked_list_association::clone() const { return new linked_list_association(*this); }

        void linked_list_association::print(std::ostream & output) const { output << *this; }

    }

    namespace internals { namespace parsers {
        /**
         * \brief TUIO 2.0 linked list association (/tuio2/lla) parser
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
        bool parse_lla(
            std::vector<libkerat::kerat_message *> & results,
            const char * path,
            const char * types,
            lo_arg ** argv,
            int argc,
            void * user_data __attribute__((unused))
        ){
            if (strcmp(path, libkerat::message::linked_list_association::PATH) != 0){ return false; }
            if ((argc%2) != 0){ return false; }
            if (types[0] != LO_INT32){ return false; }

            libkerat::message::linked_list_association * msg_associations = new libkerat::message::linked_list_association;
            msg_associations->set_session_id(argv[0]->i);
            if (types[1] == LO_TRUE){
                msg_associations->set_link_type(libkerat::message::linked_list_association::LINK_PHYSICAL);
            } else if (types[1] == LO_FALSE){
                msg_associations->set_link_type(libkerat::message::linked_list_association::LINK_LOGICAL);
            } else {
                goto xfalse;
            }

            { // load the topology
                using libkerat::helpers::link_topology;
                typedef libkerat::helpers::link_topology::internal_link_graph internal_link_graph;
                internal_link_graph links_graph;
                // origin leaf
                internal_link_graph::node_iterator current_node = links_graph.create_node(msg_associations->get_session_id());

                for (int i = 2; i < argc; i+=2){
                    if ((types[i] != LO_INT32) || (types[i+1] != LO_INT32)){ goto xfalse; }

                    internal_link_graph::node_iterator next_node = links_graph.create_node(argv[i]->i);
                    link_topology::link_entry entry;
                    libkerat::decompile_link_ports(argv[i+1]->i, entry.input_port, entry.output_port);
                    links_graph.create_edge(*current_node, *next_node, entry);
                    current_node = next_node;
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
 * \brief Pretty-print the \ref libkerat::message::linked_list_association "linked list association" message to given output stream
 * \note Format: <tt>
 * {OSC path} {session id}[ {link target session id}:{link output port}:{link input port}]*
 * </tt>
 * \param output - output stream to write to
 * \param msg_lla - linked list association message to print
 * \return modified output stream
 */
std::ostream & operator<<(std::ostream & output, const libkerat::message::linked_list_association & msg_lla){
    output << libkerat::message::linked_list_association::PATH << " " << msg_lla.get_session_id() << " "
        << std::boolalpha << msg_lla.get_link_type();

    link_node_ostream_writer node_writer(output);
    libkerat::graph_topology_linear_walk(msg_lla.get_link_graph(), node_writer, node_writer.m_edge_writer);

    return output;
}

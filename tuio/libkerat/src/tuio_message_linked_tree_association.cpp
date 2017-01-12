/**
 * \file      tuio_message_linked_tree_association.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-03-15 23:18 UTC+1
 * \copyright BSD
 */

#include <lo/lo.h>
#include <kerat/typedefs.hpp>
#include <kerat/tuio_message_linked_tree_association.hpp>
#include <kerat/utils.hpp>
#include <cstring>
#include <algorithm>
#include <cassert>

#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

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
struct link_rollback_lo_writer {
    link_rollback_lo_writer(lo_message msg):m_message(msg){ ; }
    void operator()(
        const libkerat::helpers::link_topology::internal_link_graph::node_handle & handle1 __attribute__((unused)),
        const libkerat::helpers::link_topology::internal_link_graph::node_handle & handle2 __attribute__((unused)),
        size_t rollbacks_count
    ){
#ifdef DRAFT_NONCOMPLIANT
        lo_message_add_true(m_message);
#endif
        lo_message_add_int32(m_message, rollbacks_count);
    }

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
struct link_rollback_ostream_writer {
    link_rollback_ostream_writer(std::ostream & output):m_output(output){ ; }
    void operator()(
        const libkerat::helpers::link_topology::internal_link_graph::node_handle & handle1 __attribute__((unused)),
        const libkerat::helpers::link_topology::internal_link_graph::node_handle & handle2 __attribute__((unused)),
        size_t rollbacks_count
    ){
#ifdef DRAFT_NONCOMPLIANT
        m_output << " " << std::boolalpha << true;
#endif
        m_output << " " << rollbacks_count;
    }

    std::ostream & m_output;
};

namespace libkerat {
    namespace message {

        using libkerat::helpers::link_topology;

        const char * linked_tree_association::PATH = "/tuio2/lta";

        linked_tree_association::linked_tree_association(){ ; }

        linked_tree_association::linked_tree_association(const linked_tree_association& original)
            :contact_session(original), link_topology(original)
        { ; }

        linked_tree_association::linked_tree_association(
            const link_topology::link_association_type_t type,
            const link_topology::internal_link_graph & links
        ) throw (libkerat::exception::invalid_graph_topology)
            :link_topology(type, links)
        { set_link_graph(links); }

        linked_tree_association::linked_tree_association(const session_id_t session_id, const link_association_type_t type)
            :contact_session(session_id), link_topology(type)
        { ; }

        // operators == != =
        linked_tree_association & linked_tree_association::operator=(const linked_tree_association& original){
            contact_session::operator=(original);
            link_topology::operator=(original);
            return *this;
        }
        bool linked_tree_association::operator==(const linked_tree_association& second) const {
            return (
                contact_session::operator==(second) &&
                link_topology::operator==(second)
            );
        }

        void linked_tree_association::fix_graph(){
            // ensure non-empty graph
            if (m_link_graph.empty()){
                m_link_graph.create_node(get_session_id());
            } else {
                typedef link_topology::internal_link_graph::const_node_iterator node_iterator;
                node_iterator node = internals::graph_utils::get_oriented_outin_leaf(m_link_graph.nodes_begin(), m_link_graph.nodes_end());
                assert(node != m_link_graph.nodes_end());
                internal_link_graph::node_iterator primary_node = m_link_graph.get_node(node->get_node_id());
                assert(primary_node != m_link_graph.nodes_end());
                primary_node->set_value(get_session_id());
            }
        }

        libkerat::helpers::link_topology::internal_link_graph linked_tree_association::set_link_graph(const internal_link_graph & links)
            throw (libkerat::exception::invalid_graph_topology)
        {
            libkerat::helpers::link_topology::internal_link_graph retval = m_link_graph;

            bool is_empty = links.empty();

            if (!is_empty){
                if (!graph_trunk_tree_is_valid(links)){
                    throw exception::graph_error("Invalid topology for this type of message!");
                }
                typedef link_topology::internal_link_graph::const_node_iterator const_node_iterator;
                const_node_iterator node = internals::graph_utils::get_oriented_outin_leaf(m_link_graph.nodes_begin(), m_link_graph.nodes_end());
                assert(node != m_link_graph.nodes_end());

                contact_session::set_session_id(node->get_value());
            } else {
                fix_graph();
            }

            return retval;
        }

        session_id_t linked_tree_association::set_session_id(session_id_t sid){
            session_id_t retval = contact_session::set_session_id(sid);
            fix_graph();
            return retval;
        }

        bool linked_tree_association::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            lo_message msg = lo_message_new();
            lo_message_add_int32(msg, get_session_id());
            if (m_link_type == LINK_PHYSICAL){
                lo_message_add_true(msg);
            } else {
                lo_message_add_false(msg);
            }

            link_node_lo_writer node_writer(msg);
            link_rollback_lo_writer rollback_writer(msg);
            graph_topology_trunk_tree_walk(m_link_graph, node_writer, node_writer.m_edge_writer, rollback_writer);

            lo_bundle_add_message(target, PATH, msg);
            return true;
        }

        kerat_message * linked_tree_association::clone() const { return new linked_tree_association(*this); }

        void linked_tree_association::print(std::ostream & output) const { output << *this; }

    }

    namespace internals { namespace parsers {
        /**
         * \brief TUIO 2.0 linked tree association (/tuio2/lta) parser
         * \note See \ref lta-ambiguity for reason of voluntary non-compliance
         * with the TUIO 2.0 draft.
         *
         * Since this message - as specified in TUIO 2.0 draft - is unparsable,
         * a change in output form of the message has been made - each NODE command
         * is predecesed with TRUE value. To enable this feature, compile the kerat
         * library with DRAFT_NONCOMPLIANT macro defined (or use --noncompliant during
         * configure phase). If not compiled with this feature, all incoming LTA
         * messages shall be ignored and false returned.
         *
         * The parserd messages are returned through the results argument. To
         * understand the remaining arguments, see
         * \ref libkerat::simple_client::message_convertor_entry
         *
         * \return
         * \li compiled with DRAFT_NONCOMPLIANT: true if the message was sucessfully recognized, false if an
         * error has occured or was not recognized
         * \li else: returns false. Always. See \ref lta-ambiguity for reason.
         *
         * \see libkerat::simple_client
         * libkerat::simple_client::message_convertor_entry
         * libkerat::kerat_message
         */
        bool parse_lta(
            std::vector<libkerat::kerat_message*>& results __attribute__((unused)),
            const char* path __attribute__((unused)),
            const char* types __attribute__((unused)),
            lo_arg** argv __attribute__((unused)),
            int argc __attribute__((unused)),
            void* user_data __attribute__((unused))
        ){
#ifdef DRAFT_NONCOMPLIANT
            if (strcmp(path, libkerat::message::linked_tree_association::PATH) != 0){ return false; }

            if (types[0] != LO_INT32){ return false; }

            libkerat::session_id_t sid = argv[0]->i;
            libkerat::helpers::link_topology::link_association_type_t type = libkerat::helpers::link_topology::LINK_PHYSICAL;

            if (types[1] == LO_TRUE){
                type = libkerat::helpers::link_topology::LINK_PHYSICAL;
            } else if (types[1] == LO_FALSE){
                type = libkerat::helpers::link_topology::LINK_LOGICAL;
            } else {
                return false;
            }

            typedef libkerat::helpers::link_topology::internal_link_graph link_graph;
            typedef std::stack<link_graph::node_iterator> node_stack;

            link_graph links;
            node_stack nodes;

            link_graph::node_iterator current_node = links.create_node(sid);
            nodes.push(current_node);

            int i = 2;
            while (i < argc){
                if ((types[i] == LO_TRUE) && (types[i+1] == LO_INT32)){
                    size_t rollbacks = argv[i+1]->i;

                    assert(rollbacks < nodes.size());
                    for (size_t j = 0; j < rollbacks; j++){ nodes.pop(); }
                    current_node = nodes.top();

                    i+=2;
                } else if ((types[i] == LO_INT32) && (types[i+1] == LO_INT32)){
                    libkerat::helpers::link_topology::link_entry entry;
                    libkerat::decompile_link_ports(argv[i+1]->i, entry.input_port, entry.output_port);
                    link_graph::node_iterator next_node = links.create_node(argv[i]->i);

                    links.create_edge(*current_node, *next_node, entry);

                    current_node = next_node;
                    nodes.push(current_node);
                    i += 2;
                } else {
                    return false;
                }
            }

            libkerat::message::linked_tree_association * msg_associations = new libkerat::message::linked_tree_association(type, links);
            results.push_back(msg_associations);
            return true;
#else
            std::cerr << "lta message is not parsed due to ambiguity in protocol" << std::endl;
            return false;
#endif
        }
    }} // ns parsers // ns internals
} // ns libkerat

/**
 * \ingroup global
 * \brief Pretty-print the \ref libkerat::message::linked_tree_association "linked tree association" message to given output stream
 * \note Format when compiled as draft compliant: <tt>
 * {OSC path} {session id}[ {link target session id}:{link output port}:{link input port}]+[ {node}[ {link target session id}:{link output port}:{link input port}]+]*
 * </tt>
 * \note Format when compiled as draft non-compliant: <tt>
 * {OSC path} {session id}[ {link target session id}:{link output port}:{link input port}]+[ TRUE {node}[ {link target session id}:{link output port}:{link input port}]+]*
 * </tt>
 * \param output - output stream to write to
 * \param msg_lta - linked tree association message to print
 * \return modified output stream
 */
std::ostream & operator<<(std::ostream & output, const libkerat::message::linked_tree_association & msg_lta){
    output << libkerat::message::linked_tree_association::PATH << " " << msg_lta.get_session_id() << " "
        << std::boolalpha << msg_lta.get_link_type();

    link_node_ostream_writer node_writer(output);
    link_rollback_ostream_writer rollback_writer(output);
    libkerat::graph_topology_trunk_tree_walk(msg_lta.get_link_graph(), node_writer, node_writer.m_edge_writer, rollback_writer);

    return output;
}

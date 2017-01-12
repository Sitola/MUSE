/**
 * \file      graph.hpp
 * \brief     Provides the graph subsystem for libkerat
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-03-25 15:41 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_GRAPH_HPP
#define KERAT_GRAPH_HPP

#include <set>
#include <map>
#include <list>
#include <algorithm>
#include <iterator>
#include <queue>
#include <stack>
#include <list>
#include <iostream>
#include <cassert>
#include <cstring>
#include <cmath>
#include <kerat/typedefs.hpp>
#include <kerat/exceptions.hpp>


namespace libkerat {

    template <typename GRAPH_TYPE, typename NODE_COMPARATOR, typename EDGE_COMPARATOR>
    int graph_compare(
        const GRAPH_TYPE & first_graph,
        const GRAPH_TYPE & second_graph,
        const NODE_COMPARATOR & node_comparator,
        const EDGE_COMPARATOR & edge_comparator
    );

    // compares the degree sequences of given graphs, assumes ordered o1>o2, i1>i2.
    template <typename GRAPH_DUMMY_VECTOR>
    bool compare_degree_sequence(const GRAPH_DUMMY_VECTOR & component1, const GRAPH_DUMMY_VECTOR & component2){
        typedef typename GRAPH_DUMMY_VECTOR::const_iterator iterator;

        iterator i1 = component1.begin();
        iterator i2 = component2.begin();
        
        while ((i1 != component1.end()) && (i2 != component2.end())){
            if ((*i1)->output_degree() != (*i2)->output_degree()){ return false; }
            if ((*i1)->input_degree()  != (*i2)->input_degree() ){ return false; }
            ++i1; ++i2;
        }
        
        return (i1 == component1.end()) && (i2 == component2.end());
    }

    namespace internals {

        template <typename Node, typename Edge, class NodeComparator, class EdgeComparator> class graph;

        class graph_utils;

//        template <typename GRAPH_TYPE>
//        int graph_utils::compare_graphs(const GRAPH_TYPE & first_graph, const GRAPH_TYPE & second_graph);

        /**
         * \brief Internal class that provides unoriented graph
         * \tparam Node - non-void data type whose value the node carries. Instead of void, use bool.
         * \tparam Edge - non-void data type whose value the edge carries. Instead of void, use bool.
         */
        template <typename Node, typename Edge, class NodeComparator = std::less<Node>, class EdgeComparator = std::less<Edge> >
        class graph {
        public:
            //! \brief Type of this graph, declared for easy-use
            typedef graph<Node, Edge, NodeComparator, EdgeComparator> graph_type;

            //! \brief Value type of node
            typedef Node node_value_type;

            //! \brief Value type of edge
            typedef Edge edge_value_type;

            typedef NodeComparator node_value_comparator;

            typedef EdgeComparator edge_value_comparator;

        private:
            typedef size_t node_id_t;
            typedef node_id_t edge_id_t;

            //! \brief Internal structure used to represent edges
            struct edge_entry {
                edge_entry(const node_id_t target_node):m_target_id(target_node), m_value(edge_value_type()){ ; }
                edge_entry(const node_id_t target_node, const edge_value_type & value):m_target_id(target_node),m_value(value){ ; }

                bool operator==(const edge_entry & second) const throw () {
                    return compare_graph(*this, second) == 0;
                }

                inline bool operator!=(const edge_entry & second) const throw ()
                { return !operator==(second); }

                node_id_t m_target_id;
                edge_value_type m_value;
            };

            typedef std::map<edge_id_t, edge_entry *> edge_entry_map;

            //! \brief Internal structure used to represent nodes
            struct node_entry {
                typedef std::set<node_id_t> neighbour_set;

                node_entry(const node_id_t node_id):m_node_id(node_id), m_value(node_value_type()){ ; }
                node_entry(const node_id_t node_id, const node_value_type & value):m_node_id(node_id),m_value(value){ ; }
                ~node_entry(){
                    // naive constructor, does not update backward references
                    // but is sufficient to prevent memory leaks
                    for (typename edge_entry_map::iterator e = m_edges.begin(); e != m_edges.end(); ++e){
                        delete e->second;
                        e->second = NULL;
                    }
                    m_edges.clear();
                }

                edge_entry & check_for_edge(const edge_id_t edge_id)
                    throw (libkerat::exception::invalid_graph_component_error)
                {
                    typename edge_entry_map::iterator e = m_edges.find(edge_id);
                    if (e == m_edges.end()){
                        throw libkerat::exception::invalid_graph_component_error(
                            "No such edge!",
                            libkerat::exception::invalid_graph_component_error::EDGE
                        );
                    }

                    return *(e->second);
                }
                const edge_entry & check_for_edge(const edge_id_t edge_id) const
                    throw (libkerat::exception::invalid_graph_component_error)
                {
                    typename edge_entry_map::const_iterator e = m_edges.find(edge_id);
                    if (e == m_edges.end()){
                        throw libkerat::exception::invalid_graph_component_error(
                            "No such edge!",
                            libkerat::exception::invalid_graph_component_error::EDGE
                        );
                    }

                    return *(e->second);
                }

                node_id_t m_node_id;
                node_value_type m_value;
                edge_entry_map m_edges;
                neighbour_set m_neighbours;

            };

            typedef std::map<node_id_t, node_entry *> node_entry_map;

        public:
            class node_handle;
            class edge_handle;

        private:
            //!\brief Allow friends to create handle instances
            const node_handle get_const_node_handle(node_id_t nodeid) const {
                return node_handle(const_cast<graph<Node, Edge> *>(this), nodeid);
            }

            //!\brief Allow friends to create handle instances
            node_handle get_node_handle(node_id_t nodeid) {
                return node_handle(this, nodeid);
            }

            //!\brief Allow friends to create handle instances
            const edge_handle get_const_edge_handle(const node_id_t from_id, const edge_id_t edge_id) const {
                return edge_handle(const_cast<graph<Node, Edge> *>(this), from_id, edge_id);
            }

            //!\brief Allow friends to create handle instances
            edge_handle get_edge_handle(const node_id_t from_id, const edge_id_t edge_id) {
                return edge_handle(this, from_id, edge_id);
            }

        public:
            //! \brief Standard read-write iterator for graph nodes
            struct node_iterator: public std::iterator<std::forward_iterator_tag, node_handle> {
            private:
                friend class graph<Node, Edge>;

            public:
                typedef typename node_handle::graph_type graph_type;

                node_iterator():m_graph(NULL), m_handle(NULL){ ; }
                explicit node_iterator(node_handle & source):m_graph(source.m_parent_graph), m_handle(new node_handle(source)){ ; }

                node_iterator(const node_iterator & original)
                    :m_graph(original.m_graph), m_handle(NULL)
                {
                    if (original.m_handle != NULL) {
                        m_handle = new node_handle(*original.m_handle);
                    }
                }
                ~node_iterator(){ if (m_handle) { delete m_handle; m_handle = NULL; } }

                node_iterator & operator=(const node_iterator & original){
                    m_graph = original.m_graph;

                    if (m_handle != NULL) {
                        delete m_handle;
                        m_handle = NULL;
                    }

                    if (original.m_handle != NULL) {
                        m_handle = new node_handle(*original.m_handle);
                    }

                    return *this;
                }

                node_iterator operator++(int) {
                    node_iterator retval = *this;
                    advance();
                    return retval;
                }

                node_iterator & operator++() {
                    advance();
                    return *this;
                }

                node_handle & operator*() const { return *m_handle; }
                node_handle * operator->() const { return m_handle; }

                bool operator==(const node_iterator & original) const {
                    return (m_graph == original.m_graph) && (
                        ((m_handle == NULL) && (original.m_handle == NULL))
                        || (
                           ((m_handle != NULL) && (original.m_handle != NULL)
                           && (*m_handle == *original.m_handle)
                        ))
                    );
                }
                bool operator!=(const node_iterator & original) const {
                    return !operator==(original);
                }

            private:
                friend class const_node_iterator;

                node_iterator(graph_type * parent):m_graph(parent), m_handle(NULL){ ; }

                void advance(){
                    if ((m_graph != NULL) && (m_handle != NULL)){
                        typename graph_type::node_entry_map::const_iterator g_node
                            = m_graph->m_nodes.find(m_handle->m_node_id);

                        if (m_handle != NULL){
                            delete m_handle;
                            m_handle = NULL;
                        }

                        if (g_node == m_graph->m_nodes.end()){ return; }
                        ++g_node;

                        if (g_node != m_graph->m_nodes.end()){
                            m_handle = new node_handle(m_graph, g_node->first);
                        }
                    }
                }

                graph_type * m_graph;
                node_handle * m_handle;
            };
            typedef std::vector<node_iterator> node_iterator_vector;

            /**
             * \brief Standard read-only iterator for graph nodes
             * \note For details, see \ref node_iterator
             */
            struct const_node_iterator: public std::iterator<std::forward_iterator_tag, const node_handle> {
            private:
                friend class graph<Node, Edge>;

            public:
                typedef typename node_handle::graph_type graph_type;

                const_node_iterator():m_graph(NULL), m_handle(NULL){ ; }
                explicit const_node_iterator(const node_handle & source)
                    :m_graph(source.m_parent_graph),
                    m_handle(
                        new node_handle(source.m_parent_graph, source.m_node_id)
                    )
                { ; }

                const_node_iterator(const const_node_iterator & original)
                    :m_graph(original.m_graph), m_handle(NULL)
                {
                    if (original.m_handle != NULL) {
                        m_handle = new node_handle(original.m_handle->m_parent_graph, original.m_handle->m_node_id);
                    }
                }

                const_node_iterator(const node_iterator & original)
                    :m_graph(original.m_graph), m_handle(NULL)
                {
                    if (original.m_handle != NULL) {
                        m_handle = new node_handle(original.m_handle->m_parent_graph, original.m_handle->m_node_id);
                    }
                }

                ~const_node_iterator(){ if (m_handle) { delete m_handle; m_handle = NULL; } }

                const_node_iterator & operator=(const const_node_iterator & original){
                    m_graph = original.m_graph;

                    if (m_handle != NULL) {
                        delete m_handle;
                        m_handle = NULL;
                    }

                    if (original.m_handle != NULL) {
                        m_handle = new node_handle(*original.m_handle);
                    }

                    return *this;
                }

                const_node_iterator operator++(int) {
                    const_node_iterator retval = *this;
                    advance();
                    return retval;
                }

                const_node_iterator & operator++() {
                    advance();
                    return *this;
                }

                const node_handle & operator*() const { return *m_handle; }
                const node_handle * operator->() const { return m_handle; }

                bool operator==(const const_node_iterator & original) const {
                    return (m_graph == original.m_graph) && (
                        ((m_handle == NULL) && (original.m_handle == NULL))
                        || (
                           ((m_handle != NULL) && (original.m_handle != NULL)
                           && (*m_handle == *original.m_handle)
                        ))
                    );
                }
                bool operator!=(const const_node_iterator & original) const {
                    return !operator==(original);
                }
            private:
                const_node_iterator(const graph_type * parent):m_graph(parent), m_handle(NULL){ ; }

                void advance(){
                    if ((m_graph != NULL) && (m_handle != NULL)){
                        typename graph_type::node_entry_map::const_iterator g_node
                            = m_graph->m_nodes.find(m_handle->m_node_id);

                        if (m_handle != NULL){
                            delete m_handle;
                            m_handle = NULL;
                        }

                        if (g_node == m_graph->m_nodes.end()){ return; }
                        ++g_node;

                        if (g_node != m_graph->m_nodes.end()){
                            node_handle tmp(m_graph->get_const_node_handle(g_node->first));
                            m_handle = new node_handle(tmp);
                        }
                    }
                }

                const graph_type * m_graph;
                const node_handle * m_handle;
            };
            typedef std::vector<const_node_iterator> const_node_iterator_vector;

            //! \brief Standard read-write iterator for graph edges
            struct edge_iterator: public std::iterator<std::forward_iterator_tag, edge_handle> {
            private:
                friend class graph<Node, Edge>;

            public:
                typedef typename edge_handle::graph_type graph_type;

                edge_iterator():m_graph(NULL), m_handle(NULL){ ; }
                explicit edge_iterator(edge_handle & source):m_graph(source.m_parent_graph), m_handle(new edge_handle(source)){ ; }

                edge_iterator(const edge_iterator & original)
                    :m_graph(original.m_graph), m_handle(NULL)
                {
                    if (original.m_handle != NULL) {
                        m_handle = new edge_handle(*original.m_handle);
                    }
                }
                ~edge_iterator(){ if (m_handle) { delete m_handle; m_handle = NULL; } }

                edge_iterator & operator=(const edge_iterator & original){
                    m_graph = original.m_graph;

                    if (m_handle != NULL) {
                        delete m_handle;
                        m_handle = NULL;
                    }

                    if (original.m_handle != NULL) {
                        m_handle = new edge_handle(*original.m_handle);
                    }

                    return *this;
                }

                edge_iterator operator++(int) {
                    edge_iterator retval = *this;
                    advance();
                    return retval;
                }

                edge_iterator & operator++() {
                    advance();
                    return *this;
                }

                edge_handle & operator*() const { return *m_handle; }
                edge_handle * operator->() const { return m_handle; }

                bool operator==(const edge_iterator & original) const {
                    return (m_graph == original.m_graph) && (
                        ((m_handle == NULL) && (original.m_handle == NULL))
                        || (
                           ((m_handle != NULL) && (original.m_handle != NULL)
                           && (*m_handle == *original.m_handle)
                        ))
                    );
                }
                bool operator!=(const edge_iterator & original) const {
                    return !operator==(original);
                }

            private:
                friend class const_edge_iterator;

                edge_iterator(const graph_type * parent):m_graph(parent), m_handle(NULL){ ; }

                void advance(){
                    if ((m_graph != NULL) && (m_handle != NULL)){

                        edge_id_t current_edge = m_handle->m_edge_id;
                        node_id_t m_from_node = m_handle->m_from_node_id;
                        if (m_handle != NULL){
                            delete m_handle;
                            m_handle = NULL;
                        }

                        typename node_entry_map::const_iterator node = m_graph->m_nodes.find(m_from_node);

                        // the node we were from was deleted from the graph
                        if (node == m_graph->m_nodes.end()){ return; }

                        while ((m_handle == NULL) && (node != m_graph->m_nodes.end())){
                            typename edge_entry_map::const_iterator edge = node->second->m_edges.upper_bound(current_edge);
                            if (edge == node->second->m_edges.end()){
                                ++node;
                                current_edge = 0;
                            } else {
                                edge_handle tmp = m_graph->get_const_edge_handle(node->first, edge->first);
                                m_handle = new edge_handle(tmp);
                            }
                        }
                    }
                }

                const graph_type * m_graph;
                edge_handle * m_handle;
            };
            typedef std::list<edge_iterator> edge_iterator_list;

            /**
             * \brief Standard read-only iterator for graph edges
             * \note For details, see \ref edge_iterator
             */
            struct const_edge_iterator: public std::iterator<std::forward_iterator_tag, const edge_handle> {
            private:
                friend class graph<Node, Edge>;

            public:
                typedef typename edge_handle::graph_type graph_type;

                const_edge_iterator():m_graph(NULL), m_handle(NULL){ ; }
                explicit const_edge_iterator(const edge_handle & source):m_graph(source.m_parent_graph), m_handle(new edge_handle(source)){ ; }

                const_edge_iterator(const const_edge_iterator & original)
                    :m_graph(original.m_graph), m_handle(NULL)
                {
                    if (original.m_handle != NULL) {
                        m_handle = new edge_handle(*original.m_handle);
                    }
                }

                const_edge_iterator(const edge_iterator & original)
                    :m_graph(original.m_graph), m_handle(NULL)
                {
                    if (original.m_handle != NULL) {
                        m_handle = new edge_handle(*original.m_handle);
                    }
                }

                ~const_edge_iterator(){ if (m_handle) { delete m_handle; m_handle = NULL; } }

                const_edge_iterator & operator=(const const_edge_iterator & original){
                    m_graph = original.m_graph;

                    if (m_handle != NULL) {
                        delete m_handle;
                        m_handle = NULL;
                    }

                    if (original.m_handle != NULL) {
                        m_handle = new edge_handle(*original.m_handle);
                    }

                    return *this;
                }

                const_edge_iterator operator++(int) {
                    const_edge_iterator retval = *this;
                    advance();
                    return retval;
                }

                const_edge_iterator & operator++() {
                    advance();
                    return *this;
                }

                const edge_handle & operator*() const { return *m_handle; }
                const edge_handle * operator->() const { return m_handle; }

                bool operator==(const const_edge_iterator & original) const {
                    return (m_graph == original.m_graph) && (
                        ((m_handle == NULL) && (original.m_handle == NULL))
                        || (
                           ((m_handle != NULL) && (original.m_handle != NULL)
                           && (*m_handle == *original.m_handle)
                        ))
                    );
                }
                bool operator!=(const const_edge_iterator & original) const {
                    return !operator==(original);
                }

            private:
                friend class const_const_edge_iterator;

                const_edge_iterator(const graph_type * parent):m_graph(parent), m_handle(NULL){ ; }

                void advance(){
                    if ((m_graph != NULL) && (m_handle != NULL)){

                        edge_id_t current_edge = m_handle->m_edge_id;
                        node_id_t m_from_node = m_handle->m_from_node_id;
                        if (m_handle != NULL){
                            delete m_handle;
                            m_handle = NULL;
                        }

                        typename node_entry_map::const_iterator node = m_graph->m_nodes.find(m_from_node);

                        // the node we were from was deleted from the graph
                        if (node == m_graph->m_nodes.end()){ return; }

                        while ((m_handle == NULL) && (node != m_graph->m_nodes.end())){
                            typename edge_entry_map::const_iterator edge = node->second->m_edges.upper_bound(current_edge);
                            if (edge == node->second->m_edges.end()){
                                ++node;
                                current_edge = 0;
                            } else {
                                edge_handle tmp = m_graph->get_const_edge_handle(node->first, edge->first);
                                m_handle = new edge_handle(tmp);
                            }
                        }
                    }
                }

                const graph_type * m_graph;
                edge_handle * m_handle;
            };
            typedef std::list<const_edge_iterator> const_edge_iterator_list;

        public:

            //! \brief Class providing read-write access to individual nodes
            class node_handle {
            public:
                typedef graph<Node, Edge> graph_type;
                typedef Node node_value_type;

                //! \brief Create a copy of handle
                node_handle(node_handle & original) throw ()
                    :m_parent_graph(original.m_parent_graph), m_node_id(original.m_node_id)
                { ; }

                //! \brief Gets graph's internal node identification
                node_id_t get_node_id() const throw () { return m_node_id; }

                /**
                 * \brief Gets reference to stored value
                 * \return reference to stored value
                 * \throw libkerat::exception::invalid_graph_component_error
                 * when referenced node does not exist in the graph
                 */
                node_value_type & get_value()
                    throw (libkerat::exception::invalid_graph_component_error)
                {
                    typename graph_type::node_entry & node = m_parent_graph->check_for_node(m_node_id);
                    return node.m_value;
                }

                /**
                 * \brief Gets constant reference to stored value
                 * \return constant reference to stored value
                 * \throw libkerat::exception::invalid_graph_component_error
                 * when referenced node does not exist in the graph
                 */
                const node_value_type & get_value() const
                    throw (libkerat::exception::invalid_graph_component_error)
                {
                    typename graph_type::node_entry const & node = m_parent_graph->check_for_node(m_node_id);
                    return node.m_value;
                }

                /**
                 * \brief Get pointer to the parent graph
                 * \return pointer to the parent graph
                 */
                graph_type * get_parent_graph() throw () { return m_parent_graph; }
                const graph_type * get_parent_graph() const throw () { return m_parent_graph; }

                /**
                 * \brief Set stored value
                 * \param val - value to be stored
                 * \return reference to stored value
                 * \throw libkerat::exception::invalid_graph_component_error
                 * when referenced node does not exist in the graph
                 */
                node_value_type & set_value(const node_value_type & val)
                    throw (libkerat::exception::invalid_graph_component_error)
                {
                    typename graph_type::node_entry & node = m_parent_graph->check_for_node(m_node_id);
                    node.m_value = val;
                    return node.m_value;
                }

                /**
                 * \brief get end read-write iterator for edges that origin from this node
                 *
                 * \return \ref edges_begin of next node that has edges
                 * or graph's \ref graph::edges_end if this is the last node that has edges
                 * \throw libkerat::exception::invalid_graph_component_error
                 * when referenced node does not exist in the graph
                 */
                edge_iterator edges_end()
                    throw (libkerat::exception::invalid_graph_component_error)
                {
                    typename node_entry_map::iterator entry = m_parent_graph->m_nodes.find(m_node_id);
                    edge_iterator tmp = m_parent_graph->edges_end();

                    if ((entry->second)->m_edges.empty()){
                        edge_handle tmph = m_parent_graph->get_edge_handle(m_node_id, 0);
                        tmp = edge_iterator(tmph);
                    } else {
                        edge_handle tmph = m_parent_graph->get_edge_handle(m_node_id, entry->second->m_edges.rbegin()->first);
                        tmp = edge_iterator(tmph);
                    }

                    ++tmp;
                    return tmp;
                }

                /**
                 * \brief Gets end read-only iterator for edges that origin from this node
                 * \note For details, see \ref edges_end()
                 */
                const_edge_iterator edges_end() const
                    throw (libkerat::exception::invalid_graph_component_error)
                {
                    typename node_entry_map::iterator entry = m_parent_graph->m_nodes.find(m_node_id);
                    const_edge_iterator tmp = m_parent_graph->edges_end();

                    if (entry->second->m_edges.empty()){
                        edge_handle tmph = m_parent_graph->get_const_edge_handle(m_node_id, 0);
                        tmp = edge_iterator(tmph);
                    } else {
                        edge_handle tmph = m_parent_graph->get_const_edge_handle(m_node_id, entry->second->m_edges.rbegin()->first);
                        tmp = edge_iterator(tmph);
                    }

                    ++tmp;
                    return tmp;
                }

                /**
                 * \brief Gets begin read-write iterator for edges that origin from this node
                 * \throw libkerat::exception::invalid_graph_component_error
                 * when referenced node does not exist in the graph
                 * \return \ref edge_iterator to first edge held or \ref edges_end() if no edges held
                 */
                edge_iterator edges_begin()
                    throw (libkerat::exception::invalid_graph_component_error)
                {
                    typename node_entry_map::iterator entry = m_parent_graph->m_nodes.find(m_node_id);
                    edge_iterator tmp = m_parent_graph->edges_end();

                    if (entry->second->m_edges.empty()){
                        edge_handle tmph = m_parent_graph->get_edge_handle(m_node_id, 0);
                        tmp = edge_iterator(tmph);
                        ++tmp;
                    } else {
                        edge_handle tmph = m_parent_graph->get_edge_handle(m_node_id, entry->second->m_edges.begin()->first);
                        tmp = edge_iterator(tmph);
                    }

                    return tmp;
                }

                /**
                 * \brief Gets begin read-only iterator for edges that origin from this node
                 * \note For details, see \ref edges_begin()
                 */
                const_edge_iterator edges_begin() const
                    throw (libkerat::exception::invalid_graph_component_error)
                {
                    typename node_entry_map::iterator entry = m_parent_graph->m_nodes.find(m_node_id);
                    const_edge_iterator tmp = m_parent_graph->edges_end();

                    if (entry->second->m_edges.empty()){
                        edge_handle tmph = m_parent_graph->get_const_edge_handle(m_node_id, 0);
                        tmp = edge_iterator(tmph);
                        ++tmp;
                    } else {
                        edge_handle tmph = m_parent_graph->get_const_edge_handle(m_node_id, entry->second->m_edges.begin()->first);
                        tmp = edge_iterator(tmph);
                    }

                    return tmp;
                }

                /**
                 * \brief Get the node degree considering only edges that originate from this node
                 * \throw libkerat::exception::invalid_graph_component_error
                 * when referenced node does not exist in the graph
                 * \return count of edges originating from this node
                 */
                size_t output_degree() const throw (libkerat::exception::invalid_graph_component_error){
                    return m_parent_graph->check_for_node(m_node_id).m_edges.size();
                }

                /**
                 * \brief Get count of nodes that this node points to
                 * \throw libkerat::exception::invalid_graph_component_error
                 * when referenced node does not exist in the graph
                 * \return count of nodes that edges of this node lead to
                 */
                size_t successors_count() const
                    throw (libkerat::exception::invalid_graph_component_error)
                {
                    const node_entry & node = m_parent_graph->check_for_node(m_node_id);

                    typedef typename graph_type::edge_entry_map::const_iterator const_iterator;
                    typename graph_type::node_entry::neighbour_set successors;

                    for (const_iterator succ = node.m_edges.begin(); succ != node.m_edges.end(); succ++){
                        successors.insert(succ->second.m_target_id);
                    }

                    return successors.size();
                }

                /**
                 * \brief Get the node degree considering only edges that lead to this node
                 * \throw libkerat::exception::invalid_graph_component_error
                 * when referenced node does not exist in the graph
                 * \return count of edges leading to this node
                 */
                size_t input_degree() const throw (libkerat::exception::invalid_graph_component_error){
                    const node_entry & node = m_parent_graph->check_for_node(m_node_id);

                    size_t retval = 0;
                    typedef typename graph_type::node_entry::neighbour_set::const_iterator const_iterator;
                    edge_target_predicate tg_pred(m_node_id);

                    for (const_iterator neighbour = node.m_neighbours.begin(); neighbour != node.m_neighbours.end(); neighbour++){
                        const_node_iterator neighbour_node = m_parent_graph->get_node(*neighbour);
                        retval += std::count_if(neighbour_node->edges_begin(), neighbour_node->edges_end(), tg_pred);
                    }

                    return retval;
                }

                /**
                 * \brief Get count of nodes that point to this node
                 * \throw libkerat::exception::invalid_graph_component_error
                 * when referenced node does not exist in the graph
                 * \return count of nodes that have edges pointing to this node
                 */
                size_t predecessors_count() const
                    throw (libkerat::exception::invalid_graph_component_error)
                {
                    const node_entry & node = m_parent_graph->check_for_node(m_node_id);
                    return node.m_neighbours.size();
                }

                const_node_iterator_vector predecessors() const
                    throw (libkerat::exception::invalid_graph_component_error)
                {
                    const node_entry & node = m_parent_graph->check_for_node(m_node_id);
                    const_node_iterator_vector output;
                    output.reserve(node.m_neighbours.size());

                    for (typename node_entry::neighbour_set::const_iterator n = node.m_neighbours.begin(); n != node.m_neighbours.end(); ++n){
                        output.push_back(const_node_iterator(m_parent_graph->get_const_node_handle(*n)));
                    }

                    return output;
                }
                node_iterator_vector predecessors()
                    throw (libkerat::exception::invalid_graph_component_error)
                {
                    const node_entry & node = m_parent_graph->check_for_node(m_node_id);
                    node_iterator_vector output;
                    output.reserve(node.m_neighbours.size());

                    for (typename node_entry::neighbour_set::const_iterator n = node.m_neighbours.begin(); n != node.m_neighbours.end(); ++n){
                        node_handle tmp(m_parent_graph->get_node_handle(*n));
                        output.push_back(node_iterator(tmp));
                    }

                    return output;
                }

                /**
                 * \brief get count of all edges this node is part of
                 * \throw libkerat::exception::invalid_graph_component_error
                 * when referenced node does not exist in the graph
                 * \return \ref input_degree() + \ref output_degree()
                 */
                size_t degree() const throw (libkerat::exception::invalid_graph_component_error){
                    return output_degree()+input_degree();
                }

                bool operator==(const node_handle & second) const {
                    return (
                        (m_parent_graph == second.m_parent_graph)
                        && (m_node_id == second.m_node_id)
                    );
                }
                bool operator!=(const node_handle & second) const { return !operator==(second); }

            private:
                friend class graph<Node, Edge>;

                node_handle(graph_type * parent_graph, const node_id_t node) throw ()
                    :m_parent_graph(parent_graph), m_node_id(node)
                { ; }
                // proper copy constructor
                node_handle(const node_handle & original) throw ()
                    :m_parent_graph(original.m_parent_graph), m_node_id(original.m_node_id)
                { ; }

                graph_type * m_parent_graph;
                node_id_t m_node_id;
            };

            //! \brief Class providing read-write access to individual edges
            class edge_handle {
            public:
                typedef graph<Node, Edge> graph_type;
                typedef typename graph_type::edge_value_type edge_value_type;

                //! \brief Create copy of handle
                edge_handle(edge_handle & second) throw ()
                    :m_parent_graph(second.m_parent_graph), m_from_node_id(second.m_from_node_id), m_edge_id(second.m_edge_id)
                { ; }

                ~edge_handle() throw () { ; }

                /**
                 * \brief get node from which this edge originates
                 * \throw libkerat::exception::invalid_graph_component_error
                 * when referenced edge does not exist in the graph
                 * \return handle to origin node
                 */
                const_node_iterator from_node() const throw (libkerat::exception::invalid_graph_component_error){
                    return m_parent_graph->get_node(m_from_node_id);
                }
                node_iterator from_node() throw (libkerat::exception::invalid_graph_component_error){
                    return m_parent_graph->get_node(m_from_node_id);
                }

                /**
                 * \brief get node to which this edge leads
                 * \throw libkerat::exception::invalid_graph_component_error
                 * when referenced edge does not exist in the graph
                 * \return handle to target node
                 */
                const_node_iterator to_node() const throw (libkerat::exception::invalid_graph_component_error){
                    node_entry & entry = m_parent_graph->check_for_node(m_from_node_id);
                    edge_entry & edge_entry = entry.check_for_edge(m_edge_id);
                    return m_parent_graph->get_node(edge_entry.m_target_id);
                }
                node_iterator to_node() throw (libkerat::exception::invalid_graph_component_error){
                    node_entry & entry = m_parent_graph->check_for_node(m_from_node_id);
                    edge_entry & edge_entry = entry.check_for_edge(m_edge_id);
                    return m_parent_graph->get_node(edge_entry.m_target_id);
                }

                /**
                 * \brief Get reference to value held by this edge
                 * \throw libkerat::exception::invalid_graph_component_error
                 * when referenced node or edge does not exist in the graph
                 * \return reference to value held
                 */
                edge_value_type & get_value()
                    throw (libkerat::exception::invalid_graph_component_error)
                {
                    node_entry & entry = m_parent_graph->check_for_node(m_from_node_id);
                    edge_entry & edge_entry = entry.check_for_edge(m_edge_id);
                    return edge_entry.m_value;
                }

                /**
                 * \brief Get const reference to value held by this edge
                 * \throw libkerat::exception::invalid_graph_component_error
                 * when referenced node or edge does not exist in the graph
                 * \return const reference to value held
                 */
                const edge_value_type & get_value() const
                    throw (libkerat::exception::invalid_graph_component_error)
                {
                    node_entry & entry = m_parent_graph->check_for_node(m_from_node_id);
                    edge_entry & edge_entry = entry.check_for_edge(m_edge_id);
                    return edge_entry.m_value;
                }

                /**
                 * \brief get pointer to the parent graph
                 * \return pointer to the parent graph
                 */
                const graph_type * get_parent_graph() const throw () { return m_parent_graph; }
                graph_type * get_parent_graph() throw () { return m_parent_graph; }

                /**
                 * \brief Set value held by this edge
                 * \throw libkerat::exception::invalid_graph_component_error
                 * when referenced node or edge does not exist in the graph
                 * \return reference to value held
                 */
                edge_value_type & set_value(const edge_value_type & value){
                    node_entry & entry = m_parent_graph->check_for_node(m_from_node_id);
                    edge_entry & edge_entry = entry.check_for_edge(m_edge_id);
                    edge_entry.m_value = value;
                    return edge_entry.m_value;
                }

                bool operator==(const edge_handle & second) const {
                    return (
                        (m_parent_graph == second.m_parent_graph)
                        && (m_from_node_id == second.m_from_node_id)
                        && (m_edge_id == second.m_edge_id)
                    );
                }
                bool operator!=(const edge_handle & second) const { return !operator==(second); }

            private:
                friend class graph<Node, Edge>;
                friend class graph<Node, Edge>::edge_iterator;
                friend class graph<Node, Edge>::node_handle;

                /**
                 * \brief create handle from it's unique data
                 */
                edge_handle(graph_type * parent, const node_id_t from_id, const edge_id_t edge_id) throw ()
                    :m_parent_graph(parent), m_from_node_id(from_id), m_edge_id(edge_id)
                { ; }

                // proper copy constructor, accessible by friends only
                edge_handle(const edge_handle & second) throw ()
                    :m_parent_graph(second.m_parent_graph), m_from_node_id(second.m_from_node_id), m_edge_id(second.m_edge_id)
                { ; }

                graph_type * m_parent_graph;
                node_id_t m_from_node_id;
                edge_id_t m_edge_id;
            };

        private:
            friend class libkerat::internals::graph_utils;

            node_id_t m_last_node_id;
            edge_id_t m_last_edge_id;
            node_entry_map m_nodes;
            const node_value_comparator & m_node_comparator;
            const edge_value_comparator & m_edge_comparator;

            node_entry & check_for_node(const node_id_t node_id)
                throw (libkerat::exception::invalid_graph_component_error)
            {
                typename node_entry_map::iterator found = m_nodes.find(node_id);
                if (found != m_nodes.end()){
                    return *(found->second);
                }

                throw libkerat::exception::invalid_graph_component_error(
                    "No such node!",
                    libkerat::exception::invalid_graph_component_error::NODE
                );
            }

            const node_entry & check_for_node(const node_id_t node_id) const
                throw (libkerat::exception::invalid_graph_component_error)
            {
                typename node_entry_map::const_iterator found = m_nodes.find(node_id);
                if (found != m_nodes.end()){
                    return *(found->second);
                }

                throw libkerat::exception::invalid_graph_component_error(
                    "No such node!",
                    libkerat::exception::invalid_graph_component_error::NODE
                );
            }

            bool swap_node_id(const node_id_t node_id_1, const node_id_t node_id_2){
                if (node_id_1 == node_id_2){ return true; }

                typename node_entry_map::iterator n1 = m_nodes.find(node_id_1);
                typename node_entry_map::iterator n2 = m_nodes.find(node_id_2);

                if ((n1 == m_nodes.end()) || (n2 == m_nodes.end())){ return false; }

                {// swap nodes
                    node_entry * tmp = n1->second;
                    n1->second = n2->second;
                    n2->second = tmp;
                    n1->second->m_node_id = n1->first;
                    n2->second->m_node_id = n2->first;
                }
                // node identificators updated
                // edges update is however required

                // update all affected predecessors, all_neighbours ensures that each of the predecessors is scanned only once
                typename node_entry::neighbour_set all_predecesors = n1->second->m_neighbours;
                   all_predecesors.insert(n2->second->m_neighbours.begin(), n2->second->m_neighbours.end());

                typename node_entry::neighbour_set neighbours_1;
                typename node_entry::neighbour_set neighbours_2;

                for (
                    typename node_entry::neighbour_set::const_iterator neighbour = all_predecesors.begin();
                    neighbour != all_predecesors.end();
                    ++neighbour
                ){
                    typename node_entry_map::iterator current = m_nodes.find(*neighbour);
                    assert(current != m_nodes.end());
                    for (
                        typename edge_entry_map::iterator e = current->second->m_edges.begin();
                        e != current->second->m_edges.end();
                        ++e
                    ){
                        if (e->second->m_target_id == node_id_1){
                            e->second->m_target_id = node_id_2;
                            neighbours_2.insert(current->first);
                        } else if (e->second->m_target_id == node_id_2){
                            e->second->m_target_id = node_id_1;
                            neighbours_1.insert(current->first);
                        }
                    }
                }
                n1->second->m_neighbours = neighbours_1;
                n2->second->m_neighbours = neighbours_2;
                // predecesors updated

                // update those who we point to - phase1 - clean
                for (
                    typename edge_entry_map::const_iterator e = n1->second->m_edges.begin();
                    e != n1->second->m_edges.end();
                    ++e
                ){
                    // make sure we don't update updated
                    if ((e->second->m_target_id == node_id_1) || (e->second->m_target_id == node_id_2)){
                        continue;
                    }

                    typename node_entry_map::iterator current = m_nodes.find(e->second->m_target_id);
                    assert(current != m_nodes.end());
                    // erase original node_id of n1
                    current->second->m_neighbours.erase(node_id_2);
                }
                for (
                    typename edge_entry_map::const_iterator e = n2->second->m_edges.begin();
                    e != n2->second->m_edges.end();
                    ++e
                ){
                    // make sure we don't update updated
                    if ((e->second->m_target_id == node_id_1) || (e->second->m_target_id == node_id_2)){
                        continue;
                    }

                    typename node_entry_map::iterator current = m_nodes.find(e->second->m_target_id);
                    assert(current != m_nodes.end());
                    // erase original node_id of n2
                    current->second->m_neighbours.erase(node_id_1);
                }
                // update those who we point to - phase2 - add
                for (
                    typename edge_entry_map::const_iterator e = n1->second->m_edges.begin();
                    e != n1->second->m_edges.end();
                    ++e
                ){
                    // make sure we don't update updated
                    if ((e->second->m_target_id == node_id_1) || (e->second->m_target_id == node_id_2)){
                        continue;
                    }
                    typename node_entry_map::iterator current = m_nodes.find(e->second->m_target_id);
                    // imprint new id
                    current->second->m_neighbours.insert(node_id_1);
                }
                for (
                    typename edge_entry_map::const_iterator e = n2->second->m_edges.begin();
                    e != n2->second->m_edges.end();
                    ++e
                ){
                    // make sure we don't update updated
                    if ((e->second->m_target_id == node_id_1) || (e->second->m_target_id == node_id_2)){
                        continue;
                    }
                    typename node_entry_map::iterator current = m_nodes.find(e->second->m_target_id);
                    // imprint new id
                    current->second->m_neighbours.insert(node_id_2);
                }
                // all forward's references backward references updated

                return true;
            }
            bool swap_edge_id(const edge_handle & eh1, const edge_handle & eh2){
                if (
                    (eh1.m_parent_graph != this)
                    || (eh1.m_parent_graph != eh2.m_parent_graph)
                    || (eh1.m_from_node_id != eh2.m_from_node_id)
                ) { return false; }
                if (eh1.m_edge_id == eh2.m_edge_id){ return true; }

                typename node_entry_map::iterator node = m_nodes.find(eh1.m_from_node_id);

                if (node == m_nodes.end()){ return false; }

                typename edge_entry_map::iterator e1 = node->second->m_edges.find(eh1.m_edge_id);
                typename edge_entry_map::iterator e2 = node->second->m_edges.find(eh2.m_edge_id);

                if ((e1 == node->second->m_edges.end()) || (e2 == node->second->m_edges.end())){ return false; }

                { // swap edges
                    edge_entry * tmp = e1->second;
                    e1->second = e2->second;
                    e2->second = tmp;
                }
                // edge identificators updated

                return true;
            }

            //! \brief safely deallocate all data held
            void reset(){
                typedef typename node_entry_map::iterator ne_i;
                typedef typename edge_entry_map::iterator ee_i;

                for (ne_i node = m_nodes.begin(); node != m_nodes.end(); ++node){
                    // deallocate all edge entries
                    for (ee_i edge = (node->second)->m_edges.begin(); edge != (node->second)->m_edges.end(); ++edge){
                        delete edge->second;
                        edge->second = NULL;
                    }
                    (node->second)->m_edges.clear();

                    delete node->second;
                    node->second = NULL;
                }

                m_nodes.clear();
                m_last_edge_id = 0;
                m_last_node_id = 0;
            }

        public:

            //! \brief create empty graph
            graph(const node_value_comparator & nc = node_value_comparator(), const edge_value_comparator & ec = edge_value_comparator())
                :m_last_node_id(0), m_last_edge_id(0), m_node_comparator(nc), m_edge_comparator(ec)
            {
                // no idea what to do here...
            }

            //! \copy constructor, allows to override default comparators
            graph(const graph_type & original, const node_value_comparator & nc = node_value_comparator(), const edge_value_comparator & ec = edge_value_comparator())
                :m_last_node_id(0), m_last_edge_id(0), m_node_comparator(nc), m_edge_comparator(ec)
            {
                operator=(original);
            }

            virtual ~graph(){
                reset();
                // absolutely no idea what to do here
            }

            const node_value_comparator & get_node_value_comparator() const { return m_node_comparator; }
            const edge_value_comparator & get_edge_value_comparator() const { return m_edge_comparator; }

            /**
             * \brief create a new node
             * \return handle to newly created node
             */
            node_iterator create_node() throw () {
                node_entry * node = new node_entry(++m_last_node_id);
                m_nodes.insert(typename node_entry_map::value_type(node->m_node_id, node));
                node_handle tmp(this, node->m_node_id);
                return node_iterator(tmp);
            }

            /**
             * \brief create a new node with given value
             * \param value - value for the new node to hold
             * \return handle to newly created node
             */
            node_iterator create_node(const node_value_type & value) throw () {
                node_entry * node = new node_entry(++m_last_node_id, value);
                m_nodes.insert(typename node_entry_map::value_type(node->m_node_id, node));
                node_handle tmp(this, node->m_node_id);
                return node_iterator(tmp);
            }

            /**
             * \brief get read-write iterator to node
             * \param node_id - id of node to get iterator to
             * \return iterator to node or \ref nodes_end if no such node exists
             */
            node_iterator get_node(const node_id_t node_id) throw () {
                if (m_nodes.find(node_id) != m_nodes.end()){
                    node_handle tmp(get_node_handle(node_id));
                    return node_iterator(tmp);
                } else {
                    return nodes_end();
                }
            }

            /**
             * \brief get read-only iterator to node
             * \param node_id - id of node to get iterator to
             * \return const iterator to node or \ref nodes_end if no such node exists
             */
            const_node_iterator get_node(const node_id_t node_id) const throw () {
                if (m_nodes.find(node_id) != m_nodes.end()){
                    node_handle tmp(get_const_node_handle(node_id));
                    return const_node_iterator(tmp);
                } else {
                    return nodes_end();
                }
            }

            /**
             * \brief get read-write iterator to edge
             * \param from - id of node from which the edge originates
             * \param to - id of node to which the edge leads
             * \return iterator to edge or \ref edges_end if no such edge exists
             */
            edge_iterator get_edge(const node_id_t from, const node_id_t to) throw () {
                typename node_entry_map::iterator node = m_nodes.find(from);
                if (node == m_nodes.end()){ return edges_end(); }

                edge_target_predicate predicate(to);

                typename edge_entry_map::iterator edge = std::find_if(node->second.m_edges.begin(), node->second.m_edges.end(), predicate);
                if (edge == node->second.m_edges.end()){ return edges_end(); }

                return edge_iterator(edge_handle(this, node->first, edge->first));
            }

            /**
             * \brief get read-only iterator to edge
             * \param from - id of node from which the edge originates
             * \param to - id of node to which the edge leads
             * \return const iterator to edge or \ref edges_end if no such edge exists
             */
            const_edge_iterator get_edge(const node_id_t from, const node_id_t to) const throw () {
                typename node_entry_map::const_iterator node = m_nodes.find(from);
                if (node == m_nodes.end()){ return edges_end(); }

                edge_target_predicate predicate(to);

                typename edge_entry_map::const_iterator edge = std::find_if(node->second.m_edges.begin(), node->second.m_edges.end(), predicate);
                if (edge == node->second.m_edges.end()){ return edges_end(); }

                return const_edge_iterator(get_const_edge_handle(this, node->first, edge->first));
            }

            /**
             * \brief create an edge between two nodes
             * \param from - handle of the origin node
             * \param to - handle of the target node
             * \throw libkerat::exception::invalid_graph_component_error when
             * no such node exists
             * \return read-write handle to newly created edge
             */
            edge_iterator create_edge(const node_handle & from, const node_handle & to)
                throw (libkerat::exception::invalid_graph_component_error)
            {
                // get target nodes
                node_entry & node_from = check_for_node(from.get_node_id());
                node_entry & node_to = check_for_node(to.get_node_id());

                // create forward edge record
                edge_id_t this_edge_id = ++m_last_edge_id;
                edge_entry * edge = new edge_entry(node_to.m_node_id);
                node_from.m_edges.insert(typename edge_entry_map::value_type(this_edge_id, edge));

                // create backward edge record
                node_to.m_neighbours.insert(node_from.m_node_id);

                edge_handle tmp(this, from.get_node_id(), this_edge_id);
                return edge_iterator(tmp);
            }

            /**
             * \brief create an edge between two nodes holding given value
             * \param from - handle of the origin node
             * \param to - handle of the target node
             * \param value - value for the newly created edge
             * \throw libkerat::exception::invalid_graph_component_error when
             * no such node exists
             * \return read-write handle to newly created edge
             */
            edge_iterator create_edge(const node_handle & from, const node_handle & to, const edge_value_type & value)
                throw (libkerat::exception::invalid_graph_component_error)
            {
                // get target nodes
                node_entry & node_from = check_for_node(from.get_node_id());
                node_entry & node_to = check_for_node(to.get_node_id());

                // create forward edge record
                edge_id_t this_edge_id = ++m_last_edge_id;
                edge_entry * edge = new edge_entry(node_to.m_node_id, value);
                node_from.m_edges.insert(typename edge_entry_map::value_type(this_edge_id, edge));

                // create backward edge record
                node_to.m_neighbours.insert(node_from.m_node_id);
                edge_handle tmp(this, from.get_node_id(), this_edge_id);

                return edge_iterator(tmp);
            }

            /**
             * \brief remove node from the graph
             * Removes given node from the graph, removing all corresponding edges
             * \param node - handle to the node being removed
             * \return true if such node was removed, else otherwise
             */
            bool remove_node(const node_handle & node) throw () {
                node_id_t node_id = node.get_node_id();

                typename node_entry_map::iterator found = m_nodes.find(node_id);
                if (found != m_nodes.end()){

                    // erase forward edges
                    for (
                        typename edge_entry_map::iterator i = found->second->m_edges.begin();
                        i != found->second->m_edges.end();
                        i++
                    ){
                        // erase backward references
                        typename node_entry_map::iterator found_tmp = m_nodes.find(i->second->m_target_id);
                        if (found_tmp != m_nodes.end()){
                            found_tmp->second->m_neighbours.erase(node_id);
                        }
                        delete i->second;
                        i->second = NULL;
                    }
                    found->second->m_edges.clear();

                    // erase backward edges
                    for (
                        typename node_entry::neighbour_set::iterator neighbour = found->second->m_neighbours.begin();
                        neighbour != found->second->m_neighbours.end();
                        neighbour++
                    ){
                        typename node_entry_map::iterator found_tmp = m_nodes.find(*neighbour);
                        if (found_tmp == m_nodes.end()){ continue; }

                        typedef std::set<edge_id_t> edge_set;
                        edge_set edges_to_erase;

                        // identify edges to be erased
                        for (
                            typename edge_entry_map::const_iterator i = found_tmp->second->m_edges.begin();
                            i != found_tmp->second->m_edges.end();
                            i++
                        ){
                            if (i->second->m_target_id == node_id){ edges_to_erase.insert(i->first); }
                        }

                        // erase reference
                        for (edge_set::const_iterator i = edges_to_erase.begin(); i != edges_to_erase.end(); i++){
                            delete found_tmp->second->m_edges[*i];
                            found_tmp->second->m_edges[*i] = NULL;
                            found_tmp->second->m_edges.erase(*i);
                        }
                    }
                    found->second->m_neighbours.clear();

                    m_nodes.erase(found);
                    return true;
                }

                return false;
            }

            /**
             * \brief remove edge from the graph
             * \param edge - handle to the edge being removed
             * \return true if such edge was removed, else otherwise
             */
            bool remove_edge(const edge_handle & edge) throw () {
                bool found = false;

                try {
                    node_id_t from_node_id = edge.m_from_node_id;
                    node_entry & from_node = check_for_node(from_node_id);

                    // this line is here just to perform check to make sure that the edge exists
                    node_id_t to_node_id = from_node.check_for_edge(edge.m_edge_id).m_target_id;

                    delete from_node.m_edges[edge.m_edge_id];
                    from_node.m_edges[edge.m_edge_id] = NULL;
                    from_node.m_edges.erase(edge.m_edge_id);
                    // from now on, the edge is invalid

                    // scan the target for backward reference and possibly erase
                    found = false;
                    for (typename edge_entry_map::const_iterator i = from_node.m_edges.begin(); ((!found) && (i != from_node.m_edges.end())); ++i){
                        found |= (i->second->m_target_id == to_node_id);
                    }
                    if (!found){
                        node_entry & to_node = check_for_node(to_node_id);
                        to_node.m_neighbours.erase(from_node_id);
                    }

                } catch (libkerat::exception::invalid_graph_component_error ex){
                    // no such edge exists
                }

                return found;
            }

            //! \brief Checks whether the graph has nodes
            inline bool empty() const throw () { return m_nodes.empty(); }

            //! \brief Makes the graph empty
            void clear() throw () {
                m_nodes.clear();
            }

            /**
             * \brief get end iterator to the nodes held in this graph
             * \return always-past-the-end iterator
             */
            node_iterator nodes_end() throw () {
                return node_iterator(this);
            }

            /**
             * \brief get read-only end iterator to the nodes held in this graph
             * \return always-past-the-end iterator
             */
            const_node_iterator nodes_end() const throw () {
                return const_node_iterator(this);
            }

            /**
             * \brief get read-write iterator to the nodes held in this graph
             * \return iterator to first node of this graph or \ref nodes_end if empty
             */
            node_iterator nodes_begin() throw () {
                if (m_nodes.empty()){
                    return nodes_end();
                } else {
                    node_handle tmp(get_node_handle(m_nodes.begin()->first));
                    return node_iterator(tmp);
                }
            }

            /**
             * \brief get read-only iterator to the nodes held in this graph
             * \return iterator to first node of this graph or \ref nodes_end if empty
             */
            const_node_iterator nodes_begin() const throw () {
                if (m_nodes.empty()){
                    return nodes_end();
                } else {
                    node_handle tmp(get_const_node_handle(m_nodes.begin()->first));
                    return const_node_iterator(tmp);
                }
            }

            /**
             * \brief get count of nodes in this graph
             * \return count of nodes
             */
            size_t nodes_count() const throw () { return m_nodes.size(); }


            /**
             * \brief get end iterator for edges held in this graph as whole edge set
             * \note This is the actual edges end, do not use for iterating over
             * edges of individual nodes
             * \return always-past-the-end edge iterator
             */
            edge_iterator edges_end() throw () {
                return edge_iterator(this);
            }

            /**
             * \brief get read-only end iterator for edges held in this graph as whole edge set
             * \note This is the actual edges end, do not use for iterating over
             * edges of individual nodes
             * \return always-past-the-end const edge iterator
             */
            const_edge_iterator edges_end() const throw () {
                return const_edge_iterator(this);
            }

            /**
             * \brief get iterator to the first edge held in this graph as whole edge set
             * \note This is the actual edges begin, do not use for iterating over
             * edges of individual nodes
             * \return iterator to first edge or \ref edges_end if no edges exist in this graph
             */
            edge_iterator edges_begin() throw () {
                if (m_nodes.empty()){
                    return edges_end();
                } else {
                    return get_node_handle(m_nodes.begin()->first).edges_begin();
                }
            }

            /**
             * \brief get read-only iterator to the first edge held in this graph as whole edge set
             * \note This is the actual edges begin, do not use for iterating over
             * edges of individual nodes
             * \return const iterator to first edge or \ref edges_end if no edges exist in this graph
             */
            const_edge_iterator edges_begin() const throw () {
                if (m_nodes.empty()){
                    return edges_end();
                } else {
                    return get_const_node_handle(m_nodes.begin()->first).edges_begin();
                }
            }

            //! \brief get count of edges in this graph
            size_t edges_count() const throw () {
                size_t retval = 0;
                for (typename node_entry_map::const_iterator node = m_nodes.begin(); node != m_nodes.end(); node++){
                    retval += node->second->m_edges.size();
                }
                return retval;
            }

            /**
             * \brief check whether the given iterator belongs to this graph
             * \return true if belongs
             */
            bool iterator_belongs(const const_node_iterator & node_iter) const throw() {
                return node_iter.m_graph == this;
            }

            /**
             * \brief check whether the given iterator belongs to this graph
             * \return true if belongs
             */
            bool iterator_belongs(const const_edge_iterator & edge_iter) const throw() {
                return edge_iter.m_graph == this;
            }

            //! \todo Make isomorphy based operator== for graphs
            bool operator==(const graph_type & second) const throw () {
                return graph_compare(*this, second, m_node_comparator, m_edge_comparator) == 0;
            }

            inline bool operator!=(const graph_type & second) const throw () { return !operator==(second); }

            graph_type & operator=(const graph_type & second){
                reset();

                m_last_edge_id = second.m_last_edge_id;
                m_last_node_id = second.m_last_node_id;

                typedef typename node_entry_map::const_iterator ne_i;
                typedef typename edge_entry_map::const_iterator ee_i;

                // copy nodes
                for (ne_i node = second.m_nodes.begin(); node != second.m_nodes.end(); ++node){
                    node_entry * new_node = new node_entry(node->first, (node->second)->m_value);
                    m_nodes[new_node->m_node_id] = new_node;

                    // copy outcoming edges
                    for (ee_i edge = (node->second)->m_edges.begin(); edge != (node->second)->m_edges.end(); ++edge){
                        edge_entry * new_edge = new edge_entry((edge->second)->m_target_id, (edge->second)->m_value);
                        new_node->m_edges[edge->first] = new_edge;
                    }

                    // copy neighbour map
                    new_node->m_neighbours = (node->second)->m_neighbours;
                }

                return *this;
            }

        public:

            //! \brief predicate for by-value node search
            class node_value_predicate: public std::unary_function<const node_handle &, bool> {
            public:
                node_value_predicate(const node_value_type & val):value_to_compare(val){ ; }

                bool operator()(const node_handle & node) const { return node.get_value() == value_to_compare; }

            private:
                const node_value_type & value_to_compare;
            };

            //! \brief predicate for by-value edge search
            class edge_value_predicate: public std::unary_function<const edge_handle &, bool> {
            public:
                edge_value_predicate(const edge_value_type & val):value_to_compare(val){ ; }

                bool operator()(const edge_handle & edge) const { return edge.get_value() == value_to_compare; }
            private:
                const edge_value_type & value_to_compare;
            };

            //! \brief predicate for by-target edge search
            class edge_target_predicate: public std::unary_function<const edge_handle &, bool> {
            public:
                edge_target_predicate(const node_id_t target):target_to_compare(target){ ; }

                bool operator()(const edge_handle & edge) const { return edge.to_node()->get_node_id() == target_to_compare; }
            private:
                const node_id_t target_to_compare;
            };

            //! \brief predicate for by-origin edge search
            class edge_origin_predicate: public std::unary_function<const edge_handle &, bool> {
            public:
                edge_origin_predicate(const node_id_t origin):origin_to_compare(origin){ ; }

                bool operator()(const edge_handle & edge) const { return edge.from_node()->get_node_id() == origin_to_compare; }
            private:
                const node_id_t origin_to_compare;
            };

        };

        template<class GRAPH>
        struct graph_dummy {
            typedef graph<
                typename GRAPH::node_iterator,
                typename GRAPH::edge_iterator
            > normal_dummy;

            typedef graph<
                typename GRAPH::const_node_iterator,
                typename GRAPH::const_edge_iterator
            > const_dummy;
        };

        /**
         * \brief Provides the core graph utility functions.
         * \note You should not use these directly, better write wrappers for this functions.
         * \todo Add graph isomorphy detection
         * \todo Add graph component counter
         */
        class graph_utils {
        public:
            template <typename NODE_ITERATOR>
            struct node_iterator_comparator: public std::binary_function<const NODE_ITERATOR &, const NODE_ITERATOR &, bool> {
                bool operator()(const NODE_ITERATOR & first, const NODE_ITERATOR & second) const {
                    return compare(first, second) < 0;
                }

                static int compare(const NODE_ITERATOR & first, const NODE_ITERATOR & second) {
                    // just to be on the safe side
                    assert(first->get_parent_graph() != NULL);
                    assert(second->get_parent_graph() != NULL);
                    assert(&(*first) != NULL);
                    assert(&(*second) != NULL);
                    
                    if (first->get_node_id() < second->get_node_id()){
                        return -1;
                    } else if (second->get_node_id() > first->get_node_id()){
                        return 1;
                    }
                    return 0;
                }
            };

            template <typename GRAPH_TYPE>
            static typename graph_dummy<GRAPH_TYPE>::normal_dummy create_normal_dummy(GRAPH_TYPE & graf);

            template <typename GRAPH_TYPE>
            static typename graph_dummy<GRAPH_TYPE>::const_dummy create_const_dummy(const GRAPH_TYPE & graf);

            template <typename GRAPH_TYPE>
            static std::list<GRAPH_TYPE> split_components_copy_1_0(const GRAPH_TYPE & original_graph);

            template <typename GRAPH_TYPE>
            static std::list<GRAPH_TYPE> split_strong_components_copy_1_0(const GRAPH_TYPE & original_graph);

            template <typename GRAPH_TYPE, typename NODE_COMPARATOR, typename EDGE_COMPARATOR>
            static int compare_graphs(
                const GRAPH_TYPE & first_graph,
                const GRAPH_TYPE & second_graph,
                const NODE_COMPARATOR & node_comparator,
                const EDGE_COMPARATOR & edge_comparator
            );

            template <typename GRAPH_DUMMY_TYPE, typename NODE_COMPARATOR, typename EDGE_COMPARATOR>
            static int compare_graph_component(
                GRAPH_DUMMY_TYPE & component1,
                GRAPH_DUMMY_TYPE & component2,
                const NODE_COMPARATOR & node_comparator,
                const EDGE_COMPARATOR & edge_comparator
            );




            //! \brief Checks for existence of oriented cycles
            template <typename Node, typename Edge>
            static bool oriented_contains_cycle(const libkerat::internals::graph<Node, Edge> & grph);

            //! \brief Checks for existence of unoriented cycles
            template <typename Node, typename Edge>
            static bool contains_cycle_unoriented(const libkerat::internals::graph<Node, Edge> & grph){
                if (grph.empty()){ return false; }

                typedef libkerat::internals::graph<Node, Edge> graph_type;

                typedef std::set<node_id_t> nodeid_set;
                typedef std::list<unoriented_node_note> node_list;

                typename graph_type::node_entry_map tmp_nodes(grph.m_nodes);

                // since there might be separated components
                nodeid_set component_nodes;
                while (!tmp_nodes.empty()){
                    node_list current_path;

                    {
                        unoriented_node_note tmp;
                        tmp.node_id = tmp_nodes.begin()->first;
                        tmp.edge_id = 0;
                        tmp.neighbour_id = 0;
                        current_path.push_back(tmp);
                    }

                    while (!current_path.empty()) {

                        unoriented_node_note current_node = current_path.back();
                        current_path.pop_back();

                        unoriented_node_note previous_node;
                        bool previous_node_valid = false;
                        if (!current_path.empty()){
                            previous_node = current_path.back();
                            previous_node_valid = true;
                        }

                        typename graph_type::node_entry_map::const_iterator current_node_iter = tmp_nodes.find(current_node.node_id);

                        if (current_node_iter == tmp_nodes.end()){
                            // node about to be processed belongs to another component
                            continue;
                        }

                        if (current_node.edge_id == 0){
                            bool nid_found = false;
                            if (component_nodes.find(current_node.node_id) != component_nodes.end()) {
                                // node was already processed and is claimed again as new
                                // this means a cycle was found
                                return true;
                            }
                        }

                        component_nodes.insert(current_node.node_id);

                        if (current_node.neighbour_id == 0){
                            ++current_node.edge_id;

                            typedef std::pair<
                                typename graph_type::edge_entry_map::const_iterator,
                                typename graph_type::edge_entry_map::const_iterator
                            > next_edge_iter_pair;

                            next_edge_iter_pair next_edge = current_node_iter->second->m_edges.equal_range(current_node.edge_id);

                            if (next_edge.first != current_node_iter->second->m_edges.end()){
                                current_node.edge_id = next_edge.first->first;
                                current_path.push_back(current_node);

                                bool find_next = false;
                                if ((previous_node_valid) && ((next_edge.first->second->m_target_id) != previous_node.node_id)){
                                    find_next = true;
                                }

                                if (!previous_node_valid || find_next){
                                    unoriented_node_note next_node;
                                    next_node.node_id = next_edge.first->second->m_target_id;
                                    next_node.edge_id = 0;
                                    next_node.neighbour_id = 0;
                                    current_path.push_back(next_node);
                                }
                                continue;
                            } else {
                                ++current_node.neighbour_id;
                            }
                        }

                        // since the mode is switched in previous if, no else belongs here!
                        // this should be always true due to continue statement in previous if
                        if (current_node.neighbour_id != 0){

                            typedef std::pair<
                                typename graph_type::node_entry::neighbour_set::const_iterator,
                                typename graph_type::node_entry::neighbour_set::const_iterator
                            > next_neighbour_iter_pair;

                            next_neighbour_iter_pair next_neighbour = current_node_iter->second->m_neighbours.equal_range(current_node.neighbour_id);

                            if (next_neighbour.first != current_node_iter->second->m_neighbours.end()){
                                current_node.neighbour_id = *(next_neighbour.first);
                                ++current_node.neighbour_id;
                                current_path.push_back(current_node);

                                bool find_next = false;
                                // prevent claiming the parent edge as cycle
                                if ((previous_node_valid) && (*(next_neighbour.first) != previous_node.node_id)){
                                    find_next = true;
                                }

                                if (find_next || !previous_node_valid){
                                    unoriented_node_note next_node;
                                    next_node.node_id = *(next_neighbour.first);
                                    next_node.edge_id = 0;
                                    next_node.neighbour_id = 0;
                                    current_path.push_back(next_node);
                                }
                                continue;
                            }
                        }

                    };

                    for (nodeid_set::const_iterator i = component_nodes.begin(); i != component_nodes.end(); i++){
                        tmp_nodes.erase(*i);
                    }

                }

                return false;
            }

            //! \brief Counts nodes with given value
            template <typename Node, typename Edge>
            static size_t count_node_value(const libkerat::internals::graph<Node, Edge> & grph, const typename libkerat::internals::graph<Node, Edge>::Node_type & value){
                typename libkerat::internals::graph<Node, Edge>::node_value_predicate predicate(value);
                return std::count_if(grph.nodes_begin(), grph.nodes_end(), predicate);
            }

            //! \brief Checks for existence of nodes with the same value
            template <typename Node, typename Edge>
            static bool has_duplicit_nodes(const libkerat::internals::graph<Node, Edge> & grph){
                typedef typename libkerat::internals::graph<Node, Edge>::const_node_iterator const_node_iterator;
                for (const_node_iterator node = grph.nodes_begin(); node != grph.nodes_end(); ){
                    typename libkerat::internals::graph<Node, Edge>::node_value_predicate predicate(node->get_value());
                    ++node;
                    if (std::find_if(node, grph.nodes_end(), predicate) != grph.nodes_end()){ return true; }
                }
                return false;
            }

            //! \brief Checks whether given node has output degree 0 and input degree 1
            template <typename Node>
            static bool is_inout_leaf(const Node node) throw () {
                return ((node->output_degree() == 0) && (node->input_degree() == 1));
            }

            //! \brief Checks whether given node has output degree 1 and input degree 0
            template <typename Node>
            static bool is_outin_leaf(const Node node) throw () {
                return ((node->output_degree() == 1) && (node->input_degree() == 0));
            }

            /**
             * \brief Attempts to find node with output degree 0 and input degree 1 in given range
             * \param n_begin - node interval begin
             * \param n_end - past-the-end interval iterator
             * \return n_end if not found or valid iterator from [n_begin; n_end) range
             * \throws libkerat::exception::invalid_graph_component_error when given iterators
             * do not belong to the same graph
             */
            template <typename Node>
            static Node get_oriented_inout_leaf(Node n_begin, Node n_end)
                throw (libkerat::exception::invalid_graph_component_error)
            {

                typedef typename Node::graph_type graph_type;
                typedef typename graph_type::const_iterator const_iterator;
                typedef typename graph_type::const_node_iterator const_node_iterator;

                const graph_type * grph = n_begin->get_parent_graph();
                assert(grph != NULL);

                if (!grph->iterator_belongs(n_end)){
                    throw libkerat::exception::invalid_graph_component_error(
                        "Cross-graph node interval given!",
                        libkerat::exception::invalid_graph_component_error::NODE
                    );
                }

                // find node with no predecessor and one descendent
                for (const_node_iterator node = n_begin; node != n_end; node++){
                    if (is_inout_leaf(const_node_iterator(node))) { return node; }
                }

                return n_end;
            }

            /**
             * \brief Attempts to find node with output degree 1 and input degree 0 in given range
             * \param n_begin - node interval begin
             * \param n_end - past-the-end interval iterator
             * \return n_end if not found or valid iterator from [n_begin; n_end) range
             * \throws libkerat::exception::invalid_graph_component_error when given iterators
             * do not belong to the same graph
             */
            template <typename Node>
            static Node get_oriented_outin_leaf(Node n_begin, Node n_end)
                throw (libkerat::exception::invalid_graph_component_error)
            {
                typedef typename Node::graph_type graph_type;
                typedef typename graph_type::node_entry_map::const_iterator const_iterator;
                typedef typename graph_type::const_node_iterator const_node_iterator;

                if (n_begin == n_end){ return n_end; }

                const graph_type * grph = n_begin->get_parent_graph();
                assert(grph != NULL);

                if (!grph->iterator_belongs(n_end)){
                    throw libkerat::exception::invalid_graph_component_error(
                        "Cross-graph node interval given!",
                        libkerat::exception::invalid_graph_component_error::NODE
                    );
                }

                // find node with no predecessor and one descendent
                for (Node node = n_begin; node != n_end; node++){
                    if (is_outin_leaf(const_node_iterator(node))) { return node; }
                }

                return n_end;
            }

            // \brief Same as \ref get_oriented_outin_leaf, only operates on whole graph
            template <typename Node, typename Edge>
            static typename libkerat::internals::graph<Node, Edge>::const_node_iterator get_origin_leaf(const libkerat::internals::graph<Node, Edge> & grph){
                typedef typename libkerat::internals::graph<Node, Edge> graph_type;
                typedef typename graph_type::node_entry_map::const_iterator const_iterator;
                typedef typename graph_type::const_node_iterator const_node_iterator;

                // find node with no predecessor and one descendent
                for (const_iterator node = grph.m_nodes.begin(); node != grph.m_nodes.end(); node++){
                    // whether has predecesor
                    if (!node->second->m_neighbours.empty()){ continue; }
                    // whether has children
                    if (node->second->m_edges.size() != 1){ continue; }
                    // leaf found!
                    return const_node_iterator(grph.get_node(node->second->m_node_id));
                }

                return grph.nodes_end();
            }

            // \brief Same as \ref get_oriented_inout_leaf, only operates on whole graph
            template <typename Node, typename Edge>
            static typename libkerat::internals::graph<Node, Edge>::const_node_iterator get_end_leaf(const libkerat::internals::graph<Node, Edge> & grph){
                typedef typename libkerat::internals::graph<Node, Edge> graph_type;
                typedef typename graph_type::node_entry_map::const_iterator const_iterator;
                typedef typename graph_type::const_node_iterator const_node_iterator;

                // find node with no predecessor and one descendent
                for (const_iterator node = grph.m_nodes.begin(); node != grph.m_nodes.end(); node++){
                    // whether has predecesor
                    if (node->second->m_neighbours.size() != 1){ continue; }
                    // whether has children
                    if (!node->second->m_edges.empty()){ continue; }
                    // leaf found!
                    return const_node_iterator(grph.get_node(node->second->m_node_id));
                }

                return grph.nodes_end();
            }

            // \brief Attempts to find central node of star topology presuming the center has non-zero output degree
            template <typename Node, typename Edge>
            static typename libkerat::internals::graph<Node, Edge>::const_node_iterator topology_star_get_oriented_inout_center(
                const libkerat::internals::graph<Node, Edge> & grph
            ){
                typedef typename libkerat::internals::graph<Node, Edge> graph_type;
                typedef typename graph_type::node_entry_map::const_iterator const_iterator;
                typedef typename graph_type::const_node_iterator const_node_iterator;

                size_t edges_count = grph.edges_count();
                size_t nodes_count = grph.nodes_count();
                // star topology has |V| = |E|+1
                if (nodes_count != (edges_count + 1)){ grph.nodes_end(); }

                typename libkerat::internals::graph<Node, Edge>::const_node_iterator center = grph.nodes_end();

                // find node with no predecessor and one descendent
                for (const_iterator node = grph.m_nodes.begin(); node != grph.m_nodes.end(); node++){
                    size_t neighbours_size = node->second->m_neighbours.size();
                    if (neighbours_size == 0){
                        size_t children_size = node->second->m_edges.size();
                        if (children_size == edges_count){
                            center = const_node_iterator(grph.get_node(node->first));
                        } else {
                            grph.nodes_end();
                        }
                    } else if (neighbours_size != 1){
                        // neither center, nor leaf, what the fuck is this?
                        return grph.nodes_end();
                    }
                }

                return center;
            }

            // \brief Attempts to find central node of star topology presuming the center has non-zero input degree
            template <typename Node, typename Edge>
            static typename libkerat::internals::graph<Node, Edge>::const_node_iterator topology_star_get_oriented_outin_center(
               const libkerat::internals::graph<Node, Edge> & grph
            ){
                typedef typename libkerat::internals::graph<Node, Edge> graph_type;
                typedef typename graph_type::node_entry_map::const_iterator const_iterator;

                size_t edges_count = grph.edges_count();
                size_t nodes_count = grph.nodes_count();
                // star topology has |V| = |E|+1
                if (nodes_count != (edges_count + 1)){ grph.nodes_end(); }

                typename libkerat::internals::graph<Node, Edge>::const_node_iterator center = grph.nodes_end();

                // find node with no predecessor and one descendent
                for (const_iterator node = grph.m_nodes.begin(); node != grph.m_nodes.end(); node++){
                    size_t children_size = node->second->m_edges.size();
                    if (children_size == 0){
                        size_t neighbours_size = node->second->m_neighbours.size();
                        if (neighbours_size != edges_count){
                            center = const_node_iterator(grph.get_node(node->first));
                        } else {
                            grph.end();
                        }
                    } else if (children_size != 1){
                        // neither center, nor leaf, what the fuck is this?
                        return grph.end();
                    }
                }

                return center;
            }

            //! \brief See \ref graph_topology_is_linear_oriented
            template <typename Node, typename Edge>
            static bool topology_is_linear_oriented(const libkerat::internals::graph<Node, Edge> & grph){

                if (grph.nodes_count() != (grph.edges_count() + 1)){ return false; }

                typedef typename libkerat::internals::graph<Node, Edge> graph_type;
                typedef typename graph_type::const_node_iterator const_node_iterator;

                const_node_iterator origin_leaf = get_origin_leaf(grph);
                const_node_iterator end_leaf = get_end_leaf(grph);

                // origin leaf was not found
                if (origin_leaf == grph.nodes_end()){ return false; }
                // end leaf was not found
                if (end_leaf == grph.nodes_end()){ return false; }

                size_t visited_nodes_count = 2;

                const_node_iterator current_node = const_node_iterator(origin_leaf->edges_begin()->to_node());

                while (current_node != end_leaf){
                    // does not have 1 predecesor, there for is not linear
                    if (current_node->input_degree() != 1){ return false; }
                    // has more descendetns, there for is not linear
                    if (current_node->output_degree() != 1){ return false; }

                    current_node = const_node_iterator(current_node->edges_begin()->to_node());
                    ++visited_nodes_count;
                }

                // somehow, not all nodes were visited
                if (visited_nodes_count != grph.nodes_count()){ return false; }

                return true;
            }

            /**
             * \brief Removes duplicit edges from given graph ignoring the edge value
             * \param grph - graph to remove edges from
             * \return graph with at most one edge between any two nodes
             */
            template <typename Node, typename Edge>
            static libkerat::internals::graph<Node, Edge> remove_duplicit_edges(libkerat::internals::graph<Node, Edge> grph){
                typedef typename libkerat::internals::graph<Node, Edge> graph_type;

                for (typename graph_type::node_entry_map::const_iterator node = grph.m_nodes.begin(); node != grph.m_nodes.end(); node++){
                    typename graph_type::node_entry & ne = node->second;
                    typedef typename graph_type::node_entry::neighbour_set neighbour_set;
                    typedef typename graph_type::edge_entry_map edge_map;
                    neighbour_set targets;

                    for (typename edge_map::iterator e = ne.m_edges.begin(); e != ne.m_edges.end();){
                        typename edge_map::iterator next = e;
                        ++next;

                        if (targets.find(e->second.m_target_id) != targets.end()){
                            ne.m_edges.erase(e);
                        } else {
                            targets.insert(e->second.m_target_id);
                        }

                        e = next;
                    }
                }

                return grph;
            }

            /**
             * \brief Split components of the given graph
             * \param grph - graph whose components shall be split into individual graphs
             * \return list of graphs, one graph per topology
             */
            template <typename Node, typename Edge>
            static std::list<libkerat::internals::graph<Node, Edge> > split_components(const libkerat::internals::graph<Node, Edge> & original_graph){

                // this graph now contains all possible edges between nodes
                libkerat::internals::graph<Node, Edge> grph = graph_grow(remove_duplicit_edges(original_graph));

                typedef libkerat::internals::graph<Node, Edge> graph_type;
                typedef typename graph_type::node_entry node_entry;
                typedef typename graph_type::node_entry::edges_map::const_iterator ceiter;
                typedef typename node_entry::neighbour_set neighbour_set;
                typedef typename neighbour_set::const_iterator cniter;
                neighbour_set remaining;

                std::list<graph_type> retval;

                while (!remaining.empty()){
                    std::set<node_id_t> nodes_to_process;
                    nodes_to_process.insert(*remaining.begin());
                    graph_type tmp_graph;

                    while (!nodes_to_process.empty()){
                        node_id_t node_id = *nodes_to_process.begin();
                        nodes_to_process.erase(node_id);
                        remaining.erase(node_id);

                        const node_entry & node = grph.m_nodes[node_id];
                        typename graph_type::node_entry_map::const_iterator original_node = original_graph.m_nodes.find(node_id);
                        assert(original_node != original_graph.m_nodes.end());

                        tmp_graph.m_nodes.insert(*original_node);

                        // push nodes that we point to
                        for (ceiter i = node.m_edges.begin(); i != node.m_edges.end(); i++){
                            if (remaining.find(i->second.to_node) != remaining.end()){
                                nodes_to_process.insert(i->second.to_node);
                            }
                        }
                        // push nodes that we are point to by
                        for (cniter i = node.m_neighbours.begin(); i != node.m_neighbours.end(); i++){
                            if (remaining.find(*i) != remaining.end()){
                                nodes_to_process.insert(*i);
                            }
                        }
                    }
                    retval.push_back(tmp_graph);
                }

                return retval;
            }


            /**
             * \brief Adds all edges implied by transitivity
             * \note New edges have the default value of edge value type
             * \param grph - graph to grow
             * \return grph with all transitivity-implied edges filled in
             */
            template <typename Node, typename Edge>
            static libkerat::internals::graph<Node, Edge> graph_grow(libkerat::internals::graph<Node, Edge> grph){
                typedef libkerat::internals::graph<Node, Edge> graph_type;
                typedef typename graph_type::node_entry_map::iterator niter;

                for (niter i = grph.m_nodes.begin(); i != grph.m_nodes.end(); i++){
                    for (niter j = grph.m_nodes.begin(); j != grph.m_nodes.end(); j++){
                        if (grph.get_edge(i->first, j->first) != grph.nodes_end()){ continue; }

                        for (niter k = grph.m_nodes.begin(); k != grph.m_nodes.end(); k++){
                            if (grph.get_edge(i->first, k->first) == grph.nodes_end()){ continue; }
                            if (grph.get_edge(k->first, j->first) == grph.nodes_end()){ continue; }

                            grph.create_edge(i->first, j->first);
                        }
                    }
                }

                return grph;
            }

            template <typename GRAPH_DUMMY_TYPE, typename NODE_COMPARATOR, typename EDGE_COMPARATOR>
            static void graph_component_reorder(
                GRAPH_DUMMY_TYPE & component,
                const NODE_COMPARATOR & node_comparator,
                const EDGE_COMPARATOR & edge_comparator
            );

            //! \brief structure that serves as comparator of graph components based on node and edges count
            template <class DUMMY>
            struct primitive_graph_component_comparator: public std::binary_function<const DUMMY &, const DUMMY &, bool> {
                bool operator()(const DUMMY & d1, const DUMMY & d2) const { return compare(d1, d2) < 0; }
                int compare(const DUMMY & d1, const DUMMY & d2) const {
                    size_t n1 = d1.nodes_count(); size_t n2 = d2.nodes_count();
                    if (n1 < n2){ return -1; } else if (n2 < n1){ return 1; }
                    size_t e1 = d1.edges_count(); size_t e2 = d2.edges_count();
                    if (e1 < e2){ return -1; } else if (e2 < e1){ return 1; }
                    return 0;
                }
            };

        private:

            struct scc_tarjan_entry {
                size_t index;
                size_t lowlink;
            };
            template <typename ITERATOR>
            struct scc_dfs_entry {
                scc_dfs_entry():finished(false){ ; }

                ITERATOR to_process;
                ITERATOR parent;
                bool finished;
            };

            // map the node id's to components they belong to
            typedef std::map<node_id_t, size_t> component_mapping;

            struct node_trace {
                node_id_t node_id;
                node_id_t edge_id;
            };
            struct unoriented_node_note: public node_trace {
                node_id_t neighbour_id;
            };

            static bool compare_by_node_id(const node_trace & first, const node_trace & second){
                return first.node_id == second.node_id;
            }


            //! \brief compares two dummy graph edges by their real target value
            template <
                typename DUMMY_EDGE_ITERATOR,
                typename EDGE_COMPARATOR
            >
            class dummy_edge_value_comparator {
            public:
                dummy_edge_value_comparator(const EDGE_COMPARATOR & comparator):m_comparator(comparator){ ; }

                bool operator()(const DUMMY_EDGE_ITERATOR & i1, const DUMMY_EDGE_ITERATOR & i2) const {
                    return m_comparator(*i1, *i2);
                }

            private:
                const EDGE_COMPARATOR & m_comparator;
            };

            //! \brief compares two graph nodes by their real target value and degrees
            template <
                typename NODE_HANDLE,
                typename NODE_COMPARATOR
            >
            class node_handle_sort_comparator: public std::binary_function<NODE_HANDLE, NODE_HANDLE, bool> {
            public:
                node_handle_sort_comparator(const NODE_COMPARATOR & comparator):m_comparator(comparator){ ; }

                bool operator()(const NODE_HANDLE & h1, const NODE_HANDLE & h2) const {
                    // first, compare by value
                    if (m_comparator(h1.get_value(), h2.get_value())) {
                        return true;
                    } else if (m_comparator(h2.get_value(), h1.get_value())) {
                        return false;
                    }

                    // node values are the same, compare by output degree
                    if (h1.output_degree() < h2.output_degree()){
                        return true;
                    } else if (h2.output_degree() < h1.output_degree()){
                        return false;
                    }

                    // output degrees are the same, compare by input degree
                    return (h1.input_degree() < h2.input_degree());
                }

            private:
                const NODE_COMPARATOR & m_comparator;
            };

            // \brief compares two graph nodes by their real target value and degrees
            //! \brief compares two node entry iterators by their output degree,
            //! \brief input degree and real target value in degree-descending order
            template <
                typename DUMMY_NODE_HANDLE,
                typename NODE_COMPARATOR
            >
            class dummy_node_handle_sort_comparator {
            public:
                dummy_node_handle_sort_comparator(const NODE_COMPARATOR & comparator):m_comparator(comparator){ ; }

                bool operator()(const DUMMY_NODE_HANDLE & h1, const DUMMY_NODE_HANDLE & h2) const {
                    const size_t h1o = h1.output_degree();
                    const size_t h1i = h1.input_degree();
                    const size_t h2o = h2.output_degree();
                    const size_t h2i = h2.input_degree();
                    
                    // output degree
                    if (h1o > h2o) {
                        return true;
                    } else if (h2o > h1o) {
                        return false;
                    }

                    // input degree
                    if (h1i > h2i) {
                        return true;
                    } else if (h2i > h1i) {
                        return false;
                    }
                    
                    return m_comparator(*h1.get_value(), *h2.get_value());
                }

            private:
                const node_handle_sort_comparator<
                    typename DUMMY_NODE_HANDLE::graph_type::node_value_type::graph_type::node_handle,
                    NODE_COMPARATOR
                > m_comparator;
            };

            // \brief compares two graph nodes by their real target value and degrees
            //! \brief compares two node entry iterators by their output degree,
            //! \brief input degree and real target value in degree-descending order
            template <
                typename DUMMY_NODE_ITERATOR,
                typename NODE_COMPARATOR
            >
            class dummy_node_iterator_sort_comparator {
            public:
                dummy_node_iterator_sort_comparator(const NODE_COMPARATOR & comparator):m_comparator(comparator){ ; }

                bool operator()(const DUMMY_NODE_ITERATOR & i1, const DUMMY_NODE_ITERATOR & i2) const {
                    return m_comparator(*i1, *i2);
                }

            private:
                const dummy_node_handle_sort_comparator<
                    // this is correct
                    typename DUMMY_NODE_ITERATOR::graph_type::node_handle,
                    NODE_COMPARATOR
                > m_comparator;
            };

            // \brief compares two graph nodes by their real target value and degrees
            //! \brief compares two node entry iterators by their output degree and
            //! \brief input degree regardless to their real target value in degree-descending order
            template <
                typename DUMMY_NODE_ITERATOR
            >
            struct dummy_node_iterator_degree_comparator {
            public:
                bool operator()(const DUMMY_NODE_ITERATOR & i1, const DUMMY_NODE_ITERATOR & i2) const {
                    const size_t i1o = i1->output_degree();
                    const size_t i1i = i1->input_degree();
                    const size_t i2o = i2->output_degree();
                    const size_t i2i = i2->input_degree();
                    
                    // output degree
                    if (i1o > i2o) {
                        return true;
                    } else if (i2o > i1o) {
                        return false;
                    }

                    // input degree
                    return i1i < i2i;
                }
            };

            //! \brief compares two edge entry iterators by their real target value
            template <
                typename GRAPH_TYPE,
                typename EDGE_ENTRY_ITERATOR,
                typename NODE_COMPARATOR,
                typename EDGE_COMPARATOR
            >
            class dummy_edge_entry_ptr_comparator {
            public:
                dummy_edge_entry_ptr_comparator(const GRAPH_TYPE & parent, const NODE_COMPARATOR & n_comparator, const EDGE_COMPARATOR & e_comparator)
                    :m_parent_graph(parent), m_node_comparator(n_comparator), m_edge_comparator(e_comparator)
                { ; }

                bool operator()(const EDGE_ENTRY_ITERATOR & i1, const EDGE_ENTRY_ITERATOR & i2) const {
                    if (m_edge_comparator(i1->m_value->get_value(), i2->m_value->get_value())){
                        return true;
                    } else if (m_edge_comparator(i2->m_value->get_value(), i1->m_value->get_value())){
                        return false;
                    }
                    
                    typedef typename GRAPH_TYPE::node_entry_map::const_iterator cn_iterator;
                    
                    cn_iterator n1 = m_parent_graph.m_nodes.find(i1->m_target_id);
                    cn_iterator n2 = m_parent_graph.m_nodes.find(i2->m_target_id);
                    
                    assert(n1 != m_parent_graph.m_nodes.end());
                    assert(n2 != m_parent_graph.m_nodes.end());
                    
                    return m_node_comparator(n1->second->m_value->get_value(), n2->second->m_value->get_value());
                }

            private:
                const GRAPH_TYPE & m_parent_graph;
                const NODE_COMPARATOR & m_node_comparator;
                const EDGE_COMPARATOR & m_edge_comparator;
            };

            //! \brief compares two node entry iterators by their output degree,
            //! \brief input degree and real target value in degree-descending order
            template <
                typename NODE_ENTRY_POINTER,
                typename NODE_COMPARATOR
            >
            class dummy_node_entry_ptr_comparator {
            public:
                dummy_node_entry_ptr_comparator(const NODE_COMPARATOR & comparator):m_comparator(comparator){ ; }

                bool operator()(const NODE_ENTRY_POINTER & i1, const NODE_ENTRY_POINTER & i2) const {
                    // load degrees
                    const size_t i1o = i1->m_value->output_degree();
                    const size_t i1i = i1->m_value->input_degree();
                    const size_t i2o = i2->m_value->output_degree();
                    const size_t i2i = i2->m_value->input_degree();
                    
                    // overall degree 
                    /*
                    if ((i1o + i1i) < (i2o + i2i)) {
                        return true;
                    } else if ((i2o + i2i) < (i1o + i1i)) {
                        return false;
                    }
                    */
                    
                    // output degree
                    if (i1o > i2o) {
                        return true;
                    } else if (i2o > i1o) {
                        return false;
                    }

                    // input degree
                    if (i1i > i2i) {
                        return true;
                    } else if (i2i > i1i) {
                        return false;
                    }

                    // value
                    return (m_comparator(i1->m_value->get_value(), i2->m_value->get_value()));
//                    if (m_comparator(i1->m_value->get_value(), i2->m_value->get_value())) {
//                        return true;
//                    } else if (m_comparator(i2->m_value->get_value(), i1->m_value->get_value())) {
//                        return false;
//                    }

                }

            private:
                const NODE_COMPARATOR & m_comparator;
            };


            // reorders the nodes
            //template <
            //    typename DUMMY_NODE,
            //    typename DUMMY_EDGE,
            //    template<typename, typename> class GRAPH_DUMMY,
            //    typename NODE_COMPARATOR,
            //    typename EDGE_COMPARATOR
            //>
            //struct cgc_reorderer<GRAPH_DUMMY<DUMMY_NODE, DUMMY_EDGE> >

        }; // cls graph_utils

        template <typename GRAPH_TYPE>
        typename graph_dummy<GRAPH_TYPE>::normal_dummy graph_utils::create_normal_dummy(GRAPH_TYPE & original){
            typedef typename graph_dummy<GRAPH_TYPE>::normal_dummy dummy_graph;
            typedef typename GRAPH_TYPE::node_iterator original_node_iterator;
            typedef typename GRAPH_TYPE::edge_iterator original_edge_iterator;
            typedef typename dummy_graph::node_iterator output_node_iterator;
            typedef graph_utils::node_iterator_comparator<original_node_iterator> aux_map_comparator;
            typedef std::map<original_node_iterator, output_node_iterator, aux_map_comparator> auxiliary_map;

            dummy_graph output;
            auxiliary_map fast_access;

            // clone nodes
            for (original_node_iterator node = original.nodes_begin(); node != original.nodes_end(); ++node){
                fast_access[node] = output.create_node(node);
            }

            // clone edges
            for (original_edge_iterator edge = original.edges_begin(); edge != original.edges_end(); ++edge){
                output.create_edge(*(fast_access[edge->from_node()]), *(fast_access[edge->to_node()]), edge);
            }

            return output;
        }

        template <typename GRAPH_TYPE>
        typename graph_dummy<GRAPH_TYPE>::const_dummy graph_utils::create_const_dummy(const GRAPH_TYPE & original){
            typedef typename graph_dummy<GRAPH_TYPE>::const_dummy dummy_graph;
            typedef typename GRAPH_TYPE::const_node_iterator original_node_iterator;
            typedef typename GRAPH_TYPE::const_edge_iterator original_edge_iterator;
            typedef typename dummy_graph::node_iterator output_node_iterator;
            typedef graph_utils::node_iterator_comparator<original_node_iterator> aux_map_comparator;
            typedef std::map<original_node_iterator, output_node_iterator, aux_map_comparator> auxiliary_map;

            dummy_graph output;
            auxiliary_map fast_access;

            // clone nodes
            for (original_node_iterator node = original.nodes_begin(); node != original.nodes_end(); ++node){
                fast_access[node] = output.create_node(node);
            }

            // clone edges
            for (original_edge_iterator edge = original.edges_begin(); edge != original.edges_end(); ++edge){
                output.create_edge(*(fast_access[edge->from_node()]), *(fast_access[edge->to_node()]), edge);
            }

            return output;
        }


        template <typename Node, typename Edge>
        bool graph_utils::oriented_contains_cycle(const graph<Node, Edge> & grph){
            typedef graph<Node, Edge> graph_type;
            typename graph_dummy<graph_type>::const_dummy dummy = graph_utils::create_const_dummy(grph);

            if (grph.empty()){ return false; }

            typedef std::set<node_id_t> nodeid_set;
            typedef std::list<node_trace> node_list;

            //nodeid_set forgotten_nodes;
            typename graph<Node, Edge>::node_entry_map tmp_nodes(grph.m_nodes);

            // since there might be separated components
            while (!tmp_nodes.empty()){
                nodeid_set component_nodes;
                node_list current_path;

                {
                    node_trace tmp;
                    tmp.node_id = tmp_nodes.begin()->first;
                    tmp.edge_id = 0;
                    current_path.push_back(tmp);
                }

                // common forgetful dfs
                while (!current_path.empty()) {

                    node_trace current_node = current_path.back();
                    current_path.pop_back();

                    typename graph_type::node_entry_map::const_iterator current_node_iter = tmp_nodes.find(current_node.node_id);

                    if (current_node_iter == tmp_nodes.end()){
                        // node about to be processed belongs to another component
                        continue;
                    }

                    if (current_node.edge_id == 0){
                        bool nid_found = false;
                        for (node_list::const_iterator tmp_i = current_path.begin(); !nid_found && (tmp_i != current_path.end()); tmp_i++){
                            if (tmp_i->node_id == current_node.node_id){ nid_found = true; }
                        }

                        if (nid_found){
                            // the node is in current path, that means a cycle was found
                            return true;
                        } else if (component_nodes.find(current_node.node_id) != component_nodes.end()) {
                            // node was already processed and is claimed again as new
                            // however is not in current path
                            continue;
                        }
                    }

                    ++current_node.edge_id;
                    component_nodes.insert(current_node.node_id);

                    typedef std::pair<
                        typename graph_type::edge_entry_map::const_iterator,
                        typename graph_type::edge_entry_map::const_iterator
                    > next_edge_iter_pair;

                    next_edge_iter_pair next_edge = current_node_iter->second->m_edges.equal_range(current_node.edge_id);

                    if (next_edge.first != current_node_iter->second->m_edges.end()){
                        current_node.edge_id = next_edge.first->first;
                        current_path.push_back(current_node);

                        node_trace next_node;
                        next_node.node_id = next_edge.first->second->m_target_id;
                        next_node.edge_id = 0;
                        current_path.push_back(next_node);
                    }
                };

                for (nodeid_set::const_iterator i = component_nodes.begin(); i != component_nodes.end(); ++i){
                    tmp_nodes.erase(*i);
                }
                //forgotten_nodes.insert(component_nodes.begin(), component_nodes.end());
            }

            return false;
        }

        template <typename GRAPH_TYPE>
        std::list<GRAPH_TYPE> graph_utils::split_components_copy_1_0(const GRAPH_TYPE & original_graph){

            typedef typename GRAPH_TYPE::const_node_iterator const_on_iterator;
            typedef typename GRAPH_TYPE::const_edge_iterator const_oe_iterator;
            typedef typename GRAPH_TYPE::node_iterator output_node_iterator;
            //typedef std::map<const_on_iterator, size_t> color_map;
            typedef std::set<const_on_iterator, node_iterator_comparator<const_on_iterator> > component_set;
            typedef std::stack<const_on_iterator> component_stack;

            typedef graph_utils::node_iterator_comparator<const_on_iterator> aux_map_comparator;

            typedef std::map<const_on_iterator, output_node_iterator, aux_map_comparator> auxiliary_map;


            //color_map colors;
            component_set done;
            typename std::list<GRAPH_TYPE> output;

            // this cycle is due to fact that multiple components are expected
            while (done.size() != original_graph.nodes_count()){
                // apped empty component at the end of the list
                output.push_back(GRAPH_TYPE());
                GRAPH_TYPE & separated_component = output.back();

                component_set current_component;
                component_stack discovered;

                auxiliary_map fast;

                // find initial node of this component
                for (const_on_iterator i = original_graph.nodes_begin(); (discovered.empty()) && (i != original_graph.nodes_end()); ++i){
                    if (done.find(i) == done.end()){ discovered.push(i); }
                }
                assert(!discovered.empty()); // hups, something is really wrong!

                while (!discovered.empty()){
                    const_on_iterator current = discovered.top();
                    discovered.pop();

                    if (current_component.find(current) != current_component.end()){ continue; }
                    current_component.insert(current);
                    if (fast.find(current) == fast.end()){
                        fast[current] = separated_component.create_node(current->get_value());
                    }
                    
                    // forward discovery
                    for (const_oe_iterator e = current->edges_begin(); e != current->edges_end(); ++e){
                        const_on_iterator tmp = e->to_node();
                        // if this is the first time we hit this node, create it's copy
                        if (fast.find(tmp) == fast.end()){
                             fast[tmp] = separated_component.create_node(tmp->get_value());
                        }

                        // copy edge
                        separated_component.create_edge(*(fast[current]), *(fast[tmp]), e->get_value());
                        discovered.push(tmp);
                    }

                    // backward discovery
                    typedef typename GRAPH_TYPE::const_node_iterator_vector cnil;
                    cnil backs = current->predecessors();
                    for (typename cnil::const_iterator i = backs.begin(); i != backs.end(); ++i){
                        discovered.push(*i);
                    }
                }

                done.insert(current_component.begin(), current_component.end());
            }

            return output;
        }

        template <typename GRAPH_TYPE>
        std::list<GRAPH_TYPE> graph_utils::split_strong_components_copy_1_0(const GRAPH_TYPE & original_graph){

            typedef typename GRAPH_TYPE::const_node_iterator const_on_iterator;
            typedef typename GRAPH_TYPE::const_edge_iterator const_oe_iterator;
            //typedef std::map<const_on_iterator, size_t> color_map;

            typedef node_iterator_comparator<const_on_iterator> aux_map_comparator;

            typedef std::map<const_on_iterator, scc_tarjan_entry, aux_map_comparator> index_map;

            typedef std::list<typename index_map::const_iterator> tarjan_stack;
            typedef typename std::list<scc_dfs_entry<const_on_iterator> > dfs_stack;

            std::list<GRAPH_TYPE> output_components;
            index_map indices;
            size_t global_index = 0;
            tarjan_stack tarjan_nodes;

            bool run = true;
            while (run){
                dfs_stack dfs_nodes;

                // select start vertice
                const_on_iterator start_from_here = original_graph.nodes_end();
                for (const_on_iterator i = original_graph.nodes_begin(); (i != original_graph.nodes_end()) && (start_from_here == original_graph.nodes_end()); ++i){
                    if (indices.find(i) == indices.end()){ start_from_here = i; }
                }
                if (start_from_here == original_graph.nodes_end()){ run = false; continue; }

                // init dfs stack
                typename dfs_stack::value_type initial;
                initial.to_process = start_from_here;
                initial.parent = original_graph.nodes_end();
                dfs_nodes.push_back(initial);

                while (!dfs_nodes.empty()){
                    // load current
                    typename dfs_stack::value_type & current_entry = dfs_nodes.back();
                    const_on_iterator v = current_entry.to_process; // v is for tarjan

                    // dfs on this node is done, run tarjan propagation
                    if (current_entry.finished) {
                        if (current_entry.parent != original_graph.nodes_end()){
                            indices[current_entry.parent].lowlink = std::min(indices[current_entry.parent].lowlink, indices[current_entry.to_process].lowlink);
                        }
                        dfs_nodes.pop_back();

                        if (indices[v].index == indices[v].lowlink){
                            output_components.push_back(GRAPH_TYPE());
                            GRAPH_TYPE & component = output_components.back();

                            typedef typename GRAPH_TYPE::node_iterator output_node_iterator;
                            typedef std::map<const_on_iterator, output_node_iterator, aux_map_comparator> fast_access_map;
                            fast_access_map facc;

                            // copy the nodes
                            typename index_map::const_iterator w;
                            do {
                                assert(!tarjan_nodes.empty());
                                w = tarjan_nodes.back();
                                tarjan_nodes.pop_back();

                                if (facc.find(w->first) == facc.end()){
                                    facc[w->first] = component.create_node(w->first->get_value());
                                }
                            } while ((w->first != v) && (!tarjan_nodes.empty()));

                            // copy respective edges
                            for (typename fast_access_map::const_iterator fn = facc.begin(); fn != facc.end(); ++fn){
                                const_on_iterator scanned = fn->first;
                                for (const_oe_iterator scanned_edge = scanned->edges_begin(); scanned_edge != scanned->edges_end(); ++scanned_edge){
                                    if (facc.find(scanned_edge->to_node()) != facc.end()) {
                                        component.create_edge(*(facc[scanned]), *(facc[scanned_edge->to_node()]), scanned_edge->get_value());
                                    }
                                }
                            }
                            // component done
                        }
                    } else {
                        // tarjan entry init
                        indices[current_entry.to_process].index = global_index;
                        indices[current_entry.to_process].lowlink = global_index;
                        ++global_index;
                        tarjan_nodes.push_back(indices.find(current_entry.to_process));
                        current_entry.finished = true;

                        // process edges (for undiscovered run dfs), for discovered run tarjan
                        for (const_oe_iterator e = v->edges_begin(); e != v->edges_end(); ++e){
                            const_on_iterator tmp = e->to_node();
                            typename index_map::iterator w = indices.find(tmp);
                            if (w == indices.end()){
                                typename dfs_stack::value_type next;
                                next.to_process = tmp;
                                next.parent = current_entry.to_process;
                                dfs_nodes.push_back(next);
                            } else {
                                indices[v].lowlink = std::min(indices[v].lowlink, w->second.lowlink);
                            }
                        }
                        current_entry.finished = true;
                    }
                }
            }

            return output_components;
        }

        // libkerat 1.0
        template <typename GRAPH_TYPE, typename NODE_COMPARATOR, typename EDGE_COMPARATOR>
        int graph_utils::compare_graphs(
            const GRAPH_TYPE & first_graph,
            const GRAPH_TYPE & second_graph,
            const NODE_COMPARATOR & node_comparator,
            const EDGE_COMPARATOR & edge_comparator
        ){
            typedef typename graph_dummy<GRAPH_TYPE>::const_dummy dummy_graph;
            typedef typename std::list<dummy_graph> dummy_list;
            typedef typename std::vector<dummy_graph> dummy_vector;
            
            dummy_vector dummies_1, dummies_2;

            { // copy to vector
                dummy_list tmp_dummie = split_components_copy_1_0(create_const_dummy(first_graph));
                dummies_1.resize(tmp_dummie.size());
                std::copy(tmp_dummie.begin(), tmp_dummie.end(), dummies_1.begin());

                tmp_dummie = split_components_copy_1_0(create_const_dummy(second_graph));
                dummies_2.resize(tmp_dummie.size());
                std::copy(tmp_dummie.begin(), tmp_dummie.end(), dummies_2.begin());
            }

            // test whether we at least have graps with same component count
            if (dummies_1.size() < dummies_2.size()){
                return -1;
            } else if (dummies_2.size() < dummies_1.size()){
                return 1;
            }

            primitive_graph_component_comparator<dummy_graph> orderer;

            // sort components by size
            std::sort(dummies_1.begin(), dummies_1.end(), orderer);
            std::sort(dummies_2.begin(), dummies_2.end(), orderer);
            for (
                typename dummy_vector::const_iterator i1 = dummies_1.begin(), i2 = dummies_2.begin();
                (i1 != dummies_1.end()) && (i2 != dummies_2.end());
                ++i1, ++i2
            ){
                int cv = orderer.compare(*i1, *i2);
                if (cv != 0){ return cv; }
            }

            // from now on, the only way to compare the graphs is homomorphy detection on the components

            for (
                typename dummy_vector::iterator i1 = dummies_1.begin(), i2 = dummies_2.begin();
                (i1 != dummies_1.end()) && (i2 != dummies_2.end());
                ++i1, ++i2
            ){
                int cv = compare_graph_component(*i1, *i2, node_comparator, edge_comparator);
                if (cv != 0){ return cv; }
            }

            return 0;
        }

        template <typename GRAPH_DUMMY_TYPE, typename NODE_COMPARATOR, typename EDGE_COMPARATOR>
        int graph_utils::compare_graph_component(
            GRAPH_DUMMY_TYPE & component1,
            GRAPH_DUMMY_TYPE & component2,
            const NODE_COMPARATOR & node_comparator,
            const EDGE_COMPARATOR & edge_comparator
        ){
            // it does not make sence to run this slow algorithm on completly different components
            size_t ndist1 = component1.nodes_count();
            size_t ndist2 = component2.nodes_count();
            if (ndist1 < ndist2){
                return -1;
            } else if (ndist1 > ndist2){
                return 1;
            }
            
            size_t edist1 = component1.edges_count();
            size_t edist2 = component2.edges_count();
            if (edist1 < edist2){
                return -1;
            } else if (edist1 > edist2){
                return 1;
            }

            typedef typename GRAPH_DUMMY_TYPE::node_value_type node_iterator;
            typedef typename node_iterator::value_type node_handle;
            typedef typename GRAPH_DUMMY_TYPE::node_iterator dummy_iterator;
            typedef typename GRAPH_DUMMY_TYPE::const_edge_iterator dummy_cedge_iterator;
            typedef typename GRAPH_DUMMY_TYPE::edge_iterator dummy_edge_iterator;
            typedef typename GRAPH_DUMMY_TYPE::node_handle dummy_handle;
            typedef typename std::vector<dummy_iterator> dummy_vector;
            typedef std::pair<typename dummy_vector::iterator, typename dummy_vector::iterator> node_range;

            // comparators
            dummy_node_handle_sort_comparator<dummy_handle, NODE_COMPARATOR> dummy_node_range_comparator(node_comparator);
            dummy_node_iterator_sort_comparator<dummy_iterator, NODE_COMPARATOR> dummy_node_sort_comparator(node_comparator);

            while (!component1.empty()){
                // first reorder the components as attempt to canonize them a little bit
                graph_component_reorder(component1, node_comparator, edge_comparator);
                graph_component_reorder(component2, node_comparator, edge_comparator);
                
                GRAPH_DUMMY_TYPE component1_backup(component1);
                GRAPH_DUMMY_TYPE component2_backup(component2);

                // the main check loop
                while (!component1_backup.empty()){
                    size_t r1_offset = 0;
                    size_t r2_offset = 0;
                    dummy_vector nodes1, nodes2;
                    // init for node vectors
                    while (!component1_backup.empty()){
                        // reset
                        component1 = component1_backup;
                        component2 = component2_backup;
                        
                        nodes1.clear();
                        nodes2.clear();
                        
                        for (dummy_iterator i = component1.nodes_begin(); i != component1.nodes_end(); ++i){ nodes1.push_back(i); }
                        for (dummy_iterator i = component2.nodes_begin(); i != component2.nodes_end(); ++i){ nodes2.push_back(i); }

                        std::sort(nodes1.begin(), nodes1.end(), dummy_node_sort_comparator);
                        std::sort(nodes2.begin(), nodes2.end(), dummy_node_sort_comparator);

//                        node_range range1 = std::equal_range(nodes1.begin(), nodes1.end(), *nodes1.begin(), dummy_node_degree_comparator);
//                        node_range range2 = std::equal_range(nodes2.begin(), nodes2.end(), *nodes2.begin(), dummy_node_degree_comparator);
                        node_range range1 = std::equal_range(nodes1.begin(), nodes1.end(), *nodes1.begin(), dummy_node_sort_comparator);
                        node_range range2 = std::equal_range(nodes2.begin(), nodes2.end(), *nodes2.begin(), dummy_node_sort_comparator);

                        assert(range1.first != range1.second);
                        assert(range2.first != range2.second);
                        
                        // reset commanced, apply offsets
                        range1.first += r1_offset;
                        range2.first += r2_offset;

                        // okay, this means that there are no more match candidates left,
                        // there for this two grapsh are completly different!
                        if (range2.first == range2.second){ return -1; }
                        
                        // compare nodes themself
                        if (dummy_node_sort_comparator(*range1.first, *range2.first)){
                            return -1;
                        } else if (dummy_node_sort_comparator(*range2.first, *range1.first)){
                            return 1;
                        }

                        // r1_offset is here just for nicer code, if two gras are
                        // homamorphous, there has to be twin for every node
                        
                        // test candidate - forward edges
                        {
                            dummy_cedge_iterator e1 = (*range1.first)->edges_begin();
                            dummy_cedge_iterator e2 = (*range2.first)->edges_begin();
                            
                            while ((e1 != (*range1.first)->edges_end()) && (e2 != (*range2.first)->edges_end())){
                                // test edge value
                                if (
                                    edge_comparator(e1->get_value()->get_value(), e2->get_value()->get_value())
                                    !=
                                    edge_comparator(e2->get_value()->get_value(), e1->get_value()->get_value())
                                ) {
                                    // edge values do not match!
                                    goto invalid;
                                }
                                
                                // test edge target's value
                                if (
                                    node_comparator(e1->to_node()->get_value()->get_value(), e2->to_node()->get_value()->get_value())
                                    !=
                                    node_comparator(e2->to_node()->get_value()->get_value(), e1->to_node()->get_value()->get_value())
                                ) {
                                    // target's values do not match!
                                    goto invalid;
                                }

                                // edge is okay, advance
                                ++e1; ++e2;
                            }
                            
                            assert((e1 == (*range1.first)->edges_end()) == (e2 == (*range2.first)->edges_end()));
                        }

                        // test candidate - backward edges
                        {
                            
                        dummy_vector predecessors1 = (*range1.first)->predecessors();
                        dummy_vector predecessors2 = (*range2.first)->predecessors();

                        // to be equivalent, both nodes must have the same count of predecessors
                        if (predecessors1.size() != predecessors2.size()){ goto invalid; }

                        // predecessor testing makes sence only where there are predecessors
                        if (!predecessors1.empty()){
                            
                            std::sort(predecessors1.begin(), predecessors1.end(), dummy_node_sort_comparator);
                            std::sort(predecessors2.begin(), predecessors2.end(), dummy_node_sort_comparator);
                            
                            // todo compare predecessors
                            
                            typename dummy_vector::iterator p1 = predecessors1.begin();
                            typename dummy_vector::iterator p2 = predecessors2.begin();

                            dummy_edge_iterator e1 = (*p1)->edges_end();
                            dummy_edge_iterator e2 = (*p2)->edges_end();

                            
                            while ((predecessors1.size() == predecessors2.size()) && (p2 != predecessors2.end())){
                                // compare nodes themself
                                if (
                                    dummy_node_sort_comparator(*p1, *p2) 
                                    != dummy_node_sort_comparator(*p2, *p1)
                                ){
                                    goto predecessor_candidate_invalid;
                                }

                                // iterate through the edges from predecessor to this one
                                
                                e1 = (*p1)->edges_begin();
                                e2 = (*p2)->edges_begin();

                                while ((e1 != (*p1)->edges_end()) && (e2 != (*p2)->edges_end())){

                                    // test edge target
                                    bool c1m = (e1->to_node() == *range1.first);
                                    bool c2m = (e2->to_node() == *range2.first);

                                    if (c1m != c2m){ goto predecessor_candidate_invalid; }
                                    // else assume c1m == c2m

                                    // co not care about those who do not belong to us
                                    if (c1m == false){
                                        ++e1; ++e2;
                                        continue;
                                    }

                                    // test edge value
                                    if (
                                        edge_comparator(e1->get_value()->get_value(), e2->get_value()->get_value())
                                        !=
                                        edge_comparator(e2->get_value()->get_value(), e1->get_value()->get_value())
                                    ) {
                                        // edge values do not match!
                                        goto predecessor_candidate_invalid;
                                    }
                                
                                    // edge is okay, erase it

                                    dummy_edge_iterator e1i(e1); 
                                    dummy_edge_iterator e2i(e2); 
                                    ++e1; ++e2;

                                    component1.remove_edge(*e1i);
                                    component2.remove_edge(*e2i);
                                    continue;
                                }
                                assert((e1 == (*p1)->edges_end()) == (e2 == (*p2)->edges_end()));
                                
                                //predecessor_candidate_valid:
                                    predecessors1.erase(p1);
                                    predecessors2.erase(p2);
                                    p1 = predecessors1.begin();
                                    p2 = predecessors2.begin();
                                    continue;
                                
                                predecessor_candidate_invalid:
                                    ++p2;
                                    continue;
                            }

                            if (!predecessors1.empty()){ goto invalid; }
                        } // if
                        } // code block
                        
                        // candidate if fit, remove and commance degree sequence scan
                        component1.remove_node(*(*range1.first));
                        component2.remove_node(*(*range2.first));

                        nodes1.erase(range1.first);
                        nodes2.erase(range2.first);
                        
                        std::sort(nodes1.begin(), nodes1.end(), dummy_node_sort_comparator);
                        std::sort(nodes2.begin(), nodes2.end(), dummy_node_sort_comparator);

                        if (!compare_degree_sequence(nodes1, nodes2)){ goto invalid; }
                        
                        //valid:
                            component1_backup = component1;
                            component2_backup = component2;
                            r1_offset = 0;
                            r2_offset = 0;
                            continue;

                        invalid:
                            ++r2_offset;
                            continue;
                    }
                }

            }


            return 0;
        }

        // internaly rebuilds the dummy graph component so the following rules apply
        // - nodes are reordered by weak ordering
        // - nodes with same value are reordered by their outgoing and incoming degrees
        // - edges are reordered by their values and by values of target nodes
        template <typename GRAPH_DUMMY_TYPE, typename NODE_COMPARATOR, typename EDGE_COMPARATOR>
        void graph_utils::graph_component_reorder(
            GRAPH_DUMMY_TYPE & component,
            const NODE_COMPARATOR & node_comparator,
            const EDGE_COMPARATOR & edge_comparator
        ){
            typedef typename GRAPH_DUMMY_TYPE::node_entry_map::iterator dummy_node_entry_iterator;
            typedef typename GRAPH_DUMMY_TYPE::edge_entry_map::iterator dummy_edge_entry_iterator;
            typedef typename GRAPH_DUMMY_TYPE::edge_entry * edge_entry_ptr;
            typedef typename GRAPH_DUMMY_TYPE::node_entry * node_entry_ptr;
            typedef typename GRAPH_DUMMY_TYPE::node_entry::neighbour_set::iterator node_set_iterator;

            // reorder nodes in dummy
            {
                typedef std::vector<node_entry_ptr> node_vector;
                typedef std::map<typename GRAPH_DUMMY_TYPE::node_id_t, typename GRAPH_DUMMY_TYPE::node_id_t> moved_map;
                // copy nodes
                node_vector nodes;
                for (dummy_node_entry_iterator node = component.m_nodes.begin(); node != component.m_nodes.end(); ++node){
                    nodes.push_back(node->second);
                }

                // sort node entries
                dummy_node_entry_ptr_comparator<node_entry_ptr, NODE_COMPARATOR> comparator(node_comparator);
                std::sort(nodes.begin(), nodes.end(), comparator);
                
                moved_map moved;

                // swap node entries
                for (typename node_vector::iterator source = nodes.begin(); source != nodes.end(); ++source){
                    // create new name for source
                    size_t new_id = ++component.m_last_node_id;
                    size_t old_id = (*source)->m_node_id;
                    component.m_nodes[new_id] = *source;
                    (*source)->m_node_id = new_id;
                    moved[old_id] = new_id;

                    // update forward references
                    for (dummy_edge_entry_iterator edge = (*source)->m_edges.begin(); edge != (*source)->m_edges.end(); ++edge){
                        typename GRAPH_DUMMY_TYPE::node_id_t target_id = edge->second->m_target_id;
                        typename moved_map::const_iterator target_moved = moved.find(target_id);
                        if (target_moved != moved.end()) { target_id = target_moved->second; }
                        
                        typename GRAPH_DUMMY_TYPE::node_entry_map::iterator updated_node = component.m_nodes.find(target_id);
                        assert(updated_node != component.m_nodes.end());
                        
                        updated_node->second->m_neighbours.erase(old_id);
                        updated_node->second->m_neighbours.insert(new_id);
                    }

                    // update backward references
                    for (node_set_iterator neighbour = (*source)->m_neighbours.begin(); neighbour != (*source)->m_neighbours.end(); ++neighbour) {
                        typename GRAPH_DUMMY_TYPE::node_id_t neighbour_id = *neighbour;
                        typename moved_map::const_iterator neighbour_moved = moved.find(neighbour_id);
                        assert(neighbour_moved == moved.end());
                        
                        typename GRAPH_DUMMY_TYPE::node_entry_map::iterator neighbour_entry = component.m_nodes.find(neighbour_id);
                        for (dummy_edge_entry_iterator edge = neighbour_entry->second->m_edges.begin(); edge != neighbour_entry->second->m_edges.end(); ++edge){
                            if (edge->second->m_target_id == old_id){
                                edge->second->m_target_id = new_id;
                            }
                        }
                    }

                    // delete old name
                    component.m_nodes.erase(old_id);
                }
            }
            // nodes now reordered by their value and degree

            // reorder edges in dummy
            for (dummy_node_entry_iterator node = component.m_nodes.begin(); node != component.m_nodes.end(); ++node){
                typedef std::vector<edge_entry_ptr> edge_vector;
                // copy edge iterators
                edge_vector edges;
                for (dummy_edge_entry_iterator edge = (node->second)->m_edges.begin(); edge != (node->second)->m_edges.end(); ++edge){
                    edges.push_back(edge->second);
                }
                // sort edge entries
                dummy_edge_entry_ptr_comparator<GRAPH_DUMMY_TYPE, edge_entry_ptr, NODE_COMPARATOR, EDGE_COMPARATOR> comparator(component, node_comparator, edge_comparator);
                std::sort(edges.begin(), edges.end(), comparator);

                // swap edge entries
                dummy_edge_entry_iterator target = node->second->m_edges.begin();
                typename edge_vector::iterator source = edges.begin();
                while (source != edges.end()){
                    target->second = *source;
                    ++source;
                    ++target;
                }
                // edges in this node are now reordered according to their value
            }
            // edges are now reordered by their value and target node id

        }

    }

    // libkerat 1.0 wrapper
    template <typename GRAPH_TYPE, typename NODE_COMPARATOR, typename EDGE_COMPARATOR>
    int graph_compare(
        const GRAPH_TYPE & first_graph,
        const GRAPH_TYPE & second_graph,
        const NODE_COMPARATOR & node_comparator,
        const EDGE_COMPARATOR & edge_comparator
    ){
        return internals::graph_utils::compare_graphs(first_graph, second_graph, node_comparator, edge_comparator);
    }

    template <typename GRAPH_TYPE, typename NODE_COMPARATOR, typename EDGE_COMPARATOR>
    int graph_compare(
        const GRAPH_TYPE & first_graph,
        const GRAPH_TYPE & second_graph
    ){
        return internals::graph_utils::compare_graphs(
            first_graph,
            second_graph,
            first_graph.get_node_value_comparator(),
            first_graph.get_edge_value_comparator()
        );
    }





    /**
     * \brief Detect whether the given graph, considered oriented, contains a cycle
     * \param grph - graph in which to detect cycles
     * \return true if contains oriented cycle
     */
    template <typename Node, typename Edge>
    inline bool graph_contains_cycle_oriented(const libkerat::internals::graph<Node, Edge> & grph){
        return libkerat::internals::graph_utils::oriented_contains_cycle(grph);
    }

    /**
     * \brief Detect whether the given graph, considered unoriented, contains a cycle
     * \param grph - graph in which to detect cycles
     * \return true if contains unoriented cycle
     */
    template <typename Node, typename Edge>
    inline bool graph_contains_cycle_unoriented(const libkerat::internals::graph<Node, Edge> & grph){
        return libkerat::internals::graph_utils::contains_cycle_unoriented(grph);
    }

    /**
     * \brief Counts components of the given graph
     * \param grph - graph whose components count shall be determined
     * \return for empty graph 0, for any other at least 1 and at most the \ref graph::nodes_count
     */
    template <typename Node, typename Edge>
    inline static size_t graph_count_components(const libkerat::internals::graph<Node, Edge> & grph){
        return internals::graph_utils::split_components(grph).size();
    }

    /**
     * \brief Counts how many nodes of the graph contain given value
     * \param grph - graph in which search
     * \param value - value to search for
     * \return non-negative occurences count
     */
    template <typename Node, typename Edge>
    inline size_t graph_count_node(const libkerat::internals::graph<Node, Edge> & grph, typename libkerat::internals::graph<Node, Edge>::Node_type & value){
        return libkerat::internals::graph_utils::count_node_value(grph, value);
    }

    /**
     * \brief Checks whether the given graph contains duplicit node values
     * \param grph - graph in which search
     * \return true if at least two nodes share value
     */
    template <typename Node, typename Edge>
    inline bool graph_contains_duplicit_nodes(const libkerat::internals::graph<Node, Edge> & grph){
        return libkerat::internals::graph_utils::has_duplicit_nodes(grph);
    }

    /**
     * \brief Checks whether the given graph is tree
     *
     * Graph is considered a tree if:
     * \li Has at least 1 node
     * \li Has exactly 1 component
     * \li Does not contain unoriented cycle
     *
     * \param grph - graph in which search
     * \return true if matches definition above, false otherwise
     */
    template <typename Node, typename Edge>
    inline bool graph_topology_is_tree(const libkerat::internals::graph<Node, Edge> & grph){
        return (!grph.empty())
            && (graph_components_count(grph) == 1)
            && (!graph_contains_cycle_unoriented(grph));
    }

    // forward declaration for gcc 4.7
    template <typename Node, typename Edge>
    bool graph_topology_is_trunk_tree(const libkerat::internals::graph<Node, Edge> & grph);

    /**
     * \brief Checks whether the given graph is
     * \ref graph_topology_is_trunk_tree "trunk tree" and does not contain duplicit node values
     * \param grph - graph in which search
     * \return true if matches definition above, false otherwise
     */
    template <typename Node, typename Edge>
    inline bool graph_trunk_tree_is_valid(const libkerat::internals::graph<Node, Edge> & grph){
        return graph_topology_is_trunk_tree(grph) && !graph_contains_duplicit_nodes(grph);
    }

    /**
     * \brief Checks whether the given graph is
     * \ref graph_topology_is_tree "tree" and does not contain duplicit node values
     * \param grph - graph in which search
     * \return true if matches definition above, false otherwise
     */
    template <typename Node, typename Edge>
    inline bool graph_tree_is_valid(const libkerat::internals::graph<Node, Edge> & grph){
        return graph_topology_is_tree(grph) && !graph_contains_duplicit_nodes(grph);
    }

    /**
     * \brief Checks whether the given oriented graph is linear
     *
     * Graph is considered a linear oriented if:
     * \li Has exactly 1 component
     * \li Has Accyclic chain topology - if the graph has at least 2 nodes, then
     * there is 1 node with output degree 1 and input degree 0 (origin node),
     * 1 node with input degree 1 and output degree 0 (end node) and every other
     * node has both input and output degrees equal to 1
     * \li Every node in the graph is reachable from the origin node
     *
     * \param grph - graph in which search
     * \return true if matches definition above, false otherwise
     */
    template <typename Node, typename Edge>
    inline bool graph_topology_is_linear_oriented(const libkerat::internals::graph<Node, Edge> & grph){
        return (!grph.empty())
            && (graph_components_count(grph) == 1)
            && (!libkerat::internals::graph_utils::topology_is_linear_oriented(grph));
    }

    /**
     * \brief Checks whether the given oriente graph is \ref graph_topology_is_linear_oriented "linear"
     * and does not contain duplicit values
     * \param grph - graph in which search
     * \return true if matches definition above, false otherwise
     */
    template <typename Node, typename Edge>
    inline bool graph_linear_oriented_is_valid(const libkerat::internals::graph<Node, Edge> & grph){
        return (graph_topology_is_linear_oriented(grph) && !graph_contains_duplicit_nodes(grph));
    }

    /**
     * \brief Checks whether the given oriented graph is oriented star
     *
     * Graph is considered a oriented star if:
     * \li Has exactly 1 component
     * \li Has tree-like topology
     * \li Has exactly 1 node with output degree > 1, every other node has output degree 0 or
     * exactly 1 node with input degree > 1, every other node has output degree 1
     * \li All leafs are reachable from the central node (or analogicaly for the second version).
     * \param grph - graph in which search
     * \return true if matches definition above, false otherwise
     */
    template <typename Node, typename Edge>
    inline bool graph_topology_is_star_oriented(const libkerat::internals::graph<Node, Edge> & grph){
        if (internals::graph_utils::topology_star_get_oriented_inout_center(grph) != grph.nodes_end()){ return true; }
        if (internals::graph_utils::topology_star_get_oriented_outin_center(grph) != grph.nodes_end()){ return true; }
        return false;
    }

    /**
     * \brief Checks whether the given oriented graph is trunk-tree
     *
     * Graph is considered a trunk-tree if:
     * \li Has exactly 1 component
     * \li Has tree-like topology
     * \li Has exactly 1 leaf (origin leaf) from which leads the oriented edge to the first node,
     * from this node to next until a split-node is found (like a trunk of real tree) - from now on,
     * the rest of this tree can be considered classical oriented tree.
     * \li All nodes and leafs are reachable from the origin leaf.
     * \param grph - graph in which search
     * \return true if matches definition above, false otherwise
     */
    template <typename Node, typename Edge>
    bool graph_topology_is_trunk_tree(const libkerat::internals::graph<Node, Edge> & grph){

        size_t edges_size = grph.edges_count();
        size_t nodes_size = grph.nodes_count();

        if (nodes_size != (edges_size+1) ){ return false; }
        size_t visited_nodes_count = 0;

        typedef libkerat::internals::graph<Node, Edge> graph_type;
        typedef typename graph_type::const_node_iterator const_node_iterator;
        typedef typename graph_type::const_edge_iterator const_edge_iterator;


        const_node_iterator trunk_leaf = internals::graph_utils::get_oriented_outin_leaf(grph.nodes_begin(), grph.nodes_end());
        if (trunk_leaf == grph.nodes_end()){ return false; }

        typedef std::stack<const_edge_iterator> edge_stack;
        edge_stack return_edges;
        const_node_iterator next_node = trunk_leaf;
        ++visited_nodes_count;

        while (next_node != grph.nodes_end()){
            size_t out_degree = next_node->output_degree();
            size_t in_degree = next_node->input_degree();

            if (in_degree > 1){ return false; }

            // I'm not leaf
            switch (out_degree){
                case 0: {
                    // last leaf
                    if (return_edges.empty()){
                        next_node = grph.nodes_end();
                    } else {
                        const_edge_iterator edge = return_edges.top();
                        return_edges.pop();
                        const_node_iterator hndl = edge->from_node();
                        ++edge;

                        if (edge == hndl->edges_end()){
                            // this was last edge of given subtree, so return as far
                            // as possible leave the node as is, it will take care of
                            // the rest
                        // check another branch
                        } else {
                            return_edges.push(edge);
                            ++visited_nodes_count;
                            next_node = edge->to_node();
                        }
                    }
                    break;
                }
                case 1: {
                    ++visited_nodes_count;
                    next_node = next_node->edges_begin()->to_node();
                    break;
                }
                default: {
                    return_edges.push(next_node->edges_begin());
                    ++visited_nodes_count;
                    next_node = next_node->edges_begin()->to_node();
                }
            }

        }
        if (visited_nodes_count < nodes_size){ return false; }

        return true;
    }

    /**
     * \brief Iterates through the graph as through oriented linear graph
     * \note Behaviour on topologies other than linear oriented is undefined
     * \param graph - graph to iterate over
     * \param node_callback - callback to call when node is accessed
     * \param edge_callback - callback to call when edge is accessed
     * \tparam NODE_HIT_CALLBACK - unary function type with node handle as argument
     * \tparam EDGE_HIT_CALLBACK - unary function type with edge handle as argument
     */
    template <typename Node, typename Edge, class NODE_HIT_CALLBACK, class EDGE_HIT_CALLBACK>
    void graph_topology_linear_walk(
        const libkerat::internals::graph<Node, Edge> & graph,
        NODE_HIT_CALLBACK & node_callback,
        EDGE_HIT_CALLBACK & edge_callback
    ){
        typedef typename libkerat::internals::graph<Node, Edge>::const_node_iterator const_node_iterator;
        typedef typename libkerat::internals::graph<Node, Edge>::const_edge_iterator const_edge_iterator;
        const_node_iterator node = libkerat::internals::graph_utils::get_origin_leaf(graph);

        while (node != graph.nodes_end()){
            node_callback(*node);
            if (node->output_degree() > 0){
                const_edge_iterator edge = node->edges_begin();
                edge_callback(*edge);
                node = const_node_iterator(edge->to_node());
            } else {
                node = graph.nodes_end();
            }
        }
    }

    /**
     * \brief Iterates through the graph as through trunk-tree
     * \note Behaviour on topologies other than trunk-tree is undefined
     * \param graph - graph to iterate over
     * \param node_callback - callback to call when node is accessed
     * \param edge_callback - callback to call when edge is accessed
     * \param rollback_callback - callback to call when rolling back to the next branch
     * \tparam NODE_HIT_CALLBACK - unary function type with node handle as argument
     * \tparam EDGE_HIT_CALLBACK - unary function type with edge handle as argument
     * \tparam NODE_ROLLBACK_CALLBACK - unary function type with size_t steps count as argument
     */
    template <typename Node, typename Edge, class NODE_HIT_CALLBACK, class EDGE_HIT_CALLBACK, class NODE_ROLLBACK_CALLBACK>
    void graph_topology_trunk_tree_walk(
        const libkerat::internals::graph<Node, Edge> & graph,
        NODE_HIT_CALLBACK & node_callback,
        EDGE_HIT_CALLBACK & edge_callback,
        NODE_ROLLBACK_CALLBACK & rollback_callback
    ){
        typedef typename libkerat::internals::graph<Node, Edge>::const_node_iterator const_node_iterator;
        typedef typename libkerat::internals::graph<Node, Edge>::const_edge_iterator const_edge_iterator;

        const_node_iterator trunk_leaf = libkerat::internals::graph_utils::get_oriented_outin_leaf(graph.nodes_begin(), graph.nodes_end());

        assert(trunk_leaf != graph.nodes_end());

        typedef std::stack<const_edge_iterator> edge_stack;
        edge_stack return_edges;
        const_node_iterator next_node = trunk_leaf;


        while (next_node != graph.nodes_end()){
            size_t out_degree = next_node->output_degree();
            size_t in_degree = next_node->input_degree();

            assert(in_degree <= 1);

            node_callback(*next_node);
            // I'm not leaf
            switch (out_degree){
                case 0: {
                    // leaf reached
                    bool keep_rolling_back = true;
                    size_t rollbacks_count = 0;

                    const_node_iterator original_rollback_node = next_node;

                    while ((keep_rolling_back) && (!return_edges.empty())){
                        const_edge_iterator edge = return_edges.top();
                        return_edges.pop();
                        ++rollbacks_count;
                        const_node_iterator hndl = edge->from_node();
                        ++edge;

                        if (edge == hndl->edges_end()){
                            // this was last edge of given subtree, so return as far
                            // as possible leave the node as is, it will take care of
                            // the rest
                        // check another branch
                        } else {
                            return_edges.push(edge);
                            next_node = edge->to_node();
                            rollback_callback(*original_rollback_node, *next_node, rollbacks_count);
                            edge_callback(*edge);
                            keep_rolling_back = false;
                        }
                    }
                    if (return_edges.empty()){
                        // end of graph reached
                        next_node = graph.nodes_end();
                    }

                    break;
                }
                default: {
                    const_edge_iterator edge = next_node->edges_begin();
                    return_edges.push(edge);
                    edge_callback(*edge);
                    next_node = edge->to_node();
                }
            }
        }
    }

    template <typename Node, typename Edge, class NODE_HIT_CALLBACK, class EDGE_HIT_CALLBACK, class NODE_ROLLBACK_CALLBACK>
    void graph_topology_trunk_tree_walk(
        libkerat::internals::graph<Node, Edge> & graph,
        NODE_HIT_CALLBACK & node_callback,
        EDGE_HIT_CALLBACK & edge_callback,
        NODE_ROLLBACK_CALLBACK & rollback_callback
    ){
        typedef typename libkerat::internals::graph<Node, Edge>::node_iterator const_node_iterator;
        typedef typename libkerat::internals::graph<Node, Edge>::edge_iterator const_edge_iterator;
        typedef typename libkerat::internals::graph<Node, Edge>::node_handle const_node_handle;

        const_node_iterator trunk_leaf = libkerat::internals::graph_utils::get_oriented_outin_leaf(graph.nodes_begin(), graph.nodes_end());

        assert(trunk_leaf != graph.nodes_end());

        typedef std::stack<const_edge_iterator> edge_stack;
        edge_stack return_edges;
        const_node_iterator next_node = trunk_leaf;


        while (next_node != graph.nodes_end()){
            size_t out_degree = next_node->output_degree();
            size_t in_degree = next_node->input_degree();

            assert(in_degree <= 1);

            node_callback(*next_node);
            // I'm not leaf
            switch (out_degree){
                case 0: {
                    // leaf reached
                    bool keep_rolling_back = true;
                    size_t rollbacks_count = 0;

                    const_node_iterator original_rollback_node = next_node;

                    while ((keep_rolling_back) && (!return_edges.empty())){
                        const_edge_iterator edge = return_edges.top();
                        return_edges.pop();
                        ++rollbacks_count;
                        const_node_iterator hndl = edge->from_node();
                        ++edge;

                        if (edge == hndl->edges_end()){
                            // this was last edge of given subtree, so return as far
                            // as possible leave the node as is, it will take care of
                            // the rest
                        // check another branch
                        } else {
                            return_edges.push(edge);
                            next_node = edge->to_node();
                            rollback_callback(*original_rollback_node, *next_node, rollbacks_count);
                            edge_callback(*edge);
                            keep_rolling_back = false;
                        }
                    }
                    if (return_edges.empty()){
                        // end of graph reached
                        next_node = graph.nodes_end();
                    }

                    break;
                }
                default: {
                    const_edge_iterator edge = next_node->edges_begin();
                    return_edges.push(edge);
                    edge_callback(*edge);
                    next_node = edge->to_node();
                }
            }
        }
    }



}

#endif // KERAT_GRAPH_HPP

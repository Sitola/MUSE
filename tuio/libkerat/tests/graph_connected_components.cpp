/**
 * \file      multiplexing_adaptor_test.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-03-21 17:07 UTC+2
 * \copyright BSD
 */

#include <iostream>
#include <kerat/graph.hpp>
#include <cassert>
#include <list>
#include <map>
#include <algorithm>

using std::cout;
using std::cerr;
using std::endl;

using libkerat::internals::graph;
using libkerat::internals::graph_utils;
using namespace libkerat;

typedef graph<char, uint16_t> graph_type;

graph_type make_test_graph(){
    graph_type output;
    graph_type::node_iterator node_a = output.create_node('A');
    graph_type::node_iterator node_b = output.create_node('B');
    graph_type::node_iterator node_c = output.create_node('C');
    graph_type::node_iterator node_d = output.create_node('D');
    graph_type::node_iterator node_e = output.create_node('E');
    graph_type::node_iterator node_f = output.create_node('F');
    graph_type::node_iterator node_g = output.create_node('G');
    graph_type::node_iterator node_h = output.create_node('H');
    graph_type::node_iterator node_i = output.create_node('I');
    graph_type::node_iterator node_j = output.create_node('J');
    graph_type::node_iterator node_k = output.create_node('K');
    graph_type::node_iterator node_l = output.create_node('L');
    
    #define label(a, b) ((((uint16_t)(a)) << 8) | (b))
    // first connected component
    output.create_edge(*node_a, *node_b, label('a','b'));
    output.create_edge(*node_b, *node_c, label('b','c'));
    output.create_edge(*node_c, *node_a, label('c','a'));

    // bridge
    output.create_edge(*node_b, *node_d, label('b','d'));
    
    // second connected component
    output.create_edge(*node_d, *node_f, label('d','f'));
    output.create_edge(*node_f, *node_e, label('f','e'));
    output.create_edge(*node_e, *node_d, label('e','d'));
    output.create_edge(*node_f, *node_g, label('f','g'));
    output.create_edge(*node_g, *node_h, label('g','h'));
    output.create_edge(*node_h, *node_i, label('h','i'));
    output.create_edge(*node_i, *node_e, label('i','e'));
    output.create_edge(*node_i, *node_g, label('i','g'));
    
    // edge between two splitted nodes
    output.create_edge(*node_k, *node_l, label('k','l'));
    
    return output;
}

int main(){
    typedef std::list<graph_type> graph_list;
    graph_type tg = make_test_graph();
    graph_list components = graph_utils::split_strong_components_copy_1_0(tg);
    
    for (graph_list::const_iterator component = components.begin(); component != components.end(); ++component){
        cout << endl << "component nodes:";
        for (graph_type::const_node_iterator node = component->nodes_begin(); node != component->nodes_end(); ++node){
            cout << " " << node->get_value();
        }
        cout << endl << "component edges:";
        for (graph_type::const_edge_iterator edge = component->edges_begin(); edge != component->edges_end(); ++edge){
            cout << " " << std::hex << edge->get_value();
        }
        cout << endl;
    }

    return 0;
}


/**
 * \file      graph_isomorphy.cpp
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
#include <string>

using std::cout;
using std::cerr;
using std::endl;

using libkerat::internals::graph;
using libkerat::internals::graph_utils;
using namespace libkerat;

typedef graph<char, std::string> graph_type;

template <typename GRAPH_TYPE>
void print_graph(const GRAPH_TYPE & graf){
    cout << "uzly: ";
    for (typename GRAPH_TYPE::const_node_iterator n = graf.nodes_begin(); n != graf.nodes_end(); ++n){
        cout << n->get_value() << " ";
    }
    cout << endl;

    cout << "hrany: ";
    for (typename GRAPH_TYPE::const_edge_iterator n = graf.edges_begin(); n != graf.edges_end(); ++n){
        cout << n->from_node()->get_value() << "(" << n->get_value() << ")" << n->to_node()->get_value() << " ";
    }
    cout << endl;
}

int test_primitives(){
    graph_type gr1, gr2, gr3;
    
    graph_type::node_iterator g1na = gr1.create_node('a');
    
    graph_type::node_iterator g2n_tmp = gr2.create_node('t');
    gr2.remove_node(*g2n_tmp); g2n_tmp = gr2.nodes_end();
    graph_type::node_iterator g2na = gr2.create_node('a');

    graph_type::node_iterator g3na = gr3.create_node('b');
    
    assert(gr1 == gr2);
    assert(gr1 != gr3);
    assert(gr2 != gr3);
    
    // primitive, single verticle graphs tested
    gr3.remove_node(*g3na);
    g3na = gr3.create_node('a');
    
    // two vertices, single edge, same label
    graph_type::node_iterator g1na2 = gr1.create_node('a');
    graph_type::node_iterator g2na2 = gr2.create_node('a');
    graph_type::node_iterator g3na2 = gr3.create_node('b');
    
    gr1.create_edge(*g1na, *g1na2, "aa");
    gr2.create_edge(*g2na, *g2na2, "aa");
    gr3.create_edge(*g3na, *g3na2, "aa"); // should be ab
    
    assert(gr1 == gr2);
    assert(gr1 != gr3);
    assert(gr2 != gr3);
    
    return 0;
}

int test_k5(){
    graph_type gr1, gr2, gr3;
    
    graph_type::node_iterator g1na1 = gr1.create_node('a');
    graph_type::node_iterator g1na2 = gr1.create_node('a');
    graph_type::node_iterator g1na3 = gr1.create_node('a');
    graph_type::node_iterator g1na4 = gr1.create_node('a');
    graph_type::node_iterator g1na5 = gr1.create_node('a');
    
    gr1.create_edge(*g1na1, *g1na2, "obvod_a1a2");
    gr1.create_edge(*g1na2, *g1na3, "obvod_a2a3");
    gr1.create_edge(*g1na3, *g1na4, "obvod_a3a4");
    gr1.create_edge(*g1na4, *g1na5, "obvod_a4a5");
    gr1.create_edge(*g1na5, *g1na1, "obvod_a5a1");
    
    gr1.create_edge(*g1na3, *g1na1, "stred_b3b1");
    gr1.create_edge(*g1na1, *g1na4, "stred_b1b4");
    gr1.create_edge(*g1na4, *g1na2, "stred_b4b2");
    gr1.create_edge(*g1na2, *g1na5, "stred_b2b5");
    gr1.create_edge(*g1na5, *g1na3, "stred_b5b3");
    
    graph_type::node_iterator g2na1 = gr2.create_node('a');
    graph_type::node_iterator g2na3 = gr2.create_node('a');
    graph_type::node_iterator g2na4 = gr2.create_node('a');
    graph_type::node_iterator g2na5 = gr2.create_node('a');
    graph_type::node_iterator g2na2 = gr2.create_node('a');
    
    gr2.create_edge(*g2na1, *g2na2, "obvod_a1a2");
    gr2.create_edge(*g2na5, *g2na1, "obvod_a5a1");
    gr2.create_edge(*g2na3, *g2na4, "obvod_a3a4");
    gr2.create_edge(*g2na4, *g2na2, "stred_b4b2");
    gr2.create_edge(*g2na2, *g2na5, "stred_b2b5");

    gr2.create_edge(*g2na2, *g2na3, "obvod_a2a3");
    gr2.create_edge(*g2na4, *g2na5, "obvod_a4a5");
    gr2.create_edge(*g2na3, *g2na1, "stred_b3b1");
    gr2.create_edge(*g2na1, *g2na4, "stred_b1b4");
    gr2.create_edge(*g2na5, *g2na3, "stred_b5b3");
    
    graph_type::node_iterator g3na1 = gr3.create_node('a');
    graph_type::node_iterator g3na2 = gr3.create_node('a');
    graph_type::node_iterator g3na3 = gr3.create_node('a');
    graph_type::node_iterator g3na4 = gr3.create_node('a');
    graph_type::node_iterator g3na5 = gr3.create_node('a');
    
    gr3.create_edge(*g3na1, *g3na2, "obvod_a1a2");
    gr3.create_edge(*g3na2, *g3na3, "obvod_a2a3");
    gr3.create_edge(*g3na3, *g3na4, "obvod_a3a4");
    gr3.create_edge(*g3na4, *g3na5, "obvod_a4a5");
    gr3.create_edge(*g3na5, *g3na1, "obvod_a5a1");
    
    gr3.create_edge(*g3na3, *g3na1, "stred_b3b1");
    gr3.create_edge(*g3na1, *g3na4, "stred_b1b4");
    gr3.create_edge(*g3na2, *g3na4, "stred_b4b2");
    gr3.create_edge(*g3na2, *g3na5, "stred_b2b5");
    gr3.create_edge(*g3na5, *g3na3, "stred_b5b3");
    
    assert(gr1 == gr2);
    assert(gr1 != gr3);
    assert(gr2 != gr3);
    
    return 0;
}

int test_c6(){
    graph_type gr1, gr2;
    
    graph_type::node_iterator g1na1 = gr1.create_node('a');
    graph_type::node_iterator g1na2 = gr1.create_node('a');
    graph_type::node_iterator g1na3 = gr1.create_node('b');
    graph_type::node_iterator g1na4 = gr1.create_node('a');
    graph_type::node_iterator g1na5 = gr1.create_node('a');
    graph_type::node_iterator g1na6 = gr1.create_node('b');
    graph_type::node_iterator g1na7 = gr1.create_node('c');
    graph_type::node_iterator g1na8 = gr1.create_node('c');
    
    gr1.create_edge(*g1na1, *g1na2, "left");
    gr1.create_edge(*g1na4, *g1na5, "right");
    gr1.create_edge(*g1na2, *g1na3, "upper left");
    gr1.create_edge(*g1na3, *g1na4, "upper right");
    gr1.create_edge(*g1na5, *g1na6, "lower right");
    gr1.create_edge(*g1na6, *g1na1, "lower left");
    gr1.create_edge(*g1na1, *g1na7, "lower leaf");
    gr1.create_edge(*g1na2, *g1na8, "upper leaf");
    
    
    graph_type::node_iterator g2na1 = gr2.create_node('a');
    graph_type::node_iterator g2na3 = gr2.create_node('b');
    graph_type::node_iterator g2na4 = gr2.create_node('a');
    graph_type::node_iterator g2na7 = gr2.create_node('c');
    graph_type::node_iterator g2na5 = gr2.create_node('a');
    graph_type::node_iterator g2na6 = gr2.create_node('b');
    graph_type::node_iterator g2na8 = gr2.create_node('c');
    graph_type::node_iterator g2na2 = gr2.create_node('a');
    
    gr2.create_edge(*g2na1, *g2na2, "left");
    gr2.create_edge(*g2na4, *g2na5, "right");
    gr2.create_edge(*g2na1, *g2na7, "lower leaf");
    gr2.create_edge(*g2na2, *g2na3, "upper left");
    gr2.create_edge(*g2na5, *g2na6, "lower right");
    gr2.create_edge(*g2na4, *g2na8, "upper leaf");
    gr2.create_edge(*g2na6, *g2na1, "lower left");
    gr2.create_edge(*g2na3, *g2na4, "upper right");
    
    assert(gr1 != gr2);
    
    return 0;
}

int main(){

    int retval = 0;
    retval |= test_primitives();
    retval |= test_k5();
    retval |= test_c6();

    return retval;
}


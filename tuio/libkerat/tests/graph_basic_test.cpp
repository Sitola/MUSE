/**
 * \file      graph_basic_test.cpp
 * \author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-04-01 13:07 UTC+1
 * \copyright BSD
 */

#include <kerat/graph.hpp>
#include <kerat/utils.hpp>
#include <iostream>
#include <cassert>


using std::cerr;
using std::cout;
using std::endl;

int test_1(){

    typedef libkerat::internals::graph<int, int> graph_type;
    typedef graph_type::node_iterator node_iterator;
    typedef graph_type::const_node_iterator const_node_iterator;
    typedef graph_type::edge_iterator edge_iterator;
    typedef graph_type::const_edge_iterator const_edge_iterator;

    graph_type graph;
    node_iterator first_node = graph.create_node();
    first_node->set_value(1);
    node_iterator second_node = graph.create_node(2);
    node_iterator third_node = graph.create_node(3);

    assert(graph.empty() == false);

    cout << "initial graph:" << endl;
    for (node_iterator i = graph.nodes_begin(); i != graph.nodes_end(); i++){
        cout << "node id: " << i->get_node_id() << " value: " << i->get_value() << endl;
        for (edge_iterator j = i->edges_begin(); j != i->edges_end(); j++){
            cout << "    edge from: " << j->from_node()->get_node_id() << " to: " << j->to_node()->get_node_id() << " value: " << j->get_value() << endl;
        }
    }

    edge_iterator first_edge = graph.create_edge(*first_node, *second_node);
    first_edge->set_value(21);
    edge_iterator second_edge = graph.create_edge(*second_node, *third_node, 22);
    edge_iterator third_edge = graph.create_edge(*third_node, *first_node, 23);

    cout << endl << "graph with edges:" << endl;
    for (node_iterator i = graph.nodes_begin(); i != graph.nodes_end(); i++){
        cout << "node id: " << i->get_node_id() << " value: " << i->get_value() << endl;
        for (edge_iterator j = i->edges_begin(); j != i->edges_end(); j++){
            cout << "    edge from: " << j->from_node()->get_node_id() << " to: " << j->to_node()->get_node_id() << " value: " << j->get_value() << endl;
        }
    }

    cout << endl << "edges only:" << endl;
    for (edge_iterator j = graph.edges_begin(); j != graph.edges_end(); j++){
        cout << "from: " << j->from_node()->get_node_id() << " to: " << j->to_node()->get_node_id() << " value: " << j->get_value() << endl;
    }

    graph.remove_edge(*second_edge);
    cout << endl << "remove edge:" << endl;
    for (edge_iterator j = graph.edges_begin(); j != graph.edges_end(); j++){
        cout << "from: " << j->from_node()->get_node_id() << " to: " << j->to_node()->get_node_id() << " value: " << j->get_value() << endl;
    }

    cout << endl << "Remove... " << third_node->get_node_id() << endl;
    graph.remove_node(*third_node);
    for (node_iterator i = graph.nodes_begin(); i != graph.nodes_end(); i++){
        cout << "node id: " << i->get_node_id() << " value: " << i->get_value() << endl;
        for (edge_iterator j = i->edges_begin(); j != i->edges_end(); j++){
            cout << "    edge from: " << j->from_node()->get_node_id() << " to: " << j->to_node()->get_node_id() << " value: " << j->get_value() << endl;
        }
    }

    third_node = graph.create_node(3);
    cout << endl << "Add... " << third_node->get_node_id() << endl;
    for (node_iterator i = graph.nodes_begin(); i != graph.nodes_end(); i++){
        cout << "node id: " << i->get_node_id() << " value: " << i->get_value() << endl;
        for (edge_iterator j = i->edges_begin(); j != i->edges_end(); j++){
            cout << "    edge from: " << j->from_node()->get_node_id() << " to: " << j->to_node()->get_node_id() << " value: " << j->get_value() << endl;
        }
    }

    cout << endl << "const edges only:" << endl;
    const graph_type & graph2 = graph;
    for (const_edge_iterator j = graph2.edges_begin(); j != graph2.edges_end(); j++){
        cout << "from: " << j->from_node()->get_node_id() << " to: " << j->to_node()->get_node_id() << " value: " << j->get_value() << endl;
    }

    return 0;
}

int test_2(){

    typedef libkerat::internals::graph<int, int> graph_type;
    typedef graph_type::node_iterator node_iterator;
    typedef graph_type::const_node_iterator const_node_iterator;
    typedef graph_type::edge_iterator edge_iterator;
    typedef graph_type::const_edge_iterator const_edge_iterator;

    {
        graph_type graph1;
        node_iterator node_a = graph1.create_node();
        node_iterator node_b = graph1.create_node();
        node_iterator node_c = graph1.create_node();
        node_iterator node_d = graph1.create_node();

        graph1.create_edge(*node_a, *node_b);
        graph1.create_edge(*node_b, *node_d);
        graph1.create_edge(*node_a, *node_c);
        graph1.create_edge(*node_c, *node_d);

        const graph_type * cgraph = &graph1;
        graph_type * mgraph = &graph1;

        const_edge_iterator cb = cgraph->edges_begin();
        edge_iterator b = mgraph->edges_begin();
        const_edge_iterator ce = cgraph->edges_end();
        edge_iterator e = mgraph->edges_end();

        cout << endl << "graph1:" << endl;
        for (const_edge_iterator j = graph1.edges_begin(); j != graph1.edges_end(); j++){
            cout << "from: " << j->from_node()->get_node_id() << " to: " << j->to_node()->get_node_id() << endl;
        }
        cout << "graph1 contains oriented cycle: " << libkerat::graph_contains_cycle_oriented(graph1) << endl;
        cout << "graph1 contains unoriented cycle: " << libkerat::graph_contains_cycle_oriented(graph1) << endl;
    }

    {
        graph_type graph2;
        node_iterator node_a = graph2.create_node();
        node_iterator node_b = graph2.create_node();
        node_iterator node_c = graph2.create_node();
        node_iterator node_d = graph2.create_node();

        graph2.create_edge(*node_a, *node_b);
        graph2.create_edge(*node_b, *node_c);
        graph2.create_edge(*node_b, *node_d);
        graph2.create_edge(*node_d, *node_a);

        cout << endl << "graph2:" << endl;
        for (const_edge_iterator j = graph2.edges_begin(); j != graph2.edges_end(); j++){
            cout << "from: " << j->from_node()->get_node_id() << " to: " << j->to_node()->get_node_id() << endl;
        }
        cout << "graph2 contains oriented cycle: " << libkerat::graph_contains_cycle_oriented(graph2) << endl;
        cout << "graph2 contains unoriented cycle: " << libkerat::graph_contains_cycle_unoriented(graph2) << endl;
    }

    {
        graph_type graph3;
        node_iterator node_a = graph3.create_node();
        node_iterator node_b = graph3.create_node();
        node_iterator node_c = graph3.create_node();
        node_iterator node_d = graph3.create_node();
        node_iterator node_e = graph3.create_node();
        node_iterator node_f = graph3.create_node();

        graph3.create_edge(*node_a, *node_b);
        graph3.create_edge(*node_a, *node_c);
        graph3.create_edge(*node_d, *node_b);
        graph3.create_edge(*node_e, *node_d);
        graph3.create_edge(*node_f, *node_d);
        graph3.create_edge(*node_e, *node_f);

        cout << endl << "graph3:" << endl;
        for (const_edge_iterator j = graph3.edges_begin(); j != graph3.edges_end(); j++){
            cout << "from: " << j->from_node()->get_node_id() << " to: " << j->to_node()->get_node_id() << endl;
        }
        cout << "graph3 contains oriented cycle: " << libkerat::graph_contains_cycle_oriented(graph3) << endl;
        cout << "graph3 contains unoriented cycle: " << libkerat::graph_contains_cycle_unoriented(graph3) << endl;
    }

    {
        graph_type graph4;
        node_iterator node_a = graph4.create_node();
        node_iterator node_b = graph4.create_node();
        node_iterator node_c = graph4.create_node();
        node_iterator node_d = graph4.create_node();
        node_iterator node_e = graph4.create_node();
        node_iterator node_f = graph4.create_node();

        graph4.create_edge(*node_b, *node_a);
        graph4.create_edge(*node_a, *node_c);
        graph4.create_edge(*node_c, *node_e);
        graph4.create_edge(*node_d, *node_b);
        graph4.create_edge(*node_e, *node_d);
        graph4.create_edge(*node_f, *node_d);
        graph4.create_edge(*node_e, *node_f);

        cout << endl << "graph4:" << endl;
        for (const_edge_iterator j = graph4.edges_begin(); j != graph4.edges_end(); j++){
            cout << "from: " << j->from_node()->get_node_id() << " to: " << j->to_node()->get_node_id() << endl;
        }
        cout << "graph4 contains oriented cycle: " << libkerat::graph_contains_cycle_oriented(graph4) << endl;
        cout << "graph4 contains unoriented cycle: " << libkerat::graph_contains_cycle_unoriented(graph4) << endl;
    }

    {
        graph_type graph5;
        node_iterator node_a = graph5.create_node();
        node_iterator node_b = graph5.create_node();
        node_iterator node_c = graph5.create_node();
        node_iterator node_d = graph5.create_node();

        graph5.create_edge(*node_a, *node_b);
        graph5.create_edge(*node_b, *node_d);
        graph5.create_edge(*node_a, *node_c);

        cout << endl << "graph5:" << endl;
        for (const_edge_iterator j = graph5.edges_begin(); j != graph5.edges_end(); j++){
            cout << "from: " << j->from_node()->get_node_id() << " to: " << j->to_node()->get_node_id() << endl;
        }
        cout << "graph5 contains oriented cycle: " << libkerat::graph_contains_cycle_oriented(graph5) << endl;
        cout << "graph5 contains unoriented cycle: " << libkerat::graph_contains_cycle_unoriented(graph5) << endl;
    }

    return 0;

}

int test_3(){

    typedef libkerat::internals::graph<char, int> graph_type;
    typedef graph_type::node_iterator node_iterator;
    typedef graph_type::const_node_iterator const_node_iterator;
    typedef graph_type::const_edge_iterator const_edge_iterator;
    typedef graph_type::edge_iterator edge_iterator;

    {
        graph_type trunk1;
        node_iterator node_a = trunk1.create_node('A');
        node_iterator node_b = trunk1.create_node('B');
        node_iterator node_c = trunk1.create_node('C');
        node_iterator node_d = trunk1.create_node('D');
        node_iterator node_e = trunk1.create_node('E');
        node_iterator node_f = trunk1.create_node('F');
        node_iterator node_g = trunk1.create_node('G');
        node_iterator node_h = trunk1.create_node('H');
        node_iterator node_i = trunk1.create_node('I');

        trunk1.create_edge(*node_a, *node_b, 1);
        trunk1.create_edge(*node_b, *node_c, 2);
        trunk1.create_edge(*node_c, *node_d, 2);
        trunk1.create_edge(*node_c, *node_e, 3);
        trunk1.create_edge(*node_e, *node_f, 3);
        trunk1.create_edge(*node_f, *node_g, 2);
        trunk1.create_edge(*node_g, *node_h, 1);
        trunk1.create_edge(*node_g, *node_i, 1);

        cout << endl << "trunk1:" << endl;
        for (const_edge_iterator j = trunk1.edges_begin(); j != trunk1.edges_end(); j++){
            cout << "from: " << j->from_node()->get_node_id() << " to: " << j->to_node()->get_node_id() << endl;
        }
        cout << "trunk1 is trunk: " << libkerat::graph_topology_is_trunk_tree(trunk1) << endl;
    }

    return 0;

}

int test_4(){


    return 0;

}

int main(){
//    if (test_1()){ return 1; }
//    if (test_2()){ return 2; }
//    if (test_3()){ return 3; }
    if (test_4()){ return 4; }

    return 0;
}

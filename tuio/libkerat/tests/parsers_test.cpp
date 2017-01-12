/**
 * \file      multiplexing_adaptor_test.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-03-21 17:07 UTC+2
 * \copyright BSD
 */

#include <iostream>
#include <kerat/kerat.hpp>
#include <kerat/parsers.hpp>
#include <lo/lo.h>
#include <cassert>
#include "../config.h"

using std::cout;
using std::cerr;
using std::endl;

struct testsuite: protected libkerat::server {
    testsuite()
        :m_result(false), m_convertor(NULL, NULL)
    { ; }
    
    static int lo_message_handler(
        const char *path __attribute__((unused)), 
        const char *types __attribute__((unused)), 
        lo_arg **argv __attribute__((unused)), 
        int argc __attribute__((unused)),
        lo_message msg __attribute__((unused)), 
        void *user_data __attribute__((unused))
    ){
        testsuite * cb = static_cast<testsuite *>(user_data);
        if (cb == NULL){ return -1; }
        cb->m_result = (*cb->m_convertor.m_callback)(cb->m_convertor_result, path, types, argv, argc, cb->m_convertor.m_user_data);
        return cb->m_result;
    }
    
    static lo_address local_lo_address(lo_server server){
        lo_address tmp = lo_address_new_from_url(lo_server_get_url(server));
        const char * port = lo_address_get_port(tmp);
        char port_tmp[30];
        strcpy(port_tmp, port);
        lo_address_free(tmp);
        return lo_address_new_with_proto(
            lo_server_get_protocol(server),
            "127.0.0.1",
            port_tmp
        );
    }
    
    static void lo_error_handler(int num, const char *msg, const char *where){
        std::cerr << "msg: " << msg << " where: " << where << std::endl;
    }

    std::list<libkerat::kerat_message *> m_convertor_result;
    bool m_result;
    libkerat::internals::callback_setting<libkerat::simple_client::message_convertor> m_convertor;

    int run();

    // just dummies
    virtual bool append_clone(const libkerat::kerat_message* msg __attribute__((unused))){ return true; }
    virtual bool prepare_bundle(){ return true; }
    virtual bool commit(){ return true; }
    virtual bool run_adaptors(bool paranoid = false){ return true; }
    virtual bool output_check(){ return true; }
    
    void clean(){
        m_result = 0;
        for (std::list<libkerat::kerat_message *>::iterator i = m_convertor_result.begin(); i != m_convertor_result.end(); i++){
            delete *i;
        }
        m_convertor_result.clear();
        m_convertor.m_callback = NULL;
    }

    bool test_alive();
    bool test_frame();

    bool test_pointer_2d();
    bool test_pointer_3d();
    bool test_token_2d();
    bool test_token_3d();
    bool test_bounds_2d();
    bool test_bounds_3d();
    bool test_symbol_string();
    bool test_symbol_blob();

    bool test_control();
    bool test_data_blob();
    bool test_data_string();
    bool test_signal();

    bool test_convex_hull();
    bool test_outer_contour();
    bool test_inner_contour();
    bool test_skeleton_2d();
    bool test_skeleton_3d();
    bool test_skeleton_volume();
    bool test_area();
    bool test_raw();

    bool test_alive_associations();
    bool test_container_association();
    bool test_link_association();
    bool test_linked_list_association();
    bool test_linked_tree_association();

};

int testsuite::run(){
    bool retval = true;
    retval &= test_frame();
    retval &= test_alive();

    retval &= test_pointer_2d();
    retval &= test_pointer_3d();
    retval &= test_token_2d();
    retval &= test_token_3d();
    retval &= test_bounds_2d();
    retval &= test_bounds_3d();
    retval &= test_symbol_string();
    retval &= test_symbol_blob();

    retval &= test_control();
    retval &= test_data_blob();
    retval &= test_data_string();
    retval &= test_signal();

    retval &= test_convex_hull();
    retval &= test_outer_contour();
    retval &= test_inner_contour();
    retval &= test_skeleton_2d();
    retval &= test_skeleton_3d();
    retval &= test_skeleton_volume();
    retval &= test_area();
    retval &= test_raw();

    retval &= test_alive_associations();
    retval &= test_container_association();
    retval &= test_link_association();
    retval &= test_linked_list_association();
    retval &= test_linked_tree_association();

    return !retval;
}

bool testsuite::test_alive(){
    clean();
    
    // message setup
    typedef libkerat::message::alive msg_type;
    m_convertor.m_callback = libkerat::internals::parsers::parse_alv;

    libkerat::message::alive::alive_ids alives;
    alives.insert(2);
    alives.insert(3);
    alives.insert(4);
    alives.insert(5);
    const msg_type msg(alives);
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH << " OK" << endl;
    return true;
} 

bool testsuite::test_frame(){
    clean();

    // message setup
    typedef libkerat::message::frame msg_type;
    const msg_type msg(0, LO_TT_IMMEDIATE, "Frame test", 0, 1, 1920, 1080);
    m_convertor.m_callback = libkerat::internals::parsers::parse_frm;
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);

    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH << " OK" << endl;
    return true;
} 

bool testsuite::test_control(){
    clean();

    // message setup
    typedef libkerat::message::control msg_type;
    libkerat::message::control::controls_list controls;
        controls.push_back(0.9);
        controls.push_back(0.8);
        controls.push_back(0.87);
        controls.push_back(0.6);
    const msg_type msg(90, controls);
    m_convertor.m_callback = libkerat::internals::parsers::parse_ctl;
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH << " OK" << endl;
    return true;
} 

bool testsuite::test_data_blob(){
    clean();

    // message setup
    typedef libkerat::message::data msg_type;
    
    size_t data_len = 200;
    uint8_t data[data_len];
    for (size_t i = 0; i < data_len; i++){ data[i] = rand(); }
    const msg_type msg(91, data_len, data, "data/random");
    m_convertor.m_callback = libkerat::internals::parsers::parse_dat;
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH << " OK" << endl;
    return true;
} 

bool testsuite::test_data_string(){
    clean();

    // message setup
    typedef libkerat::message::data msg_type;
    
    const msg_type msg(93, 
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec turpis justo,"
        "dignissim vel suscipit sit amet, viverra in augue. Aliquam lacinia turpis sit "
        "amet dolor rutrum rutrum cursus felis viverra. Ut sit amet augue vitae est "
        "auctor sollicitudin vehicula sed risus. Phasellus sit amet auctor est. Phasellus "
        "nec vehicula velit. Nullam ac sapien ipsum, in euismod dui. Morbi vitae odio nec "
        "metus pellentesque congue. "
    , "text/plain");
    m_convertor.m_callback = libkerat::internals::parsers::parse_dat;
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH << " OK" << endl;
    return true;
} 

bool testsuite::test_signal(){
    clean();

    // message setup
    typedef libkerat::message::signal msg_type;
    libkerat::message::signal::target_ids targets;
    targets.insert(2);
    targets.insert(3);
    targets.insert(4);
    targets.insert(5);
    
    
    const msg_type msg(93, 2, targets);
    m_convertor.m_callback = libkerat::internals::parsers::parse_sig;
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH << " OK" << endl;
    return true;
} 

bool testsuite::test_alive_associations(){
    clean();
    
    // message setup
    typedef libkerat::message::alive_associations msg_type;
    m_convertor.m_callback = libkerat::internals::parsers::parse_ala;

    libkerat::message::alive_associations::associated_ids associations;
    associations.insert(2);
    associations.insert(3);
    associations.insert(4);
    associations.insert(5);
    const msg_type msg(associations);
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH << " OK" << endl;
    return true;
} 

bool testsuite::test_container_association(){
    clean();
    
    // message setup
    typedef libkerat::message::container_association msg_type;
    m_convertor.m_callback = libkerat::internals::parsers::parse_coa;

    libkerat::message::container_association::associated_ids associations;
    associations.insert(2);
    associations.insert(3);
    associations.insert(4);
    associations.insert(5);
    const msg_type msg(93, 11, associations);
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH << " OK" << endl;
    return true;
} 

bool testsuite::test_skeleton_volume(){
    clean();

    // message setup
    typedef libkerat::message::skeleton_volume msg_type;
    libkerat::message::skeleton_volume::radius_list radiuses;
        radiuses.push_back(0.9);
        radiuses.push_back(0.8);
        radiuses.push_back(0.87);
        radiuses.push_back(0.6);
    const msg_type msg(90, radiuses);
    m_convertor.m_callback = libkerat::internals::parsers::parse_svg;
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH << " OK" << endl;
    return true;
} 

bool testsuite::test_convex_hull(){
    clean();

    // message setup
    typedef libkerat::message::convex_hull msg_type;
    libkerat::message::convex_hull::point_2d_list points;
        points.push_back(libkerat::helpers::point_2d(0.3, 10));
        points.push_back(libkerat::helpers::point_2d(0.8, 100));
        points.push_back(libkerat::helpers::point_2d(100, 30));
        points.push_back(libkerat::helpers::point_2d(10, 30));
    const msg_type msg(90, points);
    m_convertor.m_callback = libkerat::internals::parsers::parse_chg;
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH << " OK" << endl;
    return true;
} 

bool testsuite::test_outer_contour(){
    clean();

    // message setup
    typedef libkerat::message::outer_contour msg_type;
    libkerat::message::outer_contour::point_2d_list points;
        points.push_back(libkerat::helpers::point_2d(0.3, 10));
        points.push_back(libkerat::helpers::point_2d(0.8, 100));
        points.push_back(libkerat::helpers::point_2d(100, 30));
        points.push_back(libkerat::helpers::point_2d(10, 30));
    const msg_type msg(90, points);
    m_convertor.m_callback = libkerat::internals::parsers::parse_ocg;
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH << " OK" << endl;
    return true;
} 

bool testsuite::test_inner_contour(){
    clean();

    // message setup
    typedef libkerat::message::inner_contour msg_type;
    libkerat::message::inner_contour::contour_list contours;
    libkerat::message::inner_contour::point_2d_list points1;
        points1.push_back(libkerat::helpers::point_2d(0.3, 10));
        points1.push_back(libkerat::helpers::point_2d(0.8, 100));
        points1.push_back(libkerat::helpers::point_2d(100, 30));
        points1.push_back(libkerat::helpers::point_2d(10, 30));
        contours.push_back(points1);
    libkerat::message::inner_contour::point_2d_list points2;
        points2.push_back(libkerat::helpers::point_2d(10.3, 10));
        points2.push_back(libkerat::helpers::point_2d(10.8, 100));
        points2.push_back(libkerat::helpers::point_2d(1100, 30));
        points2.push_back(libkerat::helpers::point_2d(110, 30));
        contours.push_back(points2);
        
    const msg_type msg(90, contours);
    m_convertor.m_callback = libkerat::internals::parsers::parse_icg;
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH << " OK" << endl;
    return true;
} 

bool testsuite::test_area(){
    clean();

    // message setup
    typedef libkerat::message::area msg_type;
    
    libkerat::message::area::span_map spans;
    spans.insert(libkerat::message::area::span(libkerat::helpers::point_2d(0.3, 10), 10));
    spans.insert(libkerat::message::area::span(libkerat::helpers::point_2d(0.8, 10), 10));
    spans.insert(libkerat::message::area::span(libkerat::helpers::point_2d(0.5, 10), 10));
    spans.insert(libkerat::message::area::span(libkerat::helpers::point_2d(10.3, 10), 10));
        
    const msg_type msg(90, spans);
    m_convertor.m_callback = libkerat::internals::parsers::parse_arg;
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH << " OK" << endl;
    return true;
} 

bool testsuite::test_raw(){
    clean();

    // message setup
    typedef libkerat::message::raw msg_type;
    
    size_t data_len = 200;
    uint8_t data[data_len];
    for (size_t i = 0; i < data_len; i++){ data[i] = rand(); }
    const msg_type msg(91, 1, data_len, data);
    m_convertor.m_callback = libkerat::internals::parsers::parse_raw;
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH << " OK" << endl;
    return true;
} 

bool testsuite::test_pointer_3d(){
    clean();

    // message setup
    typedef libkerat::message::pointer msg_type;
    
    msg_type msg(
        30, libkerat::helpers::contact_type_user::TYPEID_HEAD, 3, 4, 
        10.2, 10.3, 10.3, 
        10, 1.0, 10, 11.1, 11.2, 13
    );
    msg.set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_3D);
    m_convertor.m_callback = libkerat::internals::parsers::parse_ptr_3d;
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH_3D, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH_2D << " OK" << endl;
    cout << msg_type::PATH_3D << " OK" << endl;
    return true;
} 

bool testsuite::test_pointer_2d(){
    clean();

    // message setup
    typedef libkerat::message::pointer msg_type;
    
    msg_type msg(
        30, libkerat::helpers::contact_type_user::TYPEID_HEAD, 3, 4, 
        10.2, 10.3,
        10, 1.0, 
        10.0, 11.1, 13.0
    );

    
    msg.set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_2D);
    m_convertor.m_callback = libkerat::internals::parsers::parse_ptr_2d;
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH_2D, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH_2D << " OK" << endl;
    cout << msg_type::PATH_3D << " OK" << endl;
    return true;
} 

bool testsuite::test_bounds_3d(){
    clean();

    // message setup
    typedef libkerat::message::bounds msg_type;
    
    msg_type msg(30,
                11, 12, 14,
                1.0, 1.2, 1.3,
                10, 20, 30,
                6000,
                0.2, 0.3, 0.4,
                1.4, 1.5, 1.6,
                1000, 10000
            );
    msg.set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_3D);
    m_convertor.m_callback = libkerat::internals::parsers::parse_bnd_3d;
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH_3D, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH_2D << " OK" << endl;
    cout << msg_type::PATH_3D << " OK" << endl;
    return true;
} 

bool testsuite::test_bounds_2d(){
    clean();

    // message setup
    typedef libkerat::message::bounds msg_type;
    
    msg_type msg(40,
                20.3, 49.0,
                3.14,
                10, 10,
                100,
                0.5, 0.5,
                0.2,
                1, 3
            );

    
    msg.set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_2D);
    m_convertor.m_callback = libkerat::internals::parsers::parse_bnd_2d;
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH_2D, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH_2D << " OK" << endl;
    cout << msg_type::PATH_3D << " OK" << endl;
    return true;
} 

bool testsuite::test_token_3d(){
    clean();

    // message setup
    typedef libkerat::message::token msg_type;
    
    msg_type msg(32, 11, 500, 3, 
       29.1, 29.2, 29.3, 
       1.2, 1.3, 1.4,
       11.1, 11.2, 11.3, 
       0.1, 0.2, 0.3, 
       2050, 300
    );
    msg.set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_3D);
    m_convertor.m_callback = libkerat::internals::parsers::parse_tok_3d;
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH_3D, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH_2D << " OK" << endl;
    cout << msg_type::PATH_3D << " OK" << endl;
    return true;
} 

bool testsuite::test_token_2d(){
    clean();

    // message setup
    typedef libkerat::message::token msg_type;
    
    msg_type msg(32, 11, 500, 3, 
       29.1, 29.2,
       1.2,
       11.1, 11.2,
       0.1,
       2050, 300
    );
    
    msg.set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_2D);
    m_convertor.m_callback = libkerat::internals::parsers::parse_tok_2d;
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH_2D, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH_2D << " OK" << endl;
    cout << msg_type::PATH_3D << " OK" << endl;
    return true;
} 

bool testsuite::test_symbol_blob(){
    clean();

    // message setup
    typedef libkerat::message::symbol msg_type;
    
    size_t data_len = 200;
    uint8_t data[data_len];
    for (size_t i = 0; i < data_len; i++){ data[i] = rand(); }
    const msg_type msg(91, 30, 500, 2, "data/random", data_len, data);
    m_convertor.m_callback = libkerat::internals::parsers::parse_sym;
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH << " OK" << endl;
    return true;
} 

bool testsuite::test_symbol_string(){
    clean();

    // message setup
    typedef libkerat::message::symbol msg_type;
    
    const msg_type msg(93, 40, 503, 3, "text/plain", 
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec turpis justo,"
        "dignissim vel suscipit sit amet, viverra in augue. Aliquam lacinia turpis sit "
        "amet dolor rutrum rutrum cursus felis viverra. Ut sit amet augue vitae est "
        "auctor sollicitudin vehicula sed risus. Phasellus sit amet auctor est. Phasellus "
        "nec vehicula velit. Nullam ac sapien ipsum, in euismod dui. Morbi vitae odio nec "
        "metus pellentesque congue. ");
    m_convertor.m_callback = libkerat::internals::parsers::parse_sym;
    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH << " OK" << endl;
    return true;
} 

bool testsuite::test_skeleton_2d(){
    clean();

    // message setup
    typedef libkerat::message::skeleton msg_type;
    libkerat::message::skeleton::skeleton_graph grph;
    typedef libkerat::message::skeleton::skeleton_graph::node_iterator iterator;
    iterator h1 = grph.create_node(libkerat::helpers::point_2d(0.3, 10));
    iterator h2 = grph.create_node(libkerat::helpers::point_2d(0.8, 100));
    grph.create_edge(*h1, *h2);
    iterator h3 = grph.create_node(libkerat::helpers::point_2d(100, 30));
    grph.create_edge(*h2, *h3);
    iterator h4 = grph.create_node(libkerat::helpers::point_2d(10, 30));
    grph.create_edge(*h2, *h4);
    
    msg_type msg(90, grph);
    msg.set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_2D);
    m_convertor.m_callback = libkerat::internals::parsers::parse_skg_2d;    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH_2D, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH_2D << " OK" << endl;
    cout << msg_type::PATH_3D << " OK" << endl;
    return true;
} 

bool testsuite::test_skeleton_3d(){
    clean();

    // message setup
    typedef libkerat::message::skeleton msg_type;
    libkerat::message::skeleton::skeleton_graph grph;
    typedef libkerat::message::skeleton::skeleton_graph::node_iterator iterator;
    iterator h1 = grph.create_node(libkerat::helpers::point_3d(0.3, 10, 8));
    iterator h2 = grph.create_node(libkerat::helpers::point_3d(0.8, 100, 110));
    grph.create_edge(*h1, *h2);
    iterator h3 = grph.create_node(libkerat::helpers::point_3d(100, 30, 150));
    grph.create_edge(*h2, *h3);
    iterator h4 = grph.create_node(libkerat::helpers::point_3d(10, 30, 200));
    grph.create_edge(*h2, *h4);
    
    msg_type msg(90, grph);
    msg.set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_3D);
    m_convertor.m_callback = libkerat::internals::parsers::parse_skg_3d;    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH_3D, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH_2D << " OK" << endl;
    cout << msg_type::PATH_3D << " OK" << endl;
    return true;
} 

bool testsuite::test_link_association(){
    clean();

    // message setup
    typedef libkerat::message::link_association msg_type;
    typedef msg_type::internal_link_graph internal_link_graph;
    typedef msg_type::link_entry link_entry;
    typedef internal_link_graph::node_iterator iterator;

    internal_link_graph grph;
    
    iterator h1 = grph.create_node(2);
    iterator h2 = grph.create_node(3);
    grph.create_edge(*h1, *h2, link_entry(10, 20));
    iterator h3 = grph.create_node(4);
    grph.create_edge(*h1, *h3, link_entry(20, 30));
    iterator h4 = grph.create_node(5);
    grph.create_edge(*h1, *h4, link_entry(30, 40));
    
    msg_type msg(msg_type::LINK_PHYSICAL, grph);
    m_convertor.m_callback = libkerat::internals::parsers::parse_lia;    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH << " OK" << endl;
    return true;
} 

bool testsuite::test_linked_list_association(){
    clean();

    // message setup
    typedef libkerat::message::linked_list_association msg_type;
    typedef msg_type::internal_link_graph internal_link_graph;
    typedef msg_type::link_entry link_entry;
    typedef internal_link_graph::node_iterator iterator;

    internal_link_graph grph;
    
    iterator h1 = grph.create_node(2);
    iterator h2 = grph.create_node(3);
    grph.create_edge(*h1, *h2, link_entry(10, 20));
    iterator h3 = grph.create_node(4);
    grph.create_edge(*h2, *h3, link_entry(20, 30));
    iterator h4 = grph.create_node(5);
    grph.create_edge(*h3, *h4, link_entry(30, 40));
    
    msg_type msg(msg_type::LINK_PHYSICAL, grph);
    m_convertor.m_callback = libkerat::internals::parsers::parse_lla;    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH << " OK" << endl;
    return true;
} 

bool testsuite::test_linked_tree_association(){
#ifndef DRAFT_NONCOMPLIANT
    std::cerr << "LTA message is not supported in TUIO 2.0 draft compliant mode!" << std::endl;
    return true;
#endif // DRAFT_NONCOMPLIANT

    clean();

    // message setup
    typedef libkerat::message::linked_tree_association msg_type;
    typedef msg_type::internal_link_graph internal_link_graph;
    typedef msg_type::link_entry link_entry;
    typedef internal_link_graph::node_iterator iterator;

    internal_link_graph grph;
    
    iterator h1 = grph.create_node(2);
    iterator h2 = grph.create_node(3);
    grph.create_edge(*h1, *h2, link_entry(10, 20));
    iterator h3 = grph.create_node(4);
    grph.create_edge(*h2, *h3, link_entry(20, 30));
    iterator h4 = grph.create_node(5);
    grph.create_edge(*h2, *h4, link_entry(30, 40));
    
    msg_type msg(msg_type::LINK_PHYSICAL, grph);
    m_convertor.m_callback = libkerat::internals::parsers::parse_lta;    
    // lo setup
    lo_server server = lo_server_new_with_proto(NULL, LO_UDP, lo_error_handler);
    assert(server != NULL);
    lo_address client = local_lo_address(server);
    assert(client != NULL);
    
    // parser test setup
    lo_server_add_method(server, msg_type::PATH, NULL, lo_message_handler, this);

    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    assert(bundle != NULL);
    
    libkerat::server::imprint_bundle(bundle, &msg);
    lo_send_bundle(client, bundle);
    lo_server_recv(server);
    
    // the parser result is now available
    assert(m_result);
    assert(m_convertor_result.size() == 1);
    
    msg_type * msg_conv = dynamic_cast<msg_type *>(*m_convertor_result.begin());
    assert(*msg_conv == msg);
    
    lo_address_free(client);
    lo_server_free(server);
    lo_bundle_free_messages(bundle);
    
    cout << msg_type::PATH << " OK" << endl;
    return true;
} 


int main(){
    
    testsuite tests;
    return tests.run();

}

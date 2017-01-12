/**
 * \file      multiplexing_adaptor_test.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-03-21 17:07 UTC+2
 * \copyright BSD
 */

#include <iostream>
#include <kerat/kerat.hpp>
#include <dtuio/dtuio.hpp>
#include <uuid/uuid.h>
#include <cassert>

using std::cout;
using std::cerr;
using std::endl;
using namespace libkerat::message;

class test_listener: public libkerat::listener, public libkerat::internals::bundle_manipulator {
public:
    test_listener();
    ~test_listener();

    void notify(const libkerat::client * client);

    void clear();

    libkerat::bundle_stack m_bundles;
};

class test_client: public libkerat::client {
private:
    libkerat::bundle_stack m_stack;

public:
    
    test_client(){ }
    
    libkerat::bundle_stack get_stack() const { return m_stack; }
    
    int test_static_00(dtuio::adaptors::viewport_scaler & scaler);
    int test_dynamic_00(dtuio::adaptors::viewport_scaler & scaler);
    int test_static_offset(dtuio::adaptors::viewport_scaler & scaler);
    int test_dynamic_offset(dtuio::adaptors::viewport_scaler & scaler);
    
    bool load(int a){ return true; }
    bool load(int a, timespec ts){ return true; }
    void purge(){ bm_stack_clear(m_stack); }
    
    void commit(){ notify_listeners(); }

};

test_listener::test_listener(){ ; }
test_listener::~test_listener(){ ; } 

void test_listener::notify(const libkerat::client * client){
    m_bundles = client->get_stack();
}

void test_listener::clear(){
    bm_stack_clear(m_bundles);
}

int test_client::test_static_00(dtuio::adaptors::viewport_scaler & scaler){
    cout << "Test case - single point, viewport (0,0,0) [1920x1080x20] -> viewport [640x480x10]" << endl;
    
    // pretest cleanup
    test_listener tl;
    purge();
    while (!m_listeners.empty()){
        del_listener(*m_listeners.begin());
    }
    
    add_listener(&scaler);
    scaler.add_listener(&tl);
    
    
#ifdef DEBUG
    libkerat::listeners::stdout_listener dumper;
    add_listener(&dumper);
    scaler.add_listener(&dumper);
#endif    
    
    // setup stack - first global messages
    libkerat::frame_id_t frameid = 3;
    
    dtuio::uuid_t vpt1_uuid;
    uuid_parse("ac0f5ba0-bc96-4f98-8c81-8303f60b3910", vpt1_uuid);
    dtuio::sensor::viewport msg_src_vpt(vpt1_uuid, libkerat::helpers::point_3d(0, 0, 0), 1920, 1080, 20);
    dtuio::sensor::viewport msg_dst_vpt(vpt1_uuid, libkerat::helpers::point_3d(0, 0, 0), 640, 480, 10);
    scaler.set_target_viewport(msg_dst_vpt);

    pointer msg_ptr1(2, 0, 0, 0, 20, 20, 20, 0);
    
        
    { // setup first frame
        libkerat::bundle_handle * handle = new libkerat::bundle_handle;
        frame msg_frm(++frameid);
        alive msg_alv;
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_frm.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_src_vpt.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_ptr1.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_alv.clone());
        bm_stack_append(m_stack, handle);
    }
    
    commit();
    
    assert(tl.m_bundles.get_length() == 1);
    
    libkerat::bundle_handle resulting_handle = tl.m_bundles.get_update();
    const pointer * rc_msg_ptr1 = resulting_handle.get_message_of_type<pointer>(0);
    assert(rc_msg_ptr1 != NULL);
    assert(rc_msg_ptr1->get_session_id() == msg_ptr1.get_session_id());

    double fx = (double)msg_dst_vpt.get_width() / (double)msg_src_vpt.get_width();
    double fy = (double)msg_dst_vpt.get_height() / (double)msg_src_vpt.get_height();
    double fz = (double)msg_dst_vpt.get_depth() / (double)msg_src_vpt.get_depth();

    bool ok = true;
    ok &= (fabs(fx*msg_ptr1.get_x() - rc_msg_ptr1->get_x()) <= 1.0);
    ok &= (fabs(fy*msg_ptr1.get_y() - rc_msg_ptr1->get_y()) <= 1.0);
    ok &= (fabs(fz*msg_ptr1.get_z() - rc_msg_ptr1->get_z()) <= 1.0);

    return ok?0:1;
}
int test_client::test_dynamic_00(dtuio::adaptors::viewport_scaler & scaler){
    cout << "Test case - multiple points, (viewport (0,0,0) [1920x1080x20] .. viewport (0,0,0) [1920x1080x20]) -> viewport [640x480x10]" << endl;
    
    // pretest cleanup
    test_listener tl;
    purge();
    while (!m_listeners.empty()){
        del_listener(*m_listeners.begin());
    }
    
    add_listener(&scaler);
    scaler.add_listener(&tl);
    
    
#ifdef DEBUG
    libkerat::listeners::stdout_listener dumper;
    add_listener(&dumper);
    scaler.add_listener(&dumper);
#endif    
    
    // setup stack - first global messages
    libkerat::frame_id_t frameid = 3;
    
    dtuio::uuid_t vpt1_uuid;
    uuid_parse("ac0f5ba0-bc96-4f98-8c81-8303f60b3910", vpt1_uuid);
    dtuio::sensor::viewport msg_src_vpt_1(vpt1_uuid, libkerat::helpers::point_3d(0, 0, 0), 1920, 1080, 20);
    dtuio::sensor::viewport msg_src_vpt_2(vpt1_uuid, libkerat::helpers::point_3d(0, 0, 0), 2*1920, 2*1080, 20);
    dtuio::sensor::viewport msg_dst_vpt(vpt1_uuid, libkerat::helpers::point_3d(0, 0, 0), 640, 480, 10);
    scaler.set_target_viewport(msg_dst_vpt);

    pointer msg_ptr2(2, 0, 0, 0, 20, 20, 20, 0);
    pointer msg_ptr1(3, 0, 0, 0, 1020, 1020, 10, 0);
        
    { // setup first frame
        libkerat::bundle_handle * handle = new libkerat::bundle_handle;
        frame msg_frm(++frameid);
        alive msg_alv;
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_frm.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_src_vpt_1.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_ptr1.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_alv.clone());
        bm_stack_append(m_stack, handle);
    }
    
    { // setup second frame
        libkerat::bundle_handle * handle = new libkerat::bundle_handle;
        frame msg_frm(++frameid);
        alive msg_alv;
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_frm.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_src_vpt_2.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_ptr1.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_ptr2.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_alv.clone());
        bm_stack_append(m_stack, handle);
    }
    
    commit();
    
    assert(tl.m_bundles.get_length() == 2);
    
    // setup variables
    libkerat::bundle_handle resulting_handle;
    const pointer * rc_msg_ptr1 = NULL;
    const pointer * rc_msg_ptr2 = NULL;
    double fx = 0;
    double fy = 0;
    double fz = 0;
    bool ok = true;
    
    // verify first frame
    resulting_handle = tl.m_bundles.get_update();
    rc_msg_ptr1 = resulting_handle.get_message_of_type<pointer>(0);
    assert(rc_msg_ptr1 != NULL);
    assert(rc_msg_ptr1->get_session_id() == msg_ptr1.get_session_id());

    fx = (double)msg_dst_vpt.get_width() / (double)msg_src_vpt_1.get_width();
    fy = (double)msg_dst_vpt.get_height() / (double)msg_src_vpt_1.get_height();
    fz = (double)msg_dst_vpt.get_depth() / (double)msg_src_vpt_1.get_depth();

    ok &= (fabs(fx*msg_ptr1.get_x() - rc_msg_ptr1->get_x()) <= 1.0);
    ok &= (fabs(fy*msg_ptr1.get_y() - rc_msg_ptr1->get_y()) <= 1.0);
    ok &= (fabs(fz*msg_ptr1.get_z() - rc_msg_ptr1->get_z()) <= 1.0);
    
    // verify second frame
    resulting_handle = tl.m_bundles.get_update();
    rc_msg_ptr1 = resulting_handle.get_message_of_type<pointer>(0);
    assert(rc_msg_ptr1 != NULL);
    assert(rc_msg_ptr1->get_session_id() == msg_ptr1.get_session_id());
    rc_msg_ptr2 = resulting_handle.get_message_of_type<pointer>(1);
    assert(rc_msg_ptr2 != NULL);
    assert(rc_msg_ptr2->get_session_id() == msg_ptr2.get_session_id());

    fx = (double)msg_dst_vpt.get_width() / (double)msg_src_vpt_2.get_width();
    fy = (double)msg_dst_vpt.get_height() / (double)msg_src_vpt_2.get_height();
    fz = (double)msg_dst_vpt.get_depth() / (double)msg_src_vpt_2.get_depth();
    
    // verify first pointer
    ok &= (fabs(fx*msg_ptr1.get_x() - rc_msg_ptr1->get_x()) <= 1.0);
    ok &= (fabs(fy*msg_ptr1.get_y() - rc_msg_ptr1->get_y()) <= 1.0);
    ok &= (fabs(fz*msg_ptr1.get_z() - rc_msg_ptr1->get_z()) <= 1.0);
    
    // verify second pointer
    ok &= (fabs(fx*msg_ptr2.get_x() - rc_msg_ptr2->get_x()) <= 1.0);
    ok &= (fabs(fy*msg_ptr2.get_y() - rc_msg_ptr2->get_y()) <= 1.0);
    ok &= (fabs(fz*msg_ptr2.get_z() - rc_msg_ptr2->get_z()) <= 1.0);
    
    return ok?0:1;
}
int test_client::test_static_offset(dtuio::adaptors::viewport_scaler & scaler){
    cout << "Test case - single point, viewport (300,300,300) [1920x1080x20] -> viewport [640x480x10]" << endl;
    
    // pretest cleanup
    test_listener tl;
    purge();
    while (!m_listeners.empty()){
        del_listener(*m_listeners.begin());
    }
    
    add_listener(&scaler);
    scaler.add_listener(&tl);
    
    
#ifdef DEBUG
    libkerat::listeners::stdout_listener dumper;
    add_listener(&dumper);
    scaler.add_listener(&dumper);
#endif    
    
    // setup stack - first global messages
    libkerat::frame_id_t frameid = 3;
    
    dtuio::uuid_t vpt1_uuid;
    uuid_parse("ac0f5ba0-bc96-4f98-8c81-8303f60b3910", vpt1_uuid);
    dtuio::sensor::viewport msg_src_vpt(vpt1_uuid, libkerat::helpers::point_3d(300, 300, 300), 1920, 1080, 20);
    dtuio::sensor::viewport msg_dst_vpt(vpt1_uuid, libkerat::helpers::point_3d(0, 0, 0), 640, 480, 10);
    scaler.set_target_viewport(msg_dst_vpt);

    pointer msg_ptr1(2, 0, 0, 0, 20, 20, 20, 0);
    
        
    { // setup first frame
        libkerat::bundle_handle * handle = new libkerat::bundle_handle;
        frame msg_frm(++frameid);
        alive msg_alv;
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_frm.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_src_vpt.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_ptr1.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_alv.clone());
        bm_stack_append(m_stack, handle);
    }
    
    commit();
    
    assert(tl.m_bundles.get_length() == 1);
    
    libkerat::bundle_handle resulting_handle = tl.m_bundles.get_update();
    const pointer * rc_msg_ptr1 = resulting_handle.get_message_of_type<pointer>(0);
    assert(rc_msg_ptr1 != NULL);
    assert(rc_msg_ptr1->get_session_id() == msg_ptr1.get_session_id());
    
    const dtuio::sensor::viewport * rc_msg_vpt1 = resulting_handle.get_message_of_type<dtuio::sensor::viewport>(0);
    assert(rc_msg_vpt1 != NULL);
    assert(*(const dtuio::helpers::uuid *)rc_msg_vpt1 == msg_dst_vpt);
    assert(*(const libkerat::helpers::point_3d *)rc_msg_vpt1 == msg_src_vpt);
    assert(rc_msg_vpt1->get_width() == msg_dst_vpt.get_width());
    assert(rc_msg_vpt1->get_height() == msg_dst_vpt.get_height());
    assert(rc_msg_vpt1->get_depth() == msg_dst_vpt.get_depth());
    

    double fx = (double)msg_dst_vpt.get_width() / (double)msg_src_vpt.get_width();
    double fy = (double)msg_dst_vpt.get_height() / (double)msg_src_vpt.get_height();
    double fz = (double)msg_dst_vpt.get_depth() / (double)msg_src_vpt.get_depth();

    bool ok = true;
    ok &= (fabs((fx*(msg_ptr1.get_x()-msg_src_vpt.get_x()) + msg_src_vpt.get_x()) - rc_msg_ptr1->get_x()) <= 1.0);
    ok &= (fabs((fy*(msg_ptr1.get_y()-msg_src_vpt.get_y()) + msg_src_vpt.get_y()) - rc_msg_ptr1->get_y()) <= 1.0);
    ok &= (fabs((fz*(msg_ptr1.get_z()-msg_src_vpt.get_z()) + msg_src_vpt.get_z()) - rc_msg_ptr1->get_z()) <= 1.0);

    return ok?0:1;
}
int test_client::test_dynamic_offset(dtuio::adaptors::viewport_scaler & scaler){
    cout << "Test case - multiple points, (viewport (960,540,10) [1920x1080x20] .. viewport (1920,1080,10) [1920x1080x20]) -> viewport [640x480x10]" << endl;
    
    // pretest cleanup
    test_listener tl;
    purge();
    while (!m_listeners.empty()){
        del_listener(*m_listeners.begin());
    }
    
    add_listener(&scaler);
    scaler.add_listener(&tl);
    
    
#ifdef DEBUG
    libkerat::listeners::stdout_listener dumper;
    add_listener(&dumper);
    scaler.add_listener(&dumper);
#endif    
    
    // setup stack - first global messages
    libkerat::frame_id_t frameid = 3;
    
    dtuio::uuid_t vpt1_uuid;
    uuid_parse("ac0f5ba0-bc96-4f98-8c81-8303f60b3910", vpt1_uuid);
    dtuio::sensor::viewport msg_src_vpt_1(vpt1_uuid, libkerat::helpers::point_3d(960, 540, 10), 1920, 1080, 20);
    dtuio::sensor::viewport msg_src_vpt_2(vpt1_uuid, libkerat::helpers::point_3d(1920, 1080, 10), 2*1920, 2*1080, 20);
    dtuio::sensor::viewport msg_dst_vpt(vpt1_uuid, libkerat::helpers::point_3d(0, 0, 0), 640, 480, 10);
    scaler.set_target_viewport(msg_dst_vpt);

    pointer msg_ptr2(2, 0, 0, 0, 20, 20, 20, 0);
    pointer msg_ptr1(3, 0, 0, 0, 1020, 1020, 10, 0);
        
    { // setup first frame
        libkerat::bundle_handle * handle = new libkerat::bundle_handle;
        frame msg_frm(++frameid);
        alive msg_alv;
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_frm.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_src_vpt_1.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_ptr1.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_alv.clone());
        bm_stack_append(m_stack, handle);
    }
    
    { // setup second frame
        libkerat::bundle_handle * handle = new libkerat::bundle_handle;
        frame msg_frm(++frameid);
        alive msg_alv;
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_frm.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_src_vpt_2.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_ptr1.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_ptr2.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_alv.clone());
        bm_stack_append(m_stack, handle);
    }
    
    commit();
    
    assert(tl.m_bundles.get_length() == 2);
    
    libkerat::bundle_handle resulting_handle;
    const pointer * rc_msg_ptr1 = NULL;
    const pointer * rc_msg_ptr2 = NULL;
    const dtuio::sensor::viewport * rc_msg_vpt1 = NULL;    
    double fx = 0;
    double fy = 0;
    double fz = 0;
    bool ok = true;
    
    // verify first frame
    resulting_handle = tl.m_bundles.get_update();
    rc_msg_ptr1 = resulting_handle.get_message_of_type<pointer>(0);
    assert(rc_msg_ptr1 != NULL);
    assert(rc_msg_ptr1->get_session_id() == msg_ptr1.get_session_id());

    rc_msg_vpt1 = resulting_handle.get_message_of_type<dtuio::sensor::viewport>(0);
    assert(rc_msg_vpt1 != NULL);
    assert(*(const dtuio::helpers::uuid *)rc_msg_vpt1 == msg_dst_vpt);
    assert(*(const libkerat::helpers::point_3d *)rc_msg_vpt1 == msg_src_vpt_1);
    assert(rc_msg_vpt1->get_width() == msg_dst_vpt.get_width());
    assert(rc_msg_vpt1->get_height() == msg_dst_vpt.get_height());
    assert(rc_msg_vpt1->get_depth() == msg_dst_vpt.get_depth());
    
    fx = (double)msg_dst_vpt.get_width() / (double)msg_src_vpt_1.get_width();
    fy = (double)msg_dst_vpt.get_height() / (double)msg_src_vpt_1.get_height();
    fz = (double)msg_dst_vpt.get_depth() / (double)msg_src_vpt_1.get_depth();

    ok &= (fabs((fx*(msg_ptr1.get_x()-msg_src_vpt_1.get_x()) + msg_src_vpt_1.get_x()) - rc_msg_ptr1->get_x()) <= 1.0);
    ok &= (fabs((fy*(msg_ptr1.get_y()-msg_src_vpt_1.get_y()) + msg_src_vpt_1.get_y()) - rc_msg_ptr1->get_y()) <= 1.0);
    ok &= (fabs((fz*(msg_ptr1.get_z()-msg_src_vpt_1.get_z()) + msg_src_vpt_1.get_z()) - rc_msg_ptr1->get_z()) <= 1.0);

    // verify second frame
    resulting_handle = tl.m_bundles.get_update();
    rc_msg_ptr1 = resulting_handle.get_message_of_type<pointer>(0);
    assert(rc_msg_ptr1 != NULL);
    assert(rc_msg_ptr1->get_session_id() == msg_ptr1.get_session_id());
    rc_msg_ptr2 = resulting_handle.get_message_of_type<pointer>(1);
    assert(rc_msg_ptr2 != NULL);
    assert(rc_msg_ptr2->get_session_id() == msg_ptr2.get_session_id());

    rc_msg_vpt1 = resulting_handle.get_message_of_type<dtuio::sensor::viewport>(0);
    assert(rc_msg_vpt1 != NULL);
    assert(*(const dtuio::helpers::uuid *)rc_msg_vpt1 == msg_dst_vpt);
    assert(*(const libkerat::helpers::point_3d *)rc_msg_vpt1 == msg_src_vpt_2);
    assert(rc_msg_vpt1->get_width() == msg_dst_vpt.get_width());
    assert(rc_msg_vpt1->get_height() == msg_dst_vpt.get_height());
    assert(rc_msg_vpt1->get_depth() == msg_dst_vpt.get_depth());
    
    fx = (double)msg_dst_vpt.get_width() / (double)msg_src_vpt_2.get_width();
    fy = (double)msg_dst_vpt.get_height() / (double)msg_src_vpt_2.get_height();
    fz = (double)msg_dst_vpt.get_depth() / (double)msg_src_vpt_2.get_depth();
    
    // verify first pointer
    ok &= (fabs((fx*(msg_ptr1.get_x()-msg_src_vpt_2.get_x()) + msg_src_vpt_2.get_x()) - rc_msg_ptr1->get_x()) <= 1.0);
    ok &= (fabs((fy*(msg_ptr1.get_y()-msg_src_vpt_2.get_y()) + msg_src_vpt_2.get_y()) - rc_msg_ptr1->get_y()) <= 1.0);
    ok &= (fabs((fz*(msg_ptr1.get_z()-msg_src_vpt_2.get_z()) + msg_src_vpt_2.get_z()) - rc_msg_ptr1->get_z()) <= 1.0);
    
    // verify second pointer
    ok &= (fabs((fx*(msg_ptr2.get_x()-msg_src_vpt_2.get_x()) + msg_src_vpt_2.get_x()) - rc_msg_ptr2->get_x()) <= 1.0);
    ok &= (fabs((fy*(msg_ptr2.get_y()-msg_src_vpt_2.get_y()) + msg_src_vpt_2.get_y()) - rc_msg_ptr2->get_y()) <= 1.0);
    ok &= (fabs((fz*(msg_ptr2.get_z()-msg_src_vpt_2.get_z()) + msg_src_vpt_2.get_z()) - rc_msg_ptr2->get_z()) <= 1.0);

    return ok?0:1;
}

int main(){
    test_client tc;

    int retval = 0;
    { 
        dtuio::sensor::viewport vpt;
        dtuio::adaptors::viewport_scaler scaler(vpt);
        int tr = tc.test_static_00(scaler);
        if (tr != 0){ cerr << "[FAILED]" << endl; } else { cout << "[  OK  ]" << endl; }
        retval |= tr;
    }
    { 
        dtuio::sensor::viewport vpt;
        dtuio::adaptors::viewport_scaler scaler(vpt);
        int tr = tc.test_static_offset(scaler);
        if (tr != 0){ cerr << "[FAILED]" << endl; } else { cout << "[  OK  ]" << endl; }
        retval |= tr;
    }
    { 
        dtuio::sensor::viewport vpt;
        dtuio::adaptors::viewport_scaler scaler(vpt);
        int tr = tc.test_dynamic_00(scaler);
        if (tr != 0){ cerr << "[FAILED]" << endl; } else { cout << "[  OK  ]" << endl; }
        retval |= tr;
    }
    { 
        dtuio::sensor::viewport vpt;
        dtuio::adaptors::viewport_scaler scaler(vpt);
        int tr = tc.test_dynamic_offset(scaler);
        if (tr != 0){ cerr << "[FAILED]" << endl; } else { cout << "[  OK  ]" << endl; }
        retval |= tr;
    }
   
    return retval;

}

/**
 * \file      multiplexing_adaptor_test.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-03-21 17:07 UTC+2
 * \copyright BSD
 */

#include <iostream>
#include <kerat/kerat.hpp>
#include <muse/muse.hpp>
#include <cassert>

using std::cout;
using std::cerr;
using std::endl;
using namespace libkerat::message;

using libkerat::helpers::point_2d;
typedef libkerat::message::convex_hull::point_2d_list point_2d_list;

libkerat::message::convex_hull make_valid_hull(){
    
    point_2d_list convex_hull_points;
    convex_hull_points.push_back(point_2d(-1, 4));
    convex_hull_points.push_back(point_2d(-1, 0));
    convex_hull_points.push_back(point_2d(3, 0));
    convex_hull_points.push_back(point_2d(5, 4));
    
    return libkerat::message::convex_hull(2, convex_hull_points);

}

libkerat::message::convex_hull make_broken_hull(){
    
    point_2d_list convex_hull_points;
    convex_hull_points.push_back(point_2d(-1, 4));
    convex_hull_points.push_back(point_2d(-1, 0));
    convex_hull_points.push_back(point_2d(3, 0));
    convex_hull_points.push_back(point_2d(5, 5));
    convex_hull_points.push_back(point_2d(4, 2));
    
    return libkerat::message::convex_hull(2, convex_hull_points);

}

libkerat::message::convex_hull make_invalid_order_hull(){
    
    point_2d_list convex_hull_points;
    convex_hull_points.push_back(point_2d(-1, 4));
    convex_hull_points.push_back(point_2d(-1, 0));
    convex_hull_points.push_back(point_2d(5, 5));
    convex_hull_points.push_back(point_2d(3, 0));
    
    return libkerat::message::convex_hull(2, convex_hull_points);

}

point_2d make_point_inside(){ return point_2d(1, 2.5); }
point_2d make_point_outside(){ return point_2d(-2, -1); }

class test_client: public libkerat::client {
private:
    libkerat::bundle_stack m_stack;

public:
    
    test_client(const libkerat::message::convex_hull & container, point_2d testpoint){
        libkerat::timetag_t time;

        alive::alive_ids alive_ids; 
        alive_ids.insert(container.get_session_id());
        alive_ids.insert(container.get_session_id()+1);
        alive alv1(alive_ids);
        frame frm1(10, time, "testsuite", 0x0, 1, 200, 200);

        libkerat::bundle_handle * handle1 = new libkerat::bundle_handle;
        bm_handle_insert(*handle1, bm_handle_end(*handle1), frm1.clone());
        bm_handle_insert(*handle1, bm_handle_end(*handle1), container.clone());
        bm_handle_insert(*handle1, bm_handle_end(*handle1), alv1.clone());
        bm_stack_append(m_stack, handle1);
        
        lo_timetag_now(&time);
        frm1.set_frame_id(frm1.get_frame_id()+1);
        frm1.set_timestamp(time);
        
        pointer ptr1(container.get_session_id()+1, pointer::TYPEID_UNKNOWN, pointer::UID_NOUSER, 0, testpoint.get_x(), testpoint.get_y(), 0, 0);
        
        libkerat::bundle_handle * handle2 = new libkerat::bundle_handle;
        bm_handle_insert(*handle2, bm_handle_end(*handle2), frm1.clone());
        bm_handle_insert(*handle2, bm_handle_end(*handle2), container.clone());
        bm_handle_insert(*handle2, bm_handle_end(*handle2), ptr1.clone());
        bm_handle_insert(*handle2, bm_handle_end(*handle2), alv1.clone());
        bm_stack_append(m_stack, handle2);
    }
    
    libkerat::bundle_stack get_stack() const { return m_stack; }
    
    bool load(int a){ return true; }
    bool load(int a, timespec ts){ return true; }
    void purge(){ ; }
    
    void commit(){ notify_listeners(); }

};

int test_scenario(const libkerat::message::convex_hull & container, point_2d testpoint, bool expected){
    muse::aggregators::convex_hull_container aggregator;
    test_client tc(container, testpoint);
    tc.add_listener(&aggregator);
    tc.commit();
    
    libkerat::bundle_stack processed = aggregator.get_stack();
    assert(processed.get_length() == 2);
    
    processed.get_update();
    libkerat::bundle_handle resulting_handle = processed.get_update();
    
    const libkerat::message::container_association * msg_coa = resulting_handle.get_message_of_type<libkerat::message::container_association>();
    const libkerat::message::alive_associations * msg_ala = resulting_handle.get_message_of_type<libkerat::message::alive_associations>();

    if ((expected) && ((msg_coa == NULL) || (msg_ala == NULL))){
        std::cerr << "[FAIL] Association was expected but engine failed to commance the association!" << std::endl;
        return 1;
    } else if ((!expected) && ((msg_coa != NULL) || (msg_ala != NULL))){
        std::cerr << "[FAIL] Association was not expected but engine created associations!" << std::endl;
        return 1;
    }
    
    if (expected){
        assert(!msg_coa->get_associations().empty());
        assert(msg_coa->get_session_id() == container.get_session_id());
        assert(*(msg_coa->get_associations().begin()) == container.get_session_id()+1);
        
        assert(!msg_ala->get_associations().empty());
        assert(*(msg_ala->get_associations().begin()) == container.get_session_id()+1);

        std::cout << "[ OK ]" << std::endl;
        return 0;
    } else {

        std::cout << "[ OK ]" << std::endl;
        return 0;
    }
}

int main(){

    int retval = 0;

    // the only valid case is for valid hull & point inside
    std::cout << "Valid, inside ";
    retval |= test_scenario(make_valid_hull(), make_point_inside(), true);
    std::cout << "Valid, outside ";
    retval |= test_scenario(make_valid_hull(), make_point_outside(), false);
    std::cout << "Broken, inside ";
    retval |= test_scenario(make_broken_hull(), make_point_inside(), false);
    std::cout << "Broken, outside ";
    retval |= test_scenario(make_broken_hull(), make_point_outside(), false);
    std::cout << "Invalid order, inside ";
    retval |= test_scenario(make_invalid_order_hull(), make_point_inside(), false);
    std::cout << "Invalid order, outside ";
    retval |= test_scenario(make_invalid_order_hull(), make_point_outside(), false);
   
    return retval;

}

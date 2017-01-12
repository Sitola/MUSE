/**
 * \file      multiplexing_adaptor_test.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-03-21 17:07 UTC+2
 * \copyright BSD
 */

#include <iostream>
#include <kerat/typedefs.hpp>
#include <kerat/multiplexing_adaptor.hpp>
#include <kerat/tuio_messages.hpp>
#include <kerat/client.hpp>
#include <cassert>
#include <vector>
#include <list>
#include <map>
#include <algorithm>

using std::cout;
using std::cerr;
using std::endl;

class test_listener: public libkerat::listener {
public:
    test_listener();
    ~test_listener();

    typedef std::list<libkerat::bundle_handle> bundles_list;

    void notify(const libkerat::client * client);

    bundles_list get_bundles();
    void clear();

protected:

    bundles_list m_bundles;

};

class test_client: public libkerat::client {
public:
    test_client();
    ~test_client();

    /**
     * Test 1 - single source, single session id
     * @return 
     */
    bool run_test_1();
    
    /**
     * Test 2 - signle source, multiple, non-overlapping sessions
     * @return 
     */
    bool run_test_2();
    
    /**
     * Test 3 - single source, multiple, overlaping sessions
     * @return 
     */
    bool run_test_3();

    /**
     * Test 4 - multiple sources, multiple, overlaping sessions
     * @return 
     */
    bool run_test_4();

    void purge();

    libkerat::bundle_stack get_stack() const ;
    bool load(int count = 1);
    bool load(int count, struct timespec timeout);

private:

    typedef std::vector<libkerat::session_id_t> id_vector;
    typedef std::vector<id_vector> frame_vector;
    typedef std::map<libkerat::instance_id_t, frame_vector> id_test_inputs_map;
    typedef std::map<libkerat::instance_id_t, int> counter_map;
    typedef std::map<libkerat::instance_id_t, frame_vector::const_iterator> frame_status_map;
    
    typedef std::map<libkerat::session_id_t, libkerat::session_id_t> id_map;
    typedef std::map<libkerat::instance_id_t, id_map> id_mapping_map;
    

    static int count_leftovers(const counter_map & counters);
    bool run_test(const id_test_inputs_map & input);

    test_listener m_listener;
    libkerat::adaptors::multiplexing_adaptor m_multiplexer;
    libkerat::bundle_stack m_stack;

};


int main(){

    test_client cl;

    bool t1r = cl.run_test_1();
    bool t2r = cl.run_test_2();
    bool t3r = cl.run_test_3();

    return (t1r && t2r && t3r)?0:1;
}

// ----------------------------------------------------------- listener

test_listener::test_listener(){ ; }
test_listener::~test_listener(){ clear(); }

void test_listener::notify(const libkerat::client* notifier){

    libkerat::bundle_stack stack = notifier->get_stack();
    
    while (stack.get_length() > 0){
        libkerat::bundle_handle handle = stack.get_update();
        m_bundles.push_back(handle);
    }

}

void test_listener::clear(){
    m_bundles.clear();
}

test_listener::bundles_list test_listener::get_bundles(){
    return m_bundles;
}

// ----------------------------------------------------------- client

test_client::test_client(){

    m_multiplexer.add_listener(&m_listener);
    this->add_listener(&m_multiplexer);

}
test_client::~test_client(){ ; }

int test_client::count_leftovers(const test_client::counter_map & counters){
    int retval = 0;
    for (counter_map::const_iterator i = counters.begin(); i != counters.end(); i++){
        retval += i->second;
    }
    return retval;
}

void test_client::purge(){
    bm_stack_clear(m_stack);
}

bool test_client::load(int count){
    return false;
}

bool test_client::load(int count, timespec timeout){
    return false;
}

libkerat::bundle_stack test_client::get_stack() const {
    return m_stack;
}

bool test_client::run_test_1(){

    id_vector input_data_0;
    id_vector input_data_1;
        input_data_1.push_back(1);
    frame_vector frames;
        frames.push_back(input_data_1);
        frames.push_back(input_data_1);
        frames.push_back(input_data_1);
        frames.push_back(input_data_0);
    id_test_inputs_map input;
        input[11] = frames;

    bool result = run_test(input);
    cout << "Test 1: " << (result?"OK":"FAIL") << endl;
    return result;
}

bool test_client::run_test_2(){

    id_vector input_data_0;
    id_vector input_data_1;
        input_data_1.push_back(167);
    id_vector input_data_2;
        input_data_2.push_back(168);
    id_vector input_data_3;
        input_data_3.push_back(169);
    id_vector input_data_4;
        input_data_4.push_back(170);
    frame_vector frames;
        frames.push_back(input_data_1);
        frames.push_back(input_data_1);
        frames.push_back(input_data_1);
        frames.push_back(input_data_0);
        frames.push_back(input_data_2);
        frames.push_back(input_data_2);
        frames.push_back(input_data_2);
        frames.push_back(input_data_0);
        frames.push_back(input_data_3);
        frames.push_back(input_data_3);
        frames.push_back(input_data_3);
        frames.push_back(input_data_0);
        frames.push_back(input_data_4);
        frames.push_back(input_data_4);
        frames.push_back(input_data_4);
        frames.push_back(input_data_0);
    id_test_inputs_map input;
        input[21] = frames;

    bool result = run_test(input);
    cout << "Test 2: " << (result?"OK":"FAIL") << endl;
    return result;
}

bool test_client::run_test_3(){

    id_vector input_data_0;
    id_vector input_data_1;
        input_data_1.push_back(17);
    id_vector input_data_2;
        input_data_2.push_back(17);
        input_data_2.push_back(18);
    id_vector input_data_3;
        input_data_3.push_back(18);
    id_vector input_data_4;
        input_data_4.push_back(18);
        input_data_4.push_back(19);
    id_vector input_data_5;
        input_data_5.push_back(19);
    id_vector input_data_6;
        input_data_6.push_back(20);
    frame_vector frames;
        frames.push_back(input_data_1);
        frames.push_back(input_data_1);
        frames.push_back(input_data_1);
        frames.push_back(input_data_2);
        frames.push_back(input_data_3);
        frames.push_back(input_data_3);
        frames.push_back(input_data_3);
        frames.push_back(input_data_4);
        frames.push_back(input_data_5);
        frames.push_back(input_data_6);
        frames.push_back(input_data_0);
    id_test_inputs_map input;
        input[31] = frames;

    bool result = run_test(input);
    cout << "Test 3: " << (result?"OK":"FAIL") << endl;
    return result;
}

bool test_client::run_test(const id_test_inputs_map& input) {

    using libkerat::internals::bundle_manipulator;

    bool is_test_ok = true;

    frame_status_map status;

    // initialize test
    for (id_test_inputs_map::const_iterator i = input.begin(); i != input.end(); i++){
        status[i->first] = i->second.begin();
    }
    bm_stack_clear(m_stack);
    m_listener.clear();
    m_multiplexer.purge();

    bool keep_going = true;

    // test data initialization
    while (keep_going){

        keep_going = false;

        for (id_test_inputs_map::const_iterator i = input.begin(); i != input.end(); i++){
            if (status[i->first] != i->second.end()){
                keep_going = true;

                libkerat::bundle_handle handle;
                libkerat::message::alive msg_alv;
                libkerat::message::frame msg_frm((status[i->first] - i->second.begin())+2);
                    msg_frm.set_app_name("test_client");
                    msg_frm.set_address(0xaabbccdd);
                    msg_frm.set_instance(i->first);

                bm_handle_insert(handle, bm_handle_end(handle), msg_frm.clone());
                
                libkerat::message::alive::alive_ids alives;

                for (id_vector::const_iterator id = status[i->first]->begin(); id != status[i->first]->end(); id++){
//                    libkerat::message::pointer msg_ptr;
//                    msg_ptr.set_session_id(*id);
//                    bundle_manipulator::bundle_add_clone(handle, &msg_ptr);
                    alives.insert(*id);
                }
                msg_alv.set_alives(alives);

                bm_handle_insert(handle, bm_handle_end(handle), msg_alv.clone());
                bm_stack_append(m_stack, bm_handle_clone(handle));

                status[i->first]++;
            }
        }
    }

    // run test
    notify_listeners();

    // check the test result
    id_mapping_map mappings;
    counter_map counters;
    test_listener::bundles_list bundles = m_listener.get_bundles();
    for (test_listener::bundles_list::const_iterator i = bundles.begin(); i != bundles.end(); i++){
        libkerat::bundle_handle bundle = *i;
        const libkerat::message::frame * msg_frm = bundle.get_frame();
        const libkerat::message::alive * msg_alv = bundle.get_alive();
        
        assert(msg_frm != NULL);
        assert(msg_alv != NULL);
        
        id_map & id_mapping = mappings[msg_frm->get_instance()];
        libkerat::message::alive::alive_ids alives = msg_alv->get_alives();

        const frame_vector & frames = input.at(msg_frm->get_instance());
        const id_vector & idvec = frames.at(msg_frm->get_frame_id()-2);
        
        counters[msg_frm->get_instance()] = idvec.size();

        size_t idc = 0;
        for (libkerat::message::alive::alive_ids::const_iterator id = alives.begin(); id != alives.end(); id++){
            id_map::iterator id_found = id_mapping.find(*id);
            
            if (id_found == id_mapping.end()){
                id_mapping[*id] = idvec[idc];
            } else if (std::find(idvec.begin(), idvec.end(), id_found->second) == idvec.end()){
                cerr << "ID " << *id << " has incorrect mapping! " << endl;
                is_test_ok = false;
            }
            idc++;
        }
        
        if (alives.size() != count_leftovers(counters)){
            cerr << "ID count for frame " << msg_frm->get_frame_id() << " instance " << msg_frm->get_instance() << " does not match!" << endl;
            is_test_ok = false;
        }
    }



    return is_test_ok;
}

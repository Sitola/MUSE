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
using muse::virtual_sensors::autoremapper;
using dtuio::sensor::sensor_properties;
using namespace dtuio::sensor_topology;

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
    
    int test_simple(autoremapper & remapper);
    int test_simple_touchtable_4(autoremapper & remapper);
    int test_complex_touchtable_4(autoremapper & remapper);
    
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

float distance(const libkerat::helpers::point_2d & pt1, const libkerat::helpers::point_2d & pt2){
    double p1x = pt1.get_x();
    double p1y = pt1.get_y();
    double p2x = pt2.get_x();
    double p2y = pt2.get_y();
    
    double x = p2x - p1x;
    double y = p2y - p1y;
    
    return sqrt(x*x + y*y);
}

int test_client::test_simple(autoremapper & remapper){
    cout << "Test case - two sensors, two pointer, single neighbour reference, no axis alignment change" << endl;
    
    // pretest cleanup
    test_listener tl;
    purge();
    while (!m_listeners.empty()){
        del_listener(*m_listeners.begin());
    }
    
    add_listener(&remapper);
    remapper.add_listener(&tl);
    
#ifdef DEBUG
    libkerat::listeners::stdout_listener dumper;
    remapper.add_listener(&dumper);
#endif    
    
    // setup stack - first global messages
    libkerat::frame_id_t frameid = 3;
    
    dtuio::uuid_t snr1_uuid;
    uuid_parse("ac0f5ba0-bc96-4f98-8c81-8303f60b3910", snr1_uuid);
    sensor_properties msg_snr1(snr1_uuid, sensor_properties::COORDINATE_TRANSLATE_SETUP_CONTINUOUS, sensor_properties::PURPOSE_EVENT_SOURCE);
    
    dtuio::uuid_t snr2_uuid;
    uuid_parse("a974d02c-e517-4895-ac24-8bbcc8148e39", snr2_uuid);
    sensor_properties msg_snr2(snr2_uuid, sensor_properties::COORDINATE_TRANSLATE_SETUP_CONTINUOUS, sensor_properties::PURPOSE_EVENT_SOURCE);
    
    neighbour msg_nbr(snr1_uuid, 0, 0, 2000, snr2_uuid);
    
    pointer msg_ptr1(2, 0, 0, 0, 20, 20, 0, 0);
    pointer msg_ptr2(4, 0, 0, 0, 20, 20, 0, 0);
    
        
    { // setup first frame - autoconfiguration info only
        libkerat::bundle_handle * handle = new libkerat::bundle_handle;
        frame msg_frm(++frameid);
        alive msg_alv;
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_frm.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_snr1.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_snr2.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_nbr.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_alv.clone());
        bm_stack_append(m_stack, handle);
    }
    
    { // setup second frame - point 1
        libkerat::bundle_handle * handle = new libkerat::bundle_handle;
        frame msg_frm(++frameid);
        alive::alive_ids ids; ids.insert(msg_ptr1.get_session_id());
        alive msg_alv(ids);
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_frm.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_snr1.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_ptr1.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_alv.clone());
        bm_stack_append(m_stack, handle);
    }
        
    { // setup second frame - point 2
        libkerat::bundle_handle * handle = new libkerat::bundle_handle;
        frame msg_frm(++frameid);
        alive::alive_ids ids; ids.insert(msg_ptr1.get_session_id());
                              ids.insert(msg_ptr2.get_session_id());
        alive msg_alv(ids);
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_frm.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_snr2.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_ptr2.clone());
        bm_handle_insert(*handle, bm_handle_end(*handle), msg_alv.clone());
        bm_stack_append(m_stack, handle);
    }
    
    commit();
    
    assert(tl.m_bundles.get_length() == 3);
    
    tl.m_bundles.get_update(); // throw away the setup-only frame
    
    libkerat::bundle_handle resulting_handle = tl.m_bundles.get_update();
    const pointer * rc_msg_ptr1 = resulting_handle.get_message_of_type<pointer>(0);
    assert(rc_msg_ptr1 != NULL);
    assert(rc_msg_ptr1->get_session_id() == msg_ptr1.get_session_id());
    msg_ptr1 = *rc_msg_ptr1;
    
    resulting_handle = tl.m_bundles.get_update();
    const pointer * rc_msg_ptr2 = resulting_handle.get_message_of_type<pointer>(0);
    assert(rc_msg_ptr2 != NULL);
    assert(rc_msg_ptr2->get_session_id() == msg_ptr2.get_session_id());
    msg_ptr1 = *rc_msg_ptr2;

    return (distance(msg_ptr1, msg_ptr2) == msg_nbr.get_distance())?0:1;
}

int test_client::test_simple_touchtable_4(autoremapper & remapper){
    const size_t SENSORS_COUNT = 4;
    
    cout << "Test case - " << SENSORS_COUNT << " sensors, " << SENSORS_COUNT << " pointers, single neighbour reference, no axis alignment change" << endl;
    
    // pretest cleanup
    test_listener tl;
    purge();
    while (!m_listeners.empty()){
        del_listener(*m_listeners.begin());
    }
    
    add_listener(&remapper);
    remapper.add_listener(&tl);
    
#ifdef DEBUG
    libkerat::listeners::stdout_listener dumper;
    remapper.add_listener(&dumper);
#endif    
    
    sensor_properties msg_snr_00(dtuio::helpers::uuid("459754e0-c080-419f-a922-343645da9942").get_uuid(), sensor_properties::COORDINATE_TRANSLATE_SETUP_ONCE, sensor_properties::PURPOSE_EVENT_SOURCE);
    sensor_properties msg_snr_01(dtuio::helpers::uuid("5cdf4818-7830-4d98-952d-8a3bde0d28ce").get_uuid(), sensor_properties::COORDINATE_TRANSLATE_SETUP_ONCE, sensor_properties::PURPOSE_EVENT_SOURCE);
    sensor_properties msg_snr_10(dtuio::helpers::uuid("92c02eff-f459-4e1a-b725-1df53c94fb8f").get_uuid(), sensor_properties::COORDINATE_TRANSLATE_SETUP_ONCE, sensor_properties::PURPOSE_EVENT_SOURCE);
    sensor_properties msg_snr_11(dtuio::helpers::uuid("f5adf779-0440-4522-a35a-3f35fea49c5c").get_uuid(), sensor_properties::COORDINATE_TRANSLATE_SETUP_ONCE, sensor_properties::PURPOSE_EVENT_SOURCE);

    const libkerat::dimmension_t DIM_X = 1920;
    const libkerat::dimmension_t DIM_Y = 1080;
    const libkerat::dimmension_t DIM_Z = 0;
    
    // viewports
    dtuio::sensor::viewport msg_vpt_00(msg_snr_00.get_uuid(), libkerat::helpers::point_3d(DIM_X/2, DIM_Y/2, DIM_Z/2), libkerat::helpers::angle_3d(), DIM_X, DIM_Y, DIM_Z);
    dtuio::sensor::viewport msg_vpt_01(msg_snr_01.get_uuid(), libkerat::helpers::point_3d(DIM_X/2, DIM_Y/2, DIM_Z/2), libkerat::helpers::angle_3d(), DIM_X, DIM_Y, DIM_Z);
    dtuio::sensor::viewport msg_vpt_10(msg_snr_10.get_uuid(), libkerat::helpers::point_3d(DIM_X/2, DIM_Y/2, DIM_Z/2), libkerat::helpers::angle_3d(), DIM_X, DIM_Y, DIM_Z);
    dtuio::sensor::viewport msg_vpt_11(msg_snr_11.get_uuid(), libkerat::helpers::point_3d(DIM_X/2, DIM_Y/2, DIM_Z/2), libkerat::helpers::angle_3d(), DIM_X, DIM_Y, DIM_Z);
    
    // points to->
    // 00 10    10
    // 01       11

    const libkerat::distance_t HORIZONTAL_DIST = 1988;
    const libkerat::distance_t VERTICAL_DIST = 1148;

    neighbour msg_nbr_00_10(msg_snr_00.get_uuid(), 0, 0, HORIZONTAL_DIST, msg_snr_10.get_uuid());
    neighbour msg_nbr_00_01(msg_snr_00.get_uuid(), M_PI/2.0, 0, VERTICAL_DIST, msg_snr_01.get_uuid());
    neighbour msg_nbr_10_11(msg_snr_10.get_uuid(), M_PI/2.0, 0, VERTICAL_DIST, msg_snr_11.get_uuid());

    // grouping    
    dtuio::sensor_topology::group_member msg_grp_00(dtuio::helpers::group_uuid("b740ec08-fc50-4ccd-ac9e-21a6cceec60a").get_group_uuid(), msg_snr_00.get_uuid());
    dtuio::sensor_topology::group_member msg_grp_01(dtuio::helpers::group_uuid("b740ec08-fc50-4ccd-ac9e-21a6cceec60a").get_group_uuid(), msg_snr_01.get_uuid());
    dtuio::sensor_topology::group_member msg_grp_10(dtuio::helpers::group_uuid("b740ec08-fc50-4ccd-ac9e-21a6cceec60a").get_group_uuid(), msg_snr_10.get_uuid());
    dtuio::sensor_topology::group_member msg_grp_11(dtuio::helpers::group_uuid("b740ec08-fc50-4ccd-ac9e-21a6cceec60a").get_group_uuid(), msg_snr_11.get_uuid());
    
    libkerat::frame_id_t global_frame_id = 4;
    
    { // add configuration
        libkerat::kerat_message * configuration[SENSORS_COUNT][6] = { 
            { &msg_snr_00, &msg_vpt_00, &msg_grp_00, &msg_nbr_00_10, &msg_nbr_00_01, NULL },
            { &msg_snr_01, &msg_vpt_01, &msg_grp_01, &msg_nbr_10_11, NULL, NULL },
            { &msg_snr_10, &msg_vpt_10, &msg_grp_10, NULL, NULL, NULL },
            { &msg_snr_11, &msg_vpt_11, &msg_grp_11, NULL, NULL, NULL }
        };
        
        for (size_t i = 0; i < SENSORS_COUNT; ++i){
            libkerat::bundle_handle * handle = new libkerat::bundle_handle;
            frame msg_frm(++global_frame_id);
            alive msg_alv;
            bm_handle_insert(*handle, bm_handle_end(*handle), msg_frm.clone());
            
            size_t j = 0;
            while (configuration[i][j] != NULL){
                bm_handle_insert(*handle, bm_handle_end(*handle), configuration[i][j]->clone());
                ++j;
            }
            
            bm_handle_insert(*handle, bm_handle_end(*handle), msg_alv.clone());
            bm_stack_append(m_stack, handle);
            
        }
    }
    
    pointer msg_ptr_00(0, 0, 0, 0, 20, 20, 0, 0);
    pointer msg_ptr_01(1, 0, 0, 0, 20, 20, 0, 0);
    pointer msg_ptr_10(2, 0, 0, 0, 20, 20, 0, 0);
    pointer msg_ptr_11(3, 0, 0, 0, 20, 20, 0, 0);
    { // configuration is away, now, prepare pointers

        libkerat::kerat_message * pointers[SENSORS_COUNT][3] = { 
            { &msg_snr_00, &msg_ptr_00, NULL },
            { &msg_snr_01, &msg_ptr_01, NULL },
            { &msg_snr_10, &msg_ptr_10, NULL },
            { &msg_snr_11, &msg_ptr_11, NULL }
        };
        
        for (size_t i = 0; i < SENSORS_COUNT; ++i){
            libkerat::bundle_handle * handle = new libkerat::bundle_handle;
            frame msg_frm(++global_frame_id);
            alive msg_alv;
            bm_handle_insert(*handle, bm_handle_end(*handle), msg_frm.clone());
            
            size_t j = 0;
            while (pointers[i][j] != NULL){
                bm_handle_insert(*handle, bm_handle_end(*handle), pointers[i][j]->clone());
                ++j;
            }
            
            bm_handle_insert(*handle, bm_handle_end(*handle), msg_alv.clone());
            bm_stack_append(m_stack, handle);
            
        }
    }
    
    // commance test
    commit();
    
    assert(tl.m_bundles.get_length() == SENSORS_COUNT*2);
    
    // find first meaningfull bundle
    libkerat::bundle_handle pointer_bundle;
    do {
        pointer_bundle = tl.m_bundles.get_update(); // throw away the setup-only frame
    } while ((tl.m_bundles.get_length() > 0) && (pointer_bundle.get_message_of_type<pointer>(0) == NULL));
    
    bool okay = true;
    { // assume pointer bundle was found, now, drift should be as follows
        libkerat::distance_t drifts[SENSORS_COUNT][2] = {
            { 0, 0 },
            { 0, VERTICAL_DIST },
            { HORIZONTAL_DIST, 0 },
            { HORIZONTAL_DIST, VERTICAL_DIST }
        };
        
        pointer * originals[SENSORS_COUNT] = {
            &msg_ptr_00,
            &msg_ptr_01,
            &msg_ptr_10,
            &msg_ptr_11
        };

        // match expectations to reality    
        size_t handle_id = 0;
        assert(tl.m_bundles.get_length()+1 == SENSORS_COUNT);
        
        for (handle_id = 0; handle_id < SENSORS_COUNT; ++handle_id){
            const pointer * msg_ptr = pointer_bundle.get_message_of_type<pointer>(0);
            
            libkerat::coord_t expected_x = originals[msg_ptr->get_session_id()]->get_x() + drifts[msg_ptr->get_session_id()][0];
            libkerat::coord_t expected_y = originals[msg_ptr->get_session_id()]->get_y() + drifts[msg_ptr->get_session_id()][1];
               
            const libkerat::coord_t TOLLERANCE = 2.0;
            libkerat::coord_t real_x = msg_ptr->get_x();
            libkerat::coord_t real_y = msg_ptr->get_y();
            
            bool matches = (fabs(real_x - expected_x) < TOLLERANCE) && (fabs(real_y - expected_y) < TOLLERANCE);
             
            if (!matches){ 
                std::cerr << "Pointer id " << msg_ptr->get_session_id() << " does not match!" << std::endl; 
                std::cerr << "expected: [" << expected_x << ", " << expected_y << "] got: [" << real_x << ", " << real_y << "]" << std::endl; 
            }
            okay &= matches;

            if (tl.m_bundles.get_length() > 0){
                pointer_bundle = tl.m_bundles.get_update();
            }
        }
        assert(handle_id == SENSORS_COUNT);
     
    }

    return okay?0:1;
}

int test_client::test_complex_touchtable_4(autoremapper & remapper){
    const size_t SENSORS_COUNT = 4;
    
    cout << "Test case - " << SENSORS_COUNT << " sensors in wrong order, " << SENSORS_COUNT << " pointers, single neighbour reference, no axis alignment change, viewport" << endl;
    
    dtuio::helpers::group_uuid group_uuid("b740ec08-fc50-4ccd-ac9e-21a6cceec60a");

    // pretest cleanup
    test_listener tl;
    purge();
    while (!m_listeners.empty()){
        del_listener(*m_listeners.begin());
    }
    
    add_listener(&remapper);
    
    dtuio::adaptors::viewport_projector projector(group_uuid.get_group_uuid());
    
    remapper.add_listener(&projector);
    projector.add_listener(&tl);
    
#ifdef DEBUG
    libkerat::listeners::stdout_listener dumper;
    //remapper.add_listener(&dumper);
    projector.add_listener(&dumper);
#endif    
    
    sensor_properties msg_snr_00(dtuio::helpers::uuid("459754e0-c080-419f-a922-343645da9942").get_uuid(), sensor_properties::COORDINATE_TRANSLATE_SETUP_ONCE, sensor_properties::PURPOSE_EVENT_SOURCE);
    sensor_properties msg_snr_01(dtuio::helpers::uuid("5cdf4818-7830-4d98-952d-8a3bde0d28ce").get_uuid(), sensor_properties::COORDINATE_TRANSLATE_SETUP_ONCE, sensor_properties::PURPOSE_EVENT_SOURCE);
    sensor_properties msg_snr_10(dtuio::helpers::uuid("92c02eff-f459-4e1a-b725-1df53c94fb8f").get_uuid(), sensor_properties::COORDINATE_TRANSLATE_SETUP_ONCE, sensor_properties::PURPOSE_EVENT_SOURCE);
    sensor_properties msg_snr_11(dtuio::helpers::uuid("f5adf779-0440-4522-a35a-3f35fea49c5c").get_uuid(), sensor_properties::COORDINATE_TRANSLATE_SETUP_ONCE, sensor_properties::PURPOSE_EVENT_SOURCE);

    const libkerat::dimmension_t DIM_X = 1920;
    const libkerat::dimmension_t DIM_Y = 1080;
    const libkerat::dimmension_t DIM_Z = 0;
    
    // viewports
    dtuio::sensor::viewport msg_vpt_00(msg_snr_00.get_uuid(), libkerat::helpers::point_3d(DIM_X/2, DIM_Y/2, DIM_Z/2), libkerat::helpers::angle_3d(), DIM_X, DIM_Y, DIM_Z);
    dtuio::sensor::viewport msg_vpt_01(msg_snr_01.get_uuid(), libkerat::helpers::point_3d(DIM_X/2, DIM_Y/2, DIM_Z/2), libkerat::helpers::angle_3d(), DIM_X, DIM_Y, DIM_Z);
    dtuio::sensor::viewport msg_vpt_10(msg_snr_10.get_uuid(), libkerat::helpers::point_3d(DIM_X/2, DIM_Y/2, DIM_Z/2), libkerat::helpers::angle_3d(), DIM_X, DIM_Y, DIM_Z);
    dtuio::sensor::viewport msg_vpt_11(msg_snr_11.get_uuid(), libkerat::helpers::point_3d(DIM_X/2, DIM_Y/2, DIM_Z/2), libkerat::helpers::angle_3d(), DIM_X, DIM_Y, DIM_Z);
    
    // points to->
    // 00 10    10
    // 01       11

    const libkerat::distance_t HORIZONTAL_DIST = 1988;
    const libkerat::distance_t VERTICAL_DIST = 1148;

    neighbour msg_nbr_00_10(msg_snr_00.get_uuid(), 0, 0, HORIZONTAL_DIST, msg_snr_10.get_uuid());
    neighbour msg_nbr_00_01(msg_snr_00.get_uuid(), M_PI/2.0, 0, VERTICAL_DIST, msg_snr_01.get_uuid());
    neighbour msg_nbr_10_11(msg_snr_10.get_uuid(), M_PI/2.0, 0, VERTICAL_DIST, msg_snr_11.get_uuid());

    // grouping    
    dtuio::sensor_topology::group_member msg_grp_00(group_uuid.get_group_uuid(), msg_snr_00.get_uuid());
    dtuio::sensor_topology::group_member msg_grp_01(group_uuid.get_group_uuid(), msg_snr_01.get_uuid());
    dtuio::sensor_topology::group_member msg_grp_10(group_uuid.get_group_uuid(), msg_snr_10.get_uuid());
    dtuio::sensor_topology::group_member msg_grp_11(group_uuid.get_group_uuid(), msg_snr_11.get_uuid());
    
    libkerat::frame_id_t global_frame_id = 4;
    
    { // add configuration
        libkerat::kerat_message * configuration[SENSORS_COUNT][6] = { 
            { &msg_snr_10, &msg_vpt_10, &msg_grp_10, NULL, NULL, NULL },
            { &msg_snr_11, &msg_vpt_11, &msg_grp_11, NULL, NULL, NULL },
            { &msg_snr_01, &msg_vpt_01, &msg_grp_01, &msg_nbr_10_11, NULL, NULL },
            { &msg_snr_00, &msg_vpt_00, &msg_grp_00, &msg_nbr_00_10, &msg_nbr_00_01, NULL }
        };
        
        for (size_t i = 0; i < SENSORS_COUNT; ++i){
            libkerat::bundle_handle * handle = new libkerat::bundle_handle;
            frame msg_frm(++global_frame_id);
            alive msg_alv;
            bm_handle_insert(*handle, bm_handle_end(*handle), msg_frm.clone());
            
            size_t j = 0;
            while (configuration[i][j] != NULL){
                bm_handle_insert(*handle, bm_handle_end(*handle), configuration[i][j]->clone());
                ++j;
            }
            
            bm_handle_insert(*handle, bm_handle_end(*handle), msg_alv.clone());
            bm_stack_append(m_stack, handle);
            
        }
    }
    
    pointer msg_ptr_00(0, 0, 0, 0, 20, 20, 0, 0);
    pointer msg_ptr_01(1, 0, 0, 0, 20, 20, 0, 0);
    pointer msg_ptr_10(2, 0, 0, 0, 20, 20, 0, 0);
    pointer msg_ptr_11(3, 0, 0, 0, 20, 20, 0, 0);
    { // configuration is away, now, prepare pointers

        libkerat::kerat_message * pointers[SENSORS_COUNT][3] = { 
            { &msg_snr_00, &msg_ptr_00, NULL },
            { &msg_snr_01, &msg_ptr_01, NULL },
            { &msg_snr_10, &msg_ptr_10, NULL },
            { &msg_snr_11, &msg_ptr_11, NULL }
        };
        
        for (size_t i = 0; i < SENSORS_COUNT; ++i){
            libkerat::bundle_handle * handle = new libkerat::bundle_handle;
            frame msg_frm(++global_frame_id);
            alive msg_alv;
            bm_handle_insert(*handle, bm_handle_end(*handle), msg_frm.clone());
            
            size_t j = 0;
            while (pointers[i][j] != NULL){
                bm_handle_insert(*handle, bm_handle_end(*handle), pointers[i][j]->clone());
                ++j;
            }
            
            bm_handle_insert(*handle, bm_handle_end(*handle), msg_alv.clone());
            bm_stack_append(m_stack, handle);
            
        }
    }
    
    // commance test
    commit();
    
    assert(tl.m_bundles.get_length() == SENSORS_COUNT*2);
    
    // find first meaningfull bundle
    libkerat::bundle_handle pointer_bundle;
    do {
        pointer_bundle = tl.m_bundles.get_update(); // throw away the setup-only frame
    } while ((tl.m_bundles.get_length() > 0) && (pointer_bundle.get_message_of_type<pointer>(0) == NULL));
    
    bool okay = true;
    { // assume pointer bundle was found, now, drift should be as follows
        libkerat::distance_t drifts[SENSORS_COUNT][2] = {
            { 0, 0 },
            { 0, VERTICAL_DIST },
            { HORIZONTAL_DIST, 0 },
            { HORIZONTAL_DIST, VERTICAL_DIST }
        };
        
        pointer * originals[SENSORS_COUNT] = {
            &msg_ptr_00,
            &msg_ptr_01,
            &msg_ptr_10,
            &msg_ptr_11
        };

        // match expectations to reality    
        size_t handle_id = 0;
        assert(tl.m_bundles.get_length()+1 == SENSORS_COUNT);
        
        for (handle_id = 0; handle_id < SENSORS_COUNT; ++handle_id){
            const pointer * msg_ptr = pointer_bundle.get_message_of_type<pointer>(0);
            
            libkerat::coord_t expected_x = originals[msg_ptr->get_session_id()]->get_x() + drifts[msg_ptr->get_session_id()][0];
            libkerat::coord_t expected_y = originals[msg_ptr->get_session_id()]->get_y() + drifts[msg_ptr->get_session_id()][1];
               
            const libkerat::coord_t TOLLERANCE = 2.0;
            libkerat::coord_t real_x = msg_ptr->get_x();
            libkerat::coord_t real_y = msg_ptr->get_y();
            
            bool matches = (fabs(real_x - expected_x) < TOLLERANCE) && (fabs(real_y - expected_y) < TOLLERANCE);
             
            if (!matches){ 
                std::cerr << "Pointer id " << msg_ptr->get_session_id() << " does not match!" << std::endl; 
                std::cerr << "expected: [" << expected_x << ", " << expected_y << "] got: [" << real_x << ", " << real_y << "]" << std::endl; 
            }
            okay &= matches;

            if (tl.m_bundles.get_length() > 0){
                pointer_bundle = tl.m_bundles.get_update();
            }
        }
        assert(handle_id == SENSORS_COUNT);
     
    }

    return okay?0:1;
}

int main(){
    test_client tc;

    int retval = 0;

    { 
        autoremapper remapper;
        retval |= tc.test_simple(remapper);
    }
    {
        autoremapper remapper;
        retval |= tc.test_simple_touchtable_4(remapper);
    }
    {
        autoremapper remapper;
        retval |= tc.test_complex_touchtable_4(remapper);
    }
   
    return retval;

}

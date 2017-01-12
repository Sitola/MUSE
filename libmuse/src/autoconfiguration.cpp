/**
 * \file      autoconfiguration.cpp
 * \brief     Implements the core autoconfiguration functionality
 * \author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-11-28 00:50 UTC+1
 * \copyright BSD
 */

#include <kerat/kerat.hpp>
#include <dtuio/dtuio.hpp>
#include <muse/autoconfiguration.hpp>

// ugly
#define is_group(primitive) (bool)((primitive)->m_role & autoremapper::i_primitive::ROLE_GROUP)
#define is_sensor(primitive) (bool)((primitive)->m_role & autoremapper::i_primitive::ROLE_SENSOR)
#define is_pivot(primitive) (bool)((primitive)->m_role & autoremapper::i_primitive::ROLE_PIVOT)

using dtuio::sensor::sensor_properties;
using dtuio::utils::spherical_to_cartesian;
using dtuio::utils::cartesian_to_spherical;

namespace muse {
    namespace virtual_sensors {

        autoremapper::i_primitive::i_primitive(autoremapper* einstein)
            :m_role(autoremapper::i_primitive::ROLE_UNKNOWN),
            m_vp_mode(autoremapper::i_primitive::VIEWPORT_UNSET),
            m_remapper(einstein),
            m_configured(false), m_parent(NULL),
            m_position_global(0, 0, 0),
            m_correction_azimuth(0),
            m_correction_altitude(0),
            m_setup_mode(sensor_properties::COORDINATE_TRANSLATE_SETUP_CONTINUOUS)
        { ; }
        
        autoremapper::i_primitive::~i_primitive(){
            for (primitive_map::iterator i = m_primitives_held.begin(); i != m_primitives_held.end(); ++i){
                m_remapper->destroy_primitive(i->first);
            }
            m_primitives_held.clear();
        }
        
        autoremapper::autoremapper(bool cut_received_topology, sensor_properties::coordinate_translation_mode_t default_mode)
            :m_cut_topology(cut_received_topology), m_default_mode(default_mode), m_update_required(false)
        { ; }
        
        autoremapper::~autoremapper(){
            for (reference_map_shell::iterator i = m_references_from.begin(); i != m_references_from.end(); ++i){
                for (reference_map::iterator j = i->second.begin(); j != i->second.end(); ++j){
                    delete j->second;
                }
            }
            m_references_from.clear();
            m_references_to.clear();
            
            for (i_primitive::primitive_map::iterator i = m_primitives_held.begin(); i != m_primitives_held.end(); ++i){
                delete i->second;
            }
            m_primitives_held.clear();
        }
        
        int autoremapper::process_bundle(const libkerat::bundle_handle& to_process, libkerat::bundle_handle& output_frame){
            // check for dtuio registrations
            process_sensor_registrations(to_process);
            process_viewport_registrations(to_process);
            process_group_registrations(to_process);
            process_neighbour_registrations(to_process);

            // commance the autoconfigure process
            commit();
            
            // first, check whether this is a dtuio bundle - if not, ignore and forward
            const sensor_properties * sensor = to_process.get_message_of_type<sensor_properties>(0);
            if (sensor == NULL){
                output_frame = to_process;
                return 0;
            }
            
            i_primitive * snr = ensure_exists(sensor->get_uuid());
#if 0 // artefact
            snr->m_role |= i_primitive::ROLE_SENSOR;
            if (snr == NULL){
                output_frame = to_process;
                std::cerr << "Unable to access " << (const dtuio::helpers::uuid *)sensor << std::endl;
                return 0;
            }
#endif
            
            // if we're not asked to run inplace, commance copy
            if (&to_process != &output_frame){
                bm_handle_copy(to_process, output_frame);
            }
            
            // prevent affecting the computed viewports by translation
            int retval = translate_bundle(snr, output_frame);
            project_group_viewports(output_frame);
            return retval;
        }

        void autoremapper::project_group_viewports(libkerat::bundle_handle & to_process){
            handle_iterator valid_pos = bm_handle_begin(to_process);
            
            while ((valid_pos != bm_handle_end(to_process)) && (
                (dynamic_cast<libkerat::message::frame *>(*valid_pos) != NULL)
             || (dynamic_cast<dtuio::sensor::sensor_properties *>(*valid_pos) != NULL)
             || (dynamic_cast<dtuio::sensor::viewport *>(*valid_pos) != NULL)
            )){ ++valid_pos; }
            
            // ?? damaged?
            if (valid_pos == bm_handle_end(to_process)){ return; }
            
            for (i_primitive::primitive_map::iterator i = m_primitives_held.begin(); i != m_primitives_held.end(); ++i){
                if (i->second->m_vp_mode == i_primitive::VIEWPORT_COMPUTED) {
                    bm_handle_insert(to_process, valid_pos, i->second->m_viewport.clone());
                }
            }
        }
        
        int autoremapper::translate_bundle(const autoremapper::i_primitive * sensor, libkerat::bundle_handle& to_process){
            // process intermediate messages
            for (handle_iterator i = bm_handle_begin(to_process); i != bm_handle_end(to_process); ++i){

                { // position corrections
                    libkerat::helpers::point_2d * hp_2point = dynamic_cast<libkerat::helpers::point_2d*>(*i);
                    if (hp_2point != NULL){
                        libkerat::helpers::point_3d * hp_3point = dynamic_cast<libkerat::helpers::point_3d*>(*i);
                        libkerat::helpers::point_3d tmp = compensate_drift_3d(
                            sensor->m_correction_azimuth, 
                            sensor->m_correction_altitude, 
                            ((hp_3point != NULL)?(*hp_3point):(*hp_2point))
                        );
                        tmp += sensor->m_position_global;

                        hp_2point->set_x(tmp.get_x());
                        hp_2point->set_y(tmp.get_y());

                        if (hp_3point != NULL){
                            hp_3point->set_z(tmp.get_z());
                        }
                    }
                }

                { // velocity corrections
                    libkerat::helpers::velocity_2d * hp_2velocity = dynamic_cast<libkerat::helpers::velocity_2d*>(*i);
                    if (hp_2velocity != NULL){
                        libkerat::helpers::velocity_3d * hp_3velocity = dynamic_cast<libkerat::helpers::velocity_3d*>(*i);

                        libkerat::helpers::point_3d tmp(hp_2velocity->get_x_velocity(), hp_2velocity->get_y_velocity(), 0);
                        if (hp_3velocity != NULL){ tmp.set_z(hp_3velocity->get_z_velocity()); }
                        
                        tmp = compensate_drift_3d(
                            sensor->m_correction_azimuth, 
                            sensor->m_correction_altitude,
                            tmp
                        );

                        hp_2velocity->set_x_velocity(tmp.get_x());
                        hp_2velocity->set_y_velocity(tmp.get_y());

                        if (hp_3velocity != NULL){
                            hp_3velocity->set_z_velocity(tmp.get_z());
                        }
                    }
                }
            }

            // scaling done
            return 0;
        }
        
        libkerat::helpers::point_3d autoremapper::compensate_drift_3d(const libkerat::angle_t azimuth, const libkerat::angle_t altitude, const libkerat::helpers::point_3d & original){
            libkerat::distance_t dist = 0;
            libkerat::angle_t azi = 0;
            libkerat::angle_t alt = 0;
            
            cartesian_to_spherical(original, azi, alt, dist);
            azi += azimuth;
            alt += altitude;
            
            return spherical_to_cartesian(azi, alt, dist);
        }
        
        void autoremapper::setup_auto_threshold(const libkerat::distance_t& dist1, const libkerat::distance_t& dist2){
            // for now, just dummy
            m_threshold = std::max(m_threshold, (float)(0.5*(dist1+dist2) + 1));
        }
        
        template <class OUTER_EINSTEIN>
        static size_t get_reference_count(const dtuio::helpers::uuid & pivot_candidate, const OUTER_EINSTEIN & from) {
            typedef std::set<dtuio::helpers::uuid> uuid_set;
            uuid_set reference_d;
            
            typedef typename OUTER_EINSTEIN::const_iterator oe_const_iterator;
            typedef typename OUTER_EINSTEIN::mapped_type::const_iterator ie_const_iterator;

            // who the candidate points to
            oe_const_iterator candidate_from = from.find(pivot_candidate);
            if (candidate_from != from.end()) {
                for (ie_const_iterator i = candidate_from->second.begin(); i != candidate_from->second.end(); ++i){
                    reference_d.insert(i->first);
                }
            }
#if 0
            // who points to the candidate
            for (oe_const_iterator i = from.begin(); i != from.end(); ++i){
                if (i->second.find(pivot_candidate) != i->second.end()){
                    reference_d.insert(i->first);
                }
            }                
#endif            
            return reference_d.size();
        }

        dtuio::helpers::uuid autoremapper::guess_pivot_candidate() const {
            size_t max_reference_count = 0;
            i_primitive::primitive_map::const_iterator winner = m_primitives_held.end();
            
            // ok, let's try using configured points as pivots first...
            
            // use intacts
            for (
                i_primitive::primitive_map::const_iterator pivot_candidate = m_primitives_held.begin();
                pivot_candidate != m_primitives_held.end();
                ++pivot_candidate
            ) {
                if (is_pivot(pivot_candidate->second)){ continue; }
                if (pivot_candidate->second->m_configured == false){ continue; }
                if (pivot_candidate->second->m_setup_mode != sensor_properties::COORDINATE_INTACT){ continue; }
                
                size_t reference_count = get_reference_count(pivot_candidate->first, m_references_from);
                if (reference_count > max_reference_count){
                    max_reference_count = reference_count;
                    winner = pivot_candidate;
                }
            }
            if (winner != m_primitives_held.end()){
                winner->second->m_role |= i_primitive::ROLE_PIVOT;
                return winner->first;
            }

            // use continuous
            for (
                i_primitive::primitive_map::const_iterator pivot_candidate = m_primitives_held.begin();
                pivot_candidate != m_primitives_held.end();
                ++pivot_candidate
            ) {
                if (is_pivot(pivot_candidate->second)){ continue; }
                if (pivot_candidate->second->m_configured == false){ continue; }
                if (pivot_candidate->second->m_setup_mode != sensor_properties::COORDINATE_TRANSLATE_SETUP_CONTINUOUS){ continue; }
                
                size_t reference_count = get_reference_count(pivot_candidate->first, m_references_from);
                if (reference_count > max_reference_count){
                    max_reference_count = reference_count;
                    winner = pivot_candidate;
                }
            }
            if (winner != m_primitives_held.end()){
                winner->second->m_role |= i_primitive::ROLE_PIVOT;
                return winner->first;
            }

            // use once
            for (
                i_primitive::primitive_map::const_iterator pivot_candidate = m_primitives_held.begin();
                pivot_candidate != m_primitives_held.end();
                ++pivot_candidate
            ) {
                if (is_pivot(pivot_candidate->second)){ continue; }
                if (pivot_candidate->second->m_configured == false){ continue; }
                if (pivot_candidate->second->m_setup_mode != sensor_properties::COORDINATE_TRANSLATE_SETUP_ONCE){ continue; }
                
                size_t reference_count = get_reference_count(pivot_candidate->first, m_references_from);
                if (reference_count > max_reference_count){
                    max_reference_count = reference_count;
                    winner = pivot_candidate;
                }
            }
            if (winner != m_primitives_held.end()){
                winner->second->m_role |= i_primitive::ROLE_PIVOT;
                return winner->first;
            }

            // ok, we do not seem to have any existing configuration, let's recompute

            // use continuous
            for (
                i_primitive::primitive_map::const_iterator pivot_candidate = m_primitives_held.begin();
                pivot_candidate != m_primitives_held.end();
                ++pivot_candidate
            ) {
                if (is_pivot(pivot_candidate->second)){ continue; }
                if (pivot_candidate->second->m_configured == true){ continue; }
                if (pivot_candidate->second->m_setup_mode != sensor_properties::COORDINATE_TRANSLATE_SETUP_CONTINUOUS){ continue; }
                
                size_t reference_count = get_reference_count(pivot_candidate->first, m_references_from);
                if (reference_count > max_reference_count){
                    max_reference_count = reference_count;
                    winner = pivot_candidate;
                }
            }
            if (winner != m_primitives_held.end()){
                winner->second->m_role |= i_primitive::ROLE_PIVOT;
                return winner->first;
            }

            // use once
            for (
                i_primitive::primitive_map::const_iterator pivot_candidate = m_primitives_held.begin();
                pivot_candidate != m_primitives_held.end();
                ++pivot_candidate
            ) {
                if (is_pivot(pivot_candidate->second)){ continue; }
                if (pivot_candidate->second->m_configured == true){ continue; }
                if (pivot_candidate->second->m_setup_mode != sensor_properties::COORDINATE_TRANSLATE_SETUP_ONCE){ continue; }
                
                size_t reference_count = get_reference_count(pivot_candidate->first, m_references_from);
                if (reference_count > max_reference_count){
                    max_reference_count = reference_count;
                    winner = pivot_candidate;
                }
            }
            if (winner != m_primitives_held.end()){
                winner->second->m_role |= i_primitive::ROLE_PIVOT;
                return winner->first;
            }

            // unable to select pivot
            return dtuio::helpers::uuid::empty_uuid();
        }
        
        bool autoremapper::reset_commit(){
            bool changes = false;
            
            for (i_primitive::primitive_map::iterator i = m_primitives_held.begin(); i != m_primitives_held.end(); ++i){
                bool reset = true;
                // "sensors" have speciffic way to be reset
                if (is_sensor(i->second)) {
                    if (
                        (
                           (i->second->m_setup_mode == sensor_properties::COORDINATE_TRANSLATE_SETUP_ONCE) 
                        && (i->second->m_configured)
                        ) || (
                           (i->second->m_setup_mode == sensor_properties::COORDINATE_INTACT)
                        )
                    ){
                        reset = false;
                    }
                }
                
                // reset pivot
                i->second->m_role &= ~i_primitive::ROLE_PIVOT;
                    
                if (reset){
                    changes = true;
                    i->second->m_configured = false;
                    i->second->m_position_global = libkerat::helpers::point_3d();
                    if (is_group(i->second)){
                        i->second->m_vp_mode = i_primitive::VIEWPORT_AWAITS;
                        i->second->m_viewport = dtuio::sensor::viewport();
                    }
                }
            }
            
            return changes;
        }
        
        void autoremapper::recalculate_group_viewports(){
            bool changed = false;
            do {
                changed = false;
                for (i_primitive::primitive_map::iterator i = m_primitives_held.begin(); i != m_primitives_held.end(); ++i){
                    i_primitive * current = i->second;
                    
                    // process only groups
                    if ((current->m_role & i_primitive::ROLE_GROUP) == 0){ continue; }
                    // process only groups that are yet uncomputed
                    if (!(current->m_vp_mode & i_primitive::VIEWPORT_AWAITS)){ continue; }
                    
                    dtuio::adaptors::viewport_projector::viewport_list elements;
                    elements.push_back(current->m_viewport);

                    // add children which are already configured
                    bool children_done = true;
                    for (i_primitive::primitive_map::iterator j = current->m_primitives_held.begin(); j != current->m_primitives_held.end(); ++j){
                        if (j->second->m_vp_mode & (i_primitive::VIEWPORT_COMPUTED | i_primitive::VIEWPORT_RECEIVED)){
                            dtuio::sensor::viewport tmp_viewport = j->second->m_viewport;
                            tmp_viewport += j->second->m_position_global;
                            elements.push_back(tmp_viewport);
                        } else {
                            children_done &= false;
                        }
                    }
                    
                    dtuio::sensor::viewport tmp = dtuio::adaptors::viewport_projector::calculate_bounding_viewport(elements);
                    tmp.set_uuid(i->first.get_uuid());
                    // check for change
                    if ((tmp != current->m_viewport) || (children_done)){ changed |= true; }
                    
                    // commance update
                    current->m_viewport = tmp;
                    if (children_done){ current->m_vp_mode = i_primitive::VIEWPORT_COMPUTED; }
                }
            } while (changed);
        }
        
        void autoremapper::commit(){
            // check whether we even need to run the commit
            if (!m_update_required){ return; }
            m_update_required = false;
            
            // commance reset
            if (!reset_commit()){ return; } // no resetting, no computing
            
            bool changed = false;
            
            typedef std::queue<dtuio::helpers::uuid> pivot_queue;
            pivot_queue pivots_to_process;

            // now, for as long as we find out something new...
            do {
                changed = false;
                
                // choose pivot, that is point that shall act as origin...
                dtuio::helpers::uuid pivot_uuid;
                while ((pivot_uuid = guess_pivot_candidate()) != dtuio::helpers::uuid::empty_uuid()){
                    
                    pivots_to_process.push(pivot_uuid);

                    while (!pivots_to_process.empty()) {
                        pivot_uuid = pivots_to_process.front();
                        pivots_to_process.pop();

                        // first of all, establish the pivot by some of already established pivots
                        changed = recompute_location(pivot_uuid);
                        // ok, so pivot placed
                        
                        i_primitive * pivot = ensure_exists(pivot_uuid);
                        pivot->m_configured = true;
                        
                        // now, attempt to align neighbours with their respective positions
                        for (
                            reference_map::iterator i = m_references_from[pivot_uuid].begin();
                            i != m_references_from[pivot_uuid].end();
                            ++i
                        ){
                            i_primitive * neighbour_processed = ensure_exists(i->first);
                            if (neighbour_processed->m_configured){ continue; }
                            
                            //! \todo check for invalid configuration
                            changed |= recompute_location(i->first);
                            neighbour_processed->m_role |= i_primitive::ROLE_PIVOT;
                            pivots_to_process.push(i->first);
                        }
                    }
                }
            } while (changed);

            for (i_primitive::primitive_map::iterator i = m_primitives_held.begin(); i != m_primitives_held.end(); ++i){
                if (!(i->second->m_configured)){
                    std::cerr << "---- " << (i->first) << " unconfigured!" << std::endl;
                }
            }
            
            recalculate_group_viewports();
        }
        
        bool autoremapper::recompute_location(const dtuio::helpers::uuid& calculated_uuid){
            libkerat::helpers::point_3d local;
            libkerat::angle_t azimuth = 0;
            libkerat::angle_t altitude = 0;
            size_t neighbours_pointing = 0;
            size_t neighbours_pointed = 0;
            
            { // now, calculate effect that all existing pivots that point to me have
                reference_map_shell::const_iterator calculated_to = m_references_to.find(calculated_uuid);
                if (calculated_to != m_references_to.end()) {
                    for (
                        reference_map::const_iterator j = calculated_to->second.begin();
                        j != calculated_to->second.end();
                        ++j
                    ){
                        i_primitive * neighbour = ensure_exists(j->first);
                        // count only the ones already configured
                        if (!neighbour->m_configured){ continue; }

                        // compute position relative to given pivot & add up
                        local += neighbour->m_position_global + spherical_to_cartesian(
                            j->second->azimuth + neighbour->m_correction_azimuth, 
                            j->second->altitude + neighbour->m_correction_altitude, 
                            j->second->distance
                        );
                        
                        ++neighbours_pointing;
                    }
                }
            }
            
            { // now, calculate effect that all existing pivots that point to me have
                reference_map_shell::const_iterator calculated_from = m_references_from.find(calculated_uuid);
                if (calculated_from != m_references_from.end()) {

                    for (
                        reference_map::const_iterator j = calculated_from->second.begin();
                        j != calculated_from->second.end();
                        ++j
                    ){
                        i_primitive * neighbour = ensure_exists(j->first);
                        // count only the ones already configured
                        if (!neighbour->m_configured){ continue; }

                        local += neighbour->m_position_global - spherical_to_cartesian(
                            j->second->azimuth + neighbour->m_correction_azimuth, 
                            j->second->altitude + neighbour->m_correction_altitude, 
                            j->second->distance
                        );
                        
                        ++neighbours_pointed;
                    }
                }
            }

            // let's find out where we are
            if ((neighbours_pointing + neighbours_pointed) > 0){ 
                local /= (neighbours_pointing + neighbours_pointed);
                // rounding added since the computation above is aproximative anyway
                local.set_x((int64_t)local.get_x());
                local.set_y((int64_t)local.get_y());
                local.set_z((int64_t)local.get_z());
            }
            
            // calculate the average rotations
            if (neighbours_pointed > 0){ 
                
                reference_map_shell::const_iterator calculated_from = m_references_from.find(calculated_uuid);
                if (calculated_from != m_references_from.end()) {

                    for (
                        reference_map::const_iterator j = calculated_from->second.begin();
                        j != calculated_from->second.end();
                        ++j
                    ){
                        i_primitive * neighbour = ensure_exists(j->first);
                        // count only the ones already configured
                        if (!neighbour->m_configured){ continue; }

                        libkerat::distance_t dist = 0;
                        libkerat::angle_t alt = 0;
                        libkerat::angle_t azi = 0;
                        
                        cartesian_to_spherical(neighbour->m_position_global - local, azi, alt, dist);
                        
                        azimuth += (azi - j->second->azimuth);
                        altitude += (alt - j->second->altitude);
                    }
                }
                
                azimuth /= neighbours_pointed;
                altitude /= neighbours_pointed;
            }
            
            // store computed location
            m_primitives_held[calculated_uuid]->m_position_global = local;
            m_primitives_held[calculated_uuid]->m_correction_azimuth = azimuth;
            m_primitives_held[calculated_uuid]->m_correction_altitude = altitude;
            
            // does somebody affect me?
            return ((neighbours_pointing + neighbours_pointed) > 0);
        }
        
        libkerat::helpers::point_3d autoremapper::get_absolute_position(const dtuio::helpers::uuid& reference, const dtuio::helpers::uuid& target) const {
            libkerat::helpers::point_3d retval;
            libkerat::angle_t azimuth = 0;
            libkerat::angle_t altitude = 0;
            
            // gain access to reference's position
            i_primitive::primitive_map::const_iterator reference_iter = m_primitives_held.find(reference);
            if (reference_iter != m_primitives_held.end()){
                retval = reference_iter->second->m_position_global;
                azimuth = reference_iter->second->m_correction_azimuth;
                altitude = reference_iter->second->m_correction_altitude;
            }
            
            
            // recalculate
            reference_map_shell::const_iterator reference_e_iter = m_references_from.find(reference);
            if (reference_e_iter != m_references_from.end()){
                reference_map::const_iterator target_e_iter = reference_e_iter->second.find(target);
                if (target_e_iter != reference_e_iter->second.end()) {
                    altitude += target_e_iter->second->altitude;
                    azimuth += target_e_iter->second->azimuth;
                    // calculate global
                    retval += spherical_to_cartesian(azimuth, altitude, target_e_iter->second->distance);
                }
            }
            
            return retval;
        }
        
        void autoremapper::process_group_registrations(const libkerat::bundle_handle& to_process){
            int i = 0;
            const dtuio::sensor_topology::group_member * member = NULL;
            while ((member = to_process.get_message_of_type<dtuio::sensor_topology::group_member>(i)) != NULL){
                process_group_registration(member);
                ++i;
            }
        }
        void autoremapper::process_neighbour_registrations(const libkerat::bundle_handle& to_process){
            int i = 0;
            const dtuio::sensor_topology::neighbour * neighbour = NULL;
            while ((neighbour = to_process.get_message_of_type<dtuio::sensor_topology::neighbour>(i)) != NULL){
                process_neighbour_registration(neighbour);
                ++i;
            }
        }
        void autoremapper::process_sensor_registrations(const libkerat::bundle_handle& to_process){
            int i = 0;
            const sensor_properties * sensor = NULL;
            while ((sensor = to_process.get_message_of_type<sensor_properties>(i)) != NULL){
                process_sensor_registration(sensor);
                ++i;
            }
        }
        void autoremapper::process_viewport_registrations(const libkerat::bundle_handle& to_process){
            int i = 0;
            const dtuio::sensor::viewport * vpt = NULL;
            while ((vpt = to_process.get_message_of_type<dtuio::sensor::viewport>(i)) != NULL){
                process_viewport_registration(vpt);
                ++i;
            }
        }
        
        autoremapper::i_primitive * autoremapper::ensure_exists(const  dtuio::helpers::uuid & primitive_uuid){
            i_primitive * primitive = NULL;
            i_primitive::primitive_map::iterator it_primitive = m_primitives_held.find(primitive_uuid);
            if (it_primitive == m_primitives_held.end()){
                primitive = new i_primitive(this);
                m_primitives_held[primitive_uuid] = primitive;
            } else {
                primitive = it_primitive->second;
            }
           
            return primitive;
        }
        
        void autoremapper::process_group_registration(const dtuio::sensor_topology::group_member * member){
            i_primitive * group_entry = ensure_exists(member->get_group_uuid());
            
            // check whether this is meaningful
            if (
                (group_entry->m_role == autoremapper::i_primitive::ROLE_GROUP)
             && (group_entry->m_primitives_held.find(member->get_uuid()) != group_entry->m_primitives_held.end())
            ){
                return;
            }
            
            i_primitive * sensor_entry = ensure_exists(member->get_uuid());

            m_update_required = true;
            // if this is yet undiscovered, make it a group
            group_entry->m_role |= autoremapper::i_primitive::ROLE_GROUP;
            
            //if (group_entry == NULL) {
            //    std::cerr << "MUSE/Autoremapper: group " << (dtuio::helpers::group_uuid &)*member
            //        << " could not have been created" << std::endl;
            //}
            //if (i_sensor == NULL) {
            //    std::cerr << "MUSE/Autoremapper: sensor " << (dtuio::helpers::uuid &)*member
            //        << " could not have been created" << std::endl;
            //}
#if 0 // makes problems for delayed group registration           
            // check whether this setup is not a duplicit
            if ((   (sensor_entry->m_setup_mode == sensor_properties::COORDINATE_INTACT)
                 || (sensor_entry->m_setup_mode == sensor_properties::COORDINATE_TRANSLATE_SETUP_ONCE)
               ) && (sensor_entry->m_configured)
            ){
                return;
            }
#endif            
            // update group connections
            if (sensor_entry->m_parent != NULL){
                if (sensor_entry->m_parent != group_entry){
                    // destroy previous relation
                    sensor_entry->m_parent->m_primitives_held.erase(member->get_uuid());
                    
                    // create new realtion
                    group_entry->m_primitives_held.insert(i_primitive::primitive_map::value_type(member->get_uuid(), sensor_entry));
                    sensor_entry->m_parent = group_entry;
                } else {
                    // noop, already done
                }
            } else {
                group_entry->m_primitives_held.insert(i_primitive::primitive_map::value_type(member->get_uuid(), sensor_entry));
                sensor_entry->m_parent = group_entry;
            }
            // sensor assigned to the group
        }
        void autoremapper::process_sensor_registration(const sensor_properties * sensor){
            i_primitive * sensor_entry = ensure_exists(sensor->get_uuid());
            
            if (
                (sensor_entry->m_role & i_primitive::ROLE_SENSOR)
             && (sensor_entry->m_setup_mode == sensor->get_coordinate_translation_mode())
            ){
                // nothink to update here
                return;
            }
                
            m_update_required = true;
            sensor_entry->m_role |= i_primitive::ROLE_SENSOR;
            sensor_entry->m_setup_mode = sensor->get_coordinate_translation_mode();

            // take care of intacts
            if (sensor_entry->m_setup_mode == sensor_properties::COORDINATE_INTACT){
                sensor_entry->m_configured = true;
                sensor_entry->m_position_global = libkerat::helpers::point_3d(0, 0, 0);
            }
        }
        void autoremapper::process_viewport_registration(const dtuio::sensor::viewport * vpt){
            i_primitive * sensor_entry = ensure_exists(vpt->get_uuid());
            
            // watch for change
            if ((sensor_entry->m_vp_mode == i_primitive::VIEWPORT_RECEIVED) && (sensor_entry->m_viewport == *vpt)){
                return;
            }

            m_update_required = true;
            sensor_entry->m_vp_mode = i_primitive::VIEWPORT_RECEIVED;
            sensor_entry->m_viewport = *vpt;
        }
        void autoremapper::process_neighbour_registration(const dtuio::sensor_topology::neighbour* neighbour){
            i_entry neighbour_entry;
            neighbour_entry.altitude = neighbour->get_altitude();
            neighbour_entry.azimuth  = neighbour->get_azimuth();
            neighbour_entry.distance = neighbour->get_distance();
            
            // check for update requirement
            reference_map_shell::const_iterator outer = m_references_to.find(neighbour->get_uuid());
            if (outer != m_references_to.end()){
                reference_map::const_iterator inner = outer->second.find(neighbour->get_neighbour_uuid());
                if ((inner != outer->second.end()) && (*(inner->second) == neighbour_entry)){
                    return;
                }
            }
            
            m_update_required = true;
            push_entry(neighbour->get_uuid(), neighbour->get_neighbour_uuid(), neighbour_entry);
        }
            
        void autoremapper::push_entry(const dtuio::helpers::uuid& from, const dtuio::helpers::uuid& to, const i_entry& entry){
            reference_map & from_inner = m_references_from[from];
            reference_map::iterator target_entry = from_inner.find(to);
            if (target_entry != from_inner.end()){
                *(target_entry->second) = entry;
                return;
            }
            
            // else, safely assume that such entry does not exist and create it
            
            // from->to
            i_entry * new_entry = new i_entry(entry);
            from_inner.insert(reference_map::value_type(to, new_entry));
            
            // to->from
            m_references_to[to].insert(reference_map::value_type(from, new_entry));
        }
        
        void autoremapper::delete_entry(const dtuio::helpers::uuid & from, const dtuio::helpers::uuid & to){
            reference_map & from_inner = m_references_from[from];
            reference_map & to_inner   = m_references_to[to];
            
            reference_map::iterator target_entry = from_inner.find(to);
            if (target_entry != from_inner.end()){
                delete target_entry->second;
                from_inner.erase(target_entry);
            }
            
            target_entry = to_inner.find(from);
            if (target_entry != to_inner.end()){
                to_inner.erase(target_entry);
            }
        }
        
        void autoremapper::delete_entry(const dtuio::helpers::uuid & that_one){
            { // delete all entries pointing to me
                reference_map_shell::iterator to_inner = m_references_to.find(that_one);
                if (to_inner == m_references_to.end()){
                for (reference_map::iterator to = to_inner->second.begin(); to != to_inner->second.end(); ++to){
                        m_references_from[to->first].erase(that_one);
                        delete to->second;
                        to->second = NULL;
                    }
                    m_references_to.erase(to_inner);
                }
            }
            
            { // delete all entries I am pointing to
                reference_map_shell::iterator from_inner = m_references_from.find(that_one);
                if (from_inner == m_references_from.end()){
                for (reference_map::iterator from = from_inner->second.begin(); from != from_inner->second.end(); ++from){
                        m_references_to[from->first].erase(that_one);
                        delete from->second;
                        from->second = NULL;
                    }
                    m_references_from.erase(from_inner);
                }
            }
        }
        
        void autoremapper::destroy_primitive(const dtuio::helpers::uuid & uuid){
            i_primitive::primitive_map::iterator it_primitive = m_primitives_held.find(uuid);
            if (it_primitive == m_primitives_held.end()) { return; }

            i_primitive * sensor = it_primitive->second;
            i_primitive * parent = sensor->m_parent;
            
            if (parent != NULL) {
                i_primitive::primitive_map::iterator p = parent->m_primitives_held.find(uuid);
                if (p != parent->m_primitives_held.end()) { 
                    parent->m_primitives_held.erase(p); 
                    sensor->m_parent = NULL;
                }
            }
            
            m_primitives_held.erase(it_primitive);
        }
        
        bool autoremapper::load(int count){
            return libkerat::adaptor::load(count);
        }

        bool autoremapper::load(int count, struct timespec timeout){
            return libkerat::adaptor::load(count, timeout);
        }

        void autoremapper::notify(const libkerat::client * cl){
            purge();
            
            libkerat::bundle_stack data = cl->get_stack();

            while (data.get_length() > 0){
                libkerat::bundle_handle * tmphx = new libkerat::bundle_handle;
                libkerat::bundle_handle current_frame = data.get_update(libkerat::bundle_stack::INDEX_OLDEST);
                process_bundle(current_frame, *tmphx);
                bm_stack_append(m_processed_frames, tmphx);
            }
            
            notify_listeners();
        }
        
        libkerat::bundle_stack autoremapper::get_stack() const { return m_processed_frames; }

        void autoremapper::purge(){ bm_stack_clear(m_processed_frames); }

    }
}

/**
 * \file      autoconfiguration.hpp
 * \brief     Provides the core autoconfiguration functionality
 * \author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-11-28 00:46 UTC+1
 * \copyright BSD
 */

#ifndef MUSE_EINSTEIN_HPP
#define MUSE_EINSTEIN_HPP

#include <kerat/kerat.hpp>
#include <dtuio/dtuio.hpp>
#include <uuid/uuid.h>
#include <map>

namespace muse {
    namespace virtual_sensors {

        class autoremapper: public libkerat::adaptor,
            private libkerat::internals::frame_manager,
            private libkerat::internals::session_manager
        {
        public:
            autoremapper(
                bool cut_received_topology = true,
                dtuio::sensor::sensor_properties::coordinate_translation_mode_t default_mode
                    = dtuio::sensor::sensor_properties::COORDINATE_TRANSLATE_SETUP_ONCE
            );

            ~autoremapper();

            void notify(const libkerat::client * notifier);

            libkerat::bundle_stack get_stack() const ;

            int process_bundle(const libkerat::bundle_handle & to_process, libkerat::bundle_handle & output_frame);

            void purge();
            
            bool load(int count = 1);
            bool load(int count, struct timespec timeout);

        private:
            struct i_primitive: 
                public dtuio::helpers::uuid
            {
                typedef uint8_t role_t;
                typedef uint8_t viewport_mode_t;
                
                static const role_t ROLE_UNKNOWN = 0 << 0;
                static const role_t ROLE_SENSOR =  1 << 0;
                static const role_t ROLE_GROUP =   1 << 1;
                static const role_t ROLE_ALL_DEVICES = ROLE_SENSOR | ROLE_GROUP;
                static const role_t ROLE_PIVOT =   1 << 2;
                
                static const viewport_mode_t VIEWPORT_UNSET =    0 << 0;
                static const viewport_mode_t VIEWPORT_RECEIVED = 1 << 0;
                static const viewport_mode_t VIEWPORT_COMPUTED = 1 << 1;
                static const viewport_mode_t VIEWPORT_AWAITS =   1 << 2;
                
                    
                typedef std::map<dtuio::helpers::uuid, i_primitive *> primitive_map;
                
                i_primitive(autoremapper * einstein);
                virtual ~i_primitive();
                
                // commons
                role_t m_role;
                viewport_mode_t m_vp_mode;
                autoremapper * m_remapper;
                bool m_configured;
                i_primitive * m_parent;
                
                
                dtuio::sensor::viewport m_viewport;
                
                

                // sensor
                libkerat::helpers::point_3d m_position_global;
                libkerat::angle_t m_correction_azimuth;
                libkerat::angle_t m_correction_altitude;
                dtuio::sensor::sensor_properties::coordinate_translation_mode_t m_setup_mode;

                // group
                primitive_map m_primitives_held;
            };
            
            //! \brief this is essentialy the same as in dtuio::topology::neighbour
            struct i_entry {
                libkerat::angle_t azimuth;
                libkerat::angle_t altitude;
                libkerat::distance_t distance;
                
                bool operator==(const i_entry & second){ 
                    return 
                        (azimuth == second.azimuth)
                     && (altitude == second.altitude) 
                     && (distance == second.distance); 
                }
            };
            
            typedef std::map<dtuio::helpers::uuid, i_entry *> reference_map;
            typedef std::map<dtuio::helpers::uuid, reference_map> reference_map_shell; 
            
            typedef std::map<dtuio::helpers::uuid, size_t> inner_distance;
            typedef std::map<dtuio::helpers::uuid, inner_distance> outer_distance;
            
            // group auxiliary
            void destroy_primitive(const dtuio::helpers::uuid & uuid);
            
            i_primitive * ensure_exists(const dtuio::helpers::uuid & group_uuid);
            
            // einstein aux
            void push_entry(const dtuio::helpers::uuid & from, const dtuio::helpers::uuid & to, const i_entry & entry);
            void delete_entry(const dtuio::helpers::uuid & from, const dtuio::helpers::uuid & to);
            void delete_entry(const dtuio::helpers::uuid & that_one);
            void setup_auto_threshold(const libkerat::distance_t & dist1, const libkerat::distance_t & dist2);
            dtuio::helpers::uuid guess_pivot_candidate() const ;
            
            libkerat::helpers::point_3d get_absolute_position(const dtuio::helpers::uuid & reference, const dtuio::helpers::uuid & target) const ;

            bool recompute_location(const dtuio::helpers::uuid & drifted);

            // processors
            void process_group_registrations(const libkerat::bundle_handle & to_process);
            void process_group_registration(const dtuio::sensor_topology::group_member * member);
            
            void process_neighbour_registrations(const libkerat::bundle_handle & to_process);
            void process_neighbour_registration(const dtuio::sensor_topology::neighbour * neighbour);
            
            void process_sensor_registrations(const libkerat::bundle_handle & to_process);
            void process_sensor_registration(const dtuio::sensor::sensor_properties * sensor);

            void process_viewport_registrations(const libkerat::bundle_handle & to_process);
            void process_viewport_registration(const dtuio::sensor::viewport * vpt);

            // the autoconfiguration core
            void commit();
            bool reset_commit();
            void recalculate_group_viewports();
            
            static libkerat::helpers::point_3d compensate_drift_3d(const libkerat::angle_t azimuth, const libkerat::angle_t altitude, const libkerat::helpers::point_3d & original);
            int translate_bundle(const i_primitive * sensor, libkerat::bundle_handle & to_process);
            void project_group_viewports(libkerat::bundle_handle & to_process);

            bool m_cut_topology;
            dtuio::sensor::sensor_properties::coordinate_translation_mode_t m_default_mode;
            i_primitive::primitive_map m_primitives_held;
            
            reference_map_shell m_references_to;
            reference_map_shell m_references_from;
            
            bool m_update_required;
            libkerat::distance_t m_threshold;
            libkerat::bundle_stack m_processed_frames;
        };

    } // ns virtual_sensors
} // ns muse

#endif // MUSE_EINSTEIN_HPP

/**
 * \file      dtuio_marker.hpp
 * \brief     Adaptor that changes plain TUIO 2.0 bundles into dTUIO bundles
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-12-03 15:33 UTC+1
 * \copyright BSD
 */


#ifndef DTUIO_ADAPTOR_MARKER_HPP
#define DTUIO_ADAPTOR_MARKER_HPP

#include <kerat/kerat.hpp>
#include <dtuio/typedefs.hpp>
#include <dtuio/helpers.hpp>
#include <dtuio/sensor_properties.hpp>

namespace dtuio {
    //! \brief Holds dTUIO base functionality adaptors
    namespace adaptors {
        class marker: public libkerat::adaptor {
        public:

            marker(
                sensor::sensor_properties::coordinate_translation_mode_t mode = sensor::sensor_properties::COORDINATE_INTACT, 
                sensor::sensor_properties::sensor_purpose_t purpose = sensor::sensor_properties::PURPOSE_EVENT_SOURCE
            );

            virtual ~marker();

            void notify(const libkerat::client * notifier);

            void purge();

            //! \brief see sensor::sensor_properties for detail
            void set_default_coordinate_translation_mode(const sensor::sensor_properties::coordinate_translation_mode_t & mode);
            //! \brief see sensor::sensor_properties for detail
            sensor::sensor_properties::coordinate_translation_mode_t get_default_coordinate_translation_mode() const;
            //! \brief see sensor::sensor_properties for detail
            void set_default_sensor_purpose(const sensor::sensor_properties::sensor_purpose_t & purpose);
            //! \brief see sensor::sensor_properties for detail
            sensor::sensor_properties::sensor_purpose_t get_default_sensor_purpose() const;

            libkerat::bundle_stack get_stack() const;

            int process_bundle(const libkerat::bundle_handle & to_process, libkerat::bundle_handle & output_frame);

        private:

            int process_bundle(libkerat::bundle_handle & to_process);

            typedef struct tuio_source_key_type {
                bool operator<(const tuio_source_key_type & second) const ;

                //! \brief Source IPv4 address of the bundle
                libkerat::addr_ipv4_t addr;
                //! \brief Source tracker instance of the bundle
                libkerat::instance_id_t instance;
                //! \brief Source tracker name
                std::string application;
            } source_key_type;

            typedef std::map<source_key_type, sensor::sensor_properties *> injections_map;
            
        protected:
            void inject(libkerat::bundle_handle & to_process, sensor::sensor_properties * signature);

            sensor::sensor_properties::coordinate_translation_mode_t m_mode; 
            sensor::sensor_properties::sensor_purpose_t m_purpose;
            injections_map m_injections;
            libkerat::bundle_stack m_processed_frames;

        };
    }
}

#endif // DTUIO_ADAPTOR_MARKER_HPP

/**
 * \file      sensor_description.hpp
 * \brief     Define the message that describes the sensor type, coordinate handling and grouping properties
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-10-28 20:37 UTC+1
 * \copyright BSD
 */


#ifndef DTUIO_SENSOR_PROPERTIES_HPP
#define DTUIO_SENSOR_PROPERTIES_HPP

#include <kerat/kerat.hpp>
#include <dtuio/typedefs.hpp>
#include <dtuio/helpers.hpp>
#include <string>
#include <stdexcept>

namespace dtuio {
    //! \brief Messages related to sensor properties, capabilities and limitations
    namespace sensor {
        //! \brief Describes sensor type and purpose
        class sensor_properties: public libkerat::kerat_message, public dtuio::helpers::uuid {
        public:
            //! \brief how to handle this sensor in autoconfigurated installations
            typedef enum { 
                //! \brief Do not translate coordinates from this sensor
                COORDINATE_INTACT = 0, 
                //! \brief Ignore all coordinate system setup messages except for the initial one
                COORDINATE_TRANSLATE_SETUP_ONCE = 1, 
                //! \brief Follow all coordinate setup commands
                COORDINATE_TRANSLATE_SETUP_CONTINUOUS = 2
            } coordinate_translation_mode_t;
            
            //! \brief whether this sensor shall be considered a event source or event metadata provider
            typedef enum { 
                //! This sensor provides no data, serves as observer only
                PURPOSE_OBSERVER = 0, 
                //! This sensor is standard event source
                PURPOSE_EVENT_SOURCE = 1, 
                //! This sensor provides additional metadata information
                PURPOSE_TAGGER = 2
            } sensor_purpose_t;
            
            sensor_properties();
            
            //! \param mode coordinate translation mode (autoconfigurated environments) (\see coordinate_translation_mode_t)
            //! \param purpose whether to use this sensor as event or metadata provider (mostly for autoconfigurated environments) (\see sensor_purpose_t)
            //! \note Equivalent for setting the sensor UUID as empty UUID
            sensor_properties(const coordinate_translation_mode_t & mode, const sensor_purpose_t & purpose);
            
            //! \param id UUID of the sensor being described
            //! \param mode coordinate translation mode (autoconfigurated environments) (\see coordinate_translation_mode_t)
            //! \param purpose whether to use this sensor as event or metadata provider (mostly for autoconfigurated environments) (\see sensor_purpose_t)
            sensor_properties(const uuid_t & id, const coordinate_translation_mode_t & mode, const sensor_purpose_t & purpose);
            
            sensor_properties(const sensor_properties & original);
            virtual ~sensor_properties();
            
            //! \brief Sets the coordinate translation mode for this sensor
            //! \param mode sensor coordinate translation mode to set
            //! \note See \ref coordinate_translation_mode_t
            void set_coordinate_translation_mode(const coordinate_translation_mode_t & mode);

            //! \brief gets the coordinate translation mode for this sensor
            inline coordinate_translation_mode_t get_coordinate_translation_mode() const { 
                return m_coordinate_translation; 
            }
            
            //! \brief Seths whether this sensor should be considered as event or metadata provider
            //! \param purpose which kind of data this sensor provides
            //! \note See \ref sensor_purpose_t
            void set_sensor_purpose(const sensor_purpose_t & purpose);

            //! \brief tells whether this sensor should be considered as event or metadata provider
            inline sensor_purpose_t get_sensor_purpose() const {
                return m_purpose;
            }
                
            libkerat::kerat_message * clone() const ;
            
            void print(std::ostream & output) const ;
                
            //! \brief OSC message path for this dTUIO message
            static const char * PATH;
        private:
            bool imprint_lo_messages(lo_bundle target) const;
            coordinate_translation_mode_t m_coordinate_translation;
            sensor_purpose_t m_purpose;
        };
    }
}

std::ostream & operator<<(std::ostream & output, const dtuio::sensor::sensor_properties & sensor_properties);

#endif // DTUIO_SENSOR_PROPERTIES_HPP

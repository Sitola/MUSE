/**
 * \file      topology_neighbour.hpp
 * \brief     Define the message that identifies neighbouring sensor
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-10-28 11:11 UTC+1
 * \copyright BSD
 */

#ifndef DTUIO_TOPOLOGY_NEIGHBOUR_HPP
#define DTUIO_TOPOLOGY_NEIGHBOUR_HPP

#include <kerat/kerat.hpp>
#include <dtuio/typedefs.hpp>
#include <dtuio/helpers.hpp>
#include <string>
#include <stdexcept>

namespace dtuio {
    
    //! \brief Sensor placement & topology messages
    namespace sensor_topology {

        /** 
         * \brief Discovered neigbour notification
         * 
         * Some sensors can detect other sensors within thei'r vicinity
         * (eg. Kinect hanging over the tabletop), this information can be utilized
         * for the environment autoconfiguration. This message represents the
         * discovered neighbour, local direction (polar coordinate system)
         * and distance to the neighbour itself.
         */
        class neighbour: public libkerat::kerat_message, public dtuio::helpers::uuid, public dtuio::helpers::neighbour_uuid {
        public:
            neighbour();
            neighbour(const neighbour & original);
            
            /**
             * \brief Represents neigbour of unknown identity
             * \note Equivalent for the neighbour message with neigbour UUID set
             * to empty UUID.
             * \param self UUID of the sensor who made the discovery
             * \param azimuth azimuth to the neighbour [rad], local polar coordinate system (relative to self)
             * \param altitude altitude to the neighbour [rad], local polar coordinate system (relative to self)
             * \param distance distance to the neighbour [px], local polar coordinate system (relative to self)
             */
            neighbour(const dtuio::uuid_t & self, libkerat::angle_t azimuth, libkerat::angle_t altitude, libkerat::distance_t distance);

            /**
             * \brief Represents neigbour of known identity
             * \param self UUID of the sensor who made the discovery
             * \param azimuth azimuth to the neighbour [rad], local polar coordinate system (relative to self)
             * \param altitude altitude to the neighbour [rad], local polar coordinate system (relative to self)
             * \param distance distance to the neighbour [px], local polar coordinate system (relative to self)
             * \param uuid UUID of the discovered neighbour
             */
            neighbour(const dtuio::uuid_t & self, libkerat::angle_t azimuth, libkerat::angle_t altitude, libkerat::distance_t distance, const dtuio::uuid_t & uuid);

            virtual ~neighbour();
            
            /**
             * \brief Sets the azimuth
             * \param azimuth in radians relative to x axis
             */
            void set_azimuth(libkerat::angle_t azimuth);
            
            /**
             * \brief Gets the currently set azimuth
             * \return azimuth in radians
             */
            libkerat::angle_t get_azimuth() const { return m_azimuth; }
            
            /**
             * \brief Sets the altitude
             * \param altitude in radians relative to x axis
             */
            void set_altitude(libkerat::angle_t altitude);
            
            /**
             * \brief Gets the currently set altitude
             * \return altitude in radians
             */
            libkerat::angle_t get_altitude() const { return m_altitude; }
            
            /**
             * \brief Sets the distance to the neigbouring sensor
             * \param distance in pixels relative to center of this sensor
             */
            void set_distance(libkerat::distance_t distance);
            
            /**
             * \brief Gets the currently set distance
             * \return distance to the neighbouring sensor
             */
            libkerat::distance_t get_distance() const { return m_distance; }
            
            libkerat::kerat_message * clone() const ;
                
            void print(std::ostream & output) const ;
                
            //! \brief OSC message path for this dTUIO message
            static const char * PATH;
        private:
            bool imprint_lo_messages(lo_bundle target) const;

            libkerat::angle_t m_azimuth;
            libkerat::angle_t m_altitude;
            libkerat::distance_t m_distance;
        };
    }
}

std::ostream & operator<<(std::ostream & output, const dtuio::sensor_topology::neighbour & neighbour);

#endif // DTUIO_TOPOLOGY_NEIGHBOUR_HPP

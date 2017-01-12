/**
 * \file      topology_group_member.hpp
 * \brief     Define the message that makes the sensor join group
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-11-21 01:10 UTC+1
 * \copyright BSD
 */


#ifndef DTUIO_TOPOLOGY_GROUP_MEMBER_HPP
#define DTUIO_TOPOLOGY_GROUP_MEMBER_HPP

#include <kerat/kerat.hpp>
#include <dtuio/typedefs.hpp>
#include <dtuio/helpers.hpp>
#include <string>
#include <stdexcept>

namespace dtuio {
    namespace sensor_topology {
        //! \brief Sensor/group registration into sensor group
        class group_member: public libkerat::kerat_message, public dtuio::helpers::group_uuid, public dtuio::helpers::uuid {
        public:
            
            //! \param group_id UUID of the target group
            //! \param sensor_id UUID of the sensor joining the group
            group_member(const uuid_t & group_id, const uuid_t & sensor_id);
            group_member(const group_member & original);
            virtual ~group_member();

            libkerat::kerat_message * clone() const ;

            void print(std::ostream & output) const ;
                
            //! \brief OSC message path for this dTUIO message
            static const char * PATH;
        private:
            bool imprint_lo_messages(lo_bundle target) const;
        };
    }
}

std::ostream & operator<<(std::ostream & output, const dtuio::sensor_topology::group_member & group_member);

#endif // DTUIO_TOPOLOGY_GROUP_MEMBER_HPP

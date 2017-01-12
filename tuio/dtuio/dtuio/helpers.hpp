/**
 * \file      helpers.hpp
 * \brief     Define the message helper classes in the same way libkerat does
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-10-28 11:11 UTC+1
 * \copyright BSD
 */

#ifndef DTUIO_HELPERS_HPP
#define DTUIO_HELPERS_HPP

#include <kerat/kerat.hpp>
#include <dtuio/typedefs.hpp>
#include <string>
#include <stdexcept>

namespace dtuio {
    //! \brief Helper classes for dTUIO messages
    namespace helpers {
        // uuid, group_uuid and neighbour_uuid are pretty much the same
        
        //! \brief Class representing the UUID (universaly unique ID) of the sensor
        class uuid {
        public:
            uuid();
            uuid(const uuid & original);
            uuid(const dtuio::uuid_t & input_id);

            //! \param uuid in the string format
            //! \throws std::invalid_argument exception when given UUID string is invalid or has wrong format
            uuid(const std::string & uuid) throw (std::invalid_argument);
            virtual ~uuid();
            
            //! \brief Sets the universaly unique ID of the sensor
            //! \param uuid in the libuuid format (individual bytes, not string)
            void set_uuid(const dtuio::uuid_t uuid);
                
            //! \brief Gets the currently set universaly unique ID of the sensor
            //! \return uuid to the uuiding sensor in the libuuid format
            const dtuio::uuid_t & get_uuid() const { return m_uuid; }
            
            /**
             * \brief Sets the universaly unique ID of the uuiding sensor
             * \param uuid in the string format
             * \throws std::invalid_argument exception when given UUID string is invalid or has wrong format
             */
            void set_uuid(const std::string & uuid) throw (std::invalid_argument);
            
            //! \brief wrapper for uuid_compare
            bool operator<(const uuid & second) const ;
            
            //! \brief wrapper for uuid_compare
            bool operator==(const uuid & second) const ;

            //! \brief wrapper for uuid_compare
            inline bool operator!=(const uuid & second) const { return !operator==(second); }
          
            //! \brief returns empty uuid
            static uuid empty_uuid(); 
           
        protected:
            dtuio::uuid_t m_uuid;
        };
        
        //! \brief Class representing the UUID of sensor group
        //! \see dtuio::helpers::uuid for details
        class group_uuid {
        public:
            group_uuid();
            group_uuid(const group_uuid & original);
            group_uuid(const dtuio::uuid_t & group_id);

            //! \param uuid in the string format
            //! \throws std::invalid_argument exception when given UUID string is invalid or has wrong format
            group_uuid(const std::string & uuid) throw (std::invalid_argument);
            virtual ~group_uuid();
            
            //! \brief Sets the universaly unique ID of the group
            //! \param uuid in the libuuid format
            void set_group_uuid(const dtuio::uuid_t uuid);
                
            //! \brief Gets the currently set universaly unique ID of the group
            //! \return uuid to the uuiding sensor in the libuuid format
            const dtuio::uuid_t & get_group_uuid() const { return m_group_uuid; }
            
            /**
             * \brief Sets the universaly unique ID of the group
             * \param uuid in the string format
             * \throws std::invalid_argument exception when given uuid is invalid or in wrong format
             */
            void set_group_uuid(const std::string & uuid) throw (std::invalid_argument);
            
            //! \brief wrapper for uuid_compare
            bool operator<(const dtuio::helpers::group_uuid & uuid2) const;
            
            //! \brief wrapper for uuid_compare
            bool operator==(const group_uuid & second) const ;

            //! \brief wrapper for uuid_compare
            inline bool operator!=(const group_uuid & second) const { return !operator==(second); }

            //! \brief returns empty group uuid
            static group_uuid empty_uuid(); 
        protected:
            dtuio::uuid_t m_group_uuid;
        };
        
        //! \brief Class representing the UUID of neighbouring sensor
        //! \see dtuio::helpers::uuid for details
        class neighbour_uuid {
        public:
            neighbour_uuid();
            neighbour_uuid(const neighbour_uuid & original);
            neighbour_uuid(const dtuio::uuid_t & neighbour_id);

            //! \param uuid in the string format
            //! \throws std::invalid_argument exception when given UUID string is invalid or has wrong format
            neighbour_uuid(const std::string & uuid) throw (std::invalid_argument);
            virtual ~neighbour_uuid();
            
            //! \brief Sets the UUID of the neighbour
            //! \param uuid in the libuuid format
            void set_neighbour_uuid(const dtuio::uuid_t uuid);
                
            //! \brief Gets the currently set UUID of the neighbour
            //! \return uuid to the uuiding sensor in the libuuid format
            const dtuio::uuid_t & get_neighbour_uuid() const { return m_neighbour_uuid; }
            
            /**
             * \brief Sets the universaly unique ID of the neighbour
             * \param uuid in the string format
             * \throws std::invalid_argument exception when given uuid is invalid or in wrong format
             */
            void set_neighbour_uuid(const std::string & uuid) throw (std::invalid_argument);
            
            //! \brief wrapper for uuid_compare
            bool operator<(const dtuio::helpers::neighbour_uuid & uuid2) const;
            
            //! \brief wrapper for uuid_compare
            bool operator==(const neighbour_uuid & second) const ;

            //! \brief wrapper for uuid_compare
            inline bool operator!=(const neighbour_uuid & second) const { return !operator==(second); }

            //! \brief returns empty group uuid
            static neighbour_uuid empty_uuid(); 
        protected:
            dtuio::uuid_t m_neighbour_uuid;
        };
    }
}

// output operators
std::ostream & operator<<(std::ostream & output, const dtuio::helpers::uuid & uuid);
std::ostream & operator<<(std::ostream & output, const dtuio::helpers::group_uuid & uuid);
std::ostream & operator<<(std::ostream & output, const dtuio::helpers::neighbour_uuid & uuid);

#endif // DTUIO_HELPERS_HPP

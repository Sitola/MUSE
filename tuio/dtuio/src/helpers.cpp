/**
 * \file      helpers.cpp
 * \brief     Implement the helper classes in similar way libkerat does
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-10-28 12:00 UTC+1
 * \copyright BSD
 */

#ifdef HAVE_CONFIG_H
    #include "../config.h"
#endif

#include <dtuio/helpers.hpp>
#include <kerat/utils.hpp>
#include <uuid/uuid.h>
#include <stdexcept>
#include <iosfwd>

namespace dtuio {
    namespace helpers {
        
        uuid::uuid(){ 
            uuid_clear(m_uuid);
        }

        uuid::uuid(const dtuio::uuid_t & uuid) {
            uuid_copy(m_uuid, uuid);
        }
        
        uuid::uuid(const uuid & original){
            uuid_copy(m_uuid, original.m_uuid);
        }

        uuid::uuid(const std::string& uuid) throw (std::invalid_argument) {
            dtuio::uuid_t tmp;
            int retval = uuid_parse(uuid.c_str(), tmp);
            if (retval != 0){
                throw std::invalid_argument("Given UUID is invalid!");
            }
            
            uuid_copy(m_uuid, tmp);
        }
        
        uuid::~uuid(){ ; }
        
        void uuid::set_uuid(const dtuio::uuid_t uuid){
            uuid_copy(m_uuid, uuid);
        }
        
        void uuid::set_uuid(const std::string& uuid) throw (std::invalid_argument) {
            dtuio::uuid_t tmp;
            int retval = uuid_parse(uuid.c_str(), tmp);
            if (retval != 0){
                throw std::invalid_argument("Given UUID is invalid!");
            }
            
            uuid_copy(m_uuid, tmp);
        }

        bool uuid::operator<(const uuid & uuid2) const {
            return uuid_compare(get_uuid(), uuid2.get_uuid()) < 0;
        }

        bool uuid::operator==(const uuid & uuid2) const {
            return uuid_compare(get_uuid(), uuid2.get_uuid()) == 0;
        }
        
        uuid uuid::empty_uuid(){
            uuid_t tmp;
            uuid_clear(tmp);
            return uuid(tmp);
        }
        
        group_uuid::group_uuid(){ 
            uuid_clear(m_group_uuid);
        }

        group_uuid::group_uuid(const dtuio::uuid_t & g_uuid) {
            uuid_copy(m_group_uuid, g_uuid);
        }
        
        group_uuid::group_uuid(const group_uuid & original){
            uuid_copy(m_group_uuid, original.m_group_uuid);
        }

        group_uuid::group_uuid(const std::string& uuid) throw (std::invalid_argument) {
            dtuio::uuid_t tmp;
            int retval = uuid_parse(uuid.c_str(), tmp);
            if (retval != 0){
                throw std::invalid_argument("Given UUID is invalid!");
            }
            
            uuid_copy(m_group_uuid, tmp);
        }
        
        group_uuid::~group_uuid(){ ; }
        
        void group_uuid::set_group_uuid(const dtuio::uuid_t uuid){
            uuid_copy(m_group_uuid, uuid);
        }
        
        void group_uuid::set_group_uuid(const std::string& uuid) throw (std::invalid_argument) {
            dtuio::uuid_t tmp;
            int retval = uuid_parse(uuid.c_str(), tmp);
            if (retval != 0){
                throw std::invalid_argument("Given UUID is invalid!");
            }
            
            uuid_copy(m_group_uuid, tmp);
        }
        
        bool group_uuid::operator<(const group_uuid & uuid2) const {
            return uuid_compare(get_group_uuid(), uuid2.get_group_uuid()) < 0;
        }

        bool group_uuid::operator==(const group_uuid & uuid2) const {
            return uuid_compare(get_group_uuid(), uuid2.get_group_uuid()) == 0;
        }

        group_uuid group_uuid::empty_uuid(){
            uuid_t tmp;
            uuid_clear(tmp);
            return group_uuid(tmp);
        }
        
        neighbour_uuid::neighbour_uuid(){ 
            uuid_clear(m_neighbour_uuid);
        }

        neighbour_uuid::neighbour_uuid(const dtuio::uuid_t & g_uuid) {
            uuid_copy(m_neighbour_uuid, g_uuid);
        }
        
        neighbour_uuid::neighbour_uuid(const neighbour_uuid & original){
            uuid_copy(m_neighbour_uuid, original.m_neighbour_uuid);
        }

        neighbour_uuid::neighbour_uuid(const std::string& uuid) throw (std::invalid_argument) {
            dtuio::uuid_t tmp;
            int retval = uuid_parse(uuid.c_str(), tmp);
            if (retval != 0){
                throw std::invalid_argument("Given UUID is invalid!");
            }
            
            uuid_copy(m_neighbour_uuid, tmp);
        }
        
        neighbour_uuid::~neighbour_uuid(){ ; }
        
        void neighbour_uuid::set_neighbour_uuid(const dtuio::uuid_t uuid){
            uuid_copy(m_neighbour_uuid, uuid);
        }
        
        void neighbour_uuid::set_neighbour_uuid(const std::string& uuid) throw (std::invalid_argument) {
            dtuio::uuid_t tmp;
            int retval = uuid_parse(uuid.c_str(), tmp);
            if (retval != 0){
                throw std::invalid_argument("Given UUID is invalid!");
            }
            
            uuid_copy(m_neighbour_uuid, tmp);
        }
        
        bool neighbour_uuid::operator<(const neighbour_uuid & uuid2) const {
            return uuid_compare(get_neighbour_uuid(), uuid2.get_neighbour_uuid()) < 0;
        }
    
        bool neighbour_uuid::operator==(const neighbour_uuid & uuid2) const {
            return uuid_compare(get_neighbour_uuid(), uuid2.get_neighbour_uuid()) == 0;
        }

        neighbour_uuid neighbour_uuid::empty_uuid(){
            uuid_t tmp;
            uuid_clear(tmp);
            return neighbour_uuid(tmp);
        }
    }
}

// output operators
std::ostream & operator<<(std::ostream & output, const dtuio::helpers::uuid & nbr){
    output << "uuid=";
            
    char buffer[40];
    memset(buffer, 0, 40);
    uuid_unparse(nbr.get_uuid(), buffer);
    output << buffer;            

    return output;
}
std::ostream & operator<<(std::ostream & output, const dtuio::helpers::group_uuid & grp){
    output << "group_uuid=";
            
    char buffer[40];
    memset(buffer, 0, 40);
    uuid_unparse(grp.get_group_uuid(), buffer);
    output << buffer;            

    return output;
}
std::ostream & operator<<(std::ostream & output, const dtuio::helpers::neighbour_uuid & grp){
    output << "neighbour_uuid=";
            
    char buffer[40];
    memset(buffer, 0, 40);
    uuid_unparse(grp.get_neighbour_uuid(), buffer);
    output << buffer;            

    return output;
}

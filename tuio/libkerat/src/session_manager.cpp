/**
 * \file      session_manager.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-09-21 14:51 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/session_manager.hpp>
#include <set>
#include <algorithm>

namespace libkerat {
    namespace internals {

        session_id_t session_manager::get_next_session_id(){

            if (!ids_held.empty()){
                current_id = (ids_held.rbegin())->first;
            }
            ++current_id;

            if (current_id == SESSION_ID_UNUSED){ 
                ++current_id;
            }

            return current_id;
        }

        session_id_t session_manager::get_auto_session_id(){
            session_id_t id = get_next_session_id();
            register_session_id(id);
            return id;
        }

        void session_manager::reset(){
            current_id = SESSION_ID_UNUSED;
            ids_held.clear();
        }

        size_t session_manager::unregister_session_id(const session_id_t id){
            size_t count = --ids_held[id];

            if (count == 0){
                ids_held.erase(id);
            }

            return count;
        }

        size_t session_manager::register_session_id(const session_id_t id){ 
            return (++ids_held[id]);
        }

        session_manager::session_id_set session_manager::get_session_ids() const {

            session_id_set retval;

            for (id_stack::const_iterator i = ids_held.begin(); i != ids_held.end(); i++){
                retval.insert(i->first);
            }

            return retval;
        }

        void session_manager::unregister_session_id_force(const session_id_t id){
            ids_held.erase(id);
        }

        void session_manager::clear_session_registry(){
            ids_held.clear();
        }

    } // namespace internals 
} // namespace libkerat

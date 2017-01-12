/**
 * \file      session_manager.hpp
 * \brief     Provides session management functionality. Intended for use in servers and message agregators.
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-09-21 14:29 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_SESSION_MANAGER_HPP
#define KERAT_SESSION_MANAGER_HPP

#include <kerat/typedefs.hpp>
#include <map>
#include <set>

using libkerat::session_id_t;

namespace libkerat {
    namespace internals {

        /**
         * \brief Provides basic session id management operations
         * 
         * Intended for use in servers and message agregators
         */
        class session_manager {
        public:
            typedef std::set<session_id_t> session_id_set;

            /**
             * \brief Get the session id and register it.
             * Calls to \ref get_next_session_id and \ref register_session_id subsequently.
             * The obtained ID should be eventualy unregistered by calling
             * \ref unregister_session_id
             * \return same as \ref get_next_session_id
             */
            session_id_t get_auto_session_id();

            /**
             * \brief Gets the first unused session id
             * \note No ID registration is performed.
             * \return next free session id
             */
            session_id_t get_next_session_id();

            /**
             * \brief Gets the last session id returned by \ref get_next_session_id.
             * \note No id registration is performed.
             * \return last returned id or \ref SESSION_ID_UNUSED if no call to
             * \ref get_next_session_id has been performed yet
             */
            inline session_id_t get_current_session_id() const { return current_id; }

            /**
             * \brief Increase instance count to the session id.
             * Each call to \ref register_session_id should have the corresponding
             * \ref unregister_session_id
             * \param id - id to manage
             * \return count of instances of this session id that this manager is avare of
             */
            size_t register_session_id(const session_id_t id);

            /**
            * \brief Decrease instance count to the session id
            * If the instance count stands 0 after decrease, the corresponding entry is deleted
            * \param id - id to manage
            * \return count of instances of this session id that this manager is avare of
            */
            size_t unregister_session_id(const session_id_t id);

            /**
             * \brief Ignore the session_id instances count and delete it
             * If you realy need to use this function you should rethink your coding practices.
             * \param id - id to delete
             */
            void unregister_session_id_force(const session_id_t id);

            /**
             * \brief Check whether given session id is managed by this session manager
             * \param id - id to search for
             * \return true if managed, false if not
             */
            inline bool session_holds_id(const session_id_t id) const { return (ids_held.find(id) != ids_held.end()); }

            /**
             * \brief Get set of all managed id's
             * \return set of all ids managed by this session_manager
             */
            session_id_set get_session_ids() const ;

            //! \brief Cleans the internal session_id registry.
            void clear_session_registry();

            virtual ~session_manager(){ ; }

            //! \brief Libkerat uses session id 0 as both empty, do not use it
            static const session_id_t SESSION_ID_UNUSED = 0;
            
        protected:
            session_manager():current_id(SESSION_ID_UNUSED){ ; }

            //! \brief resets both id counter and managed ids stack
            void reset();

        private:

            typedef std::map<session_id_t, size_t> id_stack;

            //! \brief Holds the current session id
            session_id_t current_id;
            
            //! \brief Holds all ession id's registered and their counts
            id_stack ids_held;

        }; // cls session_manager

    } // ns internals
} // ns libkerat

#endif // KERAT_SESSION_MANAGER_HPP

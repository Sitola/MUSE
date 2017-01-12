/**
 * \file      frame_manager.hpp
 * \brief     Provides frame id management functionality. Intended for use in servers and message agregators.
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-10-19 02:12 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_FRAME_MANAGER_HPP
#define KERAT_FRAME_MANAGER_HPP

#include <kerat/typedefs.hpp>

namespace libkerat {
    namespace internals {
    
        /**
        * \brief Provides basic frame id management operations
        */    
        class frame_manager {
        public:

            static const frame_id_t OUT_OF_ORDER_ID = 0;

            //! \brief Create a new frame manager
            frame_manager();

            /**
             * \brief Gets the next frame id
             * \return next frame id
             */
            frame_id_t get_next_frame_id();

            /**
             * \brief Gets the last frame id returned by \ref get_next_frame_id
             * \return last returned id or \ref OUT_OF_ORDER_ID if no call to
             * \ref get_next_frame_id has been performed yet
             */
            frame_id_t get_current_frame_id() const ;

        private:

            frame_id_t frame_id;

        };

    } // ns internals
} // ns libkerat

#endif // KERAT_FRAME_MANAGER_HPP

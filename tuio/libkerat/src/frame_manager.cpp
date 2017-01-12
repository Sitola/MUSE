/**
 * \file      frame_manager.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-10-19 02:12 UTC+2
 * \copyright BSD
 */

#include <kerat/frame_manager.hpp>

namespace libkerat {
    namespace internals {
        
        frame_manager::frame_manager():frame_id(OUT_OF_ORDER_ID){ ; }

        frame_id_t frame_manager::get_next_frame_id(){ return ++frame_id; }

        frame_id_t frame_manager::get_current_frame_id() const { return frame_id; }

    } // ns internals
} // ns libkerat


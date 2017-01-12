/**
 * \file      tuio_messages.hpp
 * \brief     Includes all tuio message types defined in libkerat.
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-09-26 18:21 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGES_HPP
#define KERAT_TUIO_MESSAGES_HPP

#include <kerat/tuio_message_frame.hpp>
#include <kerat/tuio_message_alive.hpp>

#include <kerat/tuio_message_pointer.hpp>
#include <kerat/tuio_message_token.hpp>
#include <kerat/tuio_message_bounds.hpp>
#include <kerat/tuio_message_symbol.hpp>

#include <kerat/tuio_message_control.hpp>
#include <kerat/tuio_message_data.hpp>
#include <kerat/tuio_message_signal.hpp>

#include <kerat/tuio_message_convex_hull_geometry.hpp>
#include <kerat/tuio_message_outer_contour_geometry.hpp>
#include <kerat/tuio_message_inner_contour_geometry.hpp>
#include <kerat/tuio_message_skeleton_geometry.hpp>
#include <kerat/tuio_message_skeleton_volume_geometry.hpp>
#include <kerat/tuio_message_area_geometry.hpp>
#include <kerat/tuio_message_raw.hpp>

#include <kerat/tuio_message_alive_associations.hpp>
#include <kerat/tuio_message_container_association.hpp>
#include <kerat/tuio_message_link_association.hpp>
#include <kerat/tuio_message_linked_list_association.hpp>
#include <kerat/tuio_message_linked_tree_association.hpp>

// support for pass-through of unknown OSC messages
#include <kerat/generic_osc_message.hpp>

#endif // KERAT_TUIO_MESSAGES_HPP

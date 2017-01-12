
/**
 * Defines the point_renderer tuio listener
 *
 * \file      point_renderer.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-12-06 11:20 UTC+1
 * \copyright BSD
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include <map>
#include <iostream>
#include <list>
#include <algorithm>
#include <cassert>
#include <kerat/kerat.hpp>
#include <cmath>

#include "point_renderer.hpp"
#include "config.h"

const int MARKER_SIZE = 31;
const int VELOCITY_ARROW_MAX_SIZE = 60;
const float VELOCITY_ARROW_SCALE = 6.0;

/**
 * Memset-like function that uses uint32_t as pattern
 */
inline void memset_uint32_pattern(uint32_t * data, uint32_t pattern, unsigned int count){ for (unsigned int i = 0; i < count; i++){ data[i] = pattern; } }

libkerat::listeners::stdout_listener stdo(std::cout);

point_renderer::point_renderer(uint16_t width, uint16_t height, uint8_t line_width, bool draw_track, bool top_to_bottom)
    :m_background(make_rgba_color(0x80, 0x80, 0x80, 0x00)),
    m_width(width), m_height(height), m_draw_track(draw_track), m_top_to_bottom(top_to_bottom), m_line_width(line_width),
    m_previous_segment(0), m_canvas(NULL)
{
    int retval = posix_memalign((void **)&m_canvas, sizeof(void *), m_width * m_height * sizeof(rgba_pixel_t));
    if (retval != 0){
        std::cerr << __FILE__ << ":" << __LINE__ << " " << strerror(retval) << std::endl;
        return;
    }
    memset_uint32_pattern(m_canvas, 0, m_width * m_height);
}

point_renderer::~point_renderer(){
    if (m_canvas != NULL){
        free(m_canvas);
        m_canvas = NULL;
    }
}

// =============================================== TUIO listener

void point_renderer::notify_client_bind(libkerat::client * notifier){
    std::cout << "point_renderer[" << (void *)this << "] is now connected to client [" << (void *)notifier << "]" << std::endl;
}

void point_renderer::notify_client_release(libkerat::client * notifier){
    std::cout << "point_renderer[" << (void *)this << "] is no longer connected to client [" << (void *)notifier << "]" << std::endl;
}

void point_renderer::notify(const libkerat::client* notifier){
    typedef libkerat::bundle_handle::const_iterator iterator;

    libkerat::bundle_stack stack = notifier->get_stack();

    while (stack.get_length() > 0){
        libkerat::bundle_handle f = stack.get_update();

        for (iterator i = f.begin(); i != f.end(); i++){

            const libkerat::helpers::contact_session * hp_session = dynamic_cast<const libkerat::helpers::contact_session *>(*i);

            if (hp_session != NULL){
                // make sure that every session id is accounted for
                m_alives.insert(hp_session->get_session_id());

                const libkerat::helpers::contact_component * hp_component = dynamic_cast<const libkerat::helpers::contact_component *>(*i);
                if (hp_component != NULL){
                    sid_cid_pair key;
                    key.sid = hp_session->get_session_id();
                    key.cid = hp_component->get_component_id();

                    m_components.insert(key);

                    // test for pointer
                    const libkerat::message::pointer * is_pointer = dynamic_cast<const libkerat::message::pointer *>(*i);
                    if (is_pointer != NULL){ process_pointer(*is_pointer); }
                }

                // test for bound
                const libkerat::message::bounds * is_bound = dynamic_cast<const libkerat::message::bounds *>(*i);
                if (is_bound != NULL) { process_bounds(*is_bound); }

                // test for coa
                const libkerat::message::container_association * is_coa = dynamic_cast<const libkerat::message::container_association *>(*i);
                if (is_coa != NULL) { process_container_assoc(*is_coa); }

            } else {
                // does not have session id, so can be one of these

                const libkerat::message::alive_associations * is_ala = dynamic_cast<const libkerat::message::alive_associations *>(*i);
                const libkerat::message::alive * is_alive = dynamic_cast<const libkerat::message::alive *>(*i);

                if (is_ala != NULL) {
                    process_alive_associations(*is_ala);
                } else if (is_alive != NULL){
                    process_alive(*is_alive);
                }
            }
        }
    }
}

// =========================================================== HIGH-LEVEL

void point_renderer::render(){

    if (m_canvas == NULL){ return; }
    clear();
    
    typedef track_map::iterator ptrit;
    for (ptrit i = m_pointers.begin(); i != m_pointers.end(); i++){
        typedef track_entry::point_list::iterator titer;
        titer current = i->second.points.begin();
        titer previous = i->second.points.begin();

        if (current == i->second.points.end()){ continue; }

        libkerat::component_id_t cid = current->get_component_id();
        libkerat::session_id_t sid = current->get_session_id();

        while (current != i->second.points.end()){
            current->set_component_id(cid);
            current->set_session_id(sid);
            draw_pointer(*current);

            if (current != previous){
                sid_set path;
                rgba_color_t color = get_color_for_sid(i->first.sid, path);
                int xi = current->get_x();
                int xo = previous->get_x();
                int yo = current->get_y();
                int yi = previous->get_y();

                if (cut_to_frame(xo, yo, xi, yi)){
                    draw_line(color, xo, yo, xi, yi);
                }
            }

            previous = current;
            ++current;
        }
        
    }

    typedef bounds_map::const_iterator bndit;
    for (bndit i = m_bounds.begin(); i != m_bounds.end(); i++){
        draw_bound(i->second);
    }

}

void point_renderer::clear(){
    memset_uint32_pattern(m_canvas, m_background, m_width * m_height);
}

void point_renderer::draw_pointer(const libkerat::message::pointer & pointer){
    const int LINE_SIZE = MARKER_SIZE/2 + (m_line_width/2);

    sid_set path;
    rgba_color_t color = get_color_for_sid(pointer.get_session_id(), path);

    libkerat::coord_t tmp_x = pointer.get_x();
    libkerat::coord_t tmp_y = pointer.get_y();

    // horizontal
    for (uint8_t i = 0; i < m_line_width; i++){
        int x_orig = tmp_x;
        int y = tmp_y - (m_line_width/2) + i;

        // left
        int xi = x_orig-1;
        int xo = x_orig-LINE_SIZE;
        int yo = y;
        int yi = y;

        if (cut_to_frame(xo, yo, xi, yi)){
            draw_line(color, xo, yo, xi, yi);
        }

        // right
        xi = x_orig+1;
        xo = x_orig+LINE_SIZE;
        yo = y;
        yi = y;

        if (cut_to_frame(xo, yo, xi, yi)){
            draw_line(color, xo, yo, xi, yi);
        }
    }

    // vertical
    for (uint8_t i = 0; i < m_line_width; i++){
        int y_orig = tmp_y;
        int x = tmp_x - (m_line_width/2) + i;

        // top
        int yi = y_orig-1;
        int yo = y_orig-LINE_SIZE;
        int xo = x;
        int xi = x;

        if (cut_to_frame(xo, yo, xi, yi)){
            draw_line(color, xo, yo, xi, yi);
        }

        // bottom
        yi = y_orig+1;
        yo = y_orig+LINE_SIZE;
        xo = x;
        xi = x;

        if (cut_to_frame(xo, yo, xi, yi)){
            draw_line(color, xo, yo, xi, yi);
        }
    }

    int vel_x = 0;
    int vel_y = 0;

    get_velocity_arrow_length(pointer.get_x_velocity(), pointer.get_y_velocity(), vel_x, vel_y);

    // velocity arrow
    if ((vel_x | vel_y) != 0){
        int yi = tmp_y;
        int yo = tmp_y+vel_y;
        int xi = tmp_x;
        int xo = tmp_x+vel_x;

        if (cut_to_frame(xo, yo, xi, yi)){
            draw_line(color, xo, yo, xi, yi);
        }
    }

}

void point_renderer::draw_bound(const libkerat::message::bounds & bound){
    sid_set path;
    rgba_color_t color = get_color_for_sid(bound.get_session_id(), path);

    libkerat::helpers::point_2d center = bound;

    libkerat::distance_t width = bound.get_width();
    libkerat::distance_t height = bound.get_height();
    libkerat::angle_t angle = bound.get_angle();

    libkerat::helpers::point_2d corner_1(-width/2, height/2);
    libkerat::helpers::point_2d corner_2(width/2, height/2);
    libkerat::helpers::point_2d corner_3(width/2, -height/2);
    libkerat::helpers::point_2d corner_4(-width/2, -height/2);

    corner_1 = rotate_point(corner_1, angle) + center;
    corner_2 = rotate_point(corner_2, angle) + center;
    corner_3 = rotate_point(corner_3, angle) + center;
    corner_4 = rotate_point(corner_4, angle) + center;

/*  //Draw rectangle around the blob
    draw_line(color, corner_1.get_x(), corner_1.get_y(), corner_2.get_x(), corner_2.get_y());
    draw_line(color, corner_2.get_x(), corner_2.get_y(), corner_3.get_x(), corner_3.get_y());
    draw_line(color, corner_3.get_x(), corner_3.get_y(), corner_4.get_x(), corner_4.get_y());
    draw_line(color, corner_4.get_x(), corner_4.get_y(), corner_1.get_x(), corner_1.get_y());
*/
    const double a = width/2;
    const double b = height/2;
    const double a_squared = a*a;
    const double b_squared = b*b;

    for (libkerat::coord_t tmp_x = -a; tmp_x <= 0; tmp_x++){
        double x_squared = tmp_x; x_squared *= x_squared;
        double tmp_core = b_squared - (x_squared * b_squared / a_squared);
        if (tmp_core < 0 ) { continue; }
        double tmp_y = sqrt(tmp_core);

        draw_multipoint(color, rotate_point(libkerat::helpers::point_2d(tmp_x, tmp_y), angle) + center);
        draw_multipoint(color, rotate_point(libkerat::helpers::point_2d(tmp_x, -tmp_y), angle) + center);
        draw_multipoint(color, rotate_point(libkerat::helpers::point_2d(-tmp_x,tmp_y), angle) + center);
        draw_multipoint(color, rotate_point(libkerat::helpers::point_2d(-tmp_x,-tmp_y), angle) + center);
/*
        draw_point(color, rotate_point(libkerat::helpers::point_2d(tmp_x, -2*b + tmp_y), angle) + center);
        draw_point(color, rotate_point(libkerat::helpers::point_2d(tmp_x, 2*b -tmp_y), angle) + center);
        draw_point(color, rotate_point(libkerat::helpers::point_2d(-tmp_x, -2*b +tmp_y), angle) + center);
        draw_point(color, rotate_point(libkerat::helpers::point_2d(-tmp_x, 2*b -tmp_y), angle) + center);
 */
    }


}

// ============================================================== AUXILIARY

libkerat::helpers::point_2d point_renderer::rotate_point(const libkerat::helpers::point_2d& point, libkerat::angle_t angle){
    double sin_alpha = std::sin(angle);
    double cos_alpha = std::cos(angle);
    return libkerat::helpers::point_2d((cos_alpha*point.get_x()) - (sin_alpha*point.get_y()), (cos_alpha*point.get_y()) + (sin_alpha*point.get_x()));
}

point_renderer::rgba_color_t point_renderer::get_color_for_sid(libkerat::session_id_t id, point_renderer::sid_set & path){

    rgba_color_t retval = 0;

    // this has to be done first, since m_colors is used to determine whether object is alive or not
    sid_colormap::const_iterator c = m_colors.find(id);
    if (c != m_colors.end()){
        retval = c->second;
    } else {
        retval = get_random_color();
        m_colors[id] = retval;
    }

    if (path.find(id) == path.end()){
        ca_map::const_iterator container = m_containter_associations.find(id);
        if (container != m_containter_associations.end()){
            path.insert(id);
            retval = get_color_for_sid(container->second, path);
        }
    }

    return retval;
}

void point_renderer::get_velocity_arrow_length(const libkerat::velocity_t x_vel, const libkerat::velocity_t y_vel, int & delta_x, int & delta_y){
    float arrow_original_size = (x_vel * x_vel) + (y_vel * y_vel);

    if (arrow_original_size <= 1){
        delta_x = 0;
        delta_y = 0;
        return;
    }

    float arrow = (VELOCITY_ARROW_SCALE/2.0)*log(arrow_original_size);
    if (arrow > VELOCITY_ARROW_MAX_SIZE){ arrow = VELOCITY_ARROW_MAX_SIZE; }

    float tg_alpha = y_vel / x_vel;
    float alpha = atan(tg_alpha);
    delta_y = sin(alpha)*arrow;
    delta_x = cos(alpha)*arrow;
    if (x_vel < 0){ 
        delta_x = -delta_x; 
        delta_y = -delta_y;
    }
}


// ============================================================== LOW-LEVEL RGBA
point_renderer::rgba_color_t point_renderer::make_rgba_color(const rgba_color_component_t r, const rgba_color_component_t g, const rgba_color_component_t b, const rgba_color_component_t a){

    rgba_color_t xr = r;
    rgba_color_t xg = g;
    rgba_color_t xb = b;
    rgba_color_t xa = a;

    return htonl((xr << 24) | (xg << 16) | (xb << 8) | xa);
}

void point_renderer::parse_rgba_color(const rgba_color_t rgba, rgba_color_component_t& r, rgba_color_component_t& g, rgba_color_component_t& b, rgba_color_component_t& a){

    rgba_color_t xr = (rgba >> 24) & 0xff;
    rgba_color_t xg = (rgba >> 16) & 0xff;
    rgba_color_t xb = (rgba >>  8) & 0xff;
    rgba_color_t xa = (rgba      ) & 0xff;

    r = xr;
    g = xg;
    b = xb;
    a = xa;

}

point_renderer::rgba_color_t point_renderer::get_random_color(){

    rgba_color_t color = 0;
    int segment = m_previous_segment;
    int part = 0;

#ifndef DETERMINISTIC_COLORS
    while (segment == m_previous_segment){
        segment = rand() % 6;
    }
    part = (rand() % 16)*8;
#else
    part = (segment >> 3);
    segment = (segment+5)%6;
    part = (part + 31)%256;
#endif

    switch (segment){
        case 0: {
            color = make_rgba_color(0xff, part, 0);
            break;
        }
        case 1: {
            color = make_rgba_color(0xff - part, 0xff, 0);
            break;
        }
        case 2: {
            color = make_rgba_color(0, 0xff, part);
            break;
        }
        case 3: {
            color = make_rgba_color(0, 0xff - part, 0xff);
            break;
        }
        case 4: {
            color = make_rgba_color(part, 0, 0xff);
            break;
        }
        case 5: {
            color = make_rgba_color(0xff, 0, 0xff - part);
            break;
        }
    }

#ifdef DETERMINISTIC_COLORS
    segment |= (part << 3);
#endif

    m_previous_segment = segment;

    return color;
}

// ============================================================== LOW-LEVEL DRAW

void point_renderer::draw_line(const rgba_color_t color, int x1, int y1, int x2, int y2){

    int dist_x = x2 - x1;
    int dist_y = y2 - y1;

    int steps = std::max(abs(dist_x), abs(dist_y));

    ++steps;

    double delta_x = dist_x;
    delta_x /= steps;
    double delta_y = dist_y;
    delta_y /= steps;

    double current_x = x1;
    double current_y = y1;

    for (int i = 0; i < steps; i++){

        if ((current_x >= 0) && (current_y >= 0)){
            int x = current_x;
            int y = current_y;

            if ((x < m_width) && (y < m_height)){
                if (m_top_to_bottom){
                    m_canvas[(y * m_width) + x] = color;
                } else {
                    m_canvas[(((m_height - 1) - y) * m_width) + x] = color;
                }
            }
        }

        current_x += delta_x;
        current_y += delta_y;
    }

}

inline void point_renderer::draw_point(const rgba_color_t color, const libkerat::helpers::point_2d & pt){
    int32_t ty = pt.get_y();
    int32_t tx = pt.get_x();
    if ((ty < 0) || (tx < 0)){ return; }
    if ((ty >= m_height) || (tx >= m_width)){ return; }
    if (m_top_to_bottom){
        m_canvas[(ty * m_width) + tx] = color;
    } else {
        m_canvas[(((m_height - 1) - ty) * m_width) + tx] = color;
    }
}

void point_renderer::draw_multipoint(const rgba_color_t color, const libkerat::helpers::point_2d & pt){
    int32_t ty = pt.get_y();
    int32_t tx = pt.get_x();

    for (int32_t y = 0; y < m_line_width; y++){
        int32_t x = sqrt((m_line_width*m_line_width) - (y*y));
        int x1 = tx-x;
        int x2 = tx+x;
        int y1 = ty-y;
        int y2 = y1;

        if (cut_to_frame(x1, y1, x2, y2)){
            draw_line(color, x1, y1, x2, y2);
        }
    }
}

void point_renderer::process_alive(const libkerat::message::alive& alive){
    alive_ids alives = alive.get_alives();
    alive_ids sids_to_remove = libkerat::extract_removed_ids(m_alives, alive.get_alives());

    // delete removed session ids
    for (alive_ids::const_iterator i = sids_to_remove.begin(); i != sids_to_remove.end(); i++){
        m_colors.erase(*i);
        m_containter_associations.erase(*i);
        m_bounds.erase(*i);

        // remove corresponding indirect container associations
        alive_ids aremove;
        for (ca_map::const_iterator a = m_containter_associations.begin(); a != m_containter_associations.end(); a++){
            if (a->second == *i){ aremove.insert(a->first); }
        }
        for (alive_ids::const_iterator j = aremove.begin(); j != aremove.end(); j++){
            m_containter_associations.erase(*j);
        }
    }

    // buffered objects cleanup
    sid_cid_set components_to_remove;
    for (sid_cid_set::const_iterator i = m_components.begin(); i != m_components.end(); i++){
        alive_ids::const_iterator shall_be_removed = std::find(sids_to_remove.begin(), sids_to_remove.end(), i->sid);
        if (shall_be_removed != sids_to_remove.end()) {
            components_to_remove.insert(*i);
        }
    }

    for (sid_cid_set::const_iterator i = components_to_remove.begin(); i != components_to_remove.end(); i++){
        m_pointers.erase(*i); // pointers cleanup
        m_components.erase(*i); // components cleanup
    }

//    assert(pointers_original_size - components_to_remove.size() ==  m_pointers.size());
}

void point_renderer::process_pointer(const libkerat::message::pointer& pointer){
    sid_cid_pair key;
    key.sid = pointer.get_session_id();
    key.cid = pointer.get_component_id();

    track_map::iterator i = m_pointers.find(key);
    lo_timetag_now(&m_pointers[key].stamp);
    if (!m_draw_track){ m_pointers[key].points.clear(); }
    m_pointers[key].points.push_front(pointer);
    if (i == m_pointers.end()){
        lo_timetag_now(&m_pointers[key].stamp);
        sid_set path;
        get_color_for_sid(pointer.get_session_id(), path);
    }

}

void point_renderer::process_bounds(const libkerat::message::bounds& bound){

    libkerat::session_id_t sid = bound.get_session_id();

    bounds_map::iterator i = m_bounds.find(sid);
    if (i == m_bounds.end()){
        m_bounds[sid] = bound;
        sid_set path;
        get_color_for_sid(sid, path);
    } else {
        m_bounds[sid] = bound;
    }
}

void point_renderer::process_container_assoc(const libkerat::message::container_association& association){
    const libkerat::message::alive_associations::associated_ids associated = association.get_associations();
    for (libkerat::message::alive_associations::associated_ids::const_iterator i = associated.begin(); i != associated.end(); i++){
        m_containter_associations[*i] = association.get_session_id();
    }
}

void point_renderer::process_alive_associations(const libkerat::message::alive_associations& associations){
    const libkerat::message::alive_associations::associated_ids associated = associations.get_associations();
    libkerat::message::alive_associations::associated_ids to_remove;

    for (ca_map::const_iterator i = m_containter_associations.begin(); i != m_containter_associations.end(); i++){
        if (associated.find(i->first) == associated.end()){ to_remove.insert(i->first); }
    }

    for (libkerat::message::alive_associations::associated_ids::const_iterator i = to_remove.begin(); i != to_remove.end(); i++){
        m_containter_associations.erase(*i);
    }
}

bool point_renderer::cut_to_frame(int& x1, int& y1, int& x2, int& y2){
    if ((x1 == x2) && (y1 == y2)){ return true; }

    // change points to fit x1 <= x2
    if (x1 > x2){
        int tmp = y1;
        y1 = y2;
        y2 = tmp;
        tmp = x1;
        x1 = x2;
        x2 = tmp;
    }

    // check whether this line fits at all
    if (x2 < 0){ return false; }
    if (x1 >= m_width){ return false; }

    // get direction vector
    int dx = x2 - x1;
    int dy = y2 - y1;

    { // first, take care of [x2, y2]
        // cut x2 to fit m_width
        if (x2 >= m_width) {
            assert(dx != 0);
            double outer_t = m_width - 1 - x2;  outer_t /= dx;
            x2 = m_width-1;
            y2 = dy*outer_t + y2;
        }
        // then, cut y2 to fit 0-m_height
        if (dy != 0){
            if (y2 >= m_height) {
                double outer_t = m_height - 1 - y2;  outer_t /= dy;
                y2 = m_height-1;
                x2 = dx*outer_t + x2;
            } else if (y2 < 0) {
                double outer_t = 0 - y2;  outer_t /= dy;
                y2 = 0;
                x2 = dx*outer_t + x2;
           }
        }
    }

    { // then, take care of [x1, y1]
        // cut x2 to fit m_width
        if (x1 < 0) {
            assert(dx != 0);
            double outer_t = 0 - x1;  outer_t /= dx;
            x1 = 0;
            y1 = dy*outer_t + y1;
        }
        // then, cut y1 to fit 0-m_height
        if (dy != 0) {
            if (y1 >= m_height) {
                double outer_t = m_height - 1 - y1;  outer_t /= dy;
                y1 = m_height-1;
                x1 = dx*outer_t + x1;
            } else if (y1 < 0) {
                double outer_t = 0 - y1;  outer_t /= dy;
                y1 = 0;
                x1 = dx*outer_t + x1;
            }
        }
    }

    // check whether the modified line fits at all
    // y are guaranteed to be within
    if (x2 < 0){ return false; }
    if (x1 >= m_width){ return false; }

    return true;

}

/**
 * Defines the point_renderer tuio listener
 *
 * \file      point_renderer.hpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-12-06 11:20 UTC+1
 * \copyright BSD
 */

#ifndef POINT_RENDERER_H
#define POINT_RENDERER_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <unistd.h>
#include <inttypes.h>
#include <cmath>
#include <map>
#include <list>
#include <kerat/kerat.hpp>

// defines entry in point_renderer's pointmap
class point_renderer: public libkerat::listener {
public:
    typedef uint32_t rgba_color_t;
    typedef uint8_t rgba_color_component_t;
    typedef rgba_color_t rgba_pixel_t;

    point_renderer(uint16_t width, uint16_t height, uint8_t line_width, bool draw_track = false, bool top_to_bottom = true);

    ~point_renderer();

    void notify(const libkerat::client * notifier);
    void notify_client_bind(libkerat::client * notifier);
    void notify_client_release(libkerat::client * notifier);

    void render();
    void clear();

    inline rgba_pixel_t * get_canvas() const { return m_canvas; }

    struct sid_cid_pair {
        libkerat::session_id_t   sid;
        libkerat::component_id_t cid;

        bool operator<(const sid_cid_pair & second) const { return (sid < second.sid) || ((sid == second.sid) && (cid < second.cid)); }
    };

private:
    typedef std::map<libkerat::session_id_t, rgba_color_t> sid_colormap;
    typedef std::map<sid_cid_pair, libkerat::message::pointer> pointers_map;
    typedef std::map<libkerat::session_id_t, libkerat::message::bounds> bounds_map;
    typedef std::map<libkerat::session_id_t, libkerat::session_id_t> ca_map;
    typedef std::set<sid_cid_pair> sid_cid_set;
    typedef std::set<libkerat::session_id_t> sid_set;
    typedef libkerat::message::alive::alive_ids alive_ids;

    struct track_entry {
        typedef std::list<libkerat::message::pointer> point_list;

        libkerat::timetag_t stamp;
        point_list points;
    };

    typedef std::map<sid_cid_pair, track_entry> track_map;
    
    bool cut_to_frame(int & x1, int & y1, int & x2, int & y2);

    rgba_color_t get_color_for_sid(const libkerat::session_id_t id, sid_set & searched_sids);
    void get_velocity_arrow_length(const libkerat::velocity_t x_vel, const libkerat::velocity_t y_vel, int & delta_x, int & delta_y);

    // drawing
    void draw_pointer(const libkerat::message::pointer & pointer);
    void draw_bound(const libkerat::message::bounds & bound);

    // low-level drawing
    void draw_line(const rgba_color_t color, int x1, int y1, int x2, int y2);
    void draw_multipoint(const rgba_color_t color,  const libkerat::helpers::point_2d & pt);
    inline void draw_point(const rgba_color_t color, const libkerat::helpers::point_2d & pt);

    static libkerat::helpers::point_2d rotate_point(const libkerat::helpers::point_2d & point, libkerat::angle_t angle);

    // color manipulators
    rgba_color_t get_random_color();
    rgba_color_t make_rgba_color(const rgba_color_component_t r, const rgba_color_component_t g, const rgba_color_component_t b, const rgba_color_component_t a = 0xff);
    void parse_rgba_color(const rgba_color_t rgba, rgba_color_component_t & r, rgba_color_component_t & g, rgba_color_component_t & b, rgba_color_component_t & a);
    inline void parse_rgba_color(const rgba_color_t rgba, rgba_color_component_t & r, rgba_color_component_t & g, rgba_color_component_t & b){ rgba_color_component_t a; parse_rgba_color(rgba, r, g, b, a); }

    void process_alive(const libkerat::message::alive & alive);
    void process_pointer(const libkerat::message::pointer & pointer);
    void process_bounds(const libkerat::message::bounds & bound);
    void process_container_assoc(const libkerat::message::container_association & association);
    void process_alive_associations(const libkerat::message::alive_associations & associations);

    rgba_color_t m_background;
    int m_width;
    int m_height;
    bool m_draw_track;
    bool m_top_to_bottom;
    uint8_t m_line_width;
    int m_previous_segment;

    rgba_pixel_t * m_canvas;

    track_map m_pointers;
    bounds_map m_bounds;
    ca_map m_containter_associations;

    sid_colormap m_colors;
    alive_ids m_alives;
    sid_cid_set m_components;

};

#endif // POINT_RENDERER_H

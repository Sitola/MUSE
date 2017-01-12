/**
 * \file      dollar_n.cpp
 * \brief     Implements functionality for the dollar N ($N) (multistroke) recognizer
 * \author    Martin Luptak <374178@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-11-19 14:37 UTC+1
 * \copyright BSD
 */

#include <muse/recognizers/dollar_n.hpp>
#include <muse/recognizers/recognizers_utils.hpp>

#include <math.h>
#include <float.h>

#ifdef TEST_PERFORMANCE
    #include <muse/recognizers/io_utils.hpp>
    #include <lo/lo_lowlevel.h>
#endif

namespace libreco {
    namespace recognizers {

        using std::pow;
        using libreco::rutils::multistroke_gesture;
        using libreco::rauxiliary::degrees_to_radians;

        dollar_n::dollar_n(const libreco_generic_multistroke_map & tmpls, uint16_t num_of_pts, const point_2d orig, float rot_down,
                float rot_up, float rot_thresh, uint16_t scale_box, float scale_thresh, uint16_t start_index, float start_thresh, bool eq_strokes)
        : number_of_points(num_of_pts), origin(orig), rotation_down_limit(degrees_to_radians(rot_down)),
        rotation_up_limit(degrees_to_radians(rot_up)), rotation_threshold(degrees_to_radians(rot_thresh)),
        scale_box_size(scale_box), scaling_threshold(scale_thresh), start_vector_index(start_index),
        start_vector_threshold(degrees_to_radians(start_thresh)), equal_strokes_numbers(eq_strokes) {

            //prevent reallocation
            dollar_n_templates.reserve(tmpls.size());
            generate_unistroke_permutations(tmpls);
        }

        dollar_n::dollar_n(const libreco_generic_multistroke_map & tmpls)
        : number_of_points(96), origin(point_2d(0, 0)), rotation_down_limit(degrees_to_radians(-45.0)), rotation_up_limit(degrees_to_radians(45.0)),
        rotation_threshold(degrees_to_radians(2.0)), scale_box_size(250), scaling_threshold(0.3), start_vector_index(12),
        start_vector_threshold(degrees_to_radians(30.0)), equal_strokes_numbers(true) {
            //prevent reallocation
            dollar_n_templates.reserve(tmpls.size());
            generate_unistroke_permutations(tmpls);
        }

        void dollar_n::generate_unistroke_permutations(const libreco_generic_multistroke_map & tmpls) {

            for (libreco_generic_multistroke_map::const_iterator map_iter = tmpls.begin(); map_iter != tmpls.end(); map_iter++) {
                //insert new empty multistroke_gesture to dollar_n_templates vector
                dollar_n_templates.push_back(multistroke_gesture());
                multistroke_gesture & multi_pattern = dollar_n_templates.back();

                multi_pattern.name = map_iter->first.gesture_name;
                multi_pattern.number_of_strokes = map_iter->second.size();
                multi_pattern.sensitive = map_iter->first.orientation_sensitive;

                //1st create initial order of strokes
                vector<uint16_t> order;
                //prevent reallocation
                order.reserve(map_iter->second.size());
                for (uint16_t i = 0; i < map_iter->second.size(); i++) {
                    order.push_back(i);
                }

                //2nd create all possible orders of strokes
                vector<vector<uint16_t> > orders;
                heap_permute(order.size(), order, orders);

                //first prevent reallocation and reserve enough space for unistrokes
                uint32_t space_needed = (uint32_t) orders.size() * (uint32_t) pow(2, orders.front().size());
                multi_pattern.unistrokes.reserve(space_needed);
                make_unistrokes(map_iter->second, orders, multi_pattern.unistrokes);

                for (vector<std::pair<point_2d, vector<point_2d> > >::iterator strokes_iter = multi_pattern.unistrokes.begin();
                        strokes_iter != multi_pattern.unistrokes.end(); strokes_iter++) {

                    libreco::rauxiliary::resample(strokes_iter->second, number_of_points);

                    float angle = libreco::rauxiliary::indicative_angle(strokes_iter->second);
                    libreco::rauxiliary::rotate_by_angle(strokes_iter->second, -angle);
                    dimensional_scale(strokes_iter->second);
                    if (map_iter->first.orientation_sensitive) {
                        libreco::rauxiliary::rotate_by_angle(strokes_iter->second, angle);
                    }
                    libreco::rauxiliary::translate_to(strokes_iter->second, origin);
                    strokes_iter->first = start_unit_vector(strokes_iter->second);

                }
            }
        }

        //creates all possible orders of strokes (simple permutation)
        void dollar_n::heap_permute(uint16_t n, std::vector<uint16_t> & order, std::vector<std::vector<uint16_t> > & orders) {
            if (n == 1) {
                orders.push_back(order);
                return;
            }
            for (int i = 0; i < n; i++) {
                heap_permute(n - 1, order, orders);
                if ((n % 2) == 0) {
                    uint16_t tmp = order[i];
                    order[i] = order[n - 1];
                    order[n - 1] = tmp;
                } else {
                    uint16_t tmp = order[0];
                    order[0] = order[n - 1];
                    order[n - 1] = tmp;
                }
            }
        }

        //creates all possible unistrokes from given multistroke gesture (all possible orders and directions of strokes) 
        void dollar_n::make_unistrokes(const vector<vector<point_2d> > & multistroke,
                const vector<vector<uint16_t> > & orders,
                vector<std::pair<point_2d, vector<point_2d> > > & out_unistrokes) {
            out_unistrokes.clear();
            vector<point_2d> unistroke;
            for (unsigned int p = 0; p < orders.size(); p++) {
                for (unsigned int b = 0; b < pow(2, orders[p].size()); b++) {
                    for (unsigned int i = 0; i < orders[p].size(); i++) {
                        if (((b >> i) % 2) == 1) {
                            unistroke.insert(unistroke.end(), multistroke[orders[p][i]].rbegin(), multistroke[orders[p][i]].rend());
                        } else {
                            unistroke.insert(unistroke.end(), multistroke[orders[p][i]].begin(), multistroke[orders[p][i]].end());
                        }
                    }
                    out_unistrokes.push_back(std::pair<point_2d, vector<point_2d> >(point_2d(0, 0), unistroke));
                    unistroke.clear();
                }
            }
            return;
        }

        //transforms multistroke gesture to one single stroke gesture
        void dollar_n::combine_strokes(const vector<vector<point_2d> > & multistrokes, vector<point_2d> & unistroke) {
            unistroke.clear();
            for (vector<vector<point_2d> >::const_iterator iter = multistrokes.begin(); iter != multistrokes.end(); iter++) {
                unistroke.insert(unistroke.end(), iter->begin(), iter->end());
            }
        }

        //non-uniformly scales gesture to the bounding box (based on scaling threshold set in the constructor)
        void dollar_n::dimensional_scale(vector<point_2d> & unistroke) const {
            libreco::rutils::rectangle b_box = libreco::rauxiliary::bounding_box(unistroke);
            float ratio = std::min(b_box.width() / b_box.height(), b_box.height() / b_box.width());

            if (ratio <= scaling_threshold) {
                for (vector<point_2d>::iterator iter = unistroke.begin(); iter != unistroke.end(); iter++) {
                    iter->set_x((iter->get_x() * scale_box_size) / std::max(b_box.width(), b_box.height()));
                    iter->set_y((iter->get_y() * scale_box_size) / std::max(b_box.width(), b_box.height()));
                }
            } else {
                for (vector<point_2d>::iterator iter = unistroke.begin(); iter != unistroke.end(); iter++) {
                    iter->set_x((iter->get_x() * scale_box_size) / b_box.width());
                    iter->set_y((iter->get_y() * scale_box_size) / b_box.height());
                }
            }
        }

        //computes start unit vector for given gesture based on start vector index set in the constructor
        point_2d dollar_n::start_unit_vector(const vector<point_2d> & unistroke) const {
            point_2d p;
            p.set_x(unistroke[start_vector_index].get_x() - unistroke[0].get_x());
            p.set_y(unistroke[start_vector_index].get_y() - unistroke[0].get_y());

            float length = sqrt(p.get_x() * p.get_x() + p.get_y() * p.get_y());

            p.set_x(p.get_x() / length);
            p.set_y(p.get_y() / length);

            return p;
        }

        //computes angle between two vectors
        float dollar_n::angle_between_vectors(const point_2d & p1, const point_2d & p2) {
            return std::acos(p1.get_x() * p2.get_x() + p1.get_y() * p2.get_y());
        }

        //compares unknown unistroke gesture to unistroke template
        float dollar_n::compare_gestures(const vector<multistroke_gesture>::const_iterator & tmpls_iter, const point_2d & start_vector, const vector<point_2d> & unistroke) const {
            float min_distance = FLT_MAX;

            for (vector<std::pair<point_2d, vector<point_2d> > >::const_iterator uni_iter = tmpls_iter->unistrokes.begin();
                    uni_iter != tmpls_iter->unistrokes.end(); uni_iter++) {

                if (angle_between_vectors(start_vector, uni_iter->first) <= start_vector_threshold) {
                    float distance = libreco::rauxiliary::distance_at_best_angle(unistroke, uni_iter->second,
                            rotation_down_limit, rotation_up_limit, rotation_threshold);
                    if (distance < min_distance) {
                        min_distance = distance;
                    }
                }
            }
            return min_distance;
        }

        libreco::recognizers::recognized_gestures dollar_n::recognize(const vector<vector<point_2d> > & unknown_gesture) const {
#ifdef TEST_PERFORMANCE
            libkerat::timetag_t recognize_start;
            lo_timetag_now(&recognize_start);
#endif
            //combine all strokes of mulitstroke gesture to one unistroke
            vector<point_2d> inv_unistroke;
            combine_strokes(unknown_gesture, inv_unistroke);

            //resample combined unistroke to n evenly spaced points
            libreco::rauxiliary::resample(inv_unistroke, number_of_points);

            //find indicative angle from centroid to first point
            //rotate unistroke so angle from centroid to first point is zero
            float angle = libreco::rauxiliary::indicative_angle(inv_unistroke);
            libreco::rauxiliary::rotate_by_angle(inv_unistroke, -angle);

            //dimensional sensitive scaling of gesture
            //translate to the origin
            dimensional_scale(inv_unistroke);
            libreco::rauxiliary::translate_to(inv_unistroke, origin);

            //create also orientation sensitive version of this gesture
            vector<point_2d> sens_unistroke = inv_unistroke;
            libreco::rauxiliary::rotate_by_angle(sens_unistroke, angle);

            //compute start unit vector for both gesture variants (sensitive and invariant)
            point_2d inv_start_vect = start_unit_vector(inv_unistroke);
            point_2d sens_start_vect = start_unit_vector(sens_unistroke);

            libreco::recognizers::recognized_gestures scores;
            float score = 0.0;
            float half_diagonal = 0.5 * sqrt((scale_box_size * scale_box_size) + (scale_box_size * scale_box_size));

            for (vector<libreco::rutils::multistroke_gesture>::const_iterator tmpls_iter = dollar_n_templates.begin();
                    tmpls_iter != dollar_n_templates.end(); tmpls_iter++) {

                float min_distance = FLT_MAX;
                //optional feature which compares only templates with the same number of strokes
                if (!equal_strokes_numbers || (unknown_gesture.size() == tmpls_iter->number_of_strokes)) {
                    if (tmpls_iter->sensitive) {
                        min_distance = compare_gestures(tmpls_iter, sens_start_vect, sens_unistroke);
                    } else {
                        min_distance = compare_gestures(tmpls_iter, inv_start_vect, inv_unistroke);
                    }
                }
                if (min_distance != FLT_MAX) {
                    score = 1.0 - (min_distance / half_diagonal);
                    scores.insert(std::pair<float, std::string > (score, tmpls_iter->name));
                }
            }

#ifdef TEST_PERFORMANCE
            libkerat::timetag_t recognize_end;
            lo_timetag_now(&recognize_end);
            libreco::iotools::log_performance_test(recognize_start, recognize_end, "../parsers/recognizers/performanceTests/dollarN.log",
                    "Dollar N recognizer ($N)", scores);
#endif
            return scores;
        }

    } // ns recognizers

    namespace rutils {
        //! \brief dollar_n name identifier
        const char * recognizer_name<libreco::recognizers::dollar_n>::NAME = "dollar_n $N";
    }

} // ns libreco

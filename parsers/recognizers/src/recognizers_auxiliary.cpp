/**
 * \file      recognizers_auxiliary.cpp
 * \brief     Common functions used by all recognizers
 * \author    Martin Luptak <374178@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2013-02-20 16:18 UTC+1
 * \copyright BSD
 */

#include <muse/recognizers/recognizers_auxiliary.hpp>
#include <kerat/utils.hpp>

#include <cmath>

#ifndef M_PI
    #define M_PI acos(-1);
#endif

namespace libreco {
    namespace rauxiliary {

        using std::vector;
        using libkerat::helpers::point_2d;
        
        //converts degrees to radians
        float degrees_to_radians(float degrees) {
            return degrees * (M_PI / 180.0);
        }
                
        //computes path length of stroke
        float path_length(const vector<point_2d> & points) {
            float p_length = 0.0;
            for (unsigned int i = 1; i < points.size(); i++) {
                p_length += distance_between_points(points[i - 1], points[i]);
            }
            return p_length;
        }
        
        //computes distance between two points in 2D space
        float distance_between_points(const point_2d & p_first, const point_2d & p_second) {
            float distance_x = p_second.get_x() - p_first.get_x();
            float distance_y = p_second.get_y() - p_first.get_y();
            return sqrt((distance_x * distance_x) + (distance_y * distance_y));
        }
        
        //resample input points to n equidistantly spaced points (n is number_of_points parameter)
        void resample(vector<point_2d> & points, unsigned int number_of_points) {
            float resample_dist = path_length(points) / (number_of_points - 1);
            float partial_dist_sum = 0.0;
            vector<point_2d> resampled_points;

            resampled_points.reserve(number_of_points);
            resampled_points.push_back(points[0]);

            for (unsigned int i = 1; i < points.size(); i++) {
                float partial_dist = distance_between_points(points[i - 1], points[i]);

                if ((partial_dist_sum + partial_dist) >= resample_dist) {
                    float ratio = (resample_dist - partial_dist_sum) / partial_dist;
                    float p_coord_x = points[i - 1].get_x() + ratio * (points[i].get_x() - points[i - 1].get_x());
                    float p_coord_y = points[i - 1].get_y() + ratio * (points[i].get_y() - points[i - 1].get_y());
                    resampled_points.push_back(point_2d(p_coord_x, p_coord_y));
                    points.insert(points.begin() + i, 1, point_2d(p_coord_x, p_coord_y));
                    partial_dist_sum = 0.0;
                } else {
                    partial_dist_sum += partial_dist;
                }
            }

            if (resampled_points.size() == (number_of_points - 1)) {
                resampled_points.insert(resampled_points.end(), 1, points[points.size() - 1]);
            }
            points = resampled_points;
        }
        
        //move points, so their centroid is new center parameter
        void translate_to(vector<point_2d> & points, const point_2d & new_center) {
            point_2d center = centroid(points);
            for (vector<point_2d>::iterator iter = points.begin(); iter != points.end(); iter++) {
                iter->set_x(iter->get_x() + new_center.get_x() - center.get_x());
                iter->set_y(iter->get_y() + new_center.get_y() - center.get_y());
            }
        }
        
        //computes centroid of given stroke
        const point_2d centroid(const vector<point_2d> & points) {
            float centroid_x = 0.0;
            float centroid_y = 0.0;

            for (vector<point_2d>::const_iterator iter = points.begin(); iter != points.end(); iter++) {
                centroid_x += iter->get_x();
                centroid_y += iter->get_y();
            }
            centroid_x /= points.size();
            centroid_y /= points.size();
            return point_2d(centroid_x, centroid_y);
        }
        
        float indicative_angle(const vector<point_2d> & points) {
            point_2d center = centroid(points);
            return atan2(center.get_y() - points[0].get_y(), center.get_x() - points[0].get_x());
        }
        
        //rotates all points of given stroke by given angle
        void rotate_by_angle(vector<point_2d> & pts_to_rotate, float angle) {
            point_2d center = centroid(pts_to_rotate);
            float p_coord_x = 0.0;
            float p_coord_y = 0.0;
            float cos_value = cos(angle);
            float sin_value = sin(angle);

            for (vector<point_2d>::iterator iter = pts_to_rotate.begin(); iter != pts_to_rotate.end(); iter++) {
                p_coord_x = iter->get_x();
                p_coord_y = iter->get_y();
                iter->set_x(((p_coord_x - center.get_x()) * cos_value) - ((p_coord_y - center.get_y()) * sin_value) + center.get_x());
                iter->set_y(((p_coord_x - center.get_x()) * sin_value) + ((p_coord_y - center.get_y()) * cos_value) + center.get_y());
            }
        }
        
        //computes minimal bounding box for given gesture
        libreco::rutils::rectangle bounding_box(const vector<point_2d> & points) {
            float min_x = points[0].get_x();
            float min_y = points[0].get_y();
            float max_x = min_x;
            float max_y = min_y;

            for (vector<point_2d>::const_iterator iter = points.begin() + 1; iter != points.end(); iter++) {
                float current_x = iter->get_x();
                float current_y = iter->get_y();

                max_x = std::max(max_x, current_x);
                min_x = std::min(min_x, current_x);
                max_y = std::max(max_y, current_y);
                min_y = std::min(min_y, current_y);
            }
            return libreco::rutils::rectangle(point_2d(min_x, min_y), point_2d(max_x, max_y));
        }
        
        //find minimal distance at best angle between unknown gesture a given pattern
        float distance_at_best_angle(const vector<point_2d> & points, const vector<point_2d> & pattern,
                float down_lim, float top_lim, float thres) {

            float golden_ratio = 0.5 * (sqrt(5.0) - 1.0);
            float x_1 = golden_ratio * down_lim + (1.0 - golden_ratio) * top_lim;
            float f_1 = distance_at_angle(points, pattern, x_1);
            float x_2 = golden_ratio * top_lim + (1.0 - golden_ratio) * down_lim;
            float f_2 = distance_at_angle(points, pattern, x_2);

            while (std::abs(top_lim - down_lim) > thres) {
                if (f_1 < f_2) {
                    top_lim = x_2;
                    x_2 = x_1;
                    f_2 = f_1;
                    x_1 = golden_ratio * down_lim + (1.0 - golden_ratio) * top_lim;
                    f_1 = distance_at_angle(points, pattern, x_1);
                } else {
                    down_lim = x_1;
                    x_1 = x_2;
                    f_1 = f_2;
                    x_2 = golden_ratio * top_lim + (1.0 - golden_ratio) * down_lim;
                    f_1 = distance_at_angle(points, pattern, x_2);
                }
            }
            return std::min(f_1, f_2);
        }

        float distance_at_angle(const vector<point_2d> & points, const vector<point_2d> & pattern, float angle) {
            //local copy because original points must not be modified
            vector<point_2d> local_stroke_copy = points;
            rotate_by_angle(local_stroke_copy, angle);

            return path_distance(local_stroke_copy, pattern);
        }

        float path_distance(const vector<point_2d> & points, const vector<point_2d> & pattern) {
            float distance = 0.0;
            for (unsigned int i = 0; i < points.size(); i++) {
                distance += distance_between_points(points[i], pattern[i]);
            }
            return distance / (float) points.size();
        }
        
        //insert new point in correct time order
        void insert_in_order(vector<libreco::rutils::point_time> & stroke, const libreco::rutils::point_time & point) {
            
            //if this is first point received no actions below are needed
            if (stroke.empty()) {
                stroke.push_back(point);
                return;
            }
            
            //correct order, new 2D point arrival time is greater than or equals last point arrival time
            if ((stroke.back().arrival_time < point.arrival_time) || (stroke.back().arrival_time == point.arrival_time)) {
                stroke.push_back(point);
                return;
            }
            
            //wrong order, new 2D point arrival time is lass than last point arrival time, wrong order
            //now start steps needed to place point at right place
            for (int i = stroke.size() - 1; i >= 0; i--) {
                if ((stroke[i].arrival_time < point.arrival_time) || (stroke[i].arrival_time == point.arrival_time)) {
                    stroke.insert(stroke.begin() + i + 1, 1, point);
                    //point is placed, no more actions needed
                    return;
                }
            }
            stroke.insert(stroke.begin(), 1, point);
        }

    } //ns rauxiliary
} //ns libreco
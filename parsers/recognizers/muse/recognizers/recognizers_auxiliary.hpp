/**
 * \file      recognizers_auxiliary.hpp
 * \brief     Provides the header file for auxiliary functions used by implemented recognizers
 * \author    Martin Luptak <374178@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2013-02-20 16:18 UTC+1
 * \copyright BSD
 */

#ifndef RECOGNIZERS_AUXILIARY_HPP_
#define	RECOGNIZERS_AUXILIARY_HPP_

#include <kerat/message_helpers.hpp>
#include <muse/recognizers/recognizers_utils.hpp>

#include <vector>

namespace libreco {
    namespace rauxiliary {

        using std::vector;
        using libkerat::helpers::point_2d;
        
        /**
         * \brief Converts degrees to radians
         * 
         * @param degrees   degrees to be converted
         * @return          radians
         */
        float degrees_to_radians(float degrees);
        
        /**
         * \brief Computes path length of the stroke passed to the function as argument
         * 
         * Stroke is represented as vector of points in 2D space and points are 
         * instances of libkerat::helpers::point_2d class.
         * 
         * \see libkerat::helpers::point_2d
         * 
         * \param points    length of this stroke will be computed
         * \return          path length of given stroke
         */
        float path_length(const vector<point_2d> & points);

        /**
         * \brief Computes distance between two points in 2D space.
         * 
         * Points are instances of libkerat::helpers::point_2d class.
         * 
         * \see libkerat::helpers::point_2d
         * 
         * \param p_first   first point in 2D space
         * \param p_second  second point in 2D space
         * \return          distance between first and second point
         */
        float distance_between_points(const point_2d & p_first, const point_2d & p_second);

        /**
         * \brief Resamples stroke, so it is made of N evenly spaced points.
         * 
         * Resamples input stroke to N evenly spaced points where N is specified as function argument (number_of_points).
         * This method is needed because gestures are made with different speeds what results in different number of points.
         * Linear interpolation is used for resampling.
         * 
         * \param points            input points which are going to be resampled
         * \param number_of_points  number of points after resampling is done
         */
        void resample(vector<point_2d> & points, unsigned int number_of_points);

        /**
         * \brief Gesture is moved, so its center is origin specified as function argument.
         * 
         * \param points        points which will be moved, so their center will be origin
         * \param new_center    new center, where points will be moved
         */
        void translate_to(vector<point_2d> & points, const point_2d & new_center);

        /**
         * \brief Computes centroid of stroke
         * 
         * \param points    centroid of these points will be computed
         * \return          centroid of stroke (libkerat::helpers::point_2d instance)
         */
        const point_2d centroid(const vector<point_2d> & points);

        /**
         * \brief Computes indicative angle between first point of stroke and its centroid
         * 
         * \param points    indicative angle for this stroke will be computed
         * \return          computed angle
         */
        float indicative_angle(const vector<point_2d> & points);


        /**
         * \brief Rotates stroke (and all its points) around centroid by angle given as parameter
         * 
         * \param pts_to_rotate stroke which will be rotated
         * \param angle         angle used for rotation
         */
        void rotate_by_angle(vector<point_2d> & pts_to_rotate, float angle);

        /**
         * \brief Computes minimal bounding box for given stroke.
         * 
         * \param points    bounding box will be computed for this stroke
         * \return          minimal bounding box (\ref libreco::rutils::rectangle)
         */
        libreco::rutils::rectangle bounding_box(const vector<point_2d> & points);

        /**
         * \brief Computes average distance between unknown gesture points and given pattern
         * 
         * \param points        unknown gesture to be compared with given pattern
         * \param pattern       pattern which is going to be compared with unknown gesture
         * \return              average distance between unknown gesture and pattern points
         */
        float path_distance(const vector<point_2d> & points, const vector<point_2d> & pattern);

        /**
         * \brief Searches for best score between unknown gesture and given pattern.
         * 
         * \param points        unknown gesture
         * \param pattern       template which will be compared to unknown gesture
         * \param down_lim      lower-bound for unknown gesture rotation
         * \param top_lim       upper-bound for unknown gesture rotation
         * \param thres         threshold used to stop the process of searching for better match
         * \return              the best score of scores computed for given unknown gesture and given pattern
         */
        float distance_at_best_angle(const vector<point_2d> & points, const vector<point_2d> & pattern, float down_lim, float top_lim, float thres);

        /**
         * \brief Computes distance between unknown gesture and given pattern at given angle
         * 
         * \param points    unknown gesture will be compared to given pattern
         * \param pattern   pattern which will be compared to unknown gesture
         * \param angle     unknown gesture will be rotated by this angle
         * \return          distance between given pattern and unknown gesture rotated by given angle
         */
        float distance_at_angle(const vector<point_2d> & points, const vector<point_2d> & pattern, float angle);

        /**
         * \brief Inserts point to vector in correct time order (so UDP protocol is working correctly)
         * 
         * \param stroke    vector of points correctly sorted by arrival time
         * \param point     new point which will be placed in the right place in vector, based on its creation time
         */
        void insert_in_order(vector<libreco::rutils::point_time> & stroke, const libreco::rutils::point_time & point);

    } //ns rauxiliary
} //ns libreco
#endif	/* RECOGNIZERS_AUXILIARY_HPP_ */


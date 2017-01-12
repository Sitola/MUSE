/**
 * \file      io_utils.hpp
 * \brief     Provides the header file for input and output utilities
 * \author    Martin Luptak <374178@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-11-11 09:02 UTC+1
 * \copyright BSD
 */

#ifndef IO_UTILS_HPP_
#define IO_UTILS_HPP_

#include <kerat/message_helpers.hpp>
#include <muse/recognizers/recognizers_utils.hpp>
#include <muse/recognizers/typedefs.hpp>

#include <vector>
#include <fstream>

namespace libreco {
    namespace iotools {
        
              
        /**
         * \brief Creates simple unistroke template
         * 
         * \param stroke    gesture
         * \param out_file  output file stream
         */
        void simple_unistroke_template(const std::vector<libkerat::helpers::point_2d> & stroke, std::ofstream & out_file);
        
        /**
         * \brief Creates simple multistroke template
         * 
         * \param strokes   gesture
         * \param out_file  output file stream
         */
        void simple_multistroke_template(const std::vector<std::vector<libkerat::helpers::point_2d> > & strokes, std::ofstream & out_file);
        
        /**
         * \brief Prints unistroke gesture points to output SVG file (creates visual representation of gesture)
         * 
         * \param stroke        gesture points will be printed to given SVG file
         * \param file_path     path to output SVG file
         * \param center        centroid of gesture will be also printed if specified (optional)
         * \param bounding_box  minimal bounding box of gesture will be also printed if specified (optional)
         */
        void unistroke_to_svg(const std::vector<libkerat::helpers::point_2d> & stroke, std::string file_path,
                              const libkerat::helpers::point_2d * center = NULL, const libreco::rutils::rectangle * bounding_box = NULL);
        
        
        /**
         * \brief Prints unistroke gesture points to output file stream (creates visual representation of gesture)
         * 
         * \param stroke        gesture points will be printed to given output file stream
         * \param out_file      output file stream
         * \param center        centroid of gesture will be also printed if specified (optional)
         * \param bounding_box  minimal bounding box of gesture will be also printed if specified (optional)
         */
        void unistroke_to_svg(const std::vector<libkerat::helpers::point_2d> & stroke, std::ofstream & out_file,
                              const libkerat::helpers::point_2d * center = NULL, const libreco::rutils::rectangle * bounding_box = NULL);
        
        /**
         * \brief Prints multistroke gesture points to output SVG file (creates visual representation of gesture)
         * 
         * \param strokes       multistroke gesture points will be printed to given SVG file
         * \param file_path     path to output SVG file
         * \param center        centroid of gesture will be also printed if specified (optional)
         * \param bounding_box  minimal bounding box of gesture will be also printed if specified (optional)
         */
        void multistroke_to_svg(const std::vector<std::vector<libkerat::helpers::point_2d> > & strokes, std::string file_path,
                                const libkerat::helpers::point_2d * center = NULL, const libreco::rutils::rectangle * bounding_box = NULL);
        
        /**
         * \brief Prints multistroke gesture points to output file stream (creates visual representation of gesture)
         * 
         * \param strokes       multistroke gesture points will be printed to given output file stream
         * \param out_file      output file stream
         * \param center        centroid of gesture will be also printed if specified (optional)
         * \param bounding_box  minimal bounding box of gesture will be also printed if specified (optional)
         */
        void multistroke_to_svg(const std::vector<std::vector<libkerat::helpers::point_2d> > & strokes, std::ofstream & out_file,
                                const libkerat::helpers::point_2d * center = NULL, const libreco::rutils::rectangle * bounding_box = NULL);
        
        
        /**
         * \brief Creates log file with recognition results. Used for performance testing purposes.
         * 
         * @param start             recognition start 
         * @param end               recognition end
         * @param file_path         file to write results
         * @param recognizer_name   name of recognizer used
         * @param scores            scores of recognition process
         */
        void log_performance_test(const libkerat::timetag_t & start, const libkerat::timetag_t & end, const std::string & file_path,
                                  const std::string & recognizer_name, const libreco::recognizers::recognized_gestures & scores);
        
    } // ns iotools
} // ns libreco

#endif /* IO_UTILS_HPP_ */
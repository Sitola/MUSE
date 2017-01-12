/**
 * \file      io_utils.cpp
 * \brief     Implements functionality for input and output utilities used for testing purposes
 * \author    Martin Luptak <374178@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-11-11 09:02 UTC+1
 * \copyright BSD
 */

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>

#include <kerat/message_helpers.hpp>
#include <kerat/typedefs.hpp>
#include <kerat/utils.hpp>
#include <muse/recognizers/io_utils.hpp>
#include <muse/recognizers/typedefs.hpp>

namespace libreco {
    namespace iotools {
        
        using libkerat::helpers::point_2d;
        using std::cout;
        using std::endl;
        using std::cerr;
        
        //! \brief prints SVG header to given output file stream
        static void print_svg_header(std::ofstream & out_file) {
            out_file << "<?xml version=\"1.0\" standalone=\"no\"?>" << endl;
            out_file << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">" << endl;
            
            //this option multiplies screen 4 times, so it is possible to see gestures with center on 0,0
            out_file << "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\"3840px\" height=\"2160px\" viewBox=\"-1920 -1080 3840 2160\" >" << endl;
            
            //this option can display only gestures with positive coordinates
            //out_file << "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\"1920px\" height=\"1080px\" viewBox=\"0 0 1920 1080\" >" << endl;
        }
        
        //! \brief prints given bounding box of gesture to given output file stream
        static void print_bounding_box(std::ofstream & out_file, const libreco::rutils::rectangle * bounding_box) {
            //to be sure that bounding_box is not null
            if(bounding_box == NULL) { return; }
            
            //prints minimal bounding box
            out_file << "<rect x=\"" << bounding_box->get_bottom_left().get_x() << "\" y=\"" << bounding_box->get_bottom_left().get_y()
                     << "\" width=\"" << bounding_box->get_top_right().get_x() - bounding_box->get_bottom_left().get_x()
                     << "\" height=\"" << bounding_box->get_top_right().get_y() - bounding_box->get_bottom_left().get_y()
                     << "\" style=\"fill: none; stroke: black; stroke-width:4\"/>" << endl;
        }
        
        //! \brief prints center to gesture to given output file stream
        static void print_center(std::ofstream & out_file, const point_2d * center) {
            //to be sure that center is not null
            if(center == NULL) { return; }
            
            //prints center
            out_file << "<circle cx=\"" << center->get_x() << "\" cy=\"" << center->get_y()
                     << "\" r=\"10.0\" style=\"stroke: black; fill: black;\"/>" << endl;
        }
        
        //! \brief prints gesture to given output file stream (it is possible to set first point size)
        static void print_points(std::ofstream & out_file, const std::vector<point_2d> & points, uint16_t first_point_size) {
            //to be sure that vector is not empty
            if(points.empty()) { return; }
            
            //first point is bigger so it is easy to tell where is the beginning of gesture (stroke)
            out_file << "<circle cx=\"" << points.begin()->get_x() << "\" cy=\"" << points.begin()->get_y()
                     << "\" r=\"" << first_point_size << "\" style=\"stroke: black; fill: black;\"/>" << endl;
            
            //prints points to SVG one after another
            for (std::vector<point_2d>::const_iterator iter = (points.begin() + 1); iter != points.end(); iter++) {
                out_file << "<circle cx=\"" << iter->get_x() << "\" cy=\"" << iter->get_y()
                         << "\" r=\"4.0\" style=\"stroke: black; fill: black;\"/>" << endl;
            }
        }
        
        void simple_unistroke_template(const std::vector<libkerat::helpers::point_2d> & stroke, std::ofstream & out_file) {
            if (!(out_file.is_open())) {
                cerr << "Output stream is not opened!!! Failed to create template." << endl;
                return;
            }
            
            out_file << "<uni_gesture gesture_id=\"\" name=\"\" sensitivity=\"\" revert=\"\" >" << endl;
            for(std::vector<point_2d>::const_iterator iter = stroke.begin(); iter != stroke.end(); iter++) {
                out_file << "\t" << iter->get_x() << "\t" << iter->get_y() << endl;
            }
            
            out_file << "</uni_gesture>" << endl;
        }
        
        void simple_multistroke_template(const std::vector<std::vector<libkerat::helpers::point_2d> > & strokes, std::ofstream & out_file) {
            if (!(out_file.is_open())) {
                cerr << "Output stream is not opened!!! Failed to create template." << endl;
                return;
            }
            
	    uint16_t stroke_id = 0;
            out_file << "<multi_gesture gesture_id=\"\" name=\"\" sensitivity=\"\" >" << endl;
            for(std::vector<std::vector<point_2d> >::const_iterator strokes_iter = strokes.begin(); strokes_iter != strokes.end(); strokes_iter++) {
                out_file << "\t<stroke stroke_id=\"" << ++stroke_id << "\">" << endl;
                for(std::vector<point_2d>::const_iterator pts_iter = strokes_iter->begin(); pts_iter != strokes_iter->end(); pts_iter++) {
                    out_file << "\t\t" << pts_iter->get_x() << "\t" << pts_iter->get_y() << endl;
                }
                out_file << "\t</stroke>" << endl;
            }
            
            out_file << "</multi_gesture>" << endl;
            
        }
        
        void unistroke_to_svg(const std::vector<libkerat::helpers::point_2d> & stroke, std::ofstream & out_file,
                              const libkerat::helpers::point_2d * center, const libreco::rutils::rectangle * bounding_box) {
            if (!out_file.is_open()) {
                cerr << "Output stream is not opened!!! Failed to create SVG." << endl;
                return;
            }
            
            //print SVG header
            print_svg_header(out_file);
            
            //print gesture (stroke) points to SVG file (number 6 specifies size of first point in gesture)
            print_points(out_file, stroke, 10);
            
            //print center of gesture if specified
            if(center != NULL) { 
                print_center(out_file, center);
                out_file << "<line x1=\"" << center->get_x() << "\" y1=\"" << center->get_y() << "\" x2=\"";
                out_file << stroke.begin()->get_x() << "\" y2=\"" << stroke.begin()->get_y() << "\" ";
                out_file << "style=\"stroke:rgb(0,0,0);stroke-width:6\" />" << endl;            
            }
            
            //print minimal bounding box of gesture if specified
            if(bounding_box != NULL) { print_bounding_box(out_file, bounding_box); }
            
            out_file << "</svg>" << endl;
        }
        
        void unistroke_to_svg(const std::vector<libkerat::helpers::point_2d> & stroke, std::string file_path,
                              const libkerat::helpers::point_2d * center, const libreco::rutils::rectangle * bounding_box) {
            
            //try to open file with given file path
            std::ofstream out_file(file_path.c_str());
            if (!(out_file.is_open())) {
                cerr << "Error while opening SVG file: " << file_path << endl;
                return;
            }
            unistroke_to_svg(stroke, out_file, center, bounding_box);
            out_file.close();
        }
        
        void multistroke_to_svg(const std::vector<std::vector<libkerat::helpers::point_2d> > & strokes, std::string file_path,
                                const libkerat::helpers::point_2d * center, const libreco::rutils::rectangle * bounding_box) {
            
            //try to open file with given file path
            std::ofstream out_file(file_path.c_str());
            if (!(out_file.is_open())) {
                cerr << "Error while opening SVG file: " << file_path << endl;
                return;
            }
            multistroke_to_svg(strokes, out_file, center, bounding_box);
            out_file.close();
        }
        
        void multistroke_to_svg(const std::vector<std::vector<libkerat::helpers::point_2d> > & strokes, std::ofstream & out_file,
                                const libkerat::helpers::point_2d * center, const libreco::rutils::rectangle * bounding_box) {
            if (!out_file.is_open()) {
                cerr << "Output stream is not opened!!! Failed to create SVG." << endl;
                return;
            }
            
            //print SVG header
            print_svg_header(out_file);
            
            //size of first point in stroke
            uint16_t first_point_size = 4;
            
            //print strokes to SVG file
            for(std::vector<std::vector<point_2d> >::const_iterator iter = strokes.begin(); iter != strokes.end(); iter++) {
                //first point of partial stroke is bigger and bigger in every iteration
                //so it is easy to tell order of strokes and first point of each stroke
                print_points(out_file, *iter, first_point_size + 2);
            }
            
            //print center of gesture if specified
            if(center != NULL) { print_center(out_file, center); }
            
            //print minimal bounding box of gesture if specified
            if(bounding_box != NULL) { print_bounding_box(out_file, bounding_box); }
            
            out_file << "</svg>" << endl;
            
        }
        
        void log_performance_test(const libkerat::timetag_t & start, const libkerat::timetag_t & end, const std::string & file_path,
                                  const std::string & recognizer_name, const libreco::recognizers::recognized_gestures & scores) {
            
            libkerat::timetag_t performance_time = libkerat::timetag_diff_abs(start, end);
            std::ofstream performance_file;
            
            performance_file.open(file_path.c_str(), std::fstream::app);
            if(!performance_file.is_open()) {
                std::cerr << "Error opening file for performance results: " << file_path << std::endl;
            }
            int n_best = 4;
            int iteration_counter = 0;
            
            //commented out parts create output that looks like table
            performance_file << recognizer_name; /*<< std::setw(28 - recognizer_name.length());*/
            for(libreco::recognizers::recognized_gestures::const_iterator iter = scores.begin(); iter != scores.end(); iter++) {
                std::stringstream item;
                item << iter->first << ":" << iter->second;
                performance_file << ":" << item.str();
                /*performance_file << std::setw(34 - item.str().length());*/
                
                //print only n-best list
                if(++iteration_counter >= n_best) {
                    break;
                }
            }
            
            double microseconds = (performance_time.sec * pow(10, 6)) + ((performance_time.frac / pow(2, 32)) * pow(10, 6));
            
            performance_file << ":speed" << ":"; /*<< std::setw(5);*/
            performance_file << floor(microseconds + 0.5) << std::endl;
            
            performance_file.close();
        }

    } // ns iotools
} // ns libreco

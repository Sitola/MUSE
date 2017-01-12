//============================================================================
// Name        : template_gen.cpp
// Author      : Martin Ľupták
// Version     :
// Copyright   : BSD
// Description : This source code provides basic functionality for generating
//               unistroke and multistroke templates.
//============================================================================

#include <kerat/kerat.hpp>
#include <muse/recognizers/libreco.hpp>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

using std::cout;
using std::endl;
using std::cerr;
using std::string;

using libreco::rutils::point_time;
using libkerat::helpers::point_2d;

//! \brief shows command line options for this utility
static void usage();

//! \brief creates unistroke template
static void create_unistroke(char** args);

//! \brief creates multistroke template
static void create_multistroke(char ** args, long sec);

//! \brief creates SVG representation of unistroke made by user
static void visual_representation_single(std::vector<point_2d> & points, char * current_directory);

//! \brief creates SVG representation of multistroke made by user
static void visual_representation_multi(std::vector<std::vector<point_2d> > & strokes, char * current_directory);

//! \brief converts vector of point_time structures to vector of point_2d instances
static std::vector<point_2d> convert_to_vector_point2d(const std::pair<session_id_t, std::vector<point_time> > & original) {
    std::vector<point_2d> tmp_stroke;
    std::transform(original.second.begin(), original.second.end(), std::back_inserter(tmp_stroke), point_time::get_point2d);
    return tmp_stroke;
}

int main(int argc, char** argv) {
    if (argc == 3) {
        string strokes_type = argv[1];
        if ((strokes_type.compare("--unistroke") != 0) && (strokes_type.compare("-u") != 0)) {
            cerr << "Wrong command line arguments!!!" << endl;
            usage();
            exit(EXIT_FAILURE);
        }
        create_unistroke(argv);
        return 0;
    }

    if (argc == 4) {
        string strokes_type = argv[1];
        if ((strokes_type.compare("--multistroke") != 0) && (strokes_type.compare("-m") != 0)) {
            cerr << "Wrong command line arguments!!!" << endl;
            usage();
            exit(EXIT_FAILURE);
        }

        char * endptr = NULL;
        long sec = strtol(argv[2], &endptr, 10);
        if (endptr != (argv[2] + strlen(argv[2]))) {
            cerr << "Wrong command line arguments!!!" << endl;
            usage();
            exit(EXIT_FAILURE);
        }
        create_multistroke(argv, sec);
        return 0;
    }

    cerr << "Wrong command line arguments!!!" << endl;
    usage();
    exit(EXIT_FAILURE);
}

static void create_unistroke(char** args) {

    typedef libkerat::bundle_handle::const_iterator message_iterator;
    typedef libkerat::message::pointer message_pointer;

    libkerat::adaptor * multi_adaptor = new libkerat::adaptors::multiplexing_adaptor;
    libkerat::simple_client * s_client = new libkerat::simple_client(3333);
    s_client->add_listener(multi_adaptor);

    libkerat::bundle_stack stack;
    libkerat::bundle_handle f;

    libkerat::session_set previous_alive_ids;
    std::vector<point_time> stroke;

    while (true) {
        multi_adaptor->load();
        stack = multi_adaptor->get_stack();

        if (stack.get_length() != 0) {
            f = stack.get_update();
            libkerat::timetag_t curr_timestamp = f.get_frame()->get_timestamp();

            for (message_iterator msg_iter = f.begin(); msg_iter != f.end(); msg_iter++) {
                message_pointer * ptr_test = dynamic_cast<message_pointer *> (*msg_iter);
                if (ptr_test != NULL) {
                    point_time curr_point(curr_timestamp, libkerat::helpers::point_2d(ptr_test->get_x(), ptr_test->get_y()));
                    libreco::rauxiliary::insert_in_order(stroke, curr_point);
                }
            }

            libkerat::session_set removed_ids = libkerat::extract_removed_ids(previous_alive_ids, f.get_alive()->get_alives());
            previous_alive_ids = f.get_alive()->get_alives();

            if (!removed_ids.empty()) {
                std::ofstream out_file(args[2], std::ios::app);
                if (!out_file.is_open()) {
                    cerr << "Error while opening given file: " << args[2] << endl;
                    exit(EXIT_FAILURE);
                }

                std::vector<point_2d> transformed_stroke;
                std::transform(stroke.begin(), stroke.end(), std::back_inserter(transformed_stroke), point_time::get_point2d);
                libreco::iotools::simple_unistroke_template(transformed_stroke, out_file);
                cout << "Your stroke was successfully written to this template file: " << args[2] << endl;
                out_file.close();
                stroke.clear();
                
                //create also SVG representation, so user can check what his template looks like
                visual_representation_single(transformed_stroke, args[0]);
            }
        }
    }
    delete s_client;
    delete multi_adaptor;
}

static void create_multistroke(char ** args, long sec) {
    typedef libkerat::bundle_handle::const_iterator message_iterator;
    typedef libkerat::message::pointer message_pointer;

    libkerat::adaptor * multi_adaptor = new libkerat::adaptors::multiplexing_adaptor;
    libkerat::simple_client * s_client = new libkerat::simple_client(3333);
    s_client->add_listener(multi_adaptor);

    libkerat::bundle_stack stack;
    libkerat::bundle_handle f;

    std::map<libkerat::session_id_t, std::vector<point_time> > strokes;
    std::map<libkerat::session_id_t, std::vector<point_time> >::iterator strokes_iter;
    libkerat::timetag_t limit = {sec, 0};

    while (true) {
        libkerat::timetag_t now;
        if (!strokes.empty()) {
            strokes_iter = --(strokes.end());
            lo_timetag_now(&now);
            libkerat::timetag_t diff = libkerat::timetag_diff_abs(strokes_iter->second.back().arrival_time, now);
            if (limit < diff) {
                std::ofstream out_file(args[3], std::ios::app);
                if (!out_file.is_open()) {
                    cerr << "Error while opening given file: " << args[3] << endl;
                    exit(EXIT_FAILURE);
                }
                
                std::vector<std::vector<point_2d> > tmp_strokes;
                std::transform(strokes.begin(), strokes.end(), std::back_inserter(tmp_strokes), convert_to_vector_point2d);
                if (tmp_strokes.size() == 1) {
                    libreco::iotools::simple_unistroke_template(tmp_strokes.front(), out_file);
                } else {
                    libreco::iotools::simple_multistroke_template(tmp_strokes, out_file);
                }
                cout << "Your stroke was successfully written to this template file: " << args[3] << endl;
                out_file.close();
                strokes.clear();
                
                //create also SVG representation, so user can check what his template looks like
                visual_representation_multi(tmp_strokes, args[0]);
            }
        }

        multi_adaptor->load();
        stack = multi_adaptor->get_stack();

        if (stack.get_length() != 0) {
            f = stack.get_update();
            libkerat::timetag_t curr_timestamp = f.get_frame()->get_timestamp();

            for (message_iterator msg_iter = f.begin(); msg_iter != f.end(); msg_iter++) {
                message_pointer * ptr_test = dynamic_cast<message_pointer *> (*msg_iter);
                if (ptr_test != NULL) {
                    point_time curr_point(curr_timestamp, libkerat::helpers::point_2d(ptr_test->get_x(), ptr_test->get_y()));
                    strokes_iter = strokes.insert(strokes.begin(), std::pair<libkerat::session_id_t, std::vector<point_time> > (ptr_test->get_session_id(), std::vector<point_time > ()));

                    libreco::rauxiliary::insert_in_order(strokes_iter->second, curr_point);
                }

            }
        }
    }
    delete s_client;
    delete multi_adaptor;

}

static void visual_representation_single(std::vector<point_2d> & points, char* current_directory) {
    static int file_counter = 1;
    std::stringstream file_name;
    file_name << current_directory << "_singlestroke_template_" << file_counter++ << ".svg";

    std::ofstream out_file(file_name.str().c_str(), std::ios_base::app);
    if (!out_file.is_open()) {
        cerr << "Can not create single stroke visual representation!!!" << endl;
        cerr << "Error while opening file: " << file_name.str() << endl;
        return;
    }

    libreco::iotools::unistroke_to_svg(points, out_file);
    out_file.close();
}

static void visual_representation_multi(std::vector<std::vector<point_2d> > & strokes, char * current_directory) {
    static int file_counter = 1;
    std::stringstream file_name;
    file_name << current_directory << "_multistroke_template_" << file_counter++ << ".svg";

    std::ofstream out_file(file_name.str().c_str(), std::ios_base::app);
    if (!out_file.is_open()) {
        cerr << "Can not create multi stroke visual representation!!!" << endl;
        cerr << "Error while opening file: " << file_name.str() << endl;
        return;
    }

    libreco::iotools::multistroke_to_svg(strokes, out_file);
    out_file.close();
}

static void usage() {
    cout << "Usage:" << endl;
    cout << "main [options] [file]\t\t" << endl;
    cout << endl;
    cout << "Options:" << endl;
    cout << "--unistroke                        \tCreates unistroke template." << endl;
    cout << "--multistroke <seconds>            \tCreates multistroke template with given timeout." << endl;
    cout.flush();
}


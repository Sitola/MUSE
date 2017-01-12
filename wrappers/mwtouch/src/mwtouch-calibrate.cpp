/**
 * @file      mwtouch-calibrate.cpp
 * @brief     The calibration utility for mwtouch input wrapper
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2011-03-16 23:55 UTC+2
 * @copyright BSD
 */

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <map>
#include <vector>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <linux/input.h>
#include <cassert>
#include <getopt.h>
#include <tinyxml.h>

#include "device.hpp"
#include "xdevice.hpp"
#include "window.hpp"
#include "axis.hpp"
#include "common.hpp"
#include "eventdumper.hpp"
#include "geometry.hpp"

struct calibrate_config {
    typedef enum {OPMODE_HELP, OPMODE_LIST, OPMODE_AXISDETECT, OPMODE_AUTOCONF, OPMODE_CALIBRATE} operation_mode;

    calibrate_config()
      :mode(OPMODE_CALIBRATE), sensor_fd(-1), output_file_fd(STDOUT_FILENO), verbose(false), threshold_time(2), threshold_samples(0)
    { uuid_clear(sensor_uuid); }

    operation_mode mode;
    
    std::string original_device_path;
    std::string device_path;
    std::string screen_name;
    std::string output_file_name;
    uuid_t sensor_uuid;
    
    int sensor_fd;
    int output_file_fd;
    
    bool verbose;

    x11_output_info x11_output;
    
    /**
     * Axis mappings
     */
    axis_mapping_map axes_mappings;

    double threshold_time;
    size_t threshold_samples;
     
};

static bool test_bit(int bit, uint8_t * data);

static int open_device(const char * device);

static int process_sensor(calibration_point & point, int sensor_fd, bool & touch_active, const axis_mapping_map & mappings, const bool & verbose);
inline void clean_point(calibration_point & point);
static bool extrude_coordinates(calibration_point * points, const unsigned short distance, const x11_output_info);

// for sorting the output priorities
inline bool sort_mappings_by_priority_comparator(const axis_mapping & a, const axis_mapping & b){ return a.priority > b.priority; }

static void init_mappings(axis_mapping_map & mappings){
    mappings[ABS_MT_POSITION_X] = axis_mapping(ABS_X, 2);
    mappings[ABS_MT_POSITION_Y] = axis_mapping(ABS_Y, 2);
    mappings[ABS_X] = axis_mapping(ABS_X, 1);
    mappings[ABS_Y] = axis_mapping(ABS_Y, 1);

}

static void clear_mappings(axis_mapping_map & mappings, const ev_code_t code){
    axis_mapping_map::const_iterator i = mappings.begin();
    while (i != mappings.end()){
        if (i->second.code == code){
            mappings.erase(i->first);
            i = mappings.begin();
        } else {
            i++;
        }
    }
}

static int count_mappings(const axis_mapping_map & mappings, const ev_code_t code){
    int retval = 0;
    for (axis_mapping_map::const_iterator i = mappings.begin(); i != mappings.end(); i++){
        if (i->second.code == code){ retval++; }
    }
    return retval;
}

static void print_axis(std::ostream & output, const axis_mapping_map & mappings, ev_code_t code){
        axis_mapping_map::const_iterator end = mappings.end();
        axis_mapping_map::const_iterator beg = mappings.begin();
        bool found_already = false;
        output << "[";
        for (axis_mapping_map::const_iterator i = beg; i != end; i++){
            if (i->second.code == code){
                if (found_already){ output << ", "; }
                output << "\"" << get_abs_ev_name_short(i->first) << "\"";
                found_already = true;
            }
        }
        output << "]";
}
        
static void print_running_axes(const axis_mapping_map & mappings, const ev_code_t code){
    {
        if (count_mappings(mappings, code) == 0){
            std::cerr << "Error: no mapping for axis " << get_abs_ev_name_short(code) << " found!" << std::endl;
            exit(EXIT_FAILURE);
        }

        std::cout << "Running with " << get_abs_ev_name_short(code) << " axis mapping ";
        print_axis(std::cout, mappings, code);
        std::cout << std::endl;
    }
}

static int parse_cmdline_args(int argc, char * const * argv, calibrate_config & config){

    typedef std::vector<struct option> opt_vec;
    opt_vec cmdline_opts;
    struct option tmp;
        
    tmp.name = "help";
    tmp.has_arg = 0;
    tmp.flag = NULL;
    tmp.val = 'h';
    cmdline_opts.push_back(tmp);

    tmp.name = "verbose";
    tmp.has_arg = 0;
    tmp.flag = NULL;
    tmp.val = 'v';
    cmdline_opts.push_back(tmp);

    tmp.name = "threshold";
    tmp.has_arg = 1;
    tmp.flag = NULL;
    tmp.val = 't';
    cmdline_opts.push_back(tmp);

    tmp.name = "uuid";
    tmp.has_arg = 1;
    tmp.flag = NULL;
    tmp.val = 'U';
    cmdline_opts.push_back(tmp);;

    tmp.name = "list";
    tmp.has_arg = 0;
    tmp.flag = NULL;
    tmp.val = 'l';
    cmdline_opts.push_back(tmp);;

    tmp.name = "auto";
    tmp.has_arg = 0;
    tmp.flag = NULL;
    tmp.val = 'a';
    cmdline_opts.push_back(tmp);;

    tmp.name = "detect-mappings";
    tmp.has_arg = 0;
    tmp.flag = NULL;
    tmp.val = 'm';
    cmdline_opts.push_back(tmp);;

    tmp.name = "map-y";
    tmp.has_arg = 1;
    tmp.flag = NULL;
    tmp.val = 'y';
    cmdline_opts.push_back(tmp);;

    tmp.name = "map-x";
    tmp.has_arg = 1;
    tmp.flag = NULL;
    tmp.val = 'x';
    cmdline_opts.push_back(tmp);;

    char opt = -1;
    while ((opt = getopt_long(argc, argv, "-dhlamy:x:U:", cmdline_opts.data(), NULL)) != -1){
        switch (opt){
            case 'h': {
                config.mode = calibrate_config::OPMODE_HELP;
                return EXIT_SUCCESS;
                break;
            }
            case 'l': {
                config.mode = calibrate_config::OPMODE_LIST;
                return EXIT_SUCCESS;
                break;
            }
            case 'a': {
                config.mode = calibrate_config::OPMODE_AUTOCONF;
                break;
            }
            case 'm': {
                config.mode = calibrate_config::OPMODE_AXISDETECT;
                break;
            }
            case 'v': {
                config.verbose = true;
                break;
            }
            case 't': {
                double threshold = 0;
                char * endptr = NULL;
                threshold = strtod(optarg, &endptr);
                
                if ((*endptr != 0) && (*endptr != 's')){
                    std::cerr << "Invalid threshold \"" << optarg << "\", has to be either <decimal number>s"
                        << " for seconds or <number> for received samples!!!" << std::endl;
                    return EXIT_FAILURE;
                }

                if (threshold <= 0.0){
                    std::cerr << "Threshold has to be greater than zero!" << std::endl;
                    return EXIT_FAILURE;
                }
 
                if (*endptr == 's'){
                    config.threshold_time = threshold;
                } else {
                    config.threshold_samples = threshold;
                }

                break;
            }
            case 'U': {
                if (uuid_parse(optarg, config.sensor_uuid) != 0){
                    std::cerr << "Invalid UUID \"" << optarg << "\"!" << std::endl;
                    return EXIT_FAILURE;
                }
                break;
            }
            case 'y': {
                clear_mappings(config.axes_mappings, ABS_Y);

                // parse y mapping
                bool mapping_found = false;
                
                const size_t BUFFLEN = strlen(optarg)+1;
                std::vector<char> mapping_buffer(BUFFLEN);
                memset(mapping_buffer.data(), 0, BUFFLEN);
                strcpy(mapping_buffer.data(), optarg);
                
                priority_t priority = ABS_MAX*3;

                // load given mappings
                for (char * token = strtok(mapping_buffer.data(), ","); token != NULL; token = strtok(NULL, ",")){
                    ev_value_t code = get_evcode_for_name(token);
                    if (code != MAPPING_IGNORE_CODE){
                        axis_mapping m;
                        m.code = ABS_Y;
                        m.priority = priority;
                        config.axes_mappings[code] = m;
                        priority--;
                        mapping_found = true;
                    } else {
                        std::cerr << "Unrecognized mapping axis " << token << std::endl;
                        return EXIT_FAILURE;
                    }
                }

                if (mapping_found == false){
                    std::cerr << "No mappings found for the Y axis" << std::endl;
                    return EXIT_FAILURE;
                }
                
                break;
            }
            case 'x': {
                clear_mappings(config.axes_mappings, ABS_X);

                // parse x mapping
                bool mapping_found = false;
                
                const size_t BUFFLEN = strlen(optarg)+1;
                std::vector<char> mapping_buffer(BUFFLEN);
                memset(mapping_buffer.data(), 0, BUFFLEN);
                strcpy(mapping_buffer.data(), optarg);

                priority_t priority = ABS_MAX*3;

                // load given mappings
                for (char * token = strtok(mapping_buffer.data(), ","); token != NULL; token = strtok(NULL, ",")){
                    ev_value_t code = get_evcode_for_name(token);
                    if (code != MAPPING_IGNORE_CODE){
                        axis_mapping m;
                        m.code = ABS_X;
                        m.priority = priority;
                        config.axes_mappings[code] = m;
                        priority--;
                        mapping_found = true;
                    } else {
                        std::cerr << "Unrecognized mapping axis " << token << std::endl;
                        return EXIT_FAILURE;
                    }
                }

                if (mapping_found == false){
                    std::cerr << "No mappings found for the X axis" << std::endl;
                    return EXIT_FAILURE;
                }
                
                break;
            }
            case 1: {
                if (config.device_path.empty()){
                    config.original_device_path = optarg;
                    config.device_path = config.original_device_path;
                    if (!translate_device_path(config.device_path)){
                        std::cerr << "Wrong device path!" << std::endl;
                        return EXIT_FAILURE;
                    }
                } else if (config.screen_name.empty()){
                    config.screen_name = optarg;
                } else if (config.output_file_name.empty()){
                    config.output_file_name = optarg;
                }
            }
        }
    }
    return EXIT_SUCCESS;
}

static void print_device(const ki_device_map::value_type & current_device){
        std::cout << "\t" << current_device.second.desc << std::endl;
        std::cout << "\t\tDevfs: " << current_device.first << std::endl;
        std::cout << "\t\tSysfs: " << current_device.second.sysfs_id << std::endl;
        std::for_each(current_device.second.aliases.begin(), current_device.second.aliases.end(), [](const std::string & alias){
            std::cout << "\t\tAlias: " << alias << std::endl;        
        });
}

void print_output(const x11_output_info & current){
    std::cout << "\t" << current.name << " \t" << current.width << "x" 
        << current.height << " (" << current.x << ", " << current.y << ")" 
        << std::endl;
}

void list(){
    // detect input devices
    ki_device_map devices = detect_devices();
    std::cout << "Available input devices:" << std::endl;
    std::for_each(devices.begin(), devices.end(), print_device);

    // detect screens
    const char * displayname = NULL;
    x11_output_info_list outputs = detect_outputs(displayname);
    std::cout << "Available screens to pair with:" << std::endl;
    std::for_each(outputs.begin(), outputs.end(), print_output);
}

void usage(){
    std::cout << "Usage:" << std::endl;
    std::cout << "mwtouch-calibrate --help                                           \tDisplay help" << std::endl;
    std::cout << "mwtouch-calibrate --list                                           \tLists sensors and screens to pair with" << std::endl;
    std::cout << "mwtouch-calibrate --detect-mappings device                         \tInteractive axis mapping detection" << std::endl;
    std::cout << "mwtouch-calibrate --auto [options] device output [output filename] \tAttempts automatic calibration based on device reported properties" << std::endl;
    std::cout << "mwtouch-calibrate        [options] device output [output filename] \tRuns the interactive calibration and optionaly saves to filename (stdout default)." << std::endl << std::endl;
    
    std::cout << "Options:" << std::endl;
    std::cout << "-y <list-of-axes>, --map-y=<list-of-axes>                          \tMap following axes to the Y axis" << std::endl;
    std::cout << "-x <list-of-axes>, --map-x=<list-of-axes>                          \tMap following axes to the X axis" << std::endl;
}
void help(){
    std::cout << "Calibration utility for the mwtouch input wrapper..." << std::endl << std::endl;
    usage();
}

void axes(std::string device_path){
    const int TIMEOUT = 1000000; // 1s

    int sensor_fd = open_device(device_path.c_str());

    axis_vector xs, ys;

    {
        std::cout << "Attempting to determine the X axis - please drag single finger along the X axis of sensor and then let go." << std::endl;
        axis_vector x_candidates = mwt_evdev_detect_axes(sensor_fd, TIMEOUT);
        std::cout << "Attempting to determine the Y axis - please drag single finger along the Y axis of sensor and then let go." << std::endl;
        axis_vector y_candidates = mwt_evdev_detect_axes(sensor_fd, TIMEOUT);

            xs.reserve(x_candidates.size());
            ys.reserve(y_candidates.size());

        // distribute the loaded axes equaly and uniquely
        axis_vector::const_iterator xsc = x_candidates.begin(); axis_vector::const_iterator xse = x_candidates.end();
        axis_vector::const_iterator ysc = y_candidates.begin(); axis_vector::const_iterator yse = y_candidates.end();

        for (bool keep = true; keep != false; ){
            keep = false;

            // load and store xs
            if (xsc != xse){
                axis_vector::const_iterator iv = std::find(ys.begin(), ys.end(), *xsc);

                // this has not been allocated yet
                if (iv == ys.end()){
                    xs.push_back(*xsc);
                }

                xsc++;
                keep = true;
            }

            // load and store ys
            if (ysc != yse){
                axis_vector::const_iterator yv = std::find(xs.begin(), xs.end(), *ysc);

                // this has not been allocated yet
                if (yv == xs.end()){
                    ys.push_back(*ysc);
                }

                ysc++;
                keep = true;
            }
        }
    }

    std::cout << "Candidates for X axis: ";
    axis_vector::const_iterator end = xs.end();
    axis_vector::const_iterator beg = xs.begin();
    for (axis_vector::const_iterator i = beg; i != end; i++){
        if (i != beg){
            std::cout << ",";
        }
        std::cout << get_abs_ev_name_short(i->value);
    }
    std::cout << " (you can now run the the calibration utility with --map-x=";
    for (axis_vector::const_iterator i = xs.begin(); i != end; i++){
        if (i != beg){
            std::cout << ",";
        }
        std::cout << get_abs_ev_name_short(i->value);
    }
    std::cout << " option)" << std::endl;

    std::cout << "Candidates for Y axis: ";
    end = ys.end();
    beg = ys.begin();
    for (axis_vector::const_iterator i = ys.begin(); i != end; i++){
        if (i != beg){
            std::cout << ",";
        }
        std::cout << get_abs_ev_name_short(i->value);
    }
    std::cout << " (you can now run the the calibration utility with --map-y=";
    for (axis_vector::const_iterator i = ys.begin(); i != end; i++){
        if (i != beg){
            std::cout << ",";
        }
        std::cout << get_abs_ev_name_short(i->value);
    }
    std::cout << " option)" << std::endl;

}

int open_device(const char* device){
    int sensor_fd = -1;

    // open the device node
    sensor_fd = open(device, O_RDONLY);
    if (sensor_fd < 0){
        std::cerr << "Failed to open '" << device << "' for read. ";
        switch (errno){
            case EACCES: {
                std::cerr << "Acces denied.";
                break;
            }
            case ENOENT: {
                std::cerr << "No such file or directory.";
                break;
            }
            case EPERM: {
                std::cerr << "Insufficient permissions.";
                break;
            }
            default: {
                std::cerr << "Errno " << errno << ".";
            }
        }
        std::cerr << std::endl;
    }

    return sensor_fd;
}

bool test_bit(int bit, uint8_t * data){
    int posun = bit/8;
    int bitz = bit%8;
    return (((*(data+posun)) & (uint8_t)(1 << bitz)) == (1 << bitz));
}
void set_bit(int bit, uint8_t * data){
    int posun = bit/8;
    int bitz = bit%8;
    *(data+posun) |= (uint8_t)(1 << bitz);
}

bool calibrate(calibrate_config & ca_config, node_config& config){

    const bool MODE_SAMPLES = (ca_config.threshold_samples != 0)?true:false;
    const int HOLD_THRESHOLD_OK = MODE_SAMPLES?ca_config.threshold_samples:(ca_config.threshold_time * 1000000); // microseconds
    const int HOLD_THRESHOLD_NOK = HOLD_THRESHOLD_OK * 2;
    const int HOLD_THRESHOLD_KO = HOLD_THRESHOLD_OK * 3;

    // init mappings struct
    axis_mapping_map mappings = ca_config.axes_mappings;

    // draw the window
    calibrate_window w(ca_config.x11_output, HOLD_THRESHOLD_OK, HOLD_THRESHOLD_NOK, HOLD_THRESHOLD_KO);
    w.flush();

    // for future use
    Display * disp = w.get_display();

    // select X events to react to
//    XSelectInput(disp, w.getWindow(), XExposeEvent | XMappingEvent | XResizeRequestEvent);

    int point_index = 0;
    struct timeval lasttime;
        lasttime.tv_sec = 0;
        lasttime.tv_usec = 0;

    calibration_point points[4];
    memset(points, 0, sizeof(calibration_point)*4);
    bool running = true;
    bool done = false;
    bool btn_touch_active = false;
    bool previous_active = false;
    float progress = 0;

    while(running) {

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 5000; // 5 miliseconds due to graphics

        fd_set monitors;
        FD_ZERO(&monitors);
        FD_SET(ca_config.sensor_fd, &monitors);

        // select ready for correctly closing version, not realy needed now...
        int rval = select(ca_config.sensor_fd+1, &monitors, NULL, NULL, &timeout);

        struct timeval current;
        if (gettimeofday(&current, NULL) !=  0){
            std::cerr << "gettimeofday failed with errno " << errno << std::endl;
        }

        if (rval != 0){
            if (FD_ISSET(ca_config.sensor_fd, &monitors)){
                if(btn_touch_active != previous_active){
                    memset(points + point_index, 0, sizeof(calibration_point));
                    points[point_index].first = current;
                    points[point_index].samples = 1;
                }
                
                previous_active = btn_touch_active;
                if (process_sensor(points[point_index], ca_config.sensor_fd, btn_touch_active, mappings, ca_config.verbose) != 0){
                    // quit on device failure
                    running = false;
                    continue;
                }
            }
        }

        lasttime = current;
        // if touch is not active, set progress to zero
        float cdx = btn_touch_active?(
            MODE_SAMPLES?points[point_index].samples:timeval_delta(current, points[point_index].first)
        ):0;
        progress = cdx/HOLD_THRESHOLD_KO;
        if (progress > 1){ progress = 1; }

        if ((btn_touch_active != previous_active) && (!btn_touch_active)){

            int dx = MODE_SAMPLES?points[point_index].samples:timeval_delta(lasttime, points[point_index].first);

            // finger removed too soon means go back
            if (dx < HOLD_THRESHOLD_OK){
                if (point_index != 0){
                    point_index--;
                }
                clean_point(points[point_index]);
            // okay
            } else if ((dx >= HOLD_THRESHOLD_OK) && (dx < HOLD_THRESHOLD_NOK)){

                if (point_index == 3){
                    running = false;
                    done = true;
                } else {
                    point_index++;
                }
            // try again
            } else {
                clean_point(points[point_index]);
            }

            progress = 0;
            w.clear();
        }

        // redraw
        if (XPending(disp) > 0){

            XEvent event;
            XNextEvent(disp, &event);

            if (event.type==Expose && event.xexpose.count==0) {
                std::cout << "Exposed" << event.xexpose.x << " " << event.xexpose.y << std::endl;
            }
        }

        w.draw_target_by_index(point_index);
        w.draw_progress(progress);
        w.flush();

    }


    if (done){

        config.axes_mappings = ca_config.axes_mappings;
        
        for (int i = 0; i < 4; i++){
            points[i].x = points[i].average_x;
            points[i].y = points[i].average_y;
        }
        extrude_coordinates(points, calibrate_window::TARGET_OFFSET, ca_config.x11_output);

        config.top_left.x     = points[0].x;
        config.top_left.y     = points[0].y;
        config.bottom_left.x  = points[1].x;
        config.bottom_left.y  = points[1].y;
        config.bottom_right.x = points[2].x;
        config.bottom_right.y = points[2].y;
        config.top_right.x    = points[3].x;
        config.top_right.y    = points[3].y;
    }

    return done;
}

bool autodetect(calibrate_config & ca_config, node_config & config){

    bool done = false;
    int yalv = 0;
    uint8_t abs_b[ABS_MAX/8 + 1];
    memset(abs_b, 0, sizeof(abs_b));

    uint8_t covered_mappings[ABS_MAX/8 + 1];
    memset(covered_mappings, 0, sizeof(covered_mappings));
    
    ioctl(ca_config.sensor_fd, EVIOCGBIT(EV_ABS, sizeof(abs_b)), abs_b);

    // auxiliary to determine whether any of the mapped axes is supported by device
    typedef std::map<ev_code_t, int> mappings_count_map;
    mappings_count_map mappings_count;
    mappings_count[ABS_X] = 0;
    mappings_count[ABS_Y] = 0;

    // auxiliary - used for coordinate matching for points
    axis_mapping_map mappings = ca_config.axes_mappings;
    axis_mapping_map confirmed_mappings;

    priority_t prio = ABS_CNT*3;
    
    calibration_point points[4];
    memset(points, 0, sizeof(calibration_point)*4);

    struct input_absinfo axis_info;
    std::cout << std::endl << "Supported absolute axes:" << std::endl;
    
    //int detected_mappings = 0;

    for (yalv = 0; yalv < ABS_MAX; yalv++) {
        memset(&axis_info, 0, sizeof(struct input_absinfo));

        if (!test_bit(yalv, abs_b)) { continue; }
        
        axis_mapping_map::const_iterator axis_recognized = mappings.find(yalv);
        if (axis_recognized != mappings.end()){
            if (axis_recognized->second.code == MAPPING_IGNORE_CODE){
                std::cout << "Axis " << get_abs_ev_name_short(yalv) << " is set to be ignored" << std::endl;
            } else {
                std::cout << "Axis " << get_abs_ev_name_short(yalv) << " is claimed by mapping to axis " << get_abs_ev_name_short(axis_recognized->second.code) << std::endl;            
            }
        } else {
            std::cout << "Axis " << get_abs_ev_name_short(yalv) << " is not claimed by any mapping!" << std::endl;
            continue;
        }
            
        // take care
        ++mappings_count[axis_recognized->second.code];
        mappings[axis_recognized->second.code].priority = --prio;
        confirmed_mappings.insert(*axis_recognized);
                
        if(ioctl(ca_config.sensor_fd, EVIOCGABS(yalv), &axis_info) < 0) {
            std::cerr << "Getting axis info for axis " << get_abs_ev_name_short(yalv) << "has failed!" << std::endl;
            continue;
        }

        std::cout << "\tRange: <" << axis_info.minimum << "; " << axis_info.maximum << ">" << std::endl;

        switch (axis_recognized->second.code){
            case ABS_X: {
                set_bit(ABS_X, covered_mappings);
                
                // detect axis mapping conflict
                if (points[0].x_priority != 0){
                    mappings_count[ABS_X] = -1;
                }

                if (points[0].x_priority <= mappings[yalv].priority){
                    points[0].x = axis_info.minimum;
                    points[0].x_priority = mappings[yalv].priority;
                    points[1].x = axis_info.minimum;
                    points[2].x = axis_info.maximum;
                    points[3].x = axis_info.maximum;
                }
                break;
            }
            case ABS_Y: {
                set_bit(ABS_Y, covered_mappings);

                // detect axis mapping conflict
                if (points[0].y_priority != 0){
                    mappings_count[ABS_Y] = -1;
                }
                if (points[0].y_priority <= mappings[yalv].priority){
                    points[0].y = axis_info.minimum;
                    points[0].y_priority = mappings[yalv].priority;
                    points[1].y = axis_info.maximum;
                    points[2].y = axis_info.maximum;
                    points[3].y = axis_info.minimum;
                }
                break;
            }                    
        }
    }

    done = (points[0].x_priority != 0) && (points[0].y_priority != 0);

    if (!test_bit(ABS_X, covered_mappings) || !test_bit(ABS_Y, covered_mappings)){
        std::cerr << "Device does not support any of the axes mapped to " << (!test_bit(ABS_Y, covered_mappings)?"X":"Y")
             << " axis. Run mwtouch-calibrate with --detect-mappings argument to determine mappings." << std::endl;
        return false;
    }
    
    if (mappings_count[ABS_X] < 0){
        std::cerr << "Multiple mappings to axis X found, consider removing some mappings to prevent 'virtual' contacts. "
             << "For example, if MT_POSITION_X is mapped, X should be moved to ignore section instead." << std::endl;
    }

    if (mappings_count[ABS_Y] < 0){
        std::cerr << "Multiple mappings to axis Y found, consider removing some mappings to prevent 'virtual' contacts. "
             << "For example, if MT_POSITION_Y is mapped, Y should be moved to ignore section instead." << std::endl;
    }

    if (done){
        config.top_left.x     = points[0].x;
        config.top_left.y     = points[0].y;
        config.bottom_left.x  = points[1].x;
        config.bottom_left.y  = points[1].y;
        config.bottom_right.x = points[2].x;
        config.bottom_right.y = points[2].y;
        config.top_right.x    = points[3].x;
        config.top_right.y    = points[3].y;
        config.axes_mappings = confirmed_mappings;
    }

    return done;
}

int process_sensor(calibration_point & point, int sensor_fd, bool & touch_active, const axis_mapping_map & mappings, const bool & verbose){

    struct input_event data;
    bzero(&data, sizeof(data));

    if (read(sensor_fd, &data, sizeof(struct input_event)) < 0){
        std::cerr << "Unexpected failure reading the device, aborting" << std::endl;
        return 1;
    }

    if (verbose){ format(data); }

    switch (data.type){
        case EV_ABS: {
            // proces EV_ABS

            if (data.code > ABS_MAX){
                std::cerr << "Error, received event code is higher then ABS_MAX" << std::endl;
                break;
            }

            axis_mapping_map::const_iterator mapping_found = mappings.find(data.code);
            if (mapping_found != mappings.end()){
                switch (mapping_found->second.code){
                    case ABS_X: {
                        // commit update
                        if (point.x_priority <= mapping_found->second.priority){
                            point.x_priority = mapping_found->second.priority;
                            point.x = data.value;
                        }
                        break;
                    }
                    case ABS_Y: {
                        // commit update
                        if (point.y_priority <= mapping_found->second.priority){
                            point.y_priority = mapping_found->second.priority;
                            point.y = data.value;
                        }
                        break;
                    }
                    default: {
    //                    std::cerr << "Unhandled EV_ABS/" << data.code << std::endl;
                    }

                }
            }
            break;
        }
        case EV_KEY: {

            // BTN_TOUCH is significant, ignore the rest...
            switch (data.code){
                case BTN_TOUCH: {
                    touch_active = (data.value != 0);
                    break;
                }
                default: {
                    break;
                }
            }

            break;
        }
        case EV_SYN: {
            // Process sync events -
            switch (data.code){
                case SYN_MT_REPORT: {
                    // ignore SYN_MT_REPORT - interrested only in "big" sync
                    break;
                }
                case SYN_REPORT: {
                    // moved to the else part
                    if (point.x_priority != 0){
                        int xc = point.count_x+1;
                        point.average_x = ((point.average_x * point.count_x)+point.x)/xc;
                        point.count_x = xc;
                    }
                    if (point.y_priority != 0){
                        int yc = point.count_y+1;
                        point.average_y = ((point.average_y * point.count_y)+point.y)/yc;
                        point.count_y = yc;
                    }
                    point.x_priority = 0;
                    point.y_priority = 0;
                    break;
                }
                default: {
                    // ignore everything else
                }
            }

            break;
        }
        default: {
            // seriously, are we still reading touch sensor?
            std::cerr << "Unhandled " << data.type << "/" << data.code << std::endl;
            return 0;
        }
    }

    ++point.samples;
    return 0;
}

inline void clean_point(calibration_point & point){
    memset(&point, 0, sizeof(calibration_point));
}

bool extrude_coordinates(calibration_point * points, const unsigned short distance, const x11_output_info output){

    // this function extrudes the sensor read polygon so the distance inner drift is corrected

    // geometry_vectors that denote the borders of inner polygon
    geometry_vect upper(points[3], points[0]);
    geometry_vect left(points[0], points[1]);
    geometry_vect lower(points[1], points[2]);
    geometry_vect right(points[2], points[3]);

    float x_distance_upper = distance;
        x_distance_upper *= upper.norm();
        x_distance_upper /= output.width;
    float x_distance_lower = distance;
        x_distance_lower *= lower.norm();
        x_distance_lower /= output.width;
    float y_distance_left = distance;
        y_distance_left *= left.norm();
        y_distance_left /= output.height;
    float y_distance_right = distance;
        y_distance_right *= right.norm();
        y_distance_right /= output.height;

    // extrude left upper point to left (lu_left) and up (lu_left) (similar to other points)
    geometry_point lu_left = linear_extrude_distance(points[0], upper, x_distance_upper);
    geometry_point lu_upper = linear_extrude_distance(points[0], -left, y_distance_left);

    geometry_point ll_left = linear_extrude_distance(points[1], -lower, x_distance_lower);
    geometry_point ll_lower = linear_extrude_distance(points[1], left, y_distance_left);

    geometry_point ru_right = linear_extrude_distance(points[3], -upper,  x_distance_upper);
    geometry_point ru_upper = linear_extrude_distance(points[3], right, y_distance_right);

    geometry_point rl_right = linear_extrude_distance(points[2], lower, x_distance_lower);
    geometry_point rl_lower = linear_extrude_distance(points[2], -right, y_distance_right);

    geometry_point points_a[] = {lu_upper, ll_left, rl_lower, ru_right};
    geometry_point points_b[] = {lu_left, ll_lower, rl_right, ru_upper};

    geometry_vect aux_upper(ru_upper, lu_upper);
    geometry_vect aux_left(lu_left, ll_left);
    geometry_vect aux_lower(ll_lower, rl_lower);
    geometry_vect aux_right(rl_right, ru_right);

    geometry_vect vectors_a[] = { aux_upper, aux_left, aux_lower, aux_right };
    geometry_vect vectors_b[] = { aux_left, aux_lower, aux_right, aux_upper };

    for (int i = 0; i < 4; i++){

        // test for zero vectors
        if (((vectors_a[i].x == 0) || (vectors_a[i].y == 0)) || ((vectors_b[i].x == 0) || (vectors_b[i].y == 0))){
            // serious error - zero vector found!
            std::cerr << "Found zero vector, that means that two or more points share coordinates!" << std::endl;
            return false;
        }

        // test linearity
        bool linear = false;
        bool vect_a_x_zero = false;
        if (vectors_a[i].x == 0){
            vect_a_x_zero = true;
            // testing the second vector for the same coordinate is sufficient
            if (vectors_b[i].x == 0){
                linear = true;
            }
        } else {
            double t = vectors_b[i].x/vectors_a[i].x;
            if (vectors_b[i].y == (t * vectors_a[i].x)){
                linear = true;
            }
        }
        if (linear){
            std::cerr << "Found lineary dependent vectors, that means that the loaded points do not create quadrangle!" << std::endl;
            return false;
        }

        // special cases
        bool vect_b_y_zero = false;
        if (vect_a_x_zero == true){
            points[i].x = points_b[i].x;
        }
        if (vectors_b[i].y == 0){
            vect_b_y_zero = true;
            points[i].y = points_a[i].y;
        }

        // generic
        if ((!vect_a_x_zero) && (!vect_b_y_zero)){
            double cl = vectors_b[i].y;
                cl *= (points_a[i].x - points_b[i].x);

            double cr = vectors_b[i].x;
                cr *= (points_a[i].y - points_b[i].y);

            double cd = vectors_b[i].x*vectors_a[i].y;
                cd -= vectors_a[i].x*vectors_b[i].y;

            double c = (cl - cr)/cd;

            geometry_point rslt = linear_extrude(points_a[i], vectors_a[i]*c);
            points[i].x = rslt.x;
            points[i].y = rslt.y;

        }

    }
    return true;
}

int main(int argc, char * const * argv){

    calibrate_config config;
    init_mappings(config.axes_mappings);

    if (parse_cmdline_args(argc, argv, config) != EXIT_SUCCESS){
        exit(EXIT_FAILURE);
    }
    
    switch (config.mode){
        case calibrate_config::OPMODE_HELP: {
            help();
            break;
        }
        case calibrate_config::OPMODE_LIST: {
            list();
            break;
        }
        case calibrate_config::OPMODE_AUTOCONF:
        case calibrate_config::OPMODE_CALIBRATE: {
            if (config.screen_name.empty() || config.device_path.empty()){
                std::cerr << "Wrong argument count!" << std::endl;
                usage();
                return EXIT_FAILURE;
            }
            
            // due to
            if (!config.output_file_name.empty()){
                errno = 0;
                config.output_file_fd = open(config.output_file_name.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
                if (config.output_file_fd < 0){
                    std::cerr << "Failed to open '" << config.output_file_name << "' for write. ";

                    switch (errno){
                        case EACCES: {
                            std::cerr << "Acces denied.";
                            break;
                        }
                        case ENOENT: {
                            std::cerr << "No such file or directory.";
                            break;
                        }
                        case EPERM: {
                            std::cerr << "Insufficient permissions.";
                            break;
                        }
                        default: {
                            std::cerr << "Errno " << errno << ".";
                        }
                    }
                    std::cerr << std::endl;

                    return EXIT_FAILURE;
                }
            }
            
            config.sensor_fd = open_device(config.device_path.c_str());
            if (config.sensor_fd < 0){
                std::cerr << "Failed to open given device!" << std::endl;
                if (config.output_file_fd > STDERR_FILENO){ close(config.output_file_fd); }
                return 2;
            }

            config.x11_output = get_output_by_name(config.screen_name.c_str());
            if (config.x11_output.name.empty()){
                std::cerr << "Failed to open selected display device." << std::endl;
                close(config.sensor_fd);
                if (config.output_file_fd > STDERR_FILENO){ close(config.output_file_fd); }
                return 3;
            }

            // setup defaults
            node_config output_config;
            output_config.device_path = config.device_path;
            output_config.target_addr = "localhost:3333";
            output_config.virtual_sensor_height = config.x11_output.height;
            output_config.virtual_sensor_width = config.x11_output.width;
            output_config.x = config.x11_output.x;
            output_config.y = config.x11_output.y;
            if (uuid_is_null(config.sensor_uuid)){
                uuid_clear(output_config.uuid);
                uuid_generate(output_config.uuid);
            } else {
                uuid_copy(output_config.uuid, config.sensor_uuid);
            }

            // prints x mappings
            print_running_axes(config.axes_mappings, ABS_X);
            // prints y mappings
            print_running_axes(config.axes_mappings, ABS_Y);

            bool config_done = false;
            switch (config.mode){
                case calibrate_config::OPMODE_AUTOCONF: {
                    config_done = autodetect(config, output_config);
                    break;
                }
                default: {
                    config_done = calibrate(config, output_config);
                }
            }
            
            if (config_done){
                output_config.device_path = config.original_device_path;
                write_config(output_config, config.output_file_fd);
            }
            
            if (config.output_file_fd > STDERR_FILENO){ 
                close(config.output_file_fd); 
            }

            break;
        }
        case calibrate_config::OPMODE_AXISDETECT: {

            if (!config.device_path.empty()){
                axes(config.device_path);
            } else {
                std::cerr << "Wrong argument count!" << std::endl;
                usage();
                return EXIT_FAILURE;
            }

            break;
        }
    }

    return EXIT_SUCCESS;
}

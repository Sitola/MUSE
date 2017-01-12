/**
 * \file      mwkinect.cpp
 * \brief     The main mwkinect wrapper encapsuling program
 * \author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-08-07 20:09 UTC+2
 * \copyright BSD
 */

#include <libfreenect/libfreenect.h>


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <cstring>
#include <sstream>
#include <algorithm>
#include <libfreenect.hpp>
#include <opencv2/opencv.hpp>

#include "nodeconfig.hpp"
#include "wrapper.hpp"
#include "kinect_device.hpp"
#include "gtk_ui.hpp"

static void handle_kill_signal(int ev);

static void print_usage(){
    std::cout << "TUIO 2.0 wrapper for the MicroSoft Kinect depth sensor" << std::endl << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "\tmwkinect [options] configfile" << std::endl;
    std::cout << std::endl << std::endl;

    std::cout << "Options:" << std::endl;
    std::cout << "-h, --help                        \n\tShows this help" << std::endl;
    std::cout << "-v, --verbose                     \n\tIncrease verbosity level" << std::endl;
    std::cout << "-t <address>, --target=<address>  \n\tOverride the target address set in config file" << std::endl;
    std::cout << "-p, --no-pid                      \n\tDisable single-instance on device lock" << std::endl;
    std::cout << "-i, --cli                         \n\tDo not run with gui (todo)" << std::endl;
    std::cout << "-g, --gui                         \n\tRun with gui (todo)" << std::endl;
    std::cout << "-l, --list                        \n\tList available devices" << std::endl;
    std::cout << "-c, --calibration                 \n\tLaunch calibration gui" << std::endl;
    std::cout << "-V, --video-mode=<WIDTHxHEIGHT:FPS>\n\tSet video mode to run with (see list for options)" << std::endl;
    std::cout << "-D, --depth-mode=<WIDTHxHEIGHT:FPS>\n\tSet depth mode to run with (see list for options)" << std::endl;
    std::cout << std::endl;
}

static void register_signal_handlers(){

    signal(SIGTERM, handle_kill_signal);
    // NOTE: SIGKILL is still uncaughtable
    //signal(SIGKILL, handle_kill_signal); // can't do anything about it, so at least correctly close
    signal(SIGABRT, handle_kill_signal);
    signal(SIGINT, handle_kill_signal);
    signal(SIGHUP, handle_kill_signal);

}

static void handle_kill_signal(int ev){
    std::cout << "Caught signal " << ev << ", closing up..." << std::endl;
    node_config::get_instance()->thread_com_channel.keep_running = false; // this variable is checked in the main event loop
}

static void print_identity(const node_config & config){
    std::cout << "Running as " << config.app_name << ":" << libkerat::ipv4_to_str(config.local_ip) << "/" << config.instance << std::endl;
}

static int device_setup(node_config & config){

    int err = freenect_init(&config.kinect_context, NULL);
    if ((err != 0) || (config.kinect_context == NULL)){
        std::cerr << "Failed to initialize freenect context!" << std::endl;
        return -1;
    }

    try {
        config.device = new kinect_device(config);
    } catch (const std::exception & ex){
        std::cerr << ex.what() << std::endl;

        freenect_shutdown(config.kinect_context);
        config.kinect_context = NULL;

        return 1;
    }

    size_t retry_count = 0;
    // init command channel
    errno = 0;
    while (pthread_spin_init(&config.thread_com_channel.update_lock, PTHREAD_PROCESS_PRIVATE) != 0) {
        switch (errno){
            case EAGAIN: {
                if (retry_count < 15){
                    ++retry_count;
                    std::cerr << "Failed to create lock, using random backoff..." << std::endl;
                    usleep((rand()%200)*1000);
                    continue;
                } else {
                    std::cerr << "Too many failures during command lock aquire phase, giving up..." << std::endl;
                    return 1;
                }
            }
            default: {
                perror("Failed to create command lock");
                return 1;
            }
        }
    }

    sleep(1);

    // ok, so we have device, time to select modes
    if (!config.device->set_video_mode(config.kinect_camera_format, config.kinect_camera_resolution)) {
        std::cerr << "Unable to set video mode!" << std::endl;
        return 1;
    }
    if (!config.device->set_depth_mode(config.kinect_depth_format, config.kinect_depth_resolution)) {
        std::cerr << "Unable to set depth mode!" << std::endl;
        return 1;
    }
    

    // from now on, consider kinect configured

    return 0;
}

static int device_run(node_config & config){

    // initialize the core
    kinect_wrapper wrapper_core(config);

    config.device->start();
    bool wrapper_state_change = false;

    while (config.thread_com_channel.keep_running){
        // aquire the command lock
        cv::Mat rgb_matrix, depth_matrix, preview_matrix;
        config.device->process();

        pthread_spin_lock(&config.thread_com_channel.update_lock);
        if (config.force_update) { wrapper_core.apply_config(config); }
        pthread_spin_unlock(&config.thread_com_channel.update_lock);

        if (config.device->get_camera_data(rgb_matrix, depth_matrix)) {
            bool blob_detected = wrapper_core.process(rgb_matrix, depth_matrix, preview_matrix);
            if (blob_detected != wrapper_state_change){
                config.device->set_led(blob_detected?LED_GREEN:LED_YELLOW);
            }
            wrapper_state_change = blob_detected;

            if (config.thread_com_channel.gui_instance != NULL) {
                config.thread_com_channel.gui_instance->update_preview(preview_matrix);
            }
        }
    }

    config.device->stop();

    return 0;
}

static int get_runtime_config(node_config & runtime_config, int argc, char ** argv){
    //runtime_config.disable_transformations = false;
    runtime_config.verbosity = 0;
    runtime_config.disable_pidfile = false;
    runtime_config.thread_com_channel.argc = argc;
    runtime_config.thread_com_channel.argv = argv;

    if (argc < 2){
        std::cerr << "Invalid argument count!" << std::endl;
        print_usage();
        return EXIT_FAILURE;
    }

    const size_t CMDLINE_ARGC = 13;
    struct option cmdline_opts[CMDLINE_ARGC];
    memset(&cmdline_opts, 0, sizeof(cmdline_opts));
    {
        cmdline_opts[0].name = "help";
        cmdline_opts[0].has_arg = 0;
        cmdline_opts[0].flag = NULL;
        cmdline_opts[0].val = 'h';

        cmdline_opts[1].name = "verbose";
        cmdline_opts[1].has_arg = 0;
        cmdline_opts[1].flag = NULL;
        cmdline_opts[1].val = 'v';

        cmdline_opts[2].name = "no-pid";
        cmdline_opts[2].has_arg = 0;
        cmdline_opts[2].flag = NULL;
        cmdline_opts[2].val = 'p';

        cmdline_opts[3].name = "target";
        cmdline_opts[3].has_arg = 1;
        cmdline_opts[3].flag = NULL;
        cmdline_opts[3].val = 't';

        cmdline_opts[4].name = "cli";
        cmdline_opts[4].has_arg = 0;
        cmdline_opts[4].flag = NULL;
        cmdline_opts[4].val = 'i';

        cmdline_opts[5].name = "gui";
        cmdline_opts[5].has_arg = 0;
        cmdline_opts[5].flag = NULL;
        cmdline_opts[5].val = 'g';

        cmdline_opts[6].name = "list";
        cmdline_opts[6].has_arg = 0;
        cmdline_opts[6].flag = NULL;
        cmdline_opts[6].val = 'l';

        cmdline_opts[7].name = "device";
        cmdline_opts[7].has_arg = 1;
        cmdline_opts[7].flag = NULL;
        cmdline_opts[7].val = 'd';

        cmdline_opts[8].name = "calibration";
        cmdline_opts[8].has_arg = 0;
        cmdline_opts[8].flag = NULL;
        cmdline_opts[8].val = 'c';

        cmdline_opts[9].name = "video-mode";
        cmdline_opts[9].has_arg = 1;
        cmdline_opts[9].flag = NULL;
        cmdline_opts[9].val = 'V';

        cmdline_opts[10].name = "depth-mode";
        cmdline_opts[10].has_arg = 1;
        cmdline_opts[10].flag = NULL;
        cmdline_opts[10].val = 'D';

        cmdline_opts[11].name = "no-transform";
        cmdline_opts[11].has_arg = 0;
        cmdline_opts[11].flag = NULL;
        cmdline_opts[11].val = 'n';

    }

    char opt = -1;
    while ((opt = getopt_long(argc, argv, "-hvt:piglcd:V:D:n", cmdline_opts, NULL)) != -1){
        switch (opt){
            case 'h': {
                //runtime_config.thread_com_channel.keep_running = false;
                runtime_config.operation_mode = node_config::MODE_HELP;
                return EXIT_SUCCESS;
                break;
            }
            case 'v': {
                runtime_config.verbosity = -1;
                std::cout << "Verbose mode requested..." << std::endl;
                break;
            }
            case 't': {
                if (optarg[0] == '\0'){
                    std::cerr << "An empty target address cannot be set!" << std::endl;
                    return EXIT_FAILURE;
                }

                std::cout << "Overriding target to: " << optarg << std::endl;
                runtime_config.target_addr = optarg;

                break;
            }
            case 'p': {
                runtime_config.disable_pidfile = true;
                break;
            }
            case 'i': {
                runtime_config.ui = node_config::UI_CONSOLE;
                break;
            }
            case 'g': {
                runtime_config.ui = node_config::UI_GTK;
                break;
            }
            case 'd': {
                runtime_config.device_serial = optarg;
                break;
            }
            case 'l': {
                runtime_config.operation_mode = node_config::MODE_LIST;
                return EXIT_SUCCESS;
                //break;
            }
            case 'c': {
                runtime_config.operation_mode = node_config::MODE_CALIBRATION;
                break;
            }
            case 'V': {
                if (!nc_parse_video_mode(optarg, runtime_config.kinect_camera_format, runtime_config.kinect_camera_resolution)){
                    std::cerr << "Invalid video mode \"" << optarg << "\"!" << std::endl;
                    return EXIT_FAILURE;
                }
                break;
            }
            case 'D': {
                if (!nc_parse_depth_mode(optarg, runtime_config.kinect_depth_format, runtime_config.kinect_depth_resolution)){
                    std::cerr << "Invalid depth mode \"" << optarg << "\"!" << std::endl;
                    return EXIT_FAILURE;
                }
                break;
            }
            case 1: {
                runtime_config.config_path = optarg;
            }
        }
    }

    if (runtime_config.config_path.empty()){
        std::cerr << "Configuration file not speciffied!" << std::endl;
        return EXIT_FAILURE;
    }

    bool load_successfull =
        load_config_file(runtime_config)
        && complete_config(runtime_config);

    return load_successfull?EXIT_SUCCESS:EXIT_FAILURE;
}

static void list_devices(){
    freenect_context * kinect_context = NULL;

    int err = freenect_init(&kinect_context, NULL);
    if (err != 0){
        std::cerr << "Failed to initialize freenect context!" << std::endl;
        return;
    }

    assert(kinect_context != NULL);

    freenect_device_attributes * attributes = NULL;
    const int devices_count = freenect_list_device_attributes(kinect_context, &attributes);

    std::cout << "Found " << devices_count << " devices..." << std::endl;

    freenect_device_attributes * current_attribute = attributes;
    for (int i = 0; (i < devices_count) && (current_attribute != NULL); ++i){
        std::cout << "Device " << i << ": S/N: " << current_attribute->camera_serial << std::endl;

        kinect_device_config tmp_config;
        tmp_config.set_defaults();
        tmp_config.kinect_context = kinect_context;
        tmp_config.transform_input_image = false;
        tmp_config.device_serial = current_attribute->camera_serial;

        kinect_device tmp_device(tmp_config);

        bool valid_rgb_mode = false;
        // print supported video modes
        std::cout << "Video (type:resolution:framerate): " << std::endl;
        kinect_device::frame_mode_vector modes = tmp_device.get_video_modes();
        for (kinect_device::frame_mode_vector::iterator mode = modes.begin(); mode != modes.end(); ++mode){

            switch (mode->video_format){
                case FREENECT_VIDEO_RGB:
                case FREENECT_VIDEO_IR_10BIT:
                case FREENECT_VIDEO_IR_8BIT:
                {
                    valid_rgb_mode = true;
                    break;
                }
                default: continue;
            }
            std::cout << "\t" << nc_unparse_video_mode(mode->video_format, mode->width, mode->height, mode->framerate) << std::endl;
        }

        // print supported rgb modes
        bool valid_depth_mode = false;
        std::cout << "Depth (type:resolution:framerate): " << std::endl;
        modes = tmp_device.get_depth_modes();
        for (kinect_device::frame_mode_vector::iterator mode = modes.begin(); mode != modes.end(); ++mode){
            switch (mode->depth_format){
                case FREENECT_DEPTH_10BIT:
                case FREENECT_DEPTH_11BIT:
                case FREENECT_DEPTH_REGISTERED: {
                    valid_depth_mode = true;
                    break;
                }
                default: continue;
            }
            std::cout << "\t" << nc_unparse_depth_mode(mode->depth_format, mode->width, mode->height, mode->framerate) << std::endl;
        }

        // consider device "self-introduction" by led blinking
        if (valid_depth_mode && valid_rgb_mode){
            tmp_device.set_led(LED_GREEN);
        } else {
            std::cerr << "Either video or depth modes for this device are incompatible with mwkinect." << std::endl;
            tmp_device.set_led(LED_RED);
        }
        usleep(750000);
        tmp_device.set_led(LED_OFF);

        std::cout << std::endl;

        current_attribute = current_attribute->next;
    }

    freenect_free_device_attributes(attributes);
    attributes = NULL;

    freenect_shutdown(kinect_context);
}

static bool pidfile_check(const node_config & config){
    if (config.disable_pidfile){
        std::cout << "Process id check disabled!" << std::endl;
        return true;
    }

    int old_pid = 0;
    int pidfile_fd = -1;
    bool can_run = false;

    const size_t PIDBUFFSIZE = 20;
    char buffer[PIDBUFFSIZE];
    memset(buffer, 0, PIDBUFFSIZE);

    // should be handeled elsewhere
//    if (!can_run){
//        std::cerr << "Unable to recovery from previous failures, exitting." << std::endl;
//        std::cerr << "See mwkinect --help for disabling the pidfile check." << std::endl;
//        return false;
//    }

    errno = 0;
    pidfile_fd = open(config.pidfile_name.c_str(), O_RDONLY);
    if (pidfile_fd < 0){
        if (errno == ENOENT){ return true; }
        goto perror_close;
    }

    { // read & scann
        errno = 0;
        int bytes_read = read(pidfile_fd, buffer, PIDBUFFSIZE);
        if ((bytes_read == 0) && (errno != 0)){ goto perror_close; }

        char * endptr = NULL;
        old_pid = strtol(buffer, &endptr, 10);
        if ((endptr != NULL) && (*endptr != '\n')){ goto perror_close; }
    }

    { // chekc process state
        errno = 0;
        if (kill(old_pid, 0) != 0){
            if (errno == ESRCH){
                can_run = true;
            }
        }
    }

    if (!can_run){
        std::cerr << "Wrapper is already running on this device! " << std::endl;
        std::cerr << "Remove the pid file '" << config.pidfile_name << "' or disable this check." << std::endl;
        goto error_close;
    }

    close(pidfile_fd);
    return true;

perror_close:
    perror("Failed to open/read pid file");
error_close:
    std::cerr << "See mwkinect --help for disabling the pidfile check." << std::endl;
    if (pidfile_fd >= 0){ close(pidfile_fd); }
    return false;
}

static bool pidfile_unlock(const node_config & config){
    if (config.disable_pidfile){ return true; }

    if (!config.disable_pidfile){
        unlink(config.pidfile_name.c_str());
    }

    return true;
}

static bool pidfile_lock(const node_config & config){
    if (config.disable_pidfile){ return true; }

    int pidfile_fd = open(config.pidfile_name.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0700);
    if (pidfile_fd < 0){ goto perror_close; }

    char buffer[20];
    sprintf(buffer, "%d\n", getpid());

    errno = 0;
    if (write(pidfile_fd, buffer, strlen(buffer)) < (int)strlen(buffer)){ goto perror_close; }

    close(pidfile_fd);
    return true;

perror_close:
    perror("Failed to open/write pid file");
//error_close:
    std::cerr << "See mwkinect --help for disabling the pidfile check." << std::endl;
    if (pidfile_fd >= 0){ close(pidfile_fd); }
    return false;

}

int run_wrapper(node_config & runtime_config){
    int retval = EXIT_SUCCESS;
    
    // create locks
    if (!(pidfile_check(runtime_config) && pidfile_lock(runtime_config))){
        runtime_config.thread_com_channel.keep_running = false;
        std::cerr << "Way too many errors to continue have occured!" << std::endl;
    }

    if (!runtime_config.thread_com_channel.keep_running){ return EXIT_SUCCESS; }

    errno = 0;
    print_identity(runtime_config);

    // prepare device
    retval = device_setup(runtime_config);
    if (retval != 0){ return EXIT_FAILURE; }

    // init gui
    if (runtime_config.ui == 1){
        pthread_create(&runtime_config.thread_com_channel.gui_thread, NULL, &mwkinect_gui_thread_start, &runtime_config.thread_com_channel);
    }

    // run
    retval = device_run(runtime_config);
    if (runtime_config.ui == 1){
        pthread_join(runtime_config.thread_com_channel.gui_thread, NULL);
    }

    pidfile_unlock(runtime_config);

    return retval;
}

int run_calibration(node_config & runtime_config){
    int retval = EXIT_SUCCESS;

    // running calibration, disable input transformations    
    runtime_config.transform_input_image = false;
    
    if (!runtime_config.thread_com_channel.keep_running){ return EXIT_SUCCESS; }
    errno = 0;

    // prepare device
    retval = device_setup(runtime_config);
    if (retval != 0){ return EXIT_FAILURE; }
    
    // init gui
#if 0    
    if (runtime_config.ui == 1){
        pthread_create(&runtime_config.thread_com_channel.gui_thread, NULL, &mwkinect_gui_thread_start, &runtime_config.thread_com_channel);
    }
#endif

#if 0    
    // run
    retval = device_calibrate(runtime_config);
    if (runtime_config.ui == 1){
        pthread_join(runtime_config.thread_com_channel.gui_thread, NULL);
    }
#endif
    
    return retval;
}

int main(int argc, char ** argv){
    int retval = EXIT_SUCCESS;

    if (getuid() == 0){
        std::cerr << "Yeah, like I'm gonna run with root privileges... I don't think so..." << std::endl;
        return EXIT_FAILURE;
    }
    node_config * runtime_config = node_config::get_instance();
    runtime_config->set_defaults();

    if (get_runtime_config(*runtime_config, argc, argv) != 0){
        std::cerr << "Unable to recovery from previous failures, quiting." << std::endl;
        return EXIT_FAILURE;
    }

    // from now on, we're sensitive about how we're treated considering signals
    register_signal_handlers();
    if (!(runtime_config->thread_com_channel.keep_running)){ exit(EXIT_SUCCESS); }

    switch (runtime_config->operation_mode) {
        case node_config::MODE_LIST: {
            list_devices();
            break;
        }
        case node_config::MODE_HELP: {
            print_usage();
            break;
        }
        case node_config::MODE_CALIBRATION: {
            retval = run_calibration(*runtime_config);
            break;
        }
        default: {
            retval = run_wrapper(*runtime_config);
        }
    }

    node_config::destroy();

    return retval;
}

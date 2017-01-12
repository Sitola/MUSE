/**
 * @file      mwtouch.cpp
 * @brief     The main mwtouch wrapper encapsuling program
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2011-03-17 13:28 UTC+2
 * @copyright BSD
 */

#include <stdlib.h>
#include <errno.h>
#include <linux/input.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <endian.h>
#include <signal.h>
#include <getopt.h>
#include <stdio.h>

#include <cstring>
#include <iostream>
#include <algorithm>

#include "event.hpp"
#include "common.hpp"
#include "eventdumper.hpp"
#include "event_storage.hpp"
#include "wrapper.hpp"
#include "device.hpp"

bool running = true;
const struct timeval TIMEOUT_WAIT = {1, 0};

int main(int argc, char ** argv);
//static int16_t button_to_type_id(int button);
static void print_usage();
static void register_signal_handlers();

/**
 * Sets the event loop stop flag
 * @param ev - do not care about this (viz. man signal)
 */
static void handle_kill_signal(int ev);

static void print_usage(){
    std::cout << "TUIO 2.0 wrapper for the Linux kernel input layer" << std::endl << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "\tmwtouch [options] configfile" << std::endl;
    std::cout << "\tmwtouch [options] -" << std::endl << std::endl;
    std::cout << "The wrapper can run upon either the device node, or the captured events file."
        "This mode is detected automaticcaly using the stat() syscall. "
        "If \"-\" is provided instead of config file path, the configuration is expected on stdin." << std::endl;

    std::cout << std::endl << std::endl;

    std::cout << "Options:" << std::endl;
    std::cout << "-h, --help                        \n\tShows this help" << std::endl << std::endl;
    std::cout << "-T, --disable-transformations     \n\tDisable coordinate transformations (does not affect axis mappings)" << std::endl << std::endl;
    std::cout << "-v, --verbose                     \n\tIncrease verbosity level" << std::endl << std::endl;
    std::cout << "-d <sec>, --delay=<sec>           \n\tSets the initial delay for replay mode" << std::endl << std::endl;
    std::cout << "-o <file>, --output=<file>        \n\tWrite received events to file (can be used for replay)" << std::endl << std::endl;
    std::cout << "-D <path>, --device=<path>        \n\tOverride the device path set in config file" << std::endl << std::endl;
    std::cout << "-t <address>, --target=<address>  \n\tOverride the target address set in config file" << std::endl << std::endl;

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
    running = false; // this variable is checked in the main event loop
}

static void print_identity(const node_config & config){
    std::cout << "Running as " << config.app_name << ":" << libkerat::ipv4_to_str(config.local_ip) << "/" << config.instance << std::endl;
}

static int device_setup(node_config & config, int source_fd){
    // device specific setup
    config.axes_ranges = mwt_evdev_get_supported_axes(source_fd);
    return 0;
}
static int device_run(const node_config & config, int source_fd){
   
    // initialize the core
    kinput_wrapper wrapper_core(config);

    int process_status = 0; // 0 denotes empty sync (MT_TYPE_A)

    // open the store file (if set)
    int store_fd = -1;
    errno = 0;
    if (!config.store_path.empty()){
        if ((store_fd = open(config.store_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
            std::cerr << "Output file " << config.store_path << ": open failed. Errno: " << errno << ", " << strerror(errno) << std::endl;
            return EXIT_FAILURE;
        }
    }
    
    // write header and axes records
    if (store_fd > 0){ 
        mwt_storage_file_write_header(store_fd); 
        mwt_storage_file_write_axis_map(store_fd, config.axes_ranges);
    }

    // setup grabbing
    struct input_event data;
    bzero(&data, sizeof(data));

    // to store data during cycle

    // timed wait is used to monitor events
    timeval timeout;

    timeout = TIMEOUT_WAIT;

    while (running){

        fd_set monitors;
        FD_ZERO(&monitors);
        FD_SET(source_fd, &monitors);
/*
        fd_set errors;
        FD_ZERO(&errors);
        FD_SET(source_fd, &errors);
*/
        // select ready for correctly closing version, not realy needed now...
        int rval = select(source_fd+1, &monitors, NULL, NULL, &timeout);

        // implicit behaviour - no event is expected
        timeout = TIMEOUT_WAIT;

        // for some reason select tends to fail - this should hack around it
        if ((rval > 0) && (FD_ISSET(source_fd, &monitors))){

           int units_read = read(source_fd, &data, sizeof(struct input_event));

            if (units_read > 0){

                if ((config.verbosity & VERBOSITY_LEVEL_DUMP) == VERBOSITY_LEVEL_DUMP){
                    format(data);
                }

                process_status = wrapper_core.process_event(&data);

                if (process_status == -1){
                    // SYN_DROP occured, reset the device
                    int capabilities = 0;
                    if(ioctl(source_fd, EVIOCGBIT(0, sizeof(capabilities)), &capabilities) < 0) {
                        std::cerr << "evdev buffer underrun detected, SYN_DROPPED" << std::endl;
                        continue;
                    }
                }

                if (store_fd > 0){ mwt_storage_file_write_event(store_fd, data); }
            }

            bzero(&data, sizeof(data));
        } else { // end of select
            // just for MT type A
            // only if empty sync arrived
            // and only if no touch present
            // clean the buffer

            if ((process_status == 0) && (wrapper_core.get_type() == kinput_wrapper::MULTITOUCH_TYPE_A) && (wrapper_core.is_empty())){
                wrapper_core.set_empty();
                wrapper_core.force_commit();
            }
        }
    }

    if (store_fd > 0){ close(store_fd); }

    return 0;
}

static int store_run(const node_config & config, int source_fd){

    int retval = 0;
    kinput_wrapper wrapper_core(config);

    struct timeval previous_original_launch;
        previous_original_launch.tv_sec = 0;
        previous_original_launch.tv_usec = 0;
    struct timeval drift;
        drift.tv_sec = 0;
        drift.tv_usec = 0;

    int process_status = 0;

    while (running){
        struct input_event inative;
        int status = mwt_storage_file_read_event(source_fd, inative);
        
        // test eof
        if (status != 0){ running = false; continue; }

        struct timeval sleep;
        struct timeval original_launch = inative.time;
        struct timeval scheduled;

        // safe wait for first event
        if ((previous_original_launch.tv_sec != 0) && (previous_original_launch.tv_usec != 0)){
            timersub(&original_launch, &previous_original_launch, &sleep);
            if (timercmp(&sleep, &drift, >)){
                struct timeval tmp;
                timersub(&sleep, &drift, &tmp);
                sleep = tmp;
            } else {
                sleep.tv_sec = 0;
                sleep.tv_usec = 0;
            }
        } else {
            sleep = config.delay;
        }

        if (gettimeofday(&scheduled, NULL) != 0){
            scheduled.tv_sec = 0;
            scheduled.tv_usec = 0;
        }

        previous_original_launch = original_launch;

        // device timeout emulation
        if (!timercmp(&sleep, &TIMEOUT_WAIT, <)){
            struct timespec rem; rem.tv_sec = 0; rem.tv_nsec = 0;
            struct timespec slp; slp.tv_sec = TIMEOUT_WAIT.tv_sec; slp.tv_nsec = TIMEOUT_WAIT.tv_usec * 1000;

            while (nanosleep(&slp, &rem)!=0){
                slp = rem;
                rem.tv_sec = 0; rem.tv_nsec = 0;
            }
            if ((process_status == 0) && (wrapper_core.get_type() == kinput_wrapper::MULTITOUCH_TYPE_A) && (wrapper_core.is_empty())){
                wrapper_core.set_empty();
                wrapper_core.force_commit();
            }

            timersub(&sleep, &TIMEOUT_WAIT, &drift);
            sleep = drift;

            rem.tv_sec = 0; rem.tv_nsec = 0;
            slp.tv_sec = sleep.tv_sec; slp.tv_nsec = sleep.tv_usec * 1000;
            while (nanosleep(&slp, &rem)!=0){
                slp = rem;
                rem.tv_sec = 0; rem.tv_nsec = 0;
            }
        } else {
            // no need to emulate timeout
            struct timespec rem; rem.tv_sec = 0; rem.tv_nsec = 0;
            struct timespec slp; slp.tv_sec = sleep.tv_sec; slp.tv_nsec = sleep.tv_usec * 1000;

            while (nanosleep(&slp, &rem)!=0){
                slp = rem;
                rem.tv_sec = 0; rem.tv_nsec = 0;
            }

            // test for stop during sleep
            if (!running){ continue; }

            // process loaded data
            {
                if ((config.verbosity & VERBOSITY_LEVEL_DUMP) == VERBOSITY_LEVEL_DUMP){
                    format(inative);
                }

                process_status = wrapper_core.process_event(&inative);
                // ignore SYN_DROPPED on file
            }
        }

        // compute correction for next step
        struct timeval processed;
        if ((scheduled.tv_sec != 0) && (scheduled.tv_usec != 0)
            && ((gettimeofday(&processed, NULL) != 0))
            && (timercmp(&processed, &scheduled, >)))
        {
            timersub(&processed, &scheduled, &drift);
        }
    }
    
    return retval;
}

static int store_setup(node_config & config, int source_fd){
    int retval = 0;
    // runner
    if (!mwt_storage_file_test_header(source_fd)){
        running = false;
        retval = -1;
        std::cerr << "The file provided does not have proper data format! "
            "Is this really a mwtouch dump file?" << std::endl;
    }
    
    if ((retval == 0) && (mwt_storage_file_read_axis_map(source_fd, config.axes_ranges) != 0)){
        running = false;
        retval = -1;
        std::cerr << "The file provided does not have proper data format! "
            "Is this really a mwtouch dump file?" << std::endl;
    }

    return retval;
}

static int get_runtime_config(node_config & runtime_config, int argc, char * const * argv){
    
    // defaults
    runtime_config.disable_transformations = false;
    runtime_config.delay.tv_sec = 0; runtime_config.delay.tv_usec = 0;
    runtime_config.verbosity = 0;
    runtime_config.disable_pidfile = false;
    runtime_config.config_path = "";

    if (argc < 2){
        std::cerr << "Invalid argument count!" << std::endl;
        print_usage();
        return EXIT_FAILURE;
    }

    const size_t CMDLINE_ARGC = 9;
    struct option cmdline_opts[CMDLINE_ARGC];
    memset(&cmdline_opts, 0, sizeof(cmdline_opts));
    { size_t index = 0;

        cmdline_opts[index].name = "help";
        cmdline_opts[index].has_arg = 0;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'h';
        ++index;

        cmdline_opts[index].name = "verbose";
        cmdline_opts[index].has_arg = 0;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'v';
        ++index;

        cmdline_opts[index].name = "delay";
        cmdline_opts[index].has_arg = 1;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'd';
        ++index;

        cmdline_opts[index].name = "disable-transformations";
        cmdline_opts[index].has_arg = 1;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'T';
        ++index;

        cmdline_opts[index].name = "no-pid";
        cmdline_opts[index].has_arg = 0;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'p';
        ++index;

        cmdline_opts[index].name = "output";
        cmdline_opts[index].has_arg = 1;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'o';
        ++index;

        cmdline_opts[index].name = "device";
        cmdline_opts[index].has_arg = 1;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'D';
        ++index;

        cmdline_opts[index].name = "target";
        cmdline_opts[index].has_arg = 1;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 't';
        ++index;

        assert(CMDLINE_ARGC > index);
    }

    char opt = -1;
    while ((opt = getopt_long(argc, argv, "-hvd:To:D:t:p", cmdline_opts, NULL)) != -1){
        switch (opt){
            case 'h': {
                print_usage();
                running = false;
                return EXIT_FAILURE;
                break;
            }
            case 'T': {
                runtime_config.disable_transformations = true;
                std::cout << "Disabling coordinate transformations" << std::endl;
                break;
            }
            case 'v': {
                runtime_config.verbosity = -1;
                std::cout << "Verbose mode requested..." << std::endl;
                break;
            }
            case 'd': {
                double del = strtod(optarg, NULL);
                std::cout << "Setting delay before attempting to send the first event to: " << del << " seconds" << std::endl;
                runtime_config.delay.tv_sec = del;
                runtime_config.delay.tv_usec = (del-runtime_config.delay.tv_sec)*1000000;
                break;
            }
            case 'D': {
                runtime_config.device_path = optarg;
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
            case 'o': {
                runtime_config.store_path = optarg;
                break;
            }
            case 'p': {
                runtime_config.disable_pidfile = true;;
                break;
            }
            case 1: {
                runtime_config.config_path = optarg;
            }
        }
    }

    // assume correct argument count from now
    if (runtime_config.config_path.empty()){
        std::cerr << "Configuration file not speciffied!" << std::endl;
        return EXIT_FAILURE;
    }

    bool load_successfull = 
        load_config_file(runtime_config)
        && complete_config(runtime_config);
    
    return load_successfull?EXIT_SUCCESS:EXIT_FAILURE;

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
//        std::cerr << "See mwtouch --help for disabling the pidfile check." << std::endl;
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
        if ((endptr != buffer+strlen(buffer)) && (*endptr != '\n')){ goto perror_close; }
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
    std::cerr << "See mwtouch --help for disabling the pidfile check." << std::endl;
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
    { // stupid goto complaints
        int len = strlen(buffer);
        int written = write(pidfile_fd, buffer, strlen(buffer));
        if (written < len){ goto perror_close; }
    }
    
    close(pidfile_fd);
    return true;

perror_close:    
    perror("Failed to open/write pid file");
//error_close:    
    std::cerr << "See mwtouch --help for disabling the pidfile check." << std::endl;
    if (pidfile_fd >= 0){ close(pidfile_fd); }
    return false;
    
}

int main(int argc, char ** argv){
    int retval = EXIT_SUCCESS;
    
    if (getuid() == 0){
        std::cerr << "Yeah, like I'm gonna run with root privileges... I don't think so..." << std::endl;
        return EXIT_FAILURE;
    }
    node_config runtime_config;

    if (get_runtime_config(runtime_config, argc, argv) != 0){
        std::cerr << "Unable to recovery from previous failures, quiting." << std::endl;
        return EXIT_FAILURE;
    }

    // get real device
    if (!translate_device_path(runtime_config.device_path)){
        std::cerr << "Wrong device path \"" << runtime_config.device_path << "\"!" << std::endl;
        running = false;
    }
    
    if (!running){ exit(EXIT_SUCCESS); }

    // from now on, we're sensitive about how we're treated considering signals
    register_signal_handlers();
    
    // create locks
    if (!(pidfile_check(runtime_config) && pidfile_lock(runtime_config))){
        running = false;
        std::cerr << "Way too many errors to continue have occured!" << std::endl;
    }

    // open the device/file
    int source_fd = -1;
    errno = 0;
    if (runtime_config.device_path.compare("-") == 0){
        source_fd = STDIN_FILENO;
    } else if ((source_fd = open(runtime_config.device_path.c_str(), O_RDONLY)) < 0) {
        std::cerr << runtime_config.device_path << ": open failed. Errno: " << errno << ", " << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    struct stat source_stat;
    errno = 0;
    fstat(source_fd, &source_stat);

    if (S_ISCHR(source_stat.st_mode)){
        std::cout << "Device " << runtime_config.device_path << " open successfull, treating as input event interface..." << std::endl;
        print_identity(runtime_config);
        retval = device_setup(runtime_config, source_fd);
        if (retval == 0){ retval = device_run(runtime_config, source_fd); }
        std::cout << std::endl << "Done" << std::endl;
    } else if (S_ISREG(source_stat.st_mode) || S_ISFIFO(source_stat.st_mode)){
        std::cout << runtime_config.device_path << " open successfull, treating as saved events storage" << std::endl;
        print_identity(runtime_config);
        retval = store_setup(runtime_config, source_fd);
        if (retval == 0){ retval = store_run(runtime_config, source_fd); }
        std::cout << std::endl << "Replay completed" << std::endl;
    } else {
        std::cout << "Type of " << runtime_config.device_path << " is unsupported. Unrecoverable error." << std::endl;
        close(source_fd);
    }

    close(source_fd);

    pidfile_unlock(runtime_config);

    return retval;
}

/**
 * \file      static_source.cpp
 * \brief     Provides the simple program that generates static tuio objects
 * \author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-03-18 16:55 UTC+1
 * \copyright BSD
 */

#include <kerat/kerat.hpp>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <ifaddrs.h>
#include <getopt.h>

using std::string;
using std::cerr;
using std::cout;
using std::endl;

struct static_config {

    static_config()
      :width(1920),height(1080),target_addr("localhost:3333"),source_addr(0),instance(0),send_interval(500000) // 0.5 sec
    { ; }

    /**
     * Width of the area that the sensor covers (in pixels)
     */
    uint16_t width;

    /**
     * Height of the area that the sensor covers (in pixels)
     */
    uint16_t height;

    /**
     * Target url (address:port) to send events to
     */
    string target_addr;

    /**
     * the application identifier
     */
    string app_id;

    /**
     * Source address (IPv4)
     */
    uint32_t source_addr;

    /**
     * Instance identifier
     */
    uint32_t instance;

    /**
     * Send interval in microseconds
     */
    useconds_t send_interval;
};


static string get_app_id();
static int32_t get_ipv4_addr();
static void print_usage();
static void register_signal_handlers();
static void handle_kill_signal(int ev);


static bool running = true;

string get_app_id(){

    // default hostname in case of gethostname failure
    const char * defaultHostname = "localhost";

    // RFC 2181 stands clearly that this shall never be longer than 255 octets
    const int hostnameBufferLength = 256;
    char hostnameBuffer[hostnameBufferLength];
    memset(hostnameBuffer, 0, hostnameBufferLength); // make sure it's an empty space

    errno = 0;
    // error detection
    if (gethostname(hostnameBuffer, hostnameBufferLength) == -1){
        switch (errno){
            // following means that the hostname has been truncated, correct possible missing terminating null
            case EINVAL:
            case ENAMETOOLONG: {
                hostnameBuffer[hostnameBufferLength - 1] = 0;
                cerr << "hostname has been truncated (errno: " << errno << "), using '" << hostnameBuffer << "' for hostname detection" << endl;
                break;
            }
            // otherwise use the default hostname
            case EPERM:
            case EFAULT: {
                strcpy(hostnameBuffer, defaultHostname);
                cerr << "gethostname has failed (errno: " << errno << "), using " << hostnameBuffer << endl;
                break;
            }

        }
    }

    // extract the short name
    {
        char * dot = strchrnul(hostnameBuffer, '.'); // '.' is host/domain separator
        *dot = 0; // not interested in the domain stuff, just hostname
    }

    // string typecasting
    string retval("static-source(");
    retval.append(hostnameBuffer).append(")");

    return retval;
}

int32_t get_ipv4_addr(){

    struct ifaddrs * interfaces = NULL;
    getifaddrs(&interfaces);
    int32_t address = 0;

    bool found = false;

    // search for first IPv4 address of first non-loopback interface if found, loopback otherwise
    for (struct ifaddrs * current = interfaces; (current != NULL) && (!found); current = current->ifa_next){

        // unfortunately, current specs of TUIO 2.0 consideres only the IPv4
        if ((current->ifa_addr != NULL) && (current->ifa_addr->sa_family==AF_INET)){

            // until any non-loopback interface is found, consider loopback
            bool isLoopback = strncmp(current->ifa_name, "lo", 2) == 0;
            if (((address == 0) && (isLoopback)) || (!isLoopback)) {
                address = ((sockaddr_in *)(current->ifa_addr))->sin_addr.s_addr;
            }

            // non-loopback interface was found, use it
            if (!isLoopback){
                found = true;
            }

        }

    }

    // better stay on the safe-side of SIGSEGV
    freeifaddrs(interfaces);
    interfaces = NULL;

    // convert from network byte order
    address = ntohl(address);

    // for some reason there's no network interface active, so at least pretend to run on localhost
//    if (address == 0){ address = 0x7F000001; }

    return address;
}

static void print_usage(){
    cout << "TUIO 2.0 static objects sender" << endl << endl;
    cout << "Usage:" << endl;
    cout << "\tstatic-source [options]" << endl;

    cout << endl;

    cout << "Options:" << endl;
    cout << "-h, --help                        \n\tShows this help" << endl;
    cout << "-t, --target                      \n\tSend the events to target address. Defaults to 127.0.0.1:3333" << endl;
    cout << "-d, --delay                      \n\tSend the events every <delay> seconds. Defaults to 0.5" << endl;
}

static void register_signal_handlers(){

    signal(SIGTERM, handle_kill_signal);
    // NOTE: SIGKILL is still uncaughtable
    //signal(SIGKILL, handle_kill_signal); // can't do anything about it, so at least correctly close
    signal(SIGABRT, handle_kill_signal);
    signal(SIGINT, handle_kill_signal);
    signal(SIGHUP, handle_kill_signal);

}

void handle_kill_signal(int ev){
    cout << "Caught signal " << ev << ", closing up..." << endl;
    running = false; // this variable is checked in the main event loop
}

int32_t generate_instance_id(const std::string & source){
    srand(time(NULL));

    int32_t hash = rand()%(0xffff); // otherwise the instance would get too long

    int i = 0;
    std::string::const_iterator end = source.end();
    for (std::string::const_iterator c = source.begin(); c != end; c++){
        hash += i*(*c);
        i++;
    }

    return hash;
}

void print_identity(const static_config & config){
    cout << "Running as " << libkerat::ipv4_to_str(config.source_addr) << "/" << config.instance << endl;
}

static int get_runtime_config(static_config & runtime_config, int argc, char * const * argv){
    std::string filename;

    if (argc < 1){
        cerr << "Invalid argument count!" << endl;
        print_usage();
        return EXIT_FAILURE;
    }

    struct option cmdline_opts[7];
    memset(&cmdline_opts, 0, sizeof(cmdline_opts));
    { int index = 0;

        cmdline_opts[index].name = "help";
        cmdline_opts[index].has_arg = 0;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'h';
        ++index;

        cmdline_opts[index].name = "target";
        cmdline_opts[index].has_arg = 1;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 't';
        ++index;

        cmdline_opts[index].name = "delay";
        cmdline_opts[index].has_arg = 1;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'd';
        ++index;
    }

    char opt = -1;
    while ((opt = getopt_long(argc, argv, "-ht:d:", cmdline_opts, NULL)) != -1){
        switch (opt){
            case 'h': {
                print_usage();
                running = false;
                return EXIT_FAILURE;
                break;
            }
            case 't': {
                runtime_config.target_addr = optarg;
                cout << "Setting target address to " << runtime_config.target_addr << endl;
                break;
            }
            case 'd': {
                double delay = strtod(optarg, NULL);
                runtime_config.send_interval = delay * 1000000; // delay is in seconds, so convert to microseconds
                cout << "Setting delay between bounds resend to " << delay << " seconds" << endl;
                break;
            }
        }
    }

    // complete config
    runtime_config.app_id = get_app_id();
    cout << runtime_config.app_id << endl;
    runtime_config.source_addr = get_ipv4_addr();
    runtime_config.instance = generate_instance_id(runtime_config.app_id);


    // extract target port from addr
    {
        size_t pos = runtime_config.target_addr.find(':');
        if (pos == string::npos){
            cerr << "Port not found in '" << runtime_config.target_addr << "', using default (3333)" << endl;
            runtime_config.target_addr.append(":3333");
        }
    }

    return EXIT_SUCCESS;
}

int main(int argc, char ** argv){
    if (getuid() == 0){
        cerr << "Yeah, like I'm gonna run with root privileges... I don't think so..." << endl;
        return EXIT_FAILURE;
    }

    static_config runtime_config;

    if (get_runtime_config(runtime_config, argc, argv) != 0){
//        cerr << "Unable to recovery from previous failures, quiting." << endl;
        return EXIT_FAILURE;
    }

    if (!running){ exit(EXIT_SUCCESS); }

    // from now on, we're sensitive about how we're treated considering signals
    register_signal_handlers();

    print_identity(runtime_config);

    // tuio init
    libkerat::simple_server tuio_server(runtime_config.target_addr, runtime_config.app_id, runtime_config.source_addr, runtime_config.instance, runtime_config.width, runtime_config.height);

    libkerat::distance_t horizontal_bound_width = runtime_config.width/4;
    libkerat::distance_t horizontal_bound_height = runtime_config.height/4;
    libkerat::distance_t horizontal_bound_x = runtime_config.width/4;
    libkerat::distance_t horizontal_bound_y = runtime_config.height/2;
    libkerat::session_id_t horizontal_bound_sid = tuio_server.get_auto_session_id();

    libkerat::message::bounds bnd_horizontal(
        horizontal_bound_sid,
        horizontal_bound_x, horizontal_bound_y,
        0, // this one is horizontal
        horizontal_bound_width, horizontal_bound_width,
        acos(-1)*horizontal_bound_width*horizontal_bound_height // consider it an ellipse
    );

    libkerat::distance_t rotated_bound_width = runtime_config.width/4;
    libkerat::distance_t rotated_bound_height = runtime_config.height/4;
    libkerat::distance_t rotated_bound_x = 3*runtime_config.width/4;
    libkerat::distance_t rotated_bound_y = runtime_config.height/2;
    libkerat::session_id_t rotated_bound_sid = tuio_server.get_auto_session_id();

    libkerat::message::bounds bnd_rotated(
        rotated_bound_sid,
        rotated_bound_x, rotated_bound_y,
        2, // this one is rotated by 2 radians
        rotated_bound_width, rotated_bound_width,
        acos(-1)*rotated_bound_width*rotated_bound_height // consider it an ellipse
    );

    while (running){
        tuio_server.append_clone(&bnd_horizontal);
        tuio_server.append_clone(&bnd_rotated);
        tuio_server.send();
        usleep(runtime_config.send_interval);
    }

    tuio_server.unregister_session_id(horizontal_bound_sid);
    tuio_server.unregister_session_id(rotated_bound_sid);
    tuio_server.send();

    std::cout << std::endl << "Done" << std::endl;

    return EXIT_SUCCESS;
}


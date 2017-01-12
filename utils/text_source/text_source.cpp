/**
 * \file      text_source.cpp
 * \brief     Provides the simple program that generates tuio messages based on standard input
 * \author    Matej Minárik <396546@mail.muni.cz> , Masaryk University, Brno, Czech Republic
 * \date      
 * \copyright 
 */

#include <kerat/kerat.hpp>
#include <errno.h>
#include <signal.h>
#include <algorithm>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <ifaddrs.h>
#include <getopt.h>
#include <map>
#include <set>
#include <bits/basic_string.h>

using std::string;
using std::cerr;
using std::cout;
using std::cin;
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
    string retval("text-source(");
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
    cout << "\ttext-source [options]" << endl;

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


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

//function for transforming vector of strings to list of 2d points
//take 2 words first have to have first char [ (it is point),
//@param pos position in vector, give it position to start, on the end is in it position of first occur of no point like string if all strings are point like in pos is number >number of stings
//@param words vector of strings
//@return list of points2d
std::list<libkerat::helpers::point_2d> strs_to_points2d(std::vector<std::string> words, int &pos){  
    std::list<libkerat::helpers::point_2d> output;
    while((words.size() < pos) && ((words[pos])[0] == '[' )){
        int x = strtod(words[pos].c_str() + 1, NULL);
        pos++;
        int y = strtod(words[pos].c_str(), NULL);
        pos++;
        output.push_back(libkerat::helpers::point_2d(x,y));
    }
    return output;
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


    typedef std::map<libkerat::session_id_t, libkerat::kerat_message*> id_message_map;
    typedef std::set<libkerat::session_id_t> id_message_set;
    
    string line;
    std::vector<std::string> words;
    id_message_set alive_messages;

   
    while (running){
        do{
            
            getline(cin, line);
        }while(line.empty() && running);
            words = split(line, ' ');
            if(!words[0].compare("/tuio2/alv")){
                
                std::vector<int> id_alive = std::vector<int>();
                for(int i = 1; i < words.size(); i++){
                    id_alive.push_back(strtol(words[i].c_str(),NULL,10));
                }
                id_message_set::iterator check_all_iterrator = alive_messages.begin();          
                while(alive_messages.end() != check_all_iterrator){
                    if(std::find(id_alive.begin(), id_alive.end(), *check_all_iterrator) == id_alive.end()){      //check if id of message is allive if not unregistr it and delete
                        tuio_server.unregister_session_id(*check_all_iterrator);
                        alive_messages.erase(*check_all_iterrator);
                    }
                    check_all_iterrator++;
                }
                cout  << "alive---" << endl;
                tuio_server.send();
                usleep(runtime_config.send_interval);
                
                
            }else if(!words[0].compare("/tuio2/bnd")){
                
                
                libkerat::session_id_t bound_sid = strtol(words[1].c_str(),NULL,10);
                cout << "bnd---" << bound_sid << endl;
                if(alive_messages.find(bound_sid) == alive_messages.end()){
                    alive_messages.insert(bound_sid);
                    tuio_server.register_session_id(bound_sid);
                }
                libkerat::message::bounds* current_bounds_message = new libkerat::message::bounds; 
                
                current_bounds_message->set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_2D);
                current_bounds_message->set_session_id(bound_sid);
                current_bounds_message->set_x(atof(words[2].c_str() + 1));              //move 1 because [, for atof to read number
                current_bounds_message->set_y(atof(words[3].c_str()));
                current_bounds_message->set_angle(atof(words[4].c_str()));
                current_bounds_message->set_width(atof(words[5].c_str()));
                current_bounds_message->set_height(atof(words[6].c_str()));
                current_bounds_message->set_area(atof(words[7].c_str()));
                if(words.size() > 8){
                    current_bounds_message->set_x_velocity(atof(words[8].c_str() + 1));         //move 1 because (, for atof to read number
                    current_bounds_message->set_y_velocity(atof(words[9].c_str()));
                    current_bounds_message->set_rotation_velocity(atof(words[10].c_str()));
                    current_bounds_message->set_acceleration(atof(words[11].c_str()));
                    current_bounds_message->set_rotation_acceleration(atof(words[12].c_str()));
                }else{
                    current_bounds_message->set_x_velocity(0);
                    current_bounds_message->set_y_velocity(0);
                    current_bounds_message->set_rotation_velocity(0);
                    current_bounds_message->set_acceleration(0);
                    current_bounds_message->set_rotation_acceleration(0);
                }
                tuio_server.append_clone(current_bounds_message);
                delete current_bounds_message;
                
            }else if(!words[0].compare("/tuio2/b3d")){
                
                
                libkerat::session_id_t bound_sid = strtol(words[1].c_str(),NULL,10);
                cout << "/b3d---" << bound_sid << endl;
                if(alive_messages.find(bound_sid) == alive_messages.end()){
                    alive_messages.insert(bound_sid);
                    tuio_server.register_session_id(bound_sid);
                }
                libkerat::message::bounds* current_bounds_message = new libkerat::message::bounds();
                
                current_bounds_message->set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_3D);
                current_bounds_message->set_session_id(bound_sid);
                current_bounds_message->set_x(atof(words[2].c_str() + 1));              //move 1 because [, for atof to read number
                current_bounds_message->set_y(atof(words[3].c_str()));
                current_bounds_message->set_z(atof(words[4].c_str()));
                current_bounds_message->set_yaw(atof(words[5].c_str()));
                current_bounds_message->set_pitch(atof(words[6].c_str()));
                current_bounds_message->set_roll(atof(words[7].c_str()));
                current_bounds_message->set_width(atof(words[8].c_str()));
                current_bounds_message->set_height(atof(words[9].c_str()));
                current_bounds_message->set_depth(atof(words[10].c_str()));
                current_bounds_message->set_volume(atof(words[11].c_str()));
                if(words.size() > 12){
                    current_bounds_message->set_x_velocity(atof(words[12].c_str() + 1));         //move 1 because (, for atof to read number
                    current_bounds_message->set_y_velocity(atof(words[13].c_str()));
                    current_bounds_message->set_z_velocity(atof(words[14].c_str()));
                    current_bounds_message->set_yaw_velocity(atof(words[15].c_str()));
                    current_bounds_message->set_pitch_velocity(atof(words[16].c_str()));
                    current_bounds_message->set_roll_velocity(atof(words[17].c_str()));
                    current_bounds_message->set_acceleration(atof(words[18].c_str()));
                    current_bounds_message->set_rotation_acceleration(atof(words[19].c_str()));
                }else{
                    current_bounds_message->set_x_velocity(0);
                    current_bounds_message->set_y_velocity(0);
                    current_bounds_message->set_z_velocity(0);
                    current_bounds_message->set_yaw_velocity(0);
                    current_bounds_message->set_pitch_velocity(0);
                    current_bounds_message->set_roll_velocity(0);
                    current_bounds_message->set_acceleration(0);
                    current_bounds_message->set_rotation_acceleration(0);
                }
                tuio_server.append_clone(current_bounds_message);
                delete current_bounds_message;
                
            }else if(!words[0].compare("/tuio2/ptr")){
                
                
                libkerat::session_id_t pointr_sid = strtol(words[1].c_str(),NULL,10);
                cout << "ptr---" << pointr_sid << endl;
                if(alive_messages.find(pointr_sid) == alive_messages.end()){
                    alive_messages.insert(pointr_sid);
                    tuio_server.register_session_id(pointr_sid);
                }
                libkerat::message::pointer* current_pointr_message = new libkerat::message::pointer();
                std::stringstream str_stream_pom(words[2]);
                string word;
                current_pointr_message->set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_2D);
                current_pointr_message->set_session_id(pointr_sid);
                getline(str_stream_pom, word, '/');
                current_pointr_message->set_user_id(strtol(word.c_str(),NULL,10));
                getline(str_stream_pom, word);
                current_pointr_message->set_type_id(strtol(word.c_str(),NULL,10));
                current_pointr_message->set_component_id(strtol(words[3].c_str(),NULL,10));
                current_pointr_message->set_x(atof(words[4].c_str() + 1));              //move 1 because [, for atof to read number
                current_pointr_message->set_y(atof(words[5].c_str()));
                current_pointr_message->set_width(atof(words[6].c_str()));   
                current_pointr_message->set_pressure(atof(words[7].c_str()));
                if(words.size() > 8){
                    current_pointr_message->set_x_velocity(atof(words[8].c_str() + 1));                 //move 1 because (, for atof to read number
                    current_pointr_message->set_y_velocity(atof(words[9].c_str()));
                    current_pointr_message->set_acceleration(atof(words[10].c_str()));
                }else{
                    current_pointr_message->set_x_velocity(0);                 //move 1 because (, for atof to read number
                    current_pointr_message->set_y_velocity(0);
                    current_pointr_message->set_acceleration(0);
                }
                tuio_server.append_clone(current_pointr_message);
                delete current_pointr_message;
                
            }else if(!words[0].compare("/tuio2/p3d")){
                
                
                libkerat::session_id_t pointr_sid = strtol(words[1].c_str(),NULL,10);
                cout << "p3d---" << pointr_sid << endl;
                if(alive_messages.find(pointr_sid) == alive_messages.end()){
                    alive_messages.insert(pointr_sid);
                    tuio_server.register_session_id(pointr_sid);
                }
                libkerat::message::pointer* current_pointr_message = new libkerat::message::pointer();
                std::stringstream str_stream_pom(words[2]);
                string word;
                current_pointr_message->set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_3D);
                current_pointr_message->set_session_id(pointr_sid);
                getline(str_stream_pom, word, '/');
                current_pointr_message->set_user_id(strtol(word.c_str(),NULL,10));
                getline(str_stream_pom, word);
                current_pointr_message->set_type_id(strtol(word.c_str(),NULL,10));
                current_pointr_message->set_component_id(strtol(words[3].c_str(),NULL,10));
                current_pointr_message->set_x(atof(words[4].c_str() + 1));              //move 1 because [, for atof to read number
                current_pointr_message->set_y(atof(words[5].c_str()));
                current_pointr_message->set_z(atof(words[6].c_str()));
                current_pointr_message->set_width(atof(words[7].c_str()));   
                current_pointr_message->set_pressure(atof(words[8].c_str()));
                if(words.size() > 9){
                    current_pointr_message->set_x_velocity(atof(words[9].c_str() + 1));                 //move 1 because (, for atof to read number
                    current_pointr_message->set_y_velocity(atof(words[10].c_str()));
                    current_pointr_message->set_z_velocity(atof(words[11].c_str()));
                    current_pointr_message->set_acceleration(atof(words[12].c_str()));
                }else{
                    current_pointr_message->set_x_velocity(0);                 //move 1 because (, for atof to read number
                    current_pointr_message->set_y_velocity(0);
                    current_pointr_message->set_z_velocity(0);
                    current_pointr_message->set_acceleration(0);
                }
                tuio_server.append_clone(current_pointr_message);
                delete current_pointr_message;
                
            }else if(!words[0].compare("/tuio2/tok")){
                
                
                libkerat::session_id_t token_sid = strtol(words[1].c_str(),NULL,10);
                cout << "tok---" << token_sid << endl;
                if(alive_messages.find(token_sid) == alive_messages.end()){
                    alive_messages.insert(token_sid);
                    tuio_server.register_session_id(token_sid);
                }
                libkerat::message::token* current_token_message = new libkerat::message::token();
                std::stringstream str_stream_pom(words[2]);
                string word;
                current_token_message->set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_2D);
                current_token_message->set_session_id(token_sid);
                getline(str_stream_pom, word, '/');
                current_token_message->set_user_id(strtol(word.c_str(),NULL,10));
                getline(str_stream_pom, word);
                current_token_message->set_type_id(strtol(word.c_str(),NULL,10));
                current_token_message->set_component_id(strtol(words[3].c_str(),NULL,10));
                current_token_message->set_x(atof(words[4].c_str() + 1));              //move 1 because [, for atof to read number
                current_token_message->set_y(atof(words[5].c_str()));
                current_token_message->set_yaw(atof(words[6].c_str()));
                if(words.size() > 7){
                    current_token_message->set_x_velocity(atof(words[7].c_str() + 1));                 //move 1 because (, for atof to read number
                    current_token_message->set_y_velocity(atof(words[8].c_str()));
                    current_token_message->set_yaw_velocity(atof(words[9].c_str()));
                    current_token_message->set_acceleration(atof(words[10].c_str()));
                    current_token_message->set_rotation_acceleration(atof(words[11].c_str()));
                }else{
                    current_token_message->set_x_velocity(0);                 //move 1 because (, for atof to read number
                    current_token_message->set_y_velocity(0);
                    current_token_message->set_z_velocity(0);
                    current_token_message->set_acceleration(0);
                }
                tuio_server.append_clone(current_token_message);
                delete current_token_message;
                
            }else if(!words[0].compare("/tuio2/t3d")){
                
                
                libkerat::session_id_t token_sid = strtol(words[1].c_str(),NULL,10);
                cout << "t3d---" << token_sid << endl;
                if(alive_messages.find(token_sid) == alive_messages.end()){
                    alive_messages.insert(token_sid);
                    tuio_server.register_session_id(token_sid);
                }
                libkerat::message::token* current_token_message = new libkerat::message::token();
                std::stringstream str_stream_pom(words[2]);
                string word;
                current_token_message->set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_3D);
                current_token_message->set_session_id(token_sid);
                getline(str_stream_pom, word, '/');
                current_token_message->set_user_id(strtol(word.c_str(),NULL,10));
                getline(str_stream_pom, word);
                current_token_message->set_type_id(strtol(word.c_str(),NULL,10));
                current_token_message->set_component_id(strtol(words[3].c_str(),NULL,10));
                current_token_message->set_x(atof(words[4].c_str() + 1));              //move 1 because [, for atof to read number
                current_token_message->set_y(atof(words[5].c_str()));
                current_token_message->set_z(atof(words[6].c_str()));
                current_token_message->set_yaw(atof(words[7].c_str()));
                current_token_message->set_pitch(atof(words[8].c_str()));
                current_token_message->set_roll(atof(words[9].c_str()));
                if(words.size() > 10){
                    current_token_message->set_x_velocity(atof(words[10].c_str() + 1));                 //move 1 because (, for atof to read number
                    current_token_message->set_y_velocity(atof(words[11].c_str()));
                    current_token_message->set_z_velocity(atof(words[12].c_str()));
                    current_token_message->set_yaw_velocity(atof(words[13].c_str()));
                    current_token_message->set_pitch_velocity(atof(words[14].c_str()));
                    current_token_message->set_roll_velocity(atof(words[15].c_str()));
                    current_token_message->set_acceleration(atof(words[16].c_str()));
                    current_token_message->set_rotation_acceleration(atof(words[17].c_str()));
                }else{
                    current_token_message->set_x_velocity(0);
                    current_token_message->set_y_velocity(0);
                    current_token_message->set_z_velocity(0);
                    current_token_message->set_yaw_velocity(0);
                    current_token_message->set_pitch_velocity(0);
                    current_token_message->set_roll_velocity(0);
                    current_token_message->set_acceleration(0);
                    current_token_message->set_rotation_acceleration(0);
                }
                tuio_server.append_clone(current_token_message);
                delete current_token_message;
                
            }else if(!words[0].compare("/tuio2/sym")){
                
                
                libkerat::session_id_t symbol_sid = strtol(words[1].c_str(),NULL,10);
                cout << "sym---" << symbol_sid << endl;
                if(alive_messages.find(symbol_sid) == alive_messages.end()){
                    alive_messages.insert(symbol_sid);
                    tuio_server.register_session_id(symbol_sid);
                }
                libkerat::message::symbol* current_symbol_message = new libkerat::message::symbol();
                std::stringstream str_stream_pom(words[2]);
                string word;
                current_symbol_message->set_session_id(symbol_sid);
                getline(str_stream_pom, word, '/');
                current_symbol_message->set_user_id(strtol(word.c_str(),NULL,10));
                getline(str_stream_pom, word);
                current_symbol_message->set_type_id(strtol(word.c_str(),NULL,10));
                current_symbol_message->set_component_id(strtol(words[3].c_str(),NULL,10));
                current_symbol_message->set_group(words[4]);
                string data = words[5];
                for(int i=6; i<word.size(); i++ ){
                    data.append(" ");
                    data.append(words[i]);
                }
                if(data[0] == '"'){                             //string data
                    data = data.substr(1,data.size()-2);                //remove '"' from begin and end of the string
                    current_symbol_message->set_data(data);     
                }else{                                          //blob data
                    data.erase(0,6);                            //remove "<blob/"
                    int length = strtol(data.c_str(),NULL,10);
                    data.erase(0,data.find('/')+3);             //remove "length/0x", now data contain only data + on the end '>'
                    uint8_t* data_out = new uint8_t[length+1];
                    std::stringstream data_stream(data);
                    for(int i=0; i<length; i++){
                        data_stream >> std::hex >> data_out[i];
                    }
                }
                tuio_server.append_clone(current_symbol_message);
                delete current_symbol_message;
                
            }else if(!words[0].compare("/tuio2/chg")){
                libkerat::session_id_t con_hul_geo_sid = strtol(words[1].c_str(),NULL,10);
                cout << "chg---" << con_hul_geo_sid << endl;
                if(alive_messages.find(con_hul_geo_sid) == alive_messages.end()){
                    alive_messages.insert(con_hul_geo_sid);
                    tuio_server.register_session_id(con_hul_geo_sid);
                }
                libkerat::message::convex_hull* current_con_hul_geo_message = new libkerat::message::convex_hull(con_hul_geo_sid);
                current_con_hul_geo_message->set_session_id(con_hul_geo_sid);
                int pos = 2;
                current_con_hul_geo_message->set_hull(strs_to_points2d(words,pos));
                
                tuio_server.append_clone(current_con_hul_geo_message);
                delete current_con_hul_geo_message;
                
            }else if(!words[0].compare("/tuio2/ocg")){
                libkerat::session_id_t out_con_geo_sid = strtol(words[1].c_str(),NULL,10);
                cout << "ocg---" << out_con_geo_sid << endl;
                if(alive_messages.find(out_con_geo_sid) == alive_messages.end()){
                    alive_messages.insert(out_con_geo_sid);
                    tuio_server.register_session_id(out_con_geo_sid);
                }
                libkerat::message::outer_contour* current_out_con_geo_message = new libkerat::message::outer_contour(out_con_geo_sid);
                current_out_con_geo_message->set_session_id(out_con_geo_sid);
                int pos = 2;
                current_out_con_geo_message->set_contour(strs_to_points2d(words,pos));
                
                tuio_server.append_clone(current_out_con_geo_message);
                delete current_out_con_geo_message;
                
            }else if(!words[0].compare("/tuio2/icg")){
                libkerat::session_id_t inn_con_geo_sid = strtol(words[1].c_str(),NULL,10);
                cout << "icg---" << inn_con_geo_sid << endl;
                if(alive_messages.find(inn_con_geo_sid) == alive_messages.end()){
                    alive_messages.insert(inn_con_geo_sid);
                    tuio_server.register_session_id(inn_con_geo_sid);
                }
                libkerat::message::inner_contour* current_inn_con_geo_message = new libkerat::message::inner_contour(inn_con_geo_sid);
                current_inn_con_geo_message->set_session_id(inn_con_geo_sid);
                int pos = 2;
                int num_of_sections = 0;
                libkerat::message::inner_contour::contour_list con_list;
                while(words.size() > pos){
                    con_list.push_back(strs_to_points2d(words,pos));
                    pos++;
                }
                current_inn_con_geo_message->set_contours(con_list);
                
                tuio_server.append_clone(current_inn_con_geo_message);
                delete current_inn_con_geo_message;
                
            }
            
            
            //v skeletone vytvorit graf a ten nastavit
            

        

    }
    

    tuio_server.send();

    std::cout << std::endl << "Done" << std::endl;

    return EXIT_SUCCESS;
}


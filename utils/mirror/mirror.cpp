/*
 * File:   main.cpp
 * Author: Lukáš Ručka (359687@mail.muni.cz)
 * Organization: Masaryk University; Brno, Czech Republic
 * License: BSD
 * Created on 21. april 2011, 14:16
 */

#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <signal.h>
#include <iostream>
#include <inttypes.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <getopt.h>
#include <list>
#include <sstream>
#include <sys/types.h>
#include <ifaddrs.h>
#include <kerat/kerat.hpp>

#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#ifdef HAVE_MUSE
    #include <muse/muse.hpp>
    #include <tinyxml.h>
#endif

#include "common.hpp"

using std::cout;
using std::cerr;
using std::endl;


// ============================================ typedefs
struct mirror_config {
    bool multiplexing;
#ifdef HAVE_MUSE
    TiXmlDocument muse_config;
#endif
    unsigned short port;
    uint32_t ip;
    string_list commandline_commands;
};

typedef std::list<libkerat::simple_server *> servers_list;

// ============================================ globals
#ifdef HAVE_MUSE
static muse::module_service::module_chain muse_modules;
#endif
static libkerat::simple_client * raw_client = NULL;
static libkerat::adaptors::multiplexing_adaptor * multiplexing_adaptor = NULL;
static libkerat::listeners::forwarding_listener * forwarding_listener = NULL;
static servers_list servers;
static bool running = true;
mirror_config config;

// ============================================ core
static void register_signal_handlers();
static void handle_kill_signal(int ev);
static mirror_config get_default_config();

static void usage();

static int run();

static int parse_commandline(int argc, char ** argv);

#ifdef HAVE_MUSE
static bool load_module_chain(const TiXmlDocument & xml_config, muse::module_service::module_chain & modules, std::string & err){
    
    if (xml_config.NoChildren()){ return false; }
    
    muse::module_service * modserv = muse::module_service::get_instance();

    const TiXmlElement * e_config = xml_config.RootElement();
    assert(e_config != NULL);
    const TiXmlElement * e_chain = e_config->FirstChildElement("chain");
    
    std::stringstream err_out;

    if (modserv->create_module_chain(e_chain, modules)){
        if (modules.empty()){
            err_out << "Failed to create module chain!";
        } else {
            err_out << "Failed to create module from following module data:" <<
                modules.back().second->GetText();
        }
        modserv->free_module_chain(modules);
        
        err = err_out.str();
        return false;
    }
    
    return true;
}
#endif

int main(int argc,char **argv){

#ifdef HAVE_MUSE
//    muse_init();
#endif

    srand(time(NULL));

    // for command line arguments

    // parse the command line auxiliary
    config = get_default_config();
    if (parse_commandline(argc, argv) != 0){
        cerr << "Parsing command line has failed, commiting suicide" << endl;
        exit(EXIT_FAILURE);
    }

    if (!running){ exit(EXIT_SUCCESS); }

    register_signal_handlers();

    return run();
}

void usage(){
    cout << "Usage:" << endl;
    cout << "muse-mirrord [options] [<host>[:port]]+" << endl;
    cout << endl;
    cout << "Options:" << endl;
    cout << "--raw                               \t\tDisable multiplexing client" << endl;
    cout << "--port=<port>                       \t\tPort that TUIO client shall listen on" << endl;
#ifdef HAVE_MUSE
    cout << "--muse-config=<file.xml>            \t\tUse given MUSE framework configuration" << endl;
#endif
    cout << endl;
    cout.flush();
}

static int parse_commandline(int argc, char ** argv){

    struct option cmdline_opts[10];
    memset(&cmdline_opts, 0, sizeof(cmdline_opts));
    { int index = 0;

        cmdline_opts[index].name = "help";
        cmdline_opts[index].has_arg = 0;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'h';
        ++index;

        cmdline_opts[index].name = "raw";
        cmdline_opts[index].has_arg = 0;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'r';
        ++index;

        cmdline_opts[index].name = "port";
        cmdline_opts[index].has_arg = 1;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'p';
        ++index;

#ifdef HAVE_MUSE
        cmdline_opts[index].name = "muse-config";
        cmdline_opts[index].has_arg = 1;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'm';
        ++index;
#endif

    }

  char opt = -1;
    while ((opt = getopt_long(argc, argv, "-hrp:m:", cmdline_opts, NULL)) != -1){
        switch (opt){
            case 'h': {
                usage();
                running = false;
                return EXIT_FAILURE;
                break;
            }
            case 'r': { // ================ multiplexing
                config.multiplexing = false;
                break;
            }
            case 'p': { // ================ port
                config.port = strtol(optarg, NULL, 10);
                if (config.port > 0xffff){ cerr << "Invalid port " << config.port << endl; }
                break;
            }
            #ifdef HAVE_MUSE
            case 'm': {
                config.multiplexing = false;
                config.muse_config.LoadFile(optarg);
                if (config.muse_config.Error()){
                    std::cerr << "Failed to parse given MUSE config file!" << std::endl;
                    std::cerr << config.muse_config.ErrorDesc() << std::endl;
                    return -1;
                }
                break;
            }
            #endif

            default: {
                config.commandline_commands.push_back(optarg);
            }
        }
    }

    return 0;
}

static uint32_t get_ipv4_addr(){

    struct ifaddrs * interfaces = NULL;
    getifaddrs(&interfaces);
    uint32_t address = 0;

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
static mirror_config get_default_config(){
    mirror_config retval;
    
    retval.port = 3333;
    retval.ip = get_ipv4_addr();
    retval.multiplexing = true;
    
    return retval;
}

static void register_signal_handlers(){

    signal(SIGTERM, handle_kill_signal);
    // NOTE: SIGKILL is still uncaughtable
    //signal(SIGKILL, handle_kill_signal); // can't do anything about it, so at least correctly close
    signal(SIGABRT, handle_kill_signal);
    signal(SIGINT, handle_kill_signal);
    signal(SIGHUP, handle_kill_signal);
    signal(SIGCHLD, SIG_IGN);

}

static void handle_kill_signal(int ev){
    cout << "Caught signal " << ev << ", closing up..." << endl;
    running = false; // this variable is checked in the main event loop
}

static int create_command_socket(const std::string & path){
    if (path.length() > 107){
        std::cerr << "Unable to create control socket - path \"" << path << "\" is too long!" << std::endl;
        return -1;
    }
    
    int command_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (command_socket < 0){ 
        std::cerr << "Unable to create control socket - socket() syscall has failed." << std::endl;
        return -1; 
    }
    
    struct sockaddr_un socket_path;
    memset(&socket_path, 0, sizeof(socket_path));
    socket_path.sun_family = AF_UNIX;
    strncpy(socket_path.sun_path, path.c_str(), 107);
    
    int err = bind(command_socket, (struct sockaddr *)&socket_path, sizeof(socket_path));
    if (err < 0) {
        std::cerr << "Failed to bind the command socket to filename!" << std::endl;
        close(command_socket);
        return -1;
    }
    
    return command_socket;
}

static std::string make_loaddress_uri(std::string uri){
    
    lo_address tmp = lo_address_new_from_url(uri.c_str());
    if (tmp == NULL) { // url needed
        // allow udp:// or tcp:// or... syntax
        if (uri.compare(0, 6, "udp://") != 0){ uri.insert(0, "udp://"); }
        uri.insert(0, "osc.");
    }
    
    if (tmp != NULL){ 
        lo_address_free(tmp);
        tmp = NULL;
    }
    
    // check for port
    if ((uri.compare(0, 10, "osc.tcp://") == 0) || (uri.compare(0, 10, "osc.udp://") == 0)){
        size_t bracket_pos = uri.find(']');
        if (bracket_pos == std::string::npos){
            // IPv4
            size_t colon_pos = uri.find(':', 10);
            if (colon_pos == std::string::npos){
                // port not present, make it 3333
                uri.append(":3333");
            }
        } else {
            // IPv6 - has to be followed by colon
            size_t colon_pos = uri.find(':', bracket_pos);
            if (colon_pos == std::string::npos) {
                uri.insert(bracket_pos+1, ":3333");
            } else if (bracket_pos+1 != colon_pos){ // invalid
                return "";
            } else {
                // consider ok
            }
        }
    }
    
    tmp = lo_address_new_from_url(uri.c_str());
    if (tmp == NULL){ return ""; }
    
    if (lo_address_get_port(tmp) == NULL){
        uri = "";
    } else {
        uri = lo_address_get_url(tmp);
    }
    lo_address_free(tmp);
    
    return uri;
}

static bool process_commands(string_list commands, std::string & reply){
    std::stringstream report;
    bool retval = true;
    
    while (!commands.empty()){
        command_t cmd = COMMAND_UNKNOWN;
        do {
            cmd = check_if_command(commands.front());
            if (cmd == COMMAND_UNKNOWN) {
                report << "Unrecognized command \"" << commands.front() << "\"\n";
            }
            commands.pop_front();
        } while ((cmd == COMMAND_UNKNOWN) && (!commands.empty()));
        
        switch (cmd) {
            case COMMAND_STOP:{
                running = false;                
                report << "Stopping..." << std::endl;
                retval &= true;
                goto exit;
                break;
            }
            case COMMAND_ADD: {
                while (!commands.empty() && (check_if_command(commands.front()) == COMMAND_UNKNOWN)){
                    std::string uri_original = commands.front();
                    commands.pop_front();
                    std::string uri = make_loaddress_uri(uri_original);
                    
                    if (uri.empty()){
                        report << "Address " << uri_original << " does not seem to be a valid liblo-compatible uri" << std::endl;
                        retval &= false;
                        continue;
                    }
                
                    libkerat::simple_server * new_server = NULL;
                    try {
                        new_server = new libkerat::simple_server(
                            uri, 
                            "MUSE mirror", 
                            config.ip,
                            config.port,
                            1920,
                            1080
                        );
                    } catch (const libkerat::exception::net_setup_error & e){
                        report << "Failed to add target \"" << uri << "\"!" << endl;
                        std::cerr << e.what() << std::endl;
                        continue;
                    }
                    servers.push_back(new_server);
                    new_server->add_adaptor(forwarding_listener->get_server_adaptor());
                    
                    report << "Added " << uri << std::endl;
                }
                break;
            }
            case COMMAND_DEL: {
                while (!commands.empty() && (check_if_command(commands.front()) == COMMAND_UNKNOWN)){
                    std::string seek_original = commands.front();
                    commands.pop_front();
                    std::string seek_for = make_loaddress_uri(seek_original);
                    
                    if (seek_for.empty()){
                        report << "Address " << seek_original << " does not seem to be a valid liblo-compatible uri" << std::endl;
                        retval &= false;
                        continue;
                    }
                    
                    lo_address tmp = lo_address_new_from_url(seek_for.c_str());
                    char * url_tmp  = lo_address_get_url(tmp);

                    bool deleted = false;
                    for (servers_list::iterator s = servers.begin(); (s != servers.end()) && !deleted; ++s){
                        char * url_orig = lo_address_get_url((*s)->get_target());
                        if (strcmp(url_orig, url_tmp) == 0){
                            // disconnect from the stream
                            (*s)->del_adaptor(forwarding_listener->get_server_adaptor());
                            servers.erase(s);
                            deleted = true;
                        }
                        free(url_orig);
                    }
                    
                    if (deleted){
                        report << "Target " << url_tmp << " successfully disabled." << std::endl;
                    } else {
                        report << "Unknown target " << url_tmp << "!" << std::endl;
                        retval &= false;
                    }

                    free(url_tmp);
                    lo_address_free(tmp);
                }
                break;
            }
            case COMMAND_SHOW: {
                for (servers_list::const_iterator s = servers.begin(); s != servers.end(); ++s){
                    char * url = lo_address_get_url((*s)->get_target());
                    report << "Target: " << url << std::endl;
                    free(url);
                }
                
                if (servers.empty()){
                    report << "No targets set!" << std::endl;
                }
                break;
            }
            
            case COMMAND_LOAD_CONFIG: {
                if (commands.empty()){
                    report << "Command lacks argument!" << std::endl;
                    retval &= false;
                    continue;
                }
                
                std::string xml_config = commands.front();
                commands.pop_front();

#ifndef HAVE_MUSE
                report << "Command \"config\" unsupported, recompile without the MUSE framework!" << std::endl;
                retval = false;
#else
                bool arg_is_xml = false;
                
                if (xml_config.compare(0, 1, "/") == 0){
                    arg_is_xml = false;
                } else if (xml_config.compare(0, 7, "file://") == 0){
                    arg_is_xml = false;
                } else if (xml_config.compare(0, 1, "<") == 0){
                    arg_is_xml = true;
                }
                
                TiXmlDocument tmp_doc;
                if (arg_is_xml){
                    tmp_doc.SetValue(xml_config);
                } else {
                    tmp_doc.LoadFile(xml_config, TIXML_ENCODING_UTF8);
                }
                
                if (tmp_doc.Error()){
                    report << "Unable to load given " << ((arg_is_xml)?"data":"file") << "!" << std::endl;                    
                    retval &= false;
                    continue;
                }
                
                muse::module_service::module_chain modules;
                std::string error;
                bool modules_loaded = load_module_chain(tmp_doc, modules, error);
                if (!modules_loaded){
                    report << error << std::endl;                    
                    retval &= false;
                    continue;
                }

                // exchange modules - no muse modules loaded
                if (muse_modules.empty()){
                    // is multiplexing adaptor enabled?
                    if (multiplexing_adaptor != NULL){
                        raw_client->del_listener(multiplexing_adaptor);
                        multiplexing_adaptor->del_listener(forwarding_listener);
                    } else { // no multiplexing
                        raw_client->del_listener(forwarding_listener);
                    }
                } else { // muse modules are loaded...
                    raw_client->del_listener(muse_modules.front().first);
                    muse_modules.back().first->del_listener(forwarding_listener);
                }
                
                // now, there is no connection between raw_client and forwarding adaptor

                if (modules.empty()){ // that means we will either connect through adaptor or not
                    if (multiplexing_adaptor != NULL){
                        multiplexing_adaptor->add_listener(forwarding_listener);
                        raw_client->add_listener(multiplexing_adaptor);
                    } else { // no multiplexing
                        raw_client->add_listener(forwarding_listener);
                    }
                } else {
                    modules.back().first->add_listener(forwarding_listener);
                    raw_client->add_listener(modules.front().first);
                }
                
                muse_modules = modules;
#endif
                // load_config
                break;
            }
            
            default: {
                report << "Unrecognized command!" << endl;
                retval &= false;
            }
        }
    }
    
exit:
    reply = report.str();
    return retval;
}

static void await_commands(int command_socket){

    char * buffer = (char *) malloc(MAX_COMMAND_LENGTH);
    memset(buffer, 0, MAX_COMMAND_LENGTH);

    struct sockaddr_un reply_path;
    memset(&reply_path, 0, sizeof(sockaddr_un));
    reply_path.sun_family = AF_UNIX;

    ssize_t reply_length = 0;
    socklen_t reply_path_length = sizeof(sockaddr_un);

    fd_set command_received;
    FD_SET(command_socket, &command_received);
    
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    
    if (select(command_socket + 1, &command_received, NULL, NULL, &timeout) > 0){
        if (FD_ISSET(command_socket, &command_received)){
            ssize_t rval = recvfrom(command_socket, buffer, MAX_COMMAND_LENGTH-1, 0, (struct sockaddr *)&reply_path, &reply_path_length);
            if (rval > 0){
                reply_length = rval;
            }
        }
    }

    std::string reply_text = "";

    // What can I do you for?
    if (reply_length == 0){ 
        free(buffer);
        return; 
    } else if (reply_length+1 > MAX_COMMAND_LENGTH) {
        reply_text = "Command too long...";
    } else {
        process_commands(unescape_args(buffer), reply_text);
    }

    if (!reply_text.empty() && (reply_path_length > 0)){
        ssize_t bytes_written = sendto(command_socket, reply_text.c_str(), reply_text.size(), 0, (struct sockaddr *)&reply_path, reply_path_length);
        if (bytes_written < 0){
            std::cerr << "Failed to write command answer: " << strerror(errno) << std::endl;
        }
    }
}

static int run(){
    
    int retval = 0;
    
    // libkerat init, check for network
    try {
        raw_client = new libkerat::simple_client(config.port);
    } catch (libkerat::exception::net_setup_error & ex){
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    multiplexing_adaptor = NULL;
    forwarding_listener = NULL;
    libkerat::client * last_client = raw_client;
    
    // initialize clients
#ifdef HAVE_MUSE
    { // register framework with client
        libkerat::internals::convertor_list muse_convertors(muse::get_muse_convertors());
        std::for_each(muse_convertors.begin(), muse_convertors.end(), raw_client->get_enabler_functor());
    }

    if (config.muse_config.NoChildren()){
#endif
        if (config.multiplexing){
            multiplexing_adaptor = new libkerat::adaptors::multiplexing_adaptor;
            last_client->add_listener(multiplexing_adaptor);
            last_client = multiplexing_adaptor;
        }
#ifdef HAVE_MUSE
    } else {
        std::string error;
        bool muse_succ = load_module_chain(config.muse_config, muse_modules, error);

        if (muse_succ){
            last_client->add_listener(muse_modules.front().first);
            last_client = muse_modules.back().first;
        } else {
            std::cerr << error << std::endl;
        }
    }
#endif

    forwarding_listener = new libkerat::listeners::forwarding_listener(true, true);
    last_client->add_listener(forwarding_listener);
    
    // clients initalized, everything is ok - open socket & fork

    std::string command_socket_path = get_control_socket_path(config.port);
    unlink(command_socket_path.c_str());
    int command_socket = create_command_socket(command_socket_path);
    
    // connect the hosts given on command line to the server
    {
        std::string reply;
        bool init_ok = process_commands(config.commandline_commands, reply);
        
        if (!init_ok){
            running = false;
            retval = 1;
            std::cerr << reply;
        } else {
            std::cout << reply;
        }
    }
    
    if (running){ // fork
        pid_t daemon_pid = -1;
        for (int attempts = 0; (attempts < 3) && (daemon_pid < 0); ++attempts){
            errno = 0;
            daemon_pid = fork();
            switch(errno){
                case 0: break;
                case EAGAIN:
                case ENOMEM:
                default:
                    usleep(50000);
                    break;
            }
        }
        
        if (daemon_pid < 0){
            std::cerr << "Failed to fork daemon!" << std::endl;
        }
        
        if (daemon_pid > 0){
            close(command_socket);
            return 0;
        }
    }
    
    while (running){
        // process received command
        if (command_socket >= 0){
            await_commands(command_socket);
        }
        
        // process received data tuio data

        // 5 ms timeout
        struct timespec timeout;
        timeout.tv_sec = 0;
        timeout.tv_nsec = 5 * 1000 * 1000;
        
        raw_client->load(1, timeout);
    }
    
    // cleanup
    close(command_socket);
    unlink(command_socket_path.c_str());
    for (servers_list::iterator l = servers.begin(); l != servers.end(); ++l){
        delete *l;
        *l = NULL;
    }
    servers.clear();
    
#ifdef HAVE_MUSE
    if (!muse_modules.empty()){
        muse::module_service::get_instance()->free_module_chain(muse_modules);
    }
#endif
    
    if (forwarding_listener != NULL){ delete forwarding_listener; forwarding_listener = NULL; }
    if (multiplexing_adaptor != NULL){ delete multiplexing_adaptor; multiplexing_adaptor = NULL; }
    if (raw_client != NULL){ delete raw_client; raw_client = NULL; }

    return retval;
}

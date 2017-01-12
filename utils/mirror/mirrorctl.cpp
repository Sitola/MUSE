/**
 * \file      mirrorctl.cpp
 * \brief     Provides a control utility to manipulate muse mirror
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2013-03-11 12:15 UTC+1
 * \copyright BSD
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
#include <dirent.h>
#include <kerat/kerat.hpp>

#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include "common.hpp"

using std::cout;
using std::cerr;
using std::endl;

struct mirrorctl_config {
    mirrorctl_config():port(0){}
    
    uint16_t port;
    string_list arguments;
};

// globals
static bool running = true;

// core
static void register_signal_handlers();
static void handle_kill_signal(int ev);

static void usage();

static int parse_commandline(mirrorctl_config * config, int argc, char ** argv);

static int process_command(int sockfd, const std::string & cmd, struct sockaddr_un & target){
    if (cmd.size() >= MAX_COMMAND_LENGTH){
        std::cerr << "Command is too long!" << std::endl;
        return -1;
    }
    
    ssize_t written = sendto(sockfd, cmd.c_str(), cmd.size(), 0, (struct sockaddr *)&target, sizeof(struct sockaddr_un));
    if (written != cmd.size()){
        std::cerr << "Failed to write to control socket: " << strerror(errno) << std::endl;
        return -1;
    }
    
    // allocate response buffer
    char * buffer = (char *)malloc(MAX_COMMAND_LENGTH);
    memset(buffer, 0, MAX_COMMAND_LENGTH);
    
    // wait for response for 10 sec
    struct timeval timeout;
    timeout.tv_sec = 60;
    timeout.tv_usec = 0;
    
    fd_set readfd;
    FD_SET(sockfd, &readfd);
    
    if (select(sockfd+1, &readfd, NULL, &readfd, &timeout) > 0){
        ssize_t bytes_read = recv(sockfd, buffer, MAX_COMMAND_LENGTH-1, 0);
        if (bytes_read <= 0){
            std::cerr << "Unable to receive reply..." << std::endl;
        } else {
            std::cout << buffer;
        }
    } else {
        std::cerr << "Reply timeouted..." << std::endl;
    }
    
    free(buffer);
    
    return 0;
}

static std::string get_mirrorctl_control_socket_path(const std::string & command_socket_path){
    // append pid to the filename
    std::stringstream sx;
    sx << command_socket_path;
    sx << "." << getpid();
    return sx.str();
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

int main(int argc,char **argv){

    srand(time(NULL));

    // for command line arguments

    // parse the command line auxiliary
    mirrorctl_config config;
    if (parse_commandline(&config, argc, argv) != 0){
        cerr << "Parsing command line has failed, commiting suicide" << endl;
        exit(EXIT_FAILURE);
    }

    if (!running){ exit(EXIT_SUCCESS); }

    register_signal_handlers();

    bool interactive = config.arguments.empty();
    
    struct sockaddr_un command_address;
    std::string mirrorctl_command_socket_path = "";
    int command_socket = 0;
    { // establish command channel
        std::string command_socket_path = get_control_socket_path(config.port);
        mirrorctl_command_socket_path = get_mirrorctl_control_socket_path(command_socket_path);

        if (command_socket_path.size() > 107){
            std::cerr << "Unable to access control socket - path \"" << command_socket_path << "\" is too long." << std::endl;
            return -1;         
        }
        if (mirrorctl_command_socket_path.size() > 107){
            std::cerr << "Unable to access control socket - path \"" << mirrorctl_command_socket_path << "\" is too long." << std::endl;
            return -1;         
        }

        unlink(mirrorctl_command_socket_path.c_str());
        command_socket = create_command_socket(mirrorctl_command_socket_path);
        if (command_socket < 0){
            std::cerr << "Unable to establish command socket, qutting!" << std::endl;
            close(command_socket);
            unlink(mirrorctl_command_socket_path.c_str());
            return 1;
        }

        memset(&command_address, 0, sizeof(command_address));
        command_address.sun_family = AF_UNIX;
        strncpy(command_address.sun_path, command_socket_path.c_str(), 107);
    }
    
    if (!interactive){
        std::string tmp = escape_args(config.arguments);
        process_command(command_socket, tmp, command_address);
    } else {
        std::cerr << "Command expected!" << std::endl;
    }
    
    // cleanup
    close(command_socket);
    unlink(mirrorctl_command_socket_path.c_str());
    
    return 0;
}

void usage(){
    cout << "Usage:" << endl;
    cout << "muse_mirrorctl list" << endl;
    cout << "muse_mirrorctl <port> show" << endl;
    cout << "muse_mirrorctl <port> quit" << endl;
    cout << "muse_mirrorctl <port> add [<host>[:port]]+" << endl;
    cout << "muse_mirrorctl <port> del [<host>[:port]]+" << endl;
    cout << "muse_mirrorctl <port> config <path>" << endl;
    cout << endl;
    cout << "<port>                              \t\tmuse_mirrord instance to control" << endl;
    cout << "<path>                              \t\tPath to MUSE framework XML configuration file to use as received TUIO bundle processor." << endl;
    cout << endl;
    cout.flush();
}

void list(){
    std::string path = get_control_socket_home();
    if (path.empty()){ return; }
    
    DIR * home_dir = opendir(path.c_str());
    if (home_dir == NULL){ 
        std::cerr << "Unable to open control socket home dir (" << path << ")!" << std::endl;
        return;
    }
    
    const char * mirror_base_pattern = "muse-mirror_";
    
    struct dirent * entry = NULL;
    while ((entry = readdir(home_dir)) != NULL){
        if (strncmp(entry->d_name, mirror_base_pattern, strlen(mirror_base_pattern)) == 0){
            char buffer[256];
            strcpy(buffer, entry->d_name+strlen(mirror_base_pattern));
            char * ext = strchr(buffer, '.');
            if (ext != NULL){ *ext = '\0'; }
            
            std::cout << "Found control socket for instance running on port: " << buffer << std::endl;
        }
    }
    closedir(home_dir);
}

static int parse_commandline(mirrorctl_config * config, int argc, char ** argv){

    struct option cmdline_opts[10];
    memset(&cmdline_opts, 0, sizeof(cmdline_opts));
    { int index = 0;

        cmdline_opts[index].name = "help";
        cmdline_opts[index].has_arg = 0;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'h';
        ++index;
    }

  char opt = -1;
    while ((opt = getopt_long(argc, argv, "-h", cmdline_opts, NULL)) != -1){
        switch (opt){
            case 'h': {
                usage();
                running = false;
                return EXIT_FAILURE;
                break;
            }
            default: {
                // get instance to control
                if (config->arguments.empty()){
                    if (strcmp(optarg, "list") == 0){
                        list();
                        running = false;
                        return 0;
                    } else if (config->port == 0) {
                        long tmp_port = 0;

                        char * endptr = NULL;
                        tmp_port = strtol(optarg, &endptr, 10);
                        if (*endptr != '\0'){ return 1; }

                        config->port = tmp_port;
                        continue;
                    } else if (check_if_command(optarg) != COMMAND_UNKNOWN){
                        config->arguments.push_back(optarg);
                    } else {
                        std::cerr << "Unrecognized argument \"" << optarg << "!" << std::endl;
                    }
                } else {
                    config->arguments.push_back(optarg);                
                }
            }
        }
    }

    return 0;
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
    cout << "Caught signal " << ev << ", closing up..." << endl;
    running = false; // this variable is checked in the main event loop
}

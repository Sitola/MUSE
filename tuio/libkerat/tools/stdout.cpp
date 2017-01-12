/*
 * File:   stdout.cpp
 * Author: Lukáš Ručka (359687@mail.muni.cz)
 * Organization: Masaryk University; Brno, Czech Republic
 * License: BSD
 * Created on April 11 2011, 10:34
 */


#include <iostream>
#include <string>
#include <algorithm>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <kerat/kerat.hpp>

using std::endl;
using std::cerr;
using std::cout;

using libkerat::simple_client;
using libkerat::listeners::stdout_listener;

/**
 * Struct that holds the runtime configuration
 */
struct stdout_config {
    bool multiplexing;
    uint16_t port;
};

/**
 * Make default configuration
 * @return default configuration
 */
static stdout_config get_default_config(){
    stdout_config config;
    config.multiplexing = true;
    config.port         = 3333;
    return config;
}


static void usage();
static int parse_commandline(stdout_config* config, int argc, char** argv);
static void register_signal_handlers();
static void handle_kill_signal(int ev);

bool running = true;

int main(int argc, char ** argv){

     // parse the command line auxiliary
    stdout_config config = get_default_config();
    if (parse_commandline(&config, argc, argv) != 0){
        cerr << "Parsing command line has failed, commiting suicide" << endl;
        exit(EXIT_FAILURE);
    }

    register_signal_handlers();

    stdout_listener lstnr;

    simple_client * raw_client = new simple_client(config.port);

    libkerat::client * last_client = raw_client;
    libkerat::adaptor * multiplexer = NULL;

    if (config.multiplexing){
        multiplexer = new libkerat::adaptors::multiplexing_adaptor;
        last_client->add_listener(multiplexer);
        last_client = multiplexer;
    }

    last_client->add_listener(&lstnr);

    cout << "Multiplexing " << ((config.multiplexing)?"enabled":"disabled") << endl;

    while (running){
        last_client->load();
        // do-nothink
    }

    delete raw_client;
    delete multiplexer;

    return EXIT_SUCCESS;
}

int parse_commandline(stdout_config * config, int argc, char ** argv){
    int remaining = argc-1;
    int argbase = 1;

    for (; remaining > 0;){
        if (strcmp(argv[argbase], "--help")==0){
            usage();
            return 0;
// ================ multiplexing
        } else if (strncmp(argv[argbase], "--raw", 5)==0){
            // disable multiplexing
            config->multiplexing = false;
            argbase++;
            remaining--;
// ================ port
        } else {
            char * port = (char *)argv[argbase];
            double tmp = strtod(port, NULL);
            config->port = tmp;

            if (config->port > 0xffff){
                cerr << "Invalid port " << config->port << endl;
            }

            argbase++;
            remaining--;
        }
    }
    
    return 0;
}

static void usage(){
    cout << "Usage:" << endl;
    cout << "stdout [options] [port]\t\t" << endl;
    cout << endl;
    cout << "Options:" << endl;
    cout << "--raw                               \t\tDisable multiplexing client" << endl;
    cout.flush();
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
    cout << "Caught signal " << ev << ", closing up (waiting for timeout)..." << endl;
    running = false; // this variable is checked in the main event loop
}
static std::string ip2string(int32_t ip){
    std::string retval;

    for (int i = 0; i < 4; i++){ // cause IPv4 has 4 bytes
        int32_t remainder = ip & 0xff;
        ip = ip >> 8;

        do {
            char v = remainder%10;
            remainder = remainder/10;
            retval.push_back(v+'0');
        } while (remainder != 0);

        if (i != 3){
            retval.append(".");
        }
    }

    std::reverse(retval.begin(), retval.end());
    return retval;
}


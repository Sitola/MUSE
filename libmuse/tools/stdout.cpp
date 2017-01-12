/**
 * \file      stdout.cpp
 * \author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-08-07 18:30 UTC+2
 * \copyright BSD
 */

#include <iostream>
#include <string>
#include <algorithm>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <muse/muse.hpp>
#include <tinyxml.h>
#include <getopt.h>

using std::endl;
using std::cerr;
using std::cout;

using libkerat::simple_client;
using libkerat::listeners::stdout_listener;

//! \brief Struct that holds the runtime configuration
struct stdout_config {
    uint16_t port;
    TiXmlDocument muse_config;
};

//! \brief Make default configuration
static stdout_config get_default_config(){
    stdout_config config;
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
    { // register framework with client
        libkerat::internals::convertor_list muse_convertors(muse::get_muse_convertors());
        std::for_each(muse_convertors.begin(), muse_convertors.end(), raw_client->get_enabler_functor());
    }
    
    libkerat::client * last_client = raw_client;
    muse::module_service::module_chain modules;
    
    if (!config.muse_config.NoChildren()){
        muse::module_service * modserv = muse::module_service::get_instance();
        
        const TiXmlElement * e_config = config.muse_config.RootElement();
        assert(e_config != NULL);
        const TiXmlElement * e_chain = e_config->FirstChildElement("chain");
        
        if (modserv->create_module_chain(e_chain, modules)){
            if (modules.empty()){
                std::cerr << "Failed to create module chain!" << std::endl;
            } else {
                std::cerr << "Failed to create module from following module data:" <<
                    modules.back().second->GetText() << std::endl;
            }
            modserv->free_module_chain(modules);
        }
    }

    // connect
    if (!modules.empty()){
        last_client->add_listener(modules.front().first);
        last_client = modules.back().first;
    }
    last_client->add_listener(&lstnr);
    
    // print loaded modules
    std::cout << "Modules loaded: ";
    if (modules.empty()){
        std::cout << "(empty)" << std::endl;
    } else {
        std::cout << std::endl;

        for ( 
            muse::module_service::module_chain::iterator current_module = modules.begin();
            current_module != modules.end(); 
            ++current_module
        ){
            const TiXmlElement * tmp = current_module->second;
            std::cout << "\t" << tmp->Attribute("path") << std::endl;
        }
    }
    std::cout << "--------------------------------" << std::endl;

    while (running){
        last_client->load();
        // do-nothink
    }

    muse::module_service::get_instance()->free_module_chain(modules);
    delete raw_client;

    return EXIT_SUCCESS;
}

int parse_commandline(stdout_config * config, int argc, char ** argv){

    struct option cmdline_opts[12];
    memset(&cmdline_opts, 0, sizeof(cmdline_opts));
    { int index = 0;

        cmdline_opts[index].name = "help";
        cmdline_opts[index].has_arg = 0;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'h';
        ++index;

        cmdline_opts[index].name = "port";
        cmdline_opts[index].has_arg = 1;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'p';
        ++index;
        
        cmdline_opts[index].name = "muse-config";
        cmdline_opts[index].has_arg = 1;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'm';
        ++index;
    }
    
  char opt = -1;
    while ((opt = getopt_long(argc, argv, "-hp:m:", cmdline_opts, NULL)) != -1){
        switch (opt){
            case 'h': {
                usage();
                running = false;
                return EXIT_FAILURE;
                break;
            }
            case 'p': { // ================ port
                config->port = strtol(optarg, NULL, 10);
                if (config->port > 0xffff){ cerr << "Invalid port " << config->port << endl; }
                break;
            }
            case 'm': {
                config->muse_config.LoadFile(optarg);
                if (config->muse_config.Error()){
                    std::cerr << "Failed to parse given MUSE config file!" << std::endl;
                    std::cerr << config->muse_config.ErrorDesc() << std::endl;
                    return -1;
                }
                break;
            }

            default: {
                std::cerr << "Unrecognized argument!" << std::endl;
            }
        }            
    }
/*
    if (config->muse_config.RootElement() == NULL){
        TiXmlElement e_root("muse_config");
        TiXmlElement e_chain("chain");
        TiXmlElement e_module("module");
            e_module.SetAttribute("path", "/libkerat/multiplexing_adaptor");

            e_chain.InsertEndChild(e_module);
            e_root.InsertEndChild(e_chain);
            
        config->muse_config.InsertEndChild(e_root);
    }
*/  
    return 0;
}

static void usage(){
    cout << "Usage:" << endl;
    cout << "stdout [options] [port]\t\t" << endl;
    cout << endl;
    cout << "Options:" << endl;
    cout << "--raw                   \tDisable all event transformations, print data as were received" << endl;
    cout << "--port                  \tPort to listen on" << endl;
    cout << "--muse-config=<file.xml>\tUse given MUSE framework configuration" << endl;
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


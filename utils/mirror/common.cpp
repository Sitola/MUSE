/**
 * \brief functions common for both mirror & mirrorctl
 *
 * \file      common.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2013-03-10 20:24 UTC+2
 * \copyright BSD
 */

#define _BSD_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <stdint.h>
#include <sstream>
#include <cstring>
#include <list>

#include "common.hpp"

std::string get_control_socket_home(){
    const char * tmpdir = getenv("TMPDIR");
    if (tmpdir == NULL){
        tmpdir = P_tmpdir;
    }
    
    if ((tmpdir == NULL) || (strlen(tmpdir) == 0)){
        std::cerr << "Unable to get temdir path!" << std::endl;
        exit(1);
    }
    
    return tmpdir;
}

std::string get_control_socket_path(const uint16_t port){
    std::stringstream retval;
    retval << get_control_socket_home() << "/muse-mirror_" << port << ".ctl";
    return retval.str();
}

command_t check_if_command(const std::string command_candidate){
    if (command_candidate.compare("quit") == 0){
        return COMMAND_STOP;
    } else if (command_candidate.compare("stop") == 0){
        return COMMAND_STOP;
    } else if (command_candidate.compare("show") == 0){
        return COMMAND_SHOW;
    } else if (command_candidate.compare("add") == 0){
        return COMMAND_ADD;
    } else if (command_candidate.compare("del") == 0){
        return COMMAND_DEL;
    } else if (command_candidate.compare("config") == 0){
        return COMMAND_LOAD_CONFIG;
    } 
    
    return COMMAND_UNKNOWN;
}

string_list unescape_args(const std::string & input){
    string_list retval;
    
    bool ignore_space = false;
    char escape = '\0';
    bool previous_escape = false;
    std::string tmp;
    
    for (std::string::const_iterator i = input.begin(); i != input.end(); ++i){
        if (previous_escape){
            tmp.push_back(*i);
            previous_escape = false;
            continue;
        } else if (*i == '\\'){
            previous_escape = true; continue;
        } else if (isspace(*i)){
            if (!ignore_space){
                retval.push_back(tmp);
                tmp.clear();
                continue;
            } else {
                tmp.push_back(*i);
            }
        } else if (*i == escape) {
            escape = '\0';
            continue;
        } else if ((escape == '\0') && ((*i == '"') || (*i == '\''))){ // the escape test should not actually be needed at all...
            escape = *i;
            continue;
        } else {
            tmp.push_back(*i);
        }
    }
    
    if (!tmp.empty()){
        retval.push_back(tmp);
    }
    
    return retval;
}

std::string escape_args(const string_list & args){
    std::string retval;
    
    for (string_list::const_iterator item = args.begin(); item != args.end(); ++item){
        retval.push_back('"');
        
        for (std::string::const_iterator i = item->begin(); i != item->end(); ++i){
            if (*i == '\\'){
                retval.append("\\\\");
            } else if (*i == '"') {
                retval.append("\\\"");
            } else {
                retval.push_back(*i);
            }
        }

        retval.push_back('"');
        retval.push_back(' ');
    }
    
    return retval;
}

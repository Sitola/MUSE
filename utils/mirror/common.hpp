#ifndef COMMON_HPP
#define	COMMON_HPP

#include <stdint.h>
#include <cstdlib>
#include <string>

const ssize_t MAX_COMMAND_LENGTH = 65536;

typedef enum {
    COMMAND_UNKNOWN,
    COMMAND_STOP,
    COMMAND_ADD,
    COMMAND_DEL,
    COMMAND_SHOW,
    COMMAND_LOAD_CONFIG,
} command_t;

typedef std::list<std::string> string_list;

std::string get_control_socket_home();
std::string get_control_socket_path(const uint16_t port);

command_t check_if_command(const std::string command_candidate);

string_list unescape_args(const std::string & input);
std::string escape_args(const string_list & args);
    
#endif	/* MIRROR_HPP */


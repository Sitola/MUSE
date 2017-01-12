/**
 * @file      eventdumper.hpp
 * @brief     Provides the "prettyprint" functionality for kernel event messages
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2011-08-23 11:19 UTC+2
 * @copyright BSD
 */

const char * get_event_type(int ev);
const char * get_abs_ev_name(int code);
const char * get_abs_ev_name_short(int code);
const char * get_syn_ev_name(int code);
const char * get_misc_ev_name(int code);
const char * get_btn_ev_name(int code);
const char * get_event_desc(int type, int code);
void format(struct input_event data);
ev_code_t get_evcode_for_name(const char * name);

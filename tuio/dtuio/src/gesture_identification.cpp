/**
 * \file      gesture_identification.cpp
 * \brief     Implement the message that identifies the gesture as recognized by given parser
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2013-02-26 11:07 UTC+2
 * \copyright BSD
 */

#ifdef HAVE_CONFIG_H
    #include "../config.h"
#endif

#include <dtuio/gesture_identification.hpp>
#include <dtuio/helpers.hpp>
#include <dtuio/parsers.hpp>
#include <kerat/utils.hpp>
#include <uuid/uuid.h>

namespace dtuio {
    namespace gesture {

        const char * gesture_identification::PATH = "/dtuio/gestid";


        gesture_identification::~gesture_identification(){ ; }

        gesture_identification::gesture_identification(
            const gesture_identification::recognized_gesture_map& gestures,
            libkerat::user_id_t uid,
            const libkerat::session_set & sessions,
            const std::string& recognizer_tag
        ){
            set_user(uid);
            set_gestures(gestures);
            set_session_ids(sessions);
            set_recognizer_tag(recognizer_tag);
        }

        gesture_identification::gesture_identification(
            const gesture_identification::aux_recognized_gesture_map& gestures,
            libkerat::user_id_t uid,
            const libkerat::session_set & sessions,
            const std::string& recognizer_tag
        ){
            set_user(uid);
            set_gestures(gestures);
            set_session_ids(sessions);
            set_recognizer_tag(recognizer_tag);
        }

        gesture_identification::gesture_identification(const gesture_identification & original){
            set_user(original.m_user);
            set_session_ids(original.m_ids);
            set_recognizer_tag(original.m_recognizer_tag);
            set_gestures(original.m_gestures);
        }

        libkerat::kerat_message * gesture_identification::clone() const { return new gesture_identification(*this); }

        void gesture_identification::set_gestures(const gesture_identification::recognized_gesture_map& gestures){
            m_gestures = gestures;
        }

        void gesture_identification::set_gestures(const gesture_identification::aux_recognized_gesture_map& gestures){
            m_gestures.clear();
            for (aux_recognized_gesture_map::const_iterator i = gestures.begin(); i != gestures.end(); ++i){
                m_gestures.insert(recognized_gesture_map::value_type(i->second, i->first));
            }
        }

        void gesture_identification::set_recognizer_tag(const std::string& recognizer_tag){
            m_recognizer_tag = recognizer_tag;
        }

        void gesture_identification::set_user(libkerat::user_id_t uid){
            m_user = uid;
        }

        void gesture_identification::set_session_ids(const libkerat::session_set& involved){
            m_ids = involved;
        }

        void gesture_identification::print(std::ostream & output) const { output << *this; }

        bool gesture_identification::imprint_lo_messages(lo_bundle target) const {

            if (target == NULL){ return false; }
            lo_message msg = lo_message_new();

            if (!get_recognizer_tag().empty()){
                lo_message_add_string(msg, get_recognizer_tag().c_str());
            }

            lo_message_add_int32(msg, get_user());

            const recognized_gesture_map & gestures = get_gestures();
            for (recognized_gesture_map::const_iterator i = gestures.begin(); i != gestures.end(); ++i){
                lo_message_add_float(msg, i->first);
                lo_message_add_string(msg, i->second.c_str());
            }

            for (libkerat::session_set::const_iterator i = m_ids.begin(); i != m_ids.end(); ++i){
                lo_message_add_int32(msg, *i);
            }

            lo_bundle_add_message(target, PATH, msg);
            return true;

        }
    }

    namespace internals { namespace parsers {
        bool parse_gesture_identification(
            libkerat::internals::convertor_output_container & results,
            const char * path,
            const char * types,
            lo_arg ** argv,
            int argc __attribute__((unused)),
            void * user_data __attribute__((unused))
        ){
            results.clear();
            // check for message type
            if (strcmp(path, dtuio::gesture::gesture_identification::PATH) != 0){ return false; }

            // at least user attribute has to be always present
            if (argc < 1){ return false; }

            typedef dtuio::gesture::gesture_identification::recognized_gesture_map recognized_gesture_map;

            int argbase = 0;

            // check for recognizer tag
            std::string tag;
            if (types[0] == 's'){
                tag = &argv[0]->s;
                ++argbase;
            }

            libkerat::user_id_t uid = libkerat::helpers::contact_type_user::UID_NOUSER;
            // get user
            if (types[argbase] == 'i'){
                uid = argv[argbase]->i;
                ++argbase;
            }

            // fill-in gesture vector
            recognized_gesture_map gestures;
            for (; (argbase+1 < argc) && (strncmp(types+argbase, "fs", 2) == 0); argbase += 2){
                gestures.insert(recognized_gesture_map::value_type(argv[argbase]->f, &argv[argbase+1]->s));
            }

            // fill-in the session id's sent
            libkerat::session_set sessions;
            for (; (argbase < argc) && (types[argbase] == 'i'); ++argbase){
                sessions.insert(argv[argbase]->i);
            }

            // check whether all arguments were consumed
            if (argbase != argc){ return false; }

            dtuio::gesture::gesture_identification * msg_gstid =
                new dtuio::gesture::gesture_identification(gestures, uid, sessions, tag);
            results.push_back(msg_gstid);

            return true;
        }
    } }
}

std::ostream & operator<<(std::ostream & output, const dtuio::gesture::gesture_identification & gesture){
    using dtuio::gesture::gesture_identification;

    output << gesture_identification::PATH;
    // uid
    output << " uid=" << gesture.get_user();
    // tag
    if (!gesture.get_recognizer_tag().empty()){
        output << " (" << gesture.get_recognizer_tag() << ")";
    }
    // gesture probability
    const gesture_identification::recognized_gesture_map & gestures = gesture.get_gestures();
    for (gesture_identification::recognized_gesture_map::const_iterator i = gestures.begin(); i != gestures.end(); ++i){
        output << " " << i->first << "(" << i->second << ")";
    }
    // session id's
    output << " involved_ids=[";
    const libkerat::session_set & sessions = gesture.get_session_ids();
    for (libkerat::session_set::const_iterator i = sessions.begin(); i != sessions.end(); ++i){
        if (i != sessions.begin()){ output << ", "; }
        output << *i;
    }
    output << "]";

    return output;
}

/**
 * \file      gesture_identification.hpp
 * \brief     Define the message that identifies the gesture as recognized by given parser
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2013-02-26 11:07 UTC+2
 * \copyright BSD
 */


#ifndef DTUIO_GESTURE_IDENTIFICATION_HPP
#define DTUIO_GESTURE_IDENTIFICATION_HPP

#include <kerat/kerat.hpp>
#include <dtuio/typedefs.hpp>
#include <dtuio/helpers.hpp>
#include <map>
#include <string>
#include <stdexcept>

namespace dtuio {
    //! \brief Messages representing individual aspects of gesture recognition
    namespace gesture {
        /**
         * \brief Recognized gestures probability map
         * 
         * The MUSE framework handles the gestures as gesture candidates,
         * for example in some environments the pinch gesture can be similar to
         * other gestures. If the recognizer module provided only single gesture
         * candidate and pinch was "more similar" to received data, the real expected
         * gesture could be lost. Instead, the MUSE provides the recognized gestures
         * as probability map with template name as key and normalized (0, 1] template
         * match probability.
         * 
         * The client applications are suggested to name the gesture templates in
         * hierarchical namespaces, eg. sage.window.zoom.
         * 
         * \todo Support for parametric gestures.
         */
        class gesture_identification: public libkerat::kerat_message {
        public:
            //! \brief gesture probability (normalized float) and template name
            //! \brief (std::string) multimap ordered from most probable to least
            typedef std::multimap<float, std::string, std::greater<float> > recognized_gesture_map;
            
            //! \brief gesture name (std::string) and probability (normalized float) map (auxiliary)
            typedef std::map<std::string, float> aux_recognized_gesture_map;
            
            /**
             * \param gestures gesture probability multimap this message shall carry
             * (if left unset, an empty multimmap is created)
             * \param uid user ID of the user that commanced the gesture
             * \param sessions set of session ID's involved in the gesture
             * \param recognizer_tag optional gesture recognizer signature (eg. /muse/recognizer/protractor)
             */
            gesture_identification(
                const recognized_gesture_map & gestures = recognized_gesture_map(), 
                libkerat::user_id_t uid = libkerat::helpers::contact_type_user::UID_NOUSER, 
                const libkerat::session_set & sessions = libkerat::session_set(),
                const std::string & recognizer_tag = ""
            );

            /**
             * \param gestures gesture probability map this message shall carry
             * (string->float), shall be converted to proper representation
             * \param uid user ID of the user that commanced the gesture
             * \param sessions set of session ID's involved in the gesture
             * \param recognizer_tag optional gesture recognizer signature (eg. /muse/recognizer/protractor)
             */
            gesture_identification(
                const aux_recognized_gesture_map & gestures, 
                libkerat::user_id_t uid = libkerat::helpers::contact_type_user::UID_NOUSER, 
                const libkerat::session_set & sessions = libkerat::session_set(),
                const std::string & recognizer_tag = ""
            );

            gesture_identification(const gesture_identification & original);
            virtual ~gesture_identification();
            
            /**
             * \brief Set the optional gesture recognizer signature
             * \param recognizer_tag recognizer signature (eg. /muse/recognizer/protractor)
             */
            void set_recognizer_tag(const std::string & recognizer_tag);
            
            //! \brief Get the optional recognizer signature
            //! \returns Signature as string or empty string if unset
            inline const std::string & get_recognizer_tag() const { 
                return m_recognizer_tag; 
            }
            
            //! \brief Sets the gesture match probability multimap this message shall carry
            //! \param gestures gesture (normalized probability, name) multimap
            void set_gestures(const recognized_gesture_map & gestures);
            
            //! \brief Sets the gesture match probability multimap this message shall carry
            //! \param gestures gesture (name, normalized probability) map to convert & set
            void set_gestures(const aux_recognized_gesture_map & gestures);
            
            //! \brief Get the recognized gestures candidates
            //! \returns probability, name multimap ordered from most probable candidate to the least probable
            inline const recognized_gesture_map & get_gestures() const {
                return m_gestures;
            }
            
            //! \brief Sets user ID of the user that commanced the gesture
            //! \param uid user's UID
            void set_user(libkerat::user_id_t uid);
            
            //! \brief Get the user ID of the user that commanced the gesture
            //! \returns uid or UID_NOUSER (0) if unset
            inline libkerat::user_id_t get_user() const { return m_user; }
            
            //! \brief Sets the session ID's involved in the gesture
            //! \param involved set of session ID's involved
            void set_session_ids(const libkerat::session_set & involved);
            
            //! \beirf Gets the session ID's involved in this gesture
            //! \returns set of session ID's or empty set if unset
            inline const libkerat::session_set & get_session_ids() const { return m_ids; };
                
            libkerat::kerat_message * clone() const ;
            
            void print(std::ostream & output) const ;
                
            //! \brief OSC message path for this dTUIO message
            static const char * PATH;
        private:
            bool imprint_lo_messages(lo_bundle target) const;
            
            std::string m_recognizer_tag;
            libkerat::user_id_t m_user;
            libkerat::session_set m_ids;
            recognized_gesture_map m_gestures;
        };
    }
}

std::ostream & operator<<(std::ostream & output, const dtuio::gesture::gesture_identification & gesture);

#endif // DTUIO_GESTURE_IDENTIFICATION_HPP

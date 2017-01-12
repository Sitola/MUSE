/**
 * \file      tuio_message_signal.hpp
 * \brief     TUIO 2.0 signal (/tuio2/sig)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-11-19 18:22 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGE_SIGNAL_HPP
#define KERAT_TUIO_MESSAGE_SIGNAL_HPP

#include <string>
#include <kerat/typedefs.hpp>
#include <kerat/message.hpp>
#include <kerat/message_helpers.hpp>
#include <set>
#include <ostream>

namespace libkerat {

    namespace message {

        /**
         * \brief TUIO 2.0 signal (/tuio2/sig)
         */
        class signal: public libkerat::kerat_message,
            public libkerat::helpers::contact_session,
            public libkerat::helpers::contact_component
        {
        public:
            typedef std::set<session_id_t> target_ids;

            //! \brief Creates a new empty signal message
            signal();

            //! \brief Deep copy given message
            signal(const signal & original);

            /**
             * \brief Creates a new signal message containing from given source and signal
             * \param session_id - session id of the source contact
             * \param component_id - signal component id
             */
            signal(const session_id_t session_id, const component_id_t & component_id);

            /**
             * \brief Creates a new signal message containing from given source and signal and targets
             * \param session_id - session id of the source contact
             * \param component_id - signal component id
             * \param targets - signal these session id's
             */
            signal(const session_id_t session_id, const component_id_t & component_id, const target_ids & targets);

            signal & operator=(const signal & second);
            bool operator == (const signal & second) const ;
            inline bool operator != (const signal & second) const { return !operator==(second); }

            /**
             * \brief Gets the targets set
             * \return set of session id targeted by this signal
             */
            inline target_ids get_targets() const {
                target_ids retval;
                retval = m_targets;
                return retval;
            }

            /**
             * \brief Sets the targets set
             * \param targets - set of session id targeted by this signal
             * \return previous setting
             */
            target_ids set_targets(const target_ids & targets);

            kerat_message * clone() const ;

            void print(std::ostream & output) const ;

            //! OSC path for TUIO 2.0 signal message
            static const char * PATH;

        private:

            target_ids m_targets;

            bool imprint_lo_messages(lo_bundle target) const;

        }; // cls signal
        
    } // ns message
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::signal &  msg_sig);

#endif // KERAT_TUIO_MESSAGE_SIGNAL_HPP

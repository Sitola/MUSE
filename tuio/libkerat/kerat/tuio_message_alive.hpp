/**
 * \file      tuio_message_alive.hpp
 * \brief     TUIO 2.0 alive (/tuio2/alv)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-09-26 18:21 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGE_ALIVE_HPP
#define KERAT_TUIO_MESSAGE_ALIVE_HPP

#include <string>
#include <kerat/typedefs.hpp>
#include <kerat/message.hpp>
#include <set>
#include <ostream>

namespace libkerat {
    namespace message {

        /**
         * \brief TUIO 2.0 alive (/tuio2/alv)
         *
         * \see operator<<(std::ostream & output, const libkerat::message::alive &  msg_alv)
         */
        class alive: public libkerat::kerat_message {
        public:
            typedef libkerat::session_set alive_ids;

            //! \brief Create a new empty alive message
            alive(){
                ;
            }

            //! \brief Creates a copy of given alive message
            alive(const alive & original);

            /**
             * \brief Creates a new alive message containing the given session id's
             * \param alives_to_add - set of session id's present
             */
            alive(const alive_ids & alives_to_add);

            alive & operator=(const alive & second);
            bool operator == (const alive & second) const ;
            inline bool operator != (const alive & second) const { return !operator==(second); }

            /**
             * \brief Gets the whole set of alives
             * \return alives stored in the message
             */
            inline const alive_ids & get_alives() const { return m_alives; }

            /**
             * \brief Sets the alive session id's held in this alive message
             * \param alives - set of session id's present
             * \return previous setting
             */
            alive_ids set_alives(const alive_ids & alives);

            //! \brief Clears the alive set being held
            inline void clear(){ m_alives.clear(); }

            kerat_message * clone() const ;

            void print(std::ostream & output) const ;

            //! OSC path for TUIO 2.0 alive
            static const char * PATH;

        private:

            alive_ids m_alives;

            bool imprint_lo_messages(lo_bundle target) const;


        }; // cls alive
    } // ns message
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::alive &  msg_alv);

#endif // KERAT_TUIO_MESSAGE_ALIVE_HPP

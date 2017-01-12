/**
 * \file      tuio_message_alive_associations.hpp
 * \brief     TUIO 2.0 alive associations (/tuio2/ala)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-11-19 19:21 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGE_ALIVE_ASSOCIATIONS_HPP
#define KERAT_TUIO_MESSAGE_ALIVE_ASSOCIATIONS_HPP

#include <string>
#include <kerat/typedefs.hpp>
#include <kerat/message.hpp>
#include <set>
#include <ostream>

namespace libkerat {
    namespace message {

        //! \brief TUIO 2.0 alive associations (/tuio2/ala)
        class alive_associations: public libkerat::kerat_message {
        public:
            //! type alias for associated id's set
            typedef libkerat::session_set associated_ids;

            //! \brief Creates a new empty alive associations message
            alive_associations(){ ; }

            /**
             * \brief Creates a copy of given alive associations message
             * \param original - alive associations message to make copy of
             */
            alive_associations(const alive_associations & original);

            /**
             * \brief Creates a new alive associations message containing the given session id's
             * \param associations_to_add - set of session id's that are in associated state
             */
            alive_associations(const associated_ids & associations_to_add);

            alive_associations & operator=(const alive_associations & second);
            bool operator == (const alive_associations & second) const ;
            inline bool operator != (const alive_associations & second) const { return !operator==(second); }

            /**
             * \brief Get the whole set of associated id's
             * \return associated stored in the message
             */
            inline const associated_ids & get_associations() const { return m_associations; }

            /**
             * \brief Sets the associated session id's held in this message
             * \param associations - set of session id's that are in associated state
             * \return previous setting
             */
            associated_ids set_associations(const associated_ids & associations);

            //! \brief Clears the associated list held
            inline void clear(){ m_associations.clear(); }

            kerat_message * clone() const ;

            void print(std::ostream & output) const ;

            //! OSC path for TUIO 2.0 alive associations
            static const char * PATH;

        private:

            associated_ids m_associations;

            bool imprint_lo_messages(lo_bundle target) const;

        }; // cls alive_associations
    } // ns message
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::alive_associations &  msg_ala);

#endif // KERAT_TUIO_MESSAGE_ALIVE_ASSOCIATIONS_HPP

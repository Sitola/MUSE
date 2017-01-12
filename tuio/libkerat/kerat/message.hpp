/**
 * \file      message.hpp
 * \brief     Provides the abstract class that all libkerat TUIO (and TUIO extensions) compatible messages must inherit.
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-09-19 13:55 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_MESSAGE_HPP
#define KERAT_MESSAGE_HPP

#include <kerat/typedefs.hpp>
#include <lo/lo.h>
#include <ostream>

namespace libkerat {

    class server;

    //! \brief A common ancestor for all message classes handled by the kerat library
    class kerat_message {
    public:

        virtual ~kerat_message(){ ; }

        /**
         * \brief Make copy of this message
         * \note Caller is responsible for deallocation
         * \return Pointer to new message copy
         */
        virtual kerat_message * clone() const = 0;

        /**
         * \brief Calls output &lt;&lt; *this
         * \param output - output stream to write to
         */
        virtual void print(std::ostream & output) const = 0;

    protected:
        kerat_message(){ ; }

        friend class server;

        /**
         * \brief Imprints the target bundle with OSC messages
         * 
         * \note Multiple OSC messages can be generated from single message object
         * \return true if the messages were successfully generated and imprinted, 
         * false otherwise
         */
        virtual bool imprint_lo_messages(lo_bundle target) const = 0;

    }; // cls kerat_message

} // ns libkerat

#endif // KERAT_MESSAGE_HPP

/**
 * \file      tuio_message_control.hpp
 * \brief     TUIO 2.0 control (/tuio2/ctl)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-11-18 22:38 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGE_CONTROL_HPP
#define KERAT_TUIO_MESSAGE_CONTROL_HPP

#include <kerat/message.hpp>
#include <kerat/typedefs.hpp>
#include <kerat/message_helpers.hpp>
#include <kerat/utils.hpp>
#include <string>
#include <stdexcept>
#include <vector>
#include <ostream>

namespace libkerat {
    namespace message {

        //! \brief TUIO 2.0 control (/tuio2/ctl)
        class control: public libkerat::kerat_message,
            public libkerat::helpers::contact_session
        {
        public:
            typedef std::vector<float> controls_list;

            //! \brief Creates new, empty TUIO control message
            control();

            /**
             * \brief Creates a copy of given control message
             * \param original - message to create copy of
             */
            control(const control& original);

            /**
             * \brief Creates a new ctl message containing the given ID's
             * \param session_id - session id of the control command sender contact
             * \param controls - vector of float values representing individual control dimmensions
             */
            control(const session_id_t session_id, const controls_list & controls) throw (std::out_of_range);

            control & operator=(const control & second);
            bool operator == (const control & second) const ;
            inline bool operator != (const control & second) const { return !operator==(second); }

            /**
             * \brief Gets the vector of assigned control dimmensions
             * \return controls stored in the message
             */
            inline const controls_list & get_controls() const { return m_controls; }

            /**
             * \brief Sets the control dimmensions
             * \param controls - control dimmensions to be set
             * \return previous setting
             * \throw std::out_of_range if any of the controls is outside the
             * normalized range (see \ref libkerat::is_not_normalized "is_not_normalized")
             */
            controls_list set_controls(const controls_list & controls) throw (std::out_of_range);

            //! \brief Clears the control list held
            inline void clear(){ m_controls.clear(); }

            kerat_message * clone() const ;

            void print(std::ostream & output) const ;

            //! \brief OSC path for the TUIO 2.0 control message
            static const char * PATH;

        private:

            bool imprint_lo_messages(lo_bundle target) const;

            controls_list m_controls;

        }; // cls control

    } // ns message
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::control &  msg_ctl);

#endif // KERAT_TUIO_MESSAGE_CONTROL_HPP

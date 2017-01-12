/**
 * \file      tuio_message_raw.hpp
 * \brief     TUIO 2.0 raw (/tuio2/raw)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-04-29 21:12 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGE_RAW_HPP
#define KERAT_TUIO_MESSAGE_RAW_HPP

#include <kerat/message.hpp>
#include <kerat/typedefs.hpp>
#include <kerat/message_helpers.hpp>
#include <string>
#include <ostream>

namespace libkerat {

    namespace message {

        //! \brief TUIO 2.0 raw (/tuio2/raw)
        class raw: public libkerat::kerat_message,
            public libkerat::helpers::contact_session
        {
        public:

            //! Creates new, empty TUIO raw message.
            raw();

            /**
             * \brief Creates new raw message containging given samples
             *
             * \param session_id - session id of the raw, unique for contact
             * \param width      - the corresponding normalized distance between individual samples
             * \param samples_count - count of samples
             * \param samples       - the samples
             */
            raw(const session_id_t session_id, const distance_t width, const size_t samples_count, const uint8_t * samples);

            //! \brief Create deep copy
            raw(const raw & original);

            //! \brief Destructor, DOES deallocate the data buffers
            ~raw();

            raw & operator=(const raw & original);
            bool operator==(const raw & second) const;

            inline bool operator !=(const raw & second) const { return !operator==(second); }

            /**
             * \brief Alias for \ref get_sample_distance
             * \note Included for compatibility with TUIO 2.0 draft text
             */
            inline distance_t get_width() const { return get_sample_distance(); }

            /**
             * \brief Gets the distance between individual samples
             * \return sample distance or 0 if not set
             */
            inline distance_t get_sample_distance() const { return m_width; }

            /**
             * \brief Alias for \ref set_sample_distance
             * \note Included for compatibility with TUIO 2.0 draft text
             */
            inline distance_t set_width(distance_t sample_distance) {
                distance_t oldval = m_width;
                m_width = sample_distance;
                return oldval;
            }

            /**
             * \brief Sets the distance between individual samples
             * \param sample_distance - distance between individual samples
             * \return previous setting
             */
            distance_t set_sample_distance(distance_t sample_distance) ;

            /**
             * \brief Sets the samples
             * Stores copy of given samples
             * \param samples_count - ammount of samples to be copied. If 0, no raw is copyed and the raw pointer is set to NULL
             * \param samples - samples to make copy of
             */
            void set_samples(const uint32_t samples_count, const uint8_t * samples);

            /**
             * \brief Gets the samples stored in
             * \return pointer to samples (each sample is 8-bit value)
             */
            inline const uint8_t * get_samples() const { return m_samples; }

            /**
             * \brief Gets count of samples stored in
             * \return count of samples stored in
             */
            inline uint32_t get_samples_count() const { return m_samples_count; }

            kerat_message * clone() const ;

            void print(std::ostream & output) const ;

            //! OSC path for TUIO 2.0 raw
            static const char * PATH;

        private:

            //! \brief Dealocate previously allocated samples safely
            void do_safe_dealloc();

            bool imprint_lo_messages(lo_bundle target) const;

            //! \brief nonsence, should be named m_sample_distance - width is used in draft text
            distance_t m_width;
            uint32_t m_samples_count;
            uint8_t * m_samples;

        }; // cls raw

    } // ns message
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::raw &  msg_raw);

#endif // KERAT_TUIO_MESSAGE_RAW_HPP

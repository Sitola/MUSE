/**
 * \file      tuio_message_data.hpp
 * \brief     TUIO 2.0 data (/tuio2/dat)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-11-12 23:15 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGE_DATA_HPP
#define KERAT_TUIO_MESSAGE_DATA_HPP

#include <kerat/message.hpp>
#include <kerat/typedefs.hpp>
#include <kerat/message_helpers.hpp>
#include <string>
#include <ostream>

namespace libkerat {
    namespace message {

        //! \brief TUIO 2.0 data (/tuio2/dat)
        class data: public libkerat::kerat_message,
            public libkerat::helpers::contact_session
        {
        public:

            //! \brief Type of data being held
            typedef enum {DATA_TYPE_STRING, DATA_TYPE_BLOB} data_type;

            /**
             * \brief Creates new, empty TUIO data
             *
             * The MIME type is set to "text/plain", data is set to empty string
             */
            data();

            /**
             * \brief Creates new data message containging given string
             *
             * \param session_id - session id of the data, unique for contact
             * \param data       - the data string of this data message
             * \param mime       - the MIME type of this data message, defaults to "text/plain"
             */
            data(const session_id_t session_id, const std::string & data, const std::string & mime = "text/plain");

            /**
             * \brief Creates new data message containging given data blob
             *
             * \param session_id - session id of the data, unique for contact
             * \param data_len  - length of the data to be stored (in bytes)
             * \param data       - pointer to data to be stored
             * \param mime       - the MIME type of the data, defaults to "application/octet-stream"
             */
            data(const session_id_t session_id, const uint32_t data_len, const void * data, const std::string & mime);

            //! \brief Creates a deep copy of the given data message
            data(const data & original);

            //! \brief Destructor, DOES deallocate the data buffers
            ~data();

            //! \brief Asignment, runs deep copy
            data & operator=(const data & original);

            bool operator==(const data & second) const;

            inline bool operator !=(const data & second) const { return !operator==(second); }

            /**
             * \brief Get the stored data MIME type
             * \return String containing MIME type of the stored data
             */
            inline std::string get_mime_type() const { return m_mime_type; }

            /**
             * \brief Set the MIME type that this data holds
             * \param mime - MIME type of the data
             * \return previous setting
             */
            inline std::string set_mime_type(const std::string & mime){
                std::string retval = m_mime_type;
                m_mime_type = mime;
                return retval;
            }

            /**
             * \brief Get type of stored data.
             * \return type of stored data, either \ref DATA_TYPE_STRING or \ref DATA_TYPE_BLOB
             */
            inline data_type get_data_type() const { return m_type; }

            /**
             * \brief Get the length of stored data.
             * \return length of the stored data.
             */
            inline uint32_t get_data_length() const { return m_data_length; }

            /**
             * \brief Returns the pointer to stored data.
             *
             * Check the stored data type received by call to \ref get_data_type(). If \ref get_data_type() returns
             * \ref DATA_TYPE_STRING, the received pointer should be typecasted to const char *
             * \return pointer to stored data or NULL if no data stored (or stored data with zero length)
             */
            inline const void * get_data() const { return m_data; }

            /**
             * \brief Sets the message data to given string.
             * \note Sets the data type to \ref DATA_TYPE_STRING
             * \param data - string to store copy of
             */
            void set_data(const std::string & data);

            /**
             * \brief Sets the data data to copy of given data.
             * \note Sets the data type to \ref DATA_TYPE_BLOB
             * \note If data_length \<= 0, no data is copyied and the data pointer is set to NULL
             * \param data_length - size of data copy
             * \param data - data to store copy of
             */
            void set_data(const uint32_t data_length, const void * data);

            kerat_message * clone() const ;

            void print(std::ostream & output) const ;

            //! OSC path for TUIO 2.0 data message
            static const char * PATH;

        private:

            //! \brief Dealocate previously allocated data safely
            void do_safe_dealloc();

            bool imprint_lo_messages(lo_bundle target) const;

            std::string m_mime_type;

            data_type m_type;
            uint32_t m_data_length;
            uint8_t * m_data;

        }; // cls data

    }
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::data &  msg_dat);

#endif // KERAT_TUIO_MESSAGE_DATA_HPP

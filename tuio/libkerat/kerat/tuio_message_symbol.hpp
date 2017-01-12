/**
 * \file      tuio_message_symbol.hpp
 * \brief     TUIO 2.0 symbol (/tuio2/sym)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-10-30 23:31 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGE_SYMBOL_HPP
#define KERAT_TUIO_MESSAGE_SYMBOL_HPP

#include <kerat/message.hpp>
#include <kerat/typedefs.hpp>
#include <kerat/message_helpers.hpp>
#include <string>
#include <ostream>

namespace libkerat {
    namespace message {

        //! \brief TUIO 2.0 symbol (/tuio2/sym)
        class symbol: public libkerat::kerat_message,
            public libkerat::helpers::contact_session,
            public libkerat::helpers::contact_type_user,
            public libkerat::helpers::contact_component
        {
        public:

            //! \brief Type of symbol data being held
            typedef enum {DATA_TYPE_STRING, DATA_TYPE_BLOB} data_type;

            /**
             * \brief Creates new, empty TUIO symbol
             * The group is set to empty string, data value is set to NULL with type string and length 0
             */
            symbol();

            /**
             * \brief Creates new symbol message containging given string
             *
             * \param session_id - session id of the contact
             * \param type_id    - type id of the contact (see corresponding constants of the \ref helpers::contact_type_user helper)
             * \param user_id    - id of the user manipulating the contact (see corresponding constants of the \ref helpers::contact_type_user helper)
             * \param component_id - component id of the contact - each contact can have several components
             * \param group      - symbol group that this symbol data belongs to
             * \param data       - the data string of this symbol
             */
            symbol(const session_id_t session_id, const type_id_t type_id, const user_id_t user_id, const component_id_t component_id,
                const std::string & group, const std::string & data
            );

            /**
             * \brief Creates new symbol message containging given data blob
             *
             * \param session_id - session id of the contact
             * \param type_id    - type id of the contact (see corresponding constants of the \ref helpers::contact_type_user helper)
             * \param user_id    - id of the user manipulating the contact (see corresponding constants of the \ref helpers::contact_type_user helper)
             * \param component_id - component id of the contact - each contact can have several components
             * \param group      - symbol group that this symbol data belongs to
             * \param data_len   - length of the data to be stored (in bytes)
             * \param data       - pointer to data being stored
             */
            symbol(const session_id_t session_id, const type_id_t type_id, const user_id_t user_id, const component_id_t component_id,
                const std::string & group, const uint32_t data_len, const void * data
            );

            //! \brief Deep copy this message
            symbol(const symbol & original);

            //! \brief Destructor, DOES deallocate the data buffers
            ~symbol();

            symbol & operator=(const symbol & original);

            bool operator==(const symbol & second) const;
            bool operator!=(const symbol & second) const;

            /**
             * \brief Get the symbol group this symbol belongs to
             * \return symbol group or empty string if not set
             */
            inline std::string get_group() const { return m_group; }

            /**
             * \brief Set the group that this data belongs to
             * \param group_name - name of the symbol group this symbol belongs to
             * \return prevous setting
             */
            std::string set_group(const std::string & group_name){
                std::string retval = m_group;
                m_group = group_name;
                return retval;
            }


            /**
             * \brief Get type of stored symbol data.
             * \return type of stored symbol data, either \ref DATA_TYPE_STRING or \ref DATA_TYPE_BLOB
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
             * Check the stored symbol data type received by call to \ref get_data_type(). If \ref get_data_type() returns
             * \ref DATA_TYPE_STRING, the received pointer should be typecasted to const char *
             * \return pointer to stored symbol data or NULL if no data stored (or stored data with zero length)
             */
            inline const void * get_data() const { return m_data; }

            /**
             * \brief Sets the symbol data to given string.
             * \note Sets the data type to \ref DATA_TYPE_STRING
             * \param data - string to store copy of
             */
            void set_data(const std::string & data);

            /**
             * \brief Sets the symbol data to copy of given data.
             * \note Sets the data type to \ref DATA_TYPE_BLOB
             * \note If data_length \<= 0, no data is copyied and the data pointer is set to NULL
             * \param data_length - size of data copy
             * \param data - data to store copy of
             */
            void set_data(const uint32_t data_length, const void * data);

            kerat_message * clone() const ;

            void print(std::ostream & output) const ;

            //! OSC path for TUIO 2.0 symbol message
            static const char * PATH;

        private:

            //! \brief Dealocate previously allocated data safely
            void do_safe_dealloc();

            bool imprint_lo_messages(lo_bundle target) const;

            std::string m_group;

            data_type m_type;
            uint32_t m_data_length;
            uint8_t * m_data;

        }; // cls symbol

    } // ns message
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::symbol &  msg_sym);

#endif // KERAT_TUIO_MESSAGE_SYMBOL_HPP

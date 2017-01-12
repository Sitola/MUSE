/**
 * \file      tuio_message_frame.hpp
 * \brief     TUIO 2.0 frame (/tuio2/frm)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-09-26 18:21 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGE_FRAME_HPP
#define KERAT_TUIO_MESSAGE_FRAME_HPP

#include <string>
#include <kerat/typedefs.hpp>
#include <kerat/message.hpp>
#include <kerat/utils.hpp>
#include <ostream>

namespace libkerat {
    namespace message {

        //! \brief TUIO 2.0 frame (/tuio2/frm)
        class frame: public libkerat::kerat_message {
        public:

            /**
             * \brief Creates new frame message with the current event timestamp
             * \param frame_id - id of the frame
             */
            frame(frame_id_t frame_id);

            /**
             * \brief Creates new, short frame message
             * \param frame_id - id of the frame
             * \param timestamp - timestamp of the event
             */
            frame(frame_id_t frame_id, timetag_t timestamp);

            /**
             * \brief Creates new, fully specified frame message
             * 
             * \param frame_id - id of the frame
             * \param timestamp - timestamp of the event
             * \param appname - name of the sender m_application
             * \param address - address of the sender
             * \param instance - instance ID of the sender
             * \param sensor_width - width of the sensor
             * \param sensor_height - height of the sensor
             */
            frame(frame_id_t frame_id, timetag_t timestamp, std::string appname,
                addr_ipv4_t address, instance_id_t instance, dimmension_t sensor_width, dimmension_t sensor_height);

            /**
             * \brief Creates a deep copy of the frame message
             * \param original - the original message to make copy of
             */
            frame(const frame & original);

            frame & operator=(const frame & second);
            bool operator == (const frame & second) const ;
            inline bool operator != (const frame & second) const { return !operator==(second); }

            /**
             * \brief Gets the id of the frame
             * \return frame id or \ref OUT_OF_ORDER_FRAME_ID if out-of order execution is set
             */
            inline frame_id_t get_frame_id() const { return m_f_id; }

            /**
             * \brief Gets the timestamp of the event that this frame holds
             * \return the timestamp
             */
            inline timetag_t get_timestamp() const { return m_time; }

            /**
             * \brief Gets the name of the sender application
             * \return sender appname or empty string if not set
             */
            inline std::string get_app_name() const { return m_app; }

            /**
             * \brief Gets the sender IPv4 address
             * \return sender ip or 0 if not set
             */
            inline addr_ipv4_t get_address() const { return m_addr; }

            /**
             * \brief Gets the sensor width
             * \return sensor width or 0 if not set
             */
            inline dimmension_t get_sensor_width() const { return m_dim_x; }

            /**
             * \brief Gets the sensor height
             * \return sensor height or 0 if not set
             */
            inline dimmension_t get_sensor_height() const { return m_dim_y; }

            /**
             * \brief Gets the sender application instance id
             * \return instance id or 0 if not set
             */
            inline instance_id_t get_instance() const { return m_inst; }

            /**
             * \brief Sets the sender IPv4 address
             * \param address - address to set
             * \return previous setting
             */
            addr_ipv4_t set_address(addr_ipv4_t address);

            /**
             * \brief Sets the sender application name
             * \param appname - address to set
             * \return previous setting
             */
            std::string set_app_name(std::string appname);

            /**
             * \brief Sets the sensor width
             * \param sensor_width - the width of the sensor
             * \return previous setting
             */
            dimmension_t set_sensor_width(dimmension_t sensor_width);

            /**
             * \brief Sets the frame dimmension attribute
             * \param dim - the compiled dimmensions of the sensor
             * \return previous setting
             */
            dimmensions_t set_sensor_dim(dimmensions_t dim);

            /**
             * \brief Sets the sensor height
             * \param sensor_height - the height of the sensor
             * \return previous setting
             */
            dimmension_t set_sensor_height(dimmension_t sensor_height);

            /**
             * \brief Sets the frame id
             * \param frame_id - frame id to set, \ref OUT_OF_ORDER_FRAME_ID serves
             * as both default and out-of-order execution frame id
             * \return previous setting
             */
            frame_id_t set_frame_id(frame_id_t frame_id);

            /**
             * \brief Sets the sender application instance id
             * \param instance - instance id to set
             * \return previous setting
             */
            instance_id_t set_instance(instance_id_t instance);
            
            /**
             * \brief Sets the timestamp of the outgoing event
             * \param timestamp - timestamp to set
             * \return previous setting
             */
            timetag_t set_timestamp(timetag_t timestamp);

            /**
             * \brief Checks for extended attributes
             * 
             * Checks whether at least one of the following attributes are set
             * depending on the message output mode
             * 
             * \li application id
             * \li sender address
             * \li application instance id
             * \li sensor dimmensions
             * 
             * \return true only if any of the above values are set
             */
            bool is_extended() const ;

            kerat_message * clone() const ;

            void print(std::ostream & output) const ;

            //! OSC path for TUIO 2.0 frame message
            static const char * PATH;
            
            //! Frame ID reserved for out-of-order execution
            static const frame_id_t OUT_OF_ORDER_FRAME_ID = 0;

        private:

            bool imprint_lo_messages(lo_bundle target) const;

            frame_id_t m_f_id;
            timetag_t m_time;
            std::string m_app;
            addr_ipv4_t m_addr;
            instance_id_t m_inst;
            dimmension_t m_dim_x;
            dimmension_t m_dim_y;
        }; // cls frame

    } // ns message
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::frame &  msg_frm);

#endif // KERAT_TUIO_MESSAGE_FRAME_HPP

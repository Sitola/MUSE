/**
 * \file      tuio_message_pointer.hpp
 * \brief     TUIO 2.0 pointer (/tuio2/p3d, /tuio2/ptr)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-09-19 13:51 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGE_POINTER_HPP
#define KERAT_TUIO_MESSAGE_POINTER_HPP

#include <kerat/message.hpp>
#include <kerat/typedefs.hpp>
#include <kerat/message_helpers.hpp>
#include <ostream>

namespace libkerat {
    namespace message {

        /** 
         * \brief TUIO 2.0 pointer (/tuio2/p3d, /tuio2/ptr)
         * 
         * \note For TUIO 2.0 draft (non)compliance, see below
         * \see \ref normalized-coordinates, \ref normalized-distance,
         * \ref normalized-velocity, \ref normalized-acceleration, \ref normalized-pressure
         */
        class pointer: public libkerat::kerat_message,
            public libkerat::helpers::contact_session,
            public libkerat::helpers::contact_type_user,
            public libkerat::helpers::contact_component,
            public libkerat::helpers::point_3d,
            public libkerat::helpers::velocity_3d,
            public libkerat::helpers::movement_acceleration,
            public libkerat::helpers::message_output_mode,
            public libkerat::helpers::movable_3d,
            public libkerat::helpers::scalable_independent_3d,
            public libkerat::helpers::rotatable_cs_3d
        {
        public:

            /**
             *  \brief Creates new, empty TUIO pointer
             * 
             * All message components (and corresponding helpers) are initialized
             * through their default constructors. The message output mode is set
             * to \ref OUTPUT_MODE_BOTH.
             */
            pointer();

            /**
             * \brief Creates new, fully specified 2D TUIO pointer
             * 
             * The message output mode is set to \ref OUTPUT_MODE_2D.
             * 
             * \param session_id - session id of the contact
             * \param type_id    - type id of the contact (see corresponding constants of the \ref helpers::contact_type_user helper)
             * \param user_id    - id of the user manipulating the contact (see corresponding constants of the \ref helpers::contact_type_user helper)
             * \param component_id - component id of the contact - each contact can have several components
             * \param x          - X coordinate
             * \param y          - Y coordinate
             * \param contact_width - width of the contact, contact is considered
             * to be of circular shape
             * \param contact_pressure - pressure on contact surface, for TUIO 2.0 draft compliance
             * use the normalized \<0; 1\> range or negative value to indicate howering
             * \param x_velocity - X axis component of the movement velocity
             * \param y_velocity - Y axis component of the movement velocity
             * \param movement_accel    - overall acceleration in the movement direction
             */
            pointer(const session_id_t session_id, const type_id_t type_id, const user_id_t user_id, const component_id_t component_id,
                const coord_t x, const coord_t y,
                const distance_t contact_width, const pressure_t contact_pressure,
                const velocity_t x_velocity, const velocity_t y_velocity,
                const accel_t movement_accel
            );

            /**
             * \brief Creates new, short 2D TUIO pointer
             * 
             * All other message components (and corresponding helpers) are
             * initialized through their default constructors. The message output
             * mode is set to \ref OUTPUT_MODE_2D.
             * 
             * \param session_id - session id of the contact
             * \param type_id    - type id of the contact (see corresponding constants of the \ref helpers::contact_type_user helper)
             * \param user_id    - id of the user manipulating the contact (see corresponding constants of the \ref helpers::contact_type_user helper)
             * \param component_id - component id of the contact - each contact can have several components
             * \param x          - X coordinate
             * \param y          - Y coordinate
             * \param contact_width - width of the contact, contact is considered
             * to be of circular shape
             * \param contact_pressure - pressure on contact surface, for TUIO 2.0 draft compliance
             * use the normalized \<0; 1\> range or negative value to indicate howering
             */
            pointer(const session_id_t session_id, const type_id_t type_id, const user_id_t user_id, const component_id_t component_id,
                const coord_t x, const coord_t y,
                const distance_t contact_width, const pressure_t contact_pressure
            );

            /**
             * \brief Creates new, short 3D TUIO pointer
             * 
             * All other message components (and corresponding helpers) are
             * initialized through their default constructors. The message output
             * mode is set to \ref OUTPUT_MODE_3D.
             * 
             * \param session_id - session id of the contact
             * \param type_id    - type id of the contact (see corresponding constants of the \ref helpers::contact_type_user helper)
             * \param user_id    - id of the user manipulating the contact (see corresponding constants of the \ref helpers::contact_type_user helper)
             * \param component_id - component id of the contact - each contact can have several components
             * \param x          - X coordinate
             * \param y          - Y coordinate
             * \param z          - Z coordinate
             * \param contact_width - width of the contact, contact is considered
             * to be of circular shape
             * \param contact_pressure - pressure on contact surface, for TUIO 2.0 draft compliance
             * use the normalized \<0; 1\> range or negative value to indicate howering
             */
            pointer(const session_id_t session_id, const type_id_t type_id, const user_id_t user_id, const component_id_t component_id,
                const coord_t x, const coord_t y, const coord_t z,
                const distance_t contact_width, const pressure_t contact_pressure
            );


            /**
             * \brief Creates new, fully specified 3D TUIO pointer
             * 
             * All other message components (and corresponding helpers) are
             * initialized through their default constructors. The message output
             * mode is set to \ref OUTPUT_MODE_3D.
             * 
             * \param session_id - session id of the contact
             * \param type_id    - type id of the contact (see corresponding constants of the \ref helpers::contact_type_user helper)
             * \param user_id    - id of the user manipulating the contact (see corresponding constants of the \ref helpers::contact_type_user helper)
             * \param component_id - component id of the contact - each contact can have several components
             * \param x          - X coordinate
             * \param y          - Y coordinate
             * \param z          - Z coordinate
             * \param contact_width - width of the contact, contact is considered
             * to be of circular shape
             * \param contact_pressure - pressure on contact surface, for TUIO 2.0 draft compliance
             * use the normalized \<0; 1\> range or negative value to indicate howering
             * \param x_velocity - X axis component of the movement velocity
             * \param y_velocity - Y axis component of the movement velocity
             * \param z_velocity - Z axis component of the movement velocity
             * \param movement_accel    - overall acceleration in the movement direction
             */
            pointer(const session_id_t session_id, const type_id_t type_id, const user_id_t user_id, const component_id_t component_id,
                const coord_t x, const coord_t y, const coord_t z,
                const distance_t contact_width, const pressure_t contact_pressure,
                const velocity_t x_velocity, const velocity_t y_velocity, const velocity_t z_velocity,
                const accel_t movement_accel
            );
            
            //! \brief Create a deep copy of this message
            pointer(const pointer & original);

            pointer & operator=(const pointer & second);
            bool operator == (const pointer & second) const ;
            inline bool operator != (const pointer & second) const { return !operator==(second); }

            /**
             * \brief Gets the pressure
             * \return pressure or 0 if not set
             */
            inline pressure_t get_pressure() const { return m_press; }

            /**
             * \brief Set the presure
             * \param pressure - pressure to be set, for TUIO 2.0 draft compliance
             * use the normalized \<0; 1\> range or negative value to indicate howering
             * \note For TUIO 2.0 draft (non)compliance, see \ref normalized-pressure
             * \see libkerat::normalize
             * \return previous setting
             */
            pressure_t set_pressure(pressure_t pressure);


            /**
             * \brief Gets the contact width
             * \return width or 0 if not set
             */
            inline distance_t get_width() const { return m_width; }

            /**
             * \brief Set the contact width
             * \param contact_width - contact width to be set, contact is considered
             * to be of circular shape, for TUIO 2.0 draft compliance
             * use the normalized \<0; 1\> range
             * \note For TUIO 2.0 draft (non)compliance, see \ref normalized-distance
             * \see libkerat::normalize
             * \return previous setting
             */
            distance_t set_width(distance_t contact_width);

            /**
             * \brief Checks for extended attributes
             * 
             * Checks whether at least one of the following attributes are set
             * depending on the message output mode
             * 
             * \li velocity
             * \li acceleration
             * 
             * \return true if any of the above values are set for given message output mode, false otherwise
             */
            bool is_extended() const ;

            kerat_message * clone() const ;

            void print(std::ostream & output) const ;

            // overrides for movable and rotatable
            void move_x(coord_t factor);
            void move_y(coord_t factor);
            void move_z(coord_t factor);
            void scale_x(float factor);
            void scale_y(float factor);
            void scale_z(float factor);
            void rotate_by(angle_t angle, const helpers::point_2d & center);
            void rotate_pitch(angle_t angle, const helpers::point_3d & center);
            void rotate_roll(angle_t angle, const helpers::point_3d & center);
            
            //! \brief OSC path for the 2D pointer
            static const char * PATH_2D;
            //! \brief OSC path for the 3D pointer
            static const char * PATH_3D;

        private:

            bool imprint_lo_messages(lo_bundle target) const;

            distance_t m_width;
            pressure_t m_press;

        }; // cls pointer

    } // ns message
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::pointer &  msg_ptr);

#endif // KERAT_TUIO_MESSAGE_POINTER_HPP
